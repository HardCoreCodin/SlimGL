cmake_minimum_required(VERSION 3.8)

link_libraries(opengl32.lib)

project(1_clipping)
add_executable(1_clipping WIN32 src/examples/1_clipping.cpp)

#project(2_normal_maps)
#add_executable(2_normal_maps WIN32 src/examples/2_normal_maps.cpp)
#
#project(3_mipmaps)
#add_executable(3_mipmaps WIN32 src/examples/3_mipmaps.cpp)

project(obj2mesh)
add_executable(obj2mesh src/obj2mesh.cpp)

project(bmp2texture)
add_executable(bmp2texture src/bmp2texture.cpp)