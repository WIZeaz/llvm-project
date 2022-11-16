//===-- Y86ISelLowering.cpp - Y86 DAG Lowering Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Y86 uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#include "Y86ISelLowering.h"
#include "MCTargetDesc/Y86MCTargetDesc.h"
#include "Y86.h"
#include "Y86FrameLowering.h"
#include "Y86MachineFunctionInfo.h"
#include "Y86RegisterInfo.h"
#include "Y86TargetMachine.h"
#include "Y86TargetObjectFile.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/ISDOpcodes.h"

using namespace llvm;

#include "Y86GenCallingConv.inc"

#define DEBUG_TYPE "y86-isel"

Y86TargetLowering::Y86TargetLowering(const Y86TargetMachine &TM,
                                     const Y86Subtarget &STI)
    : TargetLowering(TM), Subtarget(STI) {
  addRegisterClass(MVT::i64, &Y86::GR64RegClass);
  addRegisterClass(MVT::i32, &Y86::GR32RegClass);
  // addRegisterClass(MVT::i16, &Y86::GR16RegClass);
  // addRegisterClass(MVT::i8, &Y86::GR8RegClass);
  for (MVT VT : MVT::integer_valuetypes())
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1,
                     Promote); // signextend from i1 to VT
  // setLoadExtAction(ISD::SEXTLOAD, MVT::i64, MVT::i32, Expand);

  computeRegisterProperties(Subtarget.getRegisterInfo());
}

#ifndef NDEBUG
static bool isSortedByValueNo(ArrayRef<CCValAssign> ArgLocs) {
  return llvm::is_sorted(
      ArgLocs, [](const CCValAssign &A, const CCValAssign &B) -> bool {
        return A.getValNo() < B.getValNo();
      });
}
#endif

SDValue Y86TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  Y86MachineFunctionInfo *FuncInfo = MF.getInfo<Y86MachineFunctionInfo>();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());

  // CCInfo.AllocateStack(32, Align(8));
  CCInfo.AnalyzeArguments(Ins, CC_Y86);

  // The next loop assumes that the locations are in the same order of the
  // input arguments.
  assert(isSortedByValueNo(ArgLocs) &&
         "Argument Location list must be sorted before lowering");

  SDValue ArgValue;
  for (unsigned I = 0, InsIndex = 0, E = ArgLocs.size(); I != E;
       ++I, ++InsIndex) {
    assert(InsIndex < Ins.size() && "Invalid Ins index");
    CCValAssign &VA = ArgLocs[I];
    if (VA.isRegLoc()) {
      EVT RegVT = VA.getLocVT();
      const TargetRegisterClass *RC;
      if (RegVT == MVT::i16)
        RC = &Y86::GR16RegClass;
      else if (RegVT == MVT::i32)
        RC = &Y86::GR32RegClass;
      else if (RegVT == MVT::i64)
        RC = &Y86::GR64RegClass;
      else
        llvm_unreachable("Unsupported argument type!");
      Register Reg = MF.addLiveIn(VA.getLocReg(), RC);
      ArgValue = DAG.getCopyFromReg(Chain, dl, Reg, RegVT);
      // If this is an 8 or 16-bit value, it is really passed promoted to 32
      // bits.  Insert an assert[sz]ext to capture this, then truncate to the
      // right size.
      if (VA.getLocInfo() == CCValAssign::SExt)
        ArgValue = DAG.getNode(ISD::AssertSext, dl, RegVT, ArgValue,
                               DAG.getValueType(VA.getValVT()));
      else if (VA.getLocInfo() == CCValAssign::ZExt)
        ArgValue = DAG.getNode(ISD::AssertZext, dl, RegVT, ArgValue,
                               DAG.getValueType(VA.getValVT()));
      else if (VA.getLocInfo() == CCValAssign::BCvt)
        ArgValue = DAG.getBitcast(VA.getValVT(), ArgValue);

      if (VA.isExtInLoc()) {
        assert(VA.getValVT().isVector() && "Unsupport vector type");
        ArgValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), ArgValue);
      }

    } else {
      assert(VA.isMemLoc());
      ArgValue =
          LowerMemArgument(Chain, CallConv, Ins, dl, DAG, VA, MFI, InsIndex);
    }

    // If value is passed via pointer - do a load.
    if (VA.getLocInfo() == CCValAssign::Indirect && !Ins[I].Flags.isByVal())
      ArgValue =
          DAG.getLoad(VA.getValVT(), dl, Chain, ArgValue, MachinePointerInfo());

    InVals.push_back(ArgValue);
  }

  for (unsigned I = 0, E = Ins.size(); I != E; ++I) {
    // All Y86 ABIs require that for returning structs by value we copy the
    // sret argument into %rax/%eax (depending on ABI) for the return. Save
    // the argument into a virtual register so that we can access it from the
    // return points.
    if (Ins[I].Flags.isSRet()) {
      assert(!FuncInfo->getSRetReturnReg() &&
             "SRet return has already been set");
      MVT PtrTy = getPointerTy(DAG.getDataLayout());
      Register Reg =
          MF.getRegInfo().createVirtualRegister(getRegClassFor(PtrTy));
      FuncInfo->setSRetReturnReg(Reg);
      SDValue Copy = DAG.getCopyToReg(DAG.getEntryNode(), dl, Reg, InVals[I]);
      Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, Copy, Chain);
      break;
    }
  }

  unsigned StackSize = CCInfo.getNextStackOffset();

  return Chain;
}

