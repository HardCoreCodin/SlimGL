#pragma once

#include "../math/utils.h"
#include "../draw/line.h"
#include "../scene/scene.h"
#include "../viewport/viewport.h"

// Culling flags:
// ======================
#define IS_NEAR  0b00000001
#define IS_FAR   0b00000010
#define IS_BELOW 0b00000100
#define IS_ABOVE 0b00001000
#define IS_RIGHT 0b00010000
#define IS_LEFT  0b00100000
#define IS_OUT   0b00111111
#define IS_NDC   0b01000000

// Clipping flags:
// ===============
#define CULL    0b00000000
#define INSIDE  0b00000010



struct Rasterizer {
    static CubeMesh cube;
    Scene &scene;

    u8 *vertex_flags;
    vec3 *world_space_vertex_positions, *world_space_vertex_normals;
    vec4 *clip_space_vertex_positions;
    mat4 model_to_world_inverted_transposed, model_to_world, world_to_clip;

    static u64 GetMemorySize(u32 max_vertex_positions, u32 max_vertex_normals) {
        return (u64)max_vertex_positions * (sizeof(vec3) + sizeof(vec4) + 1) + sizeof(vec3) * (u64)max_vertex_normals;
    }
    static u64 GetMemorySize(const Scene &scene) {
        return GetMemorySize(scene.max_vertex_positions, scene.max_vertex_normals);
    }
    static u64 GetMemorySize(u32 mesh_count, String *mesh_files) {
        u32 max_vertex_positions = 0;
        u32 max_vertex_normals = 0;
        Mesh mesh;
        String *mesh_file = mesh_files;
        for (u32 i = 0; i < mesh_count; i++, mesh_file++) {
            void *file = os::openFileForReading(mesh_file->char_ptr);
            if (!file)
                continue;
            readHeader(mesh, file);
            os::closeFile(file);

            if (mesh.vertex_count  > max_vertex_positions) max_vertex_positions = mesh.vertex_count;
            if (mesh.normals_count > max_vertex_normals) max_vertex_normals     = mesh.normals_count;
        }
        return GetMemorySize(max_vertex_positions, max_vertex_normals);
    }

    explicit Rasterizer(Scene &scene, memory::MonotonicAllocator *memory_allocator = nullptr) : scene{scene} {
        memory::MonotonicAllocator temp_allocator;
        if (!memory_allocator) {
            temp_allocator = memory::MonotonicAllocator{GetMemorySize(scene), Terabytes(3)};
            memory_allocator = &temp_allocator;
        }
        vertex_flags = (u8*)memory_allocator->allocate(scene.max_vertex_positions);
        clip_space_vertex_positions  = (vec4*)memory_allocator->allocate(sizeof(vec4) * scene.max_vertex_positions);
        world_space_vertex_positions = (vec3*)memory_allocator->allocate(sizeof(vec3) * scene.max_vertex_positions);
        world_space_vertex_normals   = (vec3*)memory_allocator->allocate(sizeof(vec3) * scene.max_vertex_normals);
    };

