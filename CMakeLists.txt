cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR) #CPACK_DEBIAN_<COMPONENT>_PACKAGE_NAME

set(MUNGEFS_VERSION_MAJOR "1")
set(MUNGEFS_VERSION_MINOR "0")
set(MUNGEFS_VERSION_PATCH "6")
set(MUNGEFS_VERSION "${MUNGEFS_VERSION_MAJOR}.${MUNGEFS_VERSION_MINOR}.${MUNGEFS_VERSION_PATCH}")

set(MUNGEFS_CXX_STANDARD 14)

if (NOT IRODS_EXTERNALS_PACKAGE_ROOT)
    set(IRODS_EXTERNALS_PACKAGE_ROOT "/opt/irods-externals" CACHE STRING "Choose the location of iRODS external packages." FORCE)
    message(STATUS "Setting unspecified IRODS_EXTERNALS_PACKAGE_ROOT to '${IRODS_EXTERNALS_PACKAGE_ROOT}'. This is the correct setting for normal builds.")
endif()

macro(CHECK_DEPENDENCY_SET_FULLPATH DEPENDENCY_NAME DEPENDENCY_SUBDIRECTORY)
    if (IS_DIRECTORY ${IRODS_EXTERNALS_FULLPATH_${DEPENDENCY_NAME}})
        message(STATUS "Using user-specified value for IRODS_EXTERNALS_FULLPATH_${DEPENDENCY_NAME}: ${IRODS_EXTERNALS_FULLPATH_${DEPENDENCY_NAME}}")
    else()
        if (NOT IS_DIRECTORY ${IRODS_EXTERNALS_PACKAGE_ROOT}/${DEPENDENCY_SUBDIRECTORY})
            message(FATAL_ERROR "${DEPENDENCY_NAME} not found at ${IRODS_EXTERNALS_PACKAGE_ROOT}/${DEPENDENCY_SUBDIRECTORY}")
        endif()
        set(IRODS_EXTERNALS_FULLPATH_${DEPENDENCY_NAME} ${IRODS_EXTERNALS_PACKAGE_ROOT}/${DEPENDENCY_SUBDIRECTORY})
    endif()
endmacro()

CHECK_DEPENDENCY_SET_FULLPATH(CLANG clang13.0.0-0)
CHECK_DEPENDENCY_SET_FULLPATH(CPPZMQ cppzmq4.8.1-1)
CHECK_DEPENDENCY_SET_FULLPATH(ARCHIVE libarchive3.5.2-0)
CHECK_DEPENDENCY_SET_FULLPATH(AVRO avro1.11.0-2)
CHECK_DEPENDENCY_SET_FULLPATH(BOOST boost1.81.0-0)
CHECK_DEPENDENCY_SET_FULLPATH(CLANG_RUNTIME clang-runtime13.0.0-0)
CHECK_DEPENDENCY_SET_FULLPATH(ZMQ zeromq4-14.1.8-0)

project(mungefs
  VERSION "${MUNGEFS_VERSION}"
  LANGUAGES C CXX)

set(MUNGEFS_PLATFORM_STRING linux_platform)
set(MUNGEFS_COMPILE_DEFINITIONS ${MUNGEFS_PLATFORM_STRING} _LARGEFILE_SOURCE _FILE_OFFSET_BITS=64 _LARGE_FILES _LARGEFILE64_SOURCE BOOST_SYSTEM_NO_DEPRECATED)

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build {Debug, Release}." FORCE)
  message(STATUS "Setting unspecified CMAKE_BUILD_TYPE to '${CMAKE_BUILD_TYPE}'. This is the correct setting for normal builds.")
endif()

add_compile_options(-Wall -Werror)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(FUSE 2.6 REQUIRED)

add_executable(
  mungefs
  ${CMAKE_CURRENT_SOURCE_DIR}/mungefs_server.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/mungefs_operations.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  )

target_link_libraries(
  mungefs
  PRIVATE
  ${IRODS_EXTERNALS_FULLPATH_AVRO}/lib/libavrocpp.so
  ${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_filesystem.so
  ${IRODS_EXTERNALS_FULLPATH_BOOST}/lib/libboost_system.so
  ${IRODS_EXTERNALS_FULLPATH_ARCHIVE}/lib/libarchive.so
  ${IRODS_EXTERNALS_FULLPATH_ZMQ}/lib/libzmq.so
  ${FUSE_LIBRARIES}
  Threads::Threads
  )
target_include_directories(
  mungefs
  PRIVATE
  ${IRODS_EXTERNALS_FULLPATH_AVRO}/include
  ${IRODS_EXTERNALS_FULLPATH_BOOST}/include
  ${IRODS_EXTERNALS_FULLPATH_ZMQ}/include
  ${IRODS_EXTERNALS_FULLPATH_CPPZMQ}/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${FUSE_INCLUDE_DIRS}
  )

target_compile_definitions(mungefs PRIVATE ${MUNGEFS_COMPILE_DEFINITIONS})
target_compile_options(mungefs PRIVATE -Wno-write-strings)
set_property(TARGET mungefs PROPERTY CXX_STANDARD ${MUNGEFS_CXX_STANDARD})

add_executable(
  mungefsctl
  ${CMAKE_CURRENT_SOURCE_DIR}/mungefsctl.cpp
  )

target_link_libraries(
  mungefsctl
  PRIVATE
  ${AVRO_PATH}/lib/libavrocpp.so
  ${BOOST_PATH}/lib/libboost_filesystem.so
  ${BOOST_PATH}/lib/libboost_system.so
  ${BOOST_PATH}/lib/libboost_program_options.so
  ${ZMQ_PATH}/lib/libzmq.so
  ${FUSE_LIBRARIES}
  Threads::Threads
  )
target_include_directories(
  mungefsctl
  PRIVATE
  ${AVRO_PATH}/include
  ${BOOST_PATH}/include
  ${ZMQ_PATH}/include
  ${CPPZMQ_PATH}/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
  )

target_compile_definitions(mungefsctl PRIVATE ${MUNGEFS_COMPILE_DEFINITIONS})
target_compile_options(mungefsctl PRIVATE -Wno-write-strings)
set_property(TARGET mungefsctl PROPERTY CXX_STANDARD ${MUNGEFS_CXX_STANDARD})

set (
    AVRO_FILE
    mungefs_ctl
    )

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include)

add_custom_command(
   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/${AVRO_FILE}.hpp
   COMMAND LD_LIBRARY_PATH=${BOOST_PATH}/lib:${CLANG_LIBS} ${IRODS_EXTERNALS_FULLPATH_AVRO}/bin/avrogencpp -o ${CMAKE_CURRENT_BINARY_DIR}/include/${AVRO_FILE}.hpp -i ${CMAKE_CURRENT_SOURCE_DIR}/avro_schemas/${AVRO_FILE}.json
   MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/avro_schemas/${AVRO_FILE}.json
)

set_source_files_properties(
  ${CMAKE_CURRENT_SOURCE_DIR}/mungefsctl.cpp
  PROPERTIES
  OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/include/${AVRO_FILE}.hpp
)
set_source_files_properties(
  ${CMAKE_CURRENT_SOURCE_DIR}/mungefs_server.cpp
  PROPERTIES
  OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/include/${AVRO_FILE}.hpp
)

install(
  TARGETS
  mungefs
  mungefsctl
  RUNTIME
  DESTINATION usr/bin
  )
