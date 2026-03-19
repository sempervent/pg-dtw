cmake_minimum_required(VERSION 3.20)
project(pg_dtw VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

option(PG_DTW_BUILD_FUZZ "Build fuzz/property test harness" ON)
set(PG_DTW_EXTENSION_NAME "pg_dtw" CACHE STRING "PostgreSQL extension identifier")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(PgDtwExtension)

pg_dtw_find_postgresql()

add_library(pg_dtw MODULE
    src/pg_dtw.c
    src/options.c
    src/array_input.c
    src/canonicalize.c
    src/dtw_kernel.c
    src/normalize.c
    src/lower_bounds.c
)

target_include_directories(pg_dtw PRIVATE
    ${POSTGRESQL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(pg_dtw PRIVATE
    -Wall
    -Wextra
    -Werror
)

target_link_libraries(pg_dtw PRIVATE ${POSTGRESQL_LIBRARIES} m)
set_target_properties(pg_dtw PROPERTIES PREFIX "" OUTPUT_NAME "${PG_DTW_EXTENSION_NAME}")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/sql/pg_dtw.control.in
    ${CMAKE_CURRENT_BINARY_DIR}/pg_dtw.control
    @ONLY
)

enable_testing()
include(CTest)

if(PG_DTW_BUILD_FUZZ)
    add_executable(pg_dtw_fuzz
        test/fuzz/fuzz_dtw.c
        test/fuzz/reference_dtw.c
    )
    target_include_directories(pg_dtw_fuzz PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(pg_dtw_fuzz PRIVATE m)
    add_test(NAME fuzz_properties COMMAND pg_dtw_fuzz)
endif()

install(TARGETS pg_dtw DESTINATION "${POSTGRESQL_PKGLIBDIR}")
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/pg_dtw.control
    DESTINATION "${POSTGRESQL_EXTENSION_DIR}"
)
install(FILES
    sql/pg_dtw--0.0.1.sql
    DESTINATION "${POSTGRESQL_EXTENSION_DIR}"
)
