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

static uint8_t modRMByte(unsigned Mod, unsigned RegOpcode, unsigned RM) {
  assert(Mod < 4 && RegOpcode < 8 && RM < 8 && "ModRM Fields out of range!");
  return RM | (RegOpcode << 3) | (Mod << 6);
}

void Y86MCCodeEmitter::emitPrefix(const MCInst &MI, raw_ostream &OS,
                                  const MCSubtargetInfo &STI) const {}

unsigned Y86MCCodeEmitter::getRegEncoding(const MCOperand &MO) const {
  return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg()) & 0x7;
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
    llvm_unreachable("not implement emit SIB");
  }

  // Emit Disp
  if ((Mod == 0 && RM == 5) || (Mod == 2)) {
    // emit Disp32
    emitDoubleWord(DispImm, OS);
  } else if (Mod == 1) {
    // emit Disp8
    emitByte(DispImm, OS);
  }
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

  std::cout << "Opcode = " << Opcode << "\tMRMFormBits = " << MRMFormBits
            << "\tImmTyBits = " << ImmTyBits << std::endl;

  if (Y86II::isPseudo(TSFlags)) {
    return;
  }

  // 0. REX prefix
  if (Y86II::hasREX_W(TSFlags)){
    emitByte(0x48, OS);
  }

  // 1. Emit Opcode
  // TODO: Opcode of FormOr,FormOrI should add reg num
  if (Y86II::shouldAddReg(TSFlags)) {
    Opcode += getRegEncoding(MI.getOperand(CurOp++));
  }
  emitByte(Opcode, OS);

  // 2. Emit ModRM, SIB, Disp
  if (MRMFormBits != Y86II::NoMRM) {
    uint8_t Mod;
    uint8_t RegOpcode;
    uint8_t RM;
    // when Opcode
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

  // 3. Emit Immediate
  if (ImmTyBits != Y86II::NoImm) {
    int64_t Imm = MI.getOperand(CurOp++).getImm();
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
    default:
      llvm_unreachable("unimplemented imm type");
    }
  }
}
