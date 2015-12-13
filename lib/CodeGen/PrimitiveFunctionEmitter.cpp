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
		PrimitiveFunctionEmitter::emitMinOrMax(const MethodID methodID,
		                                       llvm::ArrayRef<SEM::Value> functionTemplateArguments,
		                                       PendingResultArray args) {
			assert(methodID == METHOD_MIN || methodID == METHOD_MAX);
			
			const auto targetType = functionTemplateArguments[0].typeRefType();
			
			const auto firstValue = args[0].resolve(irEmitter_.function());
			const auto secondValue = args[1].resolve(irEmitter_.function());
			
			// Work out the order of arguments to less than operator.
			const auto leftValue = (methodID == METHOD_MIN) ? secondValue : firstValue;
			const auto rightValue = (methodID == METHOD_MIN) ? firstValue : secondValue;
			assert(leftValue != rightValue);
			
			// less_than() takes the object and argument by reference.
			ValueToRefPendingResult leftValueResult(leftValue, targetType);
			ValueToRefPendingResult rightValueResult(rightValue, targetType);
			const auto compareResult = irEmitter_.emitComparisonCall(METHOD_LESSTHAN,
			                                                         leftValueResult,
			                                                         rightValueResult,
			                                                         targetType);
			const auto compareResultI1 = irEmitter_.emitBoolToI1(compareResult);
			return irEmitter_.builder().CreateSelect(compareResultI1,
			                                         secondValue, firstValue);
		}
		
		llvm::Value*
		PrimitiveFunctionEmitter::emitStandaloneFunction(const MethodID methodID,
		                                                 llvm::ArrayRef<SEM::Value> functionTemplateArguments,
		                                                 PendingResultArray args) {
			assert(methodID.isStandaloneFunction());
			switch (methodID) {
				case METHOD_MIN:
				case METHOD_MAX:
					return emitMinOrMax(methodID,
					                    functionTemplateArguments,
					                    std::move(args));
				default:
					llvm_unreachable("Unknown standalone function.");
			}
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
