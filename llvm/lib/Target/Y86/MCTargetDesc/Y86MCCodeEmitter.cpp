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

unsigned Y86MCCodeEmitter::getY86RegNum(const MCOperand &MO) const {
  return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg()) & 0x7;
}

unsigned Y86MCCodeEmitter::getY86RegEncoding(const MCInst &MI,
                                             unsigned OpNum) const {
  return Ctx.getRegisterInfo()->getEncodingValue(MI.getOperand(OpNum).getReg());
}
bool Y86MCCodeEmitter::isREXExtendedReg(const MCInst &MI,
                                        unsigned OpNum) const {
  return (getY86RegEncoding(MI, OpNum) >> 3) & 1;
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

// emit MemRM with Memory Operand
void Y86MCCodeEmitter::emitMemModRMByte(const MCInst &MI, uint64_t TSFlags,
                                        uint8_t RegOpcode, uint8_t OpNo,
                                        raw_ostream &OS) const {
  uint8_t Mod, RM;
  // Memory Operand = (Base, Scale, Index, Disp8/32, Segment)
  MCOperand Base, Scale, Index, Disp, Segment;
  getAddressOperands(MI, OpNo, Base, Scale, Index, Disp, Segment); // if Index is Y86::NoRegister, there is no SIB byte

  // Mod is 0,1,2 depend on Disp size
  assert(Disp.isImm() && "Disp must be Imm");
  
  llvm_unreachable("please implement it");
}

void Y86MCCodeEmitter::emitRegMemBytes(const MCInst &MI, uint64_t TSFlags,
                                       uint8_t &CurOp, raw_ostream &OS) const {
  uint64_t MRMFormBits = Y86II::getMRMFormat(TSFlags);
  uint64_t FormBits = Y86II::getFormat(TSFlags);
  uint8_t Mod;
  uint8_t RegOpcode;
  uint8_t RM;
  if (MRMFormBits == Y86II::NoMRM)
    return;

  llvm_unreachable("please implement it");
}

static void emitImmediate(MCInst MI, uint64_t ImmTyBits, uint8_t OpNo,
                          raw_ostream &OS) {
  if (ImmTyBits == Y86II::NoImm)
    return;
  int64_t Imm = MI.getOperand(OpNo).getImm();
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

static void emitREXByte(uint8_t REX_W, uint8_t REX_R, uint8_t REX_X,
                        uint8_t REX_B, raw_ostream &OS) {
  assert(REX_R < 2 && REX_X < 2 && REX_B < 2 && REX_W < 2);
  uint8_t REX = 0x40;
  REX_W <<= 3;
  REX_R <<= 2;
  REX_X <<= 1;
  REX = REX | REX_W | REX_R | REX_X | REX_B;
  emitByte(REX, OS);
}

void Y86MCCodeEmitter::emitREXPrefix(const MCInst &MI, uint64_t TSFlags,
                                     uint8_t CurOp, raw_ostream &OS) const {

  uint8_t REX_W = Y86II::hasREX_W(TSFlags);
  uint8_t REX_R = 0;
  uint8_t REX_X = 0;
  uint8_t REX_B = 0;
  uint64_t MRMFormBits = Y86II::getMRMFormat(TSFlags);
  uint64_t FormBits = Y86II::getFormat(TSFlags);

  if (MRMFormBits != Y86II::NoMRM) {

    if (MRMFormBits == Y86II::MRMrr) {
      REX_R = isREXExtendedReg(MI, CurOp++);
      REX_B = isREXExtendedReg(MI, CurOp++);
      if (FormBits == Y86II::Format::FormMR)
        std::swap(REX_R, REX_B);
    } else if (MRMFormBits == Y86II::MRMrm) {
      if (FormBits == Y86II::Format::FormMR) {
        // Order: MemOp, Reg
        REX_B = isREXExtendedReg(MI, CurOp);
        REX_X = isREXExtendedReg(MI, CurOp + 2);
        REX_R = isREXExtendedReg(MI, CurOp + 5);
      } else {
        // Order: Reg, MemOp
        REX_R = isREXExtendedReg(MI, CurOp);
        REX_B = isREXExtendedReg(MI, CurOp + 1);
        REX_X = isREXExtendedReg(MI, CurOp + 3);
      }

    } else {
      if (Y86II::hasMem(TSFlags)) {
        REX_B = isREXExtendedReg(MI, CurOp);
        REX_X = isREXExtendedReg(MI, CurOp + 2);
      } else {
        REX_B = isREXExtendedReg(MI, CurOp);
      }
    }
  }

  if (!(REX_W == 0 && REX_R == 0 && REX_X == 0 && REX_B == 0)) {
    emitREXByte(REX_W, REX_R, REX_X, REX_B, OS);
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

  LLVM_DEBUG(dbgs() << "Opcode = " << Opcode << "\nFormBits = " << FormBits
                    << "\nMRMFormBits = " << MRMFormBits
                    << "\nImmTyBits = " << ImmTyBits
                    << "\nHasREX_W = " << Y86II::hasREX_W(TSFlags) << "\n\n");

  if (Y86II::isBinOP(TSFlags))
    CurOp++;

  if (Y86II::isPseudo(TSFlags)) {
    return;
  }

  // 0. REX prefix
  emitREXPrefix(MI, TSFlags, CurOp, OS);

  // 1. Emit Opcode
  if (Y86II::shouldAddReg(TSFlags)) {
    Opcode += getY86RegEncoding(MI, CurOp++);
  }
  emitByte(Opcode, OS);

  // 2. Emit ModRM, SIB, Disp
  // TODO: Complete it
  emitRegMemBytes(MI, TSFlags, CurOp, OS);

  // 3. Emit Immediate
  emitImmediate(MI, ImmTyBits, CurOp, OS);
}
