add_custom_target(GenerateFlexLexer COMMAND flex ${CMAKE_CURRENT_SOURCE_DIR}/Lexer.l)
add_custom_target(GenerateLemonParser COMMAND lemon ${CMAKE_CURRENT_SOURCE_DIR}/Parser.y)

add_executable(locic main.c Parser.c lex.yy.c AST/ClassDecl.c AST/ClassDef.c AST/ClassMethodDecl.c AST/ClassMethodDef.c AST/File.c AST/List.c AST/Scope.c AST/Statement.c AST/Type.c AST/TypeVar.c AST/Value.c AST/Var.c)

add_dependencies(locic GenerateFlexLexer GenerateLemonParser)
