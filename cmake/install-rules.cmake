install(
    TARGETS advent-of-code-cpp_exe
    RUNTIME COMPONENT advent-of-code-cpp_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
