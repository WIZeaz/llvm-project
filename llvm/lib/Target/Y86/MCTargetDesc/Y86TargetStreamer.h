//===- Y86TargetStreamer.h ------------------------------*- C++ -*---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Y86_MCTARGETDESC_Y86TARGETSTREAMER_H
#define LLVM_LIB_TARGET_Y86_MCTARGETDESC_Y86TARGETSTREAMER_H

#include "llvm/MC/MCStreamer.h"

namespace llvm {

/// Y86 target streamer implementing Y86-only assembly directives.
class Y86TargetStreamer : public MCTargetStreamer {
public:
  Y86TargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}
};

} // end namespace llvm

#endif
