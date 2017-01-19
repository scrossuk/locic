#ifndef LOCIC_CODEGEN_TEMPLATE_HPP
#define LOCIC_CODEGEN_TEMPLATE_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TemplateBuilder.hpp>
#include <locic/CodeGen/TemplatedObject.hpp>

namespace locic {

	namespace CodeGen {
		
		/**
		 * \brief Template generate type.
		 * 
		 * Generates struct { void* rootFn; uint32_t path; }.
		 */
		llvm_abi::Type templateGeneratorType(Module& module);
		
		/**
		 * \brief Type info type.
		 * 
		 * Generates struct { void* vtable; struct { void* rootFn, uint32_t path; } generator; }.
		 */
		llvm_abi::Type typeInfoType(Module& module);
		
		/**
		 * \brief Root function arg info.
		 */
		ArgInfo rootFunctionArgInfo(Module& module);
		
		/**
		 * \brief Intermediate function arg info.
		 */
		ArgInfo intermediateFunctionArgInfo(Module& module);
		
		/**
		 * \brief Obtain template arguments from a generator pair.
		 * 
		 * This splits a pair of { rootFunction, path } apart and
		 * calls the root function with the path given, which returns
		 * the relevant template arguments.
		 */
		llvm::Value* computeTemplateArguments(Function& function, llvm::Value* generatorValue);
		
		/**
		 * \brief Null template generator.
		 * 
		 * This is just a constant struct of
		 * { rootFunction = NULL, path = 0 }.
		 */
		llvm::Constant* nullTemplateGenerator(Module& module);
		
		/**
		 * \brief Obtain a template generator for a template
		 *        instantiation.
		 * 
		 * This will generate a pair of { rootFunction, path }
		 * that can be used with 'computeTemplateArguments' to
		 * obtain the template arguments.
		 */
		llvm::Value* getTemplateGenerator(Function& function, const TemplateInst& templateInst);
		
		/**
		 * \brief Generate a root template argument generator function.
		 * 
		 * This generates code like the following:
		 *
		 * void <unnamed>(Type* types, void* context, uint32_t path) {
		 *     types[0] = { firstTypeVTablePtr, NULL, 0 };
		 *     types[1] = { secondTypeVTablePtr, NULL, 0 };
		 *     // etc.
		 *     childFn(types, rootFn, context, path, 31 - ctlz(path));
		 *     return;
		 * }
		 * 
		 * The resulting function can therefore be called to obtain
		 * template arguments given a particular path.
		 */
		llvm::Function* genTemplateRootFunction(Function& parentFunction, const TemplateInst& templateInst);
		
		/**
		 * \brief Generate the declaration for an intermediate template argument generator function.
		 * 
		 * The function will have the signature:
		 * 
		 * void (Type* types, void* rootFn, uint32_t path, size_t position)
		 * 
		 * where Type is struct { void* vtablePtr; void* rootFn, uint32_t path }.
		 * 
		 * The function is also marked as 'readnone' since it doesn't read/write memory
		 * and this should mean it is able to be thoroughly optimised (and calls to it
		 * can be eliminated if possible).
		 */
		llvm::Function* genTemplateIntermediateFunctionDecl(Module& module, TemplatedObject templatedObject);
		
		/**
		 * \brief Generate an intermediate template argument generator function.
		 * 
		 * This generates code like the following:
		 * 
		 * void typeIntermediateFunction(Type* types, void* rootFn, uint32_t path, size_t parentPosition) {
		 *     if (parentPosition == 0) return;
		 *     
		 *     const auto position = parentPosition - 2;
		 *     const auto subPath = (path >> position);
		 *     const auto mask = 0x3;
		 *     const auto component = (subPath & mask);
		 *     
		 *     if (component == 0) {
		 *         newTypes[0] = { pairType, rootFn, (subPath & ~mask) | 0x2 };
		 *         firstChildIntermediateFunction(newTypes, rootFn, path, position);
		 *         return;
		 *     } else if (component == 1) {
		 *         newTypes[0] = { vectorType, rootFn, (subPath & ~mask) | 0x3 };
		 *         secondChildIntermediateFunction(types, rootFn, path, position);
		 *         return;
		 *     } else if (component == 2) {
		 *         newTypes[1] = types[0];
		 *         thirdChildIntermediateFunction(types, rootFn, path, position);
		 *         return;
		 *     } else {
		 *         fourthChildIntermediateFunction(types, rootFn, path, position);
		 *         return;
		 *     }
		 * }
		 * 
		 * The generated function will be called by a root function, or another
		 * intermediate function, in order to set its own template arguments
		 * based on the arguments provided to it.
		 */
		llvm::Function* genTemplateIntermediateFunction(Module& module, TemplatedObject templatedObject, const TemplateBuilder& templateBuilder);
		
	}
	
}

#endif
