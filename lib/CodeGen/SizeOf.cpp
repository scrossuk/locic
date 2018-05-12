#include <locic/CodeGen/SizeOf.hpp>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/AST/Function.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>
#include <locic/AST/Var.hpp>

#include <locic/CodeGen/ASTFunctionGenerator.hpp>
#include <locic/CodeGen/CallEmitter.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VirtualCallABI.hpp>

#include <locic/Support/Utils.hpp>

namespace locic {

	namespace CodeGen {
		
		ArgInfo alignMaskArgInfo(Module& module, const AST::TypeInstance* const typeInstance) {
			const auto argInfo = !typeInstance->templateVariables().empty() ?
				ArgInfo::TemplateOnly(module, llvm_abi::SizeTy) :
				ArgInfo::Basic(module, llvm_abi::SizeTy, {});
			return argInfo.withNoMemoryAccess().withNoExcept();
		}
		
		ArgInfo sizeOfArgInfo(Module& module, const AST::TypeInstance* const typeInstance) {
			const auto argInfo = !typeInstance->templateVariables().empty() ?
				ArgInfo::TemplateOnly(module, llvm_abi::SizeTy) :
				ArgInfo::Basic(module, llvm_abi::SizeTy, {});
			return argInfo.withNoMemoryAccess().withNoExcept();
		}
		
		ArgInfo memberOffsetArgInfo(Module& module, const AST::TypeInstance* typeInstance) {
			std::vector<llvm_abi::Type> argTypes;
			argTypes.push_back(llvm_abi::SizeTy);
			
			const auto argInfo = !typeInstance->templateVariables().empty() ?
				ArgInfo::Templated(module, llvm_abi::SizeTy, std::move(argTypes)) :
				ArgInfo::Basic(module, llvm_abi::SizeTy, std::move(argTypes));
			return argInfo.withNoMemoryAccess().withNoExcept();
		}
		
		llvm::Function* genAlignMaskFunctionDecl(Module& module, const AST::TypeInstance* const typeInstance) {
			const auto& function = typeInstance->getFunction(module.getCString("__alignmask"));
			auto& astFunctionGenerator = module.astFunctionGenerator();
			return astFunctionGenerator.getDecl(typeInstance, function);
		}
		
		llvm::Value* genAlignOf(Function& function, const AST::Type* type) {
			const auto alignMask = genAlignMask(function, type);
			const auto name = makeString("alignof__%s", type->isObject() ? type->getObjectType()->fullName().last().c_str() : "");
			return function.getBuilder().CreateAdd(alignMask, ConstantGenerator(function.module()).getSizeTValue(1), name);
		}
		
		llvm::Value*
		genAlignMaskValue(Function& function, const AST::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			SetUseEntryBuilder setUseEntryBuilder(function);
			
			auto& module = function.module();
			auto& abi = module.abi();
			
			IREmitter irEmitter(function);
			
			switch (type->kind()) {
				case AST::Type::OBJECT: {
					TypeInfo typeInfo(module);
					if (typeInfo.isSizeKnownInThisModule(type)) {
						const auto abiType = genABIType(module, type);
						const auto typeAlign = abi.typeInfo().getTypeRequiredAlign(abiType);
						return ConstantGenerator(module).getSizeTValue(typeAlign.asBytes() - 1);
					}
					
					if (type->isPrimitive()) {
						return genPrimitiveAlignMask(function, type);
					}
					
					const auto callName = makeString("alignmask__%s", type->getObjectType()->fullName().last().c_str());
					const auto alignMaskFunction = genAlignMaskFunctionDecl(module, type->getObjectType());
					
					const bool hasTemplate = !type->templateArguments().empty();
					const auto args = hasTemplate ? std::vector<llvm::Value*> { getTemplateGenerator(function, TemplateInst::Type(type)) } : std::vector<llvm::Value*>{};
					
					CallEmitter callEmitter(irEmitter);
					const auto callResult = callEmitter.emitRawCall(alignMaskArgInfo(module, type->getObjectType()),
					                                                alignMaskFunction, args);
					callResult->setName(callName);
					return callResult;
				}
				
				case AST::Type::TEMPLATEVAR: {
					const auto typeInfo = function.getBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
					return module.virtualCallABI().emitCountFnCall(irEmitter,
					                                               typeInfo,
					                                               VirtualCallABI::ALIGNOF);
				}
				
				default: {
					llvm_unreachable("Unknown type enum for generating alignmask.");
				}
			}
		}
		
		llvm::Value* genAlignMask(Function& function, const AST::Type* type) {
			const auto it = function.alignMaskMap().find(type);
			if (it != function.alignMaskMap().end()) {
				return it->second;
			}
			
			const auto alignMaskValue = genAlignMaskValue(function, type);
			function.alignMaskMap().insert(std::make_pair(type, alignMaskValue));
			return alignMaskValue;
		}
		
