

// Return-value conventions common to all Y86 CC's.
def RetCC_Y86Common : CallingConv<[
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
def RetCC_Y86_Fast : CallingConv<[

  // For integers, ECX can be used as an extra return register
  CCIfType<[i8],  CCAssignToReg<[AL, DL, CL]>>,
  CCIfType<[i16], CCAssignToReg<[AX, DX, CX]>>,
  CCIfType<[i32], CCAssignToReg<[EAX, EDX, ECX]>>,

  // Otherwise, it is the same as the common Y86 calling convention.
  CCDelegateTo<RetCC_Y86Common>
]>;

// This is the root return-value convention for the Y86-32 backend.
def RetCC_Y86 : CallingConv<[
  // If FastCC, use RetCC_Y86_32_Fast.
  CCIfCC<"CallingConv::Fast", CCDelegateTo<RetCC_Y86_Fast>>,
  CCIfCC<"CallingConv::Tail", CCDelegateTo<RetCC_Y86_Fast>>,
  // CFGuard_Check never returns a value so does not need a RetCC.
  // CCIfCC<"CallingConv::Y86_RegCall", CCDelegateTo<RetCC_Y86_32_RegCall>>,

  // Otherwise, use RetCC_Y86_32_C.
  // CCDelegateTo<RetCC_Y86_32_C>
]>;

/// CC_Y86_32_Common - In all Y86-32 calling conventions, extra integers and FP
/// values are spilled on the stack.
def CC_Y86_Common : CallingConv<[
  // Handles byval/preallocated parameters.
  CCIfByVal<CCPassByVal<4, 4>>,
  CCIfPreallocated<CCPassByVal<4, 4>>,

  // Integer/Float values get stored in stack slots that are 4 bytes in
  // size and 4-byte aligned.
  CCIfType<[i32, f32], CCAssignToStack<4, 4>>,

]>;

/* // This is the root argument convention for the Y86-32 backend.
def CC_Y86_32 : CallingConv<[
  // Y86_INTR calling convention is valid in MCU target and should override the
  // MCU calling convention. Thus, this should be checked before isTargetMCU().
  CCIfCC<"CallingConv::Y86_INTR", CCCustom<"CC_Y86_Intr">>,
  CCIfCC<"CallingConv::Y86_FastCall", CCDelegateTo<CC_Y86_32_FastCall>>,
  CCIfCC<"CallingConv::Y86_VectorCall", CCDelegateTo<CC_Y86_Win32_VectorCall>>,
  CCIfCC<"CallingConv::Y86_ThisCall", CCDelegateTo<CC_Y86_32_ThisCall>>,
  CCIfCC<"CallingConv::CFGuard_Check", CCDelegateTo<CC_Y86_Win32_CFGuard_Check>>,
  CCIfCC<"CallingConv::Fast", CCDelegateTo<CC_Y86_32_FastCC>>,
  CCIfCC<"CallingConv::Tail", CCDelegateTo<CC_Y86_32_FastCC>>,
  CCIfCC<"CallingConv::GHC", CCDelegateTo<CC_Y86_32_GHC>>,
  CCIfCC<"CallingConv::Y86_RegCall", CCDelegateTo<CC_Y86_32_RegCall>>,

  // Otherwise, drop to normal Y86-32 CC
  CCDelegateTo<CC_Y86_32_C>
]>; */


def CSR_NoRegs : CalleeSavedRegs<(add)>;

def CSR_32 : CalleeSavedRegs<(add ESI, EDI, EBX, EBP)>;