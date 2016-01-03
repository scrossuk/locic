#include <locic/AST.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/NamespaceBuilder.hpp>
#include <locic/Parser/TokenReader.hpp>

namespace locic {
	
	namespace Parser {
		
		NamespaceBuilder::NamespaceBuilder(const TokenReader& reader)
		: reader_(reader) { }
		
		NamespaceBuilder::~NamespaceBuilder() { }
		
		AST::Node<AST::NamespaceDecl>
		NamespaceBuilder::makeNamespace(const String name,
		                                AST::Node<AST::NamespaceData> data,
		                                const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::NamespaceDecl(name, data));
		}
		
		AST::Node<AST::NamespaceData>
		NamespaceBuilder::makeNamespaceData(AST::NamespaceData data,
		                                    const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::NamespaceData(std::move(data)));
		}
		
		AST::Node<AST::AliasDecl>
		NamespaceBuilder::makeAlias(const String name, AST::Node<AST::Value> value,
		                            const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::AliasDecl(name, value));
		}
		
		AST::Node<AST::ModuleScope>
		NamespaceBuilder::makeUnnamedExport(AST::Node<AST::NamespaceData> data,
		                                    const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::ModuleScope::Export(data));
		}
		
		AST::Node<AST::ModuleScope>
		NamespaceBuilder::makeUnnamedImport(AST::Node<AST::NamespaceData> data,
		                                    const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::ModuleScope::Import(data));
		}
		
		AST::Node<AST::ModuleScope>
		NamespaceBuilder::makeNamedExport(AST::Node<AST::StringList> name,
		                                  AST::Node<Version> version,
		                                  AST::Node<AST::NamespaceData> data,
		                                  const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::ModuleScope::NamedExport(name, version, data));
		}
		
		AST::Node<AST::ModuleScope>
		NamespaceBuilder::makeNamedImport(AST::Node<AST::StringList> name,
		                                  AST::Node<Version> version,
		                                  AST::Node<AST::NamespaceData> data,
		                                  const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, AST::ModuleScope::NamedImport(name, version, data));
		}
		
		AST::Node<AST::StringList>
		NamespaceBuilder::makeStringList(AST::StringList list,
		                                 const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::StringList(std::move(list)));
		}
		
		AST::Node<Version>
		NamespaceBuilder::makeVersion(const Version version, const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new Version(version));
		}
		
	}
	
}
