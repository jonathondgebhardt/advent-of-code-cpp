cmake_minimum_required(VERSION 3.14)

macro(default name)
  if(NOT DEFINED "${name}")
    set("${name}" "${ARGN}")
  endif()
endmacro()

default(TIDY_COMMAND clang-tidy)
default(
    PATTERNS
    source/*.cpp source/*.hpp
    test/*.cpp test/*.hpp
)
default(FIX NO)

set(flag --header-filter=^{CMAKE_SOURCE_DIR})
set(args OUTPUT_VARIABLE output)
if(FIX)
  set(flag -i)
  set(args "")
endif()

file(GLOB_RECURSE files ${PATTERNS})
set(linty_files "")
set(output "")
set(fix_available FALSE)
string(LENGTH "${CMAKE_SOURCE_DIR}/" path_prefix_length)

foreach(file IN LISTS files)
  execute_process(
      COMMAND "${TIDY_COMMAND}" --use-color "${flag}" "${file}"
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      RESULT_VARIABLE result
      ${args}
  )
  if(NOT result EQUAL "0")
    message(FATAL_ERROR "'${file}': linter returned with ${result}")
  endif()
  if(NOT FIX)
    if(output MATCHES ": warning:")
      string(SUBSTRING "${file}" "${path_prefix_length}" -1 relative_file)
      list(APPEND linty_files "${relative_file}")
    endif()
    if(output MATCHES "fix available")
        set(fix_available TRUE)
    endif()
  endif()
  message(STATUS ${output})
  set(output "")
endforeach()

if(NOT linty_files STREQUAL "")
  list(JOIN linty_files "\n" bad_list)
  message("The following files have lint:\n\n${bad_list}\n")

  if(fix_available)
    message(FATAL_ERROR "Run again with FIX=YES to fix these files.")
  endif()
endif()
