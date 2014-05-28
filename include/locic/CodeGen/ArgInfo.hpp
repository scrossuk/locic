#ifndef LOCIC_CODEGEN_ARGINFO_HPP
#define LOCIC_CODEGEN_ARGINFO_HPP

#include <stdint.h>

#include <vector>

#include <llvm-abi/Type.hpp>

#include <locic/SEM.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
	
		class ArgInfo {
			public:
				inline static ArgInfo None() {
					return ArgInfo(false, false, false, {}, {});
				}
				
				inline static ArgInfo ContextOnly() {
					return ArgInfo(false, false, true, {}, {});
				}
				
				inline static ArgInfo TemplateOnly() {
					return ArgInfo(false, true, false, {}, {});
				}
				
				inline static ArgInfo Basic(std::vector<llvm_abi::Type> standardArguments, const std::vector<llvm::Type*>& argTypes) {
					return ArgInfo(false, false, false, std::move(standardArguments), argTypes);
				}
				
				inline ArgInfo(bool hRVA, bool hTG, bool hCA, std::vector<llvm_abi::Type> standardArguments, const std::vector<llvm::Type*>& argTypes)
					: hasReturnVarArgument_(hRVA),
					  hasTemplateGeneratorArgument_(hTG),
					  hasContextArgument_(hCA),
					  numStandardArguments_(standardArguments.size()) {
						if (hasReturnVarArgument_) {
							abiTypes_.push_back(llvm_abi::Type::Pointer());
							abiLLVMTypes_.push_back(nullptr);
						}
						
						if (hasTemplateGeneratorArgument_) {
							abiTypes_.push_back(templateGeneratorABIType());
							abiLLVMTypes_.push_back(nullptr);
						}
						
						if (hasContextArgument_) {
							abiTypes_.push_back(llvm_abi::Type::Pointer());
							abiLLVMTypes_.push_back(nullptr);
						}
						
						size_t i = 0;
						for (auto& abiType: standardArguments) {
							abiTypes_.push_back(std::move(abiType));
							abiLLVMTypes_.push_back(argTypes.at(i++));
						}
					}
				
				ArgInfo(ArgInfo&&) = default;
				ArgInfo& operator=(ArgInfo&&) = default;
					  
				bool hasReturnVarArgument() const {
					return hasReturnVarArgument_;
				}
				
				bool hasTemplateGeneratorArgument() const {
					return hasTemplateGeneratorArgument_;
				}
				
				bool hasContextArgument() const {
					return hasContextArgument_;
				}
				
				size_t returnVarArgumentOffset() const {
					return 0;
				}
				
				size_t templateGeneratorArgumentOffset() const {
					return hasReturnVarArgument() ? 1 : 0;
				}
				
				size_t contextArgumentOffset() const {
					return templateGeneratorArgumentOffset() + (hasTemplateGeneratorArgument() ? 1 : 0);
				}
				
				size_t standardArgumentOffset() const {
					return contextArgumentOffset() + (hasContextArgument() ? 1 : 0);
				}
				
				size_t numStandardArguments() const {
					return numStandardArguments_;
				}
				
				size_t numArguments() const {
					return standardArgumentOffset() + numStandardArguments();
				}
				
				const std::vector<llvm_abi::Type>& abiTypes() const {
					return abiTypes_;
				}
				
				const std::vector<llvm::Type*>& abiLLVMTypes() const {
					return abiLLVMTypes_;
				}
				
			private:
				// Non-copyable.
				ArgInfo(const ArgInfo&) = delete;
				ArgInfo& operator=(const ArgInfo&) = delete;
				
				bool hasReturnVarArgument_;
				bool hasTemplateGeneratorArgument_;
				bool hasContextArgument_;
				size_t numStandardArguments_;
				std::vector<llvm_abi::Type> abiTypes_;
				std::vector<llvm::Type*> abiLLVMTypes_;
				
		};
		
		inline ArgInfo getArgInfo(Module& module, SEM::TypeInstance* typeInstance, SEM::Function* function) {
			const bool hasReturnVarArg = !isTypeSizeAlwaysKnown(module, function->type()->getFunctionReturnType());
			const bool hasTemplateGeneratorArg = (typeInstance != nullptr && !typeInstance->templateVariables().empty());
			const bool hasContextArg = function->isMethod() && !function->isStaticMethod();
			
			std::vector<llvm_abi::Type> abiArgTypes;
			std::vector<llvm::Type*> abiLLVMArgTypes;
			for (const auto paramType:  function->type()->getFunctionParameterTypes()) {
				abiArgTypes.push_back(genABIType(module, paramType));
				abiLLVMArgTypes.push_back(genType(module, paramType));
			}
			
			return ArgInfo(hasReturnVarArg, hasTemplateGeneratorArg, hasContextArg, std::move(abiArgTypes), abiLLVMArgTypes);
		}
		
	}
	
}

#endif
