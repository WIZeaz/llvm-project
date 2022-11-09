

// Return-value conventions common to all Y86 CC's.
def RetCC_Y86Common: CallingConv<[
  // Scalar values are returned in AX first, then DX.  For i8, the ABI
  // requires the values to be in AL and AH, however this code uses AL and DL
  // instead. This is because using AH for the second register conflicts with
  // the way LLVM does multiple return values -- a return of {i16,i8} would end
  // up in AX and AH, which overlap. Front-ends wishing to conform to the ABI
  // for functions that return two i8 values are currently expected to pack the
  // values into an i16 (which uses AX, and thus AL:AH).
  //
  // For code that doesn't care about the ABI, we allow returning more than two
  // integer values in registers.
  CCIfType<[v1i1],  CCPromoteToType<i8>>,
  CCIfType<[i1],  CCPromoteToType<i8>>,
  CCIfType<[i8] , CCAssignToReg<[AL, DL, CL]>>,
  CCIfType<[i16], CCAssignToReg<[AX, DX, CX]>>,
  CCIfType<[i32], CCAssignToReg<[EAX, EDX, ECX]>>
]>;

// Y86-32 FastCC return-value convention.
/* def RetCC_Y86_Fast : CallingConv<[

  // For integers, ECX can be used as an extra return register
  CCIfType<[i8],  CCAssignToReg<[AL, DL, CL]>>,
  CCIfType<[i16], CCAssignToReg<[AX, DX, CX]>>,
  CCIfType<[i32], CCAssignToReg<[EAX, EDX, ECX]>>,

  // Otherwise, it is the same as the common Y86 calling convention.
  CCDelegateTo<RetCC_Y86Common>
]>;
 */
// This is the root return-value convention for the Y86-32 backend.
def RetCC_Y86 : CallingConv<[
  CCDelegateTo<RetCC_Y86Common>
]>;


/// CC_Y86_32_Common - In all Y86-32 calling conventions, extra integers and FP
/// values are spilled on the stack.
def CC_Y86_Common : CallingConv<[
  // Handles byval parameters.
  CCIfByVal<CCPassByVal<8, 8>>,

  // Promote i1/i8/i16/v1i1 arguments to i32.
  CCIfType<[i1, i8, i16, v1i1], CCPromoteToType<i32>>,

  // Pointers are always passed in full 64-bit registers.
  //CCIfPtr<CCCustom<"CC_X86_64_Pointer">>,

  // The first 6 integer arguments are passed in integer registers.
  CCIfType<[i32], CCAssignToReg<[EDI, ESI, EDX, ECX/* , R8D, R9D */]>>,

  // Integer/FP values get stored in stack slots that are 8 bytes in size and
  // 8-byte aligned if there are no more registers to hold them.
  CCIfType<[i32, i64, f16, f32, f64], CCAssignToStack<8, 8>>,

]>;

// X86-64 C Calling Conv
def CC_Y86 : CallingConv<[
  // Promote i1/i8/i16/v1i1 arguments to i32.
  CCIfType<[i1, i8, i16, v1i1], CCPromoteToType<i32>>,

  // The 'nest' parameter, if any, is passed in ECX.
  CCIfNest<CCAssignToReg<[ECX]>>,

  // The first 3 integer arguments, if marked 'inreg' and if the call is not
  // a vararg call, are passed in integer registers.
  CCIfNotVarArg<CCIfInReg<CCIfType<[i32], CCAssignToReg<[EAX, EDX, ECX]>>>>,

  // Otherwise, same as everything else.
  CCDelegateTo<CC_Y86_Common>
]>;

def CSR_NoRegs : CalleeSavedRegs<(add)>;

def CSR_32 : CalleeSavedRegs<(add ESI, EDI, EBX, EBP)>;