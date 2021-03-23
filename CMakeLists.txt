CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(uvpp)

FILE(GLOB UVPP_HEAD_FILES "include/uvpp/*")

add_library(uvpp INTERFACE)
message(${UVPP_HEAD_FILES})
target_sources(uvpp INTERFACE ${UVPP_HEAD_FILES})
target_include_directories(uvpp INTERFACE "include")


# Test app
# ADD_SUBDIRECTORY(test)

INSTALL(
    DIRECTORY include/
    DESTINATION include
    COMPONENT dev
)

IF(UNIX OR CYGWIN)
    SET(SHARE_DIR share/${CMAKE_PROJECT_NAME})
ELSEIF (WIN32)
    SET(SHARE_DIR bin)
ENDIF()
