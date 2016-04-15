#
# AS - C++ Application server
#
PROJECT(cas)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)

# ################################################################################
# Defaults
#
OPTION(DEBUG_MODE               "Enable debug output [default: ON]"              ON)
OPTION(EXTRA_DEBUG              "Enable more debug output [default: OFF]"        OFF)
OPTION(ENABLE_OPTIMIZATION      "Enable optimization [default: ON]"              OFF)

OPTION(BUILD_APACHE13_MODULE    "Build Apache 1.3 module [default: OFF]"         OFF)
OPTION(BUILD_APACHE2X_MODULE    "Build Apache 2.x module [default: ON]"          ON)
OPTION(BUILD_FCGI_MODULE        "Build FastCGI server [default: ON]"             ON)

OPTION(BUILD_CURL_XMLRPC_CLIENT "Build cURL-based XMLRPC client [default: ON]"   ON)

OPTION(INSTALL_EXAMPLE_MODULES  "Install example modules [default: ON]"          ON)

OPTION(INSTALL_CAS_LIBRARY      "Install CAS library (libcas) [default: ON]"     ON)
OPTION(INSTALL_CAS_MODULES      "Install CAS modules [default: ON]"              ON)
OPTION(INSTALL_CAS_HEADERS      "Install CAS headers [default: ON]"              ON)
OPTION(INSTALL_CAS_CONFIGS      "Install CAS configuration files [default: ON]"  ON)
OPTION(INSTALL_CAS_BINARIES     "Install CAS binaries [default: ON]"             ON)
OPTION(INSTALL_CAS_DATA         "Install CAS data files [default: ON]"           ON)

# Default configuration file
SET(CAS_GLOBAL_CONFIG_FILE "/usr/local/etc/cas/conf/global-config.xml")
SET(CAS_LIBEXEC_DIR        "lib/cas" CACHE STRING "Plugins install path")
SET(CAS_LIB_DIR            "lib"     CACHE STRING "Libraries install path")
#SET(APACHE_LIBEXEC_DIR   "libexec/apache")

# CAS share dir
SET(CAS_SHAREDIR "${CMAKE_INSTALL_PREFIX}/share/cas")

# Force linking with libstdc++, need for Solaris
SET(FORCE_LBSTDCPP_LINKING "OFF")

# ################################################################################
# Version
#
SET(AS_VERSION_MAJOR 3)
SET(AS_VERSION_MINOR 8)
SET(AS_VERSION_PATCH 0)

SET(AS_VERSION         "${AS_VERSION_MAJOR}.${AS_VERSION_MINOR}.${AS_VERSION_PATCH}")
SET(AS_IDENT           "Fitter-A")
SET(AS_MASTER_SITE_URL "http://cas.havoc.ru/")

# ################################################################################
# Macroses
#

# Standard CMake macroses
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckCXXSourceCompiles)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckLibraryExists)

# User-defined macroses
INCLUDE(cmake/RenameModule.cmake)
INCLUDE(cmake/ConfigurePackage.cmake)
INCLUDE(cmake/ConfigureLibrary.cmake)
INCLUDE(cmake/ConfigureMD5Support.cmake)

# ################################################################################
# Policies
#

# ################################################################################
# Check system header files
#

