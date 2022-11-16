//===-- X86MCAsmInfo.cpp - X86 asm properties -----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the X86MCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/Y86MCAsmInfo.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/CommandLine.h"
using namespace llvm;

void Y86ELFMCAsmInfo::anchor() {}

Y86ELFMCAsmInfo::Y86ELFMCAsmInfo(const Triple &T) {
  bool is64Bit = false;
  bool isX32 = false;

  // For ELF, x86-64 pointer size depends on the ABI.
  // For x86-64 without the x32 ABI, pointer size is 8. For x86 and for x86-64
  // with the x32 ABI, pointer size remains the default 4.
  CodePointerSize = (is64Bit && !isX32) ? 8 : 4;

  // OTOH, stack slot size is always 8 for x86-64, even with the x32 ABI.
  CalleeSaveStackSlotSize = is64Bit ? 8 : 4;

  // AssemblerDialect = AsmWriterFlavor;

  TextAlignFillValue = 0x90;

  // Debug Information
  SupportsDebugInformation = true;

  // Exceptions handling
  ExceptionsType = ExceptionHandling::DwarfCFI;
}