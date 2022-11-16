//===- Y86MCInstLower.h - Lower MachineInstr to MCInst --------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Y86_Y86MCINSTLOWER_H
#define LLVM_LIB_TARGET_Y86_Y86MCINSTLOWER_H

#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCContext.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
class Y86AsmPrinter;
/// Y86MCInstLower - This class is used to lower an MachineInstr into an MCInst.
class LLVM_LIBRARY_VISIBILITY Y86MCInstLower {
  MCContext *Ctx;
  Y86AsmPrinter &AsmPrinter;

public:
  Y86MCInstLower(Y86AsmPrinter &asmprinter);
  void Initialize(MCContext *C);
  void Lower(const MachineInstr *MI, MCInst &OutMI) const;
  Optional<MCOperand> LowerOperand(const MachineOperand &MO) const;
  MCOperand LowerSymbolOperand(const MachineOperand &MO) const;
};
} // namespace llvm

#endif
