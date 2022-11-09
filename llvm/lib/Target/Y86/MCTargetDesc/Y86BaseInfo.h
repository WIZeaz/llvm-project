//===-- Y86BaseInfo.h - Top level definitions for Y86 -------- --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains small standalone helper functions and enum definitions for
// the Y86 target useful for the compiler back-end and the MC libraries.
// As such, it deliberately does not include references to LLVM core
// code gen types, passes, etc..
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_Y86_MCTARGETDESC_Y86BASEINFO_H
#define LLVM_LIB_TARGET_Y86_MCTARGETDESC_Y86BASEINFO_H

#include "MCTargetDesc/Y86MCTargetDesc.h"
#include "llvm/Support/Debug.h"

namespace llvm {
namespace Y86II {
    enum Format{
        Pseudo ,
        FormO  ,
        FormRI ,
        FormRM ,
        FormMR ,
        FormMI ,
        FormOr ,
    };

    enum MRMFormat{
        MRM0r, MRM1r, MRM2r, MRM3r, MRM4r, MRM5r, MRM6r, MRM7r,
        MRM0m, MRM1m, MRM2m, MRM3m, MRM4m, MRM5m, MRM6m, MRM7m,
        MRMrr, 
        MRMrm,
        NoMRM
    };

    enum ImmType{
        NoImm      ,
        Imm8       ,
        Imm8PCRel  ,
        Imm8Reg    ,
        Imm16      ,
        Imm16PCRel ,
        Imm32      ,
        Imm32PCRel ,
        Imm32S     ,
        Imm64      ,
    };

    constexpr uint64_t getMask(uint8_t size){
        return (1<<size)-1;
    }

    inline uint64_t getFormat(uint64_t TSFlags){
        return TSFlags & getMask(4);
    }

    inline uint64_t getMRMFormat(uint64_t TSFlags){
        return (TSFlags>>4) & getMask(7);
    }

    inline uint64_t getImmType(uint64_t TSFlags){
        return (TSFlags>>11) & getMask(4);
    }

    inline uint64_t getOpcode(uint64_t TSFlags){
        return (TSFlags>>15) & getMask(8);
    }

    inline bool hasREX_W(uint64_t TSFlags){
        return (TSFlags>>23) & 1;
    }

    inline bool isPseudo(uint64_t TSFlags) {
        return getFormat(TSFlags) == Pseudo;
    }

    inline bool shouldAddReg(uint64_t TSFlags){
        uint64_t FormBits=getFormat(TSFlags);
      return FormBits == FormOr;
    }

    inline bool isFormat(uint64_t TSFlags, Y86II::Format form) {
        return getFormat(TSFlags) == form;
    }

    inline bool hasMem(uint64_t TSFlags){
        uint64_t MRMFormBits=getMRMFormat(TSFlags);
        switch (MRMFormBits){
            case MRM0m:
            case MRM1m:
            case MRM2m:
            case MRM3m:
            case MRM4m:
            case MRM5m:
            case MRM6m:
            case MRM7m:
            case MRMrm:
            return true;
            default:
            return false;
        }
    }

    inline uint8_t getExtOpcode(uint64_t TSFlags){
        uint64_t MRMFormBits=getMRMFormat(TSFlags);
        switch (MRMFormBits){
            case MRM0m: case MRM0r: return 0;
            case MRM1m: case MRM1r: return 1;
            case MRM2m: case MRM2r: return 2;
            case MRM3m: case MRM3r: return 3;
            case MRM4m: case MRM4r: return 4;
            case MRM5m: case MRM5r: return 5;
            case MRM6m: case MRM6r: return 6;
            case MRM7m: case MRM7r: return 7;
            default: llvm_unreachable("no ext opcode");
        }
    }
}

} // namespace llvm

#endif