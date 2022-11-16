
def SDTY86Ret     : SDTypeProfile<0, -1, [SDTCisVT<0, i32>]>;

def SDTBinaryArithWithFlags : SDTypeProfile<2, 2,
                                            [SDTCisSameAs<0, 2>,
                                             SDTCisSameAs<0, 3>,
                                             SDTCisInt<0>, SDTCisVT<1, i32>]>;

def Y86retflag : SDNode<"Y86ISD::RET_FLAG", SDTY86Ret,
                        [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

def Y86add_flag  : SDNode<"Y86ISD::ADD",  SDTBinaryArithWithFlags,
                          [SDNPCommutative]>;
def Y86sub_flag  : SDNode<"Y86ISD::SUB",  SDTBinaryArithWithFlags>;

// A version of ptr_rc which excludes SP, ESP, and RSP. This is used for
// the index operand of an address, to conform to x86 encoding restrictions.
def ptr_rc_nosp : PointerLikeRegClass<1>;
// A version of i8mem for use on x86-64 and x32 that uses a NOREX GPR instead
// of a plain GPR, so that it doesn't potentially require a REX prefix.
def ptr_rc_norex : PointerLikeRegClass<2>;
def ptr_rc_norex_nosp : PointerLikeRegClass<3>;


def Y86MemAsmOperand : AsmOperandClass {
 let Name = "Mem";
}

let RenderMethod = "addMemOperands", SuperClasses = [Y86MemAsmOperand] in {
    def Y86Mem8AsmOperand   : AsmOperandClass { let Name = "Mem8"; }
    def Y86Mem16AsmOperand  : AsmOperandClass { let Name = "Mem16"; }
    def Y86Mem32AsmOperand  : AsmOperandClass { let Name = "Mem32"; }
    def Y86Mem64AsmOperand  : AsmOperandClass { let Name = "Mem64"; }
}

class Y86MemOperand<string printMethod,
          AsmOperandClass parserMatchClass = Y86MemAsmOperand> : Operand<iPTR> {
  let PrintMethod = printMethod;
  let MIOperandInfo = (ops ptr_rc, i8imm, ptr_rc_nosp, i32imm, SEGMENT_REG);
  let ParserMatchClass = parserMatchClass;
  let OperandType = "OPERAND_MEMORY";
}


def i8mem_NOREX : Operand<iPTR> {
  let PrintMethod = "printbytemem";
  let MIOperandInfo = (ops ptr_rc_norex, i8imm, ptr_rc_norex_nosp, i32imm,
                       SEGMENT_REG);
  let ParserMatchClass = Y86Mem8AsmOperand;
  let OperandType = "OPERAND_MEMORY";
}

def i8mem   : Y86MemOperand<"printbytemem",   Y86Mem8AsmOperand>;
def i16mem  : Y86MemOperand<"printwordmem",   Y86Mem16AsmOperand>;
def i32mem  : Y86MemOperand<"printdwordmem",  Y86Mem32AsmOperand>;
def i64mem  : Y86MemOperand<"printqwordmem",  Y86Mem64AsmOperand>;

def addr      : ComplexPattern<iPTR, 5, "selectAddr", [], [SDNPWantParent]>;

// Helper fragments for loads.

// It's safe to fold a zextload/extload from i1 as a regular i8 load. The
// upper bits are guaranteed to be zero and we were going to emit a MOV8rm
// which might get folded during peephole anyway.
def loadi8 : PatFrag<(ops node:$ptr), (i8 (unindexedload node:$ptr)), [{
  LoadSDNode *LD = cast<LoadSDNode>(N);
  ISD::LoadExtType ExtType = LD->getExtensionType();
  return ExtType == ISD::NON_EXTLOAD || ExtType == ISD::EXTLOAD ||
         ExtType == ISD::ZEXTLOAD;
}]>;

// It's always safe to treat a anyext i16 load as a i32 load if the i16 is
// known to be 32-bit aligned or better. Ditto for i8 to i16.
def loadi16 : PatFrag<(ops node:$ptr), (i16 (unindexedload node:$ptr)), [{
  LoadSDNode *LD = cast<LoadSDNode>(N);
  ISD::LoadExtType ExtType = LD->getExtensionType();
  if (ExtType == ISD::NON_EXTLOAD)
    return true;
  if (ExtType == ISD::EXTLOAD && EnablePromoteAnyextLoad)
    return LD->getAlignment() >= 2 && LD->isSimple();
  return false;
}]>;

def loadi32 : PatFrag<(ops node:$ptr), (i32 (unindexedload node:$ptr)), [{
  LoadSDNode *LD = cast<LoadSDNode>(N);
  ISD::LoadExtType ExtType = LD->getExtensionType();
  if (ExtType == ISD::NON_EXTLOAD)
    return true;
  if (ExtType == ISD::EXTLOAD && EnablePromoteAnyextLoad)
    return LD->getAlignment() >= 4 && LD->isSimple();
  return false;
}]>;

def imm_su : PatLeaf<(imm), [{
  return !shouldAvoidImmediateInstFormsForSize(N);
}]>;

class ImmSExtAsmOperandClass : AsmOperandClass {
  let SuperClasses = [ImmAsmOperand];
  let RenderMethod = "addImmOperands";
}

// [0, 0x0000007F] | [0x000000000000FF80, 0x000000000000FFFF] |
//   [0xFFFFFFFFFFFFFF80, 0xFFFFFFFFFFFFFFFF]
def ImmSExti16i8AsmOperand : ImmSExtAsmOperandClass {
  let Name = "ImmSExti16i8";
  //let SuperClasses = [ImmSExti64i32AsmOperand];
}

// [0, 0x0000007F] | [0x00000000FFFFFF80, 0x00000000FFFFFFFF] |
//   [0xFFFFFFFFFFFFFF80, 0xFFFFFFFFFFFFFFFF]
def ImmSExti32i8AsmOperand : ImmSExtAsmOperandClass {
  let Name = "ImmSExti32i8";
}


// A couple of more descriptive operand definitions.
// 16-bits but only 8 bits are significant.
def i16i8imm  : Operand<i16> {
  let ParserMatchClass = ImmSExti16i8AsmOperand;
  let OperandType = "OPERAND_IMMEDIATE";
}

// 32-bits but only 8 bits are significant.
def i32i8imm  : Operand<i32> {
  let ParserMatchClass = ImmSExti32i8AsmOperand;
  let OperandType = "OPERAND_IMMEDIATE";
}

def i16immSExt8  : ImmLeaf<i16, [{ return isInt<8>(Imm); }]>;
def i32immSExt8  : ImmLeaf<i32, [{ return isInt<8>(Imm); }]>;

def i16immSExt8_su : PatLeaf<(i16immSExt8), [{
    return !shouldAvoidImmediateInstFormsForSize(N);
}]>;
def i32immSExt8_su : PatLeaf<(i32immSExt8), [{
    return !shouldAvoidImmediateInstFormsForSize(N);
}]>;

let hasSideEffects = 0, isMoveReg = 1, SchedRW = [WriteMove] in {

def MOV16rr : I<0x89, FormMR, MRMrr, (outs GR16:$dst), (ins GR16:$src),
                "mov\t$src, $dst", []>;
def MOV32rr : I<0x89, FormMR, MRMrr, (outs GR32:$dst), (ins GR32:$src),
                "mov\t$src, $dst", []>;
def MOV64rr : I<0x89, FormMR, MRMrr, (outs GR64:$dst), (ins GR64:$src),
                 "mov\t$src, $dst", []>, REX_W;
}

let isAsCheapAsAMove = 1, isMoveImm = 1 in {

def MOV16ri : Ii16<0xB8, FormOr, NoMRM, (outs GR16:$dst), (ins i16imm:$src),
                   "mov\t$src, $dst",
                   [(set GR16:$dst, imm:$src)]>;
def MOV32ri : Ii32<0xB8, FormOr, NoMRM, (outs GR32:$dst), (ins i32imm:$src),
                   "mov\t$src, $dst",
                   [(set GR32:$dst, imm:$src)]>;
def MOV64ri : Ii64<0xB8, FormOr, NoMRM, (outs GR64:$dst), (ins i64imm:$src),
                   "mov\t$src, $dst",
                   [(set GR64:$dst, imm:$src)]>, REX_W;
}

let canFoldAsLoad = 1, isReMaterializable = 1, SchedRW = [WriteLoad] in {

def MOV16rm : I<0x8B, FormRM, MRMrm, (outs GR16:$dst), (ins i16mem:$src),
                "mov\t$src, $dst",
                [(set GR16:$dst, (loadi16 addr:$src))]>;
def MOV32rm : I<0x8B, FormRM, MRMrm, (outs GR32:$dst), (ins i32mem:$src),
                "mov\t$src, $dst",
                [(set GR32:$dst, (loadi32 addr:$src))]>;
def MOV64rm : I<0x8B, FormRM, MRMrm, (outs GR64:$dst), (ins i64mem:$src),
                "mov\t$src, $dst",
                [(set GR64:$dst, (load addr:$src))]>, REX_W;
}


let SchedRW = [WriteStore] in {

def MOV16mr : I<0x89, FormMR, MRMrm, (outs), (ins i16mem:$dst, GR16:$src),
                "mov\t$src, $dst",
                [(store GR16:$src, addr:$dst)]>;
def MOV32mr : I<0x89, FormMR, MRMrm, (outs), (ins i32mem:$dst, GR32:$src),
                "mov\t$src, $dst",
                [(store GR32:$src, addr:$dst)]>;
def MOV64mr : I<0x89, FormMR, MRMrm, (outs), (ins i64mem:$dst, GR64:$src),
                "mov\t$src, $dst",
                [(store GR64:$src, addr:$dst)]>, REX_W;
} // SchedRW

def MOVSX64rr32: I<0x63, FormRM, MRMrr, (outs GR64:$dst), (ins GR32:$src),
                    "movs{lq|xd}\t{$src, $dst|$dst, $src}",
                    [(set GR64:$dst, (sext GR32:$src))]>, Sched<[WriteALU]>, REX_W;

def MOVSX64rm32: I<0x63, FormRM, MRMrm, (outs GR64:$dst), (ins i32mem:$src),
                    "movs{lq|xd}\t{$src, $dst|$dst, $src}",
                    [(set GR64:$dst, (i64 (sextloadi32 addr:$src)))]>,
                    Sched<[WriteALULd]>, REX_W;

def : Pat<(sext_inreg GR64:$src, i32),
          (MOVSX64rr32 (EXTRACT_SUBREG GR64:$src, sub_32bit))>;

let Defs = [RSP], Uses = [RSP], hasSideEffects=0 in {
let mayLoad = 1, SchedRW = [WriteLoad] in {
def POP16r  : I<0x58, FormOr, NoMRM, (outs GR16:$reg), (ins), "pop\t$reg", []>;
//def POP32r  : I<0x58, FormOr, NoMRM, (outs GR32:$reg), (ins), "pop\t$reg", []>;
def POP64r  : I<0x58, FormOr, NoMRM, (outs GR64:$reg), (ins), "pop\t$reg", []>;
}

let mayStore = 1, SchedRW = [WriteStore] in {
def PUSH16r  : I<0x50, FormOr, NoMRM, (outs), (ins GR16:$reg), "push\t$reg",[]>;
//def PUSH32r  : I<0x50, FormOr, NoMRM, (outs), (ins GR32:$reg), "push\t$reg",[]>;    
def PUSH64r  : I<0x50, FormOr, NoMRM, (outs), (ins GR64:$reg), "push\t$reg",[]>;    
}

}

let isTerminator = 1, isReturn = 1, isBarrier = 1,
    hasCtrlDep = 1, /*FPForm = SpecialFP,*/ SchedRW = [WriteJumpLd] in {
  def RET  : PseudoI<(outs), (ins i32imm:$adj, variable_ops), [(Y86retflag timm:$adj)]>;

  def RET32  : I   <0xC3, FormO, NoMRM, (outs), (ins variable_ops),
                    "ret", []>;
  def RETI32 : Ii16<0xC2, FormO, NoMRM, (outs), (ins i16imm:$amt, variable_ops),
                    "ret\t$amt", []>; 
}