#include <z2crlib/LLVMIR.h>

#if defined(_RELEASE) && defined(flagLLVM) && defined(_MSC_VER)

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
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
#include "llvm/IR/Argument.h"
#include "llvm/IR/Instructions.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"

#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

using namespace llvm;

LLVMIR::LLVMIR() {
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	
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
	
	bool retadded = false;
	while (n) {
		if (n->NT == NodeType::Const) {
			int val = ((ConstNode*)n)->IntVal;
			
			llvm::Value *One = builder.getInt64(val);
			auto* A = builder.CreateAlloca(Type::getInt32Ty(C()), nullptr, "dummy2");
			builder.CreateStore(One, A);
			
			builder.CreateRetVoid();
			retadded = true;
			
			return;
		}
		else if (n->NT == NodeType::Return) {
			builder.CreateRetVoid();
			retadded = true;
		}
		
		n = n->Next;
	}
	
	if (retadded == false)
		builder.CreateRetVoid();
	
	if (llvm::verifyFunction(*Add1F, &outs())) {
		errs() << f.BackName.ToStd() << ": Function verification failed!\n";
	}
}

void LLVMIR::Print() {
	ExecutionEngine* EE = EngineBuilder(std::move(moduleOwner)).create();
	
	if (verifyModule(M(), &llvm::errs())) {
	  //errs() << "Error constructing module!\n";
	}
	
	outs() << "We just constructed this LLVM module:\n\n" << M();
}

#else

#endif