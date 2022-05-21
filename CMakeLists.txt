cmake_minimum_required(VERSION 2.8.12)

project(cr CXX C)

set(CMAKE_C_FLAGS "-g")
set(CMAKE_CXX_FLAGS "-std=c++11 -g")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/builds/cmake)

# src
set (cr_src
    src/engine/Graphics.cpp
    src/engine/render/glad.c
    src/engine/render/OpenGL.cpp
    src/engine/render/Vulkan.cpp
    src/engine/render/Texture.cpp
    src/engine/render/Shader.cpp
    src/engine/render/Model.cpp
    src/engine/Core.cpp
    src/engine/Input.cpp
    src/engine/common/3rdparty/Jzon.cpp
    src/engine/common/3rdparty/MD5.cpp
    src/engine/common/Indexer.cpp
    src/engine/common/Resource.cpp
    src/engine/common/Result.cpp
    src/engine/common/Log.cpp
    src/engine/common/Tools.cpp    
    src/engine/common/Type.cpp  
    src/engine/common/Job.cpp)

add_library(cr_bin ${cr_src})

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

IF(LINUX)
	target_link_libraries(cr_bin -m64 -lGL -lGLU -lglfw -pthread -ldl  -lrt -lXrandr -lXi -lXinerama -lX11 -lXxf86vm -lvulkan -lassimp -lz)
ENDIF(LINUX)

IF(MINGW)
	target_link_libraries(cr_bin -m64 -lglfw3 -pthread -lgdi32 -lopengl32 -lvulkan-1 -lassimp -lIrrXML -lzlib)
ENDIF(MINGW)

# Apple most likely won't work
IF(APPLE)
	target_link_libraries(cr_bin -lglfw3 -pthread -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -lvulkan-1 -lassimp -lIrrXML -lzlib)
ENDIF(APPLE)

add_executable(astro src/CR.cpp)
target_link_libraries(astro cr_bin)