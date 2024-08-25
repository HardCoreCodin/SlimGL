#pragma once

#include "gl_mesh.h"
#include "gl_edges.h"
#include "gl_light.h"
#include "gl_skybox.h"
#include "gl_texture.h"
#include "gl_material.h"
#include "gl_shadow_map.h"

#include "../math/mat4_constructurs.h"
#include "../scene/scene.h"
#include "../scene/selection.h"
#include "../draw/selection.h"


namespace gl {
	constexpr int MAX_POINT_LIGHTS = 3;
	constexpr int MAX_SPOT_LIGHTS = 3;

	namespace renderer {
		const Scene *scene = nullptr;
		GLMesh *meshes = nullptr;
		GLEdges *mesh_edges = nullptr;
		GLEdges *mesh_normals = nullptr;
		GLTexture *textures = nullptr;

		mat4 view_matrix;
		mat4 projection_matrix;
		mat4 view_projection_matrix;
		mat4 *model_matrices = nullptr;

		namespace directional_shadow_pass {
			GLProgram program;
			GLMatrix4Uniform mvp{"mvp"};
			GLDirectionalShadowMap shadow_map;
        
			void init() {
				if (program.id) return;
            
				GLShader shaders[] = {
					{GL_VERTEX_SHADER, nullptr, base_vertex_shader},
					{GL_FRAGMENT_SHADER, nullptr, empty_fragment_shader}
				};
				program.compile(shaders, 2);
				mvp.setLocation(program.id);
				shadow_map.init();
			}

			void render(const mat4 &shadow_matrix) {				
				if (!program.id) init();

				glUseProgram(program.id);
				glViewport(0, 0, shadow_map.width, shadow_map.height);

				shadow_map.write();
				glClear(GL_DEPTH_BUFFER_BIT);

				for (int i = 0; i < scene->counts.geometries; i++)
				{
					mvp.update(model_matrices[i] * shadow_matrix);
					meshes[scene->geometries[i].id].render();
				}
				
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}
		}

		namespace omni_directional_shadow_pass {    
			const char* geometry_shader = R"(#version 330
	layout (triangles) in;
	layout (triangle_strip, max_vertices=18) out;

	uniform mat4 lightMatrices[6];

	out vec4 FragPos;

	void main()
	{
		for(int face = 0; face < 6; ++face)
		{
			gl_Layer = face;
			for(int i = 0; i < 3; i++)
			{
				FragPos = gl_in[i].gl_Position;
				gl_Position = lightMatrices[face] * FragPos;
				EmitVertex();
			}
			EndPrimitive();
		}
	})";

			const char* fragment_shader = R"(#version 330
	in vec4 FragPos;

	uniform vec3 lightPos;
	uniform float farPlane;

	void main()
	{
		float distance = length(FragPos.xyz - lightPos);
		distance = distance/farPlane;
		gl_FragDepth = distance;
	})";

			GLMatrix4Uniform mvp{"mvp"};
			GLMatrix4Uniform light_matrix_1{"lightMatrices", nullptr, nullptr, 0};
			GLMatrix4Uniform light_matrix_2{"lightMatrices", nullptr, nullptr, 1};
			GLMatrix4Uniform light_matrix_3{"lightMatrices", nullptr, nullptr, 2};
			GLMatrix4Uniform light_matrix_4{"lightMatrices", nullptr, nullptr, 3};
			GLMatrix4Uniform light_matrix_5{"lightMatrices", nullptr, nullptr, 4};
			GLMatrix4Uniform light_matrix_6{"lightMatrices", nullptr, nullptr, 5};
			GLVector3Uniform light_pos{"lightPos"};
			GLFloatUniform far_plane{"farPlane"};
			GLProgram program;

			void init() {
				if (program.id) return;
            
				GLShader shaders[] = {
					{GL_VERTEX_SHADER, nullptr, base_vertex_shader},
					{GL_GEOMETRY_SHADER, nullptr, geometry_shader},
					{GL_FRAGMENT_SHADER, nullptr, fragment_shader}
				};
				program.compile(shaders, 3);
				mvp.setLocation(program.id);
				far_plane.setLocation(program.id);
				light_pos.setLocation(program.id);
				light_matrix_1.setLocation(program.id);
				light_matrix_2.setLocation(program.id);
				light_matrix_3.setLocation(program.id);
				light_matrix_4.setLocation(program.id);
				light_matrix_5.setLocation(program.id);
				light_matrix_6.setLocation(program.id);
			}

