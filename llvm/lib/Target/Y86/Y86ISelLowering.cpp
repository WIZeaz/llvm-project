//===-- Y86ISelLowering.cpp - Y86 DAG Lowering Implementation -------------===//
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

#include "Y86ISelLowering.h"
#include "Y86.h"
// #include "MCTargetDesc/Y86ShuffleDecode.h"
// #include "Y86CallingConv.h"
#include "Y86FrameLowering.h"
#include "Y86RegisterInfo.h"
// #include "Y86InstrBuilder.h"
// #include "Y86IntrinsicsInfo.h"
// #include "Y86MachineFunctionInfo.h"
#include "Y86TargetMachine.h"
#include "Y86TargetObjectFile.h"

using namespace llvm;

#define DEBUG_TYPE "y86-isel"



llvm::Y86TargetLowering::Y86TargetLowering(const Y86TargetMachine &TM,
                                           const Y86Subtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {
  //addRegisterClass(MVT::i32, GR32RegClass);

}

SDValue Y86TargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
    return SDValue();
}

const char* Y86TargetLowering::getTargetNodeName(unsigned Opcode) const {
    return "Y86ISD::unknown";
}