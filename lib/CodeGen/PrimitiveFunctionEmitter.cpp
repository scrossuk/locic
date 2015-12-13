#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/PendingResult.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/PrimitiveFunctionEmitter.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		PrimitiveFunctionEmitter::PrimitiveFunctionEmitter(IREmitter& irEmitter)
		: irEmitter_(irEmitter) { }
		
		llvm::Value*
		PrimitiveFunctionEmitter::emitStandaloneFunction(const MethodID methodID,
		                                                 llvm::ArrayRef<SEM::Value> /*functionTemplateArguments*/,
		                                                 PendingResultArray /*args*/) {
			assert(methodID.isStandaloneFunction());
			llvm_unreachable("TODO");
		}
		
		llvm::Value*
		PrimitiveFunctionEmitter::emitMethod(const MethodID methodID,
		                                     const SEM::Type* const parentType,
		                                     llvm::ArrayRef<SEM::Value> functionTemplateArguments,
		                                     PendingResultArray args) {
			assert(parentType != nullptr);
			assert(!methodID.isStandaloneFunction());
			const auto& primitive = irEmitter_.module().getPrimitive(*(parentType->getObjectType()));
			return primitive.emitMethod(irEmitter_, methodID,
			                            arrayRef(parentType->templateArguments()),
			                            functionTemplateArguments,
			                            std::move(args));
		}
		
		llvm::Value*
		PrimitiveFunctionEmitter::emitFunction(const MethodID methodID,
		                                       const SEM::Type* const parentType,
		                                       llvm::ArrayRef<SEM::Value> functionTemplateArguments,
		                                       PendingResultArray args) {
			if (parentType != nullptr) {
				assert(!methodID.isStandaloneFunction());
				return emitMethod(methodID, parentType,
				                  functionTemplateArguments,
				                  std::move(args));
			} else {
				assert(methodID.isStandaloneFunction());
				return emitStandaloneFunction(methodID,
				                              functionTemplateArguments,
				                              std::move(args));
			}
		}
		
	}
	
}
