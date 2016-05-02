#include <string>

#include <locic/Support/String.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/TemplateVar.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/Value.hpp>

namespace locic {
	
	namespace AST {
		
		TemplateVar*
		TemplateVar::NoSpec(Node<TypeDecl> pType, const String& pName) {
			TemplateVar* typeVar = new TemplateVar();
			typeVar->type_ = std::move(pType);
			typeVar->name_ = pName;
			typeVar->specType_ = makeNode(Debug::SourceLocation::Null(),
			                              TypeDecl::Void());
			return typeVar;
		}
		
		TemplateVar*
		TemplateVar::WithSpec(Node<TypeDecl> pType, const String& pName,
		                          Node<TypeDecl> pSpecType) {
			assert(!pSpecType.isNull());
			TemplateVar* typeVar = new TemplateVar();
			typeVar->type_ = std::move(pType);
			typeVar->name_ = pName;
			typeVar->specType_ = std::move(pSpecType);
			return typeVar;
		}
		
		TemplateVar::TemplateVar()
		: index_(-1) { }
		
		String TemplateVar::name() const {
			return name_;
		}
		
		Node<TypeDecl>& TemplateVar::type() {
			return type_;
		}
		
		const Node<TypeDecl>& TemplateVar::type() const {
			return type_;
		}
		
		Node<TypeDecl>& TemplateVar::specType() {
			return specType_;
		}
		
		const Node<TypeDecl>& TemplateVar::specType() const {
			return specType_;
		}
		
		size_t TemplateVar::index() const {
			return index_;
		}
		
		void TemplateVar::setIndex(const size_t pIndex) {
			assert(index() == (size_t)-1);
			index_ = pIndex;
		}
		
		std::string TemplateVar::toString() const {
			return makeString("TemplateVar(type = %s, name = %s, specType = %s)",
			                  type().toString().c_str(), name().c_str(),
			                  specType().toString().c_str());
		}
		
	}
	
}

