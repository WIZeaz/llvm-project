def Xi8  : Y86TypeInfo<i8, "b", GR8, loadi8, i8mem,
                       Imm8, i8imm, imm_su, i8imm, invalid_node,
                       0, OpSizeFixed, 0>;
def Xi16 : Y86TypeInfo<i16, "w", GR16, loadi16, i16mem,
                       Imm16, i16imm, imm_su, i16i8imm, i16immSExt8_su,
                       1, OpSize16, 0>;
def Xi32 : Y86TypeInfo<i32, "l", GR32, loadi32, i32mem,
                       Imm32, i32imm, imm_su, i32i8imm, i32immSExt8_su,
                       1, OpSize32, 0>;
def Xi64 : Y86TypeInfo<i64, "q", GR64, loadi64, i64mem,
                       Imm32S, i64i32imm, i64immSExt32_su, i64i8imm, i64immSExt8_su,
                       1, OpSizeFixed, 1>;
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

defm ADD : ArithBinOp_RF<0x00, 0x02, 0x04, "add", MRM0r, MRM0m,
                         Y86add_flag, add, 1, 1, 1>;
let isCompare = 1 in {
defm SUB : ArithBinOp_RF<0x28, 0x2A, 0x2C, "sub", MRM5r, MRM5m,
                         Y86sub_flag, sub, 0, 1, 0>;
}
