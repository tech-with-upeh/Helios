cmake_minimum_required(VERSION 3.16)
project(helios LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(unofficial-uwebsockets CONFIG REQUIRED)

add_executable(helios
    main.cpp
    src/core/core.cpp
    src/lexer.cpp
    src/parser.cpp
    src/semantics.cpp
    src/webengine/webengine.cpp
    src/webengine/parseconfig.cpp
    
)
target_include_directories(helios
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/web
        ${PROJECT_SOURCE_DIR}/include/devx
        ${PROJECT_SOURCE_DIR}/include/core
)


if(WIN32)
        target_link_libraries(helios PRIVATE
            ws2_32
            mswsock
            unofficial::uwebsockets::uwebsockets
        )
endif() 

# Optional: warnings (safe)
if (MSVC)   
    target_compile_options(helios PRIVATE /w)  #W4
else()
    target_compile_options(helios PRIVATE -w)
endif()
