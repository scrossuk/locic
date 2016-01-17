#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/functional/hash.hpp>

#include <locic/Frontend/DiagnosticReceiver.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/Support/Array.hpp>
#include <locic/Support/SharedMaps.hpp>
#include <locic/Support/StableSet.hpp>
#include <locic/Support/StringHost.hpp>

#include <locic/SemanticAnalysis/AliasTypeResolver.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TemplateInst.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		class ContextImpl {
		public:
			ContextImpl(Context& context, const SharedMaps& argSharedMaps,
			            Debug::Module& pDebugModule, SEM::Context& pSemContext,
			            DiagnosticReceiver& argDiagReceiver)
			: aliasTypeResolver(context),
			sharedMaps(argSharedMaps),
			stringHost(sharedMaps.stringHost()),
			debugModule(pDebugModule),
			semContext(pSemContext),
			diagReceiver(argDiagReceiver),
			typeBuilder(context),
			methodSetsComplete(false),
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
				validVarArgTypes.insert(String(stringHost, "ptr_t"));
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
			
			AliasTypeResolver aliasTypeResolver;
			const SharedMaps& sharedMaps;
			const StringHost& stringHost;
			Debug::Module& debugModule;
			ScopeStack scopeStack;
			SEM::Context& semContext;
			DiagnosticReceiver& diagReceiver;
			TypeBuilder typeBuilder;
			bool methodSetsComplete;
			bool templateRequirementsComplete;
			std::vector<TemplateInst> templateInstantiations;
			std::set<String> validVarArgTypes;
			mutable StableSet<MethodSet> methodSets;
			std::unordered_map<std::pair<const SEM::Type*, String>, bool, hashPair<const SEM::Type*, String>> capabilities;
			
			std::unordered_map<const SEM::Type*, const MethodSet*> objectMethodSetMap;
			std::unordered_map<std::pair<const SEM::TemplatedObject*, const SEM::Type*>, const MethodSet*,
				hashPair<const SEM::TemplatedObject*, const SEM::Type*>> templateVarMethodSetMap;
			
			std::vector<std::pair<const SEM::Type*, const SEM::Type*>> assumedSatisfyPairs;
			std::vector<std::pair<const SEM::TemplateVar*, const SEM::Predicate*>> computingMethodSetTemplateVars;
		};
		
		Context::Context(const SharedMaps& argSharedMaps, Debug::Module& argDebugModule,
		                 SEM::Context& argSemContext, DiagnosticReceiver& diagReceiver)
		: impl_(new ContextImpl(*this, argSharedMaps, argDebugModule, argSemContext,
		                        diagReceiver)) { }
		
		Context::~Context() { }
		
		AliasTypeResolver& Context::aliasTypeResolver() {
			return impl_->aliasTypeResolver;
		}
		
		const SharedMaps& Context::sharedMaps() const {
			return impl_->sharedMaps;
		}
		
		String Context::getCString(const char* const cString) const {
			return String(impl_->stringHost, cString);
		}
		
		String Context::getString(const std::string& string) const {
			return String(impl_->stringHost, string);
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
		
		TypeBuilder& Context::typeBuilder() {
			return impl_->typeBuilder;
		}
		
		const MethodSet* Context::findMethodSet(const SEM::TemplatedObject* const templatedObject,
		                                        const SEM::Type* const type) const {
			assert(methodSetsComplete());
			assert(type->isObject() || type->isTemplateVar());
			
			if (type->isObject()) {
				const auto iterator = impl_->objectMethodSetMap.find(type);
				return iterator != impl_->objectMethodSetMap.end() ? iterator->second : nullptr;
			} else {
				assert(templatedObject != nullptr);
				const auto iterator = impl_->templateVarMethodSetMap.find(std::make_pair(templatedObject, type));
				return iterator != impl_->templateVarMethodSetMap.end() ? iterator->second : nullptr;
			}
		}
		
		void Context::addMethodSet(const SEM::TemplatedObject* const templatedObject,
		                           const SEM::Type* const type,
		                           const MethodSet* const methodSet) {
			assert(methodSetsComplete());
			assert(type->isObject() || type->isTemplateVar());
			
			if (type->isObject()) {
				const auto result = impl_->objectMethodSetMap.insert(std::make_pair(type, methodSet));
				if (!result.second) {
					// Overwrite any existing element.
					result.first->second = methodSet;
				}
			} else {
				assert(templatedObject != nullptr);
				const auto result = impl_->templateVarMethodSetMap.insert(std::make_pair(std::make_pair(templatedObject, type), methodSet));
				if (!result.second) {
					// Overwrite any existing element.
					result.first->second = methodSet;
				}
			}
		}
		
		std::vector<TemplateInst>& Context::templateInstantiations() {
			return impl_->templateInstantiations;
		}
		
		bool Context::templateRequirementsComplete() const {
			return impl_->templateRequirementsComplete;
		}
		
		void Context::setTemplateRequirementsComplete() {
			assert(!impl_->templateRequirementsComplete);
			impl_->templateRequirementsComplete = true;
		}
		
		bool Context::methodSetsComplete() const {
			return impl_->methodSetsComplete;
		}
		
		void Context::setMethodSetsComplete() {
			assert(!impl_->methodSetsComplete);
			impl_->methodSetsComplete = true;
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
		
		bool Context::isComputingMethodSet(const SEM::TemplateVar* const templateVar, const SEM::Predicate& predicate) const {
			for (const auto& computingMethodSetPair: impl_->computingMethodSetTemplateVars) {
				if (templateVar == computingMethodSetPair.first && predicate == *(computingMethodSetPair.second)) {
					return true;
				}
			}
			return false;
		}
		
		void Context::pushComputingMethodSet(const SEM::TemplateVar* const  templateVar, const SEM::Predicate& predicate) {
			impl_->computingMethodSetTemplateVars.push_back(std::make_pair(templateVar, &predicate));
		}
		
		void Context::popComputingMethodSet() {
			impl_->computingMethodSetTemplateVars.pop_back();
		}
		
		bool Context::isAssumedSatisfies(const SEM::Type* const checkType, const SEM::Type* const requireType) const {
			const auto pair = std::make_pair(checkType, requireType);
			for (const auto& assumedSatisfyPair: impl_->assumedSatisfyPairs) {
				if (pair == assumedSatisfyPair) {
					return true;
				}
			}
			return false;
		}
		
		void Context::pushAssumeSatisfies(const SEM::Type* const checkType, const SEM::Type* const requireType) {
			const auto pair = std::make_pair(checkType, requireType);
			impl_->assumedSatisfyPairs.push_back(pair);
		}
		
		void Context::popAssumeSatisfies() {
			impl_->assumedSatisfyPairs.pop_back();
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
			const auto selfConstType = selfType->createTransitiveConstType(thisFunction->constPredicate().copy());
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
			const auto selfConstType = selfType->createTransitiveConstType(thisFunction->constPredicate().copy());
			return SEM::Value::This(getBuiltInType(context, context.getCString("ptr_t"), { selfConstType }));
		}
		
	}
	
}

