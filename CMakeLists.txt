cmake_minimum_required(VERSION 3.25)
project(helios LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(BuildFromPkg "Build using the package version" OFF)

add_executable(helios
    main.cpp
    src/core/core.cpp
    src/lexer.cpp
    src/parser.cpp
    src/semantics.cpp
    src/preprocessor.cpp
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
  find_package(unofficial-uwebsockets CONFIG REQUIRED)
  target_link_libraries(helios PRIVATE
    ws2_32
    mswsock
    unofficial::uwebsockets::uwebsockets
        )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Android")  
  
    if(BuildFromPkg)
        message(STATUS "Logic for: Building from Package")
        find_package(uwebsockets REQUIRED)
        find_package(indicators REQUIRED)
        target_link_libraries(helios PRIVATE uwebsockets::uwebsockets)
        target_link_libraries(helios PRIVATE indicators::indicators)
    # Add your package-specific commands here
    else()
        find_package(ZLIB REQUIRED)
        find_package(Threads REQUIRED)

        target_link_libraries(helios PRIVATE 
            uSockets
            ZLIB::ZLIB
            Threads::Threads
        )
    endif()
endif()

# Optional: warnings (safe)
if (MSVC)   
    target_compile_options(helios PRIVATE /w)  #W4
else()
    target_compile_options(helios PRIVATE -w)
endif()
