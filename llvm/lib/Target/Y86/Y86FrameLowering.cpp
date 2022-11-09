#include "Y86Framelowering.h"
#include "MCTargetDesc/Y86MCTargetDesc.h"
#include "Y86InstrInfo.h"
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

  //BuildMI(MBB, MBBI, DL, TII.get(Y86::PUSH32r)).addReg(Y86::ESI);
  //BuildMI(MBB, MBBI, DL, TII.get(Y86::PUSH32r)).addReg(Y86::EDI);
  if (hasFP(MF)) {
    BuildMI(MBB, MBBI, DL, TII.get(Y86::PUSH32r)).addReg(Y86::EBP);
    BuildMI(MBB, MBBI, DL, TII.get(Y86::MOV64rr))
        .addReg(Y86::EBP)
        .addReg(Y86::ESP);
  }
  BuildMI(MBB, MBBI, DL, TII.get(Y86::SUB64ri))
      .addReg(Y86::ESP)
      .addImm(StackSize);
}
void Y86FrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  uint64_t StackSize = MFI.getStackSize();
  DebugLoc DL;

  BuildMI(MBB, MBBI, DL, TII.get(Y86::ADD64ri))
      .addReg(Y86::ESP)
      .addImm(StackSize);
  if (hasFP(MF)) {
    BuildMI(MBB, MBBI, DL, TII.get(Y86::POP32r)).addReg(Y86::EBP);
  }
  //BuildMI(MBB, MBBI, DL, TII.get(Y86::POP32r)).addReg(Y86::EDI);
  //BuildMI(MBB, MBBI, DL, TII.get(Y86::POP32r)).addReg(Y86::ESI);


}
bool Y86FrameLowering::hasFP(const MachineFunction &MF) const { return true; }