cmake_minimum_required(VERSION 3.8)

link_libraries(opengl32.lib)

#project(1_clipping)
#add_executable(1_clipping WIN32 src/examples/1_clipping.cpp)

#project(2_normal_maps)
#add_executable(2_normal_maps WIN32 src/examples/2_normal_maps.cpp)

project(1_example_gl_app)
add_executable(1_example_gl_app WIN32 "src/examples/1_example_gl_app.cpp")

project(obj2mesh)
add_executable(obj2mesh src/obj2mesh.cpp)

project(bmp2texture)
add_executable(bmp2texture src/image_loaders/bmp2texture.cpp)

project(stb2image)
add_executable(stb2image src/image_loaders/stb2image.cpp)