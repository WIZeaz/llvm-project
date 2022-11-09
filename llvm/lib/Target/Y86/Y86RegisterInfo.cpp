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
// #include "Y86MachineFunction.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "Y86GenRegisterInfo.inc"

#define DEBUG_TYPE "y86-reg-info"

Y86RegisterInfo::Y86RegisterInfo(const Y86Subtarget &ST)
    : Y86GenRegisterInfo(Y86::EIP), Subtarget(ST) {
  SlotSize = 8;
  StackPtr = Y86::ESP;
  FramePtr = Y86::EBP;
  BasePtr = Y86::EBX;
}

const TargetRegisterClass *
Y86RegisterInfo::getPointerRegClass(const MachineFunction &MF,
                                    unsigned Kind) const {
  return &Y86::GR32RegClass;
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
  for (const MCPhysReg &SubReg : subregs_inclusive(Y86::ESP))
    Reserved.set(SubReg);

  // Set the Shadow Stack Pointer as reserved.
  // Reserved.set(Y86::SSP);

  // Set the instruction pointer register and its aliases as reserved.
  for (const MCPhysReg &SubReg : subregs_inclusive(Y86::EIP))
    Reserved.set(SubReg);

  auto TFI = MF.getSubtarget().getFrameLowering();

  // Set the frame-pointer register and its aliases as reserved if needed.
  if (TFI->hasFP(MF)) {
    for (const MCPhysReg &SubReg : subregs_inclusive(Y86::EBP))
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
  // llvm_unreachable("???");
  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  MachineBasicBlock::iterator MBBI = MBB.getFirstTerminator();
  const Y86FrameLowering *TFI = getFrameLowering(MF);
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  std::cerr << "=====\n";
  MI.dump();
  // Determine base register and offset.
  int FIOffset;
  Register BasePtr;
  if (MI.isReturn()) {
    llvm_unreachable("MI should not be ret");
    /* assert((!hasStackRealignment(MF) ||
            MF.getFrameInfo().isFixedObjectIndex(FrameIndex)) &&
           "Return instruction can only reference SP relative frame objects");
    FIOffset =
        TFI->getFrameIndexReferenceSP(MF, FrameIndex, BasePtr, 0).getFixed(); */
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

  // For LEA64_32r when BasePtr is 32-bits (X32) we can use full-size 64-bit
  // register as source operand, semantic is the same and destination is
  // 32-bits. It saves one byte per lea in code since 0x67 prefix is avoided.
  // Don't change BasePtr since it is used later for stack adjustment.
  Register MachineBasePtr = BasePtr;

  // This must be part of a four operand memory reference.  Replace the
  // FrameIndex with base register.  Add an offset to the offset.
  MI.getOperand(FIOperandNum).ChangeToRegister(MachineBasePtr, false);

  if (BasePtr == StackPtr)
    FIOffset += SPAdj;

  // The frame index format for stackmaps and patchpoints is different from the
  // Y86 format. It only has a FI and an offset.
  if (Opc == TargetOpcode::STACKMAP || Opc == TargetOpcode::PATCHPOINT) {
    assert(BasePtr == FramePtr && "Expected the FP as base register");
    int64_t Offset = MI.getOperand(FIOperandNum + 1).getImm() + FIOffset;
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
    return;
  }

  if (MI.getOperand(FIOperandNum + 3).isImm()) {
    // Offset is a 32-bit integer.
    int Imm = (int)(MI.getOperand(FIOperandNum + 3).getImm());
    int Offset = FIOffset + Imm;
    /* assert((!Is64Bit || isInt<32>((long long)FIOffset + Imm)) &&
           "Requesting 64-bit offset in 32-bit immediate!"); */
    if (Offset != 0)
      MI.getOperand(FIOperandNum + 3).ChangeToImmediate(Offset);
  } else {
    // Offset is symbolic. This is extremely rare.
    uint64_t Offset =
        FIOffset + (uint64_t)MI.getOperand(FIOperandNum + 3).getOffset();
    MI.getOperand(FIOperandNum + 3).setOffset(Offset);
  }
  MI.dump();
  std::cerr << "=====\n";
}

bool Y86RegisterInfo::requiresRegisterScavenging(
    const MachineFunction &MF) const {
  return true;
}

bool Y86RegisterInfo::trackLivenessAfterRegAlloc(
    const MachineFunction &MF) const {
  return true;
}

// pure virtual method
Register Y86RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const Y86FrameLowering *TFI = getFrameLowering(MF);
  return TFI->hasFP(MF) ? FramePtr : StackPtr;
}
