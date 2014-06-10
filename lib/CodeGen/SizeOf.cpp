#include <locic/SEM.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>
#include <locic/CodeGen/VirtualCall.hpp>

namespace locic {

	namespace CodeGen {
		
		ArgInfo alignMaskArgInfo(Module& module, SEM::TypeInstance* typeInstance) {
			return !typeInstance->templateVariables().empty() ?
				ArgInfo::TemplateOnly(module, sizeTypePair(module)) :
				ArgInfo::Basic(module, sizeTypePair(module), {});
		}
		
		ArgInfo sizeOfArgInfo(Module& module, SEM::TypeInstance* typeInstance) {
			return !typeInstance->templateVariables().empty() ?
				ArgInfo::TemplateOnly(module, sizeTypePair(module)) :
				ArgInfo::Basic(module, sizeTypePair(module), {});
		}
		
		ArgInfo memberOffsetArgInfo(Module& module, SEM::TypeInstance* typeInstance) {
			std::vector<TypePair> argTypes;
			argTypes.push_back(sizeTypePair(module));
			
			return !typeInstance->templateVariables().empty() ?
				ArgInfo::Templated(module, sizeTypePair(module), std::move(argTypes)) :
				ArgInfo::Basic(module, sizeTypePair(module), std::move(argTypes));
		}
		
		llvm::Function* genAlignMaskFunction(Module& module, SEM::TypeInstance* typeInstance) {
			const auto mangledName = mangleMethodName(typeInstance, "__alignmask");
			const auto result = module.getFunctionMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			const auto argInfo = alignMaskArgInfo(module, typeInstance);
			const auto llvmFunction = createLLVMFunction(module, argInfo.makeFunctionType(), getFunctionLinkage(typeInstance, typeInstance->moduleScope()), mangledName);
			llvmFunction->setDoesNotAccessMemory();
			
			module.getFunctionMap().insert(mangledName, llvmFunction);
			
			assert(!typeInstance->isInterface());
			
			// For class declarations, the alignof() function
			// will be implemented in another module.
			if (typeInstance->isClassDecl()) return llvmFunction;
			
			// Always inline this function.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			if (typeInstance->isPrimitive()) {
				createPrimitiveAlignOf(module, typeInstance, *llvmFunction);
				return llvmFunction;
			}
			
			Function function(module, *llvmFunction, argInfo, &(module.typeTemplateBuilder(typeInstance)));
			
			const auto zero = ConstantGenerator(module).getSizeTValue(0);
			
			if (typeInstance->isUnionDatatype()) {
				// Calculate maximum alignment mask of all variants,
				// which is just a matter of OR-ing them together
				// (the tag byte has an alignment of 1 and hence an
				// alignment mask of 0).
				llvm::Value* maxVariantAlignMask = zero;
				
				for (const auto variantTypeInstance: typeInstance->variants()) {
					const auto variantAlignMask = genAlignMask(function, variantTypeInstance->selfType());
					maxVariantAlignMask = function.getBuilder().CreateOr(maxVariantAlignMask, variantAlignMask);
				}
				
				function.getBuilder().CreateRet(maxVariantAlignMask);
			} else {
				// Calculate maximum alignment mask of all variables,
				// which is just a matter of OR-ing them together.
				llvm::Value* classAlignMask = zero;
				
				for (const auto& var: typeInstance->variables()) {
					const auto varAlignMask = genAlignMask(function, var->type());
					classAlignMask = function.getBuilder().CreateOr(classAlignMask, varAlignMask);
				}
				
				function.getBuilder().CreateRet(classAlignMask);
			}
			
			function.verify();
			
			return llvmFunction;
		}
		
		llvm::Value* genAlignOf(Function& function, SEM::Type* type) {
			const auto alignMask = genAlignMask(function, type);
			const auto name = makeString("alignof__%s", type->isObject() ? type->getObjectType()->name().last().c_str() : "");
			return function.getBuilder().CreateAdd(alignMask, ConstantGenerator(function.module()).getSizeTValue(1), name);
		}
		
