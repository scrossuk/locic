#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/functional/hash.hpp>

#include <locic/Array.hpp>
#include <locic/Debug.hpp>
#include <locic/StableSet.hpp>
#include <locic/StringHost.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		class ContextImpl {
		public:
			ContextImpl(const StringHost& argStringHost, Debug::Module& pDebugModule, SEM::Context& pSemContext)
			: stringHost(argStringHost), debugModule(pDebugModule), semContext(pSemContext),
			templateRequirementsComplete(false) {
				validVarArgTypes.insert(String(stringHost, "byte_t"));
				validVarArgTypes.insert(String(stringHost, "ubyte_t"));
				validVarArgTypes.insert(String(stringHost, "short_t"));
				validVarArgTypes.insert(String(stringHost, "ushort_t"));
				validVarArgTypes.insert(String(stringHost, "int_t"));
				validVarArgTypes.insert(String(stringHost, "uint_t"));
				validVarArgTypes.insert(String(stringHost, "long_t"));
				validVarArgTypes.insert(String(stringHost, "ulong_t"));
				validVarArgTypes.insert(String(stringHost, "longlong_t"));
				validVarArgTypes.insert(String(stringHost, "ulonglong_t"));
				validVarArgTypes.insert(String(stringHost, "int8_t"));
				validVarArgTypes.insert(String(stringHost, "uint8_t"));
				validVarArgTypes.insert(String(stringHost, "int16_t"));
				validVarArgTypes.insert(String(stringHost, "uint16_t"));
				validVarArgTypes.insert(String(stringHost, "int32_t"));
				validVarArgTypes.insert(String(stringHost, "uint32_t"));
				validVarArgTypes.insert(String(stringHost, "int64_t"));
				validVarArgTypes.insert(String(stringHost, "uint64_t"));
				validVarArgTypes.insert(String(stringHost, "float_t"));
				validVarArgTypes.insert(String(stringHost, "double_t"));
				validVarArgTypes.insert(String(stringHost, "longdouble_t"));
				validVarArgTypes.insert(String(stringHost, "__ptr"));
			}
			
			template <typename Key, typename Value>
			struct hashPair {
				std::size_t operator()(const std::pair<Key, Value>& pair) const {
					std::size_t seed = 0;
					std::hash<Key> keyHashFn;
					boost::hash_combine(seed, keyHashFn(pair.first));
					std::hash<Value> valueHashFn;
					boost::hash_combine(seed, valueHashFn(pair.second));
					return seed;
				}
			};
			
			const StringHost& stringHost;
			Debug::Module& debugModule;
			ScopeStack scopeStack;
			SEM::Context& semContext;
			bool templateRequirementsComplete;
			std::vector<TemplateInstTuple> templateInstantiations;
			std::set<String> validVarArgTypes;
			mutable StableSet<MethodSet> methodSets;
			std::unordered_map<std::pair<const SEM::Type*, String>, bool, hashPair<const SEM::Type*, String>> capabilities;
			std::unordered_map<std::pair<const SEM::Type*, size_t>, const MethodSet*, hashPair<const SEM::Type*, size_t>> methodSetMap;
		};
		
		Context::Context(const StringHost& argStringHost, Debug::Module& pDebugModule, SEM::Context& pSemContext)
		: impl_(new ContextImpl(argStringHost, pDebugModule, pSemContext)) { }
		
		Context::~Context() { }
		
		String Context::getCString(const char* const cString) const {
			return String(impl_->stringHost, cString);
		}
		
		Debug::Module& Context::debugModule() {
			return impl_->debugModule;
		}
		
		ScopeStack& Context::scopeStack() {
			return impl_->scopeStack;
		}
		
		const ScopeStack& Context::scopeStack() const {
			return impl_->scopeStack;
		}
		
		SEM::Context& Context::semContext() {
			return impl_->semContext;
		}
		
		const MethodSet* Context::findMethodSet(const SEM::Type* const type) const {
			assert(type->isObject());
			const auto typeInstance = type->getObjectType();
			
			const auto iterator = impl_->methodSetMap.find(std::make_pair(type, typeInstance->functions().size()));
			return iterator != impl_->methodSetMap.end() ? iterator->second : nullptr;
		}
		
		void Context::addMethodSet(const SEM::Type* const type, const MethodSet* const methodSet) {
			assert(type->isObject());
			const auto typeInstance = type->getObjectType();
			
			impl_->methodSetMap.insert(std::make_pair(std::make_pair(type, typeInstance->functions().size()), methodSet));
		}
		
		std::vector<TemplateInstTuple>& Context::templateInstantiations() {
			return impl_->templateInstantiations;
		}
		
		bool Context::templateRequirementsComplete() const {
			return impl_->templateRequirementsComplete;
		}
		
		void Context::setTemplateRequirementsComplete() {
			impl_->templateRequirementsComplete = true;
		}
		
		const std::set<String>& Context::validVarArgTypes() const {
			return impl_->validVarArgTypes;
		}
		
		const MethodSet* Context::getMethodSet(MethodSet methodSet) const {
			const auto result = impl_->methodSets.insert(std::move(methodSet));
			return &(*(result.first));
		}
		
		Optional<bool> Context::getCapability(const SEM::Type* const type, const String& capability) const {
			const auto iterator = impl_->capabilities.find(std::make_pair(type, capability));
			if (iterator != impl_->capabilities.end()) {
				return make_optional(iterator->second);
			} else {
				return Optional<bool>();
			}
		}
		
		void Context::setCapability(const SEM::Type* type, const String& capability, const bool isCapable) {
			(void) impl_->capabilities.insert(std::make_pair(std::make_pair(type, capability), isCapable));
		}
		
		SEM::Value getSelfValue(Context& context, const Debug::SourceLocation& location) {
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
			
			// TODO: we need to actually put the predicate in the type that's returned!
			const auto selfIsConst = evaluatePredicate(context, thisFunction->constPredicate(), SEM::TemplateVarMap());
			
			const auto selfConstType = selfIsConst ? selfType->createConstType() : selfType;
			return createSelfRef(context, selfConstType);
		}
		
		SEM::Value getThisValue(Context& context, const Debug::SourceLocation& location) {
			const auto thisTypeInstance = lookupParentType(context.scopeStack());
			const auto thisFunction = lookupParentFunction(context.scopeStack());
			
			if (thisTypeInstance == nullptr) {
				throw ErrorException(makeString("Cannot access 'this' in non-method at %s.",
					location.toString().c_str()));
			}
			
			if (thisFunction->isStaticMethod()) {
				throw ErrorException(makeString("Cannot access 'this' in static method at %s.",
					location.toString().c_str()));
			}
			
			const auto selfType = thisTypeInstance->selfType();
			
			// TODO: we need to actually put the predicate in the type that's returned!
			const auto selfIsConst = evaluatePredicate(context, thisFunction->constPredicate(), SEM::TemplateVarMap());
			
			const auto selfConstType = selfIsConst ? selfType->createConstType() : selfType;
			return SEM::Value::This(getBuiltInType(context.scopeStack(), context.getCString("__ptr"), { selfConstType }));
		}
		
	}
	
}

