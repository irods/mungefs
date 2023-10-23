#[=======================================================================[.rst:
FindAvroCpp
-----------

Finds Avro C++. Does not (currently) support version matching.

IMPORTED Targets
^^^^^^^^^^^^^^^^

The following :prop_tgt:`IMPORTED` targets may be defined:

``Avro::AvroCpp``
  AvroCpp library.
``Avro::avrogencpp``
  avrogencpp command-line executable.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``AvroCpp_FOUND``
  true if AvroCpp headers, library, and executable were found
``AVROCPP_INCLUDE_DIR``
  the directory containing AvroCpp headers
``AVROCPP_LIBRARY``
  AvroCpp library to be linked
``AVROCPP_AVROGEN_EXECUTABLE``
  path to avrogencpp tool

TODO
^^^^

* Components
  * library
  * executable

#]=======================================================================]

cmake_policy(PUSH)
cmake_policy(SET CMP0054 NEW) # Only interpret if() arguments as variables or keywords when unquoted

find_path(AVROCPP_INCLUDE_DIR NAMES avro/AvroParse.hh)
find_library(AVROCPP_LIBRARY NAMES avrocpp libavrocpp)
find_program(AVROCPP_AVROGEN_EXECUTABLE NAMES avrogencpp)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(AvroCpp
	REQUIRED_VARS AVROCPP_INCLUDE_DIR AVROCPP_LIBRARY AVROCPP_AVROGEN_EXECUTABLE)

if (AvroCpp_FOUND)
	if (NOT TARGET Avro::AvroCpp)
		add_library(Avro::AvroCpp UNKNOWN IMPORTED)
		set_target_properties(Avro::AvroCpp PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${AVROCPP_INCLUDE_DIR}")
		set_target_properties(Avro::AvroCpp PROPERTIES IMPORTED_LOCATION "${AVROCPP_LIBRARY}")
	endif()
	if (NOT TARGET Avro::avrogencpp)
		add_executable(Avro::avrogencpp IMPORTED)
		set_target_properties(Avro::avrogencpp PROPERTIES IMPORTED_LOCATION "${AVROCPP_AVROGEN_EXECUTABLE}")
	endif()
endif()

cmake_policy(POP)
