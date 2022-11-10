
// Format specifies the encoding used by the instruction.  This is part of the
// ad-hoc solution used to emit machine instruction encodings by our machine
// code emitter.
class Format<bits<4> val> {
  bits<4> Value = val;
}
class MRM_Format<bits<7> val>{
  bits<7> Value = val;
}
def Pseudo         : Format<0>;
def FormO          : Format<1>;
def FormRI         : Format<2>;
def FormRM         : Format<3>; 
def FormMR         : Format<4>;
def FormMI         : Format<5>;
def FormOr         : Format<6>;

def MRM0r : MRM_Format<0>; def MRM1r : MRM_Format<1>; def MRM2r : MRM_Format<2>; def MRM3r : MRM_Format<3>; 
def MRM4r : MRM_Format<4>; def MRM5r : MRM_Format<5>; def MRM6r : MRM_Format<6>; def MRM7r : MRM_Format<7>;

def MRM0m : MRM_Format<8>;  def MRM1m : MRM_Format<9>; def MRM2m : MRM_Format<10>; def MRM3m : MRM_Format<11>; 
def MRM4m : MRM_Format<12>; def MRM5m : MRM_Format<13>; def MRM6m : MRM_Format<14>; def MRM7m : MRM_Format<15>;

def MRMrr : MRM_Format<16>;
def MRMrm : MRM_Format<17>;
def NoMRM : MRM_Format<18>;

// ImmType - This specifies the immediate type used by an instruction. This is
// part of the ad-hoc solution used to emit machine instruction encodings by our
// machine code emitter.
class ImmType<bits<4> val> {
  bits<4> Value = val;
}
def NoImm      : ImmType<0>;
def Imm8       : ImmType<1>;
def Imm8PCRel  : ImmType<2>;
def Imm8Reg    : ImmType<3>; // Register encoded in [7:4].
def Imm16      : ImmType<4>;
def Imm16PCRel : ImmType<5>;
def Imm32      : ImmType<6>;
def Imm32PCRel : ImmType<7>;
def Imm32S     : ImmType<8>;
def Imm64      : ImmType<9>;

// Operand size for encodings that change based on mode.
class OperandSize<bits<2> val> {
  bits<2> Value = val;
}
def OpSizeFixed  : OperandSize<0>; // Never needs a 0x66 prefix.
def OpSize16     : OperandSize<1>; // Needs 0x66 prefix in 32-bit mode.
def OpSize32     : OperandSize<2>; // Needs 0x66 prefix in 16-bit mode.

// Prefix byte classes which are used to indicate to the ad-hoc machine code
// emitter that various prefix bytes are required.
class OpSize16 { OperandSize OpSize = OpSize16; }
class OpSize32 { OperandSize OpSize = OpSize32; }

class REX_W  { bit HasREX_WPrefix = 1; }
class BinOP  { bit BinOP = 1; }

class Y86Inst<bits<8> op, Format f, MRM_Format mrm, ImmType i, dag outs, dag ins, string asmstr,
     list<dag> pattern=[]>: Instruction{
    let Namespace="Y86";
    bits<8> Opcode=op;
    Format Form=f;
    MRM_Format MRMForm=mrm;
    ImmType ImmTy=i;
    dag OutOperandList = outs;
    dag InOperandList = ins;
    string AsmString = asmstr;
    let Pattern=pattern;
    bit HasREX_WPrefix = 0; 
    bit BinOP=0;
    OperandSize OpSize = OpSizeFixed; // Does this instruction's encoding change
                                      // based on operand size of the mode?
    bits<2> OpSizeBits = OpSize.Value;

    // set TSFlags
    let TSFlags{3-0}=Form.Value;
    let TSFlags{10-4}=MRMForm.Value;
    let TSFlags{14-11}=ImmTy.Value;
    let TSFlags{22-15}=Opcode;
    let TSFlags{23-23}=HasREX_WPrefix;
    let TSFlags{24-24}=BinOP;
}

class PseudoI<dag outs, dag ins, list<dag> pattern>:
      Y86Inst<0, Pseudo, NoMRM, NoImm, outs, ins, "", pattern> {
  let isCodeGenOnly = 1;
  let isPseudo = 1;
}

class I<bits<8> op, Format f, MRM_Format mrm, dag outs, dag ins, string asm,
        list<dag> pattern>
  : Y86Inst<op, f, mrm, NoImm, outs, ins, asm, pattern> {
  let CodeSize = 3;
}

class Ii8<bits<8> o, Format f, MRM_Format mrm, dag outs, dag ins, string asm,
          list<dag> pattern>
  : Y86Inst<o, f, mrm, Imm8, outs, ins, asm, pattern> {

  let CodeSize = 3;
}

class Ii16<bits<8> o, Format f, MRM_Format mrm, dag outs, dag ins, string asm,
           list<dag> pattern>
  : Y86Inst<o, f, mrm, Imm16, outs, ins, asm, pattern> {

  let CodeSize = 3;
}

class Ii32<bits<8> o, Format f, MRM_Format mrm, dag outs, dag ins, string asm,
           list<dag> pattern>
  : Y86Inst<o, f, mrm, Imm32, outs, ins, asm, pattern> {

  let CodeSize = 3;
}