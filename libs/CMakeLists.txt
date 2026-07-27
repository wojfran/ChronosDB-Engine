add_library(sqlite3 STATIC 
    sqlite-amalgamation-3510300/sqlite3.c
)

target_include_directories(sqlite3 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/sqlite-amalgamation-3510300
)

# Disable warnings for sqlite3 amalgamation to keep build output clean
if(MSVC)
    target_compile_options(sqlite3 PRIVATE /W0)
else()
    target_compile_options(sqlite3 PRIVATE -w)
endif()
