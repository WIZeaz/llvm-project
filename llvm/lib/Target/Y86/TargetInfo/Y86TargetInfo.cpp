//===-- Cpu0TargetInfo.cpp - Cpu0 Target Implementation -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Y86TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target& llvm::getTheY86Target() {
  static Target TheY86Target;
  return TheY86Target;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeY86TargetInfo() {
  RegisterTarget<Triple::y86> X(getTheY86Target(), "y86", "Y86", "Y86");
}
