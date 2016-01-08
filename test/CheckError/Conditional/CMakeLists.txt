# Conditional tests.

test_check_error(DeadCodeAfterIf USE_NEW_PARSER)
test_check_error(EarlyThenReturn)
test_check_error(IntCondition)
test_check_error(MismatchedTernaryValues)
test_check_error(MultipleElse)
test_check_error(NonBoolCondition)

