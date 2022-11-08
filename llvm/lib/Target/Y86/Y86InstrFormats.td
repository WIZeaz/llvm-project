
// Format specifies the encoding used by the instruction.  This is part of the
// ad-hoc solution used to emit machine instruction encodings by our machine
// code emitter.
class Format<bits<7> val> {
  bits<7> Value = val;
}

def Pseudo        : Format<0>;
def RawFrm        : Format<1>;
def AddRegFrm     : Format<2>;
def RawFrmMemOffs : Format<3>;
def RawFrmSrc     : Format<4>;
def RawFrmDst     : Format<5>;
def RawFrmDstSrc  : Format<6>;
def RawFrmImm8    : Format<7>;
def RawFrmImm16   : Format<8>;
def AddCCFrm      : Format<9>;
def PrefixByte    : Format<10>;
def MRMr0          : Format<21>;
def MRMSrcMemFSIB  : Format<22>;
def MRMDestMemFSIB : Format<23>;
def MRMDestMem     : Format<24>;
def MRMSrcMem      : Format<25>;
def MRMSrcMem4VOp3 : Format<26>;
def MRMSrcMemOp4   : Format<27>;
def MRMSrcMemCC    : Format<28>;
def MRMXmCC: Format<30>;
def MRMXm  : Format<31>;
def MRM0m  : Format<32>;  def MRM1m  : Format<33>;  def MRM2m  : Format<34>;
def MRM3m  : Format<35>;  def MRM4m  : Format<36>;  def MRM5m  : Format<37>;
def MRM6m  : Format<38>;  def MRM7m  : Format<39>;
def MRMDestReg     : Format<40>;
def MRMSrcReg      : Format<41>;
def MRMSrcReg4VOp3 : Format<42>;
def MRMSrcRegOp4   : Format<43>;
def MRMSrcRegCC    : Format<44>;
def MRMXrCC: Format<46>;
def MRMXr  : Format<47>;
def MRM0r  : Format<48>;  def MRM1r  : Format<49>;  def MRM2r  : Format<50>;
def MRM3r  : Format<51>;  def MRM4r  : Format<52>;  def MRM5r  : Format<53>;
def MRM6r  : Format<54>;  def MRM7r  : Format<55>;
def MRM0X  : Format<56>;  def MRM1X  : Format<57>;  def MRM2X  : Format<58>;
def MRM3X  : Format<59>;  def MRM4X  : Format<60>;  def MRM5X  : Format<61>;
def MRM6X  : Format<62>;  def MRM7X  : Format<63>;
def MRM_C0 : Format<64>;  def MRM_C1 : Format<65>;  def MRM_C2 : Format<66>;
def MRM_C3 : Format<67>;  def MRM_C4 : Format<68>;  def MRM_C5 : Format<69>;
def MRM_C6 : Format<70>;  def MRM_C7 : Format<71>;  def MRM_C8 : Format<72>;
def MRM_C9 : Format<73>;  def MRM_CA : Format<74>;  def MRM_CB : Format<75>;
def MRM_CC : Format<76>;  def MRM_CD : Format<77>;  def MRM_CE : Format<78>;
def MRM_CF : Format<79>;  def MRM_D0 : Format<80>;  def MRM_D1 : Format<81>;
def MRM_D2 : Format<82>;  def MRM_D3 : Format<83>;  def MRM_D4 : Format<84>;
def MRM_D5 : Format<85>;  def MRM_D6 : Format<86>;  def MRM_D7 : Format<87>;
def MRM_D8 : Format<88>;  def MRM_D9 : Format<89>;  def MRM_DA : Format<90>;
def MRM_DB : Format<91>;  def MRM_DC : Format<92>;  def MRM_DD : Format<93>;
def MRM_DE : Format<94>;  def MRM_DF : Format<95>;  def MRM_E0 : Format<96>;
def MRM_E1 : Format<97>;  def MRM_E2 : Format<98>;  def MRM_E3 : Format<99>;
def MRM_E4 : Format<100>; def MRM_E5 : Format<101>; def MRM_E6 : Format<102>;
def MRM_E7 : Format<103>; def MRM_E8 : Format<104>; def MRM_E9 : Format<105>;
def MRM_EA : Format<106>; def MRM_EB : Format<107>; def MRM_EC : Format<108>;
def MRM_ED : Format<109>; def MRM_EE : Format<110>; def MRM_EF : Format<111>;
def MRM_F0 : Format<112>; def MRM_F1 : Format<113>; def MRM_F2 : Format<114>;
def MRM_F3 : Format<115>; def MRM_F4 : Format<116>; def MRM_F5 : Format<117>;
def MRM_F6 : Format<118>; def MRM_F7 : Format<119>; def MRM_F8 : Format<120>;
def MRM_F9 : Format<121>; def MRM_FA : Format<122>; def MRM_FB : Format<123>;
def MRM_FC : Format<124>; def MRM_FD : Format<125>; def MRM_FE : Format<126>;
def MRM_FF : Format<127>;

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

class Y86Inst<bits<8> op, Format f, ImmType i, dag outs, dag ins, string asmstr,
     list<dag> pattern=[]>: Instruction{
    let Namespace="Y86";
    bits<8> opcode=op;
    Format form=f;
    ImmType ImmT = i;
    dag OutOperandList = outs;
    dag InOperandList = ins;
    string AsmString = asmstr;
    let Pattern=pattern;
    OperandSize OpSize = OpSizeFixed; // Does this instruction's encoding change
                                      // based on operand size of the mode?
    bit hasREX_WPrefix  = 0;  // Does this inst require the REX.W prefix?
    bits<2> OpSizeBits = OpSize.Value;
}

class PseudoI<dag outs, dag ins, list<dag> pattern>:
      Y86Inst<0, Pseudo, NoImm, outs, ins, "", pattern> {
  let isCodeGenOnly = 1;
  let isPseudo = 1;
}

class I<bits<8> op, Format f, dag outs, dag ins, string asm,
        list<dag> pattern>
  : Y86Inst<op, f, NoImm, outs, ins, asm, pattern> {
  let CodeSize = 3;
}

class Ii8<bits<8> o, Format f, dag outs, dag ins, string asm,
          list<dag> pattern>
  : Y86Inst<o, f, Imm8, outs, ins, asm, pattern> {

  let CodeSize = 3;
}

class Ii16<bits<8> o, Format f, dag outs, dag ins, string asm,
           list<dag> pattern>
  : Y86Inst<o, f, Imm16, outs, ins, asm, pattern> {

  let CodeSize = 3;
}

class Ii32<bits<8> o, Format f, dag outs, dag ins, string asm,
           list<dag> pattern>
  : Y86Inst<o, f, Imm32, outs, ins, asm, pattern> {

  let CodeSize = 3;
}