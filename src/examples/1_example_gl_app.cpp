#include "../slim/core/transform.h"
#include "../slim/math/mat4_constructurs.h"
#include "../slim/viewport/navigation.h"
#include "../slim/viewport/frustum.h"
#include "../slim/scene/selection.h"
#include "../slim/draw/selection.h"


#include "../slim/app.h"
#include "../slim/gl/gl_shader.h"
#include "../slim/gl/gl_texture.h"
#include "../slim/gl/gl_mesh.h"
#include "../slim/gl/gl_light.h"
#include "../slim/gl/gl_material.h"
#include "../slim/gl/gl_skybox.h"
#include "../slim/gl/gl_edges.h"
#include "../slim/gl/gl_quad.h"

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


SkyBoxImages skybox_images;
const char *cube_map_image_file_names[6] {
    "cupertin-lake_rt.raw_image",
    "cupertin-lake_lf.raw_image",
    "cupertin-lake_up.raw_image",
    "cupertin-lake_dn.raw_image",
    "cupertin-lake_bk.raw_image",
    "cupertin-lake_ft.raw_image"
};
ImagePack<u8> cube_map_image_pack{6, skybox_images.array, cube_map_image_file_names, __FILE__};
//#include "./images.h"

File main_vertex_shader_file{"shader.vert", __FILE__};
File main_fragment_shader_file{"shader.frag", __FILE__};
File directional_shadow_map_vertex_shader_file{"directional_shadow_map.vert", __FILE__};
File directional_shadow_map_fragment_shader_file{"directional_shadow_map.frag", __FILE__};
File omni_directional_shadow_map_vertex_shader_file{"omni_directional_shadow_map.vert", __FILE__};
File omni_directional_shadow_map_geometry_shader_file{"omni_directional_shadow_map.geom", __FILE__};
File omni_directional_shadow_map_fragment_shader_file{"omni_directional_shadow_map.frag", __FILE__};


constexpr int MAX_POINT_LIGHTS = 3;
constexpr int MAX_SPOT_LIGHTS = 3;

struct ExampleGLApp : SlimApp {
    Camera camera{{0, 0, 0}, {0, 0, 0}};
    CameraRayProjection camera_ray_projection;

    Canvas canvas;
    Viewport viewport{canvas, &camera};

    GLShader main_pass_vertex_shader{GL_VERTEX_SHADER, main_vertex_shader_file.path};
    GLShader main_pass_fragment_shader{GL_FRAGMENT_SHADER, main_fragment_shader_file.path};
    GLProgram main_pass_shader_program{&main_pass_vertex_shader, 2};

    GLShader directional_shadow_pass_vertex_shader{GL_VERTEX_SHADER, directional_shadow_map_vertex_shader_file.path};
    GLShader directional_shadow_pass_fragment_shader{GL_FRAGMENT_SHADER, directional_shadow_map_fragment_shader_file.path};
    GLProgram directional_shadow_pass_shader_program{&directional_shadow_pass_vertex_shader, 2};

    GLShader omni_directional_shadow_pass_vertex_shader{GL_VERTEX_SHADER, omni_directional_shadow_map_vertex_shader_file.path};
    GLShader omni_directional_shadow_pass_fragment_shader{GL_FRAGMENT_SHADER, omni_directional_shadow_map_fragment_shader_file.path};
    GLShader omni_directional_shadow_pass_geometry_shader{GL_GEOMETRY_SHADER, omni_directional_shadow_map_geometry_shader_file.path};
    GLProgram omni_directional_shadow_pass_shader_program{&omni_directional_shadow_pass_vertex_shader, 3};

    
    GLTexture floor_albedo{images[Floor_Albedo]};
    GLTexture floor_normal{images[Floor_Normal]};
    GLTexture dog_albedo{images[Dog_Albedo]};
    GLTexture dog_normal{images[Dog_Normal]};

    DirectionalLight directional_light{
        {1.0f, 0.53f, 0.3f}, 0.1f, 0.9f, {-10.0f, -12.0f, 18.5f}, {0, 0, 5}
    };
    PointLight point_lights[2]{
        {{0.0f, 0.0f, 1.0f}, 0.0f, 1.0f, {1.0f, 2.0f, 0.0f}, 0.3f, 0.2f, 0.1f},
        {{0.0f, 1.0f, 0.0f}, 0.0f, 1.0f, {-4.0f, 3.0f, 0.0f}, 0.3f, 0.2f, 0.1f}
    };
    int point_lights_count = 2;

