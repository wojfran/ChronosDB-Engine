set(TARGET_NAME chronosdb_cli)

add_executable(${TARGET_NAME}
    src/main.cpp
)

target_include_directories(${TARGET_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/core/include
    ${CMAKE_SOURCE_DIR}/common/include
)

target_link_libraries(${TARGET_NAME} PRIVATE
    chronosdb_core
)

# SQLite dependency is already linked via chronosdb_core
