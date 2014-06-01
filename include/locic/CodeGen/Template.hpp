#ifndef LOCIC_CODEGEN_TEMPLATE_HPP
#define LOCIC_CODEGEN_TEMPLATE_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/SEM.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		/**
		 * \brief Template generate type.
		 * 
		 * Generates struct { void* rootFn; uint32_t path; }.
		 */
		llvm::Type* templateGeneratorType(Module& module);
		
		/**
		 * \brief Type info type.
		 * 
		 * Generates struct { void* vtable; struct { void* rootFn, uint32_t path; } generator; }.
		 */
		llvm::Type* typeInfoType(Module& module);
		
		/**
		 * \brief Obtain template arguments from a generator pair.
		 * 
		 * This splits a pair of { rootFunction, path } apart and
		 * calls the root function with the path given, which returns
		 * the relevant template arguments.
		 */
		llvm::Value* computeTemplateArguments(Function& function, llvm::Value* generatorValue);
		
		/**
		 * \brief Obtain a template generator for a type with
		 *        template arguments.
		 * 
		 * This will generate a pair of { rootFunction, path }
		 * that can be used with 'computeTemplateArguments' to
		 * obtain the template arguments.
		 */
		llvm::Value* computeTemplateGenerator(Function& function, SEM::Type* type);
		
		/**
		 * \brief Generate a root template argument generator function.
		 * 
		 * This generates code like the following:
		 *
		 * Type[8] <unnamed>(uint32_t path) {
		 *     Types[8] types;
		 *     types[0] = { firstTypeVTablePtr, NULL, 0 };
		 *     types[1] = { secondTypeVTablePtr, NULL, 0 };
		 *     // etc.
		 *     return childFn(types, rootFn, path, 31 - ctlz(path));
		 * }
		 * 
		 * The resulting function can therefore be called to obtain
		 * template arguments given a particular path.
		 */
		llvm::Function* genTemplateRootFunction(Module& module, SEM::Type* type);
		
		/**
		 * \brief Generate the declaration for an intermediate template argument generator function.
		 * 
		 * The function will have the signature:
		 * 
		 * Type[8] (Type[8] types, void* rootFn, uint32_t path, uint8_t position)
		 * 
		 * where Type is struct { void* vtablePtr; void* rootFn, uint32_t path }.
		 * 
		 * The function is also marked as 'readnone' since it doesn't read/write memory
		 * and this should mean it is able to be thoroughly optimised (and calls to it
		 * can be eliminated if possible).
		 */
		llvm::Function* genTemplateIntermediateFunctionDecl(Module& module, SEM::TypeInstance* typeInstance);
		
		/**
		 * \brief Generate an intermediate template argument generator function.
		 * 
		 * This generates code like the following:
		 * 
		 * Type[8] typeIntermediateFunction(Type[8] types, void* rootFn, uint32_t path, uint8_t position) {
		 *     if (position == 0) return;
		 *     
		 *     const auto subPath = (path >> position);
		 *     const auto component = (subPath & 3);
		 *     const auto mask = (1 << position) - 1;
		 *     Type[8] newTypes = types;
		 *     
		 *     if (component == 0) {
		 *         newTypes[0] = { pairType, rootFn, (mask & path) | (0x2 << position) | (0x1 << (position + 2)) };
		 *         return firstChildIntermediateFunction(newTypes, rootFn, path, position - 2);
		 *     } else if (component == 1) {
		 *         newTypes[0] = { vectorType, rootFn, (mask & path) | (0x3 << position) | (0x1 << (position + 2)) };
		 *         return secondChildIntermediateFunction(newTypes, rootFn, path, position - 2);
		 *     } else if (component == 2) {
		 *         newTypes[0] = types[0];
		 *         newTypes[1] = types[0];
		 *         return thirdChildIntermediateFunction(newTypes, rootFn, path, position - 2);
		 *     } else {
		 *         newTypes[0] = types[0];
		 *         return fourthChildIntermediateFunction(newTypes, rootFn, path, position - 2);
		 *     }
		 * }
		 * 
		 * The generated function will be called by a root function, or another
		 * intermediate function, in order to set its own template arguments
		 * based on the arguments provided to it.
		 */
		llvm::Function* genTemplateIntermediateFunction(Module& module, llvm::Function* llvmFunction, SEM::TypeInstance* parentType, const std::map<SEM::Type*, size_t>& templateUses);
		
	}
	
}

#endif
