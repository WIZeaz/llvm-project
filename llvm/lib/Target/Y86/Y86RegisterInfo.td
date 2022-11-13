//===- Y86RegisterInfo.td - Describe the Y86 Register File --*- tablegen -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the Y86 Register file, defining the registers themselves,
// aliases between the registers, and the register classes built out of the
// registers.
//
//===----------------------------------------------------------------------===//

class Y86Reg<string n, bits<16> Enc, list<Register> subregs = []> : Register<n> {
  let Namespace = "Y86";
  let HWEncoding = Enc;
  let SubRegs = subregs;
}

// Subregister indices.
let Namespace = "Y86" in {
  def sub_8bit     : SubRegIndex<8>;
  def sub_8bit_hi  : SubRegIndex<8, 8>;
  def sub_8bit_hi_phony  : SubRegIndex<8, 8>;
  def sub_16bit    : SubRegIndex<16>;
  def sub_16bit_hi : SubRegIndex<16, 16>;
  def sub_32bit    : SubRegIndex<32>;
}

//===----------------------------------------------------------------------===//
//  Register definitions...
//

// In the register alias definitions below, we define which registers alias
// which others.  We only specify which registers the small registers alias,
// because the register file generator is smart enough to figure out that
// AL aliases AX if we tell it that AX aliased AL (for example).

// Dwarf numbering is different for 32-bit and 64-bit, and there are
// variations by target as well. Currently the first entry is for Y86-64,
// second - for EH on Y86-32/Darwin and third is 'generic' one (Y86-32/Linux
// and debug information on Y86-32/Darwin)

// 8-bit registers
// Low registers
def AL : Y86Reg<"al", 0>;
def DL : Y86Reg<"dl", 2>;
def CL : Y86Reg<"cl", 1>;
def BL : Y86Reg<"bl", 3>;

// High registers. On Y86-, these cannot be used in any instruction
// with a REX prefix.
def AH : Y86Reg<"ah", 4>;
def DH : Y86Reg<"dh", 6>;
def CH : Y86Reg<"ch", 5>;
def BH : Y86Reg<"bh", 7>;

// Y86-64 only, requires REX.
let CostPerUse = [1] in {
def SIL  : Y86Reg<"sil",   6>;
def DIL  : Y86Reg<"dil",   7>;
def BPL  : Y86Reg<"bpl",   5>;
def SPL  : Y86Reg<"spl",   4>;
def R8B  : Y86Reg<"r8b",   8>;
def R9B  : Y86Reg<"r9b",   9>;
def R10B : Y86Reg<"r10b", 10>;
def R11B : Y86Reg<"r11b", 11>;
def R12B : Y86Reg<"r12b", 12>;
def R13B : Y86Reg<"r13b", 13>;
def R14B : Y86Reg<"r14b", 14>;
def R15B : Y86Reg<"r15b", 15>;
}


let isArtificial = 1 in {
// High byte of the low 16 bits of the super-register:
def SIH   : Y86Reg<"", -1>;
def DIH   : Y86Reg<"", -1>;
def BPH   : Y86Reg<"", -1>;
def SPH   : Y86Reg<"", -1>;
def R8BH  : Y86Reg<"", -1>;
def R9BH  : Y86Reg<"", -1>;
def R10BH : Y86Reg<"", -1>;
def R11BH : Y86Reg<"", -1>;
def R12BH : Y86Reg<"", -1>;
def R13BH : Y86Reg<"", -1>;
def R14BH : Y86Reg<"", -1>;
def R15BH : Y86Reg<"", -1>;

// High word of the low 32 bits of the super-register:
def HAX   : Y86Reg<"", -1>;
def HDX   : Y86Reg<"", -1>;
def HCX   : Y86Reg<"", -1>;
def HBX   : Y86Reg<"", -1>;
def HSI   : Y86Reg<"", -1>;
def HDI   : Y86Reg<"", -1>;
def HBP   : Y86Reg<"", -1>;
def HSP   : Y86Reg<"", -1>;
def HIP   : Y86Reg<"", -1>;
def R8WH  : Y86Reg<"", -1>;
def R9WH  : Y86Reg<"", -1>;
def R10WH : Y86Reg<"", -1>;
def R11WH : Y86Reg<"", -1>;
def R12WH : Y86Reg<"", -1>;
def R13WH : Y86Reg<"", -1>;
def R14WH : Y86Reg<"", -1>;
def R15WH : Y86Reg<"", -1>;
}

// 16-bit registers
let SubRegIndices = [sub_8bit, sub_8bit_hi], CoveredBySubRegs = 1 in {
def AX : Y86Reg<"ax", 0, [AL,AH]>;
def DX : Y86Reg<"dx", 2, [DL,DH]>;
def CX : Y86Reg<"cx", 1, [CL,CH]>;
def BX : Y86Reg<"bx", 3, [BL,BH]>;
}
let SubRegIndices = [sub_8bit, sub_8bit_hi_phony], CoveredBySubRegs = 1 in {
def SI : Y86Reg<"si", 6, [SIL,SIH]>;
def DI : Y86Reg<"di", 7, [DIL,DIH]>;
def BP : Y86Reg<"bp", 5, [BPL,BPH]>;
def SP : Y86Reg<"sp", 4, [SPL,SPH]>;
}

