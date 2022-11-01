//===-- Y86TargetFrameLowering.h - Define frame lowering for Y86 -*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class implements Y86-specific bits of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Y86_Y86FRAMELOWERING_H
#define LLVM_LIB_TARGET_Y86_Y86FRAMELOWERING_H

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Support/TypeSize.h"

namespace llvm {

class Y86Subtarget;
class Y86FrameLowering : public TargetFrameLowering {
protected:
  const Y86Subtarget &STI;

public:
  
  explicit Y86FrameLowering(const Y86Subtarget &sti, unsigned Alignment)
      : TargetFrameLowering(StackGrowsDown, Align(Alignment), 0,
                            Align(Alignment)),
        STI(sti) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  bool hasFP(const MachineFunction &MF) const override;
};
} // namespace llvm

#endif