//===-- Y86AsmPrinter.h - Y86 implementation of AsmPrinter ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Y86_Y86ASMPRINTER_H
#define LLVM_LIB_TARGET_Y86_Y86ASMPRINTER_H

#include "Y86MCInstLower.h"
#include "Y86TargetMachine.h"
#include "llvm/CodeGen/AsmPrinter.h"

namespace llvm {

class MCStreamer;

class LLVM_LIBRARY_VISIBILITY Y86AsmPrinter : public AsmPrinter {

  const Y86Subtarget *Subtarget = nullptr;
  Y86MCInstLower MCInstLowering;

public:
  Y86AsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)), MCInstLowering(*this) {}

  StringRef getPassName() const override { return "Y86 Assembly Printer"; }

  const Y86Subtarget &getSubtarget() const { return *Subtarget; }

  void emitInstruction(const MachineInstr *MI) override;
  bool runOnMachineFunction(MachineFunction &MF) override;
  void emitFunctionBodyStart() override;
  void emitFunctionBodyEnd() override;
};

} // end namespace llvm

#endif
