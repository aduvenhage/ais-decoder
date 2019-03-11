# setup default build type (has to be done before 'PROJECT(...)' command)
IF(DEFINED CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: Debug Release.")
ELSE()
    SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build, options are: Debug Release.")
ENDIF()

# start project
PROJECT(AisDecoder)

CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
SET(CMAKE_CONFIGURATION_TYPES Debug Release CACHE TYPE INTERNAL FORCE)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

IF(WIN32)
    # using custom package paths
    SET(WIN32_3RDPARTY_INCLUDE "C:/packages/include" CACHE PATH "")
    SET(WIN32_3RDPARTY_LIBS "C:/packages/lib" CACHE PATH "")
ENDIF(WIN32)

SET(AIS_LIB_INSTALL_PATH "lib" CACHE PATH "System path to install library to -- prefixed with CMAKE_INSTALL_PREFIX.")
SET(AIS_SRC_INSTALL_PATH "include" CACHE PATH "System path to install header files to -- prefixed with CMAKE_INSTALL_PREFIX.")

message("Build tool: " ${CMAKE_BUILD_TOOL})

# setup system/cmake config flags based on target platform
IF(WIN32)
    SET(TARGET_OS "Windows")
    SET(TARGET_OS_ID "")
    SET(TARGET_OS_CODENAME "")
ENDIF(WIN32)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    SET(TARGET_OS "Mac OS")
    SET(TARGET_OS_ID "Darwin")
    SET(TARGET_OS_CODENAME "")

    SET(MACX ON)
    set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym" CACHE STRING "")
    set(CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(TARGET_OS "Linux")

    # find Linux flavour (sets TARGET_OS_ID and TARGET_OS_CODENAME)
    find_program(LSB_RELEASE lsb_release)
    execute_process(COMMAND ${LSB_RELEASE} -is
        OUTPUT_VARIABLE TARGET_OS_ID
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(COMMAND ${LSB_RELEASE} -cs
        OUTPUT_VARIABLE TARGET_OS_CODENAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    SET(LINUX ON)

    # detect Raspbian
    IF(${TARGET_OS_ID} MATCHES "Raspbian")
        SET(RASPBIAN ON)
        message("Raspbian Detected")
    ENDIF(${TARGET_OS_ID} MATCHES "Raspbian")

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

message("Target OS: " ${TARGET_OS})
message("Target OS Flavour: " ${TARGET_OS_ID})
message("Target OS Codename: " ${TARGET_OS_CODENAME})


# compiler general include folders
SET(PATH_EXAMPLE_DATA "${CMAKE_CURRENT_LIST_DIR}/data" CACHE PATH "Data folder used by example applications.")

IF(WIN32)
    INCLUDE_DIRECTORIES(${WIN32_3RDPARTY_INCLUDE})
ELSE(WIN32)
    IF(MACX)
        INCLUDE_DIRECTORIES(/usr/local/include /opt/local/include)
    ELSE(MACX)
        INCLUDE_DIRECTORIES(/usr/include)
    ENDIF(MACX)
ENDIF(WIN32)


# setup some general compiler settings for target platform
IF(MSVC)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /D_UNICODE /DUNICODE /D _CRT_SECURE_NO_DEPRECATE /wd4996 /W4 /WX /DNDEBUG /permissive- /std:c++latest")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_UNICODE /DUNICODE /D _CRT_SECURE_NO_DEPRECATE /wd4996 /W4 /WX /DDEBUG /permissive- /std:c++latest")

ELSE(MSVC)
    IF(MACX)
        SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -Wall -std=c++17 -DNDEBUG ")
        SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -std=c++17 -DDEBUG")

    ELSE(MACX)
        SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++14 -Wall -Wno-unknown-pragmas -DNDEBUG")
        SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -std=c++14 -Wall -Wno-unknown-pragmas -DDEBUG -rdynamic")
    ENDIF(MACX)

ENDIF(MSVC)


# setup more compiler flags (NDEBUG, DEBUG)
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS EXAMPLE_DATA_PATH="${PATH_EXAMPLE_DATA}")
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_DEBUG DEBUG=1)
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_RELEASE NDEBUG=1)


# capture project/build/test files (not compiled, but visible in solutions)
set(PROJ_FILES
    ${CMAKE_SOURCE_DIR}/README.md
    ${CMAKE_SOURCE_DIR}/examples/example_utils.h
    ${CMAKE_SOURCE_DIR}/python/ais_decoder.i
    ${CMAKE_SOURCE_DIR}/python/setup.py
)

add_custom_target(proj_files SOURCES ${PROJ_FILES})
set_source_files_properties(${PROJ_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)


# include source
add_subdirectory("ais_decoder")
add_subdirectory("examples/file_reader")
add_subdirectory("examples/file_reader_mt")
add_subdirectory("examples/quick")
add_subdirectory("examples/type123_meta")
add_subdirectory("examples/nmea2nmea")








