//ADDri ADDrr ADDrm

multiclass ArithUnaryOp_r<bits<8> opcode_rr, MRM_Format mrm, PatFrag node, string asmname>{
  let Constraints = "$dst = $op1" , BinOP=1 in {
    def NAME#32r: I<opcode_rr, FormRI, mrm, (outs GR32:$dst), (ins GR32:$op1),
                !strconcat(asmname,"\t$op1"),[(set GR32:$dst, (node GR32:$op1))]>;
    def NAME#64r: I<opcode_rr, FormRI, mrm, (outs GR64:$dst), (ins GR64:$op1),
                !strconcat(asmname,"\t$op1"),[(set GR64:$dst, (node GR64:$op1))]>, REX_W;
  }
}

multiclass ArithBinOp_rr<bits<8> opcode_rr, SDNode node, string asmname>{
  let Constraints = "$dst = $op1" , BinOP=1 in {
    def NAME#32rr: I<opcode_rr, FormRM, MRMrr, (outs GR32:$dst), (ins GR32:$op1, GR32:$op2),
                !strconcat(asmname,"\t$op1, $op2"),[(set GR32:$dst, (node GR32:$op1, GR32:$op2))]>;
    def NAME#64rr: I<opcode_rr, FormRM, MRMrr, (outs GR64:$dst), (ins GR64:$op1, GR64:$op2),
                !strconcat(asmname,"\t$op1, $op2"),[(set GR64:$dst, (node GR64:$op1, GR64:$op2))]>, REX_W;
  }
}

multiclass ArithBinOp<bits<8> opcode_rr, bits<8> opcode_ri, MRM_Format mrm_ri, SDNode node, string asmname>{
  let Constraints = "$dst = $op1" , BinOP=1 in {
  def NAME#32rr: I<opcode_rr, FormRM, MRMrr, (outs GR32:$dst), (ins GR32:$op1, GR32:$op2),
              !strconcat(asmname,"\t$op1, $op2"),[(set GR32:$dst, (node GR32:$op1, GR32:$op2))]>;
  def NAME#64rr: I<opcode_rr, FormRM, MRMrr, (outs GR64:$dst), (ins GR64:$op1, GR64:$op2),
              !strconcat(asmname,"\t$op1, $op2"),[(set GR64:$dst, (node GR64:$op1, GR64:$op2))]>, REX_W;
  def NAME#32ri: Ii32<opcode_ri, FormMI, mrm_ri, (outs GR32:$dst), (ins GR32:$op1, i32imm:$op2),
              !strconcat(asmname,"\t$op1, $op2"),[(set GR32:$dst, (node GR32:$op1, imm:$op2))]>;
  def NAME#64ri: Ii32<opcode_ri, FormMI, mrm_ri, (outs GR64:$dst), (ins GR64:$op1, i64imm:$op2),
            !strconcat(asmname,"\t$op1, $op2"),[(set GR64:$dst, (node GR64:$op1, imm:$op2))]>, REX_W;
  // TODO: add BinOP#32rm and 16,8bits instr
  }
}

defm ADD: ArithBinOp<0x03, 0x81, MRM0r, add, "add">;
defm OR : ArithBinOp<0x0B, 0x81, MRM1r,or,"or">;
defm AND: ArithBinOp<0x23, 0x81, MRM4r, and, "and">;
defm SUB: ArithBinOp<0x2B, 0x81, MRM5r, sub, "sub">;
defm XOR: ArithBinOp<0x33, 0x81, MRM6r, and, "xor">;
defm IMUL: ArithBinOp_rr<0x0f, mul, "imul">;
defm NOT: ArithUnaryOp_r<0xf7, MRM2r, not, "not">;
