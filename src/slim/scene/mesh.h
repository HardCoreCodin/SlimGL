#pragma once

#include "../math/vec2.h"
#include "../math/mat3.h"

#include "./bvh.h"

struct EdgeVertexIndices {
    u32 from, to;
};

union TriangleVertexIndices {
    u32 ids[3];
    struct {
        u32 v1, v2, v3;
    };
};

struct Triangle {
    mat3 local_to_tangent;
    vec3 position, normal, U, V;
};


struct Mesh {
    AABB aabb;
    BVH bvh;
    Triangle *triangles;

    vec3 *vertex_positions{nullptr};
    vec3 *vertex_normals{nullptr};
    vec2 *vertex_uvs{nullptr};

    TriangleVertexIndices *vertex_position_indices{nullptr};
    TriangleVertexIndices *vertex_normal_indices{nullptr};
    TriangleVertexIndices *vertex_uvs_indices{nullptr};

    EdgeVertexIndices *edge_vertex_indices{nullptr};

    u32 triangle_count{0};
    u32 vertex_count{0};
    u32 edge_count{0};
    u32 normals_count{0};
    u32 uvs_count{0};

    Mesh() = default;

    Mesh(u32 triangle_count,
         u32 vertex_count,
         u32 normals_count,
         u32 uvs_count,
         u32 edge_count,

         vec3 *vertex_positions,
         vec3 *vertex_normals,
         vec2 *vertex_uvs,

         TriangleVertexIndices *vertex_position_indices,
         TriangleVertexIndices *vertex_normal_indices,
         TriangleVertexIndices *vertex_uvs_indices,

         EdgeVertexIndices *edge_vertex_indices,
         AABB aabb
    ) :
            triangle_count{triangle_count},
            vertex_count{vertex_count},
            normals_count{normals_count},
            uvs_count{uvs_count},
            edge_count{edge_count},

            vertex_positions{vertex_positions},
            vertex_normals{vertex_normals},
            vertex_uvs{vertex_uvs},

            vertex_position_indices{vertex_position_indices},
            vertex_normal_indices{vertex_normal_indices},
            vertex_uvs_indices{vertex_uvs_indices},

            edge_vertex_indices{edge_vertex_indices},
            aabb{aabb}
    {}
};


struct CubeMesh : Mesh {
    const vec3 CUBE_VERTEX_POSITIONS[CUBE_VERTEX_COUNT] = {
            {-1, -1, -1},
            {1, -1, -1},
            {1, 1, -1},
            {-1, 1, -1},
            {-1, -1, 1},
            {1, -1, 1},
            {1, 1, 1},
            {-1, 1, 1}
    };

    const TriangleVertexIndices CUBE_VERTEX_POSITION_INDICES[CUBE_TRIANGLE_COUNT] = {
            {0, 1, 2},
            {1, 5, 6},
            {5, 4, 7},
            {4, 0, 3},
            {3, 2, 6},
            {1, 0, 4},
            {0, 2, 3},
            {1, 6, 2},
            {5, 7, 6},
            {4, 3, 7},
            {3, 6, 7},
            {1, 4, 5}
    };

    const vec3 CUBE_VERTEX_NORMALS[CUBE_NORMAL_COUNT] = {
            {0, 0, -1},
            {1, 0, 0},
            {0, 0, 1},
            {-1, 0, 0},
            {0, 1, 0},
            {0, -1, 0}
    };

    const TriangleVertexIndices CUBE_VERTEX_NORMAL_INDICES[CUBE_TRIANGLE_COUNT] = {
            {0, 0, 0},
            {1, 1, 1},
            {2, 2, 2},
            {3, 3, 3},
            {4, 4, 4},
            {5, 5, 5},
            {0, 0, 0},
            {1, 1, 1},
            {2, 2, 2},
            {3, 3, 3},
            {4, 4, 4},
            {5, 5, 5}
    };

    const vec2 CUBE_VERTEX_UVS[CUBE_UV_COUNT] = {
            {0.0f, 0.0f},
            {0.0f, 1.0f},
            {1.0f, 1.0f},
            {1.0f, 0.0f},
    };

    const TriangleVertexIndices CUBE_VERTEX_UV_INDICES[CUBE_TRIANGLE_COUNT] = {
            {0, 1, 2},
            {0, 1, 2},
            {0, 1, 2},
            {0, 1, 2},
            {0, 1, 2},
            {0, 1, 2},
            {0, 2, 3},
            {0, 2, 3},
            {0, 2, 3},
            {0, 2, 3},
            {0, 2, 3},
            {0, 2, 3}
    };

