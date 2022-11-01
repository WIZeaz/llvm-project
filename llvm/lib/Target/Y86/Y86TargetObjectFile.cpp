//===-- Y86TargetObjectFile.cpp - Y86 Object Info -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "Y86TargetObjectFile.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;
using namespace dwarf;

const MCExpr *Y86ELFTargetObjectFile::getDebugThreadLocalSymbol(
    const MCSymbol *Sym) const {
  return MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_DTPOFF, getContext());
}
