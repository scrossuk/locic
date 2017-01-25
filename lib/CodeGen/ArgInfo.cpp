#include <stdint.h>

#include <vector>

#include <llvm-abi/FunctionType.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/AST/Type.hpp>
#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace CodeGen {
		
		ArgInfo ArgInfo::VoidNone(Module& module) {
			return ArgInfo(module, false, false, false, false, llvm_abi::VoidTy, {});
		}
		
		ArgInfo ArgInfo::VoidContextOnly(Module& module) {
			return ArgInfo(module, false, false, true, false, llvm_abi::VoidTy, {});
		}
		
		ArgInfo ArgInfo::VoidContextWithArgs(Module& module, llvm::ArrayRef<llvm_abi::Type> argumentTypes) {
			return ArgInfo(module, false, false, true, false, llvm_abi::VoidTy, argumentTypes);
		}
		
		ArgInfo ArgInfo::VoidTemplateOnly(Module& module) {
			return ArgInfo(module, false, true, false, false, llvm_abi::VoidTy, {});
		}
		
		ArgInfo ArgInfo::ContextOnly(Module& module, llvm_abi::Type returnType) {
			return ArgInfo(module, false, true, false, false, returnType, {});
		}
		
		ArgInfo ArgInfo::Templated(Module& module, llvm_abi::Type returnType, llvm::ArrayRef<llvm_abi::Type> argumentTypes) {
			return ArgInfo(module, false, true, false, false, returnType, argumentTypes);
		}
		
		ArgInfo ArgInfo::TemplateOnly(Module& module, llvm_abi::Type returnType) {
			return ArgInfo(module, false, true, false, false, returnType, {});
		}
		
		ArgInfo ArgInfo::VoidTemplateAndContext(Module& module) {
			return ArgInfo(module, false, true, true, false, llvm_abi::VoidTy, {});
		}
		
		ArgInfo ArgInfo::VoidTemplateAndContextWithArgs(Module& module, llvm::ArrayRef<llvm_abi::Type> argumentTypes) {
			return ArgInfo(module, false, true, true, false, llvm_abi::VoidTy, argumentTypes);
		}
		
		ArgInfo ArgInfo::TemplateAndContext(Module& module, llvm_abi::Type returnType) {
			return ArgInfo(module, false, true, true, false, returnType, {});
		}
		
		ArgInfo ArgInfo::VoidBasic(Module& module, llvm::ArrayRef<llvm_abi::Type> argumentTypes) {
			return ArgInfo(module, false, false, false, false, llvm_abi::VoidTy, argumentTypes);
		}
		
		ArgInfo ArgInfo::Basic(Module& module, llvm_abi::Type returnType, llvm::ArrayRef<llvm_abi::Type> argumentTypes) {
			return ArgInfo(module, false, false, false, false, returnType, argumentTypes);
		}
		
		ArgInfo ArgInfo::VoidVarArgs(Module& module, llvm::ArrayRef<llvm_abi::Type> argumentTypes) {
			return ArgInfo(module, false, false, false, false, llvm_abi::VoidTy, argumentTypes);
		}
		
		ArgInfo ArgInfo::VarArgs(Module& module, llvm_abi::Type returnType, llvm::ArrayRef<llvm_abi::Type> argumentTypes) {
			return ArgInfo(module, false, false, false, true, returnType, argumentTypes);
		}
		
		ArgInfo::ArgInfo(Module& module, bool hRVA, bool hTG, bool hCA, bool pIsVarArg, llvm_abi::Type pReturnType, llvm::ArrayRef<llvm_abi::Type> pArgumentTypes)
			: module_(&module),
			  hasReturnVarArgument_(hRVA),
			  hasTemplateGeneratorArgument_(hTG),
			  hasContextArgument_(hCA),
			  hasNestArgument_(false),
			  isVarArg_(pIsVarArg),
			  noMemoryAccess_(false),
			  noExcept_(false),
			  noReturn_(false),
			  numStandardArguments_(pArgumentTypes.size()),
			  returnType_(pReturnType) {
			argumentTypes_.reserve(3 + pArgumentTypes.size());
			
			if (hasReturnVarArgument_) {
				argumentTypes_.push_back(llvm_abi::PointerTy);
			}
			
			if (isVarArg_ && hasTemplateGeneratorArgument_) {
				argumentTypes_.push_back(templateGeneratorType(module));
			}
			
			if (hasContextArgument_) {
				argumentTypes_.push_back(llvm_abi::PointerTy);
			}
			
			for (auto& argType: pArgumentTypes) {
				argumentTypes_.push_back(argType);
			}
			
			if (!isVarArg_ && hasTemplateGeneratorArgument_) {
				argumentTypes_.push_back(templateGeneratorType(module));
			}
		}
		
		ArgInfo ArgInfo::withNoMemoryAccess() const {
			ArgInfo copy(*this);
			copy.noMemoryAccess_ = true;
			return copy;
		}
		
		ArgInfo ArgInfo::withNoExcept() const {
			ArgInfo copy(*this);
			copy.noExcept_ = true;
			return copy;
		}
		
		ArgInfo ArgInfo::withNoReturn() const {
			ArgInfo copy(*this);
			copy.noReturn_ = true;
			return copy;
		}
		
		ArgInfo ArgInfo::withNestArgument() const {
			if (hasNestArgument()) {
				return *this;
			}
			
			ArgInfo copy(*this);
			copy.hasNestArgument_ = true;
			copy.argumentTypes_.insert(copy.argumentTypes_.begin(),
			                           llvm_abi::PointerTy);
			return copy;
		}
		
		llvm_abi::FunctionType ArgInfo::getABIFunctionType() const {
			const auto& returnTypeRef =
				hasReturnVarArgument() ? llvm_abi::VoidTy : returnType();
			return llvm_abi::FunctionType(llvm_abi::CC_CDefault,
			                              returnTypeRef,
			                              argumentTypes(),
			                              isVarArg());
		}
		
		llvm::FunctionType* ArgInfo::makeFunctionType() const {
			return module_->abi().getFunctionType(getABIFunctionType());
		}
		
		bool ArgInfo::hasReturnVarArgument() const {
			return hasReturnVarArgument_;
		}
		
		bool ArgInfo::hasTemplateGeneratorArgument() const {
			return hasTemplateGeneratorArgument_;
		}
		
		bool ArgInfo::hasContextArgument() const {
			return hasContextArgument_;
		}
		
		bool ArgInfo::hasNestArgument() const {
			return hasNestArgument_;
		}
		
		bool ArgInfo::isVarArg() const {
			return isVarArg_;
		}
		
		bool ArgInfo::noMemoryAccess() const {
			return noMemoryAccess_;
		}
		
		bool ArgInfo::noExcept() const {
			return noExcept_;
		}
		
		bool ArgInfo::noReturn() const {
			return noReturn_;
		}
		
		size_t ArgInfo::numStandardArguments() const {
			return numStandardArguments_;
		}
		
		ArgOffsets ArgInfo::argumentOffsets() const {
			ArgOffsets argOffsets;
			
			size_t offset = 0;
			if (hasNestArgument()) {
				argOffsets.nestArgumentOffset = offset++;
			}
			if (hasReturnVarArgument()) {
				argOffsets.returnVarArgumentOffset = offset++;
			}
			if (isVarArg() && hasTemplateGeneratorArgument()) {
				// For varargs functions we pass the template
				// generator before the arguments, since the
				// callee may not know the argument count at
				// the time it queries template arguments.
				argOffsets.templateGeneratorArgumentOffset = offset++;
			}
			if (hasContextArgument()) {
				argOffsets.contextArgumentOffset = offset++;
			}
			
			argOffsets.standardArgumentOffset = offset;
			offset += numStandardArguments();
			
			if (!isVarArg() && hasTemplateGeneratorArgument()) {
				// For non-varargs functions we pass the
				// template generator after the arguments, since
				// this allows it to be efficiently discarded in
				// virtual method calls with a non-templated
				// callee.
				argOffsets.templateGeneratorArgumentOffset = offset++;
			}
			
			argOffsets.numArguments = offset;
			
			return argOffsets;
		}
		
		size_t ArgInfo::nestArgumentOffset() const {
			return argumentOffsets().nestArgumentOffset;
		}
		
		size_t ArgInfo::returnVarArgumentOffset() const {
			return argumentOffsets().returnVarArgumentOffset;
		}
		
		size_t ArgInfo::templateGeneratorArgumentOffset() const {
			return argumentOffsets().templateGeneratorArgumentOffset;
		}
		
		size_t ArgInfo::contextArgumentOffset() const {
			return argumentOffsets().contextArgumentOffset;
		}
		
		size_t ArgInfo::standardArgumentOffset() const {
			return argumentOffsets().standardArgumentOffset;
		}
		
		size_t ArgInfo::numArguments() const {
			return argumentOffsets().numArguments;
		}
		
		const llvm_abi::Type& ArgInfo::returnType() const {
			return returnType_;
		}
		
		const llvm::SmallVector<llvm_abi::Type, 10>& ArgInfo::argumentTypes() const {
			return argumentTypes_;
		}
		
		std::string ArgInfo::toString() const {
			return makeString("ArgInfo(hasReturnVarArgument = %s, "
				"hasTemplateGeneratorArgument = %s, "
				"hasContextArgument = %s, "
				"hasNestArgument = %s, "
				"isVarArg = %s, "
				"numStandardArguments = %llu, "
				"noMemoryAccess = %s, "
				"noExcept = %s, "
				"noReturn = %s, "
				"returnType = TODO, "
				"argumentTypes = TODO)",
				hasReturnVarArgument() ? "true" : "false",
				hasTemplateGeneratorArgument() ? "true" : "false",
				hasContextArgument() ? "true" : "false",
				hasNestArgument() ? "true" : "false",
				isVarArg() ? "true" : "false",
				(unsigned long long) numStandardArguments(),
				noMemoryAccess() ? "true" : "false",
				noExcept() ? "true" : "false",
				noReturn() ? "true" : "false");
		}
		
		ArgInfo getFunctionArgInfo(Module& module, const AST::FunctionType functionType) {
			const auto astReturnType = functionType.returnType();
			
			const auto& attributes = functionType.attributes();
			
			const bool isVarArg = attributes.isVarArg();
			const bool hasTemplateGeneratorArg = attributes.isTemplated();
			const bool hasContextArg = attributes.isMethod();
			
			const bool hasReturnVarArg = !TypeInfo(module).isPassedByValue(astReturnType);
			const auto returnType =
				hasReturnVarArg ? llvm_abi::VoidTy : genABIType(module, astReturnType);
			
			std::vector<llvm_abi::Type> argTypes;
			argTypes.reserve(functionType.parameterTypes().size());
			
			for (const auto& paramType: functionType.parameterTypes()) {
				argTypes.push_back(genABIArgType(module, paramType));
			}
			
			auto argInfo = ArgInfo(module, hasReturnVarArg, hasTemplateGeneratorArg, hasContextArg, isVarArg, returnType, argTypes);
			
			// Some functions will only be noexcept in certain cases (e.g.
			// when they have a noexcept predicate that queries whether a
			// templated type has a method that is marked noexcept) but for
			// CodeGen purposes we're looking for a guarantee of noexcept
			// in all cases, hence we look for always-true noexcept predicates.
			if (!attributes.noExceptPredicate().isTrivialBool()) {
				assert(!attributes.noExceptPredicate().dependsOnOnly({}));
			}
			
			if (attributes.noExceptPredicate().isTrue()) {
				argInfo = argInfo.withNoExcept();
			}
			
			return argInfo;
		}
		
	}
	
}

