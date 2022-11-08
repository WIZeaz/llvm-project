#include "Y86MCCodeEmitter.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

static void emitByte(uint8_t C, raw_ostream &OS) { OS << static_cast<char>(C); }

void Y86MCCodeEmitter::emitPrefix(const MCInst &MI, raw_ostream &OS,
                                  const MCSubtargetInfo &STI) const {
  emitByte(0x2,OS);
}

void Y86MCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
  emitByte(0x2,OS);
}