		llvm::Value* genAlignMask(Function& function, SEM::Type* type) {
			SetUseEntryBuilder setUseEntryBuilder(function);
			
			auto& module = function.module();
			auto& abi = module.abi();
			
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					// Subtract 1 because this is producing a mask.
					return ConstantGenerator(module).getSizeTValue(abi.typeAlign(genABIType(module, type)) - 1);
				}
				
				case SEM::Type::OBJECT: {
					if (isTypeSizeKnownInThisModule(module, type)) {
						return ConstantGenerator(module).getSizeTValue(abi.typeAlign(genABIType(module, type)) - 1);
					}
					
					const auto callName = makeString("alignmask__%s", type->getObjectType()->name().last().c_str());
					const bool canThrow = false;
					const auto alignMaskFunction = genAlignMaskFunction(module, type->getObjectType());
					
					const bool hasTemplate = !type->templateArguments().empty();
					const auto args = hasTemplate ? std::vector<llvm::Value*> { computeTemplateGenerator(function, type) } : std::vector<llvm::Value*>{};
					const auto callResult = genRawFunctionCall(function, alignMaskArgInfo(module, type->getObjectType()), canThrow, alignMaskFunction, args);
					callResult->setName(callName);
					return callResult;
				}
				
				case SEM::Type::TEMPLATEVAR: {
					const auto typeInfo = function.getBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
					return VirtualCall::generateCountFnCall(function, typeInfo, VirtualCall::ALIGNOF);
				}
				