			void render(const GLPointLight &gl_light, const PointLight &light) {
				if (!program.id) init();

				static mat4 shadow_map_matrices[6];

				glUseProgram(program.id);
				glViewport(0, 0, gl_light.shadow_map.width, gl_light.shadow_map.height);

				gl_light.shadow_map.write();

				glClear(GL_DEPTH_BUFFER_BIT);

				light.setShadowMapMatrices(shadow_map_matrices);
				light_matrix_1.update(shadow_map_matrices[0]);
				light_matrix_2.update(shadow_map_matrices[1]);
				light_matrix_3.update(shadow_map_matrices[2]);
				light_matrix_4.update(shadow_map_matrices[3]);
				light_matrix_5.update(shadow_map_matrices[4]);
				light_matrix_6.update(shadow_map_matrices[5]);
				light_pos.update(light.position);
				far_plane.update(light.shadow_bounds.far_distance);

				for (int i = 0; i < scene->counts.geometries; i++)
				{
					mvp.update(model_matrices[i]);
					meshes[scene->geometries[i].id].render();
				}

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			}  
		}

		namespace main_render_pass {
			GLProgram program;

			GLMatrix4Uniform model{"model"};
			GLMatrix4Uniform view{"view"};
			GLMatrix4Uniform projection{"projection"};
			GLVector3Uniform camera_position{"eyePosition"};

			GLMaterial material{"material"};
			GLTextureUniform albedo_map{"albedo_map"};
			GLTextureUniform normal_map{"normal_map"};

			GLDirectionalLight directional_light{"directionalLight", "base"};
			GLIntUniform point_light_count{"pointLightCount"};
			GLIntUniform spot_light_count{"spotLightCount"};
			GLPointLight point_lights[MAX_POINT_LIGHTS];
			GLSpotLight spot_lights[MAX_SPOT_LIGHTS];

			void init(Scene &main_scene) {
				if (program.id) return;

				GLShader shaders[] = {
					{GL_VERTEX_SHADER, File{"shader.vert", __FILE__}.path},
					{GL_FRAGMENT_SHADER, File{"shader.frag", __FILE__}.path}
				};
				program.compile(shaders, 2);

				view.setLocation(program.id);
				model.setLocation(program.id);
				projection.setLocation(program.id);
				camera_position.setLocation(program.id);

				albedo_map.setLocation(program.id);
				normal_map.setLocation(program.id);

				material.init(program.id);

				point_light_count.setLocation(program.id);
				point_light_count.update(main_scene.counts.point_lights);
				spot_light_count.setLocation(program.id);
				spot_light_count.update(main_scene.counts.spot_lights);
				directional_light.init(program.id);
			
				if (main_scene.counts.point_lights > MAX_POINT_LIGHTS)
					main_scene.counts.point_lights = MAX_POINT_LIGHTS;
				for (int i = 0; i < main_scene.counts.point_lights; i++)
				{
					new(&point_lights[i])GLPointLight{"pointLights", "base", i};
					point_lights[i].init(program.id);
				}

				if (main_scene.counts.spot_lights > MAX_SPOT_LIGHTS)
					main_scene.counts.spot_lights = MAX_SPOT_LIGHTS;
				for (int i = 0; i < main_scene.counts.spot_lights; i++)
				{
					new(&spot_lights[i])GLSpotLight{"spotLights", "base", "base", i, MAX_POINT_LIGHTS};
					spot_lights[i].init(program.id);
				}
			}

			void render(const Viewport &viewport) {
				const Camera &camera{*viewport.camera};
				mat3 camera_ray_matrix = camera.orientation;
				camera_ray_matrix.Z *= camera.focal_length;
				camera_ray_matrix.X /= viewport.frustum.projection.params.height_over_width;
				skybox::draw(camera_ray_matrix);

				glUseProgram(program.id);
        
				projection.update(projection_matrix);
				view.update(view_matrix);
				camera_position.update(camera.position);
       
				point_light_count.update(scene->counts.point_lights);
				for (u8 i = 0; i < scene->counts.point_lights; i++) 
					point_lights[i].update(scene->point_lights[i], i + 4);

				spot_light_count.update(scene->counts.spot_lights);
				for (u8 i = 0; i < scene->counts.spot_lights; i++) 
					spot_lights[i].update(scene->spot_lights[i], MAX_POINT_LIGHTS + i + 4);

				directional_light.update(scene->directional_lights[0]);
				directional_shadow_pass::shadow_map.read(GL_TEXTURE3);

				albedo_map.update(1);
				normal_map.update(2);
				directional_light.shadow_map_texture.update(3);

				
				for (int i = 0; i < scene->counts.geometries; i++)
				{
					const Geometry &geo{scene->geometries[i]};
					const Material &geo_material{scene->materials[geo.material_id]};
					const GLTexture &albedo_map_texture{textures[geo_material.texture_ids[0]]};
					const GLTexture &normal_map_texture{textures[geo_material.texture_ids[1]]};
					const GLMesh &mesh{meshes[geo.id]};

					model.update(model_matrices[i]);
					material.update(geo_material);
					albedo_map_texture.bind(GL_TEXTURE1);
					normal_map_texture.bind(GL_TEXTURE2);
					mesh.render();
				}
				
				for (int i = 0; i < scene->counts.point_lights; i++) cube::draw(scene->point_lights[i].transformationMatrix() * view_projection_matrix, BrightCyan);
				for (int i = 0; i < scene->counts.spot_lights; i++)  cube::draw(scene->spot_lights[i].transformationMatrix() * view_projection_matrix, BrightBlue);
				cube::draw(scene->directional_lights[0].transformationMatrix() * view_projection_matrix, BrightYellow);
				cube::draw(scene->directional_lights[0].shadowBoundsMatrix() * view_projection_matrix);
			}
		}
		

		void init(Scene &main_scene, CubeMapImages &skybox_images, RawImage *texture_images, u32 texture_count, bool wireframe = false, bool normals = false) {			
			scene = &main_scene;
			model_matrices = new mat4[main_scene.counts.geometries];
			meshes = new GLMesh[main_scene.counts.meshes];
			if (wireframe) mesh_edges = new GLEdges[main_scene.counts.meshes];
			if (normals) mesh_normals = new GLEdges[main_scene.counts.meshes];
			Mesh *mesh = scene->meshes;
			for (int m = 0; m < main_scene.counts.meshes; m++, mesh++) {
				meshes[m].create(*mesh);
				if (wireframe) mesh_edges[m].load(mesh->edge_vertex_indices, mesh->edge_count, mesh->vertex_positions, mesh->vertex_count);
				if (normals) {
					vec3 *normal_edges = new vec3[mesh->triangle_count * 6];
					for (int i = 0; i < mesh->triangle_count; i++) {
						auto &position_ids = mesh->vertex_position_indices[i];
						auto &normal_ids = mesh->vertex_normal_indices[i];
						normal_edges[i * 6 + 0] = mesh->vertex_positions[position_ids.v1];
						normal_edges[i * 6 + 1] = mesh->vertex_positions[position_ids.v1] + mesh->vertex_normals[normal_ids.v1] * 0.01f;
						normal_edges[i * 6 + 2] = mesh->vertex_positions[position_ids.v2];
						normal_edges[i * 6 + 3] = mesh->vertex_positions[position_ids.v2] + mesh->vertex_normals[normal_ids.v2] * 0.01f;
						normal_edges[i * 6 + 4] = mesh->vertex_positions[position_ids.v3];
						normal_edges[i * 6 + 5] = mesh->vertex_positions[position_ids.v3] + mesh->vertex_normals[normal_ids.v3] * 0.01f;
					}
					mesh_normals[m].load(normal_edges, mesh->triangle_count * 6);
					delete[] normal_edges; 
				}
			}


			textures = new GLTexture[texture_count];
			for (int i = 0; i < texture_count; i++)
				new(&textures[i])GLTexture(texture_images[i]);
			
			skybox::init(skybox_images);
			main_render_pass::init(main_scene);

			glEnable(GL_LINE_SMOOTH);
			glHint(GL_LINE_SMOOTH_HINT,  GL_NICEST);
		}
		
		void render(const Viewport &viewport, Selection *selection = nullptr, bool main_pass = true, bool wireframe = false, bool normals = false) {
			// Init matrices
			view_matrix = Mat4(*viewport.camera).inverted();
			projection_matrix = Mat4(viewport.frustum.projection);
			view_projection_matrix = view_matrix * projection_matrix;
			for (int i = 0; i < scene->counts.geometries; i++) model_matrices[i] = Mat4(scene->geometries[i].transform); 

			if (!wireframe) {
				// Generate shadow maps
				directional_shadow_pass::render(scene->directional_lights[0].shadowMapMatrix());
				for (size_t i = 0; i < scene->counts.point_lights; i++) omni_directional_shadow_pass::render(main_render_pass::point_lights[i], scene->point_lights[i]);
				for (size_t i = 0; i < scene->counts.spot_lights; i++)  omni_directional_shadow_pass::render(main_render_pass::spot_lights[i], scene->spot_lights[i]);
			}

			glViewport(0, 0, viewport.dimensions.width, viewport.dimensions.height);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_CULL_FACE);

			if (main_pass) main_render_pass::render(viewport);
			if (wireframe && mesh_edges || 
				normals && mesh_normals) {
				mat4 mvp;
				for (int i = 0; i < scene->counts.geometries; i++) {
					const Geometry &geo{scene->geometries[i]};
					if (geo.type == GeometryType_Mesh) {
						mvp = model_matrices[i] * view_projection_matrix;
						if (wireframe && mesh_edges) mesh_edges[geo.id].draw(mvp, geo.color);
						if (normals && mesh_normals) mesh_normals[geo.id].draw(mvp, geo.color);
					}
				}
			}

			if (selection) drawSelection(*selection, view_projection_matrix, scene->meshes);
			
			glUseProgram(0);
		}
	}
}