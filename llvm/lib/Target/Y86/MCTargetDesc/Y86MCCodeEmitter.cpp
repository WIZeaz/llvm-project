#include "Y86MCCodeEmitter.h"

using namespace std;

void Y86MCCodeEmitter::emitPrefix(const MCInst &MI, raw_ostream &OS,
                                  const MCSubtargetInfo &STI) const {
  OS << 0xFF;
}

void Y86MCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
  OS << 0xFF;
}