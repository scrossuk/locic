#include <stdint.h>

#include <vector>

#include <llvm-abi/Type.hpp>

#include <locic/SEM.hpp>
#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
	
		ArgInfo ArgInfo::None(Module& module) {
			return ArgInfo(module, false, false, false, {}, {});
		}
		
		ArgInfo ArgInfo::ContextOnly(Module& module) {
			return ArgInfo(module, false, false, true, {}, {});
		}
		
		ArgInfo ArgInfo::TemplateOnly(Module& module) {
			return ArgInfo(module, false, true, false, {}, {});
		}
		
		ArgInfo ArgInfo::TemplateAndContext(Module& module) {
			return ArgInfo(module, false, true, true, {}, {});
		}
		
		ArgInfo ArgInfo::Basic(Module& module, std::vector<llvm_abi::Type> standardArguments, const std::vector<llvm::Type*>& argTypes) {
			return ArgInfo(module, false, false, false, std::move(standardArguments), argTypes);
		}
		
		ArgInfo::ArgInfo(Module& module, bool hRVA, bool hTG, bool hCA, std::vector<llvm_abi::Type> standardArguments, const std::vector<llvm::Type*>& argTypes)
			: hasReturnVarArgument_(hRVA),
			  hasTemplateGeneratorArgument_(hTG),
			  hasContextArgument_(hCA),
			  numStandardArguments_(standardArguments.size()) {
			if (hasReturnVarArgument_) {
				abiTypes_.push_back(llvm_abi::Type::Pointer());
				abiLLVMTypes_.push_back(TypeGenerator(module).getI8PtrType());
			}
			
			if (hasTemplateGeneratorArgument_) {
				abiTypes_.push_back(templateGeneratorABIType());
				abiLLVMTypes_.push_back(templateGeneratorType(module));
			}
			
			if (hasContextArgument_) {
				abiTypes_.push_back(llvm_abi::Type::Pointer());
				abiLLVMTypes_.push_back(TypeGenerator(module).getI8PtrType());
			}
			
			size_t i = 0;
			
			for (auto& abiType : standardArguments) {
				abiTypes_.push_back(std::move(abiType));
				abiLLVMTypes_.push_back(argTypes.at(i++));
			}
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
		
		size_t ArgInfo::returnVarArgumentOffset() const {
			return 0;
		}
		
		size_t ArgInfo::templateGeneratorArgumentOffset() const {
			return hasReturnVarArgument() ? 1 : 0;
		}
		
		size_t ArgInfo::contextArgumentOffset() const {
			return templateGeneratorArgumentOffset() + (hasTemplateGeneratorArgument() ? 1 : 0);
		}
		
		size_t ArgInfo::standardArgumentOffset() const {
			return contextArgumentOffset() + (hasContextArgument() ? 1 : 0);
		}
		
		size_t ArgInfo::numStandardArguments() const {
			return numStandardArguments_;
		}
		
		size_t ArgInfo::numArguments() const {
			return standardArgumentOffset() + numStandardArguments();
		}
		
		const std::vector<llvm_abi::Type>& ArgInfo::abiTypes() const {
			return abiTypes_;
		}
		
		const std::vector<llvm::Type*>& ArgInfo::abiLLVMTypes() const {
			return abiLLVMTypes_;
		}
		
		ArgInfo getFunctionArgInfo(Module& module, SEM::Type* functionType) {
			assert(functionType->isFunction());
			const bool hasReturnVarArg = !isTypeSizeAlwaysKnown(module, functionType->getFunctionReturnType());
			const bool hasTemplateGeneratorArg = functionType->isFunctionTemplatedMethod();
			const bool hasContextArg = functionType->isFunctionMethod();
			
			std::vector<llvm_abi::Type> abiArgTypes;
			std::vector<llvm::Type*> abiLLVMArgTypes;
			
			for (const auto paramType: functionType->getFunctionParameterTypes()) {
				abiArgTypes.push_back(genABIArgType(module, paramType));
				abiLLVMArgTypes.push_back(genArgType(module, paramType));
			}
			
			return ArgInfo(module, hasReturnVarArg, hasTemplateGeneratorArg, hasContextArg, std::move(abiArgTypes), abiLLVMArgTypes);
		}
		
		ArgInfo getTemplateVarFunctionStubArgInfo(Module& module, SEM::Function* function) {
			const bool hasReturnVarArg = !isTypeSizeAlwaysKnown(module, function->type()->getFunctionReturnType());
			const bool hasTemplateGeneratorArg = true;
			const bool hasContextArg = function->isMethod() && !function->isStaticMethod();
			
			std::vector<llvm_abi::Type> abiArgTypes;
			std::vector<llvm::Type*> abiLLVMArgTypes;
			
			for (const auto paramType :  function->type()->getFunctionParameterTypes()) {
				abiArgTypes.push_back(genABIArgType(module, paramType));
				abiLLVMArgTypes.push_back(genArgType(module, paramType));
			}
			
			return ArgInfo(module, hasReturnVarArg, hasTemplateGeneratorArg, hasContextArg, std::move(abiArgTypes), abiLLVMArgTypes);
		}
		
	}
	
}

