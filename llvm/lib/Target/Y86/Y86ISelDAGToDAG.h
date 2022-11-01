//===---- Y86ISelDAGToDAG.h - A Dag to Dag Inst Selector for Y86 --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the Y86 target.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Y86_Y86ISELDAGTODAG_H
#define LLVM_LIB_TARGET_Y86_Y86ISELDAGTODAG_H

#include "Y86Subtarget.h"
#include "Y86.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"

//===----------------------------------------------------------------------===//
// Instruction Selector Implementation
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Y86DAGToDAGISel - Y86 specific code to select Y86 machine
// instructions for SelectionDAG operations.
//===----------------------------------------------------------------------===//
namespace llvm {

class Y86DAGToDAGISel : public SelectionDAGISel {
public:
  explicit Y86DAGToDAGISel(Y86TargetMachine &TM, CodeGenOpt::Level OL)
      : SelectionDAGISel(TM, OL), Subtarget(nullptr) {}

  // Pass Name
  StringRef getPassName() const override {
    return "Y86 DAG->DAG Pattern Instruction Selection";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

protected:

  /// Keep a pointer to the Y86Subtarget around so that we can make the right
  /// decision when generating code for different targets.
  const Y86Subtarget *Subtarget;

private:

};

}

#endif
