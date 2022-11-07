#include "Y86Framelowering.h"
#include "MCTargetDesc/Y86MCTargetDesc.h"
#include "Y86InstrInfo.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

void Y86FrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.begin();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  uint64_t StackSize = MFI.getStackSize();
  DebugLoc DL;
  if (hasFP(MF)) {
    BuildMI(MBB, MBBI, DL, TII.get(Y86::PUSH32r)).addReg(Y86::EBP);
    BuildMI(MBB, MBBI, DL, TII.get(Y86::Mov32rr))
        .addReg(Y86::ESP)
        .addReg(Y86::EBP);
  }
  BuildMI(MBB, MBBI,DL,TII.get(Y86::PUSH32r).addReg(Y86::ESI);
  BuildMI(MBB, MBBI,DL,TII.get(Y86::PUSH32r).addReg(Y86::EDI);
  llvm_unreachable("unimplemented");
}
void Y86FrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  llvm_unreachable("unimplemented");
}
bool Y86FrameLowering::hasFP(const MachineFunction &MF) const { return true; }