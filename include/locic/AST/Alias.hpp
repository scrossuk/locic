#ifndef LOCIC_AST_ALIAS_HPP
#define LOCIC_AST_ALIAS_HPP

#include <string>

#include <locic/AST/GlobalStructure.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/TemplatedObject.hpp>
#include <locic/AST/TemplateVar.hpp>
#include <locic/AST/TemplateVarArray.hpp>
#include <locic/AST/Value.hpp>
#include <locic/AST/ValueDecl.hpp>

#include <locic/Debug/SourceLocation.hpp>

#include <locic/Support/FastMap.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		class Context;
		class Type;
		
		class Alias final: public TemplatedObject {
		public:
			Alias(const String& pName, Node<ValueDecl> pValue,
			          const Debug::SourceLocation& location);
			~Alias();
			
			const Debug::SourceLocation& location() const;
			void setLocation(Debug::SourceLocation location);
			
			String name() const;
			const Node<TemplateVarList>& templateVariableDecls() const;
			const Node<RequireSpecifier>& requireSpecifier() const;
			const Node<ValueDecl>& valueDecl() const;
			
			void setRequireSpecifier(Node<RequireSpecifier> pRequireSpecifier);
			void setTemplateVariableDecls(Node<TemplateVarList> pTemplateVariables);
			
			Context& context() const;
			void setContext(Context& context);
			
			GlobalStructure& parent();
			const GlobalStructure& parent() const;
			void setParent(GlobalStructure parent);
			
			const Name& fullName() const;
			void setFullName(Name fullName);
			
			const Type* type() const;
			void setType(const Type* type);
			
			Value selfRefValue(ValueArray templateArguments) const;
			
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
			const Type* selfRefType(ValueArray templateArguments) const;
			ValueArray selfTemplateArgs() const;
			
			TemplateVarArray& templateVariables();
			const TemplateVarArray& templateVariables() const;
			
			FastMap<String, TemplateVar*>& namedTemplateVariables();
			
			const Predicate& requiresPredicate() const;
			void setRequiresPredicate(Predicate predicate);
			
			const Predicate& noexceptPredicate() const;
			
			const Value& value() const;
			void setValue(Value value);
			
			std::string toString() const;
			
		private:
			Debug::SourceLocation location_;
			String name_;
			Node<TemplateVarList> templateVariableDecls_;
			Node<RequireSpecifier> requireSpecifier_;
			Node<ValueDecl> valueDecl_;
			Context* context_;
			Optional<GlobalStructure> parent_;
			Name fullName_;
			TemplateVarArray templateVars_;
			FastMap<String, TemplateVar*> namedTemplateVariables_;
			Predicate requiresPredicate_;
			Predicate noexceptPredicate_;
			const Type* type_;
			Optional<Value> value_;
			
		};
		
	}
	
}

#endif
