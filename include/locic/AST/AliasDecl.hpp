#ifndef LOCIC_AST_ALIASDECL_HPP
#define LOCIC_AST_ALIASDECL_HPP

#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/TemplateTypeVar.hpp>
#include <locic/AST/Value.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace SEM {
		
		class Alias;
		
	}
	
	namespace AST {
	
		class AliasDecl {
		public:
			AliasDecl(const String& pName, AST::Node<Value> pValue);
			~AliasDecl();
			
			String name() const;
			const Node<TemplateTypeVarList>& templateVariables() const;
			const Node<RequireSpecifier>& requireSpecifier() const;
			const AST::Node<AST::Value>& value() const;
			
			void setRequireSpecifier(Node<RequireSpecifier> pRequireSpecifier);
			void setTemplateVariables(Node<TemplateTypeVarList> pTemplateVariables);
			
			void setAlias(SEM::Alias& alias);
			
			SEM::Alias& alias();
			const SEM::Alias& alias() const;
			
			std::string toString() const;
			
		private:
			String name_;
			Node<TemplateTypeVarList> templateVariables_;
			Node<RequireSpecifier> requireSpecifier_;
			AST::Node<AST::Value> value_;
			SEM::Alias* alias_;
			
		};
		
	}
	
}

#endif
