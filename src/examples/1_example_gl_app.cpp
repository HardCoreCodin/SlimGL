#include "../slim/core/transform.h"
#include "../slim/viewport/navigation.h"
#include "../slim/viewport/frustum.h"
#include "../slim/scene/selection.h"
#include "../slim/draw/selection.h"


#include "../slim/app.h"
#include "../slim/gl/gl_renderer.h"

#include "../slim/serialization/image.h"


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


CubeMapImages skybox_images;
const char *cube_map_image_file_names[6] {
    "cupertin-lake_rt.raw_image",
    "cupertin-lake_lf.raw_image",
    "cupertin-lake_up.raw_image",
    "cupertin-lake_dn.raw_image",
    "cupertin-lake_bk.raw_image",
    "cupertin-lake_ft.raw_image"
};
ImagePack<u8> cube_map_image_pack{6, skybox_images.array, cube_map_image_file_names, __FILE__};


constexpr int MAX_POINT_LIGHTS = 3;
constexpr int MAX_SPOT_LIGHTS = 3;

struct ExampleGLApp : SlimApp {
    Camera camera{{-25 * DEG_TO_RAD, 0, 0}, {0, 7, -11}}, *cameras{&camera};

    CameraRayProjection camera_ray_projection;

    Canvas canvas;
    Viewport viewport{canvas, &camera};
    
    GLTexture floor_albedo{images[Floor_Albedo]};
    GLTexture floor_normal{images[Floor_Normal]};
    GLTexture dog_albedo{images[Dog_Albedo]};
    GLTexture dog_normal{images[Dog_Normal]};

    DirectionalLight directional_light{
        {-60*DEG_TO_RAD, 30*DEG_TO_RAD, 0}, {1.0f, 0.53f, 0.3f}, {0, 25, -20}
    };
    PointLight point_lights[2]{
        {{1.0f, 2.0f, 0.0f}, Blue, {}, {0.0f, 1.0f} },
        {{-4.0f, 3.0f, 0.0f}, Green, {}, {0.0f, 1.0f} }
    };
    int point_lights_count = 2;

    SpotLight spot_lights[2]{
        {{DEG_TO_RAD*-90, DEG_TO_RAD*0, 0}, {3, 4, 5}, Magenta, {}, {0.0f, 2.0f}},
        {{}, {}, Cyan, {}, {0.0f, 0.0f}}
    };
    int spot_lights_count = 2;

    enum MaterialID { FloorMaterial, DogMaterial, MaterialCount };

    u8 flags{MATERIAL_HAS_NORMAL_MAP | MATERIAL_HAS_ALBEDO_MAP};
    Material floor_material{0.3f, 4, 0.7f, 0.9f, flags, 2, {Floor_Albedo, Floor_Normal}};
    Material dog_material{0.3f, 4, 1.0f, 0.6f, flags, 2, {Dog_Albedo, Dog_Normal}};
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
        OrientationUsingQuaternion rot{0, -45 * DEG_TO_RAD, 0};

    Geometry dog   {{rot,{4, 2.1f, 3}, 0.8f},
                    GeometryType_Mesh, DogMaterial,   Dog};
    Geometry dragon{{{},{-12, 2, -3}},
                    GeometryType_Mesh, DogMaterial,      Dragon};
    Geometry floor{{{},{0, -3, 0}, {20.0f, 1.0f, 20.0f}},
                   GeometryType_Mesh, FloorMaterial,      Floor};
    Geometry *geometries{&dog};

    Scene scene{{3, 1, 1, 2, 2, MaterialCount, 0, MeshCount - 1},
                geometries, &camera, &directional_light, point_lights, spot_lights, materials, nullptr, nullptr, meshes, mesh_files};

    SceneTracer scene_tracer{scene.counts.geometries, scene.mesh_stack_size};
    Selection selection{scene, scene_tracer, camera_ray_projection};

    bool wireframe = false;
    bool normals = false;
    
    void updateProjection() {
        viewport.frustum.updateProjection(camera.focal_length, viewport.dimensions.height_over_width);
    }

    void updateDimensions(u16 width, u16 height) {
        viewport.dimensions.update(width, height);
        updateProjection();
    }

    void updateNavigation(f32 delta_time) {
        viewport.navigation.update(camera, delta_time);
        updateProjection();
    }

    void OnRender() override {
        gl::renderer::render(viewport, &selection, true, wireframe, normals);
    }

    void OnInit() override {
        window::width = 640;
        window::height = 480;
        OnWindowResize(window::width, window::height);

        camera.focal_length = atan(45.0f);
        viewport.frustum.far_clipping_plane_distance = 100.0f;
        viewport.frustum.near_clipping_plane_distance = 0.1f;
        updateProjection();

        scene.counts.meshes = MeshCount;
        gl::renderer::init(scene, skybox_images, images, 4, true, true);
    }

    void OnWindowResize(u16 width, u16 height) override {
        updateDimensions(width, height);
    }

    void OnUpdate(f32 delta_time) override {
        scene.updateAABBs();
        scene.updateBVH();
        camera_ray_projection.reset(camera, viewport.dimensions, false);

        if (!mouse::is_captured) selection.manipulate(viewport);
        if (!controls::is_pressed::alt) updateNavigation(delta_time);
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
        if (key == 'V' && !is_pressed) wireframe = !wireframe;
        if (key == 'N' && !is_pressed) normals = !normals;
    }
};

SlimApp* createApp() {
    return (SlimApp*)new ExampleGLApp();
}