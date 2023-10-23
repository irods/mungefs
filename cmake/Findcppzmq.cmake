#[=======================================================================[.rst:
Findcppzmq
-----------

Finds cppzmq. Does not (currently) support version matching.

IMPORTED Targets
^^^^^^^^^^^^^^^^

The following :prop_tgt:`IMPORTED` targets may be defined:

``cppzmq::cppzmq``
  cppzmq.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``cppzmq_FOUND``
  true if cppzmq headers were found
``cppzmq_INCLUDE_DIR``
  the directory containing cppzmq headers

TODO
^^^^

* Version matching

#]=======================================================================]

cmake_policy(PUSH)
cmake_policy(SET CMP0054 NEW) # Only interpret if() arguments as variables or keywords when unquoted

unset(_cppzmq_ZeroMQ_REQUIRED)
if (cppzmq_FIND_REQUIRED)
	set(_cppzmq_ZeroMQ_REQUIRED REQUIRED)
endif()
find_package(ZeroMQ ${_cppzmq_ZeroMQ_REQUIRED})
unset(_cppzmq_ZeroMQ_REQUIRED)

find_path(cppzmq_INCLUDE_DIR NAMES zmq.hpp)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(cppzmq
	REQUIRED_VARS cppzmq_INCLUDE_DIR ZeroMQ_FOUND)

if (cppzmq_FOUND)
	if (NOT TARGET cppzmq::cppzmq)
		add_library(cppzmq::cppzmq INTERFACE IMPORTED)
		set_target_properties(cppzmq::cppzmq PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${cppzmq_INCLUDE_DIR}")
		set_target_properties(cppzmq::cppzmq PROPERTIES INTERFACE_LINK_LIBRARIES ZeroMQ::libzmq)
	endif()
endif()

cmake_policy(POP)
