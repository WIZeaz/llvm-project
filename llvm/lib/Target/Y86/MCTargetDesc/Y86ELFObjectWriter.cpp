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

/* enum Y86RelType { RT32_NONE, RT32_32, RT32_16, RT32_8 };

static Y86RelType getType32(Y86RelType T) {
  switch (T) {
  case RT64_NONE:
    return RT32_NONE;
  case RT64_64:
    llvm_unreachable("Unimplemented");
  case RT64_32:
  case RT64_32S:
    return RT32_32;
  case RT64_16:
    return RT32_16;
  case RT64_8:
    return RT32_8;
  }
  llvm_unreachable("unexpected relocation type!");
}

static unsigned getRelocType32(MCContext &Ctx,
                               MCSymbolRefExpr::VariantKind Modifier,
                               Y86RelType Type, bool IsPCRel,
                               MCFixupKind Kind) {
  switch (Modifier) {
  default:
    llvm_unreachable("Unimplemented");
  case MCSymbolRefExpr::VK_None:
  case MCSymbolRefExpr::VK_X86_ABS8:
    switch (Type) {
    case RT32_NONE:
      if (Modifier == MCSymbolRefExpr::VK_None)
        return ELF::R_386_NONE;
      llvm_unreachable("Unimplemented");
    case RT32_32:
      return IsPCRel ? ELF::R_386_PC32 : ELF::R_386_32;
    case RT32_16:
      return IsPCRel ? ELF::R_386_PC16 : ELF::R_386_16;
    case RT32_8:
      return IsPCRel ? ELF::R_386_PC8 : ELF::R_386_8;
    }
    llvm_unreachable("unexpected relocation type!");
  case MCSymbolRefExpr::VK_GOT:
    assert(Type == RT32_32);
    if (IsPCRel)
      return ELF::R_386_GOTPC;
    // Older versions of ld.bfd/ld.gold/lld do not support R_386_GOT32X and we
    // want to maintain compatibility.
    if (!Ctx.getAsmInfo()->canRelaxRelocations())
      return ELF::R_386_GOT32;

    return Kind == MCFixupKind(Y86::reloc_signed_4byte_relax)
               ? ELF::R_386_GOT32X
               : ELF::R_386_GOT32;
  case MCSymbolRefExpr::VK_GOTOFF:
    assert(Type == RT32_32);
    assert(!IsPCRel);
    return ELF::R_386_GOTOFF;
  case MCSymbolRefExpr::VK_TLSCALL:
    return ELF::R_386_TLS_DESC_CALL;
  case MCSymbolRefExpr::VK_TLSDESC:
    return ELF::R_386_TLS_GOTDESC;
  case MCSymbolRefExpr::VK_TPOFF:
    assert(Type == RT32_32);
    assert(!IsPCRel);
    return ELF::R_386_TLS_LE_32;
  case MCSymbolRefExpr::VK_DTPOFF:
    assert(Type == RT32_32);
    assert(!IsPCRel);
    return ELF::R_386_TLS_LDO_32;
  case MCSymbolRefExpr::VK_TLSGD:
    assert(Type == RT32_32);
    assert(!IsPCRel);
    return ELF::R_386_TLS_GD;
  case MCSymbolRefExpr::VK_GOTTPOFF:
    assert(Type == RT32_32);
    assert(!IsPCRel);
    return ELF::R_386_TLS_IE_32;
  case MCSymbolRefExpr::VK_PLT:
    assert(Type == RT32_32);
    return ELF::R_386_PLT32;
  case MCSymbolRefExpr::VK_INDNTPOFF:
    assert(Type == RT32_32);
    assert(!IsPCRel);
    return ELF::R_386_TLS_IE;
  case MCSymbolRefExpr::VK_NTPOFF:
    assert(Type == RT32_32);
    assert(!IsPCRel);
    return ELF::R_386_TLS_LE;
  case MCSymbolRefExpr::VK_GOTNTPOFF:
    assert(Type == RT32_32);
    assert(!IsPCRel);
    return ELF::R_386_TLS_GOTIE;
  case MCSymbolRefExpr::VK_TLSLDM:
    assert(Type == RT32_32);
    assert(!IsPCRel);
    return ELF::R_386_TLS_LDM;
  }
} */

unsigned Y86ELFObjectWriter::getRelocType(MCContext &Ctx, const MCValue &Target,
                                          const MCFixup &Fixup,
                                          bool IsPCRel) const {
  return 0;
  /* MCFixupKind Kind = Fixup.getKind();
  if (Kind >= FirstLiteralRelocationKind)
    return Kind - FirstLiteralRelocationKind;
  MCSymbolRefExpr::VariantKind Modifier = Target.getAccessVariant();
  assert((getEMachine() == ELF::EM_386 || getEMachine() == ELF::EM_IAMCU) &&
         "Unsupported ELF machine type.");
  return getRelocType32(Ctx, Modifier, getType32(Type), IsPCRel, Kind); */
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createY86ELFObjectWriter(bool IsELF64, uint8_t OSABI, uint16_t EMachine) {
  return std::make_unique<Y86ELFObjectWriter>(IsELF64, OSABI, EMachine);
}
