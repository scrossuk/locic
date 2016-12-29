#include <locic/CodeGen/ASTFunctionGenerator.hpp>

#include <locic/AST/Function.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ASTCodeEmitter.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VirtualCallABI.hpp>
#include <locic/CodeGen/VTable.hpp>

#include <locic/AST/TypeInstance.hpp>

namespace locic {
	
	namespace CodeGen {
		
		ASTFunctionGenerator::ASTFunctionGenerator(Module& module)
		: module_(module) { }
		
		llvm::GlobalValue::LinkageTypes
		ASTFunctionGenerator::getLinkage(const AST::TypeInstance* typeInstance,
		                                 const AST::Function& function) const {
			if (function.isPrimitive()) {
				// Primitive function (generated by CodeGen).
				assert(!function.hasScope());
				return llvm::Function::LinkOnceODRLinkage;
			}
			
			const auto& moduleScope = function.moduleScope();
			
			if (moduleScope.isInternal()) {
				// Internal functions.
				return llvm::Function::InternalLinkage;
			} else if (moduleScope.isImport() &&
			           function.isAutoGenerated() &&
			           !typeInstance->isClass()) {
				// Auto-generated type method (e.g. an implicitcopy for a struct).
				return llvm::Function::LinkOnceODRLinkage;
			} else {
				// Imported declarations or exported definitions.
				return llvm::Function::ExternalLinkage;
			}
		}
		
		llvm::GlobalValue::LinkageTypes
		ASTFunctionGenerator::getTypeLinkage(const AST::TypeInstance& typeInstance) const {
			if (typeInstance.isPrimitive()) {
				// Primitive type.
				return llvm::Function::LinkOnceODRLinkage;
			}
			
			const auto& moduleScope = typeInstance.moduleScope();
			
			if (moduleScope.isInternal()) {
				// Internal definition.
				return llvm::Function::InternalLinkage;
			} else if (typeInstance.isClass()) {
				return llvm::Function::ExternalLinkage;
			} else {
				// Non-class type.
				return llvm::Function::LinkOnceODRLinkage;
			}
		}
		
		void
		ASTFunctionGenerator::addStandardFunctionAttributes(const AST::FunctionType type,
		                                                    llvm::Function& llvmFunction) {
			if (!canPassByValue(module_, type.returnType())) {
				// Class return values are allocated by the caller,
				// which passes a pointer to the callee. The caller
				// and callee must, for the sake of optimisation,
				// ensure that the following attributes hold...
				
				// Caller must ensure pointer is always valid.
				llvmFunction.addAttribute(1, llvm::Attribute::StructRet);
				
				// Caller must ensure pointer does not alias with
				// any other arguments.
				llvmFunction.addAttribute(1, llvm::Attribute::NoAlias);
				
				// Callee must not capture the pointer.
				llvmFunction.addAttribute(1, llvm::Attribute::NoCapture);
			}
		}
		
		llvm::Function*
		ASTFunctionGenerator::createNamedFunction(const String& name,
		                                          const AST::FunctionType type,
		                                          const llvm::GlobalValue::LinkageTypes linkage) {
			const auto argInfo = getFunctionArgInfo(module_, type);
			const auto llvmFunction = createLLVMFunction(module_, argInfo, linkage, name);
			addStandardFunctionAttributes(type, *llvmFunction);
			return llvmFunction;
		}
		
		llvm::Function*
		ASTFunctionGenerator::getNamedFunction(const String& name,
		                                       const AST::FunctionType type,
		                                       const llvm::GlobalValue::LinkageTypes linkage) {
			const auto iterator = module_.getFunctionMap().find(name);
			if (iterator != module_.getFunctionMap().end()) {
				return iterator->second;
			}
			
			const auto llvmFunction = createNamedFunction(name,
			                                              type,
			                                              linkage);
			
			module_.getFunctionMap().insert(std::make_pair(name, llvmFunction));
			
			return llvmFunction;
		}
		
		llvm::Function*
		ASTFunctionGenerator::getDecl(const AST::TypeInstance* typeInstance,
		                              const AST::Function& function,
		                              const bool isInnerMethod) {
			if (function.isMethod()) {
				assert(typeInstance != nullptr);
			} else {
				assert(typeInstance == nullptr);
			}
			
			assert(!function.requiresPredicate().isFalse());
			
			if (!isInnerMethod) {
				const auto iterator = module_.getFunctionDeclMap().find(&function);
				
				if (iterator != module_.getFunctionDeclMap().end()) {
					return iterator->second;
				}
			}
			
			const auto mangledName =
				mangleModuleScope(module_, function.moduleScope()) +
				(function.isMethod() ?
				 mangleMethodName(module_, typeInstance, function.fullName().last()) :
				 mangleFunctionName(module_, function.fullName())) +
				(isInnerMethod ? "_internal" : "");
			
			const auto linkage = getLinkage(typeInstance, function);
			const auto llvmFunction = getNamedFunction(mangledName,
			                                           function.type(),
			                                           linkage);
			
			module_.getFunctionDeclMap().insert(std::make_pair(&function, llvmFunction));
			
			if (function.isPrimitive()) {
				assert(!isInnerMethod);
				// Generate primitive methods as needed.
				return genDef(typeInstance, function, isInnerMethod);
			}
			
			return llvmFunction;
		}
		
