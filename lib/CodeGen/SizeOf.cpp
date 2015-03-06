#include <locic/SEM.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
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
		
		ArgInfo alignMaskArgInfo(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto argInfo = !typeInstance->templateVariables().empty() ?
				ArgInfo::TemplateOnly(module, sizeTypePair(module)) :
				ArgInfo::Basic(module, sizeTypePair(module), {});
			return argInfo.withNoMemoryAccess().withNoExcept();
		}
		
		ArgInfo sizeOfArgInfo(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto argInfo = !typeInstance->templateVariables().empty() ?
				ArgInfo::TemplateOnly(module, sizeTypePair(module)) :
				ArgInfo::Basic(module, sizeTypePair(module), {});
			return argInfo.withNoMemoryAccess().withNoExcept();
		}
		
		ArgInfo memberOffsetArgInfo(Module& module, const SEM::TypeInstance* typeInstance) {
			std::vector<TypePair> argTypes;
			argTypes.push_back(sizeTypePair(module));
			
			const auto argInfo = !typeInstance->templateVariables().empty() ?
				ArgInfo::Templated(module, sizeTypePair(module), std::move(argTypes)) :
				ArgInfo::Basic(module, sizeTypePair(module), std::move(argTypes));
			return argInfo.withNoMemoryAccess().withNoExcept();
		}
		
		llvm::Function* genAlignMaskFunction(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto mangledName = mangleModuleScope(module, typeInstance->moduleScope()) +
				mangleMethodName(module, typeInstance, module.getCString("__alignmask"));
			const auto iterator = module.getFunctionMap().find(mangledName);
			
			if (iterator != module.getFunctionMap().end()) {
				return iterator->second;
			}
			
			const auto argInfo = alignMaskArgInfo(module, typeInstance);
			const auto llvmFunction = createLLVMFunction(module, argInfo, getTypeInstanceLinkage(typeInstance), mangledName);
			module.getFunctionMap().insert(std::make_pair(mangledName, llvmFunction));
			
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
			
			Function function(module, *llvmFunction, argInfo, &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			
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
		
		llvm::Value* genAlignOf(Function& function, const SEM::Type* type) {
			const auto alignMask = genAlignMask(function, type);
			const auto name = makeString("alignof__%s", type->isObject() ? type->getObjectType()->name().last().c_str() : "");
			return function.getBuilder().CreateAdd(alignMask, ConstantGenerator(function.module()).getSizeTValue(1), name);
		}
		
		llvm::Value* genAlignMaskValue(Function& function, const SEM::Type* type) {
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
					
					if (type->isPrimitive()) {
						return genPrimitiveAlignMask(function, type);
					}
					
					const auto callName = makeString("alignmask__%s", type->getObjectType()->name().last().c_str());
					const auto alignMaskFunction = genAlignMaskFunction(module, type->getObjectType());
					
					const bool hasTemplate = !type->templateArguments().empty();
					const auto args = hasTemplate ? std::vector<llvm::Value*> { getTemplateGenerator(function, TemplateInst::Type(type)) } : std::vector<llvm::Value*>{};
					const auto callResult = genRawFunctionCall(function, alignMaskArgInfo(module, type->getObjectType()), alignMaskFunction, args);
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
		
		llvm::Value* genAlignMask(Function& function, const SEM::Type* type) {
			const auto it = function.alignMaskMap().find(type);
			if (it != function.alignMaskMap().end()) {
				return it->second;
			}
			
			const auto alignMaskValue = genAlignMaskValue(function, type);
			function.alignMaskMap().insert(std::make_pair(type, alignMaskValue));
			return alignMaskValue;
		}
		
		llvm::Function* genSizeOfFunction(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto mangledName = mangleModuleScope(module, typeInstance->moduleScope()) +
				mangleMethodName(module, typeInstance, module.getCString("__sizeof"));
			const auto iterator = module.getFunctionMap().find(mangledName);
			
			if (iterator != module.getFunctionMap().end()) {
				return iterator->second;
			}
			
			const auto argInfo = sizeOfArgInfo(module, typeInstance);
			const auto llvmFunction = createLLVMFunction(module, argInfo, getTypeInstanceLinkage(typeInstance), mangledName);
			module.getFunctionMap().insert(std::make_pair(mangledName, llvmFunction));
			
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
			Function function(module, *llvmFunction, argInfo, &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			
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
		
		llvm::Value* genSizeOfValue(Function& function, const SEM::Type* const type) {
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
					
					if (type->isPrimitive()) {
						return genPrimitiveSizeOf(function, type);
					}
					
					const auto callName = makeString("sizeof__%s", type->getObjectType()->name().last().c_str());
					const auto sizeOfFunction = genSizeOfFunction(module, type->getObjectType());
					
					const bool hasTemplate = !type->templateArguments().empty();
					const auto args = hasTemplate ? std::vector<llvm::Value*> { getTemplateGenerator(function, TemplateInst::Type(type)) } : std::vector<llvm::Value*>{};
					const auto callResult = genRawFunctionCall(function, sizeOfArgInfo(module, type->getObjectType()), sizeOfFunction, args);
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
		
		llvm::Value* genSizeOf(Function& function, const SEM::Type* const type) {
			const auto it = function.sizeOfMap().find(type);
			if (it != function.sizeOfMap().end()) {
				return it->second;
			}
			
			const auto sizeOfValue = genSizeOfValue(function, type);
			function.sizeOfMap().insert(std::make_pair(type, sizeOfValue));
			return sizeOfValue;
		}
		
		llvm::Value* makeAligned(Function& function, llvm::Value* const size, llvm::Value* const alignMask) {
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
		llvm::Value* genMemberOffsetFunction(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto iterator = module.memberOffsetFunctionMap().find(typeInstance);
			if (iterator != module.memberOffsetFunctionMap().end()) {
				return iterator->second;
			}
			
			const auto mangledName = mangleMethodName(module, typeInstance, module.getCString("__memberoffset"));
			const auto argInfo = memberOffsetArgInfo(module, typeInstance);
			const auto llvmFunction = createLLVMFunction(module, argInfo, getTypeInstanceLinkage(typeInstance), mangledName);
			
			module.memberOffsetFunctionMap().insert(std::make_pair(typeInstance, llvmFunction));
			
			// Always inline this function.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			assert(!typeInstance->isInterface() && !typeInstance->isClassDecl() && !typeInstance->isPrimitive());
			
			Function function(module, *llvmFunction, argInfo, &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			
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
		
		bool isPowerOf2(size_t value) {
			return value != 0 && (value & (value - 1)) == 0;
		}
		
		size_t roundUpToAlign(size_t position, size_t align) {
			assert(isPowerOf2(align));
			return (position + (align - 1)) & (~(align - 1));
		}
		
		llvm::Value* genMemberOffset(Function& function, const SEM::Type* const type, const size_t memberIndex) {
			assert(type->isObject());
			
			auto& module = function.module();
			
			if (memberIndex == 0) {
				return ConstantGenerator(module).getSizeTValue(0);
			}
			
			if (type->isObject() && isTypeSizeKnownInThisModule(module, type)) {
				auto& abi = module.abi();
				const auto objectType = type->getObjectType();
				assert(memberIndex < objectType->variables().size());
				
				size_t offset = 0;
				
				for (size_t i = 0; i < memberIndex; i++) {
					const auto memberVar = objectType->variables().at(i);
					const auto abiType = genABIType(module, memberVar->type());
					offset = roundUpToAlign(offset, abi.typeAlign(abiType)) + abi.typeSize(abiType);
				}
				
				{
					const auto memberVar = objectType->variables().at(memberIndex);
					const auto abiType = genABIType(module, memberVar->type());
					offset = roundUpToAlign(offset, abi.typeAlign(abiType));
				}
				
				return ConstantGenerator(module).getSizeTValue(offset);
			}
			
			SetUseEntryBuilder setUseEntryBuilder(function);
			
			const auto offsetPair = std::make_pair(type, memberIndex);
			
			auto& memberOffsetMap = function.getMemberOffsetMap();
			const auto it = memberOffsetMap.find(offsetPair);
			if (it != memberOffsetMap.end()) {
				return it->second;
			}
			
			const auto callName = makeString("memberoffset_%llu__%s", (unsigned long long) memberIndex,
				type->getObjectType()->name().last().c_str());
			
			const auto memberIndexValue = ConstantGenerator(module).getSizeTValue(memberIndex);
			
			const auto memberOffsetFunction = genMemberOffsetFunction(module, type->getObjectType());
			
			const bool hasTemplate = !type->templateArguments().empty();
			const auto args = hasTemplate ?
				std::vector<llvm::Value*> { getTemplateGenerator(function, TemplateInst::Type(type)), memberIndexValue } :
				std::vector<llvm::Value*>{ memberIndexValue };
			const auto callResult = genRawFunctionCall(function, memberOffsetArgInfo(module, type->getObjectType()), memberOffsetFunction, args);
			callResult->setName(callName);
			
			memberOffsetMap.insert(std::make_pair(offsetPair, callResult));
			
			return callResult;
		}
		
		llvm::Value* genMemberPtr(Function& function, llvm::Value* const objectPointer, const SEM::Type* const objectType, const size_t memberIndex) {
			assert(objectType->isObject());
			auto& module = function.module();
			
			if (isTypeSizeKnownInThisModule(module, objectType)) {
				const auto llvmObjectPointerType = genPointerType(module, objectType);
				const auto castObjectPointer = function.getBuilder().CreatePointerCast(objectPointer, llvmObjectPointerType);
				return function.getBuilder().CreateConstInBoundsGEP2_32(castObjectPointer, 0, memberIndex);
			} else {
				const auto castObjectPointer = function.getBuilder().CreatePointerCast(objectPointer, TypeGenerator(module).getI8PtrType());
				const auto memberOffset = genMemberOffset(function, objectType, memberIndex);
				const auto memberPtr = function.getBuilder().CreateInBoundsGEP(castObjectPointer, memberOffset);
				const auto memberType = objectType->getObjectType()->variables().at(memberIndex)->type()->substitute(objectType->generateTemplateVarMap());
				return function.getBuilder().CreatePointerCast(memberPtr, genPointerType(module, memberType));
			}
		}
		
		std::pair<llvm::Value*, llvm::Value*> getUnionDatatypePointers(Function& function, const SEM::Type* const type, llvm::Value* const objectPtr) {
			assert(type->isObject() && type->getObjectType()->isUnionDatatype());
			auto& module = function.module();
			
			if (isTypeSizeKnownInThisModule(module, type)) {
				const auto loadedTagPtr = function.getBuilder().CreateConstInBoundsGEP2_32(objectPtr, 0, 0);
				const auto unionValuePtr = function.getBuilder().CreateConstInBoundsGEP2_32(objectPtr, 0, 1);
				return std::make_pair(loadedTagPtr, unionValuePtr);
			} else {
				const auto castObjectPtr = function.getBuilder().CreatePointerCast(objectPtr, TypeGenerator(module).getI8PtrType());
				const auto loadedTagPtr = castObjectPtr;
				const auto unionAlignValue = genAlignOf(function, type);
				const auto unionValuePtr = function.getBuilder().CreateInBoundsGEP(castObjectPtr, unionAlignValue);
				return std::make_pair(loadedTagPtr, unionValuePtr);
			}
		}
		
	}
	
}

