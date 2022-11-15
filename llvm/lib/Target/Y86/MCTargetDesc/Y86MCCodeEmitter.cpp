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

// Helper Functions
static inline void emitByte(uint8_t C, raw_ostream &OS) {
  OS << static_cast<char>(C);
}
static inline void emitWord(uint16_t C, raw_ostream &OS) {
  emitByte(C & 0xf, OS);
  emitByte((C >> 8) & 0xff, OS);
}
static inline void emitDoubleWord(uint32_t C, raw_ostream &OS) {
  emitByte(C & 0xff, OS);
  emitByte((C >> 8) & 0xff, OS);
  emitByte((C >> 16) & 0xff, OS);
  emitByte((C >> 24) & 0xff, OS);
}
static inline void emitQuadraWord(uint64_t C, raw_ostream &OS) {
  emitDoubleWord(C & 0xffffffff, OS);
  emitDoubleWord((C >> 32) & 0xffffffff, OS);
}

static uint8_t modRMByte(unsigned Mod, unsigned RegOpcode, unsigned RM) {
  assert(Mod < 4 && RegOpcode < 8 && RM < 8 && "ModRM Fields out of range!");
  return RM | (RegOpcode << 3) | (Mod << 6);
}

static inline void emitSIBByte(unsigned SS, unsigned Index, unsigned Base,
                               raw_ostream &OS) {
  // SIB byte is in the same format as the modRMByte.
  emitByte(modRMByte(SS, Index, Base), OS);
}

unsigned Y86MCCodeEmitter::getRegEncoding(const MCOperand &MO) const {
  return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg()) & 0x7;
}

bool Y86MCCodeEmitter::isREXExtendedReg(const MCInst &MI,
                                        unsigned OpNum) const {
  return (getRegEncoding(MI.getOperand(OpNum)) >> 3) & 1;
}

static void getAddressOperands(const MCInst &MI, uint8_t OpNo, MCOperand &Base,
                               MCOperand &Scale, MCOperand &Index,
                               MCOperand &Disp, MCOperand &Segment) {
  assert(OpNo + 5 <= MI.getNumOperands() && "not enough operand for address");
  Base = MI.getOperand(OpNo);
  Scale = MI.getOperand(OpNo + 1);
  Index = MI.getOperand(OpNo + 2);
  Disp = MI.getOperand(OpNo + 3);
  Segment = MI.getOperand(OpNo + 4);
}

void Y86MCCodeEmitter::emitMemModRMByte(const MCInst &MI, uint64_t TSFlags,
                                        uint8_t RegOpcode, uint8_t OpNo,
                                        raw_ostream &OS) const {
  uint8_t Mod, RM;
  // Memory Operand = (Base, Scale, Index, Disp8/32, Segment)
  MCOperand Base, Scale, Index, Disp, Segment;
  getAddressOperands(MI, OpNo, Base, Scale, Index, Disp, Segment);
  // Mod is 0,1,2 depend on Disp size
  assert(Disp.isImm() && "Disp must be Imm");

  int64_t DispImm = Disp.getImm();
  if (DispImm == 0)
    Mod = 0;
  else if (isInt<8>(DispImm))
    Mod = 1;
  else if (isInt<32>(DispImm))
    Mod = 2;
  else
    llvm_unreachable("Disp is too large");

  // assert(Base.isReg() && "Base must be Reg");
  if (Base.getReg() == Y86::NoRegister) {
    // Disp32 mode
    Mod = 0;
    RM = 5;
  }
  // Workaround: ModRM unsupport [EBP] address mode when Mod = 0
  if (Mod == 0 && Base.getReg() == Y86::EBP) {
    Mod = 1;
  }

  if (Index.getImm() != 0) { // SIB address mode
    RM = 4;
  } else {
    RM = getRegEncoding(Base);
  }

  emitByte(modRMByte(Mod, RegOpcode, RM), OS);

  if (RM == 4) {
    // emit SIB
    unsigned SS = Scale.getImm();
    assert(SS == 1 || SS == 2 || SS == 4 || SS == 8);

    unsigned IndexEncoding = getRegEncoding(Index);
    unsigned BaseEncoding = getRegEncoding(Base);
    emitSIBByte(SS, IndexEncoding, BaseEncoding, OS);
  }

  // emit Disp
  if ((Mod == 0 && RM == 5) || (Mod == 2)) {
    // emit Disp32
    emitDoubleWord(DispImm, OS);
  } else if (Mod == 1) {
    // emit Disp8
    emitByte(DispImm, OS);
  }
}

