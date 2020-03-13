project(FFmpeg)

cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)

find_package(FFmpeg)

set(ffmpeg_SRC
  src/main.cpp
  src/ffmpeg.cpp
  src/ffmpeg_video.cpp
  src/pbdecoderthread.cpp
  src/pbvideowidget.cpp
  src/pbdecoderthread.cpp
  src/pbframehandler.cpp
)

# Tell CMake to create the helloworld executable
add_executable(ffmpeg ${ffmpeg_SRC})

# Use the Widgets module from Qt 5.
#target_link_libraries(ffmpeg Qt5::Widgets /usr/lib/ffmpeg2.8/libavutil.so /usr/lib/ffmpeg2.8/libavformat.so /usr/lib/ffmpeg2.8/libavresample.so /usr/lib/ffmpeg2.8/libavcodec.so /usr/lib/ffmpeg2.8/libswscale.so)
target_link_libraries(ffmpeg Qt5::Widgets ${FFMPEG_LIBRARIES})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
