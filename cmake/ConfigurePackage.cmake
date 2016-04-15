# ConfigurePackage.cmake
#
# Configure package
#
MACRO(CONFIGURE_PACKAGE CONF_PROGNAME LIBRARY_NAME PACKAGE_NAME)
    MESSAGE(STATUS "")
    MESSAGE(STATUS "#### Configuring ${PACKAGE_NAME} package")

    FIND_PROGRAM(CONF_PROGRAM_${LIBRARY_NAME} ${CONF_PROGNAME})

    MESSAGE(STATUS "${PACKAGE_NAME} configurator program: ${CONF_PROGRAM_${LIBRARY_NAME}} ${CONF_PROGNAME}")

    EXECUTE_PROCESS(COMMAND ${CONF_PROGRAM_${LIBRARY_NAME}} "--includes" OUTPUT_VARIABLE CONF_INCLUDE_ARRAY)
    EXECUTE_PROCESS(COMMAND ${CONF_PROGRAM_${LIBRARY_NAME}} "--libs"     OUTPUT_VARIABLE CONF_LIBRARY_ARRAY)

    STRING(REGEX MATCHALL "([^ |\n]+)" CONF_INCLUDE_ARRAY ${CONF_INCLUDE_ARRAY})

    FOREACH(PATH_INCLUDE ${CONF_INCLUDE_ARRAY})
        MESSAGE(STATUS "${PACKAGE_NAME} include directories: ${PATH_INCLUDE}")
        INCLUDE_DIRECTORIES(${PATH_INCLUDE})
    ENDFOREACH(PATH_INCLUDE)

    STRING(REGEX MATCHALL "([^ |\n]+)" CONF_LIBRARY_ARRAY ${CONF_LIBRARY_ARRAY})
    FIND_LIBRARY(HAVE_${LIBRARY_NAME} NAMES ${LIBRARY_NAME} PATHS ${CONF_LIBRARY_ARRAY} DOC "Path where the ${PACKAGE_NAME} libraries can be found")

    MESSAGE(STATUS "${PACKAGE_NAME} library: ${HAVE_${LIBRARY_NAME}}")

ENDMACRO(CONFIGURE_PACKAGE)