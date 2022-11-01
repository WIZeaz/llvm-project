#include "Y86Framelowering.h"

using namespace llvm;

void Y86FrameLowering::emitPrologue(MachineFunction &MF,
                                          MachineBasicBlock &MBB) const {

}
void Y86FrameLowering::emitEpilogue(MachineFunction &MF,
                                          MachineBasicBlock &MBB) const {
    
}
bool Y86FrameLowering::hasFP(const MachineFunction &MF) const {
    return true; 
}