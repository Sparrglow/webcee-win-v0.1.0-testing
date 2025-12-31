cmake_minimum_required(VERSION 3.10)
project(WebCee)

if(MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/wd4996 /wd4267 /wd4013 /wd4047 /wd4819)
endif()

# --- 1. Build-time Parser Selection ---
# WebCee uses a parser to compile .wce files into generated C code + web assets.
# We prioritize Python for its rich features, but fallback to a native C parser
# if Python is not available (Zero Dependency Philosophy).

option(WEBCEE_FORCE_C_PARSER "Force using the native C parser even if Python is available" OFF)

if(NOT WEBCEE_FORCE_C_PARSER)
    find_package(Python3 COMPONENTS Interpreter QUIET)
endif()

if(Python3_FOUND)
    message(STATUS "WebCee: Python3 found (${Python3_VERSION}). Using Python parser.")
    set(WCE_PARSER_CMD ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/tools/wce_parser.py)
    set(WCE_PARSER_DEPENDS ${CMAKE_CURRENT_LIST_DIR}/tools/wce_parser.py)
    set(WCE_PARSER_TYPE "PYTHON")
else()
    message(STATUS "WebCee: Python3 NOT found. Using C parser fallback (tools/wce_parser.c).")
    
    # Define the C parser executable target
    # This will be compiled on-the-fly during the build process
    add_executable(wce_parser_tool tools/wce_parser.c)
    
    # Use the compiled executable as the command
    set(WCE_PARSER_CMD $<TARGET_FILE:wce_parser_tool>)
    set(WCE_PARSER_DEPENDS wce_parser_tool)
    set(WCE_PARSER_TYPE "C")
endif()

# --- 2. WebCee Library ---
add_library(webcee STATIC src/webcee.c)
target_include_directories(webcee PUBLIC include)
target_compile_definitions(webcee PRIVATE WEBCEE_IMPLEMENTATION=1)

if(WIN32)
    target_link_libraries(webcee ws2_32)
elseif(UNIX)
    target_link_libraries(webcee pthread)
endif()

# --- 3. Integration Helper ---
# Usage: target_add_webcee_ui(my_app ui/main.wce)
function(target_add_webcee_ui TARGET_NAME WCE_FILE)
    get_filename_component(WCE_ABS ${WCE_FILE} ABSOLUTE)
    get_filename_component(WCE_NAME ${WCE_FILE} NAME_WE)
    
    set(GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/webcee_gen_${WCE_NAME})
    file(MAKE_DIRECTORY ${GEN_DIR})
    
    set(GEN_C ${GEN_DIR}/webcee_generated.c)
    set(GEN_H ${GEN_DIR}/webcee_generated.h)

    # Unified CLI for both parsers:
    #   build <input.wce> -o <output_dir>
    set(PARSER_ARGS build ${WCE_ABS} -o ${GEN_DIR})
    
    add_custom_command(
        OUTPUT ${GEN_C} ${GEN_H}
        COMMAND ${WCE_PARSER_CMD} ${PARSER_ARGS}
        DEPENDS ${WCE_PARSER_DEPENDS} ${WCE_ABS}
        COMMENT "WebCee: Compiling UI ${WCE_NAME} using ${WCE_PARSER_TYPE} parser..."
    )
    
    # Add generated files to the target
    target_sources(${TARGET_NAME} PRIVATE ${GEN_C})
    target_include_directories(${TARGET_NAME} PRIVATE ${GEN_DIR})

    # Make generated declarations available via webcee.h (UX: only include webcee.h)
    target_compile_definitions(${TARGET_NAME} PRIVATE WEBCEE_HAS_GENERATED=1 WEBCEE_EXTERN_ONLY=1)
    
    # Link WebCee library
    target_link_libraries(${TARGET_NAME} PRIVATE webcee)

    # Deploy assets to executable directory
    add_custom_command(
        TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${GEN_DIR}/web_root
        $<TARGET_FILE_DIR:${TARGET_NAME}>/web_root
        COMMENT "WebCee: Deploying assets to output directory..."
    )
endfunction()

# --- 4. Examples ---
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_subdirectory(examples/hello_world)
endif()
