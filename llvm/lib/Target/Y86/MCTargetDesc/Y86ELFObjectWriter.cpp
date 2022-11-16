//===-- Y86ELFObjectWriter.cpp - Y86 ELF Writer ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/Y86AsmBackend.h"
#include "MCTargetDesc/Y86FixupKinds.h"
#include "MCTargetDesc/Y86MCTargetDesc.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>
#include <cstdint>

using namespace llvm;

class Y86ELFObjectWriter : public MCELFObjectTargetWriter {
public:
  Y86ELFObjectWriter(bool IsELF64, uint8_t OSABI, uint16_t EMachine);
  ~Y86ELFObjectWriter() override = default;

protected:
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
};

Y86ELFObjectWriter::Y86ELFObjectWriter(bool IsELF64, uint8_t OSABI,
                                       uint16_t EMachine)
    : MCELFObjectTargetWriter(IsELF64, OSABI, EMachine,
                              // Only i386 and IAMCU use Rel instead of RelA.
                              /*HasRelocationAddend*/
                              (EMachine != ELF::EM_386) &&
                                  (EMachine != ELF::EM_IAMCU)) {}

unsigned Y86ELFObjectWriter::getRelocType(MCContext &Ctx, const MCValue &Target,
                                          const MCFixup &Fixup,
                                          bool IsPCRel) const {
  return 0;
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createY86ELFObjectWriter(bool IsELF64, uint8_t OSABI, uint16_t EMachine) {
  return std::make_unique<Y86ELFObjectWriter>(IsELF64, OSABI, EMachine);
}
