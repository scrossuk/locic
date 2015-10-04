#include <stdint.h>

#include <vector>

#include <llvm-abi/Type.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace CodeGen {
	
		TypePair voidTypePair(Module& module) {
			return std::make_pair(llvm_abi::Type::AutoStruct(module.abiContext(), {}), TypeGenerator(module).getVoidType());
		}
		
		TypePair boolTypePair(Module& module) {
			return std::make_pair(llvm_abi::Type::Integer(module.abiContext(), llvm_abi::Bool), TypeGenerator(module).getI1Type());
		}
		
		TypePair sizeTypePair(Module& module) {
			return std::make_pair(llvm_abi::Type::Integer(module.abiContext(), llvm_abi::SizeT), getBasicPrimitiveType(module, PrimitiveSize));
		}
		
		TypePair pointerTypePair(Module& module) {
			return std::make_pair(llvm_abi::Type::Pointer(module.abiContext()), TypeGenerator(module).getPtrType());
		}
		
		ArgInfo ArgInfo::VoidNone(Module& module) {
			return ArgInfo(module, false, false, false, false, voidTypePair(module), {});
		}
		
		ArgInfo ArgInfo::VoidContextOnly(Module& module) {
			return ArgInfo(module, false, false, true, false, voidTypePair(module), {});
		}
		
		ArgInfo ArgInfo::VoidContextWithArgs(Module& module, llvm::ArrayRef<TypePair> argumentTypes) {
			return ArgInfo(module, false, false, true, false, voidTypePair(module), argumentTypes);
		}
		
		ArgInfo ArgInfo::VoidTemplateOnly(Module& module) {
			return ArgInfo(module, false, true, false, false, voidTypePair(module), {});
		}
		
		ArgInfo ArgInfo::ContextOnly(Module& module, TypePair returnType) {
			return ArgInfo(module, false, true, false, false, returnType, {});
		}
		
		ArgInfo ArgInfo::Templated(Module& module, TypePair returnType, llvm::ArrayRef<TypePair> argumentTypes) {
			return ArgInfo(module, false, true, false, false, returnType, argumentTypes);
		}
		
		ArgInfo ArgInfo::TemplateOnly(Module& module, TypePair returnType) {
			return ArgInfo(module, false, true, false, false, returnType, {});
		}
		
		ArgInfo ArgInfo::VoidTemplateAndContext(Module& module) {
			return ArgInfo(module, false, true, true, false, voidTypePair(module), {});
		}
		
		ArgInfo ArgInfo::VoidTemplateAndContextWithArgs(Module& module, llvm::ArrayRef<TypePair> argumentTypes) {
			return ArgInfo(module, false, true, true, false, voidTypePair(module), argumentTypes);
		}
		
		ArgInfo ArgInfo::TemplateAndContext(Module& module, TypePair returnType) {
			return ArgInfo(module, false, true, true, false, returnType, {});
		}
		
		ArgInfo ArgInfo::Basic(Module& module, TypePair returnType, llvm::ArrayRef<TypePair> argumentTypes) {
			return ArgInfo(module, false, false, false, false, returnType, argumentTypes);
		}
		
		ArgInfo ArgInfo::VarArgs(Module& module, TypePair returnType, llvm::ArrayRef<TypePair> argumentTypes) {
			return ArgInfo(module, false, false, false, true, returnType, argumentTypes);
		}
		
		ArgInfo::ArgInfo(Module& module, bool hRVA, bool hTG, bool hCA, bool pIsVarArg, TypePair pReturnType, llvm::ArrayRef<TypePair> pArgumentTypes)
			: module_(&module),
			  hasReturnVarArgument_(hRVA),
			  hasTemplateGeneratorArgument_(hTG),
			  hasContextArgument_(hCA),
			  isVarArg_(pIsVarArg),
			  numStandardArguments_(pArgumentTypes.size()),
			  noMemoryAccess_(false),
			  noExcept_(false),
			  noReturn_(false),
			  returnType_(pReturnType) {
			argumentTypes_.reserve(3 + pArgumentTypes.size());
			
			if (hasReturnVarArgument_) {
				if (returnType().second->isPointerTy()) {
					argumentTypes_.push_back(std::make_pair(llvm_abi::Type::Pointer(module.abiContext()), returnType().second));
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
				argumentTypes_.push_back(argType);
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
		
		llvm::FunctionType* ArgInfo::makeFunctionType() const {
			const auto voidPair = voidTypePair(*module_);
			const auto& returnTypeRef = hasReturnVarArgument() ? voidPair : returnType();
			
			llvm_abi::FunctionType abiFunctionType;
			abiFunctionType.returnType = returnTypeRef.first;
			abiFunctionType.argTypes.reserve(argumentTypes().size());
			
			std::vector<llvm::Type*> paramTypes;
			paramTypes.reserve(argumentTypes().size());
			
			for (const auto& typePair: argumentTypes()) {
				abiFunctionType.argTypes.push_back(typePair.first);
				paramTypes.push_back(typePair.second);
			}
			
			const auto genericFunctionType = TypeGenerator(*module_).getFunctionType(returnTypeRef.second, paramTypes, isVarArg());
			return module_->abi().rewriteFunctionType(genericFunctionType, abiFunctionType);
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
		
		bool ArgInfo::noMemoryAccess() const {
			return noMemoryAccess_;
		}
		
		bool ArgInfo::noExcept() const {
			return noExcept_;
		}
		
		bool ArgInfo::noReturn() const {
			return noReturn_;
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
		
		const llvm::SmallVector<TypePair, 10>& ArgInfo::argumentTypes() const {
			return argumentTypes_;
		}
		
		std::string ArgInfo::toString() const {
			return makeString("ArgInfo(hasReturnVarArgument = %s, "
				"hasTemplateGeneratorArgument = %s, "
				"hasContextArgument = %s, "
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
				isVarArg() ? "true" : "false",
				(unsigned long long) numStandardArguments(),
				noMemoryAccess() ? "true" : "false",
				noExcept() ? "true" : "false",
				noReturn() ? "true" : "false");
		}
		
		bool canPassByValue(Module& module, const SEM::Type* type) {
			// Can only pass by value if the type's size is always known
			// (it's not enough for its size to be known in this module
			// since other modules may end up using it) and if it
			// doesn't have a custom move method (which means it
			// must stay in memory and we must hold references to it).
			TypeInfo typeInfo(module);
			return typeInfo.isSizeAlwaysKnown(type) && !typeInfo.hasCustomMove(type);
		}
		
		ArgInfo getFunctionArgInfo(Module& module, const SEM::FunctionType functionType) {
			const auto semReturnType = functionType.returnType();
			
			const auto& attributes = functionType.attributes();
			
			const bool isVarArg = attributes.isVarArg();
			const bool hasTemplateGeneratorArg = attributes.isTemplated();
			const bool hasContextArg = attributes.isMethod();
			
			const bool hasReturnVarArg = !canPassByValue(module, functionType.returnType());
			
			const auto returnType = std::make_pair(genABIArgType(module, semReturnType), genArgType(module, semReturnType));
			
			std::vector<TypePair> argTypes;
			argTypes.reserve(functionType.parameterTypes().size());
			
			for (const auto& paramType: functionType.parameterTypes()) {
				argTypes.push_back(std::make_pair(genABIArgType(module, paramType), genArgType(module, paramType)));
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