CHECK_INCLUDE_FILES(sys/types.h    HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES(sys/time.h     HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILES(sys/uio.h      HAVE_SYS_UIO_H)

CHECK_INCLUDE_FILES(dlfcn.h        HAVE_DLFCN_H)
CHECK_INCLUDE_FILES(fcntl.h        HAVE_FCNTL_H)
CHECK_INCLUDE_FILES(math.h         HAVE_MATH_H)
CHECK_INCLUDE_FILES(stdio.h        HAVE_STDIO_H)
CHECK_INCLUDE_FILES(stdlib.h       HAVE_STDLIB_H)
CHECK_INCLUDE_FILES(string.h       HAVE_STRING_H)
CHECK_INCLUDE_FILES(time.h         HAVE_TIME_H)
CHECK_INCLUDE_FILES(unistd.h       HAVE_UNISTD_H)
CHECK_INCLUDE_FILES(sysexits.h     HAVE_SYSEXITS_H)
CHECK_INCLUDE_FILES(sys/resource.h HAVE_SYS_RESOURCE_H)
# ################################################################################
# Check STL types
#
CHECK_CXX_SOURCE_COMPILES("#include <map>
                           int main(void)
                           {
                               std::map<int, int> m;
                               m.insert(std::pair<int, int>(1, 2));
                               m[2] = 3;
                               return 0;
                           }
                          " STL_MAP_CHECK)
IF(NOT STL_MAP_CHECK)
    MESSAGE(STATUS "!!!! STL std::map does not work!")
ENDIF(NOT STL_MAP_CHECK)

CHECK_CXX_SOURCE_COMPILES("#include <vector>
                           int main(void)
                           {
                               std::vector<int> v;
                               v.push_back(1);
                               return 0;
                           }
                          " STL_VECTOR_CHECK)
IF(NOT STL_VECTOR_CHECK)
    MESSAGE(STATUS "!!!! STL std::vector does not work!")
ENDIF(NOT STL_VECTOR_CHECK)

CHECK_CXX_SOURCE_COMPILES("#include <string>
                           int main(void)
                           {
                               std::string s;
                               s.assign(\"Hello, World!\");
                               return 0;
                           }
                          " STL_STRING_CHECK)

IF(NOT STL_STRING_CHECK)
    MESSAGE(STATUS "!!!! STL std::string does not work!")
ENDIF(NOT STL_STRING_CHECK)

CHECK_CXX_SOURCE_COMPILES("#include <stdexcept>
                           int main(void)
                           {
                               try
                               {
                                   throw std::logic_error(\"Ouch!\");
                               }
                               catch(std::exception &e)
                               {
                                   return 0;
                               }
                               return 0;
                           }
                          " STL_STDEXCEPT_CHECK)

IF(NOT STL_STDEXCEPT_CHECK)
    MESSAGE(STATUS "!!!! STL std::logic_error/std::exception does not work!")
ENDIF(NOT STL_STDEXCEPT_CHECK)

CHECK_CXX_SOURCE_COMPILES("#include <vector>
                           int main(void)
                           {
                               vector<int> v;
                               v.push_back(1);
                               return 0;
                           }
                          " STL_VECTOR_NO_STD_CHECK)

IF(NOT STL_VECTOR_CHECK)
    IF (STL_VECTOR_NO_STD_CHECK)
        MESSAGE(STATUS "Using STL classes without STD namespace")
        SET(NO_STL_STD_PREFIX "ON")
    ENDIF (STL_VECTOR_NO_STD_CHECK)
ENDIF(NOT STL_VECTOR_CHECK)

# ################################################################################
# Set system-specific data
#
MESSAGE(STATUS "#### System name is: ${CMAKE_SYSTEM_NAME}")

IF("${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD")
    MESSAGE(STATUS "#### Building for FreeBSD...")

    SET(CAS_GLOBAL_CONFIG_FILE "/usr/local/etc/cas/global-config.xml")

    SET(CAS_LIBEXEC_DIR        "libexec/cas")
ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES "FreeBSD")

SET (RT_LIBRARY)
IF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    MESSAGE(STATUS "#### Building for Linux...")

    SET(CAS_GLOBAL_CONFIG_FILE "/etc/cas/global-config.xml")
    SET(SKIP_ICONV_LIRARY "ON")

    # Skip trpath
    SET(CMAKE_SKIP_RPATH  TRUE)

    # Skip the full RPATH for the build tree
    SET(CMAKE_SKIP_BUILD_RPATH  TRUE)

    # when building, use the install RPATH already
    # (so it doesn't need to relink when installing)
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

    # the RPATH to be used when installing
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

    # add the automatically determined parts of the RPATH
    # which point to directories outside the build tree to the install RPATH
    SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

    SET(CMAKE_PLATFORM_BUILD_FLAGS "-D_LARGEFILE64_SOURCE")

    FIND_LIBRARY(RT_LIBRARY NAMES rt PATHS /lib
                                           /usr/lib
                                           /usr/local/lib
                 DOC "Path where the rt libraries can be found")

   SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -D_REENTRANT -D_THREAD_SAFE -D_FORTIFY_SOURCE=2")

   SET(CAS_LIBEXEC_DIR      "lib/cas")
ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")

IF("${CMAKE_SYSTEM_NAME}" MATCHES "SunOS")
    MESSAGE(STATUS "#### Building for SunOS...")
    SET(SKIP_ICONV_LIRARY      "ON")
    SET(CAS_GLOBAL_CONFIG_FILE "/opt/CASWserver/conf/global-config.xml")
    SET(FORCE_LBSTDCPP_LINKING "ON")
    SET(CAS_LIBEXEC_DIR        "libexec/cas")
ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES "SunOS")

IF("${CMAKE_SYSTEM_NAME}" MATCHES "HP-UX")
    MESSAGE(STATUS "#### Building for HP-UX...")

ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES "HP-UX")

INCLUDE_DIRECTORIES(include)

# ################################################################################
# Compiler-specific
#
IF("${CMAKE_COMPILER_IS_GNUCXX}" MATCHES "1")

    SET(CMAKE_CXX_WARN_FLAGS   "-Werror -Wall -pedantic -Wno-long-long -Wno-variadic-macros")
    IF(DEBUG_MODE MATCHES "ON")
        MESSAGE(STATUS "#### Debug mode ON")
        SET(CMAKE_CXX_DEFINES      "-g -D_DEBUG")
        SET(CMAKE_CXX_OPTIMIZATION "-O0")
    ELSE (DEBUG_MODE MATCHES "OFF")
        MESSAGE(STATUS "#### Debug mode OFF")
        SET(CMAKE_CXX_DEFINES      "")
        SET(CMAKE_CXX_ARCH_FLAGS   "")
    ENDIF(DEBUG_MODE MATCHES "ON")

    IF(ENABLE_OPTIMIZATION MATCHES "ON")
        MESSAGE(STATUS "#### Optimization ON")
        SET(CMAKE_CXX_OPTIMIZATION "-O3")
    ENDIF(ENABLE_OPTIMIZATION MATCHES "ON")

    SET(CMAKE_CXX_FLAGS "${CXX_FLAGS} ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_GCOW} ${CMAKE_CXX_WARN_FLAGS} ${CMAKE_CXX_DEFINES} ${CMAKE_CXX_ARCH_FLAGS} ${CMAKE_CXX_OPTIMIZATION}")

    IF("${SKIP_RELINK_RPATH}" MATCHES "ON")
        SET(CMAKE_SKIP_RPATH  TRUE)
        MESSAGE(STATUS "#### Building for Linux...")
        # skip the full RPATH for the build tree
        SET(CMAKE_SKIP_BUILD_RPATH  TRUE)
        # when building, use the install RPATH already
        # (so it doesn't need to relink when installing)
        SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
        # the RPATH to be used when installing
        SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
        # add the automatically determined parts of the RPATH
        # which point to directories outside the build tree to the install RPATH
        SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    ENDIF("${SKIP_RELINK_RPATH}" MATCHES "ON")
ENDIF("${CMAKE_COMPILER_IS_GNUCXX}" MATCHES "1")

# ################################################################################
# Print global configuration
#
IF(ENABLE_OPTIMIZATION MATCHES "ON")
    MESSAGE(STATUS "#### INFO: Optimized build mode enabled")
ELSE(ENABLE_OPTIMIZATION MATCHES "OFF")
    MESSAGE(STATUS "#### INFO: Optimized build mode disabled")
ENDIF(ENABLE_OPTIMIZATION MATCHES "ON")

IF(DEBUG_MODE MATCHES "ON")
    MESSAGE(STATUS "#### WARNING: Debug mode enabled. Use this option only for testing!")
ENDIF(DEBUG_MODE MATCHES "ON")

# ################################################################################
# Setup libraries & headers
#

#
# Configure CTPP2 package
#
CONFIGURE_PACKAGE(ctpp2-config ctpp2 CTPP2)

#
# Expat XML Parser
#
CONFIGURE_LIBRARY(expat.h  "/usr/include /usr/local/include /opt/include /usr/sfw/include/"
                  expat    "/lib /usr/lib /usr/local/lib /opt/lib /usr/sfw/lib")

#
# PCRE library
#
CONFIGURE_LIBRARY(pcre.h  "/usr/include /usr/local/include /opt/include"
                  pcre    "/lib /usr/lib /usr/local/lib /opt/lib")

#
# Iconv charset converter, need optional library linking
#
FIND_PATH(ICONV_INCLUDE iconv.h PATHS /opt/include
                                      /usr/include
                                      /usr/local/include
          DOC "Path where the ICONV header files can be found")

IF (ICONV_INCLUDE)
    INCLUDE_DIRECTORIES(${ICONV_INCLUDE})

    IF (NOT "${SKIP_ICONV_LIRARY}" MATCHES "ON")
        FIND_LIBRARY(ICONV_LIBRARY NAMES iconv PATHS /lib
                                                     /opt/lib
                                                     /usr/lib
                                                     /usr/local/lib
                     DOC "Path where the iconv libraries can be found")
        IF (NOT ICONV_LIBRARY)
            MESSAGE(FATAL_ERROR "#### ERROR: Cannot find libiconv.so, aborting")
        ENDIF (NOT ICONV_LIBRARY)
    ENDIF (NOT "${SKIP_ICONV_LIRARY}" MATCHES "ON")

ELSE (NOT ICONV_INCLUDE)
    MESSAGE(FATAL_ERROR "#### Cannot find ICONV header files")
ENDIF (ICONV_INCLUDE)

IF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
#
# Dynamic loader, only for Linux
#
CONFIGURE_LIBRARY(dlfcn.h  "/usr/include /usr/local/include /opt/include"
                  dl       "/lib /usr/lib /usr/local/lib /opt/lib")

ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")

# ################################################################################
# Precompiled configuration
#
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/ASSysHeaders.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/include/ASSysHeaders.h)

# ################################################################################
# Main library sources
#
SET(LIBSRCS
            src/ASConfigErrorHandler.cpp
            src/ASCookie.cpp
            src/ASCTPPLogger.cpp
            src/ASDataFilter.cpp
            src/ASDebugHelper.cpp
            src/ASFilePool.cpp
            src/ASGenericConfigHandler.cpp
            src/ASGetOpt.cpp
            src/ASGlobalConfigHandler.cpp
            src/ASHandler.cpp
            src/ASHostConfigHandler.cpp
            src/ASLocation.cpp
            src/ASLocationURI.cpp
            src/ASLogger.cpp
            src/ASLoggerFile.cpp
            src/ASMD5.cpp
            src/ASModule.cpp
            src/ASObject.cpp
            src/ASPCRE.cpp
            src/ASPreRequestHandler.cpp
            src/ASRequest.cpp
            src/ASProfiler.cpp
            src/ASResourceStorage.cpp
            src/ASResponse.cpp
            src/ASResponseCodes.cpp
            src/ASServer.cpp
            src/ASServerManager.cpp
            src/ASTemplate.cpp
            src/ASUtil.cpp
            src/ASView.cpp
            src/ASXMLParser.cpp
            src/ASXMLRPCHandler.cpp
            )

# cURL library
IF(BUILD_CURL_XMLRPC_CLIENT MATCHES "ON")
    CONFIGURE_LIBRARY(curl/curl.h  "/usr/include /usr/local/include /opt/include"
                      curl         "/lib /usr/lib /usr/local/lib /opt/lib")

    SET(LIBSRCS ${LIBSRCS} src/ASXMLRPCClient.cpp)

ENDIF(BUILD_CURL_XMLRPC_CLIENT MATCHES "ON")


# ################################################################################
# Main library binary
#
ADD_LIBRARY(cas-st STATIC ${LIBSRCS})
SET_TARGET_PROPERTIES(cas-st PROPERTIES OUTPUT_NAME cas)
SET_TARGET_PROPERTIES(cas-st PROPERTIES LINKER_LANGUAGE CXX)

ADD_LIBRARY(cas SHARED ${LIBSRCS})
SET_TARGET_PROPERTIES(cas PROPERTIES OUTPUT_NAME cas)
SET_TARGET_PROPERTIES(cas PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(cas PROPERTIES VERSION ${AS_VERSION} SOVERSION ${AS_VERSION_MAJOR}.${AS_VERSION_MINOR})
TARGET_LINK_LIBRARIES(cas ${HAVE_ctpp2} ${HAVE_expat} ${ICONV_LIBRARY} ${HAVE_pcre} ${HAVE_curl} ${MD5_LIBRARY} ${HAVE_dl} ${RT_LIBRARY})
#SET_PROPERTY(TARGET cas PROPERTY LINK_INTERFACE_LIBRARIES "")

# ################################################################################
# Loadable modules
#
ADD_LIBRARY(mod_loader_test MODULE tests/ASObjectLoaderTest.cpp)
TARGET_LINK_LIBRARIES(mod_loader_test cas)
RENAME_MODULE(mod_loader_test mod_loader_test)

#
# Example handlers
#
ADD_LIBRARY(exampleclass SHARED examples/ASExampleClass.cpp examples/ASExampleObject.cpp)
SET_TARGET_PROPERTIES(exampleclass PROPERTIES OUTPUT_NAME exampleclass)
SET_TARGET_PROPERTIES(exampleclass PROPERTIES LINKER_LANGUAGE CXX)
SET_TARGET_PROPERTIES(exampleclass PROPERTIES VERSION ${AS_VERSION} SOVERSION ${AS_VERSION_MAJOR}.${AS_VERSION_MINOR})
TARGET_LINK_LIBRARIES(exampleclass cas)

ADD_LIBRARY(mod_example_handler MODULE examples/ASExampleHandler.cpp)
TARGET_LINK_LIBRARIES(mod_example_handler cas exampleclass ${HAVE_ctpp2})
RENAME_MODULE(mod_example_handler mod_example_handler)

ADD_LIBRARY(mod_example_module MODULE examples/ASExampleModule.cpp)
TARGET_LINK_LIBRARIES(mod_example_module cas exampleclass ${HAVE_ctpp2})
RENAME_MODULE(mod_example_module mod_example_module)

ADD_LIBRARY(mod_example_view MODULE examples/ASExampleView.cpp)
TARGET_LINK_LIBRARIES(mod_example_view cas ${HAVE_ctpp2})
RENAME_MODULE(mod_example_view mod_example_view)

#
# Example CTPP function
#
ADD_LIBRARY(mod_fn_example MODULE examples/ASFnExample.cpp)
TARGET_LINK_LIBRARIES(mod_fn_example cas ${HAVE_ctpp2})
RENAME_MODULE(mod_fn_example mod_fn_example)

#
# CTPP2 View class
#
ADD_LIBRARY(mod_ctpp_view MODULE modules/CTPPView.cpp)
TARGET_LINK_LIBRARIES(mod_ctpp_view cas ${HAVE_ctpp2} ${HAVE_dl})
RENAME_MODULE(mod_ctpp_view mod_ctpp_view)

#
# JSON View class
#
ADD_LIBRARY(mod_json_view MODULE modules/JSONView.cpp)
TARGET_LINK_LIBRARIES(mod_json_view cas ${HAVE_ctpp2})
RENAME_MODULE(mod_json_view mod_json_view)

#
# BSON View class
#
ADD_LIBRARY(mod_bson_view MODULE modules/BSONView.cpp)
TARGET_LINK_LIBRARIES(mod_bson_view cas ${HAVE_ctpp2})
RENAME_MODULE(mod_bson_view mod_bson_view)

#
# JSON RPC View
#
ADD_LIBRARY(mod_jsonrpc_view MODULE modules/JSONRPCView.cpp)
TARGET_LINK_LIBRARIES(mod_jsonrpc_view cas ${HAVE_ctpp2})
RENAME_MODULE(mod_jsonrpc_view mod_jsonrpc_view)

#
# BSON RPC View
#
ADD_LIBRARY(mod_bsonrpc_view MODULE modules/BSONRPCView.cpp)
TARGET_LINK_LIBRARIES(mod_bsonrpc_view cas ${HAVE_ctpp2})
RENAME_MODULE(mod_bsonrpc_view mod_bsonrpc_view)

#
# Plain data View class
#
ADD_LIBRARY(mod_plain_view MODULE modules/PlainView.cpp)
TARGET_LINK_LIBRARIES(mod_plain_view cas ${HAVE_ctpp2})
RENAME_MODULE(mod_plain_view mod_plain_view)

#
# XML RPC View class
#
ADD_LIBRARY(mod_xmlrpc_view MODULE modules/XMLRPCView.cpp)
TARGET_LINK_LIBRARIES(mod_xmlrpc_view cas ${HAVE_ctpp2})
RENAME_MODULE(mod_xmlrpc_view mod_xmlrpc_view)

#
# XML View class
#
ADD_LIBRARY(mod_xml_view MODULE modules/XMLView.cpp)
TARGET_LINK_LIBRARIES(mod_xml_view cas ${HAVE_ctpp2})
RENAME_MODULE(mod_xml_view mod_xml_view)

#
# Tab separated view
#
ADD_LIBRARY(mod_tabseparated_view MODULE modules/TabSeparatedView.cpp)
TARGET_LINK_LIBRARIES(mod_tabseparated_view cas ${HAVE_ctpp2})
RENAME_MODULE(mod_tabseparated_view mod_tabseparated_view)

# ################################################################################
# SAPI interfaces
#
IF (BUILD_APACHE13_MODULE MATCHES "ON")
    MESSAGE(STATUS "")
    MESSAGE(STATUS "#### INFO: Apache 1.3.X support enabled")

    FIND_PROGRAM(APACHE_APXS apxs)

    MESSAGE(STATUS "Apache apxs program: ${APACHE_APXS}")

    SET(APACHE_INCLUDEDIR "")
    SET(APACHE_CFLAGS "")

    EXECUTE_PROCESS(COMMAND ${APACHE_APXS} "-q" "INCLUDEDIR" OUTPUT_VARIABLE APACHE_INCLUDEDIR)
    EXECUTE_PROCESS(COMMAND ${APACHE_APXS} "-q" "CFLAGS" OUTPUT_VARIABLE APACHE_CFLAGS)

    IF ("${APACHE_LIBEXEC_DIR}" STREQUAL "")
        EXECUTE_PROCESS(COMMAND ${APACHE_APXS} "-q" "LIBEXECDIR" OUTPUT_VARIABLE APACHE_LIBEXEC_DIR)
    ENDIF ("${APACHE_LIBEXEC_DIR}" STREQUAL "")

    MESSAGE(STATUS "Apache include dir:  ${APACHE_INCLUDEDIR}")
    MESSAGE(STATUS "Apache libexec dir:  ${APACHE_LIBEXEC_DIR}")
    MESSAGE(STATUS "Apache CFLAGS:       ${APACHE_CFLAGS}")

    SET(APACHE_LIBMODSRCS  src/SAPI/Apache/ASApache13SAPI.cpp
                           src/SAPI/Apache/ASApache13Worker.cpp
                           src/SAPI/util/ASJSONRequestParser.cpp
                           src/SAPI/util/ASBSONRequestParser.cpp
                           src/SAPI/util/ASMultipartParser.cpp
                           src/SAPI/util/ASUrlencodedParser.cpp
                           src/SAPI/util/ASXMLRequestParser.cpp
                           src/SAPI/util/ASRequestParserFactory.cpp
                           src/SAPI/util/ASRawDataParser.cpp
                           src/SAPI/util/ASSAPIUtil.cpp
                           src/SAPI/Apache/mod_cas.c)

    INCLUDE_DIRECTORIES(src/SAPI/Apache src/SAPI/util)
    INCLUDE_DIRECTORIES(${APACHE_INCLUDEDIR})

    ADD_LIBRARY(mod_cas MODULE ${APACHE_LIBMODSRCS})

    SET_TARGET_PROPERTIES(mod_cas PROPERTIES CMAKE_CXX_FLAGS "${CXX_FLAGS} ${CMAKE_CXX_WARN_FLAGS} ${CMAKE_CXX_PARAMS} ${APACHE_CFLAGS} ")
    SET_TARGET_PROPERTIES(mod_cas PROPERTIES COMPILE_FLAGS " ${APACHE_CFLAGS} ")

    IF(FORCE_LBSTDCPP_LINKING MATCHES "ON")
        if(COMMAND cmake_policy)
            cmake_policy(SET CMP0003 NEW)
        endif(COMMAND cmake_policy)
        TARGET_LINK_LIBRARIES(mod_cas cas ${HAVE_ctpp2} stdc++)
    ELSE (FORCE_LBSTDCPP_LINKING MATCHES "OFF")
        TARGET_LINK_LIBRARIES(mod_cas cas ${HAVE_ctpp2})
    ENDIF (FORCE_LBSTDCPP_LINKING MATCHES "ON")

    RENAME_MODULE(mod_cas mod_cas)

ENDIF (BUILD_APACHE13_MODULE MATCHES "ON")

IF (BUILD_APACHE2X_MODULE MATCHES "ON")
    MESSAGE(STATUS "")
    MESSAGE(STATUS "#### Apache 2.X support enabled")

    FIND_PROGRAM(APACHE_APXS NAMES apxs apxs2)
    FIND_PROGRAM(APACHE_APU apu-1-config)

    MESSAGE(STATUS "Apache APU program: ${APACHE_APU}")

    SET(APR_INCLUDEDIRS "")
    SET(APR_LIBS "")
    SET(APR_LIBDIRS "")

    EXECUTE_PROCESS(COMMAND ${APACHE_APU} "--includes" OUTPUT_VARIABLE APR_INCLUDEDIRS)
    EXECUTE_PROCESS(COMMAND ${APACHE_APU} "--libs"     OUTPUT_VARIABLE APR_LIBS)
    EXECUTE_PROCESS(COMMAND ${APACHE_APU} "--ldflags"  OUTPUT_VARIABLE APR_LIBDIRS)

    STRING(REGEX MATCHALL "-I([^ |\n]+)" APR_INCLUDEDIRS ${APR_INCLUDEDIRS})
    FOREACH(PATH_INCLUDE ${APR_INCLUDEDIRS})
        STRING(REGEX REPLACE "^-I([^ |\n]+)" "\\1" PATH_INCLUDE ${PATH_INCLUDE})
        MESSAGE(STATUS "APR include directories: ${PATH_INCLUDE}")
        INCLUDE_DIRECTORIES(${PATH_INCLUDE})
    ENDFOREACH(PATH_INCLUDE)

    MESSAGE(STATUS "Apache apxs program: ${APACHE_APXS}")

    SET(APACHE_INCLUDEDIR "")
    SET(APACHE_CFLAGS "")

    EXECUTE_PROCESS(COMMAND ${APACHE_APXS} "-q" "INCLUDEDIR" OUTPUT_VARIABLE APACHE_INCLUDEDIR)
    STRING(REGEX REPLACE "\n" "" APACHE_INCLUDEDIR ${APACHE_INCLUDEDIR})

    EXECUTE_PROCESS(COMMAND ${APACHE_APXS} "-q" "CFLAGS" OUTPUT_VARIABLE APACHE_CFLAGS)
    STRING(REGEX REPLACE "\n" "" APACHE_CFLAGS ${APACHE_CFLAGS})

    IF ("${APACHE_LIBEXEC_DIR}" STREQUAL "")
        EXECUTE_PROCESS(COMMAND ${APACHE_APXS} "-q" "LIBEXECDIR" OUTPUT_VARIABLE APACHE_LIBEXEC_DIR)
        STRING(REGEX REPLACE "\n" "" APACHE_LIBEXEC_DIR ${APACHE_LIBEXEC_DIR})
    ENDIF ("${APACHE_LIBEXEC_DIR}" STREQUAL "")

    MESSAGE(STATUS "Apache include dir:  ${APACHE_INCLUDEDIR}")
    MESSAGE(STATUS "Apache libexec dir:  ${APACHE_LIBEXEC_DIR}")
    MESSAGE(STATUS "Apache CFLAGS:       ${APACHE_CFLAGS}")

    SET(APACHE_LIBMODSRCS  src/SAPI/Apache2/ASApache2XSAPI.cpp
                           src/SAPI/Apache2/ASApache2XWorker.cpp
                           src/SAPI/util/ASJSONRequestParser.cpp
                           src/SAPI/util/ASBSONRequestParser.cpp
                           src/SAPI/util/ASMultipartParser.cpp
                           src/SAPI/util/ASUrlencodedParser.cpp
                           src/SAPI/util/ASXMLRequestParser.cpp
                           src/SAPI/util/ASRequestParserFactory.cpp
                           src/SAPI/util/ASRawDataParser.cpp
                           src/SAPI/util/ASSAPIUtil.cpp
                           src/SAPI/Apache2/mod_cas2.c)

    INCLUDE_DIRECTORIES(src/SAPI/Apache2 src/SAPI/util)
    INCLUDE_DIRECTORIES(${APACHE_INCLUDEDIR})

    ADD_LIBRARY(mod_cas2 MODULE ${APACHE_LIBMODSRCS})

    SET_TARGET_PROPERTIES(mod_cas2 PROPERTIES CMAKE_CXX_FLAGS "${CXX_FLAGS} ${CMAKE_CXX_WARN_FLAGS} ${CMAKE_CXX_PARAMS} ${APACHE_CFLAGS} ")
    SET_TARGET_PROPERTIES(mod_cas2 PROPERTIES COMPILE_FLAGS " ${APACHE_CFLAGS} ")

    TARGET_LINK_LIBRARIES(mod_cas2 cas ${HAVE_ctpp2})
    RENAME_MODULE(mod_cas2 mod_cas2)

ENDIF (BUILD_APACHE2X_MODULE MATCHES "ON")

IF (BUILD_FCGI_MODULE MATCHES "ON")
    MESSAGE(STATUS "")
    MESSAGE(STATUS "#### FastCGI support enabled")

    SET(FCGI_SRCS src/SAPI/FastCGI/FastCGI.cpp
                  src/SAPI/FastCGI/FastCGIIO.cpp
                  src/SAPI/FastCGI/FastCGISAPI.cpp
                  src/SAPI/FastCGI/FastCGIConfigHandler.cpp
                  src/SAPI/FastCGI/FastCGILoggerSyslog.cpp
                  src/SAPI/FastCGI/FastCGIMain.cpp
                  src/SAPI/FastCGI/FastCGIMainSignalHandler.cpp
                  src/SAPI/FastCGI/FastCGIMainProcess.cpp
                  src/SAPI/FastCGI/FastCGIScoreboard.cpp
                  src/SAPI/FastCGI/FastCGIWorker.cpp
                  src/SAPI/FastCGI/FastCGISignalHandler.cpp
                  src/SAPI/FastCGI/FastCGIWorkerSignalHandler.cpp
                  src/SAPI/FastCGI/FastCGIWorkerSignalHandler.cpp
                  src/SAPI/FastCGI/FastCGISetProcTitle.c
                  src/SAPI/util/ASJSONRequestParser.cpp
                  src/SAPI/util/ASBSONRequestParser.cpp
                  src/SAPI/util/ASMultipartParser.cpp
                  src/SAPI/util/ASUrlencodedParser.cpp
                  src/SAPI/util/ASXMLRequestParser.cpp
                  src/SAPI/util/ASXMLRPCRequestParser.cpp
                  src/SAPI/util/ASRequestParserFactory.cpp
                  src/SAPI/util/ASRawDataParser.cpp
                  src/SAPI/util/ASSAPIUtil.cpp
       )

    INCLUDE_DIRECTORIES(src/SAPI/FastCGI src/SAPI/util)

    ADD_EXECUTABLE(cas-fcgid ${FCGI_SRCS})
    TARGET_LINK_LIBRARIES(cas-fcgid cas ${HAVE_ctpp2})

ENDIF (BUILD_FCGI_MODULE MATCHES "ON")
# ################################################################################
# Exec binary
#
ADD_EXECUTABLE(CASGenericConfigTest                tests/ASGenericConfigParser.cpp)
TARGET_LINK_LIBRARIES(CASGenericConfigTest         cas ${HAVE_ctpp2})

ADD_EXECUTABLE(cas-globalconf                      tests/ASGlobalConfigParser.cpp)
TARGET_LINK_LIBRARIES(cas-globalconf               cas ${HAVE_ctpp2})

ADD_EXECUTABLE(cas-hostconf                        tests/ASHostConfigParser.cpp)
TARGET_LINK_LIBRARIES(cas-hostconf                 cas ${HAVE_ctpp2})

ADD_EXECUTABLE(cas-xt                              tests/CASXT.cpp)
TARGET_LINK_LIBRARIES(cas-xt                       cas ${HAVE_ctpp2})

ADD_EXECUTABLE(CASPoolTest                         tests/ASPoolTest.cpp)
TARGET_LINK_LIBRARIES(CASPoolTest                  cas ${HAVE_ctpp2})

ADD_EXECUTABLE(cas-regexp                          tests/ASPCRETest.cpp)
TARGET_LINK_LIBRARIES(cas-regexp                   cas ${HAVE_ctpp2})

ADD_EXECUTABLE(cas-server                          tests/ASServerWorker.cpp)
TARGET_LINK_LIBRARIES(cas-server                   cas ${HAVE_ctpp2})

ADD_EXECUTABLE(cas-xmlrpc-parser                   tests/ASXMLRPCParser.cpp)
TARGET_LINK_LIBRARIES(cas-xmlrpc-parser            cas ${HAVE_ctpp2})

ADD_EXECUTABLE(cas_object_test                     tests/ASLoaderTest.cpp)
TARGET_LINK_LIBRARIES(cas_object_test              cas ${HAVE_ctpp2} ${HAVE_dl})

ADD_EXECUTABLE(cas_network_ranges                  tests/ASNetworkRangeTest.cpp)
TARGET_LINK_LIBRARIES(cas_network_ranges           cas ${HAVE_ctpp2})

ADD_EXECUTABLE(cas_profiler                        tests/ASProfiler.cpp)
TARGET_LINK_LIBRARIES(cas_profiler                 cas ${HAVE_ctpp2})

IF(BUILD_CURL_XMLRPC_CLIENT MATCHES "ON")
    ADD_EXECUTABLE(cas-xmlrpc-client               tests/ASXMLRPCClient.cpp)
    TARGET_LINK_LIBRARIES(cas-xmlrpc-client        cas ${HAVE_ctpp2})
ENDIF(BUILD_CURL_XMLRPC_CLIENT MATCHES "ON")

# ################################################################################
# Test suite
#
ENABLE_TESTING()

ADD_TEST(RegexpTest               cas-regexp "-([0-9]{2})-([a-zA-Z]+)-([0-9]{4})-" "prematch-08-Oct-1977-postmatch")
ADD_TEST(ObjectPoolTest           CASPoolTest)
ADD_TEST(GenericConfigParser      CASGenericConfigTest conf/generic-config.xml)
ADD_TEST(GlobalConfigParser       cas-globalconf       conf/global-config.xml.sample)
ADD_TEST(HostConfigParser         cas-hostconf         conf/vhost-config.xml.sample)
ADD_TEST(XMLRPCParser             cas-xmlrpc-parser    data/xmlrpc.xml)
ADD_TEST(LoadableModule           cas_object_test mod_loader_test.so ASObjectLoaderTest)
SET_PROPERTY(TEST LoadableModule PROPERTY ENVIRONMENT "LD_LIBRARY_PATH=.")
ADD_TEST(CAS_CTPPVIEW             cas-server conf/global-config.xml.tests www.example.com conf/vhost-config.xml.tests examples/request2.json)
ADD_TEST(NetworkRanges            cas_network_ranges)
ADD_TEST(Profiler                 cas_profiler)

SET(CAS_COMPILER_HELPER "#!/bin/sh
#
# CAS Configurator
#
PREFIX=${CMAKE_INSTALL_PREFIX}
CXXFLAGS=\"${CMAKE_CXX_FLAGS}\"
MAKE=${CMAKE_MAKE_PROGRAM}
CC=${CMAKE_C_COMPILER}
CXX=${CMAKE_CXX_COMPILER}
INCLUDE=\"${CMAKE_INSTALL_PREFIX}/include/cas\"
LIBS=\"${CMAKE_INSTALL_PREFIX}/lib\"
VERSION=\"${AS_VERSION}\"
SHAREDIR=\"${CAS_SHAREDIR}\"
LIBEXEC_DIR=\"${CAS_LIBEXEC_DIR}\"

if test \"x$1\" = \"x\"\; then
  echo \"Usage:\"
  echo \"  cas-config [--flags] [--cc] [--cxx] [--make] [--libs] [--includes] [--sharedir] [--libexecdir] [--version]\"
  echo \"                  ... [see below for complete flag list]\"
  echo \"\"
  echo \"    --version         displays the CAS version number\"
  echo \"    --flags           displays C++ compiler flags\"
  echo \"    --cc              displays executable name of C compiler\"
  echo \"    --cxx             displays executable name of C++ compiler\"
  echo \"    --make            displays executable name of make\"
  echo \"    --libs            displays list of libraries\"
  echo \"    --includes        displays list of include dirs\"
  echo \"    --sharedir        displays directory with shared files, examples, etc\"
  echo \"    --libexecdir      displays directory with loadable modules\"
  echo \"\"
else
   while test \"x$done\" = \"x\" -a \"x$1\" != \"x\"\; do
       case $1 in
           --version*)
           echo \${VERSION}
           \;\;

           --flags*)
           echo \${CXXFLAGS}
           \;\;

           --cc*)
           echo \${CC}
           \;\;

           --cxx*)
           echo \${CXX}
           \;\;

           --make*)
           echo \${MAKE}
           \;\;

           --libs*)
           echo \${LIBS}
           \;\;

           --includes*)
           echo \${INCLUDE}
           \;\;

           --sharedir*)
           echo \${SHAREDIR}
           \;\;

           --libexecdir*)
           echo \${LIBEXEC_DIR}
           \;\;
       esac
       shift
   done
