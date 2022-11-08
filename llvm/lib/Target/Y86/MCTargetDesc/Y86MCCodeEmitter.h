//===-- Y86MCCodeEmitter.h - Convert Y86 Code to Machine Code -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Y86MCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_LIB_TARGET_Y86_MCTARGETDESC_Y86MCCODEEMITTER_H
#define LLVM_LIB_TARGET_Y86_MCTARGETDESC_Y86MCCODEEMITTER_H

#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {

class MCInstrInfo;
class MCContext;
class Y86MCCodeEmitter : public MCCodeEmitter {
  const MCInstrInfo &MCII;
  MCContext &Ctx;

public:
  Y86MCCodeEmitter(const MCInstrInfo &mcii, MCContext &ctx)
      : MCII(mcii), Ctx(ctx) {}
  Y86MCCodeEmitter(const Y86MCCodeEmitter &) = delete;
  Y86MCCodeEmitter &operator=(const Y86MCCodeEmitter &) = delete;
  ~Y86MCCodeEmitter() override = default;

  void emitPrefix(const MCInst &MI, raw_ostream &OS,
                  const MCSubtargetInfo &STI) const override;



  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

private:
  void emitOpcode(const MCInst &MI, raw_ostream &OS,
                                    const MCSubtargetInfo &STI) const;
  unsigned getRegNum(const MCOperand &MO) const
  /* unsigned getY86RegNum(const MCOperand &MO) const;

  unsigned getY86RegEncoding(const MCInst &MI, unsigned OpNum) const;

  /// \param MI a single low-level machine instruction.
  /// \param OpNum the operand #.
  /// \returns true if the OpNumth operand of MI  require a bit to be set in
  /// REX prefix.
  bool isREXExtendedReg(const MCInst &MI, unsigned OpNum) const;

  void emitImmediate(const MCOperand &Disp, SMLoc Loc, unsigned ImmSize,
                     MCFixupKind FixupKind, uint64_t StartByte, raw_ostream &OS,
                     SmallVectorImpl<MCFixup> &Fixups, int ImmOffset = 0) const;

  void emitRegModRMByte(const MCOperand &ModRMReg, unsigned RegOpcodeFld,
                        raw_ostream &OS) const;

  void emitSIBByte(unsigned SS, unsigned Index, unsigned Base,
                   raw_ostream &OS) const;

  void emitMemModRMByte(const MCInst &MI, unsigned Op, unsigned RegOpcodeField,
                        uint64_t TSFlags, bool HasREX, uint64_t StartByte,
                        raw_ostream &OS, SmallVectorImpl<MCFixup> &Fixups,
                        const MCSubtargetInfo &STI,
                        bool ForceSIB = false) const;

  bool emitPrefixImpl(unsigned &CurOp, const MCInst &MI,
                      const MCSubtargetInfo &STI, raw_ostream &OS) const;

  void emitVEXOpcodePrefix(int MemOperand, const MCInst &MI,
                           raw_ostream &OS) const;

  void emitSegmentOverridePrefix(unsigned SegOperand, const MCInst &MI,
                                 raw_ostream &OS) const;

  bool emitOpcodePrefix(int MemOperand, const MCInst &MI,
                        const MCSubtargetInfo &STI, raw_ostream &OS) const;

  bool emitREXPrefix(int MemOperand, const MCInst &MI,
                     const MCSubtargetInfo &STI, raw_ostream &OS) const; */
};

} // namespace llvm

#endif