				default: {
					llvm_unreachable("Unknown type enum for generating alignmask.");
				}
			}
		}
		
		llvm::Function* genSizeOfFunction(Module& module, SEM::TypeInstance* typeInstance) {
			const auto mangledName = mangleMethodName(typeInstance, "__sizeof");
			const auto result = module.getFunctionMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			const auto argInfo = sizeOfArgInfo(module, typeInstance);
			const auto llvmFunction = createLLVMFunction(module, argInfo.makeFunctionType(), getFunctionLinkage(typeInstance, typeInstance->moduleScope()), mangledName);
			llvmFunction->setDoesNotAccessMemory();
			
			module.getFunctionMap().insert(mangledName, llvmFunction);
			
			assert(!typeInstance->isInterface());
			
			// For class declarations, the sizeof() function
			// will be implemented in another module.
			if (typeInstance->isClassDecl()) return llvmFunction;
			
			// Always inline this function.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			// Primitives have known sizes.
			if (typeInstance->isPrimitive()) {
				createPrimitiveSizeOf(module, typeInstance, *llvmFunction);
				return llvmFunction;
			}
			
			// Since the member variables are known, generate
			// the contents of the sizeof() function to sum
			// their sizes.
			Function function(module, *llvmFunction, argInfo, &(module.typeTemplateBuilder(typeInstance)));
			
			const auto zero = ConstantGenerator(module).getSizeTValue(0);
			const auto one = ConstantGenerator(module).getSizeTValue(1);
			
			if (typeInstance->isUnionDatatype()) {
				// Calculate maximum alignment and size of all variants.
				llvm::Value* maxVariantAlignMask = zero;
				llvm::Value* maxVariantSize = zero;
				
				for (const auto variantTypeInstance: typeInstance->variants()) {
					const auto variantAlignMask = genAlignMask(function, variantTypeInstance->selfType());
					const auto variantSize = genSizeOf(function, variantTypeInstance->selfType());
					
					maxVariantAlignMask = function.getBuilder().CreateOr(maxVariantAlignMask, variantAlignMask);
					
					const auto compareResult = function.getBuilder().CreateICmpUGT(variantSize, maxVariantSize);
					maxVariantSize = function.getBuilder().CreateSelect(compareResult, variantSize, maxVariantSize);
				}
				
				// Add one byte for the tag.
				llvm::Value* classSize = one;
				
				// Align for most alignment variant type.
				classSize = makeAligned(function, classSize, maxVariantAlignMask);
				
				classSize = function.getBuilder().CreateAdd(classSize, maxVariantSize);
				
				function.getBuilder().CreateRet(makeAligned(function, classSize, maxVariantAlignMask));
			} else {
				// Add up all member variable sizes.
				llvm::Value* classSize = zero;
				
				// Also need to calculate class alignment so the
				// correct amount of padding is added at the end.
				llvm::Value* classAlignMask = zero;
				
				for (const auto& var: typeInstance->variables()) {
					const auto memberAlignMask = genAlignMask(function, var->type());
					
					classAlignMask = function.getBuilder().CreateOr(classAlignMask, memberAlignMask);
					
					classSize = makeAligned(function, classSize, memberAlignMask);
					classSize = function.getBuilder().CreateAdd(classSize, genSizeOf(function, var->type()));
				}
				
				// Class sizes must be at least one byte.
				const auto isZero = function.getBuilder().CreateICmpEQ(classSize, zero);
				classSize = function.getBuilder().CreateSelect(isZero, one, classSize);
				
				function.getBuilder().CreateRet(makeAligned(function, classSize, classAlignMask));
			}
			
			function.verify();
			
			return llvmFunction;
		}
		
		llvm::Value* genSizeOfValue(Function& function, SEM::Type* type) {
			SetUseEntryBuilder setUseEntryBuilder(function);
			
			auto& module = function.module();
			auto& abi = module.abi();
			
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					return ConstantGenerator(module).getSizeTValue(abi.typeSize(genABIType(module, type)));
				}
				
				case SEM::Type::OBJECT: {
					if (isTypeSizeKnownInThisModule(module, type)) {
						return ConstantGenerator(module).getSizeTValue(abi.typeSize(genABIType(module, type)));
					}
					
					const auto callName = makeString("sizeof__%s", type->getObjectType()->name().last().c_str());
					const bool canThrow = false;
					const auto sizeOfFunction = genSizeOfFunction(module, type->getObjectType());
					
					const bool hasTemplate = !type->templateArguments().empty();
					const auto args = hasTemplate ? std::vector<llvm::Value*> { computeTemplateGenerator(function, type) } : std::vector<llvm::Value*>{};
					const auto callResult = genRawFunctionCall(function, sizeOfArgInfo(module, type->getObjectType()), canThrow, sizeOfFunction, args);
					callResult->setName(callName);
					return callResult;
				}
				
				case SEM::Type::TEMPLATEVAR: {
					const auto typeInfo = function.getBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
					return VirtualCall::generateCountFnCall(function, typeInfo, VirtualCall::SIZEOF);
				}
				
				default: {
					llvm_unreachable("Unknown type enum for generating sizeof.");
				}
			}
		}
		
		llvm::Value* genSizeOf(Function& function, SEM::Type* type) {
			auto& sizeOfMap = function.getSizeOfMap();
			const auto it = sizeOfMap.find(type);
			if (it != sizeOfMap.end()) {
				return it->second;
			}
			
			const auto sizeOfValue = genSizeOfValue(function, type);
			sizeOfMap.insert(std::make_pair(type, sizeOfValue));
			return sizeOfValue;
		}
		
		llvm::Value* makeAligned(Function& function, llvm::Value* size, llvm::Value* alignMask) {
			const auto sizePlusMask = function.getBuilder().CreateAdd(size, alignMask, "sizePlusMask");
			const auto inverseMask = function.getBuilder().CreateNot(alignMask, "inverseMask");
			return function.getBuilder().CreateAnd(sizePlusMask, inverseMask, "alignedValue");
		}
		
		/**
		 * \brief Generate member offset function.
		 * 
		 * Generates code like the following:
		 * 
		 * size_t memberOffset(size_t memberIndex) {
		 *     size_t offset = 0;
		 *     for (size_t i = 0; i < memberIndex; i++) {
		 *         offset = makeAligned(offset, memberAlignMask(i));
		 *         offset += memberSize(i);
		 *     }
		 *     return makeAligned(offset, memberAlignMask(memberIndex));
		 * }
		 */
		llvm::Value* genMemberOffsetFunction(Module& module, SEM::TypeInstance* typeInstance) {
			const auto mangledName = mangleMethodName(typeInstance, "__memberoffset");
			const auto result = module.getFunctionMap().tryGet(mangledName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			const auto argInfo = memberOffsetArgInfo(module, typeInstance);
			const auto llvmFunction = createLLVMFunction(module, argInfo.makeFunctionType(), getFunctionLinkage(typeInstance, typeInstance->moduleScope()), mangledName);
			llvmFunction->setDoesNotAccessMemory();
			llvmFunction->setDoesNotThrow();
			
			module.getFunctionMap().insert(mangledName, llvmFunction);
			
			// Always inline this function.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			assert(!typeInstance->isInterface() && !typeInstance->isClassDecl() && !typeInstance->isPrimitive());
			
			std::vector<llvm_abi::Type> abiTypes;
			abiTypes.push_back(llvm_abi::Type::Integer(llvm_abi::SizeT));
			
			Function function(module, *llvmFunction, argInfo, &(module.typeTemplateBuilder(typeInstance)));
			
			const auto& typeVars = typeInstance->variables();
			
			// Add the sizes of all the previous member variables
			// to the offset.
			llvm::Value* offsetValue = ConstantGenerator(module).getSizeTValue(0);
			const auto memberIndexValue = function.getArg(0);
			
			for (size_t i = 0; i < typeVars.size(); i++) {
				const auto& var = typeVars.at(i);
				
				offsetValue = makeAligned(function, offsetValue, genAlignMask(function, var->type()));
				
				const auto exitBB = function.createBasicBlock("exit");
				const auto nextBB = function.createBasicBlock("next");
				
				const auto varIndexValue = ConstantGenerator(module).getSizeTValue(i);
				const auto compareResult = function.getBuilder().CreateICmpEQ(memberIndexValue, varIndexValue);
				function.getBuilder().CreateCondBr(compareResult, exitBB, nextBB);
				
				function.selectBasicBlock(exitBB);
				function.getBuilder().CreateRet(offsetValue);
				
				function.selectBasicBlock(nextBB);
				
				if (i != typeVars.size() - 1) {
					offsetValue = function.getBuilder().CreateAdd(offsetValue, genSizeOf(function, var->type()));
				}
			}
			
			function.getBuilder().CreateUnreachable();
			
			function.verify();
			
			return llvmFunction;
		}
		
		llvm::Value* genMemberOffset(Function& function, SEM::Type* type, size_t memberIndex) {
			assert(type->isObject());
			
			if (memberIndex == 0) {
				return ConstantGenerator(function.module()).getSizeTValue(0);
			}
			
			SetUseEntryBuilder setUseEntryBuilder(function);
			
			const auto offsetPair = std::make_pair(type, memberIndex);
			
			auto& memberOffsetMap = function.getMemberOffsetMap();
			const auto it = memberOffsetMap.find(offsetPair);
			if (it != memberOffsetMap.end()) {
				return it->second;
			}
			
			auto& module = function.module();
			
			const auto callName = makeString("memberoffset_%llu__%s", (unsigned long long) memberIndex,
				type->getObjectType()->name().last().c_str());
			
			const auto memberIndexValue = ConstantGenerator(module).getSizeTValue(memberIndex);
			
			const auto memberOffsetFunction = genMemberOffsetFunction(module, type->getObjectType());
			
			const bool hasTemplate = !type->templateArguments().empty();
			const auto args = hasTemplate ? std::vector<llvm::Value*> { computeTemplateGenerator(function, type), memberIndexValue } : std::vector<llvm::Value*>{ memberIndexValue };
			const bool canThrow = false;
			const auto callResult = genRawFunctionCall(function, memberOffsetArgInfo(module, type->getObjectType()), canThrow, memberOffsetFunction, args);
			callResult->setName(callName);
			
			// TODO: add these to ArgInfo:
			// callInst->setDoesNotAccessMemory();
			// callInst->setDoesNotThrow();
			
			memberOffsetMap.insert(std::make_pair(offsetPair, callResult));
			
			return callResult;
		}
		
	}
	
}