fi
")

# Install hints for compiler
FILE(WRITE cas-config ${CAS_COMPILER_HELPER})

IF (INSTALL_CAS_HEADERS MATCHES "ON")
    # ################################################################################
    # Install Headers
    #
    INSTALL(FILES
                  include/ASConfigErrorHandler.hpp
                  include/ASCookie.hpp
                  include/ASDataFilter.hpp
                  include/ASDebugHelper.hpp
                  include/ASEventCodes.hpp
                  include/ASException.hpp
                  include/ASFile.hpp
                  include/ASFilePool.hpp
                  include/ASGenericConfigHandler.hpp
                  include/ASGetOpt.hpp
                  include/ASGlobalConfig.hpp
                  include/ASHandler.hpp
                  include/ASHandlerConfig.hpp
                  include/ASHostConfig.hpp
                  include/ASLoadableObject.hpp
                  include/ASLoader.hpp
                  include/ASLocation.hpp
                  include/ASLocationURI.hpp
                  include/ASLogCodes.hpp
                  include/ASLogger.hpp
                  include/ASMD5.hpp
                  include/ASModule.hpp
                  include/ASNetworkRange.hpp
                  include/ASObject.hpp
                  include/ASObjectLoader.hpp
                  include/ASPCRE.hpp
                  include/ASPreRequestHandler.hpp
                  include/ASPreRequestHandlerConfig.hpp
                  include/ASProfiler.hpp
                  include/ASPool.hpp
                  include/ASRequest.hpp
                  include/ASResourceStorage.hpp
                  include/ASResponse.hpp
                  include/ASResponseCodes.hpp
                  include/ASResponseWriter.hpp
                  include/ASServer.hpp
                  include/ASServerContext.hpp
                  include/ASServerManager.hpp
                  include/ASSysHeaders.h
                  include/ASTemplate.hpp
                  include/ASTypes.hpp
                  include/ASUtil.hpp
                  include/ASView.hpp
                  include/ASViewConfig.hpp
                  include/ASXMLHandler.hpp
                  include/ASXMLParser.hpp
                  include/ASXMLRPCHandler.hpp
                  include/STLException.hpp
                  include/STLFunctional.hpp
                  include/STLMap.hpp
                  include/STLPair.hpp
                  include/STLString.hpp
                  include/STLVector.hpp
            DESTINATION include/cas
            PERMISSIONS OWNER_READ OWNER_WRITE
                        GROUP_READ
                        WORLD_READ)

    IF(BUILD_CURL_XMLRPC_CLIENT MATCHES "ON")
        INSTALL(FILES
                      include/ASXMLRPCClient.hpp
                DESTINATION include/cas
                PERMISSIONS OWNER_READ OWNER_WRITE
                            GROUP_READ
                            WORLD_READ)
    ENDIF(BUILD_CURL_XMLRPC_CLIENT MATCHES "ON")

