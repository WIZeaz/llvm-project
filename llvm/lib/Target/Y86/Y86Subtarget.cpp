//===-- Y86Subtarget.cpp - Y86 Subtarget Information --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Y86 specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "Y86Subtarget.h"
#include "Y86InstrInfo.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define DEBUG_TYPE "y86-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "Y86GenSubtargetInfo.inc"

void Y86Subtarget::anchor() {}

Y86Subtarget::Y86Subtarget(const Triple &TT, StringRef CPU, StringRef TuneCPU,
                           StringRef FS, const Y86TargetMachine &TM)
    : Y86GenSubtargetInfo(TT, CPU, TuneCPU, FS),
      PICStyle(PICStyles::Style::None), TM(TM), TargetTriple(TT), TSInfo(),
      InstrInfo(initializeSubtargetDependencies(CPU, TuneCPU, FS)),
      TLInfo(TM, *this), FrameLowering(*this, getStackAlignment()) {

}


Y86Subtarget &Y86Subtarget::initializeSubtargetDependencies(StringRef CPU,
                                                            StringRef TuneCPU,
                                                            StringRef FS) {

  // Parse features string.
  // ParseSubtargetFeatures(CPU, /*TuneCPU*/ CPU, FS);
  // Initialize scheduling itinerary for the specified CPU.
  // InstrItins = getInstrItineraryForCPU(CPU);
  return *this;
}