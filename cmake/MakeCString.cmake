execute_process(
	COMMAND xxd -i "${INPUT_FILE}" "${OUTPUT_FILE}"
	WORKING_DIRECTORY "${WORKING_DIR}"
	)
