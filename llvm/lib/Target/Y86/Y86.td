include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Predicates & SubtargetFeature
//===----------------------------------------------------------------------===//

include "Y86RegisterInfo.td"
include "Y86Schedule.td"
include "Y86InstrFormats.td"
include "Y86InstrInfo.td"
include "Y86CallingConv.td"

def Y86InstrInfo : InstrInfo;

def : ProcessorModel<"Y86",GenericModel,[],[]>;

def Y86 : Target {
  let InstructionSet = Y86InstrInfo;
}

