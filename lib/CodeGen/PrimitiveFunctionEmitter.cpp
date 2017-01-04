#include <locic/AST/Context.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/PendingResult.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/PrimitiveFunctionEmitter.hpp>
#include <locic/CodeGen/Support.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		PrimitiveFunctionEmitter::PrimitiveFunctionEmitter(IREmitter& irEmitter)
		: irEmitter_(irEmitter) { }
		
		llvm::Value*
		PrimitiveFunctionEmitter::emitMinOrMax(const MethodID methodID,
		                                       llvm::ArrayRef<AST::Value> functionTemplateArguments,
		                                       PendingResultArray args,
		                                       llvm::Value* const hintResultValue) {
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
			const auto result =
				irEmitter_.builder().CreateSelect(compareResultI1,
				                                  secondValue, firstValue);
			
			ValueToRefPendingResult minResult(result, targetType);
			const auto movedResult = irEmitter_.emitMoveCall(minResult, targetType,
			                                                 hintResultValue);
			
			irEmitter_.emitDestructorCall(secondValue, targetType);
			irEmitter_.emitDestructorCall(firstValue, targetType);
			
			return movedResult;
		}
		
		static PrimitiveID getRangePrimitiveID(const MethodID methodID) {
			switch (methodID) {
				case METHOD_RANGE:
					return PrimitiveRange;
				case METHOD_RANGE_INCL:
					return PrimitiveRangeIncl;
				case METHOD_REVERSE_RANGE:
					return PrimitiveReverseRange;
				case METHOD_REVERSE_RANGE_INCL:
					return PrimitiveReverseRangeIncl;
				default:
					llvm_unreachable("Unknown range() function method ID.");
			}
		}
		
		llvm::Value*
		PrimitiveFunctionEmitter::emitRange(const MethodID methodID,
		                                    llvm::ArrayRef<AST::Value> functionTemplateArguments,
		                                    PendingResultArray args,
		                                    llvm::Value* const hintResultValue) {
			llvm::SmallVector<AST::Value, 1> typeTemplateArguments;
			
			const auto targetType = functionTemplateArguments[0].typeRefType();
			const auto typenameType = irEmitter_.module().context().astContext().getPrimitive(PrimitiveTypename).selfType();
			typeTemplateArguments.push_back(AST::Value::TypeRef(targetType,
			                                                    typenameType->createStaticRefType(targetType)));
			
			llvm::SmallVector<AST::Value, 1> methodFunctionTemplateArguments;
			
			const auto rangePrimitiveID = getRangePrimitiveID(methodID);
			const auto& rangeTypeInstance = irEmitter_.module().context().astContext().getPrimitive(rangePrimitiveID);
			const auto& primitive = irEmitter_.module().getPrimitive(rangeTypeInstance);
			return primitive.emitMethod(irEmitter_, METHOD_CREATE,
			                            typeTemplateArguments,
			                            methodFunctionTemplateArguments,
			                            std::move(args), hintResultValue);
		}
		
		llvm::Value*
		PrimitiveFunctionEmitter::emitStandaloneFunction(const MethodID methodID,
		                                                 llvm::ArrayRef<AST::Value> functionTemplateArguments,
		                                                 PendingResultArray args,
		                                                 llvm::Value* const hintResultValue) {
			assert(methodID.isStandaloneFunction());
			switch (methodID) {
				case METHOD_MIN:
				case METHOD_MAX:
					return emitMinOrMax(methodID,
					                    functionTemplateArguments,
					                    std::move(args),
					                    hintResultValue);
				case METHOD_RANGE:
				case METHOD_RANGE_INCL:
				case METHOD_REVERSE_RANGE:
				case METHOD_REVERSE_RANGE_INCL:
					return emitRange(methodID, functionTemplateArguments,
					                 std::move(args),
					                 hintResultValue);
				default:
					llvm_unreachable("Unknown standalone function.");
			}
		}
		
		llvm::Value*
		PrimitiveFunctionEmitter::emitMethod(const MethodID methodID,
		                                     const AST::Type* const parentType,
		                                     llvm::ArrayRef<AST::Value> functionTemplateArguments,
		                                     PendingResultArray args,
		                                     llvm::Value* const hintResultValue) {
			assert(parentType != nullptr);
			assert(!methodID.isStandaloneFunction());
			const auto& primitive = irEmitter_.module().getPrimitive(*(parentType->getObjectType()));
			return primitive.emitMethod(irEmitter_, methodID,
			                            arrayRef(parentType->templateArguments()),
			                            functionTemplateArguments,
			                            std::move(args), hintResultValue);
		}
		
		llvm::Value*
		PrimitiveFunctionEmitter::emitFunction(const MethodID methodID,
		                                       const AST::Type* const parentType,
		                                       llvm::ArrayRef<AST::Value> functionTemplateArguments,
		                                       PendingResultArray args,
		                                       llvm::Value* const hintResultValue) {
			if (parentType != nullptr) {
				assert(!methodID.isStandaloneFunction());
				return emitMethod(methodID, parentType,
				                  functionTemplateArguments,
				                  std::move(args), hintResultValue);
			} else {
				assert(methodID.isStandaloneFunction());
				return emitStandaloneFunction(methodID,
				                              functionTemplateArguments,
				                              std::move(args),
				                              hintResultValue);
			}
		}
		
	}
	
}
