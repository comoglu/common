cmake_minimum_required (VERSION 2.6...2.8.12)

project (libbson C)

set(CMAKE_INSTALL_LIBDIR "lib")
set(CMAKE_INSTALL_INCLUDEDIR "include")
set(BSON_EXTRA_ALIGN 0)

# In the future we may need to check whether static dependencies are
# available. For now, AUTO means ON.
if (ENABLE_TESTS AND NOT ENABLE_STATIC MATCHES "ON|AUTO")
   message (FATAL_ERROR "-DENABLE_STATIC=OFF also requires -DENABLE_TESTS=OFF")
endif ()

include (CheckFunctionExists)
include (CheckIncludeFile)
include (CheckStructHasMember)
include (CheckSymbolExists)
include (TestBigEndian)
include (InstallRequiredSystemLibraries)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/build/cmake)

# Set BSON_MAJOR_VERSION, BSON_MINOR_VERSION, etc.
include (LoadVersion)
LoadVersion (${PROJECT_SOURCE_DIR}/VERSION_CURRENT BSON)

#message ("libbson version (from VERSION_CURRENT file): ${BSON_VERSION}")

set (BSON_API_VERSION 1.0)

set (CPACK_PACKAGE_VERSION_MAJOR ${BSON_MAJOR_VERSION})
set (CPACK_PACKAGE_VERSION_MINOR ${BSON_MINOR_VERSION})

#include (CPack)
TEST_BIG_ENDIAN (BSON_BIG_ENDIAN)

#librt needed on linux for clock_gettime
find_library (RT_LIBRARY rt)
if (RT_LIBRARY)
   #set required libraries for CHECK_FUNCTION_EXISTS
   set (CMAKE_REQUIRED_LIBRARIES ${RT_LIBRARY})
   set (BSON_LIBRARIES ${BSON_LIBRARIES} ${RT_LIBRARY})
endif ()

# See https://public.kitware.com/Bug/view.php?id=15659
CHECK_SYMBOL_EXISTS (snprintf stdio.h BSON_HAVE_SNPRINTF)
if (NOT BSON_HAVE_SNPRINTF)
   set (BSON_HAVE_SNPRINTF 0)
else ()
   set (BSON_HAVE_SNPRINTF 1)
endif ()

CHECK_FUNCTION_EXISTS (reallocf BSON_HAVE_REALLOCF)
if (NOT BSON_HAVE_REALLOCF)
   set (BSON_HAVE_REALLOCF 0)
endif ()

CHECK_STRUCT_HAS_MEMBER ("struct timespec" tv_sec time.h BSON_HAVE_TIMESPEC)
if (NOT BSON_HAVE_TIMESPEC)
   message (STATUS "    no timespec struct")
   set (BSON_HAVE_TIMESPEC 0)
else ()
   message (STATUS "    struct timespec found")
   set (BSON_HAVE_TIMESPEC 1)
endif ()

CHECK_SYMBOL_EXISTS (gmtime_r time.h BSON_HAVE_GMTIME_R)
if (NOT BSON_HAVE_GMTIME_R)
   set (BSON_HAVE_GMTIME_R 0)
else ()
   set (BSON_HAVE_GMTIME_R 1)
endif ()

CHECK_FUNCTION_EXISTS (rand_r BSON_HAVE_RAND_R)
if (NOT BSON_HAVE_RAND_R)
   set (BSON_HAVE_RAND_R 0)
else ()
   set (BSON_HAVE_RAND_R 1)
endif ()

if (WIN32)
   set (BSON_OS 2)
else ()
   set (BSON_OS 1)
endif ()

include (CheckIncludeFiles)

CHECK_INCLUDE_FILE (strings.h BSON_HAVE_STRINGS_H)
if (NOT BSON_HAVE_STRINGS_H)
   set (BSON_HAVE_STRINGS_H 0)
endif ()

if (MSVC)
   set (BSON_HAVE_CLOCK_GETTIME 0)
   set (BSON_HAVE_STDBOOL_H 0)
   set (BSON_HAVE_STRNLEN 0)
   set (BSON_HAVE_SYSCALL_TID 0)
