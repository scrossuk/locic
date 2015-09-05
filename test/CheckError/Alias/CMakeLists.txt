# Alias tests.

test_check_error(AliasCircularDependency)
test_check_error(AliasUsePredicateAsType EXPECT_FAIL)
test_check_error(TypeBasicAliasSymbol)
test_check_error(TypeTemplatedAliasSymbol)

