cmake_minimum_required(VERSION 3.8)

link_libraries(opengl32.lib)
include_directories(src/slim src/slim/gl/core src/slim/gl/renderer src/slim/gl/scene)

project(1_example_gl_app)
add_executable(1_example_gl_app WIN32 "src/examples/1_example_gl_app.cpp")

project(obj2mesh)
add_executable(obj2mesh src/obj2mesh.cpp)

project(stb2image)
add_executable(stb2image src/image_loaders/stb2image.cpp)

project(ft2font)
add_executable(ft2font src/font_loaders/ft2font.cpp)
target_include_directories(ft2font PUBLIC ${CMAKE_SOURCE_DIR}/src/font_loaders ${CMAKE_SOURCE_DIR}/src/font_loaders/msdfgen)
target_link_libraries(ft2font PUBLIC ${CMAKE_SOURCE_DIR}/src/font_loaders/freetype/freetype.lib  ${CMAKE_SOURCE_DIR}/src/font_loaders/msdfgen/msdfgen.lib  ${CMAKE_SOURCE_DIR}/src/font_loaders/msdfgen/msdf-atlas-gen.lib)