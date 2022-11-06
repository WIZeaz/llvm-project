//===-- Y86MachineFunctionInfo.cpp - Y86 machine function info ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Y86MachineFunctionInfo.h"
#include "Y86RegisterInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"

using namespace llvm;

void Y86MachineFunctionInfo::anchor() { }

void Y86MachineFunctionInfo::setRestoreBasePointer(const MachineFunction *MF) {
  if (!RestoreBasePointerOffset) {
    const Y86RegisterInfo *RegInfo = static_cast<const Y86RegisterInfo *>(
      MF->getSubtarget().getRegisterInfo());
    unsigned SlotSize = RegInfo->getSlotSize();
    for (const MCPhysReg *CSR = MF->getRegInfo().getCalleeSavedRegs();
         unsigned Reg = *CSR; ++CSR) {
      if (Y86::GR32RegClass.contains(Reg))
        RestoreBasePointerOffset -= SlotSize;
    }
  }
}

