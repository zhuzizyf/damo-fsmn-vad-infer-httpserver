
macro(copy_shared_ossl_lib path)
	if(NOT EXISTS "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
	  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
	endif()
	file(GLOB OSSL_LIB_LIST LIST_DIRECTORIES false "${path}/*")
	foreach(OSSL_LIB_FILE ${OSSL_LIB_LIST})
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${OSSL_LIB_FILE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
		if(POCO_VERBOSE_MESSAGES)
			message(STATUS "Copying embedded OpenSSL library from ${OSSL_LIB_FILE} to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
		endif()
	endforeach()
endmacro()

macro(copy_static_ossl_lib path)
	if(NOT EXISTS "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
	  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
	endif()
	file(GLOB OSSL_LIB_LIST LIST_DIRECTORIES false "${path}/*")
	foreach(OSSL_LIB_FILE ${OSSL_LIB_LIST})
		execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${OSSL_LIB_FILE} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
		if(POCO_VERBOSE_MESSAGES)
			message(STATUS "Copying embedded OpenSSL library from ${OSSL_LIB_FILE} to ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
		endif()
	endforeach()
endmacro()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/openssl/build")
  message(STATUS "Use internal OpenSSL binaries but ${PROJECT_SOURCE_DIR}/openssl/build doesn't exists. Try to checkout ...")
  find_package(Git REQUIRED)
  execute_process(
      COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive --remote openssl 
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} 
      RESULT_VARIABLE EXITCODE)
  if(NOT "${EXITCODE}" STREQUAL "0")
    message(FATAL_ERROR "Checkout ${PROJECT_SOURCE_DIR}/openssl FAILED!")
  endif()
endif()

if( "${CMAKE_SIZEOF_VOID_P}" STREQUAL "8" )
    set(_OPENSSL_MSVC_ARCH_SUFFIX "64")
else()
    set(_OPENSSL_MSVC_ARCH_SUFFIX "32")
endif()

if(BUILD_SHARED_LIBS)
	option(OPENSSL_MSVC_STATIC_RT "Set ON to choose the MT version of the openssl lib" OFF)
	option(OPENSSL_USE_STATIC_LIBS "Set ON to look for static openssl libraries" OFF)
else()
	option(OPENSSL_MSVC_STATIC_RT "Set ON to choose the MT version of the openssl lib" ON)
	option(OPENSSL_USE_STATIC_LIBS "Set ON to look for static openssl libraries" ON)
endif()

set(OPENSSL_ROOT_DIR "${PROJECT_SOURCE_DIR}/openssl/build")

if(OPENSSL_USE_STATIC_LIBS)
	if(POCO_VERBOSE_MESSAGES)
		message(STATUS "Use internal OpenSSL static binaries from ${OPENSSL_ROOT_DIR}/win${_OPENSSL_MSVC_ARCH_SUFFIX}/lib")
	endif()
	list(APPEND CMAKE_LIBRARY_PATH
          "${OPENSSL_ROOT_DIR}/win${_OPENSSL_MSVC_ARCH_SUFFIX}/lib/debug"
          "${OPENSSL_ROOT_DIR}/win${_OPENSSL_MSVC_ARCH_SUFFIX}/lib/release")
	copy_static_ossl_lib("${OPENSSL_ROOT_DIR}/win${_OPENSSL_MSVC_ARCH_SUFFIX}/lib/debug")
	copy_static_ossl_lib("${OPENSSL_ROOT_DIR}/win${_OPENSSL_MSVC_ARCH_SUFFIX}/lib/release")
else()
	if(POCO_VERBOSE_MESSAGES)
		message(STATUS "Use internal OpenSSL shared binaries from ${OPENSSL_ROOT_DIR}/win${_OPENSSL_MSVC_ARCH_SUFFIX}/bin")
	endif()
	list(APPEND CMAKE_LIBRARY_PATH
          "${OPENSSL_ROOT_DIR}/win${_OPENSSL_MSVC_ARCH_SUFFIX}/bin/debug"
          "${OPENSSL_ROOT_DIR}/win${_OPENSSL_MSVC_ARCH_SUFFIX}/bin/release")
	copy_shared_ossl_lib("${OPENSSL_ROOT_DIR}/win${_OPENSSL_MSVC_ARCH_SUFFIX}/bin/debug")
	copy_shared_ossl_lib("${OPENSSL_ROOT_DIR}/win${_OPENSSL_MSVC_ARCH_SUFFIX}/bin/release")
	foreach(_OSSL_PATH "${OPENSSL_ROOT_DIR}/win${_OPENSSL_MSVC_ARCH_SUFFIX}/bin/debug" "${OPENSSL_ROOT_DIR}/win${_OPENSSL_MSVC_ARCH_SUFFIX}/bin/release")
	       string(REPLACE "/" "\\" _OSSL_WIN_PATH ${_OSSL_PATH})
	       string(APPEND OPENSSL_PATH ${_OSSL_WIN_PATH} "\;")
	endforeach()
	set(INTERNAL_OPENSSL_PATH "PATH=%PATH%\;${OPENSSL_PATH}")
endif()
