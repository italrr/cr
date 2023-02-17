cmake_minimum_required(VERSION 2.8.12)

project(cr CXX C)

set(CMAKE_C_FLAGS "-g")
set(CMAKE_CXX_FLAGS "-std=c++11 -g")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/builds/cmake)

# src
set (cr_src
    src/Engine/Graphics.cpp
    src/Engine/Log.cpp
    src/Engine/Job.cpp
    src/Engine/Result.cpp
    src/Engine/Tools.cpp
    src/Engine/Types.cpp
    src/Engine/Font.cpp
    src/Engine/Input.cpp
    src/Engine/Shader.cpp
    src/Engine/Network.cpp
    src/Engine/Texture.cpp
    src/Engine/Model.cpp
    src/Engine/Resource.cpp
    src/Engine/3rdparty/glad.c
    src/Engine/3rdparty/Jzon.cpp
    src/Engine/3rdparty/MD5.cpp
    src/Map.cpp
    src/Entity.cpp
    src/World.cpp
    src/Network.cpp
    src/Server.cpp
    src/Client.cpp

)

add_library(cr_bin ${cr_src})

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

IF(LINUX)
	target_link_libraries(cr_bin -m64 -lGL -lGLU -lglfw -pthread -ldl  -lrt -lXrandr -lXi -lXinerama -lX11 -lXxf86vm -lassimp -lfreetype -lz)
ENDIF(LINUX)

IF(MINGW)
	target_link_libraries(cr_bin -m64 -lglfw3 -pthread -lgdi32 -lopengl32 -lassimp -lfreetype -lz -lws2_32)
ENDIF(MINGW)

# Apple most likely won't work
IF(APPLE)
	target_link_libraries(cr_bin -lglfw3 -pthread -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -lvulkan-1 -lassimp -lfreetype -lIrrXML -lzlib)
ENDIF(APPLE)

add_executable(cr src/CR.cpp)
target_link_libraries(cr cr_bin)