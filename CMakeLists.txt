cmake_minimum_required(VERSION 2.8)

project(spotifyWebApi)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

find_package(libspotify REQUIRED)
find_package(Jansson REQUIRED)
find_package(MySqlClient)
find_package(Threads REQUIRED)


set(spotifyWebApi_SOURCES
    appkey.c
    browse.c
    cmd.c
    search.c
    spshell.c
    spshell_posix.c
    toplist.c
)

include_directories(${LIBSPOTIFY_INCLUDE_DIR} ${JANSSON_INCLUDE_DIR})

set(spotifyWebApi_LINK_LIBRARIES
    ${LIBSPOTIFY_LIBRARIES}
    ${JANSSON_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

if(LIBMYSQLCLIENT_FOUND)
    include_directories(${LIBMYSQLCLIENT_INCLUDE_DIR})
    list(APPEND spotifyWebApi_SOURCES mysql.c)
    list(APPEND spotifyWebApi_LINK_LIBRARIES ${LIBMYSQLCLIENT_LIBRARIES})
    add_definitions( -DUSE_MYSQL )
endif()

add_executable(spshell ${spotifyWebApi_SOURCES})

target_link_libraries(spshell
    ${spotifyWebApi_LINK_LIBRARIES}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(spshell
        rt
    )
endif()