ENDIF (INSTALL_CAS_HEADERS MATCHES "ON")

# ################################################################################
# Install library
#
IF (INSTALL_CAS_LIBRARY MATCHES "ON")
    INSTALL(TARGETS cas
                    cas-st
            DESTINATION ${CAS_LIB_DIR}
            PERMISSIONS OWNER_READ OWNER_WRITE
                        GROUP_READ
                        WORLD_READ)

ENDIF (INSTALL_CAS_LIBRARY MATCHES "ON")

# ################################################################################
# Install binaries
#
IF (INSTALL_CAS_BINARIES MATCHES "ON")
    INSTALL(FILES cas-globalconf
                  cas-hostconf
                  cas-server
                  cas-config
                  cas-xt
                  cas-regexp
                  cas-xmlrpc-parser
            DESTINATION bin
            PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE
                        GROUP_EXECUTE GROUP_READ
                        WORLD_EXECUTE WORLD_READ)

# ################################################################################
# Install man pages
#
#INSTALL(FILES man/cas-globalconf.1
#              man/cas-hostconf.1
#              man/cas-server.1
#              man/cas-config.1
#              man/cas-xt.1
#              man/cas-regexp.1
#              man/cas-xmlrpc-parser.1
#              man/cas-config.1
#        DESTINATION share/man/man1
#        PERMISSIONS OWNER_READ OWNER_WRITE
#                    GROUP_READ
#                    WORLD_READ)