SDValue
Y86TargetLowering::LowerMemArgument(SDValue Chain, CallingConv::ID CallConv,
                                    const SmallVectorImpl<ISD::InputArg> &Ins,
                                    const SDLoc &dl, SelectionDAG &DAG,
                                    const CCValAssign &VA,
                                    MachineFrameInfo &MFI, unsigned i) const {
  // Create the nodes corresponding to a load from this parameter slot.
  ISD::ArgFlagsTy Flags = Ins[i].Flags;
  bool AlwaysUseMutable = false;
  bool isImmutable = !AlwaysUseMutable && !Flags.isByVal();
  EVT ValVT;
  MVT PtrVT = getPointerTy(DAG.getDataLayout());

  // If value is passed by pointer we have address passed instead of the value
  // itself. No need to extend if the mask value and location share the same
  // absolute size.
  bool ExtendedInMem =
      VA.isExtInLoc() && VA.getValVT().getScalarType() == MVT::i1 &&
      VA.getValVT().getSizeInBits() != VA.getLocVT().getSizeInBits();

  if (VA.getLocInfo() == CCValAssign::Indirect || ExtendedInMem)
    ValVT = VA.getLocVT();
  else
    ValVT = VA.getValVT();

  // FIXME: For now, all byval parameter objects are marked mutable. This can be
  // changed with more analysis.
  // In case of tail call optimization mark all arguments mutable. Since they
  // could be overwritten by lowering of arguments in case of a tail call.
  if (Flags.isByVal()) {
    unsigned Bytes = Flags.getByValSize();
    if (Bytes == 0)
      Bytes = 1; // Don't create zero-sized stack objects.

    // FIXME: For now, all byval parameter objects are marked as aliasing. This
    // can be improved with deeper analysis.
    int FI = MFI.CreateFixedObject(Bytes, VA.getLocMemOffset(), isImmutable,
                                   /*isAliased=*/true);
    return DAG.getFrameIndex(FI, PtrVT);
  }

  EVT ArgVT = Ins[i].ArgVT;

  // If this is a vector that has been split into multiple parts, and the
  // scalar size of the parts don't match the vector element size, then we can't
  // elide the copy. The parts will have padding between them instead of being
  // packed like a vector.
  bool ScalarizedAndExtendedVector =
      ArgVT.isVector() && !VA.getLocVT().isVector() &&
      VA.getLocVT().getSizeInBits() != ArgVT.getScalarSizeInBits();

  // This is an argument in memory. We might be able to perform copy elision.
  // If the argument is passed directly in memory without any extension, then we
  // can perform copy elision. Large vector types, for example, may be passed
  // indirectly by pointer.
  if (Flags.isCopyElisionCandidate() &&
      VA.getLocInfo() != CCValAssign::Indirect && !ExtendedInMem &&
      !ScalarizedAndExtendedVector) {
    SDValue PartAddr;
    if (Ins[i].PartOffset == 0) {
      // If this is a one-part value or the first part of a multi-part value,
      // create a stack object for the entire argument value type and return a
      // load from our portion of it. This assumes that if the first part of an
      // argument is in memory, the rest will also be in memory.
      int FI = MFI.CreateFixedObject(ArgVT.getStoreSize(), VA.getLocMemOffset(),
                                     /*IsImmutable=*/false);
      PartAddr = DAG.getFrameIndex(FI, PtrVT);
      return DAG.getLoad(
          ValVT, dl, Chain, PartAddr,
          MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI));
    } else {
      // This is not the first piece of an argument in memory. See if there is
      // already a fixed stack object including this offset. If so, assume it
      // was created by the PartOffset == 0 branch above and create a load from
      // the appropriate offset into it.
      int64_t PartBegin = VA.getLocMemOffset();
      int64_t PartEnd = PartBegin + ValVT.getSizeInBits() / 8;
      int FI = MFI.getObjectIndexBegin();
      for (; MFI.isFixedObjectIndex(FI); ++FI) {
        int64_t ObjBegin = MFI.getObjectOffset(FI);
        int64_t ObjEnd = ObjBegin + MFI.getObjectSize(FI);
        if (ObjBegin <= PartBegin && PartEnd <= ObjEnd)
          break;
      }
      if (MFI.isFixedObjectIndex(FI)) {
        SDValue Addr =
            DAG.getNode(ISD::ADD, dl, PtrVT, DAG.getFrameIndex(FI, PtrVT),
                        DAG.getIntPtrConstant(Ins[i].PartOffset, dl));
        return DAG.getLoad(
            ValVT, dl, Chain, Addr,
            MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI,
                                              Ins[i].PartOffset));
      }
    }
  }

  int FI = MFI.CreateFixedObject(ValVT.getSizeInBits() / 8,
                                 VA.getLocMemOffset(), isImmutable);

  // Set SExt or ZExt flag.
  if (VA.getLocInfo() == CCValAssign::ZExt) {
    MFI.setObjectZExt(FI, true);
  } else if (VA.getLocInfo() == CCValAssign::SExt) {
    MFI.setObjectSExt(FI, true);
  }

  SDValue FIN = DAG.getFrameIndex(FI, PtrVT);
  SDValue Val = DAG.getLoad(
      ValVT, dl, Chain, FIN,
      MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI));
  return ExtendedInMem
             ? (VA.getValVT().isVector()
                    ? DAG.getNode(ISD::SCALAR_TO_VECTOR, dl, VA.getValVT(), Val)
                    : DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), Val))
             : Val;
}

