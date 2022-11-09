//ADDri ADDrr ADDrm


multiclass ArithBinOp<bits<8> opcode_rr, bits<8> opcode_ri, MRM_Format mrm_ri, SDNode node, string asmname>{
  let Constraints = "$dst = $op1" in {
  def NAME#32rr: I<opcode_rr, FormRM, MRMrr, (outs GR32:$dst), (ins GR32:$op1, GR32:$op2),
              !strconcat(asmname,"\t$op1, $op2"),[(set GR32:$dst, (node GR32:$op1, GR32:$op2))]>;
  def NAME#64rr: I<opcode_rr, FormRM, MRMrr, (outs GR32:$dst), (ins GR32:$op1, GR32:$op2),
              !strconcat(asmname,"\t$op1, $op2"),[(set GR32:$dst, (node GR32:$op1, GR32:$op2))]>, REX_W;
  def NAME#32ri: Ii32<opcode_ri, FormMI, mrm_ri, (outs GR32:$dst), (ins GR32:$op1, i32imm:$op2),
              !strconcat(asmname,"\t$op1, $op2"),[(set GR32:$dst, (node GR32:$op1, imm:$op2))]>;
  def NAME#64ri: Ii32<opcode_ri, FormMI, mrm_ri, (outs GR32:$dst), (ins GR32:$op1, i32imm:$op2),
            !strconcat(asmname,"\t$op1, $op2"),[(set GR32:$dst, (node GR32:$op1, imm:$op2))]>, REX_W;
  // TODO: add BinOP#32rm and 16,8bits instr
  }
}

defm ADD: ArithBinOp<0x03, 0x81, MRM0r, add, "add">;
defm SUB: ArithBinOp<0x2B, 0x81, MRM5r, sub, "sub">;