    SpotLight spot_lights[2]{
        {{1.0f, 0.0f, 1.0f}, 0.0f, 2.0f, {0.0f, 0.0f, 0.0f}, 1.0f, 0.0f, 0.0f, {0.0f, -1.0f, 0.0f}, 20.0f},
        {{0.0f, 1.0f, 1.0f}, 0.0f, 1.0f, {0.0f, -1.5f, 0.0f}, 1.0f, 0.0f, 0.0f, {-100.0f, -1.0f, 0.0f}, 20.0f}
    };
    int spot_lights_count = 2;

    enum MaterialID { FloorMaterial, DogMaterial, Rough, Phong, Blinn, Mirror, Glass, MaterialCount };

    u8 flags{MATERIAL_HAS_NORMAL_MAP | MATERIAL_HAS_ALBEDO_MAP};
    Material floor_material{0.7f, 0.9f, flags,
                            2, {Floor_Albedo, Floor_Normal}};
    Material dog_material{1.0f, 0.6f, flags,
                          2, {Dog_Albedo, Dog_Normal}};
    Material rough_material{0.8f, 0.7f};
    Material phong_material{1.0f,0.5f,0,0, {},
                            BRDF_Phong, 1.0f, 1.0, 0.0f, IOR_AIR,
                            {1.0f, 1.0f, 0.4f}};
    Material blinn_material{{1.0f, 0.3f, 1.0f},0.5f,0,0, {},
                            BRDF_Blinn, 1.0f, 1.0, 0.0f, IOR_AIR,
                            {1.0f, 0.4f, 1.0f}};
    Material mirror_material{
        0.0f,0.02f,
        MATERIAL_IS_REFLECTIVE,
        0, {},
        BRDF_CookTorrance, 1.0f, 1.0,  0.0f,
        IOR_AIR, F0_Aluminium
    };
    Material glass_material {
        0.05f,0.25f,
        MATERIAL_IS_REFRACTIVE,
        0,{},
        BRDF_CookTorrance, 1.0f, 1.0f, 0.0f,
        IOR_GLASS, F0_Glass_Low
    };
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
                    GeometryType_Mesh, Glass,      Dragon};
    Geometry floor{{{},{0, -3, 0}, {20.0f, 1.0f, 20.0f}},
                   GeometryType_Mesh, Glass,      Floor};
    Geometry directional_light_0{ {{DEG_TO_RAD*-35, DEG_TO_RAD*-30, 0}, {directional_light.position}, {0.4f}},
                   GeometryType_Mesh, Glass,      Floor};
    Geometry point_light_0{ {{}, point_lights[0].position, {0.2f}},
                   GeometryType_Mesh, Glass,      Floor};
    Geometry point_light_1{ {{}, point_lights[1].position, {0.2f}},
                   GeometryType_Mesh, Glass,      Floor};
    Geometry spot_light_0{ {{}, spot_lights[0].position, {0.2f}},
                   GeometryType_Mesh, Glass,      Floor};
    Geometry spot_light_1{ {{}, spot_lights[1].position, {0.2f}},
                   GeometryType_Mesh, Glass,      Floor};
    Geometry *geometries{&dog};

    Scene scene{{8, 1, 0, MaterialCount, 0, MeshCount - 1},
                geometries, &camera, nullptr, materials, nullptr, nullptr, meshes, mesh_files};

    SceneTracer scene_tracer{scene.counts.geometries, scene.mesh_stack_size};
    Selection selection{scene, scene_tracer, camera_ray_projection};
    
    GLMesh gl_dog_mesh{dog_mesh};
    GLMesh gl_dragon_mesh{dragon_mesh};
    GLMesh gl_floor_mesh{floor_mesh};
    GLMaterial shiny_material{4.0f, 256};
    GLMaterial dull_material{0.3f, 4};


    struct MainPassUniforms {
        GLMatrix4Uniform model{"model"};
        GLMatrix4Uniform view{"view"};
        GLMatrix4Uniform projection{"projection"};
        GLVector3Uniform camera_position{"eyePosition"};

        GLTextureUniform texture{"theTexture"};
        GLFloatUniform shininess{"material", "shininess"};
        GLFloatUniform specular_intensity{"material", "specularIntensity"};

        GLIntUniform point_light_count{"pointLightCount"};
        GLIntUniform spot_light_count{"spotLightCount"};

        GLDirectionalLight directional_light{"directionalLight", "base"};

        GLPointLight point_light_0{"pointLights", "base", 0};
        GLPointLight point_light_1{"pointLights", "base", 1};
        GLPointLight *point_lights{&point_light_0};

        GLSpotLight spot_light_0{"spotLights", "base", "base", 0, MAX_POINT_LIGHTS};
        GLSpotLight spot_light_1{"spotLights", "base", "base", 1, MAX_POINT_LIGHTS};
        GLSpotLight *spot_lights{&spot_light_0};

        MainPassUniforms(GLuint shader_program_id) {
            view.setLocation(shader_program_id);
            model.setLocation(shader_program_id);
            projection.setLocation(shader_program_id);
            camera_position.setLocation(shader_program_id);

            texture.setLocation(shader_program_id);
            shininess.setLocation(shader_program_id);
            specular_intensity.setLocation(shader_program_id);

            point_light_count.setLocation(shader_program_id);
            spot_light_count.setLocation(shader_program_id);

            directional_light.init(shader_program_id);

            point_light_0.init(shader_program_id);
            point_light_1.init(shader_program_id);
            spot_light_0.init(shader_program_id);
            spot_light_1.init(shader_program_id);
        }

        void updatePointLights(const PointLight *lights, u8 count) {
            if (count > MAX_POINT_LIGHTS) count = (u8)MAX_POINT_LIGHTS;
            point_light_count.update(count);
            for (u8 i = 0; i < count; i++) point_lights[i].update(lights[i], i + 3);
        }

        void updateSpotLights(const SpotLight *lights, u8 count) {
            if (count > MAX_SPOT_LIGHTS) count = (u8)MAX_POINT_LIGHTS;
            spot_light_count.update(count);
            for (u8 i = 0; i < count; i++) spot_lights[i].update(lights[i], MAX_POINT_LIGHTS + i + 3);
        }
    };
    MainPassUniforms main_pass_uniforms{main_pass_shader_program.id};

    struct ShadowPassUniforms {
        GLShadowMap directional_shadow_map;
        GLMatrix4Uniform model{"model"};
        GLMatrix4Uniform directional_light_transform{"directionalLightTransform"};

        ShadowPassUniforms(GLuint shader_program_id) {
            directional_shadow_map.init();
            model.setLocation(shader_program_id);
            directional_light_transform.setLocation(shader_program_id);
        }
    };
    ShadowPassUniforms directional_shadow_pass_uniforms{directional_shadow_pass_shader_program.id};

    struct OmniDirectionalShadowPassUniforms {
        GLMatrix4Uniform model{"model"};
        GLMatrix4Uniform light_matrix_1{"lightMatrices", nullptr, nullptr, 0};
        GLMatrix4Uniform light_matrix_2{"lightMatrices", nullptr, nullptr, 1};
        GLMatrix4Uniform light_matrix_3{"lightMatrices", nullptr, nullptr, 2};
        GLMatrix4Uniform light_matrix_4{"lightMatrices", nullptr, nullptr, 3};
        GLMatrix4Uniform light_matrix_5{"lightMatrices", nullptr, nullptr, 4};
        GLMatrix4Uniform light_matrix_6{"lightMatrices", nullptr, nullptr, 5};
        GLVector3Uniform light_pos{"lightPos"};
        GLFloatUniform far_plane{"farPlane"};

        OmniDirectionalShadowPassUniforms(GLuint shader_program_id) {
            model.setLocation(shader_program_id);
            far_plane.setLocation(shader_program_id);
            light_pos.setLocation(shader_program_id);
            light_matrix_1.setLocation(shader_program_id);
            light_matrix_2.setLocation(shader_program_id);
            light_matrix_3.setLocation(shader_program_id);
            light_matrix_4.setLocation(shader_program_id);
            light_matrix_5.setLocation(shader_program_id);
            light_matrix_6.setLocation(shader_program_id);
        }
    };
    OmniDirectionalShadowPassUniforms omni_directional_shadow_pass_uniforms{omni_directional_shadow_pass_shader_program.id};

    GLSkyBox skybox{skybox_images};
    //GLEdges dog_edges;//{dragon_mesh.edge_vertex_indices, dragon_mesh.edge_count, dragon_mesh.vertex_positions, dragon_mesh.vertex_count};
    //GLEdges cube_edges{floor_mesh.edge_vertex_indices, floor_mesh.edge_count, floor_mesh.vertex_positions, floor_mesh.vertex_count};

    mat4 view_projection;

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

    void RenderPass() {
        // Clear the window
        glViewport(0, 0, viewport.dimensions.width, viewport.dimensions.height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glEnable(GL_CULL_FACE);

        mat3 camera_ray_matrix = camera.orientation;
        camera_ray_matrix.Z *= camera.focal_length;
        camera_ray_matrix.X /= viewport.frustum.projection.params.height_over_width;
        skybox.draw(camera_ray_matrix);

        glUseProgram(main_pass_shader_program.id);
        
        main_pass_uniforms.projection.update(Mat4(viewport.frustum.projection));
        main_pass_uniforms.view.update(Mat4(camera).inverted());
        main_pass_uniforms.camera_position.update(camera.position);
       
        main_pass_uniforms.updatePointLights(point_lights, point_lights_count);
        main_pass_uniforms.updateSpotLights(spot_lights, spot_lights_count);
        main_pass_uniforms.directional_light.update(directional_light);

        directional_shadow_pass_uniforms.directional_shadow_map.read(GL_TEXTURE2);

        main_pass_uniforms.texture.update(1);
        main_pass_uniforms.directional_light.shadow_map_texture.update(2);

        main_pass_uniforms.model.update(Mat4(dog.transform));
        dog_albedo.bind();
        dull_material.bind(main_pass_uniforms.specular_intensity.id, main_pass_uniforms.shininess.id);
        gl_dog_mesh.render();

        main_pass_uniforms.model.update(Mat4(dragon.transform));
        dog_albedo.bind();
        dull_material.bind(main_pass_uniforms.specular_intensity.id, main_pass_uniforms.shininess.id);
        gl_dragon_mesh.render();

        main_pass_uniforms.model.update(Mat4(floor.transform));
        floor_albedo.bind();
        dull_material.bind(main_pass_uniforms.specular_intensity.id, main_pass_uniforms.shininess.id);
        gl_floor_mesh.render();

        gl::cube::draw(Mat4(directional_light_0.transform) * view_projection, BrightYellow);
        gl::cube::draw(Mat4(point_light_0.transform) * view_projection, BrightCyan);
        gl::cube::draw(Mat4(point_light_1.transform) * view_projection, BrightCyan);
        gl::cube::draw(Mat4(spot_light_0.transform) * view_projection, BrightBlue);
        gl::cube::draw(Mat4(spot_light_1.transform) * view_projection, BrightBlue);
        //dog_edges.draw(Mat4(dog.transform) * view_projection, Red);

        /*
        vec3 Z{directional_light.direction.normalized()};
        vec3 X{vec3::Y.cross(Z)};
        vec3 Y{Z.cross(X)};*/
        mat4 V{Mat4(directional_light.orientation , directional_light.position)};
        gl::cube::draw(Mat4(Mat3(Mat4(directional_light.projection).inverted())) * V * view_projection);
    }

    void DirectionalShadowMapPass() {
        glUseProgram(directional_shadow_pass_shader_program.id);
        glViewport(0, 0,
                   directional_shadow_pass_uniforms.directional_shadow_map.width,
                   directional_shadow_pass_uniforms.directional_shadow_map.height);

        directional_shadow_pass_uniforms.directional_shadow_map.write();
        glClear(GL_DEPTH_BUFFER_BIT);

        directional_shadow_pass_uniforms.directional_light_transform.update(directional_light.transformationMatrix());
        
        directional_shadow_pass_uniforms.model.update(Mat4(dog.transform));
        gl_dog_mesh.render();
        directional_shadow_pass_uniforms.model.update(Mat4(dragon.transform));
        gl_dragon_mesh.render();
        directional_shadow_pass_uniforms.model.update(Mat4(floor.transform));
        gl_floor_mesh.render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }    

    void OmniShadowMapPass(GLPointLight* gl_light, PointLight* light)
    {
        static mat4 light_matrices[6];

        glUseProgram(omni_directional_shadow_pass_shader_program.id);
        glViewport(0, 0, gl_light->shadow_map.width, gl_light->shadow_map.height);

        gl_light->shadow_map.write();

        glClear(GL_DEPTH_BUFFER_BIT);

        light->setTransforms(light_matrices);
        omni_directional_shadow_pass_uniforms.light_matrix_1.update(light_matrices[0]);
        omni_directional_shadow_pass_uniforms.light_matrix_2.update(light_matrices[1]);
        omni_directional_shadow_pass_uniforms.light_matrix_3.update(light_matrices[2]);
        omni_directional_shadow_pass_uniforms.light_matrix_4.update(light_matrices[3]);
        omni_directional_shadow_pass_uniforms.light_matrix_5.update(light_matrices[4]);
        omni_directional_shadow_pass_uniforms.light_matrix_6.update(light_matrices[5]);
        omni_directional_shadow_pass_uniforms.light_pos.update(light->position);
        omni_directional_shadow_pass_uniforms.far_plane.update(light->projection.params.far_distance);

        omni_directional_shadow_pass_uniforms.model.update(Mat4(dog.transform));
        gl_dog_mesh.render();
        omni_directional_shadow_pass_uniforms.model.update(Mat4(dragon.transform));
        gl_dragon_mesh.render();
        omni_directional_shadow_pass_uniforms.model.update(Mat4(floor.transform));
        gl_floor_mesh.render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OnRender() override {
        DirectionalShadowMapPass();
        for (size_t i = 0; i < point_lights_count; i++)
        {
            OmniShadowMapPass(&main_pass_uniforms.point_lights[i], &point_lights[i]);
        }
        for (size_t i = 0; i < spot_lights_count; i++)
        {
            OmniShadowMapPass(&main_pass_uniforms.spot_lights[i], &spot_lights[i]);
        }
        RenderPass();
        drawSelection(selection, view_projection, meshes);
        glUseProgram(0);
    }

    void OnInit() override {
        window::width = 640;
        window::height = 480;
        OnWindowResize(window::width, window::height);

        camera.focal_length = atan(45.0f);
        viewport.frustum.far_clipping_plane_distance = 100.0f;
        viewport.frustum.near_clipping_plane_distance = 0.1f;
        updateProjection();
        
        glEnable(GL_LINE_SMOOTH);
	    glHint(GL_LINE_SMOOTH_HINT,  GL_NICEST);

        vec3 *dog_normal_edges = new vec3[dog_mesh.triangle_count * 6];
        for (int i = 0; i < dog_mesh.triangle_count; i++) {
            auto &position_ids = dog_mesh.vertex_position_indices[i];
            auto &normal_ids = dog_mesh.vertex_normal_indices[i];
            dog_normal_edges[i * 6 + 0] = dog_mesh.vertex_positions[position_ids.v1];
            dog_normal_edges[i * 6 + 1] = dog_mesh.vertex_positions[position_ids.v1] + dog_mesh.vertex_normals[normal_ids.v1] * 0.01f;
            dog_normal_edges[i * 6 + 2] = dog_mesh.vertex_positions[position_ids.v2];
            dog_normal_edges[i * 6 + 3] = dog_mesh.vertex_positions[position_ids.v2] + dog_mesh.vertex_normals[normal_ids.v2] * 0.01f;
            dog_normal_edges[i * 6 + 4] = dog_mesh.vertex_positions[position_ids.v3];
            dog_normal_edges[i * 6 + 5] = dog_mesh.vertex_positions[position_ids.v3] + dog_mesh.vertex_normals[normal_ids.v3] * 0.01f;
        }
        //dog_edges.load(dog_normal_edges, dog_mesh.triangle_count * 6);
        delete[] dog_normal_edges; 
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
        
        directional_light.orientation = Mat3(directional_light_0.transform.orientation);
        directional_light.position = directional_light_0.transform.position;

        point_lights[0].position = point_light_0.transform.position;
        point_lights[1].position = point_light_1.transform.position;
        
        spot_lights[0].position = spot_light_0.transform.position;
        spot_lights[1].position = spot_light_1.transform.position;
        
        spot_lights[0].direction = Mat3(spot_light_0.transform.orientation).forward;
        spot_lights[1].direction = Mat3(spot_light_1.transform.orientation).forward;

        //spot_lights[0].position = camera.position - vec3{0.0, 0.3f, 0.0f};
        //spot_lights[0].direction = camera.orientation.forward.normalized();

        view_projection = Mat4(camera).inverted() * Mat4(viewport.frustum.projection);
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
    }

    void OnShutdown() override {

    }
};

SlimApp* createApp() {
    return (SlimApp*)new ExampleGLApp();
}