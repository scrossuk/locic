#include <stdexcept>
#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/SEM.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/SEMFunctionGenerator.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/VirtualCall.hpp>

namespace locic {
	
	namespace CodeGen {
		
		ArgInfo destructorArgInfo(Module& module, const SEM::TypeInstance& typeInstance) {
			const bool hasTemplateArgs = !typeInstance.templateVariables().empty();
			const auto argInfo = hasTemplateArgs ? ArgInfo::VoidTemplateAndContext(module) : ArgInfo::VoidContextOnly(module);
			return argInfo.withNoExcept();
		}
		
		void genDestructorCall(Function& function, const SEM::Type* const type, llvm::Value* value) {
			auto& module = function.module();
			
			if (type->isObject()) {
				TypeInfo typeInfo(module);
				if (!typeInfo.hasCustomDestructor(type)) {
					return;
				}
				
				if (type->isPrimitive()) {
					genPrimitiveDestructorCall(function, type, value);
					return;
				}
				
				const auto& typeInstance = *(type->getObjectType());
				
				// Call destructor.
				const auto argInfo = destructorArgInfo(module, typeInstance);
				const auto destructorFunction = genDestructorFunctionDecl(module, typeInstance);
				
				const auto castValue = function.getBuilder().CreatePointerCast(value, TypeGenerator(module).getPtrType());
				
				llvm::SmallVector<llvm::Value*, 2> args;
				if (!type->templateArguments().empty()) {
					args.push_back(getTemplateGenerator(function, TemplateInst::Type(type)));
				}
				args.push_back(castValue);
				
				(void) genRawFunctionCall(function, argInfo, destructorFunction, args);
			} else if (type->isTemplateVar()) {
				const auto typeInfo = function.getEntryBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
				const auto castValue = function.getBuilder().CreatePointerCast(value, TypeGenerator(module).getPtrType());
				VirtualCall::generateDestructorCall(function, typeInfo, castValue);
			}
		}
		
		void scheduleDestructorCall(Function& function, const SEM::Type* type, llvm::Value* value) {
			TypeInfo typeInfo(function.module());
			if (!typeInfo.hasCustomDestructor(type)) {
				return;
			}
			
			function.pushUnwindAction(UnwindAction::Destructor(type, value));
		}
		
		Debug::SourcePosition getDebugDestructorPosition(Module& module, const SEM::TypeInstance& typeInstance) {
			const auto iterator = typeInstance.functions().find(module.getCString("__destroy"));
			if (iterator != typeInstance.functions().end()) {
				return iterator->second->debugInfo()->scopeLocation.range().end();
			} else {
				return typeInstance.debugInfo()->location.range().start();
			}
		}
		
		llvm::DISubprogram genDebugDestructorFunction(Module& module, const SEM::TypeInstance& typeInstance, llvm::Function* const function) {
			const auto& typeInstanceInfo = *(typeInstance.debugInfo());
			
			const auto position = getDebugDestructorPosition(module, typeInstance);
			
			const auto file = module.debugBuilder().createFile(typeInstanceInfo.location.fileName());
			const auto lineNumber = position.lineNumber();
			const bool isInternal = typeInstance.moduleScope().isInternal();
			const bool isDefinition = true;
			const auto functionName = typeInstance.name() + module.getCString("~");
			
			std::vector<LLVMMetadataValue*> debugArgs;
			debugArgs.push_back(module.debugBuilder().createVoidType());
			
			const auto functionType = module.debugBuilder().createFunctionType(file, debugArgs);
			
			return module.debugBuilder().createFunction(file, lineNumber, isInternal,
				isDefinition, functionName, functionType, function);
		}
		
		llvm::Function* getNullDestructorFunction(Module& module) {
			const auto mangledName = module.getCString("__null_destructor");
			
			const auto iterator = module.getFunctionMap().find(mangledName);
			
			if (iterator != module.getFunctionMap().end()) {
				return iterator->second;
			}
			
			const auto argInfo = ArgInfo::VoidTemplateAndContext(module).withNoExcept().withNoMemoryAccess();
			const auto llvmFunction = createLLVMFunction(module, argInfo, llvm::Function::InternalLinkage, mangledName);
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			module.getFunctionMap().insert(std::make_pair(mangledName, llvmFunction));
			
			Function function(module, *llvmFunction, argInfo);
			function.getBuilder().CreateRetVoid();
			
			return llvmFunction;
		}
		
		llvm::Function* genVTableDestructorFunction(Module& module, const SEM::TypeInstance& typeInstance) {
			TypeInfo typeInfo(module);
			if (!typeInfo.objectHasCustomDestructor(typeInstance)) {
				return getNullDestructorFunction(module);
			}
			
			const auto destructorFunction = genDestructorFunctionDecl(module, typeInstance);
			
			if (!typeInstance.templateVariables().empty()) {
				return destructorFunction;
			}
			
			// Create stub to call destructor with no template generator.
			const auto argInfo = ArgInfo::VoidTemplateAndContext(module).withNoExcept();
			const auto llvmFunction = createLLVMFunction(module, argInfo, llvm::Function::InternalLinkage, module.getCString(""));
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			Function function(module, *llvmFunction, argInfo);
			
			const auto debugInfo = genDebugDestructorFunction(module, typeInstance, llvmFunction);
			function.attachDebugInfo(debugInfo);
			function.setDebugPosition(getDebugDestructorPosition(module, typeInstance));
			
			genRawFunctionCall(function, destructorArgInfo(module, typeInstance), destructorFunction, std::vector<llvm::Value*> { function.getRawContextValue() });
			
			function.getBuilder().CreateRetVoid();
			
			return llvmFunction;
		}
		
		llvm::Function* genDestructorFunctionDecl(Module& module, const SEM::TypeInstance& typeInstance) {
			const auto iterator = module.getDestructorMap().find(&typeInstance);
			
			if (iterator != module.getDestructorMap().end()) {
				return iterator->second;
			}
			
			const auto& function = typeInstance.functions().at(module.getCString("__destroy"));
			
			auto& semFunctionGenerator = module.semFunctionGenerator();
			const auto llvmFunction = semFunctionGenerator.getDecl(&typeInstance,
			                                                       *function);
			
			const auto argInfo = destructorArgInfo(module, typeInstance);
			if (argInfo.hasTemplateGeneratorArgument()) {
				// Always inline templated destructors.
				llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			}
			
			module.getDestructorMap().insert(std::make_pair(&typeInstance, llvmFunction));
			
			return llvmFunction;
		}
		
	}
	
}

