#include "Y86MCTargetDesc.h"
#include "MCTargetDesc/Y86AsmBackend.h"
#include "MCTargetDesc/Y86MCAsmInfo.h"
#include "MCTargetDesc/Y86MCCodeEmitter.h"
#include "MCTargetDesc/Y86TargetStreamer.h"
#include "TargetInfo/Y86TargetInfo.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/Triple.h"
#include "llvm/DebugInfo/CodeView/CodeView.h"
#include "llvm/MC/MCDwarf.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Host.h"

using namespace llvm;

#define GET_REGINFO_MC_DESC
#include "Y86GenRegisterInfo.inc"

#define GET_INSTRINFO_MC_DESC
#define GET_INSTRINFO_MC_HELPERS
#include "Y86GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "Y86GenSubtargetInfo.inc"

static MCInstrInfo *createY86MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitY86MCInstrInfo(X); // defined in Y86GenInstrInfo.inc
  return X;
}

static MCRegisterInfo *createY86MCRegisterInfo(const Triple &TT) {
  unsigned RA = Y86::EIP;

  MCRegisterInfo *X = new MCRegisterInfo();
  InitY86MCRegisterInfo(X, RA);
  //   Y86_MC::initLLVMToSEHAndCVRegMapping(X);
  return X;
}

static MCAsmInfo *createY86MCAsmInfo(const MCRegisterInfo &MRI,
                                     const Triple &TheTriple,
                                     const MCTargetOptions &Options) {
  bool is64Bit = true;

  MCAsmInfo *MAI;
  MAI = new Y86ELFMCAsmInfo(TheTriple);
  // Initialize initial frame state.
  // Calculate amount of bytes used for return address storing
  int stackGrowth = is64Bit ? -8 : -4;

  // Initial state of the frame pointer is esp+stackGrowth.
  unsigned StackPtr = /* is64Bit ? Y86::RSP : */ Y86::ESP;
  MCCFIInstruction Inst = MCCFIInstruction::cfiDefCfa(
      nullptr, MRI.getDwarfRegNum(StackPtr, true), -stackGrowth);
  MAI->addInitialFrameState(Inst);

  // Add return address to move list
  unsigned InstPtr = /* is64Bit ? Y86::RIP : */ Y86::EIP;
  MCCFIInstruction Inst2 = MCCFIInstruction::createOffset(
      nullptr, MRI.getDwarfRegNum(InstPtr, true), stackGrowth);
  MAI->addInitialFrameState(Inst2);

  return MAI;
}

MCTargetStreamer *createY86ObjectTargetStreamer(MCStreamer &S,
                                                const MCSubtargetInfo &STI) {
  return new Y86TargetStreamer(S);
}

static MCCodeEmitter *createY86MCCodeEmitter(const MCInstrInfo &MCII,
                                             const MCRegisterInfo &MRI,
                                             MCContext &Ctx) {
  return new Y86MCCodeEmitter(MCII, Ctx);
}

MCSubtargetInfo *Y86_MC::createY86MCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU, StringRef FS) {
  std::string ArchFS = "";
  // assert(!ArchFS.empty() && "Failed to parse X86 triple");
  if (!FS.empty())
    ArchFS = (Twine(ArchFS) + "," + FS).str();

  if (CPU.empty())
    CPU = "y86";

  return createY86MCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, ArchFS);
}

static MCStreamer *createMCStreamer(const Triple &TT, MCContext &Context,
                                    std::unique_ptr<MCAsmBackend> &&MAB,
                                    std::unique_ptr<MCObjectWriter> &&OW,
                                    std::unique_ptr<MCCodeEmitter> &&Emitter,
                                    bool RelaxAll) {
  return createELFStreamer(Context, std::move(MAB), std::move(OW),
                           std::move(Emitter), RelaxAll);
}

static MCAsmBackend *createY86AsmBackend(const Target &T,
                                               const MCSubtargetInfo &STI,
                                               const MCRegisterInfo &MRI,
                                               const MCTargetOptions &Options) {
  Triple TheTriple = STI.getTargetTriple();
  uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(TheTriple.getOS());
  return new Y86AsmBackend(T, OSABI, STI);
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeY86TargetMC() {
  Target *T = &getTheY86Target();

  // Register the MC asm info.
  RegisterMCAsmInfoFn X(*T, createY86MCAsmInfo);

  TargetRegistry::RegisterMCAsmBackend(*T, createY86AsmBackend);

  TargetRegistry::RegisterMCSubtargetInfo(*T, Y86_MC::createY86MCSubtargetInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(*T, createY86MCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(*T, createY86MCRegisterInfo);

  // Register the elf streamer.
  TargetRegistry::RegisterELFStreamer(*T, createMCStreamer);

  // Register the code emitter.
  TargetRegistry::RegisterMCCodeEmitter(*T, createY86MCCodeEmitter);
}
