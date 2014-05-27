#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TargetInfo.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Function* genTemplateRootFunction(Module& module, const std::vector<SEM::Type*>& templateArguments) {
			assert(isRootTypeList(templateArguments));
			
			TypeGenerator typeGen(module);
			const auto pathType = typeGen.getI32Type();
			const auto typeInfoType = typeGen.getStructType({ typeGen.getI8PtrType(), typeGen.getI8PtrType(), pathType });
			const auto typeInfoArrayType = typeGen.getArrayType(8, typeInfoType);
			const auto functionType = typeGen.getFunctionType(typeInfoArrayType, { pathType } );
			const auto llvmFunction = createLLVMFunction(module, functionType, linkage, NO_FUNCTION_NAME);
			
			Function function(module, llvmFunction, ArgInfo(...));
			
			llvm::Value* newTypesValue = constGen.getUndef(typeInfoArrayType);
			
			for (size_t i = 0; i < templateArguments.size(); i++) {
				const auto& templateArg = templateArguments.at(i);
				
			}
			
			/* TODO: generate code similar to:
			Type[8] rootFn(uint32_t path) {
				Types[8] types;
				types[0] = { someType0, NULL, 0 };
				types[1] = { someType1, NULL, 0 };
				// etc.
				return childFn(types, rootFn, path, 0);
			}
			*/
			
			return llvmFunction;
		}
		
		inline uint8_t getNextPowerOfTwo(size_t value) {
			uint8_t power = 0;
			while (value != 0) {
				value >>= 1;
				power++;
			}
			return power;
		}
		
		void addUseSet(size_t& currentId, std::map<SEM::Type*, size_t>& currentMap, SEM::Type* type) {
			assert(type->isObjectType());
			currentMap.insert(std::make_pair(type, currentId++));
			for (const auto& arg: type->templateArguments()) {
				if (!arg->isObjectType()) continue;
				addUseSet(currentId, currentMap, arg);
			}
		}
		
		std::map<SEM::Type*, size_t> createUseSet(const std::vector<SEM::Type*>& templateUses) {
			size_t currentId = 0;
			std::map<SEM::Type*, size_t> currentMap;
			for (const auto arg: templateUses) {
				addUseSet(currentId, currentMap, arg);
			}
			return currentMap;
		}
		
		llvm::Function* genTemplateIntermediateFunction(Module& module, const std::map<SEM::TemplateVar*, size_t>& templateVarMap, const std::vector<SEM::Type*>& templateUses) {
			assert(!templateVars.empty() && !templateUses.empty());
			
			TypeGenerator typeGen(module);
			const auto pathType = typeGen.getI32Type();
			const auto typeInfoType = typeGen.getStructType({ typeGen.getI8PtrType(), typeGen.getI8PtrType(), pathType });
			const auto typeInfoArrayType = typeGen.getArrayType(8, typeInfoType);
			const auto functionType = typeGen.getFunctionType(typeInfoArrayType, { typeInfoArrayType, typeGen.getI8PtrType(), pathType, typeGen.getI8Type() } );
			const auto llvmFunction = createLLVMFunction(module, functionType, linkage, NO_FUNCTION_NAME);
			
			const auto bitsRequired = getNextPowerOfTwo(templateUses.size());
			
			ConstantGenerator constGen(module);
			
			Function function(module, llvmFunction, ArgInfo(...));
			
			const auto typesArg = function.getArg(0);
			const auto rootFnArg = function.getArg(1);
			const auto pathArg = function.getArg(2);
			const auto positionArg = function.getArg(3);
			
			auto& builder = function.getBuilder();
			
			const auto subPath = builder.CreateLShr(pathArg, positionArg);
			
			const auto pathEndBB = function.createBasicBlock("pathEnd");
			const auto processSubpathBB = function.createBasicBlock("processSubpath");
			
			const auto compareValue = builder.CreateICmpEq(subPath, constGen.getI32(1));
			builder.CreateCondBr(compareValue, pathEndBB, processSubpathBB);
			
			function.selectBasicBlock(pathEndBB);
			builder.CreateRet(typesArg);
			
			function.selectBasicBlock(processSubpathBB);
			
			const auto component = builder.CreateAnd(subPath, constGen.getI32((1 << bitsRequired) - 1));
			const auto mask = builder.CreateSub(builder.CreateShl(constGen.getI32(1), positionArg), constGen.getI32(1));
			
			const auto templateUseMap = createTemplateUseMap(templateUses);
			
			for (size_t i = 0; i < templateUses.size(); i++) {
				const auto foundComponentEntryBB = function.createBasicBlock("foundComponentEntry");
				const auto tryNextComponentEntryBB = function.createBasicBlock("tryNextComponentEntry");
				const auto componentCompareValue = builder.CreateICmpEq(component, constGen.getI32(i));
				builder.CreateCondBr(componentCompareValue, foundComponentEntryBB, tryNextComponentEntryBB);
				
				function.selectBasicBlock(foundComponentEntryBB);
				
				const auto& templateUse = templateUses.at(i);
				
				llvm::Value* newTypesValue = constGen.getUndef(typeInfoArrayType);
				
				for (size_t j = 0; j < templateUse->templateArguments().size(); j++) {
					const auto& templateUseArg = templateUse->templateArguments().at(j);
					if (templateUseArg->isTemplateVar()) {
						const auto templateVarOffset = templateVarMap.at(templateUseArg.templateVar());
						const auto templateVarValue = builder.CreateExtractValue(typesArg, templateVarOffset);
						newTypesValue = builder.CreateInsertValue(newTypesValue, templateVarValue);
					} else {
						
					}
				}
				
				function.selectBasicBlock(tryNextComponentEntryBB);
			}
			
			// Path is not valid...
			builder.CreateUnreachable();
			
			/* TODO: generate code similar to:
			Type[8] getTypeF(Type[8] types, RootFn rootFn, uint32_t path, uint8_t position) {
				const auto subPath = (path >> position);
				if (subPath == 1) return types;
				
				const auto component = (subPath & 3);
				const auto mask = (1 << position) - 1;
				Type[8] newTypes = types;
				
				if (component == 0) {
					newTypes[0] = { pairType, rootFn, (mask & path) | (0x2 << position) | (0x1 << (position + 2)) };
					return getTypeG(newTypes, rootFn, path, position + 2);
				} else if (component == 1) {
					newTypes[0] = { vectorType, rootFn, (mask & path) | (0x3 << position) | (0x1 << (position + 2)) };
					return getTypeG(newTypes, rootFn, path, position + 2);
				} else if (component == 2) {
					newTypes[0] = types[0];
					newTypes[1] = types[0];
					return getTemplateArgsPair(newTypes, rootFn, path, position + 2);
				} else {
					newTypes[0] = types[0];
					return getTemplateArgsVector(newTypes, rootFn, path, position + 2);
				}
			}
			*/
			
			return llvmFunction;
		}
		
		bool isRootType(SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::VOID: {
					return true;
				}
				
				case SEM::Type::OBJECT: {
					return isRootTypeList(type->templateArguments());
				}
				
				case SEM::Type::REFERENCE: {
					// TODO?
					return true;
				}
				
				case SEM::Type::FUNCTION: {
					return isRootType(type->getFunctionReturnType()) && isRootTypeList(type->getFunctionParameterTypes());
				}
				
				case SEM::Type::METHOD: {
					return isRootType(type->getMethodFunctionType());
				}
				
				case SEM::Type::INTERFACEMETHOD: {
					return isRootType(type->getInterfaceMethodFunctionType());
				}
				
				case SEM::Type::TEMPLATEVAR: {
					return false;
				}
				
				default: {
					llvm_unreachable("Unknown SEM::Type kind in isRootType()");
				}
			}
		}
		
		bool isRootTypeList(const std::vector<SEM::Type*>& templateArguments) {
			for (const auto arg: templateArguments) {
				if (!isRootType(arg)) return false;
			}
			return true;
		}
		
	}
	
}

