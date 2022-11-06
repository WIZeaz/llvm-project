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
  RET_FLAG,
};
} // namespace Y86ISD

class Y86TargetLowering final : public TargetLowering {
public:
  explicit Y86TargetLowering(const Y86TargetMachine &TM,
                             const Y86Subtarget &STI);
  bool useSoftFloat() const override { return true; }
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;
  const char *getTargetNodeName(unsigned Opcode) const override;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;
  SDValue LowerMemArgument(SDValue Chain, CallingConv::ID CallConv,
                           const SmallVectorImpl<ISD::InputArg> &Ins,
                           const SDLoc &dl, SelectionDAG &DAG,
                           const CCValAssign &VA, MachineFrameInfo &MFI,
                           unsigned i) const;
  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &dl,
                      SelectionDAG &DAG) const;

private:
  const Y86Subtarget &Subtarget;
};

} // namespace llvm

#endif