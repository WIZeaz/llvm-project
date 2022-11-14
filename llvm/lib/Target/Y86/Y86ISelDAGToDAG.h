#include "MCTargetDesc/Y86MCTargetDesc.h"
#include "Y86.h"
#include "Y86InstrInfo.h"
#include "Y86RegisterInfo.h"
#include "Y86TargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicsX86.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/KnownBits.h"
#include "llvm/Support/MathExtras.h"
namespace llvm {

FunctionPass *createY86ISelDag(Y86TargetMachine&, CodeGenOpt::Level );

class Y86DAGToDAGISel : public SelectionDAGISel {
public:
  explicit Y86DAGToDAGISel(Y86TargetMachine &TM, CodeGenOpt::Level OL)
      : SelectionDAGISel(TM, OL), Subtarget(nullptr) {}
  // bool runOnMachineFunction(MachineFunction &MF) override;
  // Pass Name
  bool runOnMachineFunction(MachineFunction &MF) override {
    // Make sure we re-emit a set of the global base reg if necessary
    Subtarget = &MF.getSubtarget<Y86Subtarget>();
    TLI = Subtarget->getTargetLowering();
    SelectionDAGISel::runOnMachineFunction(MF);
    return true;
  }
  StringRef getPassName() const override {
    return "Y86 DAG->DAG Pattern Instruction Selection";
  }

  bool shouldAvoidImmediateInstFormsForSize(SDNode *N) const;

  // bool runOnMachineFunction(MachineFunction &MF) override;

protected:
  /// Keep a pointer to the Y86Subtarget around so that we can make the right
  /// decision when generating code for different targets.
  const Y86Subtarget *Subtarget;
  const Y86TargetLowering *TLI;
  bool EnablePromoteAnyextLoad = true;

private:
  void Select(SDNode *N) override;
  void selectFrameIndex(SDNode *SN, SDNode *N, unsigned Offset=0);
  bool selectAddr(SDNode *Parent, SDValue N, SDValue &Base, SDValue &Scale,
                  SDValue &Index, SDValue &Disp, SDValue &Segment);
#include "Y86GenDAGISel.inc"
};


} // namespace llvm