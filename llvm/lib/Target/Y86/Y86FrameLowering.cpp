#include "Y86FrameLowering.h"
#include "MCTargetDesc/Y86MCTargetDesc.h"
#include "Y86InstrInfo.h"
#include "Y86MachineFunctionInfo.h"
#include "Y86Subtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

Y86FrameLowering::Y86FrameLowering(const Y86Subtarget &sti, unsigned Alignment)
    : TargetFrameLowering(StackGrowsDown, Align(Alignment), 0,
                          Align(Alignment)),
      STI(sti), TII(*sti.getInstrInfo()), TRI(sti.getRegisterInfo()) {}

void Y86FrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.begin();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  uint64_t StackSize = MFI.getStackSize();
  DebugLoc DL;

  // BuildMI(MBB, MBBI, DL, TII.get(Y86::PUSH64r)).addReg(Y86::RSI);
  // BuildMI(MBB, MBBI, DL, TII.get(Y86::PUSH64r)).addReg(Y86::RDI);
  if (hasFP(MF)) {
    BuildMI(MBB, MBBI, DL, TII.get(Y86::PUSH64r)).addReg(Y86::RBP);
    BuildMI(MBB, MBBI, DL, TII.get(Y86::MOV64rr))
        .addReg(Y86::RBP)
        .addReg(Y86::RSP);
  }
  BuildMI(MBB, MBBI, DL, TII.get(Y86::SUB64ri),Y86::RSP)
      .addReg(Y86::RSP)
      .addImm(StackSize);
}
void Y86FrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  uint64_t StackSize = MFI.getStackSize();
  DebugLoc DL;

  BuildMI(MBB, MBBI, DL, TII.get(Y86::ADD64ri), Y86::RSP)
      .addReg(Y86::RSP)
      .addImm(StackSize);
  if (hasFP(MF)) {
    BuildMI(MBB, MBBI, DL, TII.get(Y86::POP64r)).addReg(Y86::RBP);
  }
  // BuildMI(MBB, MBBI, DL, TII.get(Y86::POP64r)).addReg(Y86::RDI);
  // BuildMI(MBB, MBBI, DL, TII.get(Y86::POP64r)).addReg(Y86::RSI);
}
bool Y86FrameLowering::hasFP(const MachineFunction &MF) const { return true; }

StackOffset Y86FrameLowering::getFrameIndexReference(const MachineFunction &MF,
                                                     int FI,
                                                     Register &FrameReg) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  unsigned SlotSize = TRI->getSlotSize();
  bool IsFixed = MFI.isFixedObjectIndex(FI);
  // We can't calculate offset from frame pointer if the stack is realigned,
  // so enforce usage of stack/base pointer.  The base pointer is used when we
  // have dynamic allocas in addition to dynamic realignment.

  FrameReg = TRI->getFrameRegister(MF);

  // Offset will hold the offset from the stack pointer at function entry to the
  // object.
  // We need to factor in additional offsets applied during the prologue to the
  // frame, base, and stack pointer depending on which is used.
  int Offset = MFI.getObjectOffset(FI) - getOffsetOfLocalArea();
  const Y86MachineFunctionInfo *Y86FI = MF.getInfo<Y86MachineFunctionInfo>();
  uint64_t StackSize = MFI.getStackSize();

  if (FrameReg == TRI->getFramePtr()) {
    // Skip saved EBP/RBP
    // Offset += SlotSize;

    // Skip the RETADDR move area
    int TailCallReturnAddrDelta = Y86FI->getTCReturnAddrDelta();
    if (TailCallReturnAddrDelta < 0)
      Offset -= TailCallReturnAddrDelta;

    return StackOffset::getFixed(Offset);
  }

  // FrameReg is either the stack pointer or a base pointer. But the base is
  // located at the end of the statically known StackSize so the distinction
  // doesn't really matter.
  // if (TRI->hasStackRealignment(MF) || TRI->hasBasePointer(MF))
  //  assert(isAligned(MFI.getObjectAlign(FI), -(Offset + StackSize)));
  return StackOffset::getFixed(Offset + StackSize);
}