ENDIF (INSTALL_CAS_BINARIES MATCHES "ON")

# ################################################################################
# Install modules
#
IF (INSTALL_CAS_MODULES MATCHES "ON")
    INSTALL(FILES mod_ctpp_view.so
                  mod_bson_view.so
                  mod_json_view.so
                  mod_bsonrpc_view.so
                  mod_jsonrpc_view.so
                  mod_plain_view.so
                  mod_xmlrpc_view.so
                  mod_xml_view.so
                  mod_tabseparated_view.so
            DESTINATION ${CAS_LIBEXEC_DIR}
            PERMISSIONS OWNER_READ OWNER_WRITE
                        GROUP_READ
                        WORLD_READ)
ENDIF (INSTALL_CAS_MODULES MATCHES "ON")

# ################################################################################
# Install default template files
#

# ################################################################################
# Install example configuration
#
IF (INSTALL_CAS_CONFIGS MATCHES "ON")
    INSTALL(FILES conf/global-config.xml.sample
                  conf/vhost-config.xml.sample
                  conf/module-config.xml.sample
            DESTINATION ${CAS_SHAREDIR}/conf
            PERMISSIONS OWNER_READ OWNER_WRITE
                        GROUP_READ
                        WORLD_READ)
ENDIF (INSTALL_CAS_CONFIGS MATCHES "ON")

