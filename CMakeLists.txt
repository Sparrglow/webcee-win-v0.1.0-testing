cmake_minimum_required(VERSION 3.10)
project(WebCee C)

# --- 1. Build the Compiler Tool ---
file(GLOB COMPILER_SOURCES "compiler/core/*.c" "compiler/main.c")
add_executable(wce_tool ${COMPILER_SOURCES})
target_include_directories(wce_tool PRIVATE compiler/core)

# --- 2. WebCee Runtime Library ---
add_library(webcee STATIC src/webcee.c)
target_include_directories(webcee PUBLIC include)

if(WIN32)
    target_link_libraries(webcee ws2_32)
elseif(UNIX)
    target_link_libraries(webcee pthread)
endif()

# --- 3. Integration Helper ---
function(target_add_webcee_ui TARGET_NAME WCE_FILE)
    get_filename_component(WCE_ABS ${WCE_FILE} ABSOLUTE)
    get_filename_component(WCE_NAME ${WCE_FILE} NAME_WE)
    
    set(GEN_C ${CMAKE_CURRENT_BINARY_DIR}/${WCE_NAME}_gen.c)
    
    add_custom_command(
        OUTPUT ${GEN_C}
        COMMAND wce_tool ${WCE_ABS} ${GEN_C}
        DEPENDS wce_tool ${WCE_ABS}
        COMMENT "WebCee: Compiling UI ${WCE_NAME}..."
    )
    
    target_sources(${TARGET_NAME} PRIVATE ${GEN_C})
    target_compile_definitions(${TARGET_NAME} PRIVATE WEBCEE_HAS_GENERATED=1)
    target_link_libraries(${TARGET_NAME} PRIVATE webcee)
endfunction()

# --- 4. Examples ---
add_executable(showcase examples/showcase/main.c)
target_add_webcee_ui(showcase examples/showcase/ui.wce)
