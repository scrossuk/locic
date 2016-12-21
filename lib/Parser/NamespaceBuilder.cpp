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
			return AST::makeNode(location, new AST::NamespaceDecl(name, std::move(data)));
		}
		
		AST::Node<AST::NamespaceData>
		NamespaceBuilder::makeNamespaceData(AST::NamespaceData data,
		                                    const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::NamespaceData(std::move(data)));
		}
		
		AST::Node<AST::Alias>
		NamespaceBuilder::makeAlias(const String name, AST::Node<AST::ValueDecl> value,
		                            const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::Alias(name, std::move(value),
			                                              location));
		}
		
		AST::Node<AST::StaticAssert>
		NamespaceBuilder::makeStaticAssert(AST::Node<AST::PredicateDecl> predicate,
		                                   const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::StaticAssert(std::move(predicate)));
		}
		
		AST::Node<AST::ModuleScopeDecl>
		NamespaceBuilder::makeUnnamedExport(AST::Node<AST::NamespaceData> data,
		                                    const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location,
			                     AST::ModuleScopeDecl::Export(std::move(data)));
		}
		
		AST::Node<AST::ModuleScopeDecl>
		NamespaceBuilder::makeUnnamedImport(AST::Node<AST::NamespaceData> data,
		                                    const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location,
			                     AST::ModuleScopeDecl::Import(std::move(data)));
		}
		
		AST::Node<AST::ModuleScopeDecl>
		NamespaceBuilder::makeNamedExport(AST::Node<AST::StringList> name,
		                                  AST::Node<Version> version,
		                                  AST::Node<AST::NamespaceData> data,
		                                  const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location,
			                     AST::ModuleScopeDecl::NamedExport(std::move(name),
					                                       std::move(version),
					                                       std::move(data)));
		}
		
		AST::Node<AST::ModuleScopeDecl>
		NamespaceBuilder::makeNamedImport(AST::Node<AST::StringList> name,
		                                  AST::Node<Version> version,
		                                  AST::Node<AST::NamespaceData> data,
		                                  const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location,
			                     AST::ModuleScopeDecl::NamedImport(std::move(name),
					                                       std::move(version),
					                                       std::move(data)));
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