void Y86MCCodeEmitter::emitRegMemBytes(const MCInst &MI, raw_ostream &OS,
                                       uint64_t TSFlags, uint8_t &CurOp) const {
  uint64_t MRMFormBits = Y86II::getMRMFormat(TSFlags);
  uint64_t FormBits = Y86II::getFormat(TSFlags);
  uint8_t Mod;
  uint8_t RegOpcode;
  uint8_t RM;
  if (MRMFormBits == Y86II::NoMRM)
    return;

  if (MRMFormBits == Y86II::MRMrr) {
    // Mod must be 0b11
    Mod = 3;
    RegOpcode = getRegEncoding(MI.getOperand(CurOp++));
    RM = getRegEncoding(MI.getOperand(CurOp++));
    if (FormBits == Y86II::Format::FormMR)
      std::swap(RM, RegOpcode);
    emitByte(modRMByte(Mod, RegOpcode, RM), OS);
  } else if (MRMFormBits == Y86II::MRMrm) {
    // Segment always be zero
    MCOperand Reg;
    if (FormBits == Y86II::Format::FormMR) {
      // Order: MemOp, Reg
      Reg = MI.getOperand(CurOp + 5);
      RegOpcode = getRegEncoding(Reg);
      emitMemModRMByte(MI, TSFlags, RegOpcode, CurOp, OS);
    } else {
      // Order: Reg, MemOp
      Reg = MI.getOperand(CurOp);
      RegOpcode = getRegEncoding(Reg);
      emitMemModRMByte(MI, TSFlags, RegOpcode, CurOp + 1, OS);
    }
    CurOp += 6;

  } else {
    RegOpcode = Y86II::getExtOpcode(TSFlags);
    if (Y86II::hasMem(TSFlags)) {
      emitMemModRMByte(MI, TSFlags, RegOpcode, CurOp, OS);
      CurOp += 5;
    } else {
      Mod = 3;
      RM = getRegEncoding(MI.getOperand(CurOp++));
      emitByte(modRMByte(Mod, RegOpcode, RM), OS);
    }
  }
}

static void emitImmediate(int64_t Imm, uint64_t ImmTyBits, raw_ostream &OS) {
  if (ImmTyBits == Y86II::NoImm)
    return;

  switch (ImmTyBits) {
  case Y86II::Imm8:
    emitByte(Imm, OS);
    break;
  case Y86II::Imm16:
    emitWord(Imm, OS);
    break;
  case Y86II::Imm32:
    emitDoubleWord(Imm, OS);
    break;
  case Y86II::Imm64:
    emitQuadraWord(Imm, OS);
    break;
  default:
    llvm_unreachable("unimplemented imm type");
  }
}

void Y86MCCodeEmitter::checkREXMemBits(const MCInst &MI, uint64_t OpNo,
                                       uint8_t &REX_X, uint8_t &REX_B) {}

static void emitREXByte(uint8_t REX_W, uint8_t REX_R, uint8_t REX_X,
                        uint8_t REX_B, raw_ostream &OS) {
  assert(REX_R < 2 && REX_X < 2 && REX_B < 2 && REX_W < 2);
  uint8_t REX = 0x40;
  REX_W <<= 3;
  REX_R <<= 2;
  REX_X <<= 1;
  REX = REX | REX_W | REX_R | REX_X;
  emitByte(REX, OS);
}

void Y86MCCodeEmitter::emitREXPrefix(const MCInst &MI, uint64_t TSFlags,
                                     uint8_t CurOp, raw_ostream &OS) const {
  if (!Y86II::hasREX_W(TSFlags)) return;

  uint8_t REX_W = 1;
  uint8_t REX_R = 0;
  uint8_t REX_X = 0;
  uint8_t REX_B = 0;
  uint64_t MRMFormBits = Y86II::getMRMFormat(TSFlags);
  uint64_t FormBits = Y86II::getFormat(TSFlags);

  if (MRMFormBits != Y86II::NoMRM) {

    // assign REX_R
    if (MRMFormBits == Y86II::MRMrm || MRMFormBits == Y86II::MRMrr) {
      if (FormBits == Y86II::FormRI || FormBits == Y86II::FormRM)
        REX_R = isREXExtendedReg(MI, CurOp);
      else if (FormBits == Y86II::FormMR)
        REX_R = isREXExtendedReg(MI, CurOp + 5);
    }

    // check REX_X REX_B
    if (MRMFormBits == Y86II::MRMrm) {
      if (FormBits == Y86II::FormRM)
        CurOp += 1;
    }

    if (!Y86II::hasMem(TSFlags)) { // MRMFormBits is MRM*m or MRMrm
      checkREXMemBits(MI, CurOp, REX_X, REX_B);
    }
  }
  emitREXByte(REX_W, REX_R, REX_X, REX_B, OS);
}

void Y86MCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
  unsigned Opc = MI.getOpcode();
  const MCInstrDesc &Desc = MCII.get(Opc);
  uint64_t TSFlags = Desc.TSFlags;
  uint64_t FormBits = Y86II::getFormat(TSFlags);
  uint64_t MRMFormBits = Y86II::getMRMFormat(TSFlags);
  uint64_t ImmTyBits = Y86II::getImmType(TSFlags);
  uint64_t Opcode = Y86II::getOpcode(TSFlags);
  uint8_t CurOp = 0;

  LLVM_DEBUG(dbgs() << "Opcode = " << Opcode << "\nMRMFormBits = "
                    << MRMFormBits << "\nImmTyBits = " << ImmTyBits << "\n\n");

  if (Y86II::isBinOP(TSFlags))
    CurOp++;

  if (Y86II::isPseudo(TSFlags)) {
    return;
  }

  // 0. REX prefix
  emitREXPrefix(MI, TSFlags, CurOp, OS);

  // 1. Emit Opcode
  // TODO: Opcode of FormOr should add reg num
  if (Y86II::shouldAddReg(TSFlags)) {
    Opcode += getRegEncoding(MI.getOperand(CurOp++));
  }
  emitByte(Opcode, OS);

  // 2. Emit ModRM, SIB, Disp
  emitRegMemBytes(MI, OS, TSFlags, CurOp);

  // 3. Emit Immediate
  emitImmediate(MI.getOperand(CurOp++).getImm(), ImmTyBits, OS);
}
