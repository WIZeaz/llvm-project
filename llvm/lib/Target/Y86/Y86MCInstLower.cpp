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
    return LowerSymbolOperand(MO /* , GetSymbolFromOperand(MO) */);
  case MachineOperand::MO_MCSymbol:
    return LowerSymbolOperand(MO /* , MO.getMCSymbol() */);
  case MachineOperand::MO_JumpTableIndex:
    return LowerSymbolOperand(
        MO /* , AsmPrinter.GetJTISymbol(MO.getIndex()) */);
  case MachineOperand::MO_ConstantPoolIndex:
    return LowerSymbolOperand(
        MO /* , AsmPrinter.GetCPISymbol(MO.getIndex()) */);
  case MachineOperand::MO_BlockAddress:
    return LowerSymbolOperand(
        MO/* , this->AsmPrinter.GetBlockAddressSymbol(MO.getBlockAddress()) */);
  case MachineOperand::MO_RegisterMask:
    // Ignore call clobbers.
    return None;
  }
}

MCOperand Y86MCInstLower::LowerSymbolOperand(const MachineOperand &MO) const {
  llvm_unreachable("LowerSymbolOperand is not implemented!");
  // FIXME: We would like an efficient form for this, so we don't have to do a
  // lot of extra uniquing.
  const MCExpr *Expr = nullptr;
  MCSymbolRefExpr::VariantKind RefKind = MCSymbolRefExpr::VK_None;
  return MCOperand();
}