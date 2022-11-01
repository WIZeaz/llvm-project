//===-- Y86Schedule.td - Y86 Scheduling Definitions --------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// InstrSchedModel annotations for out-of-order CPUs.

// Instructions with folded loads need to read the memory operand immediately,
// but other register operands don't have to be read until the load is ready.
// These operands are marked with ReadAfterLd.
def ReadAfterLd : SchedRead;
def ReadAfterVecLd : SchedRead;
def ReadAfterVecXLd : SchedRead;
def ReadAfterVecYLd : SchedRead;

// Instructions that move data between general purpose registers and vector
// registers may be subject to extra latency due to data bypass delays.
// This SchedRead describes a bypass delay caused by data being moved from the
// integer unit to the floating point unit.
def ReadInt2Fpu : SchedRead;

// Instructions with both a load and a store folded are modeled as a folded
// load + WriteRMW.
def WriteRMW : SchedWrite;

// Helper to set SchedWrite ExePorts/Latency/ResourceCycles/NumMicroOps.
multiclass Y86WriteRes<SchedWrite SchedRW,
                       list<ProcResourceKind> ExePorts,
                       int Lat, list<int> Res, int UOps> {
  def : WriteRes<SchedRW, ExePorts> {
    let Latency = Lat;
    let ResourceCycles = Res;
    let NumMicroOps = UOps;
  }
}

// Most instructions can fold loads, so almost every SchedWrite comes in two
// variants: With and without a folded load.
// An Y86FoldableSchedWrite holds a reference to the corresponding SchedWrite
// with a folded load.
class Y86FoldableSchedWrite : SchedWrite {
  // The SchedWrite to use when a load is folded into the instruction.
  SchedWrite Folded;
  // The SchedRead to tag register operands than don't need to be ready
  // until the folded load has completed.
  SchedRead ReadAfterFold;
}

