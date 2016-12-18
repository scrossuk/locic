#ifndef LOCIC_AST_ALIAS_HPP
#define LOCIC_AST_ALIAS_HPP

#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/TemplateVar.hpp>
#include <locic/AST/TemplateVarArray.hpp>
#include <locic/AST/Value.hpp>

#include <locic/Debug/SourceLocation.hpp>

#include <locic/SEM/GlobalStructure.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplatedObject.hpp>
#include <locic/SEM/Value.hpp>

#include <locic/Support/FastMap.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace SEM {
		
		class Context;
		class Type;
		
	}
	
	namespace AST {
		
		class Alias final: public SEM::TemplatedObject {
		public:
			Alias(const String& pName, AST::Node<Value> pValue,
			          const Debug::SourceLocation& location);
			~Alias();
			
			const Debug::SourceLocation& location() const;
			void setLocation(Debug::SourceLocation location);
			
			String name() const;
			const Node<TemplateVarList>& templateVariableDecls() const;
			const Node<RequireSpecifier>& requireSpecifier() const;
			const AST::Node<AST::Value>& valueDecl() const;
			
			void setRequireSpecifier(Node<RequireSpecifier> pRequireSpecifier);
			void setTemplateVariableDecls(Node<TemplateVarList> pTemplateVariables);
			
			SEM::Context& context() const;
			void setContext(SEM::Context& context);
			
			SEM::GlobalStructure& parent();
			const SEM::GlobalStructure& parent() const;
			void setParent(SEM::GlobalStructure parent);
			
			const Name& fullName() const;
			void setFullName(Name fullName);
			
			const SEM::Type* type() const;
			void setType(const SEM::Type* type);
			
			SEM::Value selfRefValue(SEM::ValueArray templateArguments) const;
			
			/**
			 * \brief Get type of 'self'.
			 * 
			 * This creates an alias type with template
			 * arguments that refer to the type alias'
			 * own template variables.
			 * 
			 * For example, given:
			 * 
			 *     template <typename A, typename B>
			 *     using SomeAlias = ...;
			 * 
			 * ...this function will return:
			 * 
			 *     SomeAlias<A, B>
			 * 
			 */
			const SEM::Type* selfRefType(SEM::ValueArray templateArguments) const;
			SEM::ValueArray selfTemplateArgs() const;
			
			TemplateVarArray& templateVariables();
			const TemplateVarArray& templateVariables() const;
			
			FastMap<String, TemplateVar*>& namedTemplateVariables();
			
			const SEM::Predicate& requiresPredicate() const;
			void setRequiresPredicate(SEM::Predicate predicate);
			
			const SEM::Predicate& noexceptPredicate() const;
			
			const SEM::Value& value() const;
			void setValue(SEM::Value value);
			
			std::string toString() const;
			
		private:
			Debug::SourceLocation location_;
			String name_;
			Node<TemplateVarList> templateVariableDecls_;
			Node<RequireSpecifier> requireSpecifier_;
			Node<Value> valueDecl_;
			SEM::Context* context_;
			Optional<SEM::GlobalStructure> parent_;
			Name fullName_;
			TemplateVarArray templateVars_;
			FastMap<String, TemplateVar*> namedTemplateVariables_;
			SEM::Predicate requiresPredicate_;
			SEM::Predicate noexceptPredicate_;
			const SEM::Type* type_;
			Optional<SEM::Value> value_;
			
		};
		
	}
	
}

#endif
