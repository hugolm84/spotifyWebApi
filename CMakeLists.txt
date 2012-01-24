cmake_minimum_required(VERSION 2.8)

project(spotifyWebApi)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

find_package(libspotify REQUIRED)
find_package(Jansson REQUIRED)
find_package(MySqlClient REQUIRED)
find_package(Threads REQUIRED)

include_directories(${LIBSPOTIFY_INCLUDE_DIR})

add_executable(spshell
    appkey.c
    browse.c
    cmd.c
    mysql.c
    search.c
    spshell.c
    spshell_posix.c
    toplist.c
)

target_link_libraries(spshell
    ${LIBSPOTIFY_LIBRARIES}
    ${JANSSON_LIBRARIES}
    ${LIBMYSQLCLIENT_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

if(UNIX AND NOT APPLE)
    target_link_libraries(spshell
        rt
    )
endif()