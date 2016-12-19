#include <stdexcept>

#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>

#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		TypeBuilder::TypeBuilder(Context& argContext)
		: context_(argContext), cachedVoidType_(nullptr), cachedBoolType_(nullptr),
		cachedIntType_(nullptr), cachedSizeType_(nullptr), cachedTypenameType_(nullptr),
		cachedMovableType_(nullptr) { }
		
		const SEM::Type*
		TypeBuilder::getPrimitiveType(PrimitiveID primitiveID,
		                              SEM::ValueArray templateArguments) {
			if (primitiveID == PrimitiveUByte) {
				primitiveID = PrimitiveUInt8;
			}
			const auto& typeInstance = context_.semContext().getPrimitive(primitiveID);
			return SEM::Type::Object(&typeInstance, std::move(templateArguments));
		}
		
		const SEM::Type*
		TypeBuilder::getVoidType() {
			if (cachedVoidType_ != nullptr) {
				return cachedVoidType_;
			}
			
			cachedVoidType_ = getPrimitiveType(PrimitiveVoid);
			return cachedVoidType_;
		}
		
		const SEM::Type*
		TypeBuilder::getBoolType() {
			if (cachedBoolType_ != nullptr) {
				return cachedBoolType_;
			}
			
			cachedBoolType_ = getPrimitiveType(PrimitiveBool);
			return cachedBoolType_;
		}
		
		const SEM::Type*
		TypeBuilder::getIntType() {
			if (cachedIntType_ != nullptr) {
				return cachedIntType_;
			}
			
			cachedIntType_ = getPrimitiveType(PrimitiveInt);
			return cachedIntType_;
		}
		
		const SEM::Type*
		TypeBuilder::getSizeType() {
			if (cachedSizeType_ != nullptr) {
				return cachedSizeType_;
			}
			
			cachedSizeType_ = getPrimitiveType(PrimitiveSize);
			return cachedSizeType_;
		}
		
		const SEM::Type*
		TypeBuilder::getTypenameType() {
			if (cachedTypenameType_ != nullptr) {
				return cachedTypenameType_;
			}
			
			cachedTypenameType_ = getPrimitiveType(PrimitiveTypename);
			return cachedTypenameType_;
		}
		
		const SEM::Type*
		TypeBuilder::getMovableInterfaceType() {
			if (cachedMovableType_ != nullptr) {
				return cachedMovableType_;
			}
			
			cachedMovableType_ = getBuiltInType(context_,
			                                    context_.getCString("movable_t"),
			                                    {});
			return cachedMovableType_;
		}
		
		const SEM::Type*
		TypeBuilder::getPointerType(const SEM::Type* const elementType) {
			SEM::ValueArray array;
			array.push_back(elementType->asValue());
			return getPrimitiveType(PrimitivePtr, std::move(array));
		}
		
		const SEM::Type*
		TypeBuilder::getConstantStaticArrayType(const SEM::Type* elementType,
		                                        const size_t arraySize,
		                                        const Debug::SourceLocation& location) {
			auto arraySizeValue = SEM::Value::Constant(Constant::Integer(arraySize),
			                                           getSizeType());
			return getStaticArrayType(elementType,
			                          std::move(arraySizeValue),
			                          location);
		}
		
		class StaticArraySizeInvalidTypeDiag: public Error {
		public:
			StaticArraySizeInvalidTypeDiag(const SEM::Type* actualType,
			                               const SEM::Type* expectedType)
			: actualTypeString_(actualType->toDiagString()),
			expectedTypeString_(expectedType->toDiagString()) { }
			
			std::string toString() const {
				return makeString("static array size has type '%s', which doesn't "
				                  "match expected type '%s'", actualTypeString_.c_str(),
				                  expectedTypeString_.c_str());
			}
			
		private:
			std::string actualTypeString_;
			std::string expectedTypeString_;
			
		};
		
		const SEM::Type*
		TypeBuilder::getStaticArrayType(const SEM::Type* const elementType,
		                                SEM::Value arraySize,
		                                const Debug::SourceLocation& location) {
			SEM::ValueArray templateArgValues;
			templateArgValues.reserve(2);
			
			const auto arraySizeType = getSizeType();
			
			if (arraySize.isConstant() && arraySize.constant().isInteger()) {
				arraySize = SEM::Value::Constant(arraySize.constant(),
				                                 arraySizeType);
			}
			
			if (arraySize.type() != arraySizeType) {
				context_.issueDiag(StaticArraySizeInvalidTypeDiag(arraySize.type(),
				                                                  arraySizeType),
				                   location);
			}
			
			templateArgValues.push_back(SEM::Value::TypeRef(elementType,
			                                                getTypenameType()->createStaticRefType(elementType)));
			templateArgValues.push_back(std::move(arraySize));
			
			return getPrimitiveType(PrimitiveStaticArray, std::move(templateArgValues));
		}
		
		static SEM::ValueArray getFunctionTemplateArgs(Context& context, const AST::FunctionType functionType) {
			const auto& parameterTypes = functionType.parameterTypes();
			
			SEM::ValueArray templateArgs;
			templateArgs.reserve(1 + parameterTypes.size());
			
			auto& typeBuilder = context.typeBuilder();
			
			const auto boolType = typeBuilder.getBoolType();
			
			auto reducedNoexceptPredicate = reducePredicate(context, functionType.attributes().noExceptPredicate().copy());
			templateArgs.push_back(SEM::Value::PredicateExpr(std::move(reducedNoexceptPredicate), boolType));
			
			const auto typenameType = typeBuilder.getTypenameType();
			
			const auto returnType = functionType.returnType();
			templateArgs.push_back(SEM::Value::TypeRef(returnType, typenameType->createStaticRefType(returnType)));
			
			for (const auto& paramType: parameterTypes) {
				templateArgs.push_back(SEM::Value::TypeRef(paramType, typenameType->createStaticRefType(paramType)));
			}
			
			return templateArgs;
		}
		
		const SEM::Type*
		TypeBuilder::getPrimitiveCallableType(const AST::FunctionType functionType,
		                                      const char* const functionTypeName) {
			return getBuiltInTypeWithValueArgs(context_,
			                                   context_.getCString(functionTypeName),
			                                   getFunctionTemplateArgs(context_, functionType));
		}
		
		static const char* getFunctionPointerName(const size_t numArguments) {
			return PrimitiveID::FunctionPtr(numArguments).toCString();
		}
		
		const SEM::Type*
		TypeBuilder::getTrivialFunctionPointerType(const AST::FunctionType functionType) {
			return getPrimitiveCallableType(functionType,
			                                getFunctionPointerName(functionType.parameterTypes().size()));
		}
		
		static const char* getTemplatedFunctionPointerName(const size_t numArguments) {
			return PrimitiveID::TemplatedFunctionPtr(numArguments).toCString();
		}
		
		const SEM::Type*
		TypeBuilder::getTemplatedFunctionPointerType(const AST::FunctionType functionType) {
			return getPrimitiveCallableType(functionType,
			                                getTemplatedFunctionPointerName(functionType.parameterTypes().size()));
		}
		
		static const char* getMethodFunctionPointerName(const size_t numArguments) {
			return PrimitiveID::MethodFunctionPtr(numArguments).toCString();
		}
		
		const SEM::Type*
		TypeBuilder::getMethodFunctionPointerType(const AST::FunctionType functionType) {
			return getPrimitiveCallableType(functionType,
			                                getMethodFunctionPointerName(functionType.parameterTypes().size()));
		}
		
		static const char* getTemplatedMethodFunctionPointerName(const size_t numArguments) {
			return PrimitiveID::TemplatedMethodFunctionPtr(numArguments).toCString();
		}
		
		const SEM::Type*
		TypeBuilder::getTemplatedMethodFunctionPointerType(const AST::FunctionType functionType) {
			return getPrimitiveCallableType(functionType,
			                                getTemplatedMethodFunctionPointerName(functionType.parameterTypes().size()));
		}
		
		static const char* getVarArgFunctionPointerName(const size_t numArguments) {
			return PrimitiveID::VarArgFunctionPtr(numArguments).toCString();
		}
		
		const SEM::Type*
		TypeBuilder::getVarArgFunctionPointerType(const AST::FunctionType functionType) {
			return getPrimitiveCallableType(functionType,
			                                getVarArgFunctionPointerName(functionType.parameterTypes().size()));
		}
		
		const SEM::Type*
		TypeBuilder::getFunctionPointerType(const AST::FunctionType functionType) {
			const auto& attributes = functionType.attributes();
			
			if (attributes.isVarArg()) {
				return getVarArgFunctionPointerType(functionType);
			} else if (attributes.isMethod()) {
				if (attributes.isTemplated()) {
					return getTemplatedMethodFunctionPointerType(functionType);
				} else {
					return getMethodFunctionPointerType(functionType);
				}
			} else {
				if (attributes.isTemplated()) {
					return getTemplatedFunctionPointerType(functionType);
				} else {
					return getTrivialFunctionPointerType(functionType);
				}
			}
		}
		
		static const char* getTrivialMethodName(const size_t numArguments) {
			return PrimitiveID::Method(numArguments).toCString();
		}
		
		const SEM::Type*
		TypeBuilder::getTrivialMethodType(const AST::FunctionType functionType) {
			return getPrimitiveCallableType(functionType,
			                                getTrivialMethodName(functionType.parameterTypes().size()));
		}
		
		static const char* getTemplatedMethodName(const size_t numArguments) {
			return PrimitiveID::TemplatedMethod(numArguments).toCString();
		}
		
		const SEM::Type*
		TypeBuilder::getTemplatedMethodType(const AST::FunctionType functionType) {
			return getPrimitiveCallableType(functionType,
			                                getTemplatedMethodName(functionType.parameterTypes().size()));
		}
		
		const SEM::Type*
		TypeBuilder::getMethodType(const AST::FunctionType functionType) {
			const auto& attributes = functionType.attributes();
			assert(!attributes.isVarArg());
			assert(attributes.isMethod());
			
			if (attributes.isTemplated()) {
				return getTemplatedMethodType(functionType);
			} else {
				return getTrivialMethodType(functionType);
			}
		}
		
		static const char* getInterfaceMethodName(const size_t numArguments) {
			return PrimitiveID::InterfaceMethod(numArguments).toCString();
		}
		
		const SEM::Type*
		TypeBuilder::getInterfaceMethodType(const AST::FunctionType functionType) {
			const auto& attributes = functionType.attributes();
			(void) attributes;
			assert(!attributes.isVarArg());
			assert(attributes.isMethod());
			
			return getPrimitiveCallableType(functionType,
			                                getInterfaceMethodName(functionType.parameterTypes().size()));
		}
		
		static const char* getStaticInterfaceMethodName(const size_t numArguments) {
			return PrimitiveID::StaticInterfaceMethod(numArguments).toCString();
		}
		
		const SEM::Type*
		TypeBuilder::getStaticInterfaceMethodType(const AST::FunctionType functionType) {
			const auto& attributes = functionType.attributes();
			(void) attributes;
			assert(!attributes.isVarArg());
			assert(!attributes.isMethod());
			
			return getPrimitiveCallableType(functionType,
			                                getStaticInterfaceMethodName(functionType.parameterTypes().size()));
		}
		
	}
	
}

