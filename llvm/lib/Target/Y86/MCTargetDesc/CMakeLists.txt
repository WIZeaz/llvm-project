add_llvm_component_library(LLVMY86Desc
  Y86MCTargetDesc.cpp

  LINK_COMPONENTS
  MC
  MCDisassembler
  Support
  Y86Info
  BinaryFormat

  ADD_TO_COMPONENT
  Y86
  )
