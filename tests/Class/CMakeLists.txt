
macro(class_test name)
	add_test(NAME Test${name} COMMAND loci-test ${CMAKE_CURRENT_SOURCE_DIR}/${name}.loci --expected-output=${CMAKE_CURRENT_SOURCE_DIR}/${name}.log)
endmacro(class_test)

class_test(BasicClass)
class_test(DefaultConstructor)
class_test(Destructor)
class_test(EmptyClass)
class_test(ImplicitCopy)
class_test(NullConstructor)