		llvm::Function* genSizeOfFunctionDecl(Module& module, const AST::TypeInstance* const typeInstance) {
			const auto& function = typeInstance->getFunction(module.getCString("__sizeof"));
			auto& astFunctionGenerator = module.astFunctionGenerator();
			return astFunctionGenerator.getDecl(typeInstance, function);
		}
		
		llvm::Value* genSizeOfValue(Function& function, const AST::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			SetUseEntryBuilder setUseEntryBuilder(function);
			
			auto& module = function.module();
			auto& abi = module.abi();
			
			IREmitter irEmitter(function);
			
			switch (type->kind()) {
				case AST::Type::OBJECT: {
					TypeInfo typeInfo(module);
					if (typeInfo.isSizeKnownInThisModule(type)) {
						const auto abiType = genABIType(module, type);
						const auto typeSize = abi.typeInfo().getTypeAllocSize(abiType);
						return ConstantGenerator(module).getSizeTValue(typeSize.asBytes());
					}
					
					if (type->isPrimitive()) {
						const auto sizeValue = genPrimitiveSizeOf(function, type);
						if (sizeValue != nullptr) {
							return sizeValue;
						}
					}
					
					const auto callName = makeString("sizeof__%s", type->getObjectType()->fullName().last().c_str());
					const auto sizeOfFunction = genSizeOfFunctionDecl(module, type->getObjectType());
					
					const bool hasTemplate = !type->templateArguments().empty();
					const auto args = hasTemplate ? std::vector<llvm::Value*> { getTemplateGenerator(function, TemplateInst::Type(type)) } : std::vector<llvm::Value*>{};
					
					CallEmitter callEmitter(irEmitter);
					const auto callResult = callEmitter.emitRawCall(sizeOfArgInfo(module, type->getObjectType()),
					                                                sizeOfFunction, args);
					callResult->setName(callName);
					return callResult;
				}
				
				case AST::Type::TEMPLATEVAR: {
					const auto typeInfo = function.getBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
					return module.virtualCallABI().emitCountFnCall(irEmitter,
					                                               typeInfo,
					                                               VirtualCallABI::SIZEOF);
				}
				
				default: {
					llvm_unreachable("Unknown type enum for generating sizeof.");
				}
			}
		}
		
