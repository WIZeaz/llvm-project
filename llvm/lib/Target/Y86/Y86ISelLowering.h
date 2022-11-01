//===-- Y86ISelLowering.h - Y86 DAG Lowering Interface ----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Y86 uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Y86_Y86ISELLOWERING_H
#define LLVM_LIB_TARGET_Y86_Y86ISELLOWERING_H

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
class Y86Subtarget;
class Y86TargetMachine;

namespace Y86ISD {
enum NodeType : unsigned {
  // Start the numbering where the builtin ops leave off.
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  CALL,
};
} // namespace Y86ISD

class Y86TargetLowering final : public TargetLowering {
public:
  explicit Y86TargetLowering(const Y86TargetMachine &TM,
                             const Y86Subtarget &STI);
  bool useSoftFloat() const override { return true; }
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;
  const char *getTargetNodeName(unsigned Opcode) const override;

private:
  const Y86Subtarget &Subtarget;
};

} // namespace llvm

#endif