		bool
		ASTFunctionGenerator::hasDef(const AST::TypeInstance* typeInstance,
		                             const AST::Function& function) {
			if (function.hasScope()) {
				// This function has some associated code
				// specified by the user.
				return true;
			}
			
			if (function.isPrimitive()) {
				// This function is a language primitive (e.g.
				// a method of 'int_t').
				return true;
			}
			
			const bool isClassDecl = typeInstance != nullptr &&
			                         typeInstance->isClassDecl();
			
			if (function.isAutoGenerated() && !isClassDecl) {
				// This function has been specified to be
				// default-generated and is NOT in a class
				// declaration (class declaration methods will
				// be default-generated in their own module).
				return true;
			}
			
			// We shouldn't be generating any IR code for this
			// function.
			return false;
		}
		
		llvm::Function*
		ASTFunctionGenerator::genDef(const AST::TypeInstance* typeInstance,
		                             const AST::Function& function,
		                             const bool isInnerMethod) {
			assert(!isInnerMethod ||
			       function.fullName().last() == "__moveto" ||
			       function.fullName().last() == "__destroy");
			assert(!isInnerMethod || !function.isPrimitive());
			const auto llvmFunction = getDecl(typeInstance,
			                                  function,
			                                  isInnerMethod);
			
			if (!hasDef(typeInstance, function)) {
				// A declaration, so it has no associated code.
				assert(!isInnerMethod);
				return llvmFunction;
			}
			
			const auto templatedObject =
				!function.templateVariables().empty() || typeInstance == nullptr ?
					TemplatedObject::Function(typeInstance,
					                          // FIXME: Remove const_cast.
								  const_cast<AST::Function*>(&function)) :
					TemplatedObject::TypeInstance(typeInstance);
			
			auto& templateBuilder = module_.templateBuilder(templatedObject);
			const auto argInfo = getFunctionArgInfo(module_, function.type());
			Function functionGenerator(module_, *llvmFunction, argInfo, &templateBuilder);
			
			if (argInfo.hasTemplateGeneratorArgument() || (typeInstance != nullptr && typeInstance->isPrimitive())) {
				// Always inline if possible for templated functions
				// or methods of primitives.
				llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			}
			
			if (isInnerMethod) {
				llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			}
			
			const auto debugSubprogram = genDebugFunctionInfo(module_,
			                                                  typeInstance,
			                                                  function,
			                                                  llvmFunction);
			assert(debugSubprogram);
			functionGenerator.attachDebugInfo(*debugSubprogram);
			
			functionGenerator.setDebugPosition(function.debugInfo()->scopeLocation.range().start());
			
			ASTCodeEmitter codeEmitter(functionGenerator);
			codeEmitter.emitFunctionCode(typeInstance,
			                             function,
			                             isInnerMethod);
			
			if (!function.templateVariables().empty() && !function.isPrimitive()) {
				(void) genTemplateIntermediateFunction(module_,
				                                       templatedObject,
				                                       templateBuilder);
				
				// Update all instructions needing the bits required value
				// with the correct value (now it is known).
				templateBuilder.updateAllInstructions(module_);
			}
			
			functionGenerator.verify();
			
			return llvmFunction;
		}
		
		llvm::Function*
		ASTFunctionGenerator::genTemplateFunctionStub(const AST::TemplateVar* templateVar,
		                                              const String& functionName,
		                                              AST::FunctionType functionType,
		                                              llvm::DebugLoc debugLoc) {
			// --- Generate function declaration.
			const auto argInfo = getFunctionArgInfo(module_, functionType);
			const auto llvmFunction = createLLVMFunction(module_, argInfo, llvm::Function::InternalLinkage, module_.getCString("templateFunctionStub"));
			
			// Always inline template function stubs.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			addStandardFunctionAttributes(functionType, *llvmFunction);
			
			// --- Generate function code.
			
			Function functionGenerator(module_, *llvmFunction, argInfo);
			functionGenerator.getBuilder().SetCurrentDebugLocation(debugLoc);
			
			const auto typeInfoValue = functionGenerator.getBuilder().CreateExtractValue(functionGenerator.getTemplateArgs(), { (unsigned) templateVar->index() });
			
			const auto ptrType = TypeGenerator(module_).getPtrType();
			const auto contextPointer = argInfo.hasContextArgument() ? functionGenerator.getContextValue() : ConstantGenerator(module_).getNull(ptrType);
			
			const auto methodHash = CreateMethodNameHash(functionName);
			const auto methodHashValue = ConstantGenerator(module_).getI64(methodHash);
			
			const VirtualObjectComponents objectComponents(getTypeInfoComponents(functionGenerator, typeInfoValue), contextPointer);
			const VirtualMethodComponents methodComponents(objectComponents, methodHashValue);
			
			const auto argList = functionGenerator.getArgList();
			
			const bool hasReturnVar = !canPassByValue(module_, functionType.returnType());
			const auto hintResultValue = hasReturnVar ? functionGenerator.getReturnVar() : nullptr;
			
			IREmitter irEmitter(functionGenerator);
			const auto result = module_.virtualCallABI().emitCall(irEmitter,
			                                                      functionType,
			                                                      methodComponents,
			                                                      argList,
			                                                      hintResultValue);
			
			if (hasReturnVar) {
				irEmitter.emitMoveStore(result, functionGenerator.getReturnVar(), functionType.returnType());
				irEmitter.emitReturnVoid();
			} else if (result->getType()->isVoidTy()) {
				irEmitter.emitReturnVoid();
			} else {
				functionGenerator.returnValue(result);
			}
			
			// Check the generated function is correct.
			functionGenerator.verify();
			
			return llvmFunction;
		}
		
	}
	
}