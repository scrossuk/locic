#include <locic/Debug.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		Context::Context(Debug::Module& pDebugModule, SEM::Context& pSemContext)
			: debugModule_(pDebugModule), semContext_(pSemContext),
			templateRequirementsComplete_(false) {
				validVarArgTypes_.insert("byte_t");
				validVarArgTypes_.insert("ubyte_t");
				validVarArgTypes_.insert("short_t");
				validVarArgTypes_.insert("ushort_t");
				validVarArgTypes_.insert("int_t");
				validVarArgTypes_.insert("uint_t");
				validVarArgTypes_.insert("long_t");
				validVarArgTypes_.insert("ulong_t");
				validVarArgTypes_.insert("longlong_t");
				validVarArgTypes_.insert("ulonglong_t");
				validVarArgTypes_.insert("int8_t");
				validVarArgTypes_.insert("uint8_t");
				validVarArgTypes_.insert("int16_t");
				validVarArgTypes_.insert("uint16_t");
				validVarArgTypes_.insert("int32_t");
				validVarArgTypes_.insert("uint32_t");
				validVarArgTypes_.insert("int64_t");
				validVarArgTypes_.insert("uint64_t");
				validVarArgTypes_.insert("float_t");
				validVarArgTypes_.insert("double_t");
				validVarArgTypes_.insert("longdouble_t");
				validVarArgTypes_.insert("__ptr");
			}
		
		Debug::Module& Context::debugModule() {
			return debugModule_;
		}
		
		ScopeStack& Context::scopeStack() {
			return scopeStack_;
		}
		
		const ScopeStack& Context::scopeStack() const {
			return scopeStack_;
		}
		
		SEM::Context& Context::semContext() {
			return semContext_;
		}
		
		std::vector<TemplateInstTuple>& Context::templateInstantiations() {
			return templateInstantiations_;
		}
		
		bool Context::templateRequirementsComplete() const {
			return templateRequirementsComplete_;
		}
		
		void Context::setTemplateRequirementsComplete() {
			templateRequirementsComplete_ = true;
		}
		
		const std::set<std::string>& Context::validVarArgTypes() const {
			return validVarArgTypes_;
		}
		
		const MethodSet* Context::getMethodSet(MethodSet methodSet) const {
			const auto result = methodSets_.insert(std::move(methodSet));
			return &(*(result.first));
		}
		
		SEM::Value* getSelfValue(Context& context, const Debug::SourceLocation& location) {
			const auto thisTypeInstance = lookupParentType(context.scopeStack());
			const auto thisFunction = lookupParentFunction(context.scopeStack());
			
			if (thisTypeInstance == nullptr) {
				throw ErrorException(makeString("Cannot access 'self' in non-method at %s.", location.toString().c_str()));
			}
			
			if (thisFunction->isStaticMethod()) {
				throw ErrorException(makeString("Cannot access 'self' in static method at %s.",
					location.toString().c_str()));
			}
			
			const auto selfType = thisTypeInstance->selfType();
			const auto selfConstType = thisFunction->isConstMethod() ? selfType->createConstType() : selfType;
			return createSelfRef(context, selfConstType);
		}
		
	}
	
}

