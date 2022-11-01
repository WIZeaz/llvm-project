//===-- Y86TargetObjectFile.h - Y86 Object Info -----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Y86_Y86TARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_Y86_Y86TARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

  /// This implementation is used for Y86 ELF targets that don't
  /// have a further specialization.
  class Y86ELFTargetObjectFile : public TargetLoweringObjectFileELF {
  public:
    Y86ELFTargetObjectFile() {
      //PLTRelativeVariantKind = MCSymbolRefExpr::VK_PLT;
    }
    /// Describe a TLS variable address within debug info.
    const MCExpr *getDebugThreadLocalSymbol(const MCSymbol *Sym) const override;
  };

} // end namespace llvm

#endif
