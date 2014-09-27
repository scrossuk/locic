separate_arguments(INPUT_FILES)

set(ConcatString "")

foreach(file_source_name ${INPUT_FILES})
	file(READ "${file_source_name}" FileContents)
	set(ConcatString "${ConcatString}${FileContents}")
endforeach(file_source_name)

file(WRITE "${OUTPUT_FILE}" "${ConcatString}")