def IP : Y86Reg<"ip", 0>;

// Y86-64 only, requires REX.
let SubRegIndices = [sub_8bit, sub_8bit_hi_phony], CostPerUse = [1],
    CoveredBySubRegs = 1 in {
def R8W  : Y86Reg<"r8w",   8, [R8B,R8BH]>;
def R9W  : Y86Reg<"r9w",   9, [R9B,R9BH]>;
def R10W : Y86Reg<"r10w", 10, [R10B,R10BH]>;
def R11W : Y86Reg<"r11w", 11, [R11B,R11BH]>;
def R12W : Y86Reg<"r12w", 12, [R12B,R12BH]>;
def R13W : Y86Reg<"r13w", 13, [R13B,R13BH]>;
def R14W : Y86Reg<"r14w", 14, [R14B,R14BH]>;
def R15W : Y86Reg<"r15w", 15, [R15B,R15BH]>;
}

// def Undef: Y86Reg<"undef",9>, DwarfRegNum<[9]>;
// 32-bit registers
let SubRegIndices = [sub_16bit, sub_16bit_hi], CoveredBySubRegs = 1 in {
def EAX : Y86Reg<"eax", 0, [AX, HAX]>, DwarfRegNum<[-2, 0, 0]>;
def EDX : Y86Reg<"edx", 2, [DX, HDX]>, DwarfRegNum<[-2, 2, 2]>;
def ECX : Y86Reg<"ecx", 1, [CX, HCX]>, DwarfRegNum<[-2, 1, 1]>;
def EBX : Y86Reg<"ebx", 3, [BX, HBX]>, DwarfRegNum<[-2, 3, 3]>;
def ESI : Y86Reg<"esi", 6, [SI, HSI]>, DwarfRegNum<[-2, 6, 6]>;
def EDI : Y86Reg<"edi", 7, [DI, HDI]>, DwarfRegNum<[-2, 7, 7]>;
def EBP : Y86Reg<"ebp", 5, [BP, HBP]>, DwarfRegNum<[-2, 4, 5]>;
def ESP : Y86Reg<"esp", 4, [SP, HSP]>, DwarfRegNum<[-2, 5, 4]>;
def EIP : Y86Reg<"eip", 0, [IP, HIP]>, DwarfRegNum<[-2, 8, 8]>;
}

// Y86-64 only, requires REX
let SubRegIndices = [sub_16bit, sub_16bit_hi], CostPerUse = [1],
    CoveredBySubRegs = 1 in {
def R8D  : Y86Reg<"r8d",   8, [R8W,R8WH]>;
def R9D  : Y86Reg<"r9d",   9, [R9W,R9WH]>;
def R10D : Y86Reg<"r10d", 10, [R10W,R10WH]>;
def R11D : Y86Reg<"r11d", 11, [R11W,R11WH]>;
def R12D : Y86Reg<"r12d", 12, [R12W,R12WH]>;
def R13D : Y86Reg<"r13d", 13, [R13W,R13WH]>;
def R14D : Y86Reg<"r14d", 14, [R14W,R14WH]>;
def R15D : Y86Reg<"r15d", 15, [R15W,R15WH]>;
}

// 64-bit registers, Y86-64 only
let SubRegIndices = [sub_32bit] in {
def RAX : Y86Reg<"rax", 0, [EAX]>, DwarfRegNum<[0, -2, -2]>;
def RDX : Y86Reg<"rdx", 2, [EDX]>, DwarfRegNum<[1, -2, -2]>;
def RCX : Y86Reg<"rcx", 1, [ECX]>, DwarfRegNum<[2, -2, -2]>;
def RBX : Y86Reg<"rbx", 3, [EBX]>, DwarfRegNum<[3, -2, -2]>;
def RSI : Y86Reg<"rsi", 6, [ESI]>, DwarfRegNum<[4, -2, -2]>;
def RDI : Y86Reg<"rdi", 7, [EDI]>, DwarfRegNum<[5, -2, -2]>;
def RBP : Y86Reg<"rbp", 5, [EBP]>, DwarfRegNum<[6, -2, -2]>;
def RSP : Y86Reg<"rsp", 4, [ESP]>, DwarfRegNum<[7, -2, -2]>;

// These also require REX.
let CostPerUse = [1] in {
def R8  : Y86Reg<"r8",   8, [R8D]>,  DwarfRegNum<[ 8, -2, -2]>;
def R9  : Y86Reg<"r9",   9, [R9D]>,  DwarfRegNum<[ 9, -2, -2]>;
def R10 : Y86Reg<"r10", 10, [R10D]>, DwarfRegNum<[10, -2, -2]>;
def R11 : Y86Reg<"r11", 11, [R11D]>, DwarfRegNum<[11, -2, -2]>;
def R12 : Y86Reg<"r12", 12, [R12D]>, DwarfRegNum<[12, -2, -2]>;
def R13 : Y86Reg<"r13", 13, [R13D]>, DwarfRegNum<[13, -2, -2]>;
def R14 : Y86Reg<"r14", 14, [R14D]>, DwarfRegNum<[14, -2, -2]>;
def R15 : Y86Reg<"r15", 15, [R15D]>, DwarfRegNum<[15, -2, -2]>;
def RIP : Y86Reg<"rip",  0, [EIP]>,  DwarfRegNum<[16, -2, -2]>;
}}


