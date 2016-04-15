IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    MESSAGE(STATUS "#### INFO: MD5 (Win32 CryptoAPI) hash function support enabled")
ELSE (${CMAKE_SYSTEM_NAME} MATCHES "Windows")

    FIND_PATH(MD5_INCLUDE openssl/md5.h PATHS /opt/include
                                              /usr/include
                                              /usr/local/include
              DOC "Path where the openssl header files can be found")

    IF (MD5_INCLUDE)
        INCLUDE_DIRECTORIES(${MD5_INCLUDE})

        FIND_LIBRARY(MD5_LIBRARY NAMES crypto PATHS /lib
                                                    /opt/lib
                                                    /usr/lib
                                                    /usr/local/lib
                     DOC "Path where the openssl libraries can be found")
        IF (MD5_LIBRARY)
            SET(PROJECT_LIBS "${PROJECT_LIBS} crypto")
        ELSE (MD5_LIBRARY)
            MESSAGE(STATUS "#### ERROR: Cannot find libcrypto.so, aborting")
        ENDIF (MD5_LIBRARY)

        SET(AS_MD5_SUPPORT "ON")

        MESSAGE(STATUS "#### INFO: MD5 (openssl) hash function support enabled")

    ELSE (MD5_INCLUDE)
        # reset FIND_PATH
        SET(MD5_INCLUDE)
        FIND_PATH(MD5_INCLUDE md5.h PATHS /opt/include
                                          /usr/include
                                          /usr/local/include
              DOC "Path where the openssl header files can be found")
        IF (MD5_INCLUDE)
            INCLUDE_DIRECTORIES(${MD5_INCLUDE})

            FIND_LIBRARY(MD5_LIBRARY NAMES md md5 PATHS /lib
                                                        /opt/lib
                                                        /usr/lib
                                                        /usr/local/lib
                         DOC "Path where the md5 libraries can be found")
            IF (MD5_LIBRARY)
                SET(PROJECT_LIBS "${PROJECT_LIBS} ${MD5_LIBRARY}")
                SET(AS_MD5_SUPPORT         "ON")
                SET(AS_MD5_WITHOUT_OPENSSL "ON")
                MESSAGE(STATUS "#### INFO: MD5 (libmd5) hash function support enabled")
            ELSE (MD5_LIBRARY)
                MESSAGE(STATUS "#### ERROR: Cannot find md5.so, aborting")
            ENDIF (MD5_LIBRARY)
        ELSE (MD5_INCLUDE)
            SET(MD5_SUPPORT "")
            MESSAGE(STATUS "#### WARNING: MD5 hash function support disabled: md5.h not found")
        ENDIF (MD5_INCLUDE)

    ENDIF (MD5_INCLUDE)
ENDIF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