# ################################################################################
# Install data
#
IF (INSTALL_CAS_DATA MATCHES "ON")
    INSTALL(FILES examples/request.json
            DESTINATION ${CAS_SHAREDIR}/examples
            PERMISSIONS OWNER_READ OWNER_WRITE
                        GROUP_READ
                        WORLD_READ)

    INSTALL(FILES xt/HandlerTemplate.tmpl
                  xt/PreRequestHandlerTemplate.tmpl
                  xt/ViewTemplate.tmpl
                  xt/CMakeLists.tmpl
                  cmake/ConfigureLibrary.cmake
                  cmake/ConfigurePackage.cmake
                  cmake/RenameModule.cmake
            DESTINATION ${CAS_SHAREDIR}/xt
            PERMISSIONS OWNER_READ OWNER_WRITE
                        GROUP_READ
                        WORLD_READ)
ENDIF (INSTALL_CAS_DATA MATCHES "ON")

#
# Install Apache 1.3 module
#
IF (BUILD_APACHE13_MODULE MATCHES "ON")
    IF ("${APACHE_LIBEXEC_DIR}" STREQUAL "")
        MESSAGE(STATUS "Skip Apache 1.3 module installation")
    ELSE (NOT "${APACHE_LIBEXEC_DIR}" STREQUAL "")
        MESSAGE(STATUS "Installing Apache 1.3 module")
        INSTALL(FILES mod_cas.so
                DESTINATION ${APACHE_LIBEXEC_DIR}
                PERMISSIONS OWNER_READ OWNER_WRITE
                            GROUP_READ
                            WORLD_READ)
    ENDIF ("${APACHE_LIBEXEC_DIR}" STREQUAL "")

