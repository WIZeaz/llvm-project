//===-- Y86InstrInfo.h - Y86 Instruction Information ------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_Y86_Y86INSTRINFO_H
#define LLVM_LIB_TARGET_Y86_Y86INSTRINFO_H

// #include "MCTargetDesc/Y86BaseInfo.h"
#include "Y86RegisterInfo.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include <vector>

#define GET_INSTRINFO_HEADER
#include "Y86GenInstrInfo.inc"

namespace llvm {
class Y86Subtarget;

class Y86InstrInfo final : public Y86GenInstrInfo {
  Y86Subtarget &Subtarget;
  const Y86RegisterInfo RI;

  virtual void anchor();

public:
  explicit Y86InstrInfo(Y86Subtarget &STI);

  /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  /// such, whenever a client has an instance of instruction info, it should
  /// always be able to get register info as well (through this method).
  ///
  const Y86RegisterInfo &getRegisterInfo() const { return RI; }

  /// Returns the stack pointer adjustment that happens inside the frame
  /// setup..destroy sequence (e.g. by pushes, or inside the callee).
  int64_t getFrameAdjustment(const MachineInstr &I) const {
    assert(isFrameInstr(I));
    if (isFrameSetup(I))
      return I.getOperand(2).getImm();
    return I.getOperand(1).getImm();
  }

  /// Sets the stack pointer adjustment made inside the frame made up by this
  /// instruction.
  void setFrameAdjustment(MachineInstr &I, int64_t V) const {
    assert(isFrameInstr(I));
    if (isFrameSetup(I))
      I.getOperand(2).setImm(V);
    else
      I.getOperand(1).setImm(V);
  }

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, MCRegister DestReg, MCRegister SrcReg,
                   bool KillSrc) const override;
  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI, Register SrcReg,
                           bool isKill, int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI) const override;

  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI, Register DestReg,
                            int FrameIndex, const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI) const override;

  bool expandPostRAPseudo(MachineInstr &MI) const override;


#define GET_INSTRINFO_HELPER_DECLS
#include "Y86GenInstrInfo.inc"

protected:
private:
};

} // namespace llvm

#endif
