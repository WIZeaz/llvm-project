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

#include "Y86.h"
#include "Y86MachineFunctionInfo.h"
#include "Y86RegisterInfo.h"
#include "Y86Subtarget.h"

using namespace llvm;

#define DEBUG_TYPE "y86-isel"

namespace{


    
}