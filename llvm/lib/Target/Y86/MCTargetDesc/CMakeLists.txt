add_llvm_component_library(LLVMY86Desc
  Y86MCTargetDesc.cpp
  Y86MCCodeEmitter.cpp
  Y86MCAsmInfo.cpp
  Y86AsmBackend.cpp
  Y86ELFObjectWriter.cpp

  LINK_COMPONENTS
  MC
  MCDisassembler
  Support
  Y86Info
  BinaryFormat

  ADD_TO_COMPONENT
  Y86
  )
