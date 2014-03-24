
macro(fix_msvc_flags)
	if(MSVC) 
		foreach(flag_var CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RELWITHDEBINFO)  
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}") 
			string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}") 
		endforeach(flag_var) 
		SET (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}" CACHE STRING "MSVC C Debug MT flags " FORCE)     
		SET (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "MSVC CXX Debug MT flags " FORCE) 
		SET (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}" CACHE STRING "MSVC C Release MT flags " FORCE) 
		SET (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "MSVC CXX Release MT flags " FORCE) 
		SET (CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL}" CACHE STRING "MSVC C Debug MT flags " FORCE)     
		SET (CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}" CACHE STRING "MSVC C Release MT flags " FORCE) 
		SET (CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO}" CACHE STRING "MSVC CXX Debug MT flags " FORCE)     
		SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" CACHE STRING "MSVC CXX Release MT flags " FORCE) 
	endif()

endmacro()

macro(fix_gnu_gcc_flags)
	if(CMAKE_COMPILER_IS_GNUCXX)
		#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x -fPIC")
		#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC" CACHE STRING "fPIC options for SWIG" FORCE)
		#fPIC option is for SWIG
	endif()
endmacro()

macro(fix_compiler_flags)
	fix_msvc_flags()
	fix_gnu_gcc_flags()
endmacro()

macro(add_python_module modname LIBS)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	set_source_files_properties(${modname}.i PROPERTIES CPLUSPLUS ON)
	set_source_files_properties(${modname}.i PROPERTIES SWIG_FLAGS "-includeall")
	set(CMAKE_SWIG_OUTDIR ${MODULE_OUTPUT_DIRECTORY})
	swig_add_module(${modname} python ${modname}.i ${modname}.cpp)
	swig_link_libraries(${modname} ${LIBS} ${PYTHON_LIBRARIES})
	set_target_properties(${SWIG_MODULE_${modname}_REAL_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MODULE_OUTPUT_DIRECTORY})
endmacro()


macro(swig_python_init) 
	find_package(SWIG REQUIRED)
	include(${SWIG_USE_FILE})
	find_package(PythonLibs)
	message(STATUS "Python include path: ${PYTHON_INCLUDE_PATH}")
	message(STATUS "Python libraries:    ${PYTHON_LIBRARIES}")
	include_directories(${PYTHON_INCLUDE_PATH})
	set(CMAKE_SWIG_FLAGS "")
endmacro()


macro(swig_java_init)
	find_package(SWIG)
	include(${SWIG_USE_FILE})
	find_package(Java)
	find_package(JNI)

	#cmake 2.6 patch 
	find_program(Java_JAVAC_EXECUTABLE NAMES javac HINTS ${_JAVA_HINTS} PATHS ${_JAVA_PATHS})
	find_program(Java_JAR_EXECUTABLE NAMES jar HINTS ${_JAVA_HINTS} PATHS ${_JAVA_PATHS})
	message(STATUS "Java include path: ${JAVA_INCLUDE_PATH}")
	if (Java_FOUND AND JNI_FOUND)
		include_directories(${JAVA_INCLUDE_PATH} ${JNI_INCLUDE_DIRS})
	endif()
endmacro()

