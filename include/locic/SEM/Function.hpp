#ifndef LOCIC_SEM_FUNCTION_HPP
#define LOCIC_SEM_FUNCTION_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <locic/AST/Function.hpp>
#include <locic/AST/Node.hpp>

#include <locic/Debug/FunctionInfo.hpp>

#include <locic/SEM/FunctionType.hpp>
#include <locic/SEM/GlobalStructure.hpp>
#include <locic/SEM/ModuleScope.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplatedObject.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/TemplateVarArray.hpp>

#include <locic/Support/FastMap.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/Optional.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	template <typename Key, typename Value>
	class Map;

	namespace SEM {
	
		class Scope;
		class TypeInstance;
		class Var;
		
		/**
		 * \brief Function
		 * 
		 * This class encapsulates all of the properties of
		 * a function, including:
		 * 
		 * - A name
		 * - Whether it's a static method, definition etc.
		 * - Template variables
		 * - Parameter variables
		 * - Const predicate
		 * - Require predicate
		 * - A scope containing statements
		 */
		class Function final: public TemplatedObject {
			public:
				Function(GlobalStructure parent,
				         Name name, ModuleScope moduleScope);
				
				Function(GlobalStructure parent,
				         AST::Node<AST::Function> function,
				         ModuleScope moduleScope);
				
				GlobalStructure& parent();
				const GlobalStructure& parent() const;
				
				Namespace& nameSpace();
				const Namespace& nameSpace() const;
				
				const AST::Node<AST::Function>& astFunction() const;
				
				const Name& name() const;
				String canonicalName() const;
				
				void setType(FunctionType type);
				const FunctionType& type() const;
				
				const ModuleScope& moduleScope() const;
				
				bool isDeclaration() const;
				
				bool isDefinition() const;
				
				/**
				 * \brief Get/set whether this function is auto-generated.
				 * 
				 * Default functions are generated for various types
				 * and provide a generic implementation (e.g. copy an
				 * object by copying each member value) that doesn't
				 * have to be specified manually.
				 */
				void setDefault(bool pIsDefault);
				bool isDefault() const;
				
				/**
				 * \brief Get/set whether this function is primitive.
				 * 
				 * A primitive function is an 'axiom' of the language,
				 * such as the methods of type 'int'.
				 */
				void setPrimitive(bool pIsPrimitive);
				bool isPrimitive() const;
				
				void setMethod(bool pIsMethod);
				bool isMethod() const;
				
				void setStaticMethod(bool pIsStaticMethod);
				bool isStaticMethod() const;
				
				TemplateVarArray& templateVariables();
				const TemplateVarArray& templateVariables() const;
				
				FastMap<String, TemplateVar*>& namedTemplateVariables();
				const FastMap<String, TemplateVar*>& namedTemplateVariables() const;
				
				const Predicate& constPredicate() const;
				void setConstPredicate(Predicate predicate);
				
				const Predicate& requiresPredicate() const;
				void setRequiresPredicate(Predicate predicate);
				
				const Predicate& noexceptPredicate() const;
				
				void setParameters(std::vector<Var*> pParameters);
				const std::vector<Var*>& parameters() const;
				
				FastMap<String, Var*>& namedVariables();
				const FastMap<String, Var*>& namedVariables() const;
				
				void setScope(std::unique_ptr<Scope> newScope);
				const Scope& scope() const;
				
				void setDebugInfo(Debug::FunctionInfo debugInfo);
				const Optional<Debug::FunctionInfo>& debugInfo() const;
				
				std::string toString() const;
				
			private:
				GlobalStructure parent_;
				AST::Node<AST::Function> function_;
				bool isDefault_, isPrimitive_;
				bool isMethod_, isStaticMethod_;
				FunctionType type_;
				Name name_;
				Optional<Debug::FunctionInfo> debugInfo_;
				
				TemplateVarArray templateVariables_;
				FastMap<String, TemplateVar*> namedTemplateVariables_;
				Predicate constPredicate_;
				Predicate requiresPredicate_;
				std::vector<Var*> parameters_;
				FastMap<String, Var*> namedVariables_;
				
				ModuleScope moduleScope_;
				std::unique_ptr<Scope> scope_;
				
		};
		
	}
	
}

#endif