SDValue
Y86TargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &OutVals,
                               const SDLoc &dl, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  Y86MachineFunctionInfo *FuncInfo = MF.getInfo<Y86MachineFunctionInfo>();

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, RetCC_Y86);
  SmallVector<std::pair<Register, SDValue>, 4> RetVals;
  for (unsigned I = 0, OutsIndex = 0, E = RVLocs.size(); I != E;
       ++I, ++OutsIndex) {
    CCValAssign &VA = RVLocs[I];
    assert(VA.isRegLoc() && "Can only return in registers!");

    SDValue ValToCopy = OutVals[OutsIndex];
    EVT ValVT = ValToCopy.getValueType();

    // Promote values to the appropriate types.
    if (VA.getLocInfo() == CCValAssign::SExt)
      ValToCopy = DAG.getNode(ISD::SIGN_EXTEND, dl, VA.getLocVT(), ValToCopy);
    else if (VA.getLocInfo() == CCValAssign::ZExt)
      ValToCopy = DAG.getNode(ISD::ZERO_EXTEND, dl, VA.getLocVT(), ValToCopy);
    else if (VA.getLocInfo() == CCValAssign::AExt) {
      ValToCopy = DAG.getNode(ISD::ANY_EXTEND, dl, VA.getLocVT(), ValToCopy);
    } else if (VA.getLocInfo() == CCValAssign::BCvt)
      ValToCopy = DAG.getBitcast(VA.getLocVT(), ValToCopy);

    assert(VA.getLocInfo() != CCValAssign::FPExt &&
           "Unexpected FP-extend for return value.");

    assert(!VA.needsCustom() && "Y86 do not support any custom");
    RetVals.push_back(std::make_pair(VA.getLocReg(), ValToCopy));
  }

  SDValue Flag;
  SmallVector<SDValue, 6> RetOps;
  RetOps.push_back(Chain); // Operand #0 = Chain (updated below)
  // Operand #1 = Bytes To Pop
  RetOps.push_back(
      DAG.getTargetConstant(FuncInfo->getBytesToPopOnReturn(), dl, MVT::i32));

  // Copy the result values into the output registers.
  for (auto &RetVal : RetVals) {
    Chain = DAG.getCopyToReg(Chain, dl, RetVal.first, RetVal.second, Flag);
    Flag = Chain.getValue(1);
    RetOps.push_back(
        DAG.getRegister(RetVal.first, RetVal.second.getValueType()));
  }

  // All x86 ABIs require that for returning structs by value we copy
  // the sret argument into %rax/%eax (depending on ABI) for the return.
  // We saved the argument into a virtual register in the entry block,
  // so now we copy the value out and into %rax/%eax.
  //
  // Checking Function.hasStructRetAttr() here is insufficient because the IR
  // may not have an explicit sret argument. If FuncInfo.CanLowerReturn is
  // false, then an sret argument may be implicitly inserted in the SelDAG. In
  // either case FuncInfo->setSRetReturnReg() will have been called.
  if (Register SRetReg = FuncInfo->getSRetReturnReg()) {

    // So here, we use RetOps[0] (i.e Chain_0) for getCopyFromReg.
    SDValue Val = DAG.getCopyFromReg(RetOps[0], dl, SRetReg,
                                     getPointerTy(MF.getDataLayout()));

    Register RetValReg = Y86::EAX;
    Chain = DAG.getCopyToReg(Chain, dl, RetValReg, Val, Flag);
    Flag = Chain.getValue(1);

    // RAX/EAX now acts like a return value.
    RetOps.push_back(
        DAG.getRegister(RetValReg, getPointerTy(DAG.getDataLayout())));
  }

  const Y86RegisterInfo *TRI = Subtarget.getRegisterInfo();

  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);
  Y86ISD::NodeType opcode = Y86ISD::RET_FLAG;
  return DAG.getNode(opcode, dl, MVT::Other, RetOps);
}

SDValue Y86TargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  return SDValue();
}

const char *Y86TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case Y86ISD::CALL:
    return "Y86ISD::CALL";
  case Y86ISD::RET_FLAG:
    return "Y86ISD::RET_FLAG";
  default:
    return "Y86ISD::unknown";
  }
}