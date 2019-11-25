macro(set_cxx_standard version)
  if (CMAKE_VERSION VERSION_LESS "3.1")
	if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++${version}")
	endif ()
  else ()
	set (CMAKE_CXX_STANDARD ${version})
  endif ()
endmacro(set_cxx_standard)

function(llvm_config)
  set(oneValueArgs GET_INCDIR GET_LIBDIR)
  cmake_parse_arguments(LLVM "" "${oneValueArgs}" "" ${ARGV})
  if(NOT DEFINED LLVMConfig)
	set(LLVMConfig llvm-config)
  endif()

  if(DEFINED LLVM_GET_INCDIR)
	execute_process(COMMAND ${LLVMConfig} --includedir OUTPUT_VARIABLE ${LLVM_GET_INCDIR})
	string(STRIP "${${LLVM_GET_INCDIR}}" "${LLVM_GET_INCDIR}")
	set(${LLVM_GET_INCDIR} ${${LLVM_GET_INCDIR}} PARENT_SCOPE)
  endif()

  if(DEFINED LLVM_GET_LIBDIR)
	execute_process(COMMAND ${LLVMConfig} --libdir OUTPUT_VARIABLE ${LLVM_GET_LIBDIR})
	string(STRIP "${${LLVM_GET_LIBDIR}}" "${LLVM_GET_LIBDIR}")
	set(${LLVM_GET_LIBDIR} ${${LLVM_GET_LIBDIR}} PARENT_SCOPE)
  endif()
endfunction()

# name FILES pattern DEPENDS libraries
function(add_clang_library name)
  set(multiValueArgs FILES DEPENDS)
  cmake_parse_arguments(RLIB "" "" "${multiValueArgs}" ${ARGV})
  file(GLOB RLIB_SRCS ${RLIB_FILES})
  add_library(${name} STATIC ${RLIB_SRCS})
  if(DEFINED RLIB_DEPENDS)
	target_link_libraries(${name} ${RLIB_DEPENDS})
  endif()
  install(TARGETS ${name} ARCHIVE DESTINATION lib)
endfunction()

function(add_clang_target name)
  set(multiValueArgs FILES DEPENDS)
  cmake_parse_arguments(RBIN "" "" "${multiValueArgs}" ${ARGV})
  file(GLOB RBIN_SRCS ${RBIN_FILES})
  add_executable(${name} ${RBIN_SRCS})
  if(DEFINED RBIN_DEPENDS)
	target_link_libraries(${name} ${RBIN_DEPENDS})
  endif()
  install(TARGETS ${name} DESTINATION bin)
endfunction()

function(add_clang_format)
  set(multiValueArgs DIRECTORY)
  cmake_parse_arguments(CF "" "" "${multiValueArgs}" ${ARGV})

  file(GLOB_RECURSE ALL_SOURCE_FILES *.cpp *.h *.c *.cc *.hpp)
  # message("${ALL_SOURCE_FILES}")

  foreach (SOURCE_FILE ${ALL_SOURCE_FILES})
	set(DIR_NOTFOUND 1)
	foreach (DIR ${CF_DIRECTORY})
	  string(FIND ${SOURCE_FILE} ${PROJECT_SOURCE_DIR}/${DIR} PROJECT_TRDPARTY_DIR_FOUND)
	  if (NOT ${PROJECT_TRDPARTY_DIR_FOUND} EQUAL -1)
		unset(DIR_NOTFOUND)
	  endif ()
	endforeach()

	if(DEFINED DIR_NOTFOUND)
	  # message("remove ${SOURCE_FILE}")
	  list(REMOVE_ITEM ALL_SOURCE_FILES ${SOURCE_FILE})
	else()
	  # message("keep ${SOURCE_FILE}")
	endif()
  endforeach ()

  # message("${ALL_SOURCE_FILES}")

  add_custom_target(
	clang-format
	COMMAND clang-format
	-i ${ALL_SOURCE_FILES}
	)
endfunction()

# omit symlink
function(find_real_library toVar name)
  find_library(${ARGV})
  set (FOUND_LIB ${${toVar}})
  set (REAL_LIB ${FOUND_LIB})
  if (IS_SYMLINK ${FOUND_LIB})
	file(READ_SYMLINK ${FOUND_LIB} REAL_LIB)
	if (NOT IS_ABSOLUTE ${REAL_LIB})
	  get_filename_component(REAL_LIB_DIR ${FOUND_LIB} DIRECTORY)
	  set (REAL_LIB "${REAL_LIB_DIR}/${REAL_LIB}")
	endif ()
  endif ()
  set (${toVar} ${REAL_LIB} PARENT_SCOPE)
endfunction()

# static analyzer
add_custom_target(
  static-check
  COMMAND make clean
  COMMAND scan-build make
  # install scan-build by `sudo apt-get install clang'
  # or `sudo apt-get install clang-tools' on ubuntu-18.04
  )