    CubeMesh() : Mesh{
            CUBE_TRIANGLE_COUNT,
            CUBE_VERTEX_COUNT,
            CUBE_NORMAL_COUNT,
            CUBE_UV_COUNT,
            0,

            (vec3*)CUBE_VERTEX_POSITIONS,
            (vec3*)CUBE_VERTEX_NORMALS,
            (vec2*)CUBE_VERTEX_UVS,

            (TriangleVertexIndices*)CUBE_VERTEX_POSITION_INDICES,
            (TriangleVertexIndices*)CUBE_VERTEX_NORMAL_INDICES,
            (TriangleVertexIndices*)CUBE_VERTEX_UV_INDICES,
            nullptr,

            {-1 , +1}
    } {}
};

struct GridMesh : Mesh {
    static u32 getGridMemorySize(u32 u_segments, u32 v_segments) {
        u32 U = u_segments + 1;
        u32 V = v_segments + 1;
        u32 uv_segments = u_segments * v_segments;

        u32 vertices = U * V;
        u32 edges = uv_segments * 3;
        u32 triangles = uv_segments * 2;

        return (
                (sizeof(vec2) + sizeof(vec3) * 2) * vertices +
                sizeof(EdgeVertexIndices) * edges +
                sizeof(TriangleVertexIndices) * triangles * 3
                );
    }

    GridMesh(u32 u_segments, u32 v_segments, memory::MonotonicAllocator *memory_allocator = nullptr) {
        u32 U = u_segments + 1;
        u32 V = v_segments + 1;
        u32 uv_segments = u_segments * v_segments;

        uvs_count = normals_count = vertex_count = U * V;
        edge_count = uv_segments * 3;
        triangle_count = uv_segments * 2;

        memory::MonotonicAllocator temp_allocator;
        if (!memory_allocator) {
            temp_allocator = memory::MonotonicAllocator{getGridMemorySize(u_segments, v_segments), Terabytes(4)};
            memory_allocator = &temp_allocator;
        }

        vertex_positions = (vec3*)memory_allocator->allocate(sizeof(vec3) * vertex_count);
        vertex_normals   = (vec3*)memory_allocator->allocate(sizeof(vec3) * normals_count);
        vertex_uvs       = (vec2*)memory_allocator->allocate(sizeof(vec2) * uvs_count);

        vertex_position_indices = (TriangleVertexIndices*)memory_allocator->allocate(sizeof(TriangleVertexIndices) * triangle_count);
        vertex_normal_indices   = (TriangleVertexIndices*)memory_allocator->allocate(sizeof(TriangleVertexIndices) * triangle_count);
        vertex_uvs_indices      = (TriangleVertexIndices*)memory_allocator->allocate(sizeof(TriangleVertexIndices) * triangle_count);

        edge_vertex_indices = (EdgeVertexIndices*)memory_allocator->allocate(sizeof(EdgeVertexIndices) * edge_count);

        f32 u_step = 1.0f / (f32)u_segments;
        f32 v_step = 1.0f / (f32)v_segments;

        u32 triangle_index = 0;
        u32 edge_index = 0;
        u32 vertex_index = 0;
        vec2 uv_start;
        vec2 *uv = vertex_uvs;
        vec3 *pos = vertex_positions;
        vec3 *n = vertex_normals;

        for (u32 v = 0; v < V; v++) {
            uv_start.x = 0.0f;
            for (u32 u = 0; u < U; u++, uv++, n++, pos++, vertex_index++) {
                *uv = uv_start;
                *pos = vec3{uv_start.x, 0.0f, uv_start.y};
                *n = {0, 1, 0};

                if (u != u_segments &&
                    v != v_segments) {
                    edge_vertex_indices[edge_index++] = {vertex_index, vertex_index + 1};
                    edge_vertex_indices[edge_index++] = {vertex_index, vertex_index + U};
                    edge_vertex_indices[edge_index++] = {vertex_index, vertex_index + U + 1};

                    vertex_position_indices[triangle_index].v1 = vertex_index;
                    vertex_position_indices[triangle_index].v2 = vertex_index + U + 1;
                    vertex_position_indices[triangle_index].v3 = vertex_index + U;
                    vertex_normal_indices[triangle_index] = vertex_uvs_indices[triangle_index] = vertex_position_indices[triangle_index];
                    triangle_index++;

                    vertex_position_indices[triangle_index].v1 = vertex_index;
                    vertex_position_indices[triangle_index].v2 = vertex_index + 1;
                    vertex_position_indices[triangle_index].v3 = vertex_index + U + 1;
                    vertex_normal_indices[triangle_index] = vertex_uvs_indices[triangle_index] = vertex_position_indices[triangle_index];
                    triangle_index++;
                }

                uv_start.x += u_step;
            }
            uv_start.y += v_step;
        }
    }
};