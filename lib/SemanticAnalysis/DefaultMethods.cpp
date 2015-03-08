#include <stdexcept>
#include <vector>

#include <locic/Constant.hpp>
#include <locic/MakeArray.hpp>
#include <locic/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Function* CreateDefaultConstructorDecl(Context& context, SEM::TypeInstance* const typeInstance, const Name& name) {
			const auto semFunction = new SEM::Function(name.copy(), typeInstance->moduleScope().copy());
			semFunction->setRequiresPredicate(typeInstance->requiresPredicate().copy());
			
			semFunction->setMethod(true);
			semFunction->setStaticMethod(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = false;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default constructor only moves, and since moves never
			// throw the constructor never throws.
			const bool isNoExcept = true;
			
			auto constructTypes = typeInstance->constructTypes();
			
			std::vector<SEM::Var*> argVars;
			argVars.reserve(constructTypes.size());
			for (const auto constructType: constructTypes) {
				const bool isLvalConst = false;
				const auto lvalType = makeValueLvalType(context, isLvalConst, constructType);
				argVars.push_back(SEM::Var::Basic(constructType, lvalType));
			}
			
			semFunction->setParameters(std::move(argVars));
			semFunction->setType(SEM::Type::Function(isVarArg, isDynamicMethod, isTemplatedMethod, isNoExcept, typeInstance->selfType(), std::move(constructTypes)));
			return semFunction;
		}
		
		SEM::Function* CreateDefaultMoveDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			const auto semFunction = new SEM::Function(name.copy(), typeInstance->moduleScope().copy());
			semFunction->setRequiresPredicate(typeInstance->requiresPredicate().copy());
			
			semFunction->setMethod(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Move never throws.
			const bool isNoExcept = true;
			
			const auto voidType = getBuiltInType(context.scopeStack(), context.getCString("void_t"), {});
			const auto voidPtrType = getBuiltInType(context.scopeStack(), context.getCString("__ptr"), { voidType });
			
			const auto sizeType = getBuiltInType(context.scopeStack(), context.getCString("size_t"), {});
			
			SEM::TypeArray argTypes;
			argTypes.reserve(2);
			argTypes.push_back(voidPtrType);
			argTypes.push_back(sizeType);
			
			std::vector<SEM::Var*> argVars;
			argVars.reserve(2);
			
			{
				const bool isLvalConst = false;
				const auto lvalType = makeValueLvalType(context, isLvalConst, voidPtrType);
				argVars.push_back(SEM::Var::Basic(voidPtrType, lvalType));
			}
			
			{
				const bool isLvalConst = false;
				const auto lvalType = makeValueLvalType(context, isLvalConst, sizeType);
				argVars.push_back(SEM::Var::Basic(sizeType, lvalType));
			}
			
			semFunction->setParameters(std::move(argVars));
			semFunction->setType(SEM::Type::Function(isVarArg, isDynamicMethod, isTemplatedMethod, isNoExcept, voidType, std::move(argTypes)));
			return semFunction;
		}
		
		bool HasNoExceptImplicitCopy(Context& context, SEM::TypeInstance* typeInstance) {
			if (typeInstance->isUnionDatatype()) {
				for (auto variantTypeInstance: typeInstance->variants()) {
					if (!HasNoExceptImplicitCopy(context, variantTypeInstance)) {
						return false;
					}
				}
				return true;
			} else {
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(typeInstance));
				for (const auto& var: typeInstance->variables()) {
					if (!supportsNoExceptImplicitCopy(context, var->constructType())) {
						return false;
					}
				}
				return true;
			}
		}
		
		bool HasNoExceptExplicitCopy(Context& context, SEM::TypeInstance* typeInstance) {
			if (typeInstance->isUnionDatatype()) {
				for (auto variantTypeInstance: typeInstance->variants()) {
					if (!HasNoExceptExplicitCopy(context, variantTypeInstance)) {
						return false;
					}
				}
				return true;
			} else {
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(typeInstance));
				for (const auto& var: typeInstance->variables()) {
					if (!supportsNoExceptExplicitCopy(context, var->constructType())) {
						return false;
					}
				}
				return true;
			}
		}
		
		bool HasNoExceptCompare(Context& context, SEM::TypeInstance* typeInstance) {
			if (typeInstance->isUnionDatatype()) {
				for (auto variantTypeInstance: typeInstance->variants()) {
					if (!HasNoExceptCompare(context, variantTypeInstance)) {
						return false;
					}
				}
				return true;
			} else {
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(typeInstance));
				for (const auto& var: typeInstance->variables()) {
					if (!supportsNoExceptCompare(context, var->constructType())) {
						return false;
					}
				}
				return true;
			}
		}
		
		SEM::Function* CreateDefaultImplicitCopyDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			const auto semFunction = new SEM::Function(name.copy(), typeInstance->moduleScope().copy());
			
			semFunction->setMethod(true);
			semFunction->setConstPredicate(SEM::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default copy method may throw since it
			// may call child copy methods that throw.
			const bool isNoExcept = HasNoExceptImplicitCopy(context, typeInstance);
			
			semFunction->setType(SEM::Type::Function(isVarArg, isDynamicMethod, isTemplatedMethod, isNoExcept, typeInstance->selfType(), {}));
			return semFunction;
		}
		
		SEM::Function* CreateDefaultExplicitCopyDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			const auto semFunction = new SEM::Function(name.copy(), typeInstance->moduleScope().copy());
			
			semFunction->setMethod(true);
			semFunction->setConstPredicate(SEM::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default copy method may throw since it
			// may call child copy methods that throw.
			const bool isNoExcept = HasNoExceptExplicitCopy(context, typeInstance);
			
			semFunction->setType(SEM::Type::Function(isVarArg, isDynamicMethod, isTemplatedMethod, isNoExcept, typeInstance->selfType(), {}));
			return semFunction;
		}
		
		SEM::Function* CreateDefaultCompareDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			const auto semFunction = new SEM::Function(name.copy(), typeInstance->moduleScope().copy());
			
			semFunction->setMethod(true);
			semFunction->setConstPredicate(SEM::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod =true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default compare method may throw since it
			// may call child compare methods that throw.
			const bool isNoExcept = HasNoExceptCompare(context, typeInstance);
			
			const auto selfType = typeInstance->selfType();
			const auto argType = createReferenceType(context, selfType->createConstType());
			const auto compareResultType = getBuiltInType(context.scopeStack(), context.getCString("compare_result_t"), {});
			
			SEM::TypeArray argTypes;
			argTypes.reserve(1);
			argTypes.push_back(argType);
			
			semFunction->setType(SEM::Type::Function(isVarArg, isDynamicMethod, isTemplatedMethod, isNoExcept, compareResultType, std::move(argTypes)));
			semFunction->setParameters({ SEM::Var::Basic(argType, argType) });
			return semFunction;
		}
		
		SEM::Function* CreateDefaultMethodDecl(Context& context, SEM::TypeInstance* typeInstance, bool isStatic, const Name& name, const Debug::SourceLocation& location) {
			assert(!typeInstance->isClassDecl());
			assert(!typeInstance->isInterface());
			
			const auto canonicalName = CanonicalizeMethodName(name.last());
			if (canonicalName == "create") {
				if (!isStatic) {
					throw ErrorException(makeString("Default method '%s' must be static at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				return CreateDefaultConstructorDecl(context, typeInstance, name);
			} else if (canonicalName == "implicitcopy") {
				if (isStatic) {
					throw ErrorException(makeString("Default method '%s' must be non-static at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				return CreateDefaultImplicitCopyDecl(context, typeInstance, name);
			} else if (canonicalName == "copy") {
				if (isStatic) {
					throw ErrorException(makeString("Default method '%s' must be non-static at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				return CreateDefaultExplicitCopyDecl(context, typeInstance, name);
			} else if (canonicalName == "compare") {
				if (isStatic) {
					throw ErrorException(makeString("Default method '%s' must be non-static at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				return CreateDefaultCompareDecl(context, typeInstance, name);
			}
			
			throw ErrorException(makeString("Unknown default method '%s' at position %s.",
				name.toString().c_str(), location.toString().c_str()));
		}
		
		bool HasDefaultConstructor(Context&, SEM::TypeInstance* const typeInstance) {
			return typeInstance->isDatatype() || typeInstance->isStruct() || typeInstance->isException();
		}
		
		bool HasDefaultMove(Context& context, SEM::TypeInstance* const typeInstance) {
			assert(!typeInstance->isInterface() &&
				!typeInstance->isPrimitive());
			
			// There's only a default move method if the user
			// hasn't specified a custom move method.
			return typeInstance->functions().find(context.getCString("__moveto")) == typeInstance->functions().end();
		}
		
		bool HasDefaultImplicitCopy(Context& context, SEM::TypeInstance* const typeInstance) {
			assert(!typeInstance->isClassDecl() &&
				!typeInstance->isInterface() &&
				!typeInstance->isPrimitive());
			
			if (typeInstance->isUnionDatatype()) {
				for (auto variantTypeInstance: typeInstance->variants()) {
					if (!HasDefaultImplicitCopy(context, variantTypeInstance)) {
						return false;
					}
				}
				return true;
			} else {
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(typeInstance));
				for (auto var: typeInstance->variables()) {
					if (!supportsImplicitCopy(context, var->constructType())) {
						return false;
					}
				}
				return true;
			}
		}
		
		bool HasDefaultExplicitCopy(Context& context, SEM::TypeInstance* const typeInstance) {
			assert(!typeInstance->isClassDecl() &&
				!typeInstance->isInterface() &&
				!typeInstance->isPrimitive());
			
			if (typeInstance->isUnionDatatype()) {
				for (auto variantTypeInstance: typeInstance->variants()) {
					if (!HasDefaultExplicitCopy(context, variantTypeInstance)) {
						return false;
					}
				}
				return true;
			} else {
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(typeInstance));
				for (auto var: typeInstance->variables()) {
					if (!supportsExplicitCopy(context, var->constructType())) {
						return false;
					}
				}
				return true;
			}
		}
		
		bool HasDefaultCompare(Context& context, SEM::TypeInstance* const typeInstance) {
			assert(!typeInstance->isClassDecl() &&
				!typeInstance->isInterface() &&
				!typeInstance->isPrimitive());
			
			if (typeInstance->isUnionDatatype()) {
				for (auto variantTypeInstance: typeInstance->variants()) {
					if (!supportsCompare(context, variantTypeInstance->selfType())) {
						return false;
					}
				}
				return true;
			} else {
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TypeInstance(typeInstance));
				for (auto var: typeInstance->variables()) {
					if (!supportsCompare(context, var->constructType())) {
						return false;
					}
				}
				return true;
			}
		}
		
		void CreateDefaultConstructor(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location) {
			auto functionScope = SEM::Scope::Create();
			
			assert(!typeInstance->isUnionDatatype());
			
			std::vector<SEM::Value> constructValues;
			for (const auto& argVar: function->parameters()) {
				auto argVarValue = createLocalVarRef(context, argVar);
				constructValues.push_back(CallValue(context, GetSpecialMethod(context, std::move(argVarValue), context.getCString("move"), location), {}, location));
			}
			
			auto internalConstructedValue = SEM::Value::InternalConstruct(typeInstance, std::move(constructValues));
			functionScope->statements().push_back(SEM::Statement::Return(std::move(internalConstructedValue)));
			
			function->setScope(std::move(functionScope));
		}
		
		void CreateDefaultMove(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location) {
			const auto selfValue = createSelfRef(context, typeInstance->selfType());
			
			auto functionScope = SEM::Scope::Create();
			
			const auto ptrVar = function->parameters().at(0);
			const auto ptrValue = createLocalVarRef(context, ptrVar);
			
			const auto positionVar = function->parameters().at(1);
			const auto positionValue = createLocalVarRef(context, positionVar);
			
			const auto ubyteType = getBuiltInType(context.scopeStack(), context.getCString("ubyte_t"), {});
			const auto sizeType = getBuiltInType(context.scopeStack(), context.getCString("size_t"), {});
			
			if (typeInstance->isUnionDatatype()) {
				{
					// Move the tag.
					auto tagValue = SEM::Value::UnionTag(selfValue.copy(), ubyteType);
					auto tagRefValue = bindReference(context, std::move(tagValue));
					
					std::vector<SEM::Value> moveArgs = makeArray( ptrValue.copy(), positionValue.copy() );
					
					auto moveResult = CallValue(context, GetSpecialMethod(context, std::move(tagRefValue), context.getCString("__moveto"), location), std::move(moveArgs), location);
					functionScope->statements().push_back(SEM::Statement::ValueStmt(std::move(moveResult)));
				}
				
				// Calculate the position of the union data so that this
				// can be passed to the move methods of the union types.
				auto unionDataOffset = SEM::Value::UnionDataOffset(typeInstance, sizeType);
				const auto unionDataPosition = CallValue(context, GetMethod(context, positionValue.copy(), context.getCString("add"), location), makeArray( std::move(unionDataOffset) ), location);
				
				std::vector<SEM::SwitchCase*> switchCases;
				for (const auto variantTypeInstance: typeInstance->variants()) {
					const auto variantType = variantTypeInstance->selfType();
					const auto caseVar = SEM::Var::Basic(variantType, variantType);
					auto caseVarValue = createLocalVarRef(context, caseVar);
					
					auto caseScope = SEM::Scope::Create();
					std::vector<SEM::Value> moveArgs = makeArray( ptrValue.copy(), unionDataPosition.copy() );
					auto moveResult = CallValue(context, GetSpecialMethod(context, std::move(caseVarValue), context.getCString("__moveto"), location), std::move(moveArgs), location);
					caseScope->statements().push_back(SEM::Statement::ValueStmt(std::move(moveResult)));
					caseScope->statements().push_back(SEM::Statement::ReturnVoid());
					
					switchCases.push_back(new SEM::SwitchCase(caseVar, std::move(caseScope)));
				}
				functionScope->statements().push_back(SEM::Statement::Switch(selfValue.copy(), switchCases, nullptr));
			} else {
				for (size_t i = 0; i < typeInstance->variables().size(); i++) {
					const auto& memberVar = typeInstance->variables().at(i);
					auto memberOffset = SEM::Value::MemberOffset(typeInstance, i, sizeType);
					auto memberPosition = CallValue(context, GetMethod(context, positionValue.copy(), context.getCString("add"), location), makeArray( std::move(memberOffset) ), location);
					
					std::vector<SEM::Value> moveArgs = makeArray( ptrValue.copy(), std::move(memberPosition) );
					auto selfMember = createMemberVarRef(context, selfValue.copy(), memberVar);
					auto moveResult = CallValue(context, GetSpecialMethod(context, std::move(selfMember), context.getCString("__moveto"), location), std::move(moveArgs), location);
					
					functionScope->statements().push_back(SEM::Statement::ValueStmt(std::move(moveResult)));
				}
				
				functionScope->statements().push_back(SEM::Statement::ReturnVoid());
			}
			
			function->setScope(std::move(functionScope));
		}
		
		void CreateDefaultCopy(Context& context, const String& functionName, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location) {
			const auto selfType = typeInstance->selfType();
			const auto selfValue = createSelfRef(context, typeInstance->selfType());
			
			auto functionScope = SEM::Scope::Create();
			
			if (typeInstance->isUnionDatatype()) {
				std::vector<SEM::SwitchCase*> switchCases;
				for (const auto variantTypeInstance: typeInstance->variants()) {
					const auto variantType = variantTypeInstance->selfType();
					const auto caseVar = SEM::Var::Basic(variantType, variantType);
					auto caseVarValue = createLocalVarRef(context, caseVar);
					
					auto caseScope = SEM::Scope::Create();
					auto copyResult = CallValue(context, GetSpecialMethod(context, std::move(caseVarValue), functionName, location), {}, location);
					auto copyResultCast = SEM::Value::Cast(selfType, std::move(copyResult));
					caseScope->statements().push_back(SEM::Statement::Return(std::move(copyResultCast)));
					
					switchCases.push_back(new SEM::SwitchCase(caseVar, std::move(caseScope)));
				}
				functionScope->statements().push_back(SEM::Statement::Switch(selfValue.copy(), std::move(switchCases), nullptr));
			} else {
				std::vector<SEM::Value> copyValues;
				
				for (const auto memberVar: typeInstance->variables()) {
					auto selfMember = tryDissolveValue(context, createMemberVarRef(context, selfValue.copy(), memberVar), location);
					auto copyResult = CallValue(context, GetSpecialMethod(context, std::move(selfMember), functionName, location), {}, location);
					copyValues.push_back(std::move(copyResult));
				}
				
				auto internalConstructedValue = SEM::Value::InternalConstruct(typeInstance, std::move(copyValues));
				functionScope->statements().push_back(SEM::Statement::Return(std::move(internalConstructedValue)));
			}
			
			function->setScope(std::move(functionScope));
		}
		
		void CreateDefaultImplicitCopy(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location) {
			CreateDefaultCopy(context, context.getCString("implicitcopy"), typeInstance, function, location);
		}
		
		void CreateDefaultExplicitCopy(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location) {
			CreateDefaultCopy(context, context.getCString("copy"), typeInstance, function, location);
		}
		
		void CreateDefaultCompare(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location) {
			const auto selfValue = createSelfRef(context, typeInstance->selfType());
			
			const auto compareResultType = getBuiltInType(context.scopeStack(), context.getCString("compare_result_t"), {});
			
			const auto operandVar = function->parameters().at(0);
			const auto operandValue = createLocalVarRef(context, operandVar);
			
			auto functionScope = SEM::Scope::Create();
			
			if (typeInstance->isUnionDatatype()) {
				std::vector<SEM::SwitchCase*> switchCases;
				for (size_t i = 0; i < typeInstance->variants().size(); i++) {
					const auto variantTypeInstance = typeInstance->variants().at(i);
					const auto variantType = variantTypeInstance->selfType();
					const auto caseVar = SEM::Var::Basic(variantType, variantType);
					const auto caseVarValue = createLocalVarRef(context, caseVar);
					
					auto caseScope = SEM::Scope::Create();
					
					std::vector<SEM::SwitchCase*> subSwitchCases;
					for (size_t j = 0; j < typeInstance->variants().size(); j++) {
						const auto subVariantTypeInstance = typeInstance->variants().at(j);
						const auto subVariantType = subVariantTypeInstance->selfType();
						const auto subCaseVar = SEM::Var::Basic(subVariantType, subVariantType);
						auto subCaseVarValue = createLocalVarRef(context, subCaseVar);
						
						auto subCaseScope = SEM::Scope::Create();
						auto minusOneConstant = SEM::Value::Constant(Constant::Integer(-1), compareResultType);
						auto plusOneConstant = SEM::Value::Constant(Constant::Integer(1), compareResultType);
						if (i < j) {
							subCaseScope->statements().push_back(SEM::Statement::Return(std::move(minusOneConstant)));
						} else if (i > j) {
							subCaseScope->statements().push_back(SEM::Statement::Return(std::move(plusOneConstant)));
						} else {
							auto compareResult = CallValue(context, GetMethod(context, caseVarValue.copy(), context.getCString("compare"), location), makeArray( std::move(subCaseVarValue) ), location);
							subCaseScope->statements().push_back(SEM::Statement::Return(std::move(compareResult)));
						}
						
						subSwitchCases.push_back(new SEM::SwitchCase(subCaseVar, std::move(subCaseScope)));
					}
					
					caseScope->statements().push_back(SEM::Statement::Switch(derefValue(operandValue.copy()), subSwitchCases, nullptr));
					
					switchCases.push_back(new SEM::SwitchCase(caseVar, std::move(caseScope)));
				}
				
				functionScope->statements().push_back(SEM::Statement::Switch(selfValue.copy(), switchCases, nullptr));
			} else {
				auto currentScope = functionScope.get();
				
				for (const auto memberVar: typeInstance->variables()) {
					auto selfMember = createMemberVarRef(context, selfValue.copy(), memberVar);
					auto operandMember = createMemberVarRef(context, operandValue.copy(), memberVar);
					
					auto compareResult = CallValue(context, GetMethod(context, std::move(selfMember), context.getCString("compare"), location), makeArray( std::move(operandMember) ), location);
					auto isEqual = CallValue(context, GetMethod(context, compareResult.copy(), context.getCString("isEqual"), location), {}, location);
					
					auto ifTrueScope = SEM::Scope::Create();
					auto ifFalseScope = SEM::Scope::Create();
					
					const auto nextScope = ifTrueScope.get();
					
					ifFalseScope->statements().push_back(SEM::Statement::Return(std::move(compareResult)));
					
					const auto ifStatement = SEM::Statement::If({ new SEM::IfClause(std::move(isEqual), std::move(ifTrueScope)) }, std::move(ifFalseScope));
					currentScope->statements().push_back(ifStatement);
					
					currentScope = nextScope;
				}
				
				auto zeroConstant = SEM::Value::Constant(Constant::Integer(0), compareResultType);
				currentScope->statements().push_back(SEM::Statement::Return(std::move(zeroConstant)));
			}
			
			function->setScope(std::move(functionScope));
		}
		
		void CreateDefaultMethod(Context& context, SEM::TypeInstance* const typeInstance, SEM::Function* const function, const Debug::SourceLocation& location) {
			assert(!typeInstance->isClassDecl() && !typeInstance->isInterface());
			assert(function->isDeclaration());
			
			const auto& name = function->name();
			const auto canonicalName = CanonicalizeMethodName(name.last());
			if (canonicalName == "__moveto") {
				CreateDefaultMove(context, typeInstance, function, location);
			} else if (canonicalName == "create") {
				assert(!typeInstance->isException());
				CreateDefaultConstructor(context, typeInstance, function, location);
			} else if (canonicalName == "implicitcopy") {
				if (!HasDefaultImplicitCopy(context, typeInstance)) {
					throw ErrorException(makeString("Default method '%s' cannot be generated because member types do not support it, at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				CreateDefaultImplicitCopy(context, typeInstance, function, location);
			} else if (canonicalName == "copy") {
				if (!HasDefaultExplicitCopy(context, typeInstance)) {
					throw ErrorException(makeString("Default method '%s' cannot be generated because member types do not support it, at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				CreateDefaultExplicitCopy(context, typeInstance, function, location);
			} else if (canonicalName == "compare") {
				if (!HasDefaultCompare(context, typeInstance)) {
					throw ErrorException(makeString("Default method '%s' cannot be generated because member types do not support it, at position %s.",
						name.toString().c_str(), location.toString().c_str()));
				}
				
				CreateDefaultCompare(context, typeInstance, function, location);
			} else {
				throw std::runtime_error(makeString("Unknown default method '%s'.", name.toString().c_str()));
			}
		}
		
	}
	
}