    void rasterize(const Viewport &viewport, bool draw_wireframe = false) {
        const Camera &camera = *viewport.camera;
        const Dimensions &dim = viewport.dimensions;
        const Frustum &frustum = viewport.frustum;
        const Frustum::Projection &projection = frustum.projection;

        const f32 pz = projection.shear;
        const f32 n = frustum.near_clipping_plane_distance;
        static vec3 world_positions[6], normals[6];
        static vec4 positions[6];
        static vec2 uvs[6], uv_in, uv_out;
        Shaded shaded;

        mat4 world_to_view{Mat4(camera.rotation, camera.position).inverted()};
        mat4 view_to_clip{
                projection.scale.x, 0, 0, 0,
                0, projection.scale.y, 0, 0,
                0, 0, projection.scale.z, 1,
                0, 0, projection.shear, 0
        };
        world_to_clip = world_to_view * view_to_clip;

        bool last_UV_taken;
        f32 dot, t, one_minus_t, one_over_ABC, ABC, du, dv, pixel_depth;
        u32 face_index, vertex_index, face_count, vertex_count, clipped_index, pixel_offset,
            v1_index, out1_index, in1_index, first_x, last_x,
            v2_index, out2_index, in2_index, first_y, last_y,
            v3_index;
        u8 v1_flags, new_v2num, out1_num, in1_num,
           v2_flags, new_v1num, out2_num, in2_num,
           v3_flags;

        vec2 A, B, C, AB, AC, BC, pixel_min, pixel_max, uv1, uv2, uv3, lastUV, screen_transform;
        vec3 normal, attr_in, attr_out, new_v1, new_v2, pos1, pos2, pos3, norm1, norm2, norm3, ABCw, ABCp, w, dX, dY, K, bary, bary_start;
        vec4 v1, v2, v3, in1, in2, out1, out2, *position, *clipped;

        vec2 last_pixel_coord{dim.f_width - 1, dim.f_height - 1};
        f32 width = dim.f_width, height = dim.f_height;
        i32 stride = dim.width;
        if ( viewport.canvas.antialias) {
            screen_transform.x = dim.f_width;
            screen_transform.y = dim.f_height;
            stride <<= 1;
            width *= 2;
            height *= 2;
            last_pixel_coord = {width - 1, height - 1};
        } else {
            screen_transform.x = dim.h_width;
            screen_transform.y = dim.h_height;
        }

        PixelShader pixel_shader;
        shaded.viewing_origin = viewport.camera->position;

        bool mesh_has_normals, mesh_has_uvs, clipping_produced_an_extra_face;
        TriangleVertexIndices position_indices, normal_indices, uvs_indices;
        Pixel *pixel;
        f32 *depth;
        Mesh *mesh;
        Geometry *geometry = scene.geometries;
        for (u32 geometry_id = 0; geometry_id < scene.counts.geometries; geometry_id++, geometry++) {
            if (geometry->type == GeometryType_Box)
                mesh = &cube;
            else if (geometry->type == GeometryType_Mesh)
                mesh = scene.meshes + geometry->id;
            else
                continue;

            shaded.geometry = geometry;
            shaded.material = scene.materials + geometry->material_id;
            vertex_count = mesh->vertex_count;

            // Prepare a matrix for converting from model space to clip space:
            model_to_world = Mat4(geometry->transform.rotation,
                                  geometry->transform.scale,
                                  geometry->transform.position);
            model_to_world_inverted_transposed = model_to_world.inverted().transposed();

            // Execute mesh shader and skip this geometry if it got culled:
            if (!shaded.material->mesh_shader(*mesh, *this))
                continue;

            // Cull the vertices and skip this geometry if it's entirely outside the view frustum:


            // Check vertex positions against the frustum:
            // ----------------------------------------------------
            // For each vertex, check where it is in relation to the view frustum,
            // and collect the results into a flags array (single number bit-pattern per-vertex).
            // While doing so, keep track of which side(s) of the frustum are shared by all vertices.
            // When done, first analyze the results before continuing to phase 2.
            // Early bail-out if any of these conditions are met:
            // a. The entire mesh is outside the frustum - early return with the CULL flag.
            // b. The entire mesh is inside the frustum - early return with the INSIDE flag.
            // c. No geometric clipping is needed - early return with the INSIDE flag.
            bool needs_clipping = false;
            bool has_inside = false;

            u8 shared_directions = IS_OUT;
            u8        directions = IS_OUT;

            u8 *flags = vertex_flags;
            position = clip_space_vertex_positions;
            for (vertex_index = 0; vertex_index < vertex_count; vertex_index++, position++, flags++) {
                *flags = CULL;

                if (position->z < 0) {
                    // Af at lease one vertex is outside the view frustum behind the near clipping plane,
                    // the geometry needs to be checked for clipping
                    needs_clipping = true;
                    *flags = IS_NEAR;
                    continue;
                } else directions = position->z > position->w ? IS_FAR : 0;

                if (     position->x >  position->w) directions |= IS_RIGHT;
                else if (position->x < -position->w) directions |= IS_LEFT;

                if (     position->y >  position->w) directions |= IS_ABOVE;
                else if (position->y < -position->w) directions |= IS_BELOW;

                if (directions) {
                    // This vertex is outside of the view frustum.
                    *flags = directions;
                    // Note: This flag 'may' get removed from this vertex before the perspective-devide
                    // (so it won't be skipped, essentially bringing it back) if it's still needed for culling/clipping.

                    // Intersect the shared directions so-far, against this current out-direction:
                    shared_directions &= directions;
                    // Note: This will end-up beign zero if either:
                    // a. All vertices are inside the frustum - no need for face clipping.
                    // b. All vertices are outside the frustum in at least one direction shared by all.
                    //   (All vertices are above and/or all vertices on the left and/or all vertices behind, etc.)
                } else {
                    has_inside = true;
                    *flags = IS_NDC;
                }
            }

            if (!has_inside && shared_directions)
                // All vertices are completely outside, and all share at least one out-region.
                // The entire mesh is completely outside the frustum and does not intersect it in any way.
                // It can be safely culled altogether.
                continue;

            // The mesh intersects the frustum in some way

            pixel_shader = shaded.material->pixel_shader;

            mesh_has_uvs     = mesh->uvs_count     != 0;
            mesh_has_normals = mesh->normals_count != 0;
            face_count = mesh->triangle_count;

            // Check its faces as well and check for clipping cases:
            for (face_index = 0; face_index < face_count; face_index++) {
                // Fetch the index and out-direction flags of each of the face's vertices:
                position_indices = mesh->vertex_position_indices[face_index];

                v1_index = position_indices.v1;
                v2_index = position_indices.v2;
                v3_index = position_indices.v3;

                v1_flags = vertex_flags[v1_index] & IS_OUT;
                v2_flags = vertex_flags[v2_index] & IS_OUT;
                v3_flags = vertex_flags[v3_index] & IS_OUT;

                positions[0] = clip_space_vertex_positions[v1_index];
                positions[1] = clip_space_vertex_positions[v2_index];
                positions[2] = clip_space_vertex_positions[v3_index];

                world_positions[0] = world_space_vertex_positions[v1_index];
                world_positions[1] = world_space_vertex_positions[v2_index];
                world_positions[2] = world_space_vertex_positions[v3_index];

                if (mesh_has_normals) {
                    normal_indices = mesh->vertex_normal_indices[face_index];
                    for (u8 i = 0; i < 3; i++) normals[i] = world_space_vertex_normals[normal_indices.ids[i]];
                }

                if (mesh_has_uvs) {
                    uvs_indices = mesh->vertex_uvs_indices[face_index];
                    for (u8 i = 0; i < 3; i++) uvs[i] = mesh->vertex_uvs[uvs_indices.ids[i]];
                }

                if ( viewport.frustum.cull_back_faces) {
                    // Check face orientation "early" (before the perspective divide)
                    // Compute a normal vector of the face from these 2 direction vectors:
                    pos1 = Vec3(positions[0]);
                    pos2 = Vec3(positions[1]);
                    pos3 = Vec3(positions[2]);
                    normal = (pos3 - pos1).cross(pos2 - pos1);

                    // Dot the vector from the face to the origin with the normal:
                    dot = normal.z*(pz - pos1.z) - normal.y*pos1.y - normal.x*pos1.x;
                    if (dot < 0.0001f) {
                        // if the angle is 90 the face is at grazing angle to the camera.
                        // if the angle is greater than 90 degrees the face faces away from the camera.
                        continue;
                    }
                }

                clipping_produced_an_extra_face = false;
                if (needs_clipping) {
                    if ((v1_flags | v2_flags) | v3_flags) {
                        // One or more vertices are outside - check edges for intersections:
                        if ((v1_flags & v2_flags) & v3_flags) {
                            // All vertices share one or more out-direction(s).
                            // The face is fully outside the frustum, and does not intersect it.
                            continue;
                            // Note: This includes the cases where "all" 3 vertices cross the near clipping plane.
                            // Below there are checks for when "any" of the vertices cross it (1 or 2, but not 3).
                        }

                        // One or more vertices are outside, and no out-direction is shared across them.
                        // The face is visible in the view frustum in some way.
                        vertex_flags[v1_index] |= IS_NDC;
                        vertex_flags[v2_index] |= IS_NDC;
                        vertex_flags[v3_index] |= IS_NDC;

                        // Check if any vertex crosses the near clipping plane:
                        if (v1_flags & IS_NEAR ||
                            v2_flags & IS_NEAR ||
                            v3_flags & IS_NEAR) {
                            // There is at least one vertex behind the near clipping plane.
                            // The face needs to be clipped
                            // Clipping is done only against the near clipping plane, so there are only 2 possible cases:
                            // 1: One vertex is inside the frustum and the other two are outside beyond the near clipping plane.
                            // 2: Two vertices are inside the frustum and the third is outside beyond the near clipping plane.

                            // Figure out which case applies to this current face, and which vertices are in/out:
                            in2_num   = out2_num = 0;
                            in2_index = out2_index = -1;
                            if (v1_flags & IS_NEAR) {
                                out1_index = v1_index;
                                out1_num = 1;
                                if (v2_flags & IS_NEAR) {
                                    out2_index = v2_index;
                                    out2_num = 2;
                                    in1_index = v3_index;
                                    in1_num = 3;
                                } else {
                                    in1_index = v2_index;
                                    in1_num = 2;
                                    if (v3_flags & IS_NEAR) {
                                        out2_index = v3_index;
                                        out2_num = 3;
                                    } else {
                                        in2_index = v3_index;
                                        in2_num = 3;
                                    }
                                }
                            } else {
                                in1_index = v1_index;
                                in1_num = 1;
                                if (v2_flags & IS_NEAR) {
                                    out1_index = v2_index;
                                    out1_num = 2;
                                    if (v3_flags & IS_NEAR) {
                                        out2_index = v3_index;
                                        out2_num = 3;
                                    } else {
                                        in2_index = v3_index;
                                        in2_num = 3;
                                    }
                                } else {
                                    in2_index = v2_index;
                                    in2_num = 2;
                                    out1_index = v3_index;
                                    out1_num = 3;
                                }
                            }
                            in1  = clip_space_vertex_positions[in1_index];
                            out1 = clip_space_vertex_positions[out1_index];

                            // Compute and store the (relative)amount by which the FIRST outside
                            // vertex would need to be moved 'inwards' towards the FIRST inside vertex:
                            t = out1.z / (out1.z - in1.z);
                            one_minus_t = 1 - t;
                            // Note:
                            // Clip space is set up such that a depth of 0 is where the near clipping plane is.
                            // So 'out1z' would be (negative)distance of the vertex from the near clipping plane,
                            // representing the 'amount' by which the 'outside' vertex would needs to be 'pushed' forward
                            // to land it on the near clipping plane. The denominator here is the 'full' depth-distance
                            // between the 2 vertices - the sum of distances of the 2 vertices from to the clipping plane.
                            // The ratio of the former from the latter is thus the (positive)interpolation amount 't' (0->1).
                            // Since 'out1z' would be negative here, 'in1z' is negated as well to get the full (negative)sum.
                            // Since both the numerator and the denominator here would be negative, the result is positive.
                            // The interpolation value 't' is with respect to the 'outside' vertex, and so would be multiplied
                            // by any attribute-value of the 'outside' vertex. The complement of that (1 - t) would be multiplied
                            // by any attribute-value of the 'inside' vertex, and the sum would be the interpolated value.
                            // *The same logic applies for the second interpolation in either of it's 2 cases below.

                            // Compute the index of the "unshared" position-value(s) of the 'clipped' vertex of this face:
                            clipped_index = out1_num - 1;
                            clipped = positions + clipped_index;

                            // Compute the new clip-space coordinates of the clipped-vertex:
                            new_v1.z = fast_mul_add(out1.z, one_minus_t, t*in1.z);
                            clipped->x = new_v1.x = fast_mul_add(out1.x, one_minus_t, t*in1.x);
                            clipped->y = new_v1.y = fast_mul_add(out1.y, one_minus_t, t*in1.y);
                            clipped->z = 0;
                            clipped->w = n;
                            // Note:
                            // The 'Z' coordinate of this new vertex position in clip-space is set to '0' since it has
                            // now been moved onto the clipping plane itself. Similarly, the 'W' coordinate is set to
                            // what the "original" Z-depth value this vertex "would have had", had it been on the
                            // near clipping plane in view-space in the first place.
                            // *The same logic applies for the second interpolation in either of it's 2 cases below.

                            attr_in  = world_space_vertex_positions[position_indices.ids[in1_num - 1]];
                            attr_out = world_space_vertex_positions[position_indices.ids[out1_num - 1]];
                            world_positions[clipped_index] = attr_out.scaleAdd(one_minus_t, attr_in * t);

                            if (mesh_has_normals) {
                                attr_in  = world_space_vertex_normals[normal_indices.ids[in1_num - 1]];
                                attr_out = world_space_vertex_normals[normal_indices.ids[out1_num - 1]];
                                normals[clipped_index] = attr_out.scaleAdd(one_minus_t, attr_in * t).normalized();
                            }

                            if (mesh_has_uvs) {
                                uv_in  = mesh->vertex_uvs[uvs_indices.ids[in1_num - 1]];
                                uv_out = mesh->vertex_uvs[uvs_indices.ids[out1_num - 1]];
                                uvs[clipped_index] = uv_out.scaleAdd(one_minus_t, uv_in * t);
                            }

                            if (out2_num) {
                                // One vertex is inside the frustum, and the other two are outside beyond the near clipping plane.
                                // The triangle just needs to get smaller by moving the 2 outside-vertices back to the near clipping plane.

                                // Compute and store the (relative)amount by which the SECOND outside
                                // vertex needs to be moved inwards towards the FIRST inside vertex:
                                out2 = clip_space_vertex_positions[out2_index];
                                t = out2.z / (out2.z - in1.z);
                                one_minus_t = 1 - t;

                                // Compute the index of the "unshared" position-value(s) of the 'clipped' vertex of this face:
                                clipped_index = out2_num - 1;
                                clipped = positions + clipped_index;

                                // Compute the new clip-space coordinates of the clipped-vertex:
                                clipped->x = fast_mul_add(out2.x, one_minus_t, t*in1.x);
                                clipped->y = fast_mul_add(out2.y, one_minus_t, t*in1.y);
                                clipped->z = 0;
                                clipped->w = n;

                                attr_in  = world_space_vertex_positions[position_indices.ids[in1_num - 1]];
                                attr_out = world_space_vertex_positions[position_indices.ids[out2_num - 1]];
                                world_positions[clipped_index] = attr_out.scaleAdd(one_minus_t, attr_in * t);

                                if (mesh_has_normals) {
                                    attr_in  = world_space_vertex_normals[normal_indices.ids[in1_num - 1]];
                                    attr_out = world_space_vertex_normals[normal_indices.ids[out2_num - 1]];
                                    normals[clipped_index] = attr_out.scaleAdd(one_minus_t, attr_in * t).normalized();
                                }

                                if (mesh_has_uvs) {
                                    uv_in  = mesh->vertex_uvs[uvs_indices.ids[in1_num - 1]];
                                    uv_out = mesh->vertex_uvs[uvs_indices.ids[out2_num - 1]];
                                    uvs[clipped_index] = uv_out.scaleAdd(one_minus_t, uv_in * t);
                                }
                            } else {
                                // Two vertices are inside the frustum, and the third one is behind the near clipping plane.
                                // Clipping forms a quad which needs to be split into 2 triangles.
                                // The first one is formed from the original one, by moving the vertex that is behind the
                                // clipping plane right up-to the near clipping plane itself (exactly as in the first case).
                                // The second triangle is a new triangle that needs to be created, from the 2 vertices that
                                // are inside, plus a new vertex that would need to be interpolated by moving the same vertex
                                // that is outside up-to the near clipping plane but towards the other vertex that is inside.

                                // Compute and store the (relative)amount by which the FIRST outside vertex
                                // needs to be moved inwards towards the SECOND inside vertex:
                                in2 = clip_space_vertex_positions[in2_index];
                                t = out1.z / (out1.z - in2.z);
                                one_minus_t = 1 - t;

                                new_v2 = Vec3(out1).scaleAdd(one_minus_t, Vec3(in2) * t);

                                // Determine orientation:
                                // Compute 2 direction vectors forming a plane for the face:
                                // Compute a normal vector of the face from these 2 direction vectors:
                                normal = (new_v1 - Vec3(in2)).cross(new_v2 - Vec3(in2));

                                // Dot the vector from the face to the origin with the normal:
                                dot = normal.z*(pz - in2.z) - normal.y*in2.y - normal.x*in2.x;
                                if (dot > 0) {
                                    // if the angle is greater than 90 degrees the face is facing the camera
                                    new_v1num = 2;
                                    new_v2num = 1;
                                } else {
                                    // if the angle is 90 the face is at grazing angle to the camera.
                                    // if the angle is greater than 90 degrees the face faces away from the camera.
                                    new_v1num = 1;
                                    new_v2num = 2;
                                }

                                // Since this vertex belongs to an 'extra' new face, the index is offset to that index-space
                                clipped_index = 3 + new_v2num;
                                positions[3] = in2;
                                positions[3 + new_v1num] = *clipped;
                                clipped = positions + clipped_index;

                                // Compute the new clip-space coordinates of the clipped-vertex:
                                clipped->x = new_v2.x;
                                clipped->y = new_v2.y;
                                clipped->z = 0;
                                clipped->w = n;

                                attr_in  = world_space_vertex_positions[position_indices.ids[in2_num - 1]];
                                attr_out = world_space_vertex_positions[position_indices.ids[out1_num - 1]];
                                world_positions[clipped_index] = attr_out.scaleAdd(one_minus_t, attr_in * t);
                                world_positions[3] = attr_in;
                                world_positions[3 + new_v1num] = world_positions[out1_num - 1];

                                if (mesh_has_normals) {
                                    attr_in  = world_space_vertex_normals[normal_indices.ids[in2_num - 1]];
                                    attr_out = world_space_vertex_normals[normal_indices.ids[out1_num - 1]];
                                    normals[clipped_index] = attr_out.scaleAdd(one_minus_t, attr_in * t).normalized();
                                    normals[3] = attr_in;
                                    normals[3 + new_v1num] = normals[out1_num - 1];
                                }

                                if (mesh_has_uvs) {
                                    uv_in  = mesh->vertex_uvs[uvs_indices.ids[in2_num - 1]];
                                    uv_out = mesh->vertex_uvs[uvs_indices.ids[out1_num - 1]];
                                    uvs[clipped_index] = uv_out.scaleAdd(one_minus_t, uv_in * t);
                                    uvs[3] = uv_in;
                                    uvs[3 + new_v1num] = uvs[out1_num - 1];
                                }

                                clipping_produced_an_extra_face = true;
                            }
                        }
                        // Even if no vertices are behind the view frustum the face is still visible.
                        // It either intersects the frustum in direction(s) other than the near clipping plane,
                        // or it may fully surround the whole view frustum.
                        // No geometric clipping is needed, but the face can not be culled.
                    } // else: No vertices are outside the frustum (the face is fully within it).
                } // else: No vertex is behind the near clipping plane.
                // Since geometric clipping is done only against the near clipping plane, there is nothing to clip.
                // The other sides of the frustum would get raster-clipped later by clamping pixels outside the screen.

                vertex_count = clipping_produced_an_extra_face ? 6 : 3;
                position = positions;
                for (vertex_index = 0; vertex_index < vertex_count; vertex_index++, position++) {
                    // The perspective divide should finalize normalizing the depth values
                    // into the 0 -> 1 space, by dividing clip-space 'z' by clip-space 'w'
                    // (coordinates that came out of the multiplication by the projection matrix)
                    // screen_z = clip_z / clip_w = Z[i] / W[i]
                    // However: The rasterizer is going to then need to convert this value
                    // into a spectrum of values that are all divided by 'clip_w'
                    // in order to linearly interpolate it there (from vertex values to a pixel value).

                    // Store reciprocals for use in rasterization:
                    position->w = 1.0f / position->w;
                    position->x *= position->w;
                    position->y *= position->w;
                    position->z *= position->w;
                    // Scale the normalized screen to the pixel size:
                    // (from normalized size of -1->1 horizontally and vertically having a width and height of 2)
                    position->x *= screen_transform.x;
                    position->y *= -screen_transform.y;

                    // Move the screen up and to the right appropriately,
                    // such that it goes 0->width horizontally and 0->height vertically:
                    position->x += screen_transform.x;
                    position->y += screen_transform.y;

                    // Scale the normalized screen to the pixel size:
                    // (from normalized size of -1->1 horizontally and vertically having a width and height of 2)
                    // Then, move the screen up and to the right appropriately,
                    // such that it goes 0->width horizontally and 0->height vertically:
//                position->x = fast_mul_add(position->x, screen_transform.x, screen_transform.x);
//                position->x = fast_mul_add(position->y, -screen_transform.y, screen_transform.y);
                }

                vertex_count = clipping_produced_an_extra_face ? 2 : 1;
                v1_index = 0;
                v2_index = 1;
                v3_index = 2;

                for (vertex_index = 0; vertex_index < vertex_count; vertex_index++, v1_index += 3, v2_index += 3, v3_index += 3) {
                    if (pixel_shader) {
                        v1 = positions[v1_index];
                        v2 = positions[v2_index];
                        v3 = positions[v3_index];

                        A = {v1.x, v1.y};
                        B = {v2.x, v2.y};
                        C = {v3.x, v3.y};
                        w = {v1.w, v2.w, v3.w};

                        // Cull this triangle against the edges of the viewport:
                        pixel_min = minimum(minimum(A, B), C);
                        if (pixel_min.x >= width ||
                            pixel_min.y >= height)
                            continue;

                        pixel_max = maximum(maximum(A, B), C);
                        if (pixel_max.x < 0 ||
                            pixel_max.y < 0)
                            continue;

                        // Clip the bounds of the triangle to the viewport:
                        if (pixel_min.x < 0) pixel_min.x = 0;
                        if (pixel_min.y < 0) pixel_min.y = 0;
                        if (pixel_max.x > last_pixel_coord.x) pixel_max.x = last_pixel_coord.x;
                        if (pixel_max.y > last_pixel_coord.y) pixel_max.y = last_pixel_coord.y;

                        // Compute area components:
                        AB = B - A;
                        AC = C - A;

                        ABC = AC.x*AB.y - AC.y*AB.x;

                        // Cull faces facing backwards:
                        if (ABC <= 0)
                            continue;

                        // Floor bounds coordinates down to their integral component:
                        first_x = (u32)pixel_min.x;
                        first_y = (u32)pixel_min.y;
                        last_x  = (u32)pixel_max.x;
                        last_y  = (u32)pixel_max.y;

                        pixel_min.x = (f32)first_x;
                        pixel_min.y = (f32)first_y;

                        // Compute weight constants:
                        BC = C - B;
                        dX = {BC.y, -AC.y, AB.y};
                        dY = {-BC.x, AC.x, -AB.x};
                        K = {B.y * C.x - B.x * C.y,
                             C.y * A.x - C.x * A.y,
                             A.y * B.x - A.x * B.y};

                        one_over_ABC = 1.0f / ABC;
                        dX *= one_over_ABC;
                        dY *= one_over_ABC;
                        K *= one_over_ABC;

                        bary = ((f32)first_x + 0.5f) * dX +
                               ((f32)first_y + 0.5f) * dY +
                               K;

                        // Compute edge exclusions:
                        // Drawing: Top-down
                        // Origin: Top-left
                        // Shadow rules: Top/Left
                        // Winding: CW (Flipped vertically due to top-down drawing!)
//                        if (BC.y < 0 || (BC.y == 0 && BC.x > 0)) bary.x -= 0.0001f;
//                        if (-AC.y < 0 || (-AC.y == 0 && -AC.x > 0)) bary.y -= 0.0001f;
//                        if (AB.y < 0 || (AB.y == 0 && AB.x > 0)) bary.z -= 0.0001f;

                        pos1 = world_positions[v1_index];
                        pos2 = world_positions[v2_index];
                        pos3 = world_positions[v3_index];

                        norm1 = normals[v1_index];
                        norm2 = normals[v2_index];
                        norm3 = normals[v3_index];

                        uv1 = uvs[v1_index];
                        uv2 = uvs[v2_index];
                        uv3 = uvs[v3_index];

                        // Scan the bounds:
                        bary_start = bary;
                        for (u32 y = first_y; y <= last_y; y++, bary_start += dY) {
                            last_UV_taken = false;
                            bary = bary_start;
                            for (u32 x = first_x; x <= last_x; x++, bary += dX) {
                                if (dX.x < 0 && bary.x < 0 ||
                                    dX.y < 0 && bary.y < 0 ||
                                    dX.z < 0 && bary.z < 0)
                                    break;

                                if (bary.x < 0 ||
                                    bary.y < 0 ||
                                    bary.z < 0)
                                    continue;

                                // Cull and test pixel based on its depth:
                                pixel_offset = viewport.canvas.antialias ? (
                                        (viewport.dimensions.stride * (y >> 1)) + (x >> 1) + (y & 1) * 2 + (x & 1)
                                        ) : (
                                        (viewport.dimensions.stride * y) + x
                                                );
                                depth = viewport.canvas.depths + pixel_offset;
                                pixel = viewport.canvas.pixels + pixel_offset;

                                ABCw = bary * w;
                                shaded.depth = (f64)ABCw.x + (f64)ABCw.y + (f64)ABCw.z;
                                if (shaded.depth < 0) continue;

                                shaded.depth = 1.0 / shaded.depth;
                                pixel_depth = (f32)shaded.depth;
                                if (pixel_depth > *depth)
                                    continue;

                                ABCp = ABCw * pixel_depth;

                                shaded.position = pos1.scaleAdd(ABCp.x, pos2.scaleAdd(ABCp.y, pos3 * ABCp.z));
                                if (mesh_has_normals) shaded.normal = norm1.scaleAdd(ABCp.x, norm2.scaleAdd(ABCp.y, norm3 * ABCp.z)).normalized();
                                if (mesh_has_uvs) {
                                    shaded.u = fast_mul_add(uv1.u, ABCp.x, (fast_mul_add(uv2.u, ABCp.y, uv3.u * ABCp.z)));
                                    shaded.v = fast_mul_add(uv1.v, ABCp.x, (fast_mul_add(uv2.v, ABCp.y, uv3.v * ABCp.z)));

                                    if (last_UV_taken) {
                                        du = shaded.u - lastUV.u;
                                        dv = shaded.v - lastUV.v;
                                    } else {
                                        ABCp = bary + dX;
                                        if (ABCp.x < 0)
                                            ABCp = bary - dX;

                                        ABCp *= w;
                                        ABCp /= ABCp.x + ABCp.y + ABCp.z;
                                        du = fast_mul_add(uv1.u, ABCp.x, (fast_mul_add(uv2.u, ABCp.y, uv3.u * ABCp.z))) - shaded.u;
                                        dv = fast_mul_add(uv1.v, ABCp.x, (fast_mul_add(uv2.v, ABCp.y, uv3.v * ABCp.z))) - shaded.v;
                                        last_UV_taken = true;
                                    }

                                    if (du < 0) du = -du;
                                    if (dv < 0) dv = -dv;

                                    shaded.uv_area = du*dv;

                                    lastUV.u = shaded.u;
                                    lastUV.v = shaded.v;
                                }
                                shaded.coords.x = x;
                                shaded.coords.y = y;
                                shaded.color = 0.0f;
                                shaded.opacity = 1;
                                pixel_shader(shaded, scene);
                                viewport.canvas.setPixel((i32)x, (i32)y, shaded.color, shaded.opacity, pixel_depth);
                            }
                        }
                    }
                    if (draw_wireframe || !pixel_shader) {
                        Color color{vertex_index ? Red : White};
                        new_v1 = Vec3(positions[v1_index]);
                        new_v2 = Vec3(positions[v2_index]);
                        if (viewport.canvas.antialias) {
                            new_v1.x *= 0.5f;
                            new_v1.y *= 0.5f;
                            new_v2.x *= 0.5f;
                            new_v2.y *= 0.5f;
                        }
                        new_v1.z -= 0.001f;
                        new_v2.z -= 0.001f;
                        drawLine(new_v1.x, new_v1.y, new_v1.z,
                                 new_v2.x, new_v2.y, new_v2.z,
                                 viewport.canvas, color, 1, 0);
                        new_v1 = Vec3(positions[v2_index]);
                        new_v2 = Vec3(positions[v3_index]);
                        if (viewport.canvas.antialias) {
                            new_v1.x *= 0.5f;
                            new_v1.y *= 0.5f;
                            new_v2.x *= 0.5f;
                            new_v2.y *= 0.5f;
                        }
                        new_v1.z -= 0.001f;
                        new_v2.z -= 0.001f;
                        drawLine(new_v1.x, new_v1.y, new_v1.z,
                                 new_v2.x, new_v2.y, new_v2.z,
                                 viewport.canvas, color, 1, 0);
                        new_v1 = Vec3(positions[v3_index]);
                        new_v2 = Vec3(positions[v1_index]);
                        if ( viewport.canvas.antialias) {
                            new_v1.x *= 0.5f;
                            new_v1.y *= 0.5f;
                            new_v2.x *= 0.5f;
                            new_v2.y *= 0.5f;
                        }
                        new_v1.z -= 0.001f;
                        new_v2.z -= 0.001f;
                        drawLine(new_v1.x, new_v1.y, new_v1.z,
                                 new_v2.x, new_v2.y, new_v2.z,
                                 viewport.canvas, color, 1, 0);
                    }
                }
            }
        }
    }
};
CubeMesh Rasterizer::cube;