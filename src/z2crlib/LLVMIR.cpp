#include <z2crlib/LLVMIR.h>

#ifdef _RELEASE

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

using namespace llvm;

LLVMIR::LLVMIR() {
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	
	//LLVMInitializeNativeAsmPrinter();
	
	context = new llvm::LLVMContext();
	
	// Create some module to put our function into it.
	moduleOwner = std::make_unique<Module>("test", C());
	mod = moduleOwner.get();
}

LLVMIR::~LLVMIR() {
	delete context;
	llvm_shutdown();
}

void LLVMIR::AddFunc(ZFunction& f) {
	llvm::Function *Add1F =
	    llvm::Function::Create(FunctionType::get(Type::getVoidTy(C()), false),
	                     llvm::Function::ExternalLinkage, f.BackName.ToStd(), M());
	                     
	BasicBlock *BB = BasicBlock::Create(C(), "entry", Add1F);
	
	IRBuilder<> builder(BB);
	
	Node* n = f.Nodes.First;
	
	while (n) {
		if (n->NT == NodeType::Const) {
			int val = ((ConstNode*)n)->IntVal;
			
			llvm::Value *One = builder.getInt32(val);
			auto* A = builder.CreateAlloca(Type::getInt32Ty(C()), nullptr, "dummy");
			builder.CreateStore(One, A);
			
			builder.CreateRetVoid();
			
			return;
		}
		n = n->Next;
	}
}

void LLVMIR::Print() {
	ExecutionEngine* EE = EngineBuilder(std::move(moduleOwner)).create();
	
	if (!verifyModule(M())) {
	  errs() << "Error constructing module!\n";
	}
	outs() << "We just constructed this LLVM module:\n\n" << M();
}

#else

#endif