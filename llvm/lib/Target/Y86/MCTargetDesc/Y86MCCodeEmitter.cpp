#include "Y86MCCodeEmitter.h"
#include "MCTargetDesc/Y86MCTargetDesc.h"
#include "Y86BaseInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
using namespace llvm;

#define DEBUG_TYPE "y86-code-emitter"

static void emitByte(uint8_t C, raw_ostream &OS) { OS << static_cast<char>(C); }

void Y86MCCodeEmitter::emitPrefix(const MCInst &MI, raw_ostream &OS,
                                  const MCSubtargetInfo &STI) const {
  emitByte(0x2, OS);
}

void Y86MCCodeEmitter::emitOpcode(const MCInst &MI, raw_ostream &OS,
                                  const MCSubtargetInfo &STI) const {
  if (isFormat(TSFlags))
}

unsigned Y86MCCodeEmitter::getRegNum(const MCOperand &MO) const {
  return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg()) & 0x7;
}

void Y86MCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
  unsigned Opcode = MI.getOpcode();
  const MCInstrDesc &Desc = MCII.get(Opcode);
  uint64_t TSFlags = Desc.TSFlags;
  uint64_t FormBits = Y86II::getFormat(TSFlags);
  uint64_t MRMFormBits = Y86II::getMRMFormat(TSFlags);
  uint64_t ImmTyBits = Y86II::getImmType(TSFlags);
  uint64_t Opcode = Y86II::getOpcode(TSFlags);

  if (Y86II::isPseudo(TSFlags)) {
    return;
  }

  // 1. Emit Opcode
  // TODO: Opcode of FormOr,FormOrI should add reg num
  if (Y86II::shouldAddReg(TSFlags)) {
    Opcode += getRegNum(MI.getOperand(0));
  }
  emitByte(Opcode);

  // 2. Emit ModRM, SIB, Disp
  if (MRMFormBits != Y86II::NoMRM) {
    uint8_t Mod;
    uint8_t RegOpcode;
    uint8_t RM;
    // when Opcode
    if (MRMFormat == Y86II::MRMrr) {
      // Mod must be 0b11
      Mod = 3;
      RegOpcode = getRegNum(MI.getOperand(0));
      RM = getRegNum(MI.getOperand(1));
      if (FormBits == Y86II::Format::FormMR)
        swap(RM, RegOpcode);
    } else if (MRMFormat == Y86II::MRMrm) {
      
    } else {
      RegOpcode = Y86II::getExtOpcode(TSFlags);
    }
  }

  // 3. Emit Immediate
}
