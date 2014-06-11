#include <stdint.h>

#include <vector>

#include <llvm-abi/Type.hpp>

#include <locic/SEM.hpp>
#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
	
		TypePair voidTypePair(Module& module) {
			return std::make_pair(llvm_abi::Type::AutoStruct({}), TypeGenerator(module).getVoidType());
		}
		
		TypePair sizeTypePair(Module& module) {
			return std::make_pair(llvm_abi::Type::Integer(llvm_abi::SizeT), getNamedPrimitiveType(module, "size_t"));
		}
		
		TypePair pointerTypePair(Module& module) {
			return std::make_pair(llvm_abi::Type::Pointer(), TypeGenerator(module).getI8PtrType());
		}
		
		ArgInfo ArgInfo::VoidNone(Module& module) {
			return ArgInfo(module, false, false, false, false, voidTypePair(module), {});
		}
		
		ArgInfo ArgInfo::VoidContextOnly(Module& module) {
			return ArgInfo(module, false, false, true, false, voidTypePair(module), {});
		}
		
		ArgInfo ArgInfo::VoidTemplateOnly(Module& module) {
			return ArgInfo(module, false, true, false, false, voidTypePair(module), {});
		}
		
		ArgInfo ArgInfo::Templated(Module& module, TypePair returnType, std::vector<TypePair> argumentTypes) {
			return ArgInfo(module, false, true, false, false, std::move(returnType), std::move(argumentTypes));
		}
		
		ArgInfo ArgInfo::TemplateOnly(Module& module, TypePair returnType) {
			return ArgInfo(module, false, true, false, false, std::move(returnType), {});
		}
		
		ArgInfo ArgInfo::VoidTemplateAndContext(Module& module) {
			return ArgInfo(module, false, true, true, false, voidTypePair(module), {});
		}
		
		ArgInfo ArgInfo::TemplateAndContext(Module& module, TypePair returnType) {
			return ArgInfo(module, false, true, true, false, std::move(returnType), {});
		}
		
		ArgInfo ArgInfo::Basic(Module& module, TypePair returnType, std::vector<TypePair> argumentTypes) {
			return ArgInfo(module, false, false, false, false, std::move(returnType), std::move(argumentTypes));
		}
		
		ArgInfo::ArgInfo(Module& module, bool hRVA, bool hTG, bool hCA, bool pIsVarArg, TypePair pReturnType, std::vector<TypePair> pArgumentTypes)
			: module_(module),
			  hasReturnVarArgument_(hRVA),
			  hasTemplateGeneratorArgument_(hTG),
			  hasContextArgument_(hCA),
			  isVarArg_(pIsVarArg),
			  numStandardArguments_(pArgumentTypes.size()),
			  returnType_(std::move(pReturnType)) {
			if (hasReturnVarArgument_) {
				if (returnType().second->isPointerTy()) {
					argumentTypes_.push_back(std::make_pair(llvm_abi::Type::Pointer(), returnType().second));
				} else {
					argumentTypes_.push_back(pointerTypePair(module));
				}
			}
			
			if (hasTemplateGeneratorArgument_) {
				argumentTypes_.push_back(templateGeneratorType(module));
			}
			
			if (hasContextArgument_) {
				argumentTypes_.push_back(pointerTypePair(module));
			}
			
			for (auto& argType: pArgumentTypes) {
				argumentTypes_.push_back(std::move(argType));
			}
		}
		
		llvm::FunctionType* ArgInfo::makeFunctionType() const {
			const auto voidPair = voidTypePair(module_);
			const auto& returnTypeRef = hasReturnVarArgument() ? voidPair : returnType();
			
			llvm_abi::FunctionType abiFunctionType;
			abiFunctionType.returnType = returnTypeRef.first.copy();
			
			std::vector<llvm::Type*> paramTypes;
			
			for (const auto& typePair: argumentTypes()) {
				abiFunctionType.argTypes.push_back(typePair.first.copy());
				paramTypes.push_back(typePair.second);
			}
			
			const auto genericFunctionType = TypeGenerator(module_).getFunctionType(returnTypeRef.second, paramTypes, isVarArg());
			return module_.abi().rewriteFunctionType(genericFunctionType, abiFunctionType);
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
		
		bool ArgInfo::isVarArg() const {
			return isVarArg_;
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
		
		const TypePair& ArgInfo::returnType() const {
			return returnType_;
		}
		
		const std::vector<TypePair>& ArgInfo::argumentTypes() const {
			return argumentTypes_;
		}
		
		ArgInfo getFunctionArgInfo(Module& module, SEM::Type* functionType) {
			assert(functionType->isFunction());
			
			const auto semReturnType = functionType->getFunctionReturnType();
			
			const bool isVarArg = functionType->isFunctionVarArg();
			const bool hasReturnVarArg = !isTypeSizeAlwaysKnown(module, semReturnType);
			const bool hasTemplateGeneratorArg = functionType->isFunctionTemplatedMethod();
			const bool hasContextArg = functionType->isFunctionMethod();
			
			TypePair returnType = std::make_pair(genABIArgType(module, semReturnType), genArgType(module, semReturnType));
			
			std::vector<TypePair> argTypes;
			
			for (const auto paramType: functionType->getFunctionParameterTypes()) {
				argTypes.push_back(std::make_pair(genABIArgType(module, paramType), genArgType(module, paramType)));
			}
			
			return ArgInfo(module, hasReturnVarArg, hasTemplateGeneratorArg, hasContextArg, isVarArg, std::move(returnType), std::move(argTypes));
		}
		
		ArgInfo getTemplateVarFunctionStubArgInfo(Module& module, SEM::Function* function) {
			const auto functionType = function->type();
			const auto semReturnType = functionType->getFunctionReturnType();
			
			const bool isVarArg = functionType->isFunctionVarArg();
			const bool hasReturnVarArg = !isTypeSizeAlwaysKnown(module, semReturnType);
			const bool hasTemplateGeneratorArg = true;
			const bool hasContextArg = functionType->isFunctionMethod();
			
			TypePair returnType = std::make_pair(genABIArgType(module, semReturnType), genArgType(module, semReturnType));
			
			std::vector<TypePair> argTypes;
			
			for (const auto paramType: functionType->getFunctionParameterTypes()) {
				argTypes.push_back(std::make_pair(genABIArgType(module, paramType), genArgType(module, paramType)));
			}
			
			return ArgInfo(module, hasReturnVarArg, hasTemplateGeneratorArg, hasContextArg, isVarArg, std::move(returnType), std::move(argTypes));
		}
		
	}
	
}

