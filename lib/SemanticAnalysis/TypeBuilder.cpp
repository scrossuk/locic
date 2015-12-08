#include <stdexcept>

#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		TypeBuilder::TypeBuilder(Context& argContext)
		: context_(argContext), cachedBoolType_(nullptr) { }
		
		const SEM::Type*
		TypeBuilder::getBoolType() {
			if (cachedBoolType_ != nullptr) {
				return cachedBoolType_;
			}
			
			cachedBoolType_ = getBuiltInType(context_,
			                                 context_.getCString("bool"),
			                                 {});
			return cachedBoolType_;
		}
		
		const SEM::Type*
		TypeBuilder::getSizeType() {
			return getBuiltInType(context_,
			                      context_.getCString("size_t"),
			                      {});
		}
		
		const SEM::Type*
		TypeBuilder::getPointerType(const SEM::Type* const elementType) {
			return getBuiltInType(context_,
			                      context_.getCString("__ptr"),
			                      { elementType });
		}
		
		const SEM::Type*
		TypeBuilder::getConstantStaticArrayType(const SEM::Type* elementType,
		                                        size_t arraySize,
		                                        const Debug::SourceLocation& location) {
			auto arraySizeValue = SEM::Value::Constant(Constant::Integer(arraySize),
			                                           getSizeType());
			return getStaticArrayType(elementType,
			                          std::move(arraySizeValue),
			                          location);
		}
		
		const SEM::Type*
		TypeBuilder::getStaticArrayType(const SEM::Type* const elementType,
		                                SEM::Value arraySize,
		                                const Debug::SourceLocation& location) {
			const auto& typeInstance = getBuiltInTypeInstance(context_,
			                                                  context_.getCString("static_array_t"));
			const auto& templateVariables = typeInstance.templateVariables();
			
			SEM::ValueArray templateArgValues;
			templateArgValues.reserve(2);
			
			const auto arraySizeType = templateVariables[1]->type();
			
			if (arraySize.isConstant() && arraySize.constant().isInteger()) {
				arraySize = SEM::Value::Constant(arraySize.constant(),
				                                 arraySizeType);
			}
			
			if (arraySize.type() != arraySizeType) {
				throw ErrorException(makeString("Static array size '%s' has type '%s', which doesn't match expected type '%s', at position %s.",
					arraySize.toString().c_str(),
					arraySize.type()->toString().c_str(),
					arraySizeType->toString().c_str(),
					location.toString().c_str()));
			}
			
			templateArgValues.push_back(SEM::Value::TypeRef(elementType,
			                                                templateVariables[0]->type()->createStaticRefType(elementType)));
			templateArgValues.push_back(std::move(arraySize));
			
			return SEM::Type::Object(&typeInstance, std::move(templateArgValues));
		}
		
		static SEM::ValueArray getFunctionTemplateArgs(Context& context, const SEM::FunctionType functionType) {
			const auto& parameterTypes = functionType.parameterTypes();
			
			SEM::ValueArray templateArgs;
			templateArgs.reserve(1 + parameterTypes.size());
			
			TypeBuilder typeBuilder(context);
			
			const auto boolType = typeBuilder.getBoolType();
			
			auto reducedNoexceptPredicate = reducePredicate(context, functionType.attributes().noExceptPredicate().copy());
			templateArgs.push_back(SEM::Value::PredicateExpr(std::move(reducedNoexceptPredicate), boolType));
			
			const auto typenameType = getBuiltInType(context, context.getCString("typename_t"), {});
			
			const auto returnType = functionType.returnType();
			templateArgs.push_back(SEM::Value::TypeRef(returnType, typenameType->createStaticRefType(returnType)));
			
			for (const auto& paramType: parameterTypes) {
				templateArgs.push_back(SEM::Value::TypeRef(paramType, typenameType->createStaticRefType(paramType)));
			}
			
			return templateArgs;
		}
		
		const SEM::Type*
		TypeBuilder::getPrimitiveCallableType(const SEM::FunctionType functionType,
		                                      const std::string& prefix,
		                                      const std::string& suffix) {
			const auto& parameterTypes = functionType.parameterTypes();
			const auto functionTypeName = makeString("%s%llu_%s", prefix.c_str(), static_cast<unsigned long long>(parameterTypes.size()), suffix.c_str());
			return getBuiltInTypeWithValueArgs(context_,
			                                   context_.getString(functionTypeName),
			                                   getFunctionTemplateArgs(context_, functionType));
		}
		
		const SEM::Type*
		TypeBuilder::getTrivialFunctionPointerType(const SEM::FunctionType functionType) {
			return getPrimitiveCallableType(functionType, "function", "ptr_t");
		}
		
		const SEM::Type*
		TypeBuilder::getTemplatedFunctionPointerType(const SEM::FunctionType functionType) {
			return getPrimitiveCallableType(functionType, "templatedfunction", "ptr_t");
		}
		
		const SEM::Type*
		TypeBuilder::getMethodFunctionPointerType(const SEM::FunctionType functionType) {
			return getPrimitiveCallableType(functionType, "methodfunction", "ptr_t");
		}
		
		const SEM::Type*
		TypeBuilder::getTemplatedMethodFunctionPointerType(const SEM::FunctionType functionType) {
			return getPrimitiveCallableType(functionType, "templatedmethodfunction", "ptr_t");
		}
		
		const SEM::Type*
		TypeBuilder::getVarArgFunctionPointerType(const SEM::FunctionType functionType) {
			return getPrimitiveCallableType(functionType, "varargfunction", "ptr_t");
		}
		
		const SEM::Type*
		TypeBuilder::getFunctionPointerType(const SEM::FunctionType functionType) {
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
		
		const SEM::Type*
		TypeBuilder::getTrivialMethodType(const SEM::FunctionType functionType) {
			return getPrimitiveCallableType(functionType, "method", "t");
		}
		
		const SEM::Type*
		TypeBuilder::getTemplatedMethodType(const SEM::FunctionType functionType) {
			return getPrimitiveCallableType(functionType, "templatedmethod", "t");
		}
		
		const SEM::Type*
		TypeBuilder::getMethodType(const SEM::FunctionType functionType) {
			const auto& attributes = functionType.attributes();
			assert(!attributes.isVarArg());
			assert(attributes.isMethod());
			
			if (attributes.isTemplated()) {
				return getTemplatedMethodType(functionType);
			} else {
				return getTrivialMethodType(functionType);
			}
		}
		
		const SEM::Type*
		TypeBuilder::getInterfaceMethodType(const SEM::FunctionType functionType) {
			const auto& attributes = functionType.attributes();
			(void) attributes;
			assert(!attributes.isVarArg());
			assert(attributes.isMethod());
			
			return getPrimitiveCallableType(functionType, "interfacemethod", "t");
		}
		
		const SEM::Type*
		TypeBuilder::getStaticInterfaceMethodType(const SEM::FunctionType functionType) {
			const auto& attributes = functionType.attributes();
			(void) attributes;
			assert(!attributes.isVarArg());
			assert(!attributes.isMethod());
			
			return getPrimitiveCallableType(functionType, "staticinterfacemethod", "t");
		}
		
	}
	
}

