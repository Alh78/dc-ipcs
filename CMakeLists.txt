cmake_minimum_required(VERSION 3.16)
project(dc_ipcs C)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")

set(ENABLE_CJSON_TEST OFF CACHE INTERNAL "")
add_subdirectory(external/cJSON)

target_include_directories(cjson PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/cJSON>)

set(BUILD_CURL_EXE OFF CACHE INTERNAL "")
set(BUILD_STATIC_LIBS ON CACHE INTERNAL "")
set(CURL_STATICLIB ON CACHE INTERNAL "")

set(HTTP_ONLY ON CACHE INTERNAL "")
set(CURL_DISABLE_DOCS ON CACHE INTERNAL "")
set(CURL_DISABLE_VERBOSE_STRINGS ON CACHE INTERNAL "")

set(CURL_ZLIB OFF CACHE INTERNAL "")
set(CURL_BROTLI OFF CACHE INTERNAL "")
set(CURL_ZSTD OFF CACHE INTERNAL "")
set(CURL_USE_LIBPSL OFF CACHE INTERNAL "")
set(CURL_USE_LIBSSH2 OFF CACHE INTERNAL "")
set(USE_NGHTTP2 OFF CACHE INTERNAL "")

if(WIN32)
    set(CURL_USE_SCHANNEL ON CACHE INTERNAL "")
else()
    set(CURL_USE_OPENSSL ON CACHE INTERNAL "")
endif()

if(ANDROID)
    set(HAVE_MEMSET_EXPLICIT OFF CACHE BOOL "Force disable memset_explicit" FORCE)
endif()

add_subdirectory(external/curl)

add_library(log_lib STATIC
    external/log.c/src/log.c
)
target_include_directories(log_lib PUBLIC
    external/log.c/src
)

add_executable(dc-ipcs
    main.c
    network.c
    storage.c
    suntime-calc.c
)

target_include_directories(dc-ipcs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(dc-ipcs PRIVATE
    cjson
    libcurl_static
    log_lib
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    file(GLOB TEST_ASSET_FILES "assets/*")
    file(COPY ${TEST_ASSET_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

