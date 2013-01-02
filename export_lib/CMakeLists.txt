cmake_minimum_required(VERSION 2.6)

project(export_lib)

swig_python_init()
swig_java_init()

add_python_module(count2d_exp mscdsa)
add_python_module(gntf_exp gntf mscdsa)

add_java_module(gntf_exp gntf mscdsa)