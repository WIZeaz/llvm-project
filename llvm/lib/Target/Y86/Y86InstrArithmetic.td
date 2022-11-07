//ADDri ADDrr ADDrm
let Constraints = "$src = $dst"{
def ADD8ri: Ii32<0x02, FormRI, MRMrm, Imm8, (outs GR8:$dst), (ins GR8:$src, imm8:$imm),
                  "add\t$src, $imm",[(set GR)]>;

}