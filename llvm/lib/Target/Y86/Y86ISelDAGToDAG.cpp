//===- Y86ISelDAGToDAG.cpp - A DAG pattern matching inst selector for Y86 -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines a DAG pattern matching instruction selector for Y86,
// converting from a legalized dag to a Y86 dag.
//
//===----------------------------------------------------------------------===//

#include "Y86ISelDAGToDAG.h"
#include "Y86.h"
#include "Y86MachineFunctionInfo.h"
#include "Y86RegisterInfo.h"
#include "Y86Subtarget.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/KnownBits.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;

#define DEBUG_TYPE "y86-isel"

void Y86DAGToDAGISel::Select(SDNode *Node) {

  unsigned Opcode = Node->getOpcode();

  if (Node->isMachineOpcode()) {
    LLVM_DEBUG(errs() << "== "; Node->dump(CurDAG); errs() << "\n");
    Node->setNodeId(-1);
    return;
  }

  switch (Opcode) {
  default:
    break;
  }

  // Select the default instruction
  SelectCode(Node);
}

bool Y86DAGToDAGISel::shouldAvoidImmediateInstFormsForSize(SDNode *N) const{
  return false;
}

/// Returns true if it is able to pattern match an addressing mode.
/// It returns the operands which make up the maximal addressing mode it can
/// match by reference.
///
/// Parent is the parent node of the addr operand that is being matched.  It
/// is always a load, store, atomic node, or null.  It is only null when
/// checking memory operands for inline asm nodes.
// addr= Base+Scale*Index+[Disp] Scale=1,2,4,8
// Segment is deprecated in x86-64
bool Y86DAGToDAGISel::selectAddr(SDNode *Parent, SDValue N, SDValue &Base,
                                 SDValue &Scale, SDValue &Index, SDValue &Disp,
                                 SDValue &Segment) {
  SDLoc DL(N);
  MVT VT = N.getSimpleValueType();
  if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(N)) {
    Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), VT);
    SDValue zero = CurDAG->getTargetConstant(0, DL, VT);
    Scale = zero;
    Index = zero;
    Disp = zero;
    Segment = zero;
    return true;
  }
  return false;
}

FunctionPass *createY86ISelDag(Y86TargetMachine &TM,
                               CodeGenOpt::Level OptLevel) {
  return new Y86DAGToDAGISel(TM, OptLevel);
}