/* let Constraints = "$src = $dst"{
def ADD8ri: Ii32<0x02, FormRI, MRMrm, Imm8, (outs GR8:$dst), (ins GR8:$src, imm8:$imm),
                  "add\t$src, $imm",[(set GR)]>;

/// Y86TypeInfo - This is a bunch of information that describes relevant Y86
/// information about value types.  For example, it can tell you what the
/// register class and preferred load to use.
class Y86TypeInfo<ValueType vt, string instrsuffix, RegisterClass regclass,
                  PatFrag loadnode, Y86MemOperand memoperand, ImmType immkind,
                  Operand immoperand, SDPatternOperator immoperator,
                  Operand imm8operand, SDPatternOperator imm8operator,
                  bit hasOddOpcode, OperandSize opSize,
                  bit hasREX_WPrefix> {
  /// VT - This is the value type itself.
  ValueType VT = vt;

  /// InstrSuffix - This is the suffix used on instructions with this type.  For
  /// example, i8 -> "b", i16 -> "w", i32 -> "l", i64 -> "q".
  string InstrSuffix = instrsuffix;

  /// RegClass - This is the register class associated with this type.  For
  /// example, i8 -> GR8, i16 -> GR16, i32 -> GR32, i64 -> GR64.
  RegisterClass RegClass = regclass;

  /// LoadNode - This is the load node associated with this type.  For
  /// example, i8 -> loadi8, i16 -> loadi16, i32 -> loadi32, i64 -> loadi64.
  PatFrag LoadNode = loadnode;

  /// MemOperand - This is the memory operand associated with this type.  For
  /// example, i8 -> i8mem, i16 -> i16mem, i32 -> i32mem, i64 -> i64mem.
  Y86MemOperand MemOperand = memoperand;

  /// ImmEncoding - This is the encoding of an immediate of this type.  For
  /// example, i8 -> Imm8, i16 -> Imm16, i32 -> Imm32.  Note that i64 -> Imm32
  /// since the immediate fields of i64 instructions is a 32-bit sign extended
  /// value.
  ImmType ImmEncoding = immkind;

  /// ImmOperand - This is the operand kind of an immediate of this type.  For
  /// example, i8 -> i8imm, i16 -> i16imm, i32 -> i32imm.  Note that i64 ->
  /// i64i32imm since the immediate fields of i64 instructions is a 32-bit sign
  /// extended value.
  Operand ImmOperand = immoperand;

  /// ImmOperator - This is the operator that should be used to match an
  /// immediate of this kind in a pattern (e.g. imm, or i64immSExt32).
  SDPatternOperator ImmOperator = immoperator;

  /// Imm8Operand - This is the operand kind to use for an imm8 of this type.
  /// For example, i8 -> <invalid>, i16 -> i16i8imm, i32 -> i32i8imm.  This is
  /// only used for instructions that have a sign-extended imm8 field form.
  Operand Imm8Operand = imm8operand;

  /// Imm8Operator - This is the operator that should be used to match an 8-bit
  /// sign extended immediate of this kind in a pattern (e.g. imm16immSExt8).
  SDPatternOperator Imm8Operator = imm8operator;

  /// HasOddOpcode - This bit is true if the instruction should have an odd (as
  /// opposed to even) opcode.  Operations on i8 are usually even, operations on
  /// other datatypes are odd.
  bit HasOddOpcode = hasOddOpcode;

  /// OpSize - Selects whether the instruction needs a 0x66 prefix based on
  /// 16-bit vs 32-bit mode. i8/i64 set this to OpSizeFixed. i16 sets this
  /// to Opsize16. i32 sets this to OpSize32.
  OperandSize OpSize = opSize;

  /// HasREX_WPrefix - This bit is set to true if the instruction should have
  /// the 0x40 REX prefix.  This is set for i64 types.
  bit HasREX_WPrefix = hasREX_WPrefix;
}

def invalid_node : SDNode<"<<invalid_node>>", SDTIntLeaf,[],"<<invalid_node>>">;

def Xi8  : Y86TypeInfo<i8, "b", GR8, loadi8, i8mem,
                       Imm8, i8imm, imm_su, i8imm, invalid_node,
                       0, OpSizeFixed, 0>;
def Xi16 : Y86TypeInfo<i16, "w", GR16, loadi16, i16mem,
                       Imm16, i16imm, imm_su, i16i8imm, i16immSExt8_su,
                       1, OpSize16, 0>;
def Xi32 : Y86TypeInfo<i32, "l", GR32, loadi32, i32mem,
                       Imm32, i32imm, imm_su, i32i8imm, i32immSExt8_su,
                       1, OpSize32, 0>;
/// ITy - This instruction base class takes the type info for the instruction.
/// Using this, it:
/// 1. Concatenates together the instruction mnemonic with the appropriate
///    suffix letter, a tab, and the arguments.
/// 2. Infers whether the instruction should have a 0x66 prefix byte.
/// 3. Infers whether the instruction should have a 0x40 REX_W prefix.
/// 4. Infers whether the low bit of the opcode should be 0 (for i8 operations)
///    or 1 (for i16,i32,i64 operations).
class ITy<bits<8> opcode, Format f, Y86TypeInfo typeinfo, dag outs, dag ins,
          string mnemonic, string args, list<dag> pattern>
  : I<{opcode{7}, opcode{6}, opcode{5}, opcode{4},
       opcode{3}, opcode{2}, opcode{1}, typeinfo.HasOddOpcode },
      f, outs, ins,
      !strconcat(mnemonic, "{", typeinfo.InstrSuffix, "}\t", args), pattern> {

  // Infer instruction prefixes from type info.
  let OpSize = typeinfo.OpSize;
  let hasREX_WPrefix  = typeinfo.HasREX_WPrefix;
}

// BinOpRI - Instructions like "add reg, reg, imm".
class BinOpRI<bits<8> opcode, string mnemonic, Y86TypeInfo typeinfo,
              Format f, dag outlist, Y86FoldableSchedWrite sched, list<dag> pattern>
  : ITy<opcode, f, typeinfo, outlist,
        (ins typeinfo.RegClass:$src1, typeinfo.ImmOperand:$src2),
        mnemonic, "{$src2, $src1|$src1, $src2}", pattern>,
    Sched<[sched]> {
  let ImmT = typeinfo.ImmEncoding;
}
// BinOpRR - Instructions like "add reg, reg, reg".
class BinOpRR<bits<8> opcode, string mnemonic, Y86TypeInfo typeinfo,
              dag outlist, Y86FoldableSchedWrite sched, list<dag> pattern>
  : ITy<opcode, MRMDestReg, typeinfo, outlist,
        (ins typeinfo.RegClass:$src1, typeinfo.RegClass:$src2),
        mnemonic, "{$src2, $src1|$src1, $src2}", pattern>,
    Sched<[sched]>;

// BinOpRI8 - Instructions like "add reg, reg, imm8".
class BinOpRI8<bits<8> opcode, string mnemonic, Y86TypeInfo typeinfo,
               Format f, dag outlist, Y86FoldableSchedWrite sched, list<dag> pattern>
  : ITy<opcode, f, typeinfo, outlist,
        (ins typeinfo.RegClass:$src1, typeinfo.Imm8Operand:$src2),
        mnemonic, "{$src2, $src1|$src1, $src2}", pattern>,
    Sched<[sched]> {
  let ImmT = Imm8; // Always 8-bit immediate.
}

// BinOpRR_RF - Instructions like "add reg, reg, reg", where the pattern has
// both a regclass and EFLAGS as a result.
class BinOpRR_RF<bits<8> opcode, string mnemonic, Y86TypeInfo typeinfo,
                 SDNode opnode>
  : BinOpRR<opcode, mnemonic, typeinfo, (outs typeinfo.RegClass:$dst), WriteALU,
            [(set typeinfo.RegClass:$dst, EFLAGS,
                  (opnode typeinfo.RegClass:$src1, typeinfo.RegClass:$src2))]>;

// BinOpRI_RF - Instructions like "add reg, reg, imm".
class BinOpRI_RF<bits<8> opcode, string mnemonic, Y86TypeInfo typeinfo,
                 SDNode opnode, Format f>
  : BinOpRI<opcode, mnemonic, typeinfo, f, (outs typeinfo.RegClass:$dst), WriteALU,
            [(set typeinfo.RegClass:$dst, EFLAGS,
                (opnode typeinfo.RegClass:$src1, typeinfo.ImmOperator:$src2))]>;

// BinOpRI8_RF - Instructions like "add reg, reg, imm8".
class BinOpRI8_RF<bits<8> opcode, string mnemonic, Y86TypeInfo typeinfo,
                  SDPatternOperator opnode, Format f>
  : BinOpRI8<opcode, mnemonic, typeinfo, f, (outs typeinfo.RegClass:$dst), WriteALU,
             [(set typeinfo.RegClass:$dst, EFLAGS,
               (opnode typeinfo.RegClass:$src1, typeinfo.Imm8Operator:$src2))]>;

/// ArithBinOp_RF - This is an arithmetic binary operator where the pattern is
/// defined with "(set GPR:$dst, EFLAGS, (...".
///
/// It would be nice to get rid of the second and third argument here, but
/// tblgen can't handle dependent type references aggressively enough: PR8330
multiclass ArithBinOp_RF<bits<8> BaseOpc, bits<8> BaseOpc2, bits<8> BaseOpc4,
                         string mnemonic, Format RegMRM, Format MemMRM,
                         SDNode opnodeflag, SDNode opnode,
                         bit CommutableRR, bit ConvertibleToThreeAddress,
                         bit ConvertibleToThreeAddressRR> {
  let Defs = [EFLAGS] in {
    let Constraints = "$src1 = $dst" in {
      let isCommutable = CommutableRR in {
        let isConvertibleToThreeAddress = ConvertibleToThreeAddressRR in {
          def NAME#8rr  : BinOpRR_RF<BaseOpc, mnemonic, Xi8 , opnodeflag>;
          def NAME#16rr : BinOpRR_RF<BaseOpc, mnemonic, Xi16, opnodeflag>;
          def NAME#32rr : BinOpRR_RF<BaseOpc, mnemonic, Xi32, opnodeflag>;
        } // isConvertibleToThreeAddress
      } // isCommutable

      let isConvertibleToThreeAddress = ConvertibleToThreeAddress in {
        def NAME#8ri   : BinOpRI_RF<0x80, mnemonic, Xi8 , opnodeflag, RegMRM>;

        // NOTE: These are order specific, we want the ri8 forms to be listed
        // first so that they are slightly preferred to the ri forms.
        def NAME#16ri8 : BinOpRI8_RF<0x82, mnemonic, Xi16, opnodeflag, RegMRM>;
        def NAME#32ri8 : BinOpRI8_RF<0x82, mnemonic, Xi32, opnodeflag, RegMRM>;

        def NAME#16ri  : BinOpRI_RF<0x80, mnemonic, Xi16, opnodeflag, RegMRM>;
        def NAME#32ri  : BinOpRI_RF<0x80, mnemonic, Xi32, opnodeflag, RegMRM>;
    } // Constraints = "$src1 = $dst"
  } // Defs = [EFLAGS]
}
}

defm ADD : ArithBinOp_RF<0x00, 0x02, 0x04, "add", MRM0r, MRM0m,
                         Y86add_flag, add, 1, 1, 1>;
let isCompare = 1 in {
defm SUB : ArithBinOp_RF<0x28, 0x2A, 0x2C, "sub", MRM5r, MRM5m,
                         Y86sub_flag, sub, 0, 1, 0>;
} */