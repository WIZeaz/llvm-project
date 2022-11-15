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
#include "llvm/CodeGen/ISDOpcodes.h"
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
#include <iostream>

using namespace llvm;

#define DEBUG_TYPE "y86-isel"

void Y86DAGToDAGISel::selectFrameIndex(SDNode *SN, SDNode *N, unsigned Offset) {
  SDLoc DL(SN);
  MVT VT = N->getSimpleValueType(0);
  int FI = cast<FrameIndexSDNode>(N)->getIndex();
  SDValue TFI = CurDAG->getTargetFrameIndex(FI, VT);
  unsigned Opc;
  if (VT == MVT::i32) {
    Opc = Y86::ADD32ri;
  } else if (VT == MVT::i64) {
    Opc = Y86::ADD64ri;
  } else
    llvm_unreachable("unsuported FrameIndex ValueType");

  auto OffsetConstant = CurDAG->getTargetConstant(
      Offset, DL, TLI->getPointerTy(CurDAG->getDataLayout()));

  if (SN->hasOneUse())
    CurDAG->SelectNodeTo(SN, Opc, VT, TFI, OffsetConstant);
  else
    ReplaceNode(SN, CurDAG->getMachineNode(Opc, DL, VT, TFI,
                                           OffsetConstant));
}

void Y86DAGToDAGISel::Select(SDNode *Node) {

  unsigned Opcode = Node->getOpcode();

  if (Node->isMachineOpcode()) {
    LLVM_DEBUG(errs() << "== "; Node->dump(CurDAG); errs() << "\n");
    Node->setNodeId(-1);
    return;
  }

  switch (Opcode) {
  case ISD::FrameIndex:
    selectFrameIndex(Node, Node);
    return;
  }

  // Select the default instruction
  SelectCode(Node);
}

bool Y86DAGToDAGISel::shouldAvoidImmediateInstFormsForSize(SDNode *N) const {
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
  SDValue Zero = CurDAG->getTargetConstant(0, DL, VT);
  Scale = Zero;
  Index = Zero;
  Disp = Zero;
  Segment = Zero;
  LLVM_DEBUG(
    dbgs()<<"start selectAddr";
    N.dump();
    Parent->dump()
  );
  

  // [Base]
  if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(N)) {
    LLVM_DEBUG(dbgs()<<"select [Base] mode (FrameIndex)\n");
    Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), VT);
    return true;
  }

  // [Base] + disp
  if (CurDAG->isBaseWithConstantOffset(N)) {
    LLVM_DEBUG(dbgs()<<"select [Base]+disp mode\n");
    // Base can be FrameIndex or SDValue
    // If the first operand is a FI, get the TargetFI Node
    if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(N.getOperand(0)))
      Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), VT);
    else
      Base = N.getOperand(0);
    ConstantSDNode *CN = dyn_cast<ConstantSDNode>(N.getOperand(1));
    Disp = CurDAG->getTargetConstant(CN->getSExtValue(), DL, VT);
    return true;
  }

  // default: Use Base
  LLVM_DEBUG(dbgs()<<"select [Base] mode (default)\n");
  Base = N;
  return true;
  
}

FunctionPass *createY86ISelDag(Y86TargetMachine &TM,
                               CodeGenOpt::Level OptLevel) {
  return new Y86DAGToDAGISel(TM, OptLevel);
}
