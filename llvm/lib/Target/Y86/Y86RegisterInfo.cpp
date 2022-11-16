//===-- Y86RegisterInfo.cpp - Y86 Register Information -== --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Y86 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "Y86RegisterInfo.h"
#include "MCTargetDesc/Y86MCTargetDesc.h"
#include "Y86Subtarget.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <iterator>

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "Y86GenRegisterInfo.inc"

#define DEBUG_TYPE "y86-reg-info"

Y86RegisterInfo::Y86RegisterInfo(const Y86Subtarget &ST)
    : Y86GenRegisterInfo(Y86::RIP), Subtarget(ST) {
  SlotSize = 8;
  StackPtr = Y86::RSP;
  FramePtr = Y86::RBP;
  BasePtr = Y86::RBX;
}

const TargetRegisterClass *
Y86RegisterInfo::getPointerRegClass(const MachineFunction &MF,
                                    unsigned Kind) const {
  switch (Kind) {
  case 0:
    return &Y86::GR64RegClass;
  case 1:
    return &Y86::GR64_NOSPRegClass;
  case 2:
    return &Y86::GR64_NOREXRegClass;
  case 3:
    return &Y86::GR64_NOREX_NOSPRegClass;
  default:
    llvm_unreachable("unexpected PointerRegClass");
  }
}

const MCPhysReg *
Y86RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_32_SaveList;
}

const uint32_t *Y86RegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                                      CallingConv::ID) const {
  return CSR_32_RegMask;
}

BitVector Y86RegisterInfo::getReservedRegs(const MachineFunction &MF) const {

  BitVector Reserved(getNumRegs());

  // set stack pointer
  for (const MCPhysReg &SubReg : subregs_inclusive(Y86::RSP))
    Reserved.set(SubReg);

  // Set the Shadow Stack Pointer as reserved.
  // Reserved.set(Y86::SSP);

  // Set the instruction pointer register and its aliases as reserved.
  for (const MCPhysReg &SubReg : subregs_inclusive(Y86::RIP))
    Reserved.set(SubReg);

  auto TFI = MF.getSubtarget().getFrameLowering();

  // Set the frame-pointer register and its aliases as reserved if needed.
  if (TFI->hasFP(MF)) {
    for (const MCPhysReg &SubReg : subregs_inclusive(Y86::RBP))
      Reserved.set(SubReg);
  }

  Reserved.set(Y86::CS);
  Reserved.set(Y86::SS);
  Reserved.set(Y86::DS);
  Reserved.set(Y86::ES);
  Reserved.set(Y86::FS);
  Reserved.set(Y86::GS);

  return Reserved;
}

void Y86RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                          int SPAdj, unsigned FIOperandNum,
                                          RegScavenger *RS) const {
  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const Y86InstrInfo *TII = Subtarget.getInstrInfo();
  MachineBasicBlock::iterator MBBI = MBB.getFirstTerminator();
  const Y86FrameLowering *TFI = getFrameLowering(MF);
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  // Determine base register and offset.
  int FIOffset;
  Register BasePtr;
  if (MI.isReturn()) {
    llvm_unreachable("MI should not be ret");
  } else {
    FIOffset = TFI->getFrameIndexReference(MF, FrameIndex, BasePtr).getFixed();
  }

  // LOCAL_ESCAPE uses a single offset, with no register. It only works in the
  // simple FP case, and doesn't work with stack realignment. On 32-bit, the
  // offset is from the traditional base pointer location.  On 64-bit, the
  // offset is from the SP at the end of the prologue, not the FP location. This
  // matches the behavior of llvm.frameaddress.
  unsigned Opc = MI.getOpcode();
  if (Opc == TargetOpcode::LOCAL_ESCAPE) {
    MachineOperand &FI = MI.getOperand(FIOperandNum);
    FI.ChangeToImmediate(FIOffset);
    return;
  }

  if (Opc == Y86::ADD32ri || Opc == Y86::ADD64ri) {
    assert(BasePtr == FramePtr && "Expected the FP as base register");
    Register DestReg = MI.getOperand(0).getReg();
    BuildMI(MBB, II, MI.getDebugLoc(), TII->get(Y86::MOV64rr), DestReg)
        .addReg(BasePtr);

    MI.getOperand(FIOperandNum).ChangeToRegister(DestReg, false);
    int64_t Offset = MI.getOperand(FIOperandNum + 1).getImm() + FIOffset;
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);

    return;
  }

  Register MachineBasePtr = BasePtr;

  // This must be part of a four operand memory reference.  Replace the
  // FrameIndex with base register.  Add an offset to the offset.
  MI.getOperand(FIOperandNum).ChangeToRegister(MachineBasePtr, false);

  if (BasePtr == StackPtr)
    FIOffset += SPAdj;

  if (MI.getOperand(FIOperandNum + 3).isImm()) {
    // Offset is a 32-bit integer.
    int Imm = (int)(MI.getOperand(FIOperandNum + 3).getImm());
    int Offset = FIOffset + Imm;
    assert(isInt<32>((long long)FIOffset + Imm) &&
           "Requesting 64-bit offset in 32-bit immediate!");
    if (Offset != 0)
      MI.getOperand(FIOperandNum + 3).ChangeToImmediate(Offset);
  } else {
    // Offset is symbolic. This is extremely rare.
    uint64_t Offset =
        FIOffset + (uint64_t)MI.getOperand(FIOperandNum + 3).getOffset();
    MI.getOperand(FIOperandNum + 3).setOffset(Offset);
  }
}

bool Y86RegisterInfo::requiresRegisterScavenging(
    const MachineFunction &MF) const {
  return true;
}

bool Y86RegisterInfo::trackLivenessAfterRegAlloc(
    const MachineFunction &MF) const {
  return true;
}

Register Y86RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const Y86FrameLowering *TFI = getFrameLowering(MF);
  return TFI->hasFP(MF) ? FramePtr : StackPtr;
}
