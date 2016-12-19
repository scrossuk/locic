#include <string>

#include <locic/Support/String.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/TemplateVar.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/ValueDecl.hpp>

namespace locic {
	
	namespace AST {
		
		TemplateVar*
		TemplateVar::NoSpec(Node<TypeDecl> pType, const String& pName) {
			TemplateVar* typeVar = new TemplateVar();
			typeVar->typeDecl_ = std::move(pType);
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
			typeVar->typeDecl_ = std::move(pType);
			typeVar->name_ = pName;
			typeVar->specType_ = std::move(pSpecType);
			return typeVar;
		}
		
		TemplateVar::TemplateVar()
		: context_(nullptr), type_(nullptr), selfRefType_(nullptr),
		index_(-1), isVirtual_(false) { }
		
		SEM::Context& TemplateVar::context() const {
			assert(context_ != nullptr);
			return *context_;
		}
		
		void TemplateVar::setContext(SEM::Context& pContext) {
			assert(context_ == nullptr);
			context_ = &pContext;
		}
		
		String TemplateVar::name() const {
			return name_;
		}
		
		const Name& TemplateVar::fullName() const {
			assert(!fullName_.empty());
			return fullName_;
		}
		
		void TemplateVar::setFullName(Name pFullName) {
			assert(fullName_.empty());
			fullName_ = std::move(pFullName);
		}
		
		size_t TemplateVar::index() const {
			assert(index_ != (size_t)-1);
			return index_;
		}
		
		void TemplateVar::setIndex(const size_t pIndex) {
			assert(index_ == (size_t)-1);
			index_ = pIndex;
		}
		
		Node<TypeDecl>& TemplateVar::typeDecl() {
			return typeDecl_;
		}
		
		const Node<TypeDecl>& TemplateVar::typeDecl() const {
			return typeDecl_;
		}
		
		Node<TypeDecl>& TemplateVar::specType() {
			return specType_;
		}
		
		const Node<TypeDecl>& TemplateVar::specType() const {
			return specType_;
		}
		
		const Type* TemplateVar::type() const {
			assert(type_ != nullptr);
			return type_;
		}
		
		void TemplateVar::setType(const Type* pType) {
			assert(type_ == nullptr && pType != nullptr);
			type_ = pType;
			if (type_->isBuiltInTypename()) {
				selfRefType_ = Type::TemplateVarRef(this);
				type_ = type_->createStaticRefType(selfRefType_);
			}
		}
		
		bool TemplateVar::isVirtual() const {
			return isVirtual_;
		}
		
		void TemplateVar::setVirtual(bool pIsVirtual) {
			isVirtual_ = pIsVirtual;
		}
		
		SEM::Value TemplateVar::selfRefValue() const {
			if (type()->isBuiltInTypename()) {
				return SEM::Value::TypeRef(selfRefType(), type());
			} else {
				return SEM::Value::TemplateVarRef(this, type());
			}
		}
		
		const Type* TemplateVar::selfRefType() const {
			assert(type()->isBuiltInTypename());
			return selfRefType_;
		}
		
		void TemplateVar::setDebugInfo(Debug::TemplateVarInfo pDebugInfo) {
			debugInfo_ = make_optional(pDebugInfo);
		}
		
		Optional<Debug::TemplateVarInfo> TemplateVar::debugInfo() const {
			return debugInfo_;
		}
		
		std::string TemplateVar::toString() const {
			return makeString("TemplateVar(type = %s, name = %s, specType = %s)",
			                  typeDecl().toString().c_str(), name().c_str(),
			                  specType().toString().c_str());
		}
		
	}
	
}

