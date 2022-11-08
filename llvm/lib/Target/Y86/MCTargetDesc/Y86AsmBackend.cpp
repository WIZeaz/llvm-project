#include "MCTargetDesc/Y86AsmBackend.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCValue.h"
#include "llvm/MC/MCObjectWriter.h"
using namespace llvm;

static unsigned getFixupKindSize(unsigned Kind) {
  switch (Kind) {
  default:
    llvm_unreachable("invalid fixup kind!");
  case FK_NONE:
    return 0;
  case FK_PCRel_1:
  case FK_SecRel_1:
  case FK_Data_1:
    return 1;
  case FK_PCRel_2:
  case FK_SecRel_2:
  case FK_Data_2:
    return 2;
  case FK_PCRel_4:
  case Y86::reloc_riprel_4byte:
  case Y86::reloc_riprel_4byte_relax:
  case Y86::reloc_riprel_4byte_relax_rex:
  case Y86::reloc_riprel_4byte_movq_load:
  case Y86::reloc_signed_4byte:
  case Y86::reloc_signed_4byte_relax:
  case Y86::reloc_global_offset_table:
  case Y86::reloc_branch_4byte_pcrel:
  case FK_SecRel_4:
  case FK_Data_4:
    return 4;
  case FK_PCRel_8:
  case FK_SecRel_8:
  case FK_Data_8:
  case Y86::reloc_global_offset_table8:
    return 8;
  }
}

void Y86AsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                               const MCValue &Target,
                               MutableArrayRef<char> Data, uint64_t Value,
                               bool IsResolved,
                               const MCSubtargetInfo *STI) const {
  unsigned Kind = Fixup.getKind();
  if (Kind >= FirstLiteralRelocationKind)
    return;
  unsigned Size = getFixupKindSize(Kind);

  assert(Fixup.getOffset() + Size <= Data.size() && "Invalid fixup offset!");

  int64_t SignedValue = static_cast<int64_t>(Value);
  if ((Target.isAbsolute() || IsResolved) &&
      getFixupKindInfo(Fixup.getKind()).Flags & MCFixupKindInfo::FKF_IsPCRel) {
    // check that PC relative fixup fits into the fixup size.
    if (Size > 0 && !isIntN(Size * 8, SignedValue))
      Asm.getContext().reportError(
          Fixup.getLoc(), "value of " + Twine(SignedValue) +
                              " is too large for field of " + Twine(Size) +
                              ((Size == 1) ? " byte." : " bytes."));
  } else {
    // Check that uppper bits are either all zeros or all ones.
    // Specifically ignore overflow/underflow as long as the leakage is
    // limited to the lower bits. This is to remain compatible with
    // other assemblers.
    assert((Size == 0 || isIntN(Size * 8 + 1, SignedValue)) &&
           "Value does not fit in the Fixup field");
  }

  for (unsigned i = 0; i != Size; ++i)
    Data[Fixup.getOffset() + i] = uint8_t(Value >> (i * 8));
}

const MCFixupKindInfo &Y86AsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  const static MCFixupKindInfo Infos[Y86::NumTargetFixupKinds] = {
      {"reloc_riprel_4byte", 0, 32, MCFixupKindInfo::FKF_IsPCRel},
      {"reloc_riprel_4byte_movq_load", 0, 32, MCFixupKindInfo::FKF_IsPCRel},
      {"reloc_riprel_4byte_relax", 0, 32, MCFixupKindInfo::FKF_IsPCRel},
      {"reloc_riprel_4byte_relax_rex", 0, 32, MCFixupKindInfo::FKF_IsPCRel},
      {"reloc_signed_4byte", 0, 32, 0},
      {"reloc_signed_4byte_relax", 0, 32, 0},
      {"reloc_global_offset_table", 0, 32, 0},
      {"reloc_global_offset_table8", 0, 64, 0},
      {"reloc_branch_4byte_pcrel", 0, 32, MCFixupKindInfo::FKF_IsPCRel},
  };

  // Fixup kinds from .reloc directive are like R_386_NONE/R_Y86_64_NONE. They
  // do not require any extra processing.
  if (Kind >= FirstLiteralRelocationKind)
    return MCAsmBackend::getFixupKindInfo(FK_NONE);

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
         "Invalid kind!");
  assert(Infos[Kind - FirstTargetFixupKind].Name && "Empty fixup name!");
  return Infos[Kind - FirstTargetFixupKind];
}

bool Y86AsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                       const MCSubtargetInfo *STI) const {
  return true;
}


std::unique_ptr<MCObjectTargetWriter>
Y86AsmBackend::createObjectTargetWriter() const {
  return createY86ELFObjectWriter(/*IsELF64*/ false, OSABI, ELF::EM_X86_64);
}
