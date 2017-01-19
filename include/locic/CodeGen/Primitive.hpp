#ifndef LOCIC_CODEGEN_PRIMITIVE_HPP
#define LOCIC_CODEGEN_PRIMITIVE_HPP

#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/CodeGen/PendingResult.hpp>
#include <locic/AST/ValueArray.hpp>

namespace locic {
	
	class MethodID;
	
	namespace CodeGen {
		
		class IREmitter;
		class Module;
		class TypeGenerator;
		class TypeInfo;
		
		class Primitive {
		public:
			virtual ~Primitive() { }
			
			/**
			 * \brief Query whether the type's size is always statically known.
			 * 
			 * This returns true if the type's size (and alignment) is
			 * known in every module, and false otherwise.
			 * 
			 * \param typeInfo Type information.
			 * \param templateArguments The template arguments provided to the primitive type.
			 * \return Whether the type's size is always known.
			 */
			virtual bool isSizeAlwaysKnown(const TypeInfo& typeInfo,
			                               llvm::ArrayRef<AST::Value> templateArguments) const = 0;
			
			/**
			 * \brief Query whether the type's size is statically known in this module.
			 * 
			 * This returns true if the type's size (and alignment) is
			 * known in this module, and false otherwise.
			 * 
			 * \param typeInfo Type information.
			 * \param templateArguments The template arguments provided to the primitive type.
			 * \return Whether the type's size is known in this module.
			 */
			virtual bool isSizeKnownInThisModule(const TypeInfo& typeInfo,
			                                     llvm::ArrayRef<AST::Value> templateArguments) const = 0;
			
			/**
			 * \brief Query whether the primitive has a custom destructor.
			 */
			virtual bool hasCustomDestructor(const TypeInfo& typeInfo,
			                                 llvm::ArrayRef<AST::Value> templateArguments) const = 0;
			
			/**
			 * \brief Query whether the primitive has a custom move method.
			 */
			virtual bool hasCustomMove(const TypeInfo& typeInfo,
			                           llvm::ArrayRef<AST::Value> templateArguments) const = 0;
			
			/**
			 * \brief Get the ABI type corresponding to the primitive.
			 * 
			 * \param module Current module. FIXME: Remove this argument.
			 * \param abiTypeBuilder The ABI type builder.
			 * \param templateArguments The template arguments provided to the primitive type.
			 * \return The ABI type.
			 */
			virtual llvm_abi::Type getABIType(Module& module,
			                                  const llvm_abi::TypeBuilder& abiTypeBuilder,
			                                  llvm::ArrayRef<AST::Value> templateArguments) const = 0;
			
			/**
			 * \brief Emit method code for primitive.
			 * 
			 * This functions emits the IR code for a selected method
			 * with the given template arguments and runtime
			 * arguments to the provided IR emitter.
			 * 
			 * \param irEmitter The IR emitter for generating code.
			 * \param typeTemplateArguments The template arguments provided to the primitive type.
			 * \param functionTemplateArguments The template arguments provided to the primitive method.
			 * \param args The runtime arguments to the function.
			 * \param resultPtr Pointer to store result, or NULL if no pointer is available.
			 * \return The IR value result.
			 */
			virtual llvm::Value* emitMethod(IREmitter& irEmitter, MethodID methodID,
			                                llvm::ArrayRef<AST::Value> typeTemplateArguments,
			                                llvm::ArrayRef<AST::Value> functionTemplateArguments,
			                                PendingResultArray args,
			                                llvm::Value* resultPtr) const = 0;
			
		};
		
	}
	
}

#endif
