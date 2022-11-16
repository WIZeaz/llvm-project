//===-- Y86InstrInfo.cpp - Y86 Instruction Information --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Y86 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "Y86InstrInfo.h"
#include "MCTargetDesc/Y86MCTargetDesc.h"
#include "Y86InstrBuilder.h"
#include "Y86MachineFunctionInfo.h"
#include "Y86RegisterInfo.h"
#include "Y86Subtarget.h"
#include "Y86TargetMachine.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Sequence.h"
#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/LiveVariables.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/StackMaps.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

#define DEBUG_TYPE "y86-instr-info"

#define GET_INSTRINFO_CTOR_DTOR
#include "Y86GenInstrInfo.inc"

// Pin the vtable to this file.
void Y86InstrInfo::anchor() {}

Y86InstrInfo::Y86InstrInfo(Y86Subtarget &STI)
    : Y86GenInstrInfo(),
      Subtarget(STI), RI(STI) {
}

void Y86InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator MI,
                               const DebugLoc &DL, MCRegister DestReg,
                               MCRegister SrcReg, bool KillSrc) const {
  // First deal with the normal symmetric copies.
  // bool HasAVX = Subtarget.hasAVX();
  // bool HasVLX = Subtarget.hasVLX();
  unsigned Opc = 0;
  if (Y86::GR64RegClass.contains(DestReg, SrcReg))
    Opc = Y86::MOV64rr;
  else if (Y86::GR32RegClass.contains(DestReg, SrcReg))
    Opc = Y86::MOV32rr;
  else if (Y86::GR16RegClass.contains(DestReg, SrcReg))
    Opc = Y86::MOV16rr;
  else
    llvm_unreachable("unsupport copy regclass");

  if (Opc) {
    BuildMI(MBB, MI, DL, get(Opc), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
    return;
  }

  if (SrcReg == Y86::EFLAGS || DestReg == Y86::EFLAGS) {
    // FIXME: We use a fatal error here because historically LLVM has tried
    // lower some of these physreg copies and we want to ensure we get
    // reasonable bug reports if someone encounters a case no other testing
    // found. This path should be removed after the LLVM 7 release.
    report_fatal_error("Unable to copy EFLAGS physical register!");
  }

  LLVM_DEBUG(dbgs() << "Cannot copy " << RI.getName(SrcReg) << " to "
                    << RI.getName(DestReg) << '\n');
  report_fatal_error("Cannot emit physreg copy instruction");
}

static unsigned getLoadStoreRegOpcode(Register Reg,
                                      const TargetRegisterClass *RC,
                                      bool IsStackAligned,
                                      const Y86Subtarget &STI, bool load) {

  switch (STI.getRegisterInfo()->getSpillSize(*RC)) {
  default:
    llvm_unreachable("Unknown spill size");
  case 1:
    llvm_unreachable("unsupport spill size 1");
  case 2:
    assert(Y86::GR16RegClass.hasSubClassEq(RC) && "Unknown 2-byte regclass");
    return load ? Y86::MOV16rm : Y86::MOV16mr;
  case 4:
    if (Y86::GR32RegClass.hasSubClassEq(RC))
      return load ? Y86::MOV32rm : Y86::MOV32mr;
    llvm_unreachable("Unknown 4-byte regclass");
  }
}

static unsigned getStoreRegOpcode(Register SrcReg,
                                  const TargetRegisterClass *RC,
                                  bool IsStackAligned,
                                  const Y86Subtarget &STI) {
  return getLoadStoreRegOpcode(SrcReg, RC, IsStackAligned, STI, false);
}

static unsigned getLoadRegOpcode(Register DestReg,
                                 const TargetRegisterClass *RC,
                                 bool IsStackAligned, const Y86Subtarget &STI) {
  return getLoadStoreRegOpcode(DestReg, RC, IsStackAligned, STI, true);
}

void Y86InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MI,
                                       Register SrcReg, bool isKill,
                                       int FrameIdx,
                                       const TargetRegisterClass *RC,
                                       const TargetRegisterInfo *TRI) const {
  const MachineFunction &MF = *MBB.getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  assert(MFI.getObjectSize(FrameIdx) >= TRI->getSpillSize(*RC) &&
         "Stack slot too small for store");
  unsigned Alignment = std::max<uint32_t>(TRI->getSpillSize(*RC), 16);
  bool isAligned =
      (Subtarget.getFrameLowering()->getStackAlign() >= Alignment) ||
      (RI.canRealignStack(MF) && !MFI.isFixedObjectIndex(FrameIdx));
  unsigned Opc = getStoreRegOpcode(SrcReg, RC, isAligned, Subtarget);
  addFrameReference(BuildMI(MBB, MI, DebugLoc(), get(Opc)), FrameIdx)
      .addReg(SrcReg, getKillRegState(isKill));
}

void Y86InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MI,
                                        Register DestReg, int FrameIdx,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI) const {
  const MachineFunction &MF = *MBB.getParent();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  unsigned Alignment = std::max<uint32_t>(TRI->getSpillSize(*RC), 16);
  bool isAligned =
      (Subtarget.getFrameLowering()->getStackAlign() >= Alignment) ||
      (RI.canRealignStack(MF) && !MFI.isFixedObjectIndex(FrameIdx));
  unsigned Opc = getLoadRegOpcode(DestReg, RC, isAligned, Subtarget);
  addFrameReference(BuildMI(MBB, MI, DebugLoc(), get(Opc), DestReg), FrameIdx);
}

bool Y86InstrInfo::expandPostRAPseudo(MachineInstr &MI) const {
  MachineBasicBlock &MBB = *MI.getParent();
  switch (MI.getOpcode()) {
  case Y86::RET: {
    // Adjust stack to erase error code
    int64_t StackAdj = MI.getOperand(0).getImm();
    MachineInstrBuilder MIB;
    if (StackAdj == 0) {
      MIB = BuildMI(MBB, MI, MI.getDebugLoc(), get(Y86::RET32));
    } else if (isUInt<16>(StackAdj)) {
      MIB =
          BuildMI(MBB, MI, MI.getDebugLoc(), get(Y86::RETI32)).addImm(StackAdj);
    } else {
      report_fatal_error("StackAdj is too large");
    }
    for (unsigned I = 1, E = MI.getNumOperands(); I != E; ++I)
      MIB.add(MI.getOperand(I));
    MBB.erase(MI);
    return true;
  }
  }
  return false;
}

#define GET_INSTRINFO_HELPERS
#include "Y86GenInstrInfo.inc"
