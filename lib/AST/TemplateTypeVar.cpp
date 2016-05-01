#include <string>

#include <locic/Support/String.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/TemplateTypeVar.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/Value.hpp>

namespace locic {
	
	namespace AST {
		
		TemplateTypeVar*
		TemplateTypeVar::NoSpec(Node<TypeDecl> pType, const String& pName) {
			TemplateTypeVar* typeVar = new TemplateTypeVar();
			typeVar->type_ = std::move(pType);
			typeVar->name_ = pName;
			typeVar->specType_ = makeNode(Debug::SourceLocation::Null(),
			                              TypeDecl::Void());
			return typeVar;
		}
		
		TemplateTypeVar*
		TemplateTypeVar::WithSpec(Node<TypeDecl> pType, const String& pName,
		                          Node<TypeDecl> pSpecType) {
			assert(!pSpecType.isNull());
			TemplateTypeVar* typeVar = new TemplateTypeVar();
			typeVar->type_ = std::move(pType);
			typeVar->name_ = pName;
			typeVar->specType_ = std::move(pSpecType);
			return typeVar;
		}
		
		TemplateTypeVar::TemplateTypeVar()
		: index_(-1) { }
		
		String TemplateTypeVar::name() const {
			return name_;
		}
		
		Node<TypeDecl>& TemplateTypeVar::type() {
			return type_;
		}
		
		const Node<TypeDecl>& TemplateTypeVar::type() const {
			return type_;
		}
		
		Node<TypeDecl>& TemplateTypeVar::specType() {
			return specType_;
		}
		
		const Node<TypeDecl>& TemplateTypeVar::specType() const {
			return specType_;
		}
		
		size_t TemplateTypeVar::index() const {
			return index_;
		}
		
		void TemplateTypeVar::setIndex(const size_t pIndex) {
			assert(index() == (size_t)-1);
			index_ = pIndex;
		}
		
		std::string TemplateTypeVar::toString() const {
			return makeString("TemplateTypeVar(type = %s, name = %s, specType = %s)",
			                  type().toString().c_str(), name().c_str(),
			                  specType().toString().c_str());
		}
		
	}
	
}