ENDIF (BUILD_APACHE13_MODULE MATCHES "ON")

#
# Install Apache 2.X module
#
IF (BUILD_APACHE2X_MODULE MATCHES "ON")
    IF ("${APACHE_LIBEXEC_DIR}" STREQUAL "")
        MESSAGE(STATUS "Skip Apache 2.X module installation")
    ELSE (NOT "${APACHE_LIBEXEC_DIR}" STREQUAL "")
        MESSAGE(STATUS "Installing Apache 2.X module")
        INSTALL(FILES mod_cas2.so
                DESTINATION ${APACHE_LIBEXEC_DIR}
                PERMISSIONS OWNER_READ OWNER_WRITE
                            GROUP_READ
                            WORLD_READ)
    ENDIF ("${APACHE_LIBEXEC_DIR}" STREQUAL "")

ENDIF (BUILD_APACHE2X_MODULE MATCHES "ON")

#
# Install FastCGI module
#
IF (BUILD_FCGI_MODULE MATCHES "ON")
    MESSAGE(STATUS "Installing FastCGI module")
    INSTALL(TARGETS cas-fcgid
            DESTINATION sbin
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE)
ENDIF (BUILD_FCGI_MODULE MATCHES "ON")

#
# Install example modules
#
IF (INSTALL_EXAMPLE_MODULES MATCHES "ON")
    MESSAGE(STATUS "Installing example CAS modules")
    INSTALL(FILES mod_example_handler.so
                  mod_example_module.so
                  mod_example_view.so
            DESTINATION ${CAS_LIBEXEC_DIR}
            PERMISSIONS OWNER_READ OWNER_WRITE
                        GROUP_READ
                        WORLD_READ)

    MESSAGE(STATUS "Installing example libraries")
    INSTALL(TARGETS exampleclass
            DESTINATION ${CAS_LIB_DIR}
            PERMISSIONS OWNER_READ OWNER_WRITE
                        GROUP_READ
                        WORLD_READ)

ENDIF (INSTALL_EXAMPLE_MODULES MATCHES "ON")
# End.