else ()
   check_symbol_exists (clock_gettime time.h BSON_HAVE_CLOCK_GETTIME)
   if (NOT BSON_HAVE_CLOCK_GETTIME)
      set (BSON_HAVE_CLOCK_GETTIME 0)
   endif ()
   check_symbol_exists (strnlen string.h BSON_HAVE_STRNLEN)
   if (NOT BSON_HAVE_STRNLEN)
      set (BSON_HAVE_STRNLEN 0)
   endif ()
   CHECK_INCLUDE_FILE (stdbool.h BSON_HAVE_STDBOOL_H)
   if (NOT BSON_HAVE_STDBOOL_H)
      set (BSON_HAVE_STDBOOL_H 0)
   endif ()
   CHECK_SYMBOL_EXISTS (SYS_gettid sys/syscall.h BSON_HAVE_SYSCALL_TID)
   check_symbol_exists (syscall unistd.h _TMP_HAVE_SYSCALL)
   if (NOT BSON_HAVE_SYSCALL_TID OR NOT _TMP_HAVE_SYSCALL OR APPLE OR ANDROID)
      set (BSON_HAVE_SYSCALL_TID 0)
   endif ()
endif ()

if (BSON_BIG_ENDIAN)
   set (BSON_BYTE_ORDER 4321)
else ()
   set (BSON_BYTE_ORDER 1234)
endif ()

include (CheckAtomics)

configure_file (
   "${PROJECT_SOURCE_DIR}/src/bson/bson-config.h.in"
   "${PROJECT_BINARY_DIR}/src/bson/bson-config.h"
)

configure_file (
   "${PROJECT_SOURCE_DIR}/src/bson/bson-version.h.in"
   "${PROJECT_BINARY_DIR}/src/bson/bson-version.h"
)

if (ENABLE_APPLE_FRAMEWORK)
   configure_file (
      "${PROJECT_SOURCE_DIR}/src/bson/modules/module.modulemap.in"
      "${PROJECT_BINARY_DIR}/src/bson/modules/module.modulemap"
   )
endif ()

include_directories ("${PROJECT_BINARY_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/src/common")

if (APPLE)
   cmake_policy (SET CMP0042 OLD)
endif ()

#include (MaintainerFlags)

if (ENABLE_COVERAGE)
   set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g --coverage")
endif ()

set (SOURCES
   ${PROJECT_SOURCE_DIR}/src/bson/bcon.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-atomic.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-clock.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-context.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-decimal128.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-error.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-iso8601.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-iter.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-json.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-keys.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-md5.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-memory.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-oid.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-reader.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-string.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-timegm.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-utf8.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-value.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-version-functions.c
   ${PROJECT_SOURCE_DIR}/src/bson/bson-writer.c
   ${PROJECT_SOURCE_DIR}/src/jsonsl/jsonsl.c
   ${PROJECT_SOURCE_DIR}/src/common/common-b64.c
   ${PROJECT_SOURCE_DIR}/src/common/common-md5.c
)

set (HEADERS
   ${PROJECT_BINARY_DIR}/src/bson/bson-config.h
   ${PROJECT_BINARY_DIR}/src/bson/bson-version.h
   ${PROJECT_SOURCE_DIR}/src/bson/bcon.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-atomic.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-clock.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-compat.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-context.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-decimal128.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-endian.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-error.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-iter.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-json.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-keys.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-macros.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-md5.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-memory.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-oid.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-prelude.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-reader.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-string.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-types.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-utf8.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-value.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-version-functions.h
   ${PROJECT_SOURCE_DIR}/src/bson/bson-writer.h
)

set (HEADERS_FORWARDING
   ${PROJECT_SOURCE_DIR}/src/bson/forwarding/bson.h
)

