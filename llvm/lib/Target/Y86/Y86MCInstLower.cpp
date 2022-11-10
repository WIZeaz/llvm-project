#include "Y86MCInstLower.h"
#include "Y86AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"
using namespace llvm;

Y86MCInstLower::Y86MCInstLower(Y86AsmPrinter &asmprinter)
    : AsmPrinter(asmprinter) {}

void Y86MCInstLower::Initialize(MCContext *C) { Ctx = C; }

void Y86MCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    if (auto MCOp = LowerOperand(MO))
      OutMI.addOperand(MCOp.getValue());
  }
}

Optional<MCOperand>
Y86MCInstLower::LowerOperand(const MachineOperand &MO) const {
  switch (MO.getType()) {
  default:
    llvm_unreachable("unknown operand type");
  case MachineOperand::MO_Register:
    if (MO.isImplicit())
      return None;
    return MCOperand::createReg(MO.getReg());
  case MachineOperand::MO_Immediate:
    return MCOperand::createImm(MO.getImm());
  case MachineOperand::MO_GlobalAddress:
  case MachineOperand::MO_ExternalSymbol:
  case MachineOperand::MO_MCSymbol:
  case MachineOperand::MO_JumpTableIndex:
  case MachineOperand::MO_ConstantPoolIndex:
  case MachineOperand::MO_BlockAddress:
  case MachineOperand::MO_RegisterMask:
    // Ignore call clobbers.
    return None;
  }
}

MCOperand Y86MCInstLower::LowerSymbolOperand(const MachineOperand &MO) const {
  llvm_unreachable("LowerSymbolOperand is not implemented!");
  return MCOperand();
}