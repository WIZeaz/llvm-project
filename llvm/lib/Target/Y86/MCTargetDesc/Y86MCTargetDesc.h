//===-- Y86MCTargetDesc.h - Y86 Target Descriptions -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides Y86 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Y86_MCTARGETDESC_Y86MCTARGETDESC_H
#define LLVM_LIB_TARGET_Y86_MCTARGETDESC_Y86MCTARGETDESC_H

#include <memory>
#include <string>

namespace llvm {
class formatted_raw_ostream;
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInst;
class MCInstPrinter;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCObjectWriter;
class MCRegister;
class MCRegisterInfo;
class MCStreamer;
class MCSubtargetInfo;
class MCTargetOptions;
class MCTargetStreamer;
class Target;
class Triple;
class StringRef;

/// Flavour of dwarf regnumbers
///
namespace DWARFFlavour {
enum { X86_64 = 0, X86_32_DarwinEH = 1, X86_32_Generic = 2 };
}

///  Native Y86 register numbers
///
namespace N86 {
enum { EAX = 0, ECX = 1, EDX = 2, EBX = 3, ESP = 4, EBP = 5, ESI = 6, EDI = 7 };
}

namespace Y86_MC {
// std::string ParseY86Triple(const Triple &TT);

// unsigned getDwarfRegFlavour(const Triple &TT, bool isEH);

// void initLLVMToSEHAndCVRegMapping(MCRegisterInfo *MRI);

/// Returns true if this instruction has a LOCK prefix.
// bool hasLockPrefix(const MCInst &MI);

/// Create a Y86 MCSubtargetInfo instance. This is exposed so Asm parser, etc.
/// do not need to go through TargetRegistry.
MCSubtargetInfo *createY86MCSubtargetInfo(const Triple &TT, StringRef CPU,
                                          StringRef FS);
} // namespace Y86_MC



/// Construct an Y86 ELF object writer.
std::unique_ptr<MCObjectTargetWriter>
createY86ELFObjectWriter(bool IsELF64, uint8_t OSABI, uint16_t EMachine);

/// Construct an Y86 Win COFF object writer.
/* std::unique_ptr<MCObjectTargetWriter>
createY86WinCOFFObjectWriter(bool Is64Bit);
 */
/// Returns the sub or super register of a specific Y86 register.
/// e.g. getY86SubSuperRegister(Y86::EAX, 16) returns Y86::AX.
/// Aborts on error.
// MCRegister getY86SubSuperRegister(MCRegister, unsigned, bool High = false);

/// Returns the sub or super register of a specific Y86 register.
/// Like getY86SubSuperRegister() but returns 0 on error.
/* MCRegister getY86SubSuperRegisterOrZero(MCRegister, unsigned,
                                        bool High = false); */

} // namespace llvm

// Defines symbolic names for Y86 registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "Y86GenRegisterInfo.inc"

// Defines symbolic names for the Y86 instructions.
//
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "Y86GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "Y86GenSubtargetInfo.inc"

#endif
