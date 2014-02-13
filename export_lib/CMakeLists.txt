cmake_minimum_required(VERSION 2.6)

project(export_lib)


if (

if(BUILD_EXT_API)
	swig_python_init()
	swig_java_init()

	add_python_module(count2d_exp mscdsa)
	add_python_module(cwig_exp cwig mscdsa)
	if (Java_FOUND)
		add_java_module(cwig_exp cwig mscdsa)
	endif()
endif()