		llvm::Value* genSizeOf(Function& function, const AST::Type* const type) {
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
		llvm::Value* genMemberOffsetFunction(Module& module, const AST::TypeInstance* const typeInstance) {
			const auto iterator = module.memberOffsetFunctionMap().find(typeInstance);
			if (iterator != module.memberOffsetFunctionMap().end()) {
				return iterator->second;
			}
			
			const auto mangledName = mangleMethodName(module, typeInstance, module.getCString("__memberoffset"));
			const auto argInfo = memberOffsetArgInfo(module, typeInstance);
			
			auto& astFunctionGenerator = module.astFunctionGenerator();
			const auto llvmFunction = argInfo.createFunction(mangledName.c_str(),
			                                                 astFunctionGenerator.getTypeLinkage(*typeInstance));
			
			module.memberOffsetFunctionMap().insert(std::make_pair(typeInstance, llvmFunction));
			
			// Always inline this function.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			assert(!typeInstance->isInterface() && !typeInstance->isClassDecl() && !typeInstance->isPrimitive());
			
			Function function(module, *llvmFunction, argInfo, &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			IREmitter irEmitter(function);
			
			const auto& typeVars = typeInstance->variables();
			
			// Add the sizes of all the previous member variables to the offset.
			llvm::Value* offsetValue = ConstantGenerator(module).getSizeTValue(0);
			const auto memberIndexValue = function.getArg(0);
			
			for (size_t i = 0; i < typeVars.size(); i++) {
				const auto& var = typeVars.at(i);
				
				offsetValue = makeAligned(function, offsetValue, genAlignMask(function, var->type()));
				
				const auto exitBB = irEmitter.createBasicBlock("exit");
				const auto nextBB = irEmitter.createBasicBlock("next");
				
				const auto varIndexValue = ConstantGenerator(module).getSizeTValue(i);
				const auto compareResult = function.getBuilder().CreateICmpEQ(memberIndexValue, varIndexValue);
				irEmitter.emitCondBranch(compareResult, exitBB, nextBB);
				
				irEmitter.selectBasicBlock(exitBB);
				irEmitter.emitReturn(offsetValue);
				
				irEmitter.selectBasicBlock(nextBB);
				
				if (i != typeVars.size() - 1) {
					offsetValue = function.getBuilder().CreateAdd(offsetValue, genSizeOf(function, var->type()));
				}
			}
			
			irEmitter.emitUnreachable();
			
			return llvmFunction;
		}
		
		llvm::Value* genSuffixByteOffset(Function& function, const AST::TypeInstance& typeInstance) {
			auto& module = function.module();
			
			const auto zero = ConstantGenerator(module).getSizeTValue(0);
			
			// Add up all member variable sizes.
			llvm::Value* classSize = zero;
			
			// Also need to calculate class alignment so the
			// correct amount of padding is added at the end.
			llvm::Value* classAlignMask = zero;
			
			for (const auto& var: typeInstance.variables()) {
				const auto memberAlignMask = genAlignMask(function, var->type());
				
				classAlignMask = function.getBuilder().CreateOr(classAlignMask, memberAlignMask);
				
				classSize = makeAligned(function, classSize, memberAlignMask);
				
				// Add can't overflow.
				const bool hasNoUnsignedWrap = true;
				const bool hasNoSignedWrap = false;
				classSize = function.getBuilder().CreateAdd(classSize, genSizeOf(function, var->type()),
									    "", hasNoUnsignedWrap, hasNoSignedWrap);
			}
			
			return makeAligned(function, classSize, classAlignMask);
		}
		
		llvm::Value* genMemberOffset(Function& function, const AST::Type* const type, const size_t memberIndex) {
			assert(type->isObject());
			
			auto& module = function.module();
			
			if (memberIndex == 0 || type->isUnion()) {
				return ConstantGenerator(module).getSizeTValue(0);
			}
			
			TypeInfo typeInfo(module);
			if (typeInfo.isSizeKnownInThisModule(type)) {
				auto& abi = module.abi();
				const auto objectType = type->getObjectType();
				assert(memberIndex < objectType->variables().size());
				
				size_t offset = 0;
				
				for (size_t i = 0; i < memberIndex; i++) {
					const auto memberVar = objectType->variables().at(i);
					const auto abiType = genABIType(module, memberVar->type());
					const auto typeAlign = abi.typeInfo().getTypeRequiredAlign(abiType);
					const auto typeSize = abi.typeInfo().getTypeAllocSize(abiType);
					offset = roundUpToAlign(offset, typeAlign.asBytes()) + typeSize.asBytes();
				}
				
				{
					const auto memberVar = objectType->variables().at(memberIndex);
					const auto abiType = genABIType(module, memberVar->type());
					const auto typeAlign = abi.typeInfo().getTypeRequiredAlign(abiType);
					offset = roundUpToAlign(offset, typeAlign.asBytes());
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
				type->getObjectType()->fullName().last().c_str());
			
			const auto memberIndexValue = ConstantGenerator(module).getSizeTValue(memberIndex);
			
			const auto memberOffsetFunction = genMemberOffsetFunction(module, type->getObjectType());
			
			const bool hasTemplate = !type->templateArguments().empty();
			const auto args = hasTemplate ?
				std::vector<llvm::Value*> { memberIndexValue, getTemplateGenerator(function, TemplateInst::Type(type)) } :
				std::vector<llvm::Value*>{ memberIndexValue };
			
			IREmitter irEmitter(function);
			CallEmitter callEmitter(irEmitter);
			const auto callResult = callEmitter.emitRawCall(memberOffsetArgInfo(module, type->getObjectType()),
			                                                memberOffsetFunction, args);
			callResult->setName(callName);
			
			memberOffsetMap.insert(std::make_pair(offsetPair, callResult));
			
			return callResult;
		}
		
		llvm::Value* genMemberPtr(Function& function, llvm::Value* const objectPointer, const AST::Type* const objectType, const size_t memberIndex) {
			assert(objectType->isObject());
			if (memberIndex == 0) {
				return objectPointer;
			}
			
			auto& module = function.module();
			
			IREmitter irEmitter(function);
			
			TypeInfo typeInfo(module);
			if (typeInfo.isSizeKnownInThisModule(objectType) && !objectType->isUnion()) {
				return irEmitter.emitConstInBoundsGEP2_32(genABIType(module, objectType),
				                                          objectPointer,
				                                          0, memberIndex);
			} else {
				const auto memberOffset = genMemberOffset(function, objectType, memberIndex);
				return irEmitter.emitInBoundsGEP(llvm_abi::Int8Ty,
				                                 objectPointer,
				                                 memberOffset);
			}
		}
		
		std::pair<llvm::Value*, llvm::Value*> getVariantPointers(Function& function, const AST::Type* const type, llvm::Value* const objectPtr) {
			assert(type->isVariant());
			auto& module = function.module();
			
			IREmitter irEmitter(function);
			
			TypeInfo typeInfo(module);
			if (typeInfo.isSizeKnownInThisModule(type)) {
				const auto abiType = genABIType(module, type);
				const auto loadedTagPtr = irEmitter.emitConstInBoundsGEP2_32(abiType,
				                                                             objectPtr, 0, 0);
				const auto unionValuePtr = irEmitter.emitConstInBoundsGEP2_32(abiType,
				                                                              objectPtr, 0, 1);
				return std::make_pair(loadedTagPtr, unionValuePtr);
			} else {
				const auto unionAlignValue = genAlignOf(function, type);
				const auto unionValuePtr = irEmitter.emitInBoundsGEP(llvm_abi::Int8Ty,
				                                                     objectPtr,
				                                                     unionAlignValue);
				return std::make_pair(objectPtr, unionValuePtr);
			}
		}
		
	}
	
}

