//===-- Y86MCAsmInfo.h - Y86 asm properties --------------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the Y86MCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Y86_MCTARGETDESC_Y86MCASMINFO_H
#define LLVM_LIB_TARGET_Y86_MCTARGETDESC_Y86MCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"
namespace llvm {
class Triple;
class Y86ELFMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit Y86ELFMCAsmInfo(const Triple &Triple);
};
} // namespace llvm

#endif