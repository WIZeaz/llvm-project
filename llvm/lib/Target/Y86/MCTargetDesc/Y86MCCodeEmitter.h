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

#include "llvm/ADT/SmallVector.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"

namespace llvm {

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
  unsigned getRegEncoding(const MCOperand &MO) const;
  void emitMemModRMByte(const MCInst &MI, uint64_t TSFlags, uint8_t RegOpcode,
                        uint8_t OpNo, raw_ostream &OS) const;
  void emitRegMemBytes(const MCInst &MI, raw_ostream &OS, uint64_t TSFlags, uint8_t& CurOp) const;
};

} // namespace llvm

#endif