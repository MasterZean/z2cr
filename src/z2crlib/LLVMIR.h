#ifndef _z2crlib_LLVMIR_h_
#define _z2crlib_LLVMIR_h_

#include <z2crlib/ZCompiler.h>

namespace llvm {
	class LLVMContext;
	class Module;
};

class LLVMIR {
public:
	LLVMIR();
	~LLVMIR();
	
	void AddFunc(ZFunction& f);
	
	void Print();
	
private:
	llvm::LLVMContext* context = nullptr;
	std::unique_ptr<llvm::Module> moduleOwner;
	llvm::Module* mod;
	
	inline llvm::LLVMContext& C() {
		return *context;
	}
	
	inline llvm::Module& M() {
		return *mod;
	}
};

#endif
