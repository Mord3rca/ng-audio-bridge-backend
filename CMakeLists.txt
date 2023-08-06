cmake_minimum_required(VERSION 3.6)
project("newgrounds-audio")

set(NG_AUDIO_COMPATIBILITY_ENDPOINTS OFF CACHE BOOL "Compile compatibility endpoints for old flash appliaction")

find_package(PkgConfig)
pkg_check_modules(Pistache REQUIRED IMPORTED_TARGET libpistache)
pkg_check_modules(Jsoncpp REQUIRED IMPORTED_TARGET jsoncpp)
pkg_check_modules(Sqlite REQUIRED IMPORTED_TARGET sqlite3)

FILE(GLOB BACKEND_SOURCES "src/*.cpp")

add_executable(ng-backend ${BACKEND_SOURCES})
target_compile_options (ng-backend PRIVATE -std=c++17)
target_link_libraries(ng-backend PkgConfig::Pistache PkgConfig::Sqlite PkgConfig::Jsoncpp pthread)

if (NG_AUDIO_COMPATIBILITY_ENDPOINTS)
        target_compile_options(ng-backend PRIVATE -DNG_AUDIO_BRIDGE_COMPAT)
	target_link_libraries(ng-backend curl)
endif()
