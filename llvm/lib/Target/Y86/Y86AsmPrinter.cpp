//===-- Y86AsmPrinter.cpp - Convert Y86 LLVM code to AT&T assembly --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to Y86 machine code.
//
//===----------------------------------------------------------------------===//

#include "Y86AsmPrinter.h"
#include "MCTargetDesc/Y86TargetStreamer.h"
#include "TargetInfo/Y86TargetInfo.h"
#include "Y86InstrInfo.h"
#include "Y86MachineFunctionInfo.h"
#include "Y86Subtarget.h"
#include "llvm/BinaryFormat/COFF.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineModuleInfoImpls.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSectionCOFF.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MachineValueType.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

//===----------------------------------------------------------------------===//
// Primitive Helper Functions.
//===----------------------------------------------------------------------===//

/// runOnMachineFunction - Emit the function body.
///
bool Y86AsmPrinter::runOnMachineFunction(MachineFunction &MF) {
  Subtarget = &MF.getSubtarget<Y86Subtarget>();
  /*   EmitFPOData =
        Subtarget->isTargetWin32() &&
     MF.getMMI().getModule()->getCodeViewFlag();
   */
  SetupMachineFunction(MF);
  emitFunctionBody();

  /*   if (Subtarget->isTargetCOFF()) {
      bool Local = MF.getFunction().hasLocalLinkage();
      OutStreamer->BeginCOFFSymbolDef(CurrentFnSym);
      OutStreamer->EmitCOFFSymbolStorageClass(
          Local ? COFF::IMAGE_SYM_CLASS_STATIC :
    COFF::IMAGE_SYM_CLASS_EXTERNAL);
      OutStreamer->EmitCOFFSymbolType(COFF::IMAGE_SYM_DTYPE_FUNCTION
                                                 <<
    COFF::SCT_COMPLEX_TYPE_SHIFT); OutStreamer->EndCOFFSymbolDef();
    } */

  // Emit the rest of the function body.

  // We didn't modify anything.
  return false;
}

void Y86AsmPrinter::emitFunctionBodyStart() {
    MCInstLowering.Initialize(&MF->getContext());
  /*   if (EmitFPOData) {
      if (auto *XTS =
          static_cast<Y86TargetStreamer *>(OutStreamer->getTargetStreamer()))
        XTS->emitFPOProc(
            CurrentFnSym,
            MF->getInfo<Y86MachineFunctionInfo>()->getArgumentStackSize());
    } */
}

void Y86AsmPrinter::emitFunctionBodyEnd() {
  /*   if (EmitFPOData) {
      if (auto *XTS =
              static_cast<Y86TargetStreamer
    *>(OutStreamer->getTargetStreamer())) XTS->emitFPOEndProc();
    } */
}

void Y86AsmPrinter::emitInstruction(const MachineInstr *MI) {
  MachineBasicBlock::const_instr_iterator I = MI->getIterator();
  MachineBasicBlock::const_instr_iterator E = MI->getParent()->instr_end();

  // print whole bundle
  do {
    MCInst TmpInst0;
    MCInstLowering.Lower(&*I, TmpInst0);
    OutStreamer->emitInstruction(TmpInst0, getSubtargetInfo());
  } while ((++I != E) && I->isInsideBundle());
}
//===----------------------------------------------------------------------===//
// Target Registry Stuff
//===----------------------------------------------------------------------===//

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeY86AsmPrinter() {
  RegisterAsmPrinter<Y86AsmPrinter> X(getTheY86Target());
}