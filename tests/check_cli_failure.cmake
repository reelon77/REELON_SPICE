if(NOT DEFINED TINYSPICE_EXE OR NOT DEFINED INPUT_PATH)
    message(FATAL_ERROR "TinySpice executable and input path are required")
endif()

execute_process(
    COMMAND "${TINYSPICE_EXE}" "${INPUT_PATH}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE standard_out
    ERROR_VARIABLE standard_error
)

if(result EQUAL 0)
    message(FATAL_ERROR "TinySpice unexpectedly succeeded")
endif()

if(NOT standard_out STREQUAL "")
    message(FATAL_ERROR "failure path unexpectedly wrote stdout: ${standard_out}")
endif()

string(FIND "${standard_error}" "cannot open input file" expected_message)
if(expected_message EQUAL -1)
    message(FATAL_ERROR "stderr did not explain the missing input: ${standard_error}")
endif()
