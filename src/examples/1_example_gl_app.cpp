#include "../slim/app.h"
#include "../slim/core/transform.h"
#include "../slim/viewport/navigation.h"
#include "../slim/viewport/frustum.h"
#include "../slim/scene/selection.h"
#include "../slim/draw/selection.h"
#include "../slim/serialization/image.h"
#include "../slim/gl/gl_renderer.h"


enum ImageID {
    Floor_Albedo,
    Floor_Normal,

    Dog_Albedo,
    Dog_Normal,

    ImageCount
};

RawImage images[4];
const char *image_file_names[ImageCount] {
    "floor_albedo.raw_image",
    "floor_normal.raw_image",
    "dog_albedo.raw_image",
    "dog_normal.raw_image"
};
ImagePack<u8> image_pack{ImageCount, images, image_file_names, __FILE__};

constexpr u8 CUBE_MAP_SETS_COUNT = 2;
constexpr int MAX_POINT_LIGHTS = 3;
constexpr int MAX_SPOT_LIGHTS = 3;

union CubeMapSets {
    struct {
        CubeMapSet cathedral;
        CubeMapSet bolonga;
    };
    CubeMapSet array[CUBE_MAP_SETS_COUNT];

    CubeMapSets() {}
};
CubeMapSets cube_map_sets;

const char *bolonga_akybox[6] {
    "bolonga_color_full_px.raw_image",
    "bolonga_color_full_nx.raw_image",
    "bolonga_color_full_py.raw_image",
    "bolonga_color_full_ny.raw_image",
    "bolonga_color_full_pz.raw_image",
    "bolonga_color_full_nz.raw_image"
};
ImagePack<u8> bolonga_skybox_image_pack{6, cube_map_sets.bolonga.skybox.array, bolonga_akybox, __FILE__};

const char *bolonga_radiance[6] {
    "bolonga_radiance_px.raw_image",
    "bolonga_radiance_nx.raw_image",
    "bolonga_radiance_py.raw_image",
    "bolonga_radiance_ny.raw_image",
    "bolonga_radiance_pz.raw_image",
    "bolonga_radiance_nz.raw_image"
};
ImagePack<u8> bolonga_radiance_image_pack{6, cube_map_sets.bolonga.radiance.array, bolonga_radiance, __FILE__};

const char *bolonga_irradiance[6] {
    "bolonga_irradiance_px.raw_image",
    "bolonga_irradiance_nx.raw_image",
    "bolonga_irradiance_py.raw_image",
    "bolonga_irradiance_ny.raw_image",
    "bolonga_irradiance_pz.raw_image",
    "bolonga_irradiance_nz.raw_image"
};
ImagePack<u8> bolonga_irradiance_image_pack{6, cube_map_sets.bolonga.irradiance.array, bolonga_irradiance, __FILE__};


const char *cathedral_akybox[6] {
    "cathedral_color_full_px.raw_image",
    "cathedral_color_full_nx.raw_image",
    "cathedral_color_full_py.raw_image",
    "cathedral_color_full_ny.raw_image",
    "cathedral_color_full_pz.raw_image",
    "cathedral_color_full_nz.raw_image"
};
ImagePack<u8> cathedral_skybox_image_pack{6, cube_map_sets.cathedral.skybox.array, cathedral_akybox, __FILE__};

const char *cathedral_radiance[6] {
    "cathedral_radiance_px.raw_image",
    "cathedral_radiance_nx.raw_image",
    "cathedral_radiance_py.raw_image",
    "cathedral_radiance_ny.raw_image",
    "cathedral_radiance_pz.raw_image",
    "cathedral_radiance_nz.raw_image"
};
ImagePack<u8> cathedral_radiance_image_pack{6, cube_map_sets.cathedral.radiance.array, cathedral_radiance, __FILE__};

const char *cathedral_irradiance[6] {
    "cathedral_irradiance_px.raw_image",
    "cathedral_irradiance_nx.raw_image",
    "cathedral_irradiance_py.raw_image",
    "cathedral_irradiance_ny.raw_image",
    "cathedral_irradiance_pz.raw_image",
    "cathedral_irradiance_nz.raw_image"
};
ImagePack<u8> cathedral_irradiance_image_pack{6, cube_map_sets.cathedral.irradiance.array, cathedral_irradiance, __FILE__};


struct ExampleGLApp : SlimApp {
    Camera camera{{-25 * DEG_TO_RAD, 0, 0}, {0, 7, -11}}, *cameras{&camera};
    CameraRayProjection camera_ray_projection;
    Canvas canvas;
    Viewport viewport{canvas, &camera};
    
    DirectionalLight directional_light{
        {-60*DEG_TO_RAD, 30*DEG_TO_RAD, 0}, {1.0f, 0.83f, 0.7f}, 1.7f, {0, 25, -20}
    };
    PointLight point_lights[2]{
        {{1.0f, 2.0f, 0.0f}, Blue, 200.0f},
        {{-4.0f, 3.0f, 0.0f}, Green, 200.0f}
    };
    SpotLight spot_lights[2]{
        {{DEG_TO_RAD*-90, DEG_TO_RAD*0, 0}, {3, 4, 5}, Magenta, 300.0f},
        {{DEG_TO_RAD*-90, DEG_TO_RAD*0, 0}, {-3, 4, -5}, Cyan, 300.0f}
    };

    enum MaterialID { FloorMaterial, DogMaterial, DragonMaterial, MaterialCount };