add_library (bson_shared SHARED ${SOURCES} ${HEADERS} ${HEADERS_FORWARDING})
set (CMAKE_CXX_VISIBILITY_PRESET hidden)
set_target_properties (bson_shared PROPERTIES COMPILE_DEFINITIONS "BSON_COMPILATION;JSONSL_PARSE_NAN")
set_target_properties (bson_shared PROPERTIES VERSION 0.0.0 SOVERSION 0)
set_target_properties (bson_shared PROPERTIES OUTPUT_NAME "bson-${BSON_API_VERSION}" PREFIX "lib")

if (ENABLE_APPLE_FRAMEWORK)
   set_target_properties(bson_shared PROPERTIES 
      FRAMEWORK TRUE 
      MACOSX_FRAMEWORK_BUNDLE_VERSION ${MONGOC_VERSION} 
      MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${MONGOC_VERSION}
      MACOSX_FRAMEWORK_IDENTIFIER org.mongodb.bson
      OUTPUT_NAME "bson"
      PUBLIC_HEADER "${HEADERS}" 
   )
endif ()

if (RT_LIBRARY)
   target_link_libraries (bson_shared ${RT_LIBRARY})
endif ()

find_library (M_LIBRARY m)
if (M_LIBRARY)
   target_link_libraries (bson_shared ${M_LIBRARY})
   set (BSON_LIBRARIES ${BSON_LIBRARIES} ${M_LIBRARY})
endif ()

