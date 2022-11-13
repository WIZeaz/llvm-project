//===-- Y86TargetMachine.cpp - Define TargetMachine for Y86 -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about Y86 target spec.
//
//===----------------------------------------------------------------------===//

#include "Y86ISelDAGToDAG.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Target/TargetOptions.h"

#include "TargetInfo/Y86TargetInfo.h"
#include "Y86.h"
#include "Y86TargetMachine.h"
#include "Y86TargetObjectFile.h"

using namespace llvm;

#define DEBUG_TYPE "Y86"

extern "C" void LLVMInitializeY86Target() {
  RegisterTargetMachine<Y86TargetMachine> X(getTheY86Target());
}

static std::string computeDataLayout(const Triple &TT, StringRef CPU,
                                     const TargetOptions &Options) {
  std::string Ret = "";

  // Y86 is little endian
  Ret += "e";

  Ret += "-m:m";

  Ret += "-p:64:64";

  // 8 and 16 bit integers only need to have natural alignment, but try to
  // align them to 32 bits. 64 bit integers have natural alignment.
  Ret += "-i8:8:32-i16:16:32-i64:64";

  // 32 bit registers are always available and the stack is at least 64 bit
  // aligned.
  Ret += "-n32-S64";

  return Ret;
}

static Reloc::Model getEffectiveRelocModel(bool JIT,
                                           Optional<Reloc::Model> RM) {
  if (!RM.hasValue() || JIT)
    return Reloc::Static;
  return *RM;
}

// DataLayout --> Big-endian, 32-bit pointer/ABI/alignment
// The stack is always 8 byte aligned
// On function prologue, the stack is created by decrementing
// its pointer. Once decremented, all references are done with positive
// offset from the stack/frame pointer, using StackGrowsUp enables
// an easier handling.
// Using CodeModel::Large enables different CALL behavior.
Y86TargetMachine::Y86TargetMachine(const Target &T, const Triple &TT,
                                   StringRef CPU, StringRef FS,
                                   const TargetOptions &Options,
                                   Optional<Reloc::Model> RM,
                                   Optional<CodeModel::Model> CM,
                                   CodeGenOpt::Level OL, bool JIT)
    //- Default is big endian
    : LLVMTargetMachine(T, computeDataLayout(TT, CPU, Options), TT, CPU, FS,
                        Options, getEffectiveRelocModel(JIT, RM),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<Y86ELFTargetObjectFile>()),
      DefaultSubtarget(TT, CPU, CPU, FS, *this) {

  initAsmInfo();
}

Y86TargetMachine::~Y86TargetMachine() {}

const Y86Subtarget *
Y86TargetMachine::getSubtargetImpl(const Function &F) const {
  std::string CPU = TargetCPU;
  std::string FS = TargetFS;

  auto &I = SubtargetMap[CPU + FS];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = std::make_unique<Y86Subtarget>(TargetTriple, CPU, CPU, FS, *this);
  }
  return I.get();
}

namespace {
//@Y86PassConfig {
/// Y86 Code Generator Pass Configuration Options.
class Y86PassConfig : public TargetPassConfig {
public:
  Y86PassConfig(Y86TargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  Y86TargetMachine &getY86TargetMachine() const {
    return getTM<Y86TargetMachine>();
  }

  const Y86Subtarget &getY86Subtarget() const {
    return *getY86TargetMachine().getSubtargetImpl();
  }

  bool addInstSelector() override;

  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *Y86TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new Y86PassConfig(*this, PM);
}

/* void Y86PassConfig::addIRPasses() {
  TargetPassConfig::addIRPasses();
  // addPass(createAtomicExpandPass());
} */

// Install an instruction selector pass using
// the ISelDag to gen Y86 code.
bool Y86PassConfig::addInstSelector() {
  addPass(new Y86DAGToDAGISel(getY86TargetMachine(), getOptLevel()));
  return false;
}

#ifdef ENABLE_GPRESTORE
void Y86PassConfig::addPreRegAlloc() {
  if (!Y86ReserveGP) {
    // $gp is a caller-saved register.
    addPass(createY86EmitGPRestorePass(getY86TargetMachine()));
  }
  return;
}
#endif

// Implemented by targets that want to run passes immediately before
// machine code is emitted. return true if -print-machineinstrs should
// print out the code after the passes.
void Y86PassConfig::addPreEmitPass() {
  // Y86TargetMachine &TM = getY86TargetMachine();
  return;
}