    u8 flags{MATERIAL_HAS_NORMAL_MAP | MATERIAL_HAS_ALBEDO_MAP};
    Material floor_material{0.6f, 0.2f, flags, 2, {Floor_Albedo, Floor_Normal}};
    Material dog_material{1.0f, 0.3f, flags, 2, {Dog_Albedo, Dog_Normal}};
    Material dragon_material{1.0f, 0.3f, 0, 0, {Dog_Albedo, Dog_Normal}, 1.0f, F0_Gold};
    Material *materials{&floor_material};

    enum MesheID { Dog, Dragon, Floor, MeshCount };

    Mesh dog_mesh;
    Mesh dragon_mesh;
    Mesh floor_mesh{CubeEdgesType::BBox, true};
    Mesh *meshes = &dog_mesh;
    char mesh_file_string_buffers[MeshCount - 1][100]{};
    String mesh_files[MeshCount - 1] = {
        String::getFilePath("dog.mesh"   ,mesh_file_string_buffers[Dog   ],__FILE__),
        String::getFilePath("dragon.mesh",mesh_file_string_buffers[Dragon],__FILE__)
    };

    Geometry dog   {{{0, -45 * DEG_TO_RAD, 0},{4, 2.1f, 3}, 0.8f}, GeometryType_Mesh, DogMaterial,    Dog};
    Geometry dragon{{{},{-12, 2, -3}},                             GeometryType_Mesh, DragonMaterial, Dragon};
    Geometry floor{{{},{0, -3, 0}, {20.0f, 1.0f, 20.0f}},          GeometryType_Mesh, FloorMaterial,  Floor};
    Geometry *geometries{&dog};

    Scene scene{{3, 1, 1, 2, 2, MaterialCount, 0, MeshCount - 1},
        geometries, &camera, &directional_light, point_lights, spot_lights, materials, 
        nullptr, nullptr, meshes, mesh_files};

    SceneTracer scene_tracer{scene.counts.geometries, scene.mesh_stack_size};
    Selection selection{scene, scene_tracer, camera_ray_projection};

    void OnInit() override {
        window::width = 640;
        window::height = 480;
        OnWindowResize(window::width, window::height);

        camera.focal_length = atan(45.0f);
        viewport.frustum.far_clipping_plane_distance = 100.0f;
        viewport.frustum.near_clipping_plane_distance = 0.1f;
        viewport.frustum.updateProjection(camera.focal_length, viewport.dimensions.height_over_width);

        scene.counts.meshes = MeshCount;
        gl::renderer::init(scene, cube_map_sets.array, CUBE_MAP_SETS_COUNT, images, 4, true, true);
    }

    void OnUpdate(f32 delta_time) override {
        scene.updateAABBs();
        scene.updateBVH();
        camera_ray_projection.reset(camera, viewport.dimensions, false);

        if (!mouse::is_captured) selection.manipulate(viewport);
        if (!controls::is_pressed::alt) {
            viewport.navigation.update(camera, delta_time);
            viewport.frustum.updateProjection(camera.focal_length, viewport.dimensions.height_over_width);
        }
    }

    void OnRender() override { 
        gl::renderer::render(viewport, &selection); 
    }

    void OnMouseButtonDown(mouse::Button &mouse_button) override {
        mouse::pos_raw_diff_x = mouse::pos_raw_diff_y = 0;
    }

    void OnMouseButtonDoubleClicked(mouse::Button &mouse_button) override {
        if (&mouse_button == &mouse::left_button) {
            mouse::is_captured = !mouse::is_captured;
            os::setCursorVisibility(!mouse::is_captured);
            os::setWindowCapture(    mouse::is_captured);
            OnMouseButtonDown(mouse_button);
        }
    }

    void OnKeyChanged(u8 key, bool is_pressed) override {
        Move &move = viewport.navigation.move;
        Turn &turn = viewport.navigation.turn;
        if (key == 'Q') turn.left     = is_pressed;
        if (key == 'E') turn.right    = is_pressed;
        if (key == 'R') move.up       = is_pressed;
        if (key == 'F') move.down     = is_pressed;
        if (key == 'W') move.forward  = is_pressed;
        if (key == 'S') move.backward = is_pressed;
        if (key == 'A') move.left     = is_pressed;
        if (key == 'D') move.right    = is_pressed;

        if (!is_pressed) {
            if (key == 'V') gl::renderer::settings::wireframe = !gl::renderer::settings::wireframe;
            if (key == 'N') gl::renderer::settings::normals = !gl::renderer::settings::normals;
            if (key == 'C') gl::renderer::settings::cube_map_set_index = (gl::renderer::settings::cube_map_set_index + 1) % CUBE_MAP_SETS_COUNT;
            if (key == 'I') gl::renderer::settings::IBL_intensity += controls::is_pressed::ctrl ? -1.0f : 1.0f;
            if (key == 'L') directional_light.intensity += controls::is_pressed::ctrl ? -0.1f : 0.1f;
            if (gl::renderer::settings::IBL_intensity < 0.0f)
                gl::renderer::settings::IBL_intensity = 0.0f;
            if (directional_light.intensity < 0.0f)
                directional_light.intensity = 0.0f;
        }
    }
        
    void OnWindowResize(u16 width, u16 height) override { 
        viewport.dimensions.update(width, height);
        viewport.frustum.updateProjection(camera.focal_length, viewport.dimensions.height_over_width);
    }
};

SlimApp* createApp() {
    return (SlimApp*)new ExampleGLApp();
}