// Status flags register.
//
// Note that some flags that are commonly thought of as part of the status
// flags register are modeled separately. Typically this is due to instructions
// reading and updating those flags independently of all the others. We don't
// want to create false dependencies between these instructions and so we use
// a separate register to model them.
def EFLAGS : Y86Reg<"flags", 0>;

// The direction flag.
def DF : Y86Reg<"dirflag", 0>;

// Segment registers
def CS : Y86Reg<"cs", 1>;
def DS : Y86Reg<"ds", 3>;
def SS : Y86Reg<"ss", 2>;
def ES : Y86Reg<"es", 0>;
def FS : Y86Reg<"fs", 4>;
def GS : Y86Reg<"gs", 5>;


def GR64 : RegisterClass<"Y86", [i64], 64,
                         (add RAX, RCX, RDX, RSI, RDI, R8, R9, R10, R11,
                              RBX, R14, R15, R12, R13, RBP, RSP, RIP)>;
def GR32 : RegisterClass<"Y86", [i32], 32, (add EAX, ECX, EDX, ESI, EDI, EBX, EBP, ESP,
                              R8D, R9D, R10D, R11D, R14D, R15D, R12D, R13D)>;
def GR16 : RegisterClass<"Y86", [i16], 16, (add AX, CX, DX, SI, DI, BX, BP, SP,
                              R8W, R9W, R10W, R11W, R14W, R15W, R12W, R13W)>;
def GR8  : RegisterClass<"Y86", [i8],  8,  (add AL, CL, DL, BL, SIL, DIL, BPL, SPL)>;


// GR16_NOREX - GR16 registers which do not require a REX prefix.
def GR16_NOREX : RegisterClass<"Y86", [i16], 16,
                               (add AX, CX, DX, SI, DI, BX, BP, SP)>;
// GR32_NOREX - GR32 registers which do not require a REX prefix.
def GR32_NOREX : RegisterClass<"Y86", [i32], 32,
                               (add EAX, ECX, EDX, ESI, EDI, EBX, EBP, ESP)>;
// GR64_NOREX - GR64 registers which do not require a REX prefix.
def GR64_NOREX : RegisterClass<"Y86", [i64], 64,
                            (add RAX, RCX, RDX, RSI, RDI, RBX, RBP, RSP, RIP)>;


// Control registers.
// def CONTROL_REG : RegisterClass<"Y86", [i64], 64, (sequence "CR%u", 0, 15)>;

// GR8_ABCD_L, GR8_ABCD_H, GR16_ABCD, GR32_ABCD, GR64_ABCD - Subclasses of
// GR8, GR16, GR32, and GR64 which contain just the "a" "b", "c", and "d"
// registers. On Y86-32, GR16_ABCD and GR32_ABCD are classes for registers
// that support 8-bit subreg operations. On Y86-64, GR16_ABCD, GR32_ABCD,
// and GR64_ABCD are classes for registers that support 8-bit h-register
// operations.
def GR8_ABCD_L : RegisterClass<"Y86", [i8], 8, (add AL, CL, DL, BL)>;
def GR8_ABCD_H : RegisterClass<"Y86", [i8], 8, (add AH, CH, DH, BH)>;
def GR16_ABCD : RegisterClass<"Y86", [i16], 16, (add AX, CX, DX, BX)>;
def GR32_ABCD : RegisterClass<"Y86", [i32], 32, (add EAX, ECX, EDX, EBX)>;
def GR32_TC   : RegisterClass<"Y86", [i32], 32, (add EAX, ECX, EDX, ESP)>;



// Segment registers for use by MOV instructions (and others) that have a
//   segment register as one operand.  Always contain a 16-bit segment
//   descriptor.
def SEGMENT_REG : RegisterClass<"Y86", [i16], 16, (add CS, DS, SS, ES, FS, GS)>;

// Status flags registers.
def CCR : RegisterClass<"Y86", [i32], 32, (add EFLAGS)> {
  let CopyCost = -1;  // Don't allow copying of status registers.
  let isAllocatable = 0;
}

// GR8_NOREX - GR8 registers which do not require a REX prefix.
def GR8_NOREX : RegisterClass<"Y86", [i8], 8,
                              (add AL, CL, DL, AH, CH, DH, BL, BH)> {
  let AltOrders = [(sub GR8_NOREX, AH, BH, CH, DH)];
  let AltOrderSelect = [{
    return MF.getSubtarget<Y86Subtarget>().is64Bit();
  }];
}