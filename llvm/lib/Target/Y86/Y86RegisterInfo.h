//===-- Y86RegisterInfo.h - Y86 Register Information Impl -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Y86 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Y86_Y86REGISTERINFO_H
#define LLVM_LIB_TARGET_Y86_Y86REGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "Y86GenRegisterInfo.inc"


namespace llvm {
class Y86Subtarget;
class TargetInstrInfo;
class Type;

class Y86RegisterInfo : public Y86GenRegisterInfo {
protected:
  const Y86Subtarget &Subtarget;

public:
  Y86RegisterInfo(const Y86Subtarget &Subtarget);

  const TargetRegisterClass *getPointerRegClass(const MachineFunction &MF,
                                                unsigned Kind) const override;

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  bool requiresRegisterScavenging(const MachineFunction &MF) const override;

  bool trackLivenessAfterRegAlloc(const MachineFunction &MF) const override;

  /// Stack Frame Processing Methods
  void eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  /// Debug information queries.
  Register getFrameRegister(const MachineFunction &MF) const override;

  Register getStackRegister() const { return StackPtr; }
  Register getBaseRegister() const { return BasePtr; }
  /// Returns physical register used as frame pointer.
  /// This will always returns the frame pointer register, contrary to
  /// getFrameRegister() which returns the "base pointer" in situations
  /// involving a stack, frame and base pointer.
  Register getFramePtr() const { return FramePtr; }
  unsigned getSlotSize() const { return SlotSize; }

private:
  unsigned SlotSize;

  /// StackPtr - Y86 physical register used as stack ptr.
  ///
  unsigned StackPtr;

  /// FramePtr - Y86 physical register used as frame ptr.
  ///
  unsigned FramePtr;
  /// BasePtr - Y86 physical register used as a base ptr in complex stack
  /// frames. I.e., when we need a 3rd base, not just SP and FP, due to
  /// variable size stack objects.
  unsigned BasePtr;
};

} // end namespace llvm

#endif
