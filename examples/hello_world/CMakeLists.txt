project(hello_world)

add_executable(hello_world src/main.c)

# Use the WebCee integration macro provided by the root CMakeLists.txt
target_add_webcee_ui(hello_world ui/pure_c.wce)

# Ensure we can find the generated headers
target_include_directories(hello_world PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