macro(add_java_module MOD_NAME LIBS)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	set_source_files_properties(${MOD_NAME}.i PROPERTIES CPLUSPLUS ON)
	set_source_files_properties(${MOD_NAME}.i PROPERTIES SWIG_FLAGS "")
	if (NOT MODULE_OUTPUT_DIRECTORY)
		set(MODULE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
	endif()

	set(JAVA_SOURCE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/javasrc)
	set(JAVA_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/javabuild)
	file(MAKE_DIRECTORY ${JAVA_SOURCE_DIRECTORY})
	file(MAKE_DIRECTORY ${JAVA_BINARY_DIRECTORY})
	set(CMAKE_SWIG_OUTDIR ${JAVA_SOURCE_DIRECTORY})

	swig_add_module(${MOD_NAME} java ${MOD_NAME}.i ${MOD_NAME}.cpp)
	swig_link_libraries(${MOD_NAME} ${LIBS})
	set_target_properties(${SWIG_MODULE_${MOD_NAME}_REAL_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MODULE_OUTPUT_DIRECTORY})
	set_target_properties(${SWIG_MODULE_${MOD_NAME}_REAL_NAME} PROPERTIES PREFIX lib)

	set(JAVA_SOURCE_CODE ${JAVA_SOURCE_DIRECTORY}/*.java)

	set(JAR_OUTPUT_FILE ${MODULE_OUTPUT_DIRECTORY}/${MOD_NAME}.jar)
	add_custom_target(${MOD_NAME}_jar ALL DEPENDS ${JAR_OUTPUT_FILE})

	add_custom_command(
		OUTPUT ${JAR_OUTPUT_FILE}
		COMMENT "Creating jar file..."
		COMMAND ${Java_JAVAC_EXECUTABLE} -d ${JAVA_BINARY_DIRECTORY} ${JAVA_SOURCE_CODE}
		COMMAND ${Java_JAR_EXECUTABLE} cf ${JAR_OUTPUT_FILE} -C ${JAVA_BINARY_DIRECTORY} "."
		DEPENDS ${JAVA_SOURCE_CODE}
		)
endmacro()

function(add_sources target)
	# define the <target>_SRCS properties if necessary
	get_property(prop_defined GLOBAL PROPERTY ${target}_SRCS DEFINED)
	if(NOT prop_defined)
		define_property(GLOBAL PROPERTY ${target}_SRCS
			BRIEF_DOCS "Sources for the ${target} target"
			FULL_DOCS "List of source files for the ${target} target")
	endif()
	# create list of sources (absolute paths)
	set(SRCSX)
	foreach(src ${ARGN})
	#foreach(src IN LISTS ARGN)
		if(NOT IS_ABSOLUTE "${src}")
			get_filename_component(src "${src}" ABSOLUTE)
		endif()
		list(APPEND SRCSX "${src}")
	endforeach()
	# append to global property
	set_property(GLOBAL APPEND PROPERTY "${target}_SRCS" "${SRCSX}")
endfunction()


macro (add_test_files)
    file (RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    #foreach (_src ${ARGN})
    #    if (_relPath)
    #        list (APPEND TEST_FILES "${_relPath}/${_src}")
    #    else()
    #        list (APPEND TEST_FILES "${_src}")
    #    endif()
    #endforeach()
	foreach(src ${ARGN})
		if(NOT IS_ABSOLUTE "${src}")
			get_filename_component(src "${src}" ABSOLUTE)
		endif()
		list(APPEND TEST_FILES "${src}")
	endforeach()
	
    if (_relPath)
        # propagate TEST_FILES to parent directory
        set (TEST_FILES ${TEST_FILES} PARENT_SCOPE)
    endif()
endmacro()

macro (add_benchmark_files)
    file (RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    #foreach (_src ${ARGN})
    #    if (_relPath)
    #        list (APPEND BENCHMARK_FILES "${_relPath}/${_src}")
    #    else()
    #        list (APPEND BENCHMARK_FILES "${_src}")
    #    endif()
    #endforeach()
	foreach(src ${ARGN})
		if(NOT IS_ABSOLUTE "${src}")
			get_filename_component(src "${src}" ABSOLUTE)
		endif()
		list(APPEND BENCHMARK_FILES "${src}")
	endforeach()
	
    if (_relPath)
        # propagate BENCHMARK_FILES to parent directory
        set (BENCHMARK_FILES ${BENCHMARK_FILES} PARENT_SCOPE)
    endif()
endmacro()

macro (strip_target target_name)
    if (UNIX)
    	ADD_CUSTOM_COMMAND(TARGET ${target_name} POST_BUILD
    		COMMAND ${CMAKE_STRIP} $<TARGET_FILE:${target_name}>
    		COMMENT "Strip target: ${target_name}"
    	)
    endif()
endmacro()