set (THREADS_PREFER_PTHREAD_FLAG 1)
#find_package (Threads REQUIRED)
#target_link_libraries (bson_shared Threads::Threads)
if (CMAKE_USE_PTHREADS_INIT)
   set (BSON_LIBRARIES ${BSON_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif ()

if (WIN32)
   # gethostbyname
   target_link_libraries (bson_shared ws2_32)
   # Can't find_library () system dependencies
   # must be handled specially since we can't resolve them
   set (BSON_SYSTEM_LIBRARIES ${BSON_SYSTEM_LIBRARIES} ws2_32)
endif ()

if (ENABLE_STATIC MATCHES "ON|AUTO")
   add_library (bson_static STATIC ${SOURCES} ${HEADERS} ${HEADERS_FORWARDING})
   set_target_properties (bson_static PROPERTIES COMPILE_DEFINITIONS "BSON_COMPILATION;BSON_STATIC;JSONSL_PARSE_NAN")
   set_target_properties (bson_static PROPERTIES VERSION 0.0.0)
   set_target_properties (bson_static PROPERTIES OUTPUT_NAME "bson-static-${BSON_API_VERSION}")
   target_link_libraries (bson_static Threads::Threads)
   if (RT_LIBRARY)
      target_link_libraries (bson_static ${RT_LIBRARY})
   endif ()
   if (M_LIBRARY)
      target_link_libraries (bson_static ${M_LIBRARY})
   endif ()
   if (NOT UNIX)
      # gethostbyname
      target_link_libraries (bson_static ws2_32)
   endif ()
endif ()

function (add_example bin src)
   set (BSON_EXAMPLE_SOURCES ${PROJECT_SOURCE_DIR}/${src})
   add_executable (${bin} ${BSON_EXAMPLE_SOURCES})

   # Link against the shared lib like normal apps
   target_link_libraries (${bin} bson_shared)

   set (EXAMPLES ${EXAMPLES} ${bin})
endfunction ()

if (ENABLE_EXAMPLES)
   add_example (bcon-col-view examples/bcon-col-view.c)
   add_example (bcon-speed examples/bcon-speed.c)
   add_example (bson-metrics examples/bson-metrics.c)
   if (NOT WIN32)
      target_link_libraries (bson-metrics m)
      add_example (bson-streaming-reader examples/bson-streaming-reader.c)
   endif ()
   add_example (bson-to-json examples/bson-to-json.c)
   add_example (bson-validate examples/bson-validate.c)
   add_example (json-to-bson examples/json-to-bson.c)
   add_example (bson-check-depth examples/bson-check-depth.c)
endif () # ENABLE_EXAMPLES

set (BSON_HEADER_INSTALL_DIR
   "${CMAKE_INSTALL_INCLUDEDIR}/libbson-${BSON_API_VERSION}"
)

install (
   TARGETS bson_shared ${EXAMPLES}
   LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
   RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
   FRAMEWORK DESTINATION ${CMAKE_INSTALL_BINDIR} 
)
if (ENABLE_STATIC MATCHES "ON|AUTO")
   install (
      TARGETS bson_static ${EXAMPLES}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      FRAMEWORK DESTINATION ${CMAKE_INSTALL_BINDIR} 
   )
endif ()
install (
   FILES ${HEADERS}
   DESTINATION "${BSON_HEADER_INSTALL_DIR}/bson"
)
install (
   FILES ${HEADERS_FORWARDING}
   DESTINATION "${BSON_HEADER_INSTALL_DIR}"
)

if (ENABLE_APPLE_FRAMEWORK)
   install (
      FILES "${PROJECT_BINARY_DIR}/src/bson/modules/module.modulemap"
      DESTINATION "${CMAKE_INSTALL_BINDIR}/bson.framework/Modules/"
   )
endif ()

set (LIBBSON_LIBRARIES "")
foreach (_lib ${CMAKE_C_IMPLICIT_LINK_LIBRARIES} ${BSON_LIBRARIES})
   if (_lib MATCHES ".*/.*" OR _lib MATCHES "^-")
      set (LIBBSON_LIBRARIES "${LIBBSON_LIBRARIES} ${_lib}")
   else ()
      set (LIBBSON_LIBRARIES "${LIBBSON_LIBRARIES} -l${_lib}")
   endif ()
endforeach ()
# System dependencies don't match the above regexs, but also don't want the -l
foreach (_lib ${BSON_SYSTEM_LIBRARIES})
   set (LIBBSON_LIBRARIES "${LIBBSON_LIBRARIES} ${_lib}")
endforeach ()

set (VERSION "${BSON_VERSION}")
set (prefix "${CMAKE_INSTALL_PREFIX}")
set (libdir "\${prefix}/lib")
configure_file (
   ${CMAKE_CURRENT_SOURCE_DIR}/src/libbson-1.0.pc.in
   ${CMAKE_CURRENT_BINARY_DIR}/src/libbson-1.0.pc
   @ONLY)

install (
   FILES
      ${CMAKE_CURRENT_BINARY_DIR}/src/libbson-1.0.pc
   DESTINATION
      ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

if (ENABLE_STATIC MATCHES "ON|AUTO")
   configure_file (
      ${CMAKE_CURRENT_SOURCE_DIR}/src/libbson-static-1.0.pc.in
      ${CMAKE_CURRENT_BINARY_DIR}/src/libbson-static-1.0.pc
      @ONLY)

   install (
      FILES
         ${CMAKE_CURRENT_BINARY_DIR}/src/libbson-static-1.0.pc
      DESTINATION
         ${CMAKE_INSTALL_LIBDIR}/pkgconfig
   )
endif ()

#include (build/cmake/BSONPackage.cmake)

if (ENABLE_MAN_PAGES STREQUAL ON OR ENABLE_HTML_DOCS STREQUAL ON)
   find_package (Sphinx REQUIRED)
   add_subdirectory (doc)
   add_custom_target (bson-doc
      ALL
      DEPENDS
      $<$<STREQUAL:"${ENABLE_MAN_PAGES}","ON">:bson-man>
      $<$<STREQUAL:"${ENABLE_HTML_DOCS}","ON">:bson-html>
   )
endif ()

add_subdirectory (build)
# sub-directory 'doc' was already included above
#add_subdirectory (examples)
add_subdirectory (src)
#add_subdirectory (tests)

#set_local_dist (src_libbson_DIST_local
#   CMakeLists.txt
#   NEWS
#   THIRD_PARTY_NOTICES
#)

set (src_libbson_DIST
   ${src_libbson_DIST_local}
   ${src_libbson_build_DIST}
   ${src_libbson_doc_DIST}
   ${src_libbson_examples_DIST}
   ${src_libbson_src_DIST}
   ${src_libbson_tests_DIST}
   PARENT_SCOPE
)