// Multiclass that produces a linked pair of SchedWrites.
multiclass Y86SchedWritePair<SchedRead ReadAfter = ReadAfterLd> {
  // Register-Memory operation.
  def Ld : SchedWrite;
  // Register-Register operation.
  def NAME : Y86FoldableSchedWrite {
    let Folded = !cast<SchedWrite>(NAME#"Ld");
    let ReadAfterFold = ReadAfter;
  }
}

// Helpers to mark SchedWrites as unsupported.
multiclass Y86WriteResUnsupported<SchedWrite SchedRW> {
  let Unsupported = 1 in {
    def : WriteRes<SchedRW, []>;
  }
}
multiclass Y86WriteResPairUnsupported<Y86FoldableSchedWrite SchedRW> {
  let Unsupported = 1 in {
    def : WriteRes<SchedRW, []>;
    def : WriteRes<SchedRW.Folded, []>;
  }
}

// Multiclass that wraps Y86FoldableSchedWrite for each vector width.
class Y86SchedWriteWidths<Y86FoldableSchedWrite sScl,
                          Y86FoldableSchedWrite s128,
                          Y86FoldableSchedWrite s256,
                          Y86FoldableSchedWrite s512> {
  Y86FoldableSchedWrite Scl = sScl; // Scalar float/double operations.
  Y86FoldableSchedWrite MMX = sScl; // MMX operations.
  Y86FoldableSchedWrite XMM = s128; // XMM operations.
  Y86FoldableSchedWrite YMM = s256; // YMM operations.
  Y86FoldableSchedWrite ZMM = s512; // ZMM operations.
}

// Multiclass that wraps Y86SchedWriteWidths for each fp vector type.
class Y86SchedWriteSizes<Y86SchedWriteWidths sPH,
                         Y86SchedWriteWidths sPS,
                         Y86SchedWriteWidths sPD> {
  Y86SchedWriteWidths PH = sPH;
  Y86SchedWriteWidths PS = sPS;
  Y86SchedWriteWidths PD = sPD;
}

// Multiclass that wraps move/load/store triple for a vector width.
class Y86SchedWriteMoveLS<SchedWrite MoveRR,
                          SchedWrite LoadRM,
                          SchedWrite StoreMR> {
  SchedWrite RR = MoveRR;
  SchedWrite RM = LoadRM;
  SchedWrite MR = StoreMR;
}

// Multiclass that wraps masked load/store writes for a vector width.
class Y86SchedWriteMaskMove<SchedWrite LoadRM, SchedWrite StoreMR> {
  SchedWrite RM = LoadRM;
  SchedWrite MR = StoreMR;
}

// Multiclass that wraps Y86SchedWriteMoveLS for each vector width.
class Y86SchedWriteMoveLSWidths<Y86SchedWriteMoveLS sScl,
                                Y86SchedWriteMoveLS s128,
                                Y86SchedWriteMoveLS s256,
                                Y86SchedWriteMoveLS s512> {
  Y86SchedWriteMoveLS Scl = sScl; // Scalar float/double operations.
  Y86SchedWriteMoveLS MMX = sScl; // MMX operations.
  Y86SchedWriteMoveLS XMM = s128; // XMM operations.
  Y86SchedWriteMoveLS YMM = s256; // YMM operations.
  Y86SchedWriteMoveLS ZMM = s512; // ZMM operations.
}

// Loads, stores, and moves, not folded with other operations.
def WriteLoad    : SchedWrite;
def WriteStore   : SchedWrite;
def WriteStoreNT : SchedWrite;
def WriteMove    : SchedWrite;
def WriteVecMaskedGatherWriteback : SchedWrite;
def WriteCopy    : WriteSequence<[WriteLoad, WriteStore]>; // mem->mem copy

// Arithmetic.
defm WriteALU    : Y86SchedWritePair; // Simple integer ALU op.
defm WriteADC    : Y86SchedWritePair; // Integer ALU + flags op.
def  WriteALURMW : WriteSequence<[WriteALULd, WriteRMW]>;
def  WriteADCRMW : WriteSequence<[WriteADCLd, WriteRMW]>;
def  WriteLEA    : SchedWrite;        // LEA instructions can't fold loads.

// Integer multiplication
defm WriteIMul8     : Y86SchedWritePair; // Integer 8-bit multiplication.
defm WriteIMul16    : Y86SchedWritePair; // Integer 16-bit multiplication.
defm WriteIMul16Imm : Y86SchedWritePair; // Integer 16-bit multiplication by immediate.
defm WriteIMul16Reg : Y86SchedWritePair; // Integer 16-bit multiplication by register.
defm WriteIMul32    : Y86SchedWritePair; // Integer 32-bit multiplication.
defm WriteIMul32Imm : Y86SchedWritePair; // Integer 32-bit multiplication by immediate.
defm WriteIMul32Reg : Y86SchedWritePair; // Integer 32-bit multiplication by register.
defm WriteIMul64    : Y86SchedWritePair; // Integer 64-bit multiplication.
defm WriteIMul64Imm : Y86SchedWritePair; // Integer 64-bit multiplication by immediate.
defm WriteIMul64Reg : Y86SchedWritePair; // Integer 64-bit multiplication by register.
defm WriteMULX32    : Y86SchedWritePair; // Integer 32-bit Multiplication without affecting flags.
defm WriteMULX64    : Y86SchedWritePair; // Integer 64-bit Multiplication without affecting flags.
def  WriteIMulH     : SchedWrite;        // Integer multiplication, high part (only used by the RR variant of MULX).
def  WriteIMulHLd   : SchedWrite;        // Integer multiplication, high part (only used by the RM variant of MULX).

def  WriteBSWAP32 : SchedWrite; // Byte Order (Endianness) 32-bit Swap.
def  WriteBSWAP64 : SchedWrite; // Byte Order (Endianness) 64-bit Swap.
defm WriteCMPXCHG : Y86SchedWritePair; // Compare and set, compare and swap.
def  WriteCMPXCHGRMW : SchedWrite;     // Compare and set, compare and swap.
def  WriteXCHG    : SchedWrite;        // Compare+Exchange - TODO RMW support.

// Integer division.
defm WriteDiv8   : Y86SchedWritePair;
defm WriteDiv16  : Y86SchedWritePair;
defm WriteDiv32  : Y86SchedWritePair;
defm WriteDiv64  : Y86SchedWritePair;
defm WriteIDiv8  : Y86SchedWritePair;
defm WriteIDiv16 : Y86SchedWritePair;
defm WriteIDiv32 : Y86SchedWritePair;
defm WriteIDiv64 : Y86SchedWritePair;

defm WriteBSF : Y86SchedWritePair; // Bit scan forward.
defm WriteBSR : Y86SchedWritePair; // Bit scan reverse.
defm WritePOPCNT : Y86SchedWritePair; // Bit population count.
defm WriteLZCNT : Y86SchedWritePair; // Leading zero count.
defm WriteTZCNT : Y86SchedWritePair; // Trailing zero count.
defm WriteCMOV  : Y86SchedWritePair; // Conditional move.
def  WriteFCMOV : SchedWrite; // X87 conditional move.
def  WriteSETCC : SchedWrite; // Set register based on condition code.
def  WriteSETCCStore : SchedWrite;
def  WriteLAHFSAHF : SchedWrite; // Load/Store flags in AH.

def  WriteBitTest      : SchedWrite; // Bit Test
def  WriteBitTestImmLd : SchedWrite;
def  WriteBitTestRegLd : SchedWrite;

def  WriteBitTestSet       : SchedWrite; // Bit Test + Set
def  WriteBitTestSetImmLd  : SchedWrite;
def  WriteBitTestSetRegLd  : SchedWrite;
def  WriteBitTestSetImmRMW : WriteSequence<[WriteBitTestSetImmLd, WriteRMW]>;
def  WriteBitTestSetRegRMW : WriteSequence<[WriteBitTestSetRegLd, WriteRMW]>;

// Integer shifts and rotates.
defm WriteShift    : Y86SchedWritePair;
defm WriteShiftCL  : Y86SchedWritePair;
defm WriteRotate   : Y86SchedWritePair;
defm WriteRotateCL : Y86SchedWritePair;

// Double shift instructions.
def  WriteSHDrri  : SchedWrite;
def  WriteSHDrrcl : SchedWrite;
def  WriteSHDmri  : SchedWrite;
def  WriteSHDmrcl : SchedWrite;

// BMI1 BEXTR/BLS, BMI2 BZHI
defm WriteBEXTR : Y86SchedWritePair;
defm WriteBLS   : Y86SchedWritePair;
defm WriteBZHI  : Y86SchedWritePair;

// Idioms that clear a register, like xorps %xmm0, %xmm0.
// These can often bypass execution ports completely.
def WriteZero : SchedWrite;

// Branches don't produce values, so they have no latency, but they still
// consume resources. Indirect branches can fold loads.
defm WriteJump : Y86SchedWritePair;

// Floating point. This covers both scalar and vector operations.
def  WriteFLD0          : SchedWrite;
def  WriteFLD1          : SchedWrite;
def  WriteFLDC          : SchedWrite;
def  WriteFLoad         : SchedWrite;
def  WriteFLoadX        : SchedWrite;
def  WriteFLoadY        : SchedWrite;
def  WriteFMaskedLoad   : SchedWrite;
def  WriteFMaskedLoadY  : SchedWrite;
def  WriteFStore        : SchedWrite;
def  WriteFStoreX       : SchedWrite;
def  WriteFStoreY       : SchedWrite;
def  WriteFStoreNT      : SchedWrite;
def  WriteFStoreNTX     : SchedWrite;
def  WriteFStoreNTY     : SchedWrite;

def  WriteFMaskedStore32  : SchedWrite;
def  WriteFMaskedStore64  : SchedWrite;
def  WriteFMaskedStore32Y : SchedWrite;
def  WriteFMaskedStore64Y : SchedWrite;

def  WriteFMove         : SchedWrite;
def  WriteFMoveX        : SchedWrite;
def  WriteFMoveY        : SchedWrite;
def  WriteFMoveZ        : SchedWrite;

defm WriteFAdd    : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point add/sub.
defm WriteFAddX   : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point add/sub (XMM).
defm WriteFAddY   : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point add/sub (YMM).
defm WriteFAddZ   : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point add/sub (ZMM).
defm WriteFAdd64  : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point double add/sub.
defm WriteFAdd64X : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point double add/sub (XMM).
defm WriteFAdd64Y : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point double add/sub (YMM).
defm WriteFAdd64Z : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point double add/sub (ZMM).
defm WriteFCmp    : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point compare.
defm WriteFCmpX   : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point compare (XMM).
defm WriteFCmpY   : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point compare (YMM).
defm WriteFCmpZ   : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point compare (ZMM).
defm WriteFCmp64  : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point double compare.
defm WriteFCmp64X : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point double compare (XMM).
defm WriteFCmp64Y : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point double compare (YMM).
defm WriteFCmp64Z : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point double compare (ZMM).
defm WriteFCom    : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point compare to flags (X87).
defm WriteFComX   : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point compare to flags (SSE).
defm WriteFMul    : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point multiplication.
defm WriteFMulX   : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point multiplication (XMM).
defm WriteFMulY   : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point multiplication (YMM).
defm WriteFMulZ   : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point multiplication (YMM).
defm WriteFMul64  : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point double multiplication.
defm WriteFMul64X : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point double multiplication (XMM).
defm WriteFMul64Y : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point double multiplication (YMM).
defm WriteFMul64Z : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point double multiplication (ZMM).
defm WriteFDiv    : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point division.
defm WriteFDivX   : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point division (XMM).
defm WriteFDivY   : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point division (YMM).
defm WriteFDivZ   : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point division (ZMM).
defm WriteFDiv64  : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point double division.
defm WriteFDiv64X : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point double division (XMM).
defm WriteFDiv64Y : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point double division (YMM).
defm WriteFDiv64Z : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point double division (ZMM).
defm WriteFSqrt  : Y86SchedWritePair<ReadAfterVecLd>;   // Floating point square root.
defm WriteFSqrtX : Y86SchedWritePair<ReadAfterVecXLd>;  // Floating point square root (XMM).
defm WriteFSqrtY : Y86SchedWritePair<ReadAfterVecYLd>;  // Floating point square root (YMM).
defm WriteFSqrtZ : Y86SchedWritePair<ReadAfterVecYLd>;  // Floating point square root (ZMM).
defm WriteFSqrt64  : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point double square root.
defm WriteFSqrt64X : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point double square root (XMM).
defm WriteFSqrt64Y : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point double square root (YMM).
defm WriteFSqrt64Z : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point double square root (ZMM).
defm WriteFSqrt80  : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point long double square root.
defm WriteFRcp   : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point reciprocal estimate.
defm WriteFRcpX  : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point reciprocal estimate (XMM).
defm WriteFRcpY  : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point reciprocal estimate (YMM).
defm WriteFRcpZ  : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point reciprocal estimate (ZMM).
defm WriteFRsqrt : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point reciprocal square root estimate.
defm WriteFRsqrtX: Y86SchedWritePair<ReadAfterVecXLd>; // Floating point reciprocal square root estimate (XMM).
defm WriteFRsqrtY: Y86SchedWritePair<ReadAfterVecYLd>; // Floating point reciprocal square root estimate (YMM).
defm WriteFRsqrtZ: Y86SchedWritePair<ReadAfterVecYLd>; // Floating point reciprocal square root estimate (ZMM).
defm WriteFMA    : Y86SchedWritePair<ReadAfterVecLd>;  // Fused Multiply Add.
defm WriteFMAX   : Y86SchedWritePair<ReadAfterVecXLd>; // Fused Multiply Add (XMM).
defm WriteFMAY   : Y86SchedWritePair<ReadAfterVecYLd>; // Fused Multiply Add (YMM).
defm WriteFMAZ   : Y86SchedWritePair<ReadAfterVecYLd>; // Fused Multiply Add (ZMM).
defm WriteDPPD   : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point double dot product.
defm WriteDPPS   : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point single dot product.
defm WriteDPPSY  : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point single dot product (YMM).
defm WriteDPPSZ  : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point single dot product (ZMM).
defm WriteFSign  : Y86SchedWritePair<ReadAfterVecLd>;  // Floating point fabs/fchs.
defm WriteFRnd   : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point rounding.
defm WriteFRndY  : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point rounding (YMM).
defm WriteFRndZ  : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point rounding (ZMM).
defm WriteFLogic  : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point and/or/xor logicals.
defm WriteFLogicY : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point and/or/xor logicals (YMM).
defm WriteFLogicZ : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point and/or/xor logicals (ZMM).
defm WriteFTest   : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point TEST instructions.
defm WriteFTestY  : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point TEST instructions (YMM).
defm WriteFTestZ  : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point TEST instructions (ZMM).
defm WriteFShuffle  : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point vector shuffles.
defm WriteFShuffleY : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point vector shuffles (YMM).
defm WriteFShuffleZ : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point vector shuffles (ZMM).
defm WriteFVarShuffle  : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point vector variable shuffles.
defm WriteFVarShuffleY : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point vector variable shuffles (YMM).
defm WriteFVarShuffleZ : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point vector variable shuffles (ZMM).
defm WriteFBlend  : Y86SchedWritePair<ReadAfterVecXLd>; // Floating point vector blends.
defm WriteFBlendY : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point vector blends (YMM).
defm WriteFBlendZ : Y86SchedWritePair<ReadAfterVecYLd>; // Floating point vector blends (ZMM).
defm WriteFVarBlend  : Y86SchedWritePair<ReadAfterVecXLd>; // Fp vector variable blends.
defm WriteFVarBlendY : Y86SchedWritePair<ReadAfterVecYLd>; // Fp vector variable blends (YMM).
defm WriteFVarBlendZ : Y86SchedWritePair<ReadAfterVecYLd>; // Fp vector variable blends (YMZMM).

// FMA Scheduling helper class.
class FMASC { Y86FoldableSchedWrite Sched = WriteFAdd; }

// Horizontal Add/Sub (float and integer)
defm WriteFHAdd  : Y86SchedWritePair<ReadAfterVecXLd>;
defm WriteFHAddY : Y86SchedWritePair<ReadAfterVecYLd>;
defm WriteFHAddZ : Y86SchedWritePair<ReadAfterVecYLd>;
defm WritePHAdd  : Y86SchedWritePair<ReadAfterVecLd>;
defm WritePHAddX : Y86SchedWritePair<ReadAfterVecXLd>;
defm WritePHAddY : Y86SchedWritePair<ReadAfterVecYLd>;
defm WritePHAddZ : Y86SchedWritePair<ReadAfterVecYLd>;

// Vector integer operations.
def  WriteVecLoad         : SchedWrite;
def  WriteVecLoadX        : SchedWrite;
def  WriteVecLoadY        : SchedWrite;
def  WriteVecLoadNT       : SchedWrite;
def  WriteVecLoadNTY      : SchedWrite;
def  WriteVecMaskedLoad   : SchedWrite;
def  WriteVecMaskedLoadY  : SchedWrite;
def  WriteVecStore        : SchedWrite;
def  WriteVecStoreX       : SchedWrite;
def  WriteVecStoreY       : SchedWrite;
def  WriteVecStoreNT      : SchedWrite;
def  WriteVecStoreNTY     : SchedWrite;
def  WriteVecMaskedStore32  : SchedWrite;
def  WriteVecMaskedStore64  : SchedWrite;
def  WriteVecMaskedStore32Y : SchedWrite;
def  WriteVecMaskedStore64Y : SchedWrite;
def  WriteVecMove         : SchedWrite;
def  WriteVecMoveX        : SchedWrite;
def  WriteVecMoveY        : SchedWrite;
def  WriteVecMoveZ        : SchedWrite;
def  WriteVecMoveToGpr    : SchedWrite;
def  WriteVecMoveFromGpr  : SchedWrite;

defm WriteVecALU    : Y86SchedWritePair<ReadAfterVecLd>;  // Vector integer ALU op, no logicals.
defm WriteVecALUX   : Y86SchedWritePair<ReadAfterVecXLd>; // Vector integer ALU op, no logicals (XMM).
defm WriteVecALUY   : Y86SchedWritePair<ReadAfterVecYLd>; // Vector integer ALU op, no logicals (YMM).
defm WriteVecALUZ   : Y86SchedWritePair<ReadAfterVecYLd>; // Vector integer ALU op, no logicals (ZMM).
defm WriteVecLogic  : Y86SchedWritePair<ReadAfterVecLd>;  // Vector integer and/or/xor logicals.
defm WriteVecLogicX : Y86SchedWritePair<ReadAfterVecXLd>; // Vector integer and/or/xor logicals (XMM).
defm WriteVecLogicY : Y86SchedWritePair<ReadAfterVecYLd>; // Vector integer and/or/xor logicals (YMM).
defm WriteVecLogicZ : Y86SchedWritePair<ReadAfterVecYLd>; // Vector integer and/or/xor logicals (ZMM).
defm WriteVecTest  : Y86SchedWritePair<ReadAfterVecXLd>;  // Vector integer TEST instructions.
defm WriteVecTestY : Y86SchedWritePair<ReadAfterVecYLd>;  // Vector integer TEST instructions (YMM).
defm WriteVecTestZ : Y86SchedWritePair<ReadAfterVecYLd>;  // Vector integer TEST instructions (ZMM).
defm WriteVecShift  : Y86SchedWritePair<ReadAfterVecLd>;  // Vector integer shifts (default).
defm WriteVecShiftX : Y86SchedWritePair<ReadAfterVecXLd>; // Vector integer shifts (XMM).
defm WriteVecShiftY : Y86SchedWritePair<ReadAfterVecYLd>; // Vector integer shifts (YMM).
defm WriteVecShiftZ : Y86SchedWritePair<ReadAfterVecYLd>; // Vector integer shifts (ZMM).
defm WriteVecShiftImm : Y86SchedWritePair<ReadAfterVecLd>;  // Vector integer immediate shifts (default).
defm WriteVecShiftImmX: Y86SchedWritePair<ReadAfterVecXLd>; // Vector integer immediate shifts (XMM).
defm WriteVecShiftImmY: Y86SchedWritePair<ReadAfterVecYLd>; // Vector integer immediate shifts (YMM).
defm WriteVecShiftImmZ: Y86SchedWritePair<ReadAfterVecYLd>; // Vector integer immediate shifts (ZMM).
defm WriteVecIMul  : Y86SchedWritePair<ReadAfterVecLd>;  // Vector integer multiply (default).
defm WriteVecIMulX : Y86SchedWritePair<ReadAfterVecXLd>; // Vector integer multiply (XMM).
defm WriteVecIMulY : Y86SchedWritePair<ReadAfterVecYLd>; // Vector integer multiply (YMM).
defm WriteVecIMulZ : Y86SchedWritePair<ReadAfterVecYLd>; // Vector integer multiply (ZMM).
defm WritePMULLD   : Y86SchedWritePair<ReadAfterVecXLd>; // Vector PMULLD.
defm WritePMULLDY  : Y86SchedWritePair<ReadAfterVecYLd>; // Vector PMULLD (YMM).
defm WritePMULLDZ  : Y86SchedWritePair<ReadAfterVecYLd>; // Vector PMULLD (ZMM).
defm WriteShuffle  : Y86SchedWritePair<ReadAfterVecLd>;  // Vector shuffles.
defm WriteShuffleX : Y86SchedWritePair<ReadAfterVecXLd>; // Vector shuffles (XMM).
defm WriteShuffleY : Y86SchedWritePair<ReadAfterVecYLd>; // Vector shuffles (YMM).
defm WriteShuffleZ : Y86SchedWritePair<ReadAfterVecYLd>; // Vector shuffles (ZMM).
defm WriteVarShuffle  : Y86SchedWritePair<ReadAfterVecLd>;  // Vector variable shuffles.
defm WriteVarShuffleX : Y86SchedWritePair<ReadAfterVecXLd>; // Vector variable shuffles (XMM).
defm WriteVarShuffleY : Y86SchedWritePair<ReadAfterVecYLd>; // Vector variable shuffles (YMM).
defm WriteVarShuffleZ : Y86SchedWritePair<ReadAfterVecYLd>; // Vector variable shuffles (ZMM).
defm WriteBlend  : Y86SchedWritePair<ReadAfterVecXLd>; // Vector blends.
defm WriteBlendY : Y86SchedWritePair<ReadAfterVecYLd>; // Vector blends (YMM).
defm WriteBlendZ : Y86SchedWritePair<ReadAfterVecYLd>; // Vector blends (ZMM).
defm WriteVarBlend  : Y86SchedWritePair<ReadAfterVecXLd>; // Vector variable blends.
defm WriteVarBlendY : Y86SchedWritePair<ReadAfterVecYLd>; // Vector variable blends (YMM).
defm WriteVarBlendZ : Y86SchedWritePair<ReadAfterVecYLd>; // Vector variable blends (ZMM).
defm WritePSADBW  : Y86SchedWritePair<ReadAfterVecLd>;  // Vector PSADBW.
defm WritePSADBWX : Y86SchedWritePair<ReadAfterVecXLd>; // Vector PSADBW (XMM).
defm WritePSADBWY : Y86SchedWritePair<ReadAfterVecYLd>; // Vector PSADBW (YMM).
defm WritePSADBWZ : Y86SchedWritePair<ReadAfterVecYLd>; // Vector PSADBW (ZMM).
defm WriteMPSAD  : Y86SchedWritePair<ReadAfterVecXLd>; // Vector MPSAD.
defm WriteMPSADY : Y86SchedWritePair<ReadAfterVecYLd>; // Vector MPSAD (YMM).
defm WriteMPSADZ : Y86SchedWritePair<ReadAfterVecYLd>; // Vector MPSAD (ZMM).
defm WritePHMINPOS : Y86SchedWritePair<ReadAfterVecXLd>;  // Vector PHMINPOS.

// Vector insert/extract operations.
defm WriteVecInsert : Y86SchedWritePair; // Insert gpr to vector element.
def  WriteVecExtract : SchedWrite; // Extract vector element to gpr.
def  WriteVecExtractSt : SchedWrite; // Extract vector element and store.

// MOVMSK operations.
def WriteFMOVMSK    : SchedWrite;
def WriteVecMOVMSK  : SchedWrite;
def WriteVecMOVMSKY : SchedWrite;
def WriteMMXMOVMSK  : SchedWrite;

// Conversion between integer and float.
defm WriteCvtSD2I  : Y86SchedWritePair<ReadAfterVecLd>;  // Double -> Integer.
defm WriteCvtPD2I  : Y86SchedWritePair<ReadAfterVecXLd>; // Double -> Integer (XMM).
defm WriteCvtPD2IY : Y86SchedWritePair<ReadAfterVecYLd>; // Double -> Integer (YMM).
defm WriteCvtPD2IZ : Y86SchedWritePair<ReadAfterVecYLd>; // Double -> Integer (ZMM).

defm WriteCvtSS2I  : Y86SchedWritePair<ReadAfterVecLd>;  // Float -> Integer.
defm WriteCvtPS2I  : Y86SchedWritePair<ReadAfterVecXLd>; // Float -> Integer (XMM).
defm WriteCvtPS2IY : Y86SchedWritePair<ReadAfterVecYLd>; // Float -> Integer (YMM).
defm WriteCvtPS2IZ : Y86SchedWritePair<ReadAfterVecYLd>; // Float -> Integer (ZMM).

defm WriteCvtI2SD  : Y86SchedWritePair<ReadAfterVecLd>;  // Integer -> Double.
defm WriteCvtI2PD  : Y86SchedWritePair<ReadAfterVecXLd>; // Integer -> Double (XMM).
defm WriteCvtI2PDY : Y86SchedWritePair<ReadAfterVecYLd>; // Integer -> Double (YMM).
defm WriteCvtI2PDZ : Y86SchedWritePair<ReadAfterVecYLd>; // Integer -> Double (ZMM).

defm WriteCvtI2SS  : Y86SchedWritePair<ReadAfterVecLd>;  // Integer -> Float.
defm WriteCvtI2PS  : Y86SchedWritePair<ReadAfterVecXLd>; // Integer -> Float (XMM).
defm WriteCvtI2PSY : Y86SchedWritePair<ReadAfterVecYLd>; // Integer -> Float (YMM).
defm WriteCvtI2PSZ : Y86SchedWritePair<ReadAfterVecYLd>; // Integer -> Float (ZMM).

defm WriteCvtSS2SD  : Y86SchedWritePair<ReadAfterVecLd>;  // Float -> Double size conversion.
defm WriteCvtPS2PD  : Y86SchedWritePair<ReadAfterVecXLd>; // Float -> Double size conversion (XMM).
defm WriteCvtPS2PDY : Y86SchedWritePair<ReadAfterVecYLd>; // Float -> Double size conversion (YMM).
defm WriteCvtPS2PDZ : Y86SchedWritePair<ReadAfterVecYLd>; // Float -> Double size conversion (ZMM).

defm WriteCvtSD2SS  : Y86SchedWritePair<ReadAfterVecLd>;  // Double -> Float size conversion.
defm WriteCvtPD2PS  : Y86SchedWritePair<ReadAfterVecXLd>; // Double -> Float size conversion (XMM).
defm WriteCvtPD2PSY : Y86SchedWritePair<ReadAfterVecYLd>; // Double -> Float size conversion (YMM).
defm WriteCvtPD2PSZ : Y86SchedWritePair<ReadAfterVecYLd>; // Double -> Float size conversion (ZMM).

defm WriteCvtPH2PS  : Y86SchedWritePair<ReadAfterVecXLd>; // Half -> Float size conversion.
defm WriteCvtPH2PSY : Y86SchedWritePair<ReadAfterVecYLd>; // Half -> Float size conversion (YMM).
defm WriteCvtPH2PSZ : Y86SchedWritePair<ReadAfterVecYLd>; // Half -> Float size conversion (ZMM).

def  WriteCvtPS2PH    : SchedWrite; // // Float -> Half size conversion.
def  WriteCvtPS2PHY   : SchedWrite; // // Float -> Half size conversion (YMM).
def  WriteCvtPS2PHZ   : SchedWrite; // // Float -> Half size conversion (ZMM).
def  WriteCvtPS2PHSt  : SchedWrite; // // Float -> Half + store size conversion.
def  WriteCvtPS2PHYSt : SchedWrite; // // Float -> Half + store size conversion (YMM).
def  WriteCvtPS2PHZSt : SchedWrite; // // Float -> Half + store size conversion (ZMM).

// CRC32 instruction.
defm WriteCRC32 : Y86SchedWritePair<ReadAfterLd>;

// Strings instructions.
// Packed Compare Implicit Length Strings, Return Mask
defm WritePCmpIStrM : Y86SchedWritePair<ReadAfterVecXLd>;
// Packed Compare Explicit Length Strings, Return Mask
defm WritePCmpEStrM : Y86SchedWritePair<ReadAfterVecXLd>;
// Packed Compare Implicit Length Strings, Return Index
defm WritePCmpIStrI : Y86SchedWritePair<ReadAfterVecXLd>;
// Packed Compare Explicit Length Strings, Return Index
defm WritePCmpEStrI : Y86SchedWritePair<ReadAfterVecXLd>;

// AES instructions.
defm WriteAESDecEnc : Y86SchedWritePair<ReadAfterVecXLd>; // Decryption, encryption.
defm WriteAESIMC : Y86SchedWritePair<ReadAfterVecXLd>; // InvMixColumn.
defm WriteAESKeyGen : Y86SchedWritePair<ReadAfterVecXLd>; // Key Generation.

// Carry-less multiplication instructions.
defm WriteCLMul : Y86SchedWritePair<ReadAfterVecXLd>;

// EMMS/FEMMS
def WriteEMMS : SchedWrite;

// Load/store MXCSR
def WriteLDMXCSR : SchedWrite;
def WriteSTMXCSR : SchedWrite;

// Catch-all for expensive system instructions.
def WriteSystem : SchedWrite;

// AVX2.
defm WriteFShuffle256 : Y86SchedWritePair<ReadAfterVecYLd>; // Fp 256-bit width vector shuffles.
defm WriteFVarShuffle256 : Y86SchedWritePair<ReadAfterVecYLd>; // Fp 256-bit width variable shuffles.
defm WriteShuffle256 : Y86SchedWritePair<ReadAfterVecYLd>; // 256-bit width vector shuffles.
defm WriteVPMOV256 : Y86SchedWritePair<ReadAfterVecYLd>; // 256-bit width packed vector width-changing move.
defm WriteVarShuffle256 : Y86SchedWritePair<ReadAfterVecYLd>; // 256-bit width vector variable shuffles.
defm WriteVarVecShift  : Y86SchedWritePair<ReadAfterVecXLd>; // Variable vector shifts.
defm WriteVarVecShiftY : Y86SchedWritePair<ReadAfterVecYLd>; // Variable vector shifts (YMM).
defm WriteVarVecShiftZ : Y86SchedWritePair<ReadAfterVecYLd>; // Variable vector shifts (ZMM).

// Old microcoded instructions that nobody use.
def WriteMicrocoded : SchedWrite;

// Fence instructions.
def WriteFence : SchedWrite;

// Nop, not very useful expect it provides a model for nops!
def WriteNop : SchedWrite;

// Move/Load/Store wrappers.
def WriteFMoveLS
 : Y86SchedWriteMoveLS<WriteFMove, WriteFLoad, WriteFStore>;
def WriteFMoveLSX
 : Y86SchedWriteMoveLS<WriteFMoveX, WriteFLoadX, WriteFStoreX>;
def WriteFMoveLSY
 : Y86SchedWriteMoveLS<WriteFMoveY, WriteFLoadY, WriteFStoreY>;
def WriteFMoveLSZ
 : Y86SchedWriteMoveLS<WriteFMoveZ, WriteFLoadY, WriteFStoreY>;
def SchedWriteFMoveLS
  : Y86SchedWriteMoveLSWidths<WriteFMoveLS, WriteFMoveLSX,
                              WriteFMoveLSY, WriteFMoveLSZ>;

def WriteFMoveLSNT
 : Y86SchedWriteMoveLS<WriteFMove, WriteFLoad, WriteFStoreNT>;
def WriteFMoveLSNTX
 : Y86SchedWriteMoveLS<WriteFMove, WriteFLoad, WriteFStoreNTX>;
def WriteFMoveLSNTY
 : Y86SchedWriteMoveLS<WriteFMoveY, WriteFLoadY, WriteFStoreNTY>;
def SchedWriteFMoveLSNT
  : Y86SchedWriteMoveLSWidths<WriteFMoveLSNT, WriteFMoveLSNTX,
                              WriteFMoveLSNTY, WriteFMoveLSNTY>;

def WriteVecMoveLS
 : Y86SchedWriteMoveLS<WriteVecMove, WriteVecLoad, WriteVecStore>;
def WriteVecMoveLSX
 : Y86SchedWriteMoveLS<WriteVecMoveX, WriteVecLoadX, WriteVecStoreX>;
def WriteVecMoveLSY
 : Y86SchedWriteMoveLS<WriteVecMoveY, WriteVecLoadY, WriteVecStoreY>;
def WriteVecMoveLSZ
 : Y86SchedWriteMoveLS<WriteVecMoveZ, WriteVecLoadY, WriteVecStoreY>;
def SchedWriteVecMoveLS
  : Y86SchedWriteMoveLSWidths<WriteVecMoveLS, WriteVecMoveLSX,
                              WriteVecMoveLSY, WriteVecMoveLSZ>;

def WriteVecMoveLSNT
 : Y86SchedWriteMoveLS<WriteVecMove, WriteVecLoadNT, WriteVecStoreNT>;
def WriteVecMoveLSNTX
 : Y86SchedWriteMoveLS<WriteVecMoveX, WriteVecLoadNT, WriteVecStoreNT>;
def WriteVecMoveLSNTY
 : Y86SchedWriteMoveLS<WriteVecMoveY, WriteVecLoadNTY, WriteVecStoreNTY>;
def SchedWriteVecMoveLSNT
  : Y86SchedWriteMoveLSWidths<WriteVecMoveLSNT, WriteVecMoveLSNTX,
                              WriteVecMoveLSNTY, WriteVecMoveLSNTY>;

// Conditional SIMD Packed Loads and Stores wrappers.
def WriteFMaskMove32
  : Y86SchedWriteMaskMove<WriteFMaskedLoad, WriteFMaskedStore32>;
def WriteFMaskMove64
  : Y86SchedWriteMaskMove<WriteFMaskedLoad, WriteFMaskedStore64>;
def WriteFMaskMove32Y
  : Y86SchedWriteMaskMove<WriteFMaskedLoadY, WriteFMaskedStore32Y>;
def WriteFMaskMove64Y
  : Y86SchedWriteMaskMove<WriteFMaskedLoadY, WriteFMaskedStore64Y>;
def WriteVecMaskMove32
  : Y86SchedWriteMaskMove<WriteVecMaskedLoad, WriteVecMaskedStore32>;
def WriteVecMaskMove64
  : Y86SchedWriteMaskMove<WriteVecMaskedLoad, WriteVecMaskedStore64>;
def WriteVecMaskMove32Y
  : Y86SchedWriteMaskMove<WriteVecMaskedLoadY, WriteVecMaskedStore32Y>;
def WriteVecMaskMove64Y
  : Y86SchedWriteMaskMove<WriteVecMaskedLoadY, WriteVecMaskedStore64Y>;

// Vector width wrappers.
def SchedWriteFAdd
 : Y86SchedWriteWidths<WriteFAdd, WriteFAddX, WriteFAddY, WriteFAddZ>;
def SchedWriteFAdd64
 : Y86SchedWriteWidths<WriteFAdd64, WriteFAdd64X, WriteFAdd64Y, WriteFAdd64Z>;
def SchedWriteFHAdd
 : Y86SchedWriteWidths<WriteFHAdd, WriteFHAdd, WriteFHAddY, WriteFHAddZ>;
def SchedWriteFCmp
 : Y86SchedWriteWidths<WriteFCmp, WriteFCmpX, WriteFCmpY, WriteFCmpZ>;
def SchedWriteFCmp64
 : Y86SchedWriteWidths<WriteFCmp64, WriteFCmp64X, WriteFCmp64Y, WriteFCmp64Z>;
def SchedWriteFMul
 : Y86SchedWriteWidths<WriteFMul, WriteFMulX, WriteFMulY, WriteFMulZ>;
def SchedWriteFMul64
 : Y86SchedWriteWidths<WriteFMul64, WriteFMul64X, WriteFMul64Y, WriteFMul64Z>;
def SchedWriteFMA
 : Y86SchedWriteWidths<WriteFMA, WriteFMAX, WriteFMAY, WriteFMAZ>;
def SchedWriteDPPD
 : Y86SchedWriteWidths<WriteDPPD, WriteDPPD, WriteDPPD, WriteDPPD>;
def SchedWriteDPPS
 : Y86SchedWriteWidths<WriteDPPS, WriteDPPS, WriteDPPSY, WriteDPPSZ>;
def SchedWriteFDiv
 : Y86SchedWriteWidths<WriteFDiv, WriteFDivX, WriteFDivY, WriteFDivZ>;
def SchedWriteFDiv64
 : Y86SchedWriteWidths<WriteFDiv64, WriteFDiv64X, WriteFDiv64Y, WriteFDiv64Z>;
def SchedWriteFSqrt
 : Y86SchedWriteWidths<WriteFSqrt, WriteFSqrtX,
                       WriteFSqrtY, WriteFSqrtZ>;
def SchedWriteFSqrt64
 : Y86SchedWriteWidths<WriteFSqrt64, WriteFSqrt64X,
                       WriteFSqrt64Y, WriteFSqrt64Z>;
def SchedWriteFRcp
 : Y86SchedWriteWidths<WriteFRcp, WriteFRcpX, WriteFRcpY, WriteFRcpZ>;
def SchedWriteFRsqrt
 : Y86SchedWriteWidths<WriteFRsqrt, WriteFRsqrtX, WriteFRsqrtY, WriteFRsqrtZ>;
def SchedWriteFRnd
 : Y86SchedWriteWidths<WriteFRnd, WriteFRnd, WriteFRndY, WriteFRndZ>;
def SchedWriteFLogic
 : Y86SchedWriteWidths<WriteFLogic, WriteFLogic, WriteFLogicY, WriteFLogicZ>;
def SchedWriteFTest
 : Y86SchedWriteWidths<WriteFTest, WriteFTest, WriteFTestY, WriteFTestZ>;

def SchedWriteFShuffle
 : Y86SchedWriteWidths<WriteFShuffle, WriteFShuffle,
                       WriteFShuffleY, WriteFShuffleZ>;
def SchedWriteFVarShuffle
 : Y86SchedWriteWidths<WriteFVarShuffle, WriteFVarShuffle,
                       WriteFVarShuffleY, WriteFVarShuffleZ>;
def SchedWriteFBlend
 : Y86SchedWriteWidths<WriteFBlend, WriteFBlend, WriteFBlendY, WriteFBlendZ>;
def SchedWriteFVarBlend
 : Y86SchedWriteWidths<WriteFVarBlend, WriteFVarBlend,
                       WriteFVarBlendY, WriteFVarBlendZ>;

def SchedWriteCvtDQ2PD
 : Y86SchedWriteWidths<WriteCvtI2SD, WriteCvtI2PD,
                       WriteCvtI2PDY, WriteCvtI2PDZ>;
def SchedWriteCvtDQ2PS
 : Y86SchedWriteWidths<WriteCvtI2SS, WriteCvtI2PS,
                       WriteCvtI2PSY, WriteCvtI2PSZ>;
def SchedWriteCvtPD2DQ
 : Y86SchedWriteWidths<WriteCvtSD2I, WriteCvtPD2I,
                       WriteCvtPD2IY, WriteCvtPD2IZ>;
def SchedWriteCvtPS2DQ
 : Y86SchedWriteWidths<WriteCvtSS2I, WriteCvtPS2I,
                       WriteCvtPS2IY, WriteCvtPS2IZ>;
def SchedWriteCvtPS2PD
 : Y86SchedWriteWidths<WriteCvtSS2SD, WriteCvtPS2PD,
                       WriteCvtPS2PDY, WriteCvtPS2PDZ>;
def SchedWriteCvtPD2PS
 : Y86SchedWriteWidths<WriteCvtSD2SS, WriteCvtPD2PS,
                       WriteCvtPD2PSY, WriteCvtPD2PSZ>;

def SchedWriteVecALU
 : Y86SchedWriteWidths<WriteVecALU, WriteVecALUX, WriteVecALUY, WriteVecALUZ>;
def SchedWritePHAdd
 : Y86SchedWriteWidths<WritePHAdd, WritePHAddX, WritePHAddY, WritePHAddZ>;
def SchedWriteVecLogic
 : Y86SchedWriteWidths<WriteVecLogic, WriteVecLogicX,
                       WriteVecLogicY, WriteVecLogicZ>;
def SchedWriteVecTest
 : Y86SchedWriteWidths<WriteVecTest, WriteVecTest,
                       WriteVecTestY, WriteVecTestZ>;
def SchedWriteVecShift
 : Y86SchedWriteWidths<WriteVecShift, WriteVecShiftX,
                       WriteVecShiftY, WriteVecShiftZ>;
def SchedWriteVecShiftImm
 : Y86SchedWriteWidths<WriteVecShiftImm, WriteVecShiftImmX,
                       WriteVecShiftImmY, WriteVecShiftImmZ>;
def SchedWriteVarVecShift
 : Y86SchedWriteWidths<WriteVarVecShift, WriteVarVecShift,
                       WriteVarVecShiftY, WriteVarVecShiftZ>;
def SchedWriteVecIMul
 : Y86SchedWriteWidths<WriteVecIMul, WriteVecIMulX,
                       WriteVecIMulY, WriteVecIMulZ>;
def SchedWritePMULLD
 : Y86SchedWriteWidths<WritePMULLD, WritePMULLD,
                       WritePMULLDY, WritePMULLDZ>;
def SchedWriteMPSAD
 : Y86SchedWriteWidths<WriteMPSAD, WriteMPSAD,
                       WriteMPSADY, WriteMPSADZ>;
def SchedWritePSADBW
 : Y86SchedWriteWidths<WritePSADBW, WritePSADBWX,
                       WritePSADBWY, WritePSADBWZ>;

def SchedWriteShuffle
 : Y86SchedWriteWidths<WriteShuffle, WriteShuffleX,
                       WriteShuffleY, WriteShuffleZ>;
def SchedWriteVarShuffle
 : Y86SchedWriteWidths<WriteVarShuffle, WriteVarShuffleX,
                       WriteVarShuffleY, WriteVarShuffleZ>;
def SchedWriteBlend
 : Y86SchedWriteWidths<WriteBlend, WriteBlend, WriteBlendY, WriteBlendZ>;
def SchedWriteVarBlend
 : Y86SchedWriteWidths<WriteVarBlend, WriteVarBlend,
                       WriteVarBlendY, WriteVarBlendZ>;

// Vector size wrappers.
// FIXME: Currently PH uses the same schedule method as PS.
// We may refine them later.
def SchedWriteFAddSizes
 : Y86SchedWriteSizes<SchedWriteFAdd, SchedWriteFAdd, SchedWriteFAdd64>;
def SchedWriteFCmpSizes
 : Y86SchedWriteSizes<SchedWriteFCmp, SchedWriteFCmp, SchedWriteFCmp64>;
def SchedWriteFMulSizes
 : Y86SchedWriteSizes<SchedWriteFMul, SchedWriteFMul, SchedWriteFMul64>;
def SchedWriteFDivSizes
 : Y86SchedWriteSizes<SchedWriteFDiv, SchedWriteFDiv, SchedWriteFDiv64>;
def SchedWriteFSqrtSizes
 : Y86SchedWriteSizes<SchedWriteFSqrt, SchedWriteFSqrt, SchedWriteFSqrt64>;
def SchedWriteFLogicSizes
 : Y86SchedWriteSizes<SchedWriteFLogic, SchedWriteFLogic, SchedWriteFLogic>;
def SchedWriteFShuffleSizes
 : Y86SchedWriteSizes<SchedWriteFShuffle, SchedWriteFShuffle, SchedWriteFShuffle>;

//===----------------------------------------------------------------------===//
// Generic Processor Scheduler Models.

// IssueWidth is analogous to the number of decode units. Core and its
// descendents, including Nehalem and SandyBridge have 4 decoders.
// Resources beyond the decoder operate on micro-ops and are bufferred
// so adjacent micro-ops don't directly compete.
//
// MicroOpBufferSize > 1 indicates that RAW dependencies can be
// decoded in the same cycle. The value 32 is a reasonably arbitrary
// number of in-flight instructions.
//
// HighLatency=10 is optimistic. Y86InstrInfo::isHighLatencyDef
// indicates high latency opcodes. Alternatively, InstrItinData
// entries may be included here to define specific operand
// latencies. Since these latencies are not used for pipeline hazards,
// they do not need to be exact.
//
// The GenericY86Model contains no instruction schedules
// and disables PostRAScheduler.
class GenericY86Model : SchedMachineModel {
  let IssueWidth = 4;
  let MicroOpBufferSize = 32;
  let LoadLatency = 4;
  let HighLatency = 10;
  let PostRAScheduler = 0;
  let CompleteModel = 0;
}

def GenericModel : GenericY86Model;

// Define a model with the PostRAScheduler enabled.
def GenericPostRAModel : GenericY86Model {
  let PostRAScheduler = 1;
}
