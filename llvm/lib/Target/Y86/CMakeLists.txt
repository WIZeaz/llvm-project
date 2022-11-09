add_llvm_component_group(Y86)

set(LLVM_TARGET_DEFINITIONS Y86.td)

#tablegen(LLVM Y86GenAsmMatcher.inc -gen-asm-matcher)
#tablegen(LLVM Y86GenAsmWriter.inc -gen-asm-writer)
#tablegen(LLVM Y86GenCompressInstEmitter.inc -gen-compress-inst-emitter)
tablegen(LLVM Y86GenDAGISel.inc -gen-dag-isel)
tablegen(LLVM Y86GenCallingConv.inc -gen-callingconv)
#tablegen(LLVM Y86GenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM Y86GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM Y86GenRegisterInfo.inc -gen-register-info)
#tablegen(LLVM Y86GenSearchableTables.inc -gen-searchable-tables)
tablegen(LLVM Y86GenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(Y86CommonTableGen)

add_llvm_target(Y86CodeGen
  Y86ISelLowering.cpp
  Y86ISelDAGToDAG.cpp
  Y86TargetMachine.cpp
  Y86Subtarget.cpp
  Y86InstrInfo.cpp
  Y86FrameLowering.cpp
  Y86RegisterInfo.cpp
  Y86TargetObjectFile.cpp
  Y86AsmPrinter.cpp
  Y86MCInstLower.cpp
  Y86MachineFunctionInfo.cpp
  

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  Core
  CodeGen
  MC
  Y86Desc
  Y86Info
  SelectionDAG
  Support
  Target
  TransformUtils
  GlobalISel

  ADD_TO_COMPONENT
  Y86
  )

#add_subdirectory(AsmParser)
#add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
