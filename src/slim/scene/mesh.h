#pragma once
#include <stdio.h>
#include "../math/vec2.h"
#include "../math/mat3.h"

#include "./bvh.h"


struct Triangle {
    mat3 local_to_tangent;
    vec3 position, normal, n1, n2, n3;
    vec2 uv1, uv2, uv3;
    f32 uv_coverage, padding;
};

Triangle CUBE_TRIANGLES[] = { // Triangles:
    {
        {0.000000f, -0.000000f, 1.000000f, 0.500000f, -0.500000f, -0.000000f, 0.000000f, 0.500000f, 0.000000f},
        { 1.000000f, -1.000000f, -1.000000f},
        { 1.000000f, 0.000000f, 0.000000f},
        { 1.000000f, 0.000000f, 0.000000f},
        { 1.000000f, 0.000000f, 0.000000f},
        { 1.000000f, 0.000000f, 0.000000f},
        { 0.000000f, 0.000000f},
        { 0.000000f, 1.000000f},
        { 1.000000f, 1.000000f},
        0.250000f
    },
    {
        {0.000000f, -0.000000f, 1.000000f, 0.500000f, 0.000000f, -0.000000f, -0.500000f, 0.500000f, 0.000000f},
        { 1.000000f, -1.000000f, -1.000000f},
        { 1.000000f, 0.000000f, 0.000000f},
        { 1.000000f, 0.000000f, 0.000000f},
        { 1.000000f, 0.000000f, 0.000000f},
        { 1.000000f, 0.000000f, 0.000000f},
        { 0.000000f, 0.000000f},
        { 1.000000f, 1.000000f},
        { 1.000000f, 0.000000f},
        0.250000f
    },
    {
        {0.500000f, -0.500000f, 0.000000f, 0.500000f, 0.000000f, -0.000000f, 0.000000f, 0.000000f, 1.000000f},
        { 1.000000f, -1.000000f, 1.000000f},
        { 0.000000f, -0.000000f, 1.000000f},
        { 0.000000f, 0.000000f, 1.000000f},
        { 0.000000f, 0.000000f, 1.000000f},
        { 0.000000f, 0.000000f, 1.000000f},
        { 0.000000f, 0.000000f},
        { 1.000000f, 1.000000f},
        { 1.000000f, 0.000000f},
        0.250000f
    },
    {
        {0.000000f, -0.500000f, 0.000000f, 0.500000f, -0.500000f, -0.000000f, -0.000000f, 0.000000f, 1.000000f},
        { 1.000000f, -1.000000f, 1.000000f},
        { 0.000000f, 0.000000f, 1.000000f},
        { 0.000000f, 0.000000f, 1.000000f},
        { 0.000000f, 0.000000f, 1.000000f},
        { 0.000000f, 0.000000f, 1.000000f},
        { 0.000000f, 0.000000f},
        { 0.000000f, 1.000000f},
        { 1.000000f, 1.000000f},
        0.250000f
    },
    {
        {0.000000f, 0.500000f, 0.000000f, -0.000000f, 0.000000f, 1.000000f, 0.500000f, -0.500000f, 0.000000f},
        { -1.000000f, 1.000000f, -1.000000f},
        { 0.000000f, 1.000000f, 0.000000f},
        { 0.000000f, 1.000000f, 0.000000f},
        { 0.000000f, 1.000000f, 0.000000f},
        { 0.000000f, 1.000000f, 0.000000f},
        { 0.000000f, 0.000000f},
        { 0.000000f, 1.000000f},
        { 1.000000f, 1.000000f},
        0.250000f
    },
    {
        {-0.500000f, 0.500000f, 0.000000f, -0.000000f, 0.000000f, 1.000000f, 0.500000f, -0.000000f, 0.000000f},
        { -1.000000f, 1.000000f, -1.000000f},
        { 0.000000f, 1.000000f, 0.000000f},
        { 0.000000f, 1.000000f, 0.000000f},
        { 0.000000f, 1.000000f, 0.000000f},
        { 0.000000f, 1.000000f, 0.000000f},
        { 0.000000f, 0.000000f},
        { 1.000000f, 1.000000f},
        { 1.000000f, 0.000000f},
        0.250000f
    },
    {
        {0.500000f, -0.500000f, 0.000000f, 0.000000f, 0.000000f, -1.000000f, 0.500000f, 0.000000f, 0.000000f},
        { 1.000000f, -1.000000f, -1.000000f},
        { 0.000000f, -1.000000f, 0.000000f},
        { 0.000000f, -1.000000f, 0.000000f},
        { 0.000000f, -1.000000f, 0.000000f},
        { 0.000000f, -1.000000f, 0.000000f},
        { 0.000000f, 0.000000f},
        { 1.000000f, 1.000000f},
        { 1.000000f, 0.000000f},
        0.250000f
    },
    {
        {0.000000f, -0.500000f, 0.000000f, 0.000000f, -0.000000f, -1.000000f, 0.500000f, -0.500000f, 0.000000f},
        { 1.000000f, -1.000000f, -1.000000f},
        { 0.000000f, -1.000000f, 0.000000f},
        { 0.000000f, -1.000000f, 0.000000f},
        { 0.000000f, -1.000000f, 0.000000f},
        { 0.000000f, -1.000000f, 0.000000f},
        { 0.000000f, 0.000000f},
        { 0.000000f, 1.000000f},
        { 1.000000f, 1.000000f},
        0.250000f
    },
    {
        {-0.500000f, 0.500000f, 0.000000f, 0.500000f, -0.000000f, -0.000000f, 0.000000f, -0.000000f, -1.000000f},
        { -1.000000f, -1.000000f, -1.000000f},
        { 0.000000f, 0.000000f, -1.000000f},
        { 0.000000f, 0.000000f, -1.000000f},
        { 0.000000f, 0.000000f, -1.000000f},
        { 0.000000f, 0.000000f, -1.000000f},
        { 0.000000f, 0.000000f},
        { 1.000000f, 1.000000f},
        { 1.000000f, 0.000000f},
        0.250000f
    },
    {
        {-0.000000f, 0.500000f, 0.000000f, 0.500000f, -0.500000f, -0.000000f, 0.000000f, -0.000000f, -1.000000f},
        { -1.000000f, -1.000000f, -1.000000f},
        { 0.000000f, 0.000000f, -1.000000f},
        { 0.000000f, 0.000000f, -1.000000f},
        { 0.000000f, 0.000000f, -1.000000f},
        { 0.000000f, 0.000000f, -1.000000f},
        { 0.000000f, 0.000000f},
        { 0.000000f, 1.000000f},
        { 1.000000f, 1.000000f},
        0.250000f
    },
    {
        {0.000000f, -0.000000f, -1.000000f, 0.500000f, 0.000000f, 0.000000f, 0.500000f, -0.500000f, 0.000000f},
        { -1.000000f, -1.000000f, 1.000000f},
        { -1.000000f, 0.000000f, 0.000000f},
        { -1.000000f, 0.000000f, 0.000000f},
        { -1.000000f, 0.000000f, 0.000000f},
        { -1.000000f, 0.000000f, 0.000000f},
        { 0.000000f, 0.000000f},
        { 1.000000f, 1.000000f},
        { 1.000000f, 0.000000f},
        0.250000f
    },
    {
        {0.000000f, -0.000000f, -1.000000f, 0.500000f, -0.500000f, -0.000000f, 0.000000f, -0.500000f, 0.000000f},
        { -1.000000f, -1.000000f, 1.000000f},
        { -1.000000f, 0.000000f, 0.000000f},
        { -1.000000f, 0.000000f, 0.000000f},
        { -1.000000f, 0.000000f, 0.000000f},
        { -1.000000f, 0.000000f, 0.000000f},
        { 0.000000f, 0.000000f},
        { 0.000000f, 1.000000f},
        { 1.000000f, 1.000000f},
        0.250000f
    }
};

//memory::MonotonicAllocator temp_allocator{BVHBuilder::getSizeInBytes(CUBE_TRIANGLE_COUNT)};
//        BVHBuilder builder{CUBE_TRIANGLE_COUNT, &temp_allocator};
//        builder.buildMesh(floor_mesh);
//        temp_allocator.releaseMemory();
//        printf("\n{ // Triangles:\n");
//        for (u32 i = 0; i < floor_mesh.triangle_count; i++) {
//            auto& t{floor_mesh.triangles[i]};
//            auto& m{t.local_to_tangent};
//            auto& p{t.position};
//            auto& n{t.normal};
//            auto& n1{t.n1};
//            auto& n2{t.n2};
//            auto& n3{t.n3};
//            auto& u1{t.uv1};
//            auto& u2{t.uv2};
//            auto& u3{t.uv3};
//            auto& c{t.uv_coverage};
//            auto& pd{t.padding};
//            printf("{\n"
//                   "    {%ff, %ff, %ff,\n"
//                   "     %ff, %ff, %ff,\n"
//                   "     %ff, %ff, %ff},\n"
//                   "    { %ff, %ff, %ff},\n"
//                   "    { %ff, %ff, %ff},\n"
//                   "    { %ff, %ff, %ff},\n"
//                   "    { %ff, %ff, %ff},\n"
//                   "    { %ff, %ff, %ff},\n"
//                   "    { %ff, %ff},\n"
//                   "    { %ff, %ff},\n"
//                   "    { %ff, %ff},\n"
//                   "    %ff\n"
//                   "},\n",
//                   m.X.x, m.X.y, m.X.z,
//                   m.Y.x, m.Y.y, m.Y.z,
//                   m.Z.x, m.Z.y, m.Z.z,
//                   p.x, p.y, p.z,
//                   n.x, n.y, n.z,
//                   n1.x, n1.y, n1.z,
//                   n2.x, n2.y, n2.z,
//                   n3.x, n3.y, n3.z,
//                   u1.x, u1.y,
//                   u2.x, u2.y,
//                   u3.x, u3.y,
//                   c);
//        }
//        printf("}\n\n");

BVHNode CUBE_BVH_NODES[] = { // BVHNodes:
    {{{-1.000100f, -1.000100f, -1.000100f}, {1.000100f, 1.000100f, 1.000100f}}, 1, 0, 0, 0},
    {{{-1.000100f, -1.000000f, -1.000000f}, {-0.999900f, 1.000000f, 1.000000f}}, 10, 2, 1, 0},
    {{{-1.000000f, -1.000100f, -1.000100f}, {1.000100f, 1.000100f, 1.000100f}}, 3, 0, 1, 0},
    {{{-1.000000f, -1.000100f, -1.000100f}, {1.000000f, 1.000100f, 1.000100f}}, 5, 0, 2, 0},
    {{{0.999900f, -1.000000f, -1.000000f}, {1.000100f, 1.000000f, 1.000000f}}, 0, 2, 2, 0},
    {{{-1.000000f, -1.000000f, -1.000100f}, {1.000000f, 1.000000f, -0.999900f}}, 8, 2, 3, 0},
    {{{-1.000000f, -1.000100f, -1.000000f}, {1.000000f, 1.000100f, 1.000100f}}, 7, 0, 3, 0},
    {{{-1.000000f, -1.000100f, -1.000000f}, {1.000000f, 1.000100f, 1.000000f}}, 9, 0, 4, 0},
    {{{-1.000000f, -1.000000f, 0.999900f}, {1.000000f, 1.000000f, 1.000100f}}, 2, 2, 4, 0},
    {{{-1.000000f, -1.000100f, -1.000000f}, {1.000000f, -0.999900f, 1.000000f}}, 6, 2, 5, 0},
    {{{-1.000000f, 0.999900f, -1.000000f}, {1.000000f, 1.000100f, 1.000000f}}, 4, 2, 5, 0}
};
//        printf("\n\nnode_count=%lu, height=%u\n", floor_mesh.bvh.node_count, floor_mesh.bvh.height);
//        printf("\n{ // BVHNodes:\n");
//        for (u32 ni = 0; ni < floor_mesh.bvh.node_count; ni++) {
//            auto& n{floor_mesh.bvh.nodes[ni]};
//            printf("   {{{%ff, %ff, %ff}, {%ff, %ff, %ff}}, %lu, %u, %u, %u},\n",
//                   n.aabb.min.x, n.aabb.min.y, n.aabb.min.z,
//                   n.aabb.max.x, n.aabb.max.y, n.aabb.max.z,
//                   n.first_index, n.leaf_count, n.depth, n.flags);
//        }
//        printf("}\n\n");

#define L (-1.0f)
#define R (+1.0f)
#define B (-1.0f)
#define T (+1.0f)
#define K (-1.0f)
#define F (+1.0f)

const vec3 CUBE_VERTEX_POSITIONS[] = {
    {L, B, K},
    {L, B, F},
    {L, T, K},
    {L, T, F},
    {R, B, K},
    {R, B, F},
    {R, T, K},
    {R, T, F}
};

#define LBK (0)
#define LBF (1)
#define LTK (2)
#define LTF (3)
#define RBK (4)
#define RBF (5)
#define RTK (6)
#define RTF (7)

const EdgeVertexIndices CUBE_EDGES[] = {
    {LBK, RBK},
    {RBK, RTK},
    {RTK, LTK},
    {LTK, LBK},

    {RBF, LBF},
    {LBF, LTF},
    {LTF, RTF},
    {RTF, RBF},

    {LBK, LBF},
    {RBK, RBF},
    {LTK, LTF},
    {RTK, RTF},

    {LBK, RTK},
    {RTK, LTF},
    {LTF, LBK},

    {RBK, RTF},
    {RTF, LBF},
    {LBF, RBK}
};

const EdgeVertexIndices CUBE_QUAD_EDGES[] = {
    {LBK, RBK},
    {RBK, RTK},
    {RTK, LTK},
    {LTK, LBK},

    {RBK, RBF},
    {RBF, RTF},
    {RTF, RTK},
    {RTK, RBK},

    {RBF, LBF},
    {LBF, LTF},
    {LTF, RTF},
    {RTF, RBF},

    {LBF, LBK},
    {LBK, LTK},
    {LTK, LTF},
    {LTF, LBF},

    {LTK, RTK},
    {RTK, RTF},
    {RTF, LTF},
    {LTF, LTK},

    {RBK, LBK},
    {LBK, LBF},
    {LBF, RBF},
    {RBF, RBK}
};

const vec3 CUBE_VERTEX_NORMALS[] = {
    {L, 0, 0},
    {R, 0, 0},
    {0, B, 0},
    {0, T, 0},
    {0, 0, K},
    {0, 0, F}
};
const vec3 CUBE_VERTEX_SRC_TANGENTS[] = {
    {L, 0, 0},
    {R, 0, 0},
    {0, B, 0},
    {0, T, 0},
    {0, 0, K},
    {0, 0, F},
    {0, -2, -2},
    {0, +2, +2},
    {0, -2, +2},
    {0, +2, -2},
    {+2, 0, +2},
    {-2, 0, -2},
    {+2, 0, -2},
    {-2, 0, +2},
    {+2, -2, 0},
    {-2, +2, 0},
    {-2, -2, 0},
    {+2, +2, 0},
};
const TriangleVertexIndices CUBE_VERTEX_TANGENT_INDICES[] = {
    {0, 1, 2},
    {3, 4, 5},
    {6, 7, 8},
    {9, 10, 11},
    {12, 13, 14},
    {15, 16, 17},
    {18, 19, 20},
    {21, 22, 23},
    {24, 25, 26},
    {27, 28, 29},
    {30, 31, 32},
    {33, 34, 35},
};
const TriangleVertexIndices CUBE_VERTEX_TANGENT_SRC_INDICES[] = {
    {5, 6, 3},
    {2, 4, 7},

    {4, 8, 3},
    {2, 5, 9},

    {0, 10, 4},
    {5, 1, 11},

    {0, 12, 5},
    {4, 1, 13},

    {0, 14, 3},
    {2, 1, 15},

    {1, 16, 3},
    {2, 0, 5}
};

const TriangleVertexIndices CUBE_VERTEX_POSITION_INDICES[] = {
     {LTK, LTF, LBK},
     {LTF, LBF, LBK},

     {RTF, RTK, RBF},
     {RTK, RBK, RBF},

     {RBK, LBK, RBF},
     {LBK, LBF, RBF},

     {RTF, LTF, RTK},
     {LTF, LTK, RTK},

     {RTK, LTK, RBK},
     {LTK, LBK, RBK},

     {LTF, RTF, LBF},
     {RTF, RBF, LBF},
};

const vec2 CUBE_QUAD_VERTEX_UVS[] = {
    {0.0f, 0.0f}, // 0 LTF RTK LBK LTF LTK RTF
    {1.0f, 0.0f}, // 1 LTK RTF RBK RTF RTK LTF
    {0.0f, 1.0f}, // 2 LBF RBK LBF LTK LBK RBF
    {1.0f, 1.0f}, // 3 LBK RBF RBF RTK RBK LBF
};

const TriangleVertexIndices CUBE_QUAD_VERTEX_UV_INDICES[] = {
    {1, 0, 3},
    {0, 2, 3},

    {1, 0, 3},
    {0, 2, 3},

    {1, 0, 3},
    {0, 2, 3},

    {1, 0, 3},
    {0, 2, 3},

    {1, 0, 3},
    {0, 2, 3},

    {1, 0, 3},
    {0, 2, 3},
};
vec3 CUBE_VERTEX_TANGENTS[36] = { // Tangents:
    {0, 0, -2},
    {0, 0, -2},
    {0, 0, -2},
    {0, 0, -2},
    {0, 0, -2},
    {0, 0, -2},
    {0, 0, 2},
    {0, 0, 2},
    {0, 0, 2},
    {0, 0, 2},
    {0, 0, 2},
    {0, 0, 2},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {2, 0, 0},
    {-2, 0, 0},
    {-2, 0, 0},
    {-2, 0, 0},
    {-2, 0, 0},
    {-2, 0, 0},
    {-2, 0, 0},
};

const TriangleVertexIndices CUBE_VERTEX_NORMAL_INDICES[] = {
    {0, 0, 0},
    {0, 0, 0},
    {1, 1, 1},
    {1, 1, 1},
    {2, 2, 2},
    {2, 2, 2},
    {3, 3, 3},
    {3, 3, 3},
    {4, 4, 4},
    {4, 4, 4},
    {5, 5, 5},
    {5, 5, 5}
};

#undef L
#undef R
#undef B
#undef T
#undef K
#undef F

#undef LBK
#undef LBF
#undef LTF
#undef LTK
#undef RBK
#undef RBF
#undef RTF
#undef RTK

enum class CubeEdgesType {
    Full,
    Quad,
    BBox
};

struct Mesh {
    AABB aabb;
    BVH bvh;
    Triangle *triangles;

    vec3 *vertex_positions{nullptr};
    vec3 *vertex_normals{nullptr};
    vec3 *vertex_tangents{nullptr};
    vec2 *vertex_uvs{nullptr};

    TriangleVertexIndices *vertex_position_indices{nullptr};
    TriangleVertexIndices *vertex_normal_indices{nullptr};
    TriangleVertexIndices *vertex_tangent_indices{nullptr};
    TriangleVertexIndices *vertex_uvs_indices{nullptr};

    EdgeVertexIndices *edge_vertex_indices{nullptr};

    u32 triangle_count{0};
    u32 vertex_count{0};
    u32 edge_count{0};
    u32 normals_count{0};
    u32 tangents_count{0};
    u32 uvs_count{0};

    Mesh() = default;

    Mesh(u32 triangle_count,
         u32 vertex_count,
         u32 normals_count,
         u32 tangents_count,
         u32 uvs_count,
         u32 edge_count,

         vec3 *vertex_positions,
         vec3 *vertex_normals,
         vec3 *vertex_tangents,
         vec2 *vertex_uvs,

         TriangleVertexIndices *vertex_position_indices,
         TriangleVertexIndices *vertex_normal_indices,
         TriangleVertexIndices *vertex_tangent_indices,
         TriangleVertexIndices *vertex_uvs_indices,

         EdgeVertexIndices *edge_vertex_indices,
         AABB aabb
    ) :
            triangle_count{triangle_count},
            vertex_count{vertex_count},
            normals_count{normals_count},
            tangents_count{tangents_count},
            uvs_count{uvs_count},
            edge_count{edge_count},

            vertex_positions{vertex_positions},
            vertex_normals{vertex_normals},
            vertex_tangents{vertex_tangents},
            vertex_uvs{vertex_uvs},

            vertex_position_indices{vertex_position_indices},
            vertex_normal_indices{vertex_normal_indices},
            vertex_tangent_indices{vertex_tangent_indices},
            vertex_uvs_indices{vertex_uvs_indices},

            edge_vertex_indices{edge_vertex_indices},
            aabb{aabb}
    {}

    void loadEdges(Edge *edges) const {
        EdgeVertexIndices *ids = edge_vertex_indices;
        for (u32 edge_index = 0; edge_index < edge_count; edge_index++, ids++)
            edges[edge_index] = {
                vertex_positions[ids->from],
                vertex_positions[ids->to]
            };
    }

    explicit Mesh(CubeEdgesType edges, bool quad_uvs = true) {
        loadCube(edges, quad_uvs);
    }

    void loadCube(CubeEdgesType edges = CubeEdgesType::BBox, bool quad_uvs = true) {
        triangle_count = CUBE_TRIANGLE_COUNT;
        vertex_count = CUBE_VERTEX_COUNT;
        normals_count = CUBE_NORMAL_COUNT;
        tangents_count = CUBE_TANGENT_COUNT;

        vertex_positions = (vec3*)CUBE_VERTEX_POSITIONS;
        vertex_normals = (vec3*)CUBE_VERTEX_NORMALS;
        vertex_tangents = (vec3*)CUBE_VERTEX_TANGENTS;

        vertex_position_indices = (TriangleVertexIndices*)CUBE_VERTEX_POSITION_INDICES;
        vertex_normal_indices = (TriangleVertexIndices*)CUBE_VERTEX_NORMAL_INDICES;
        vertex_tangent_indices = (TriangleVertexIndices*)CUBE_VERTEX_TANGENT_INDICES;

        if (quad_uvs) {
            uvs_count = CUBE_QUAD_UV_COUNT;
            vertex_uvs = (vec2*)CUBE_QUAD_VERTEX_UVS;
            vertex_uvs_indices = (TriangleVertexIndices*)CUBE_QUAD_VERTEX_UV_INDICES;
        } else {
            uvs_count = CUBE_FLAT_UV_COUNT;
            vertex_uvs = (vec2*)CUBE_QUAD_VERTEX_UVS;
            vertex_uvs_indices = (TriangleVertexIndices*)CUBE_QUAD_VERTEX_UVS;
        }
//
//        vec3 *tangent = vertex_tangents;
//        vec2 uv[3];
//        vec3 pos[3];
//        for (u32 f = 0; f < triangle_count; f++) {
//            for (u8 i = 0; i < 3; i++) {
//                uv[i] = vertex_uvs[vertex_uvs_indices[f].ids[i]];
//                pos[i] = vertex_positions[vertex_position_indices[f].ids[i]];
//            }
//            for (u8 i = 0; i < 3; i++, tangent++) {
//                u8 i1 = i;
//                u8 i2 = (i + 1) % 3;
//                u8 i3 = (i + 2) % 3;
//                vec3 edge1 = pos[i2] - pos[i1];
//                vec3 edge2 = pos[i3] - pos[i1];
//                vec2 deltaUV1 = uv[i2] - uv[i1];
//                vec2 deltaUV2 = uv[i3] - uv[i1];
//
//                *tangent = vec3{
//                    (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x),
//                    (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y),
//                    (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z)
//                } / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
//            }
//        }
//        printf("\n{ // Tangents:\n");
//        for (u32 i = 0; i < 36; i++) {
//            printf("   {%ff, %ff, %ff},\n",
//                   vertex_tangents[i].x, vertex_tangents[i].y, vertex_tangents[i].z);
//        }
//        printf("}\n\n");
        switch (edges) {
            case CubeEdgesType::Full:
                edge_count = CUBE_TRIANGLE_EDGE_COUNT;
                edge_vertex_indices = (EdgeVertexIndices*)CUBE_EDGES;
                break;
            case CubeEdgesType::BBox:
                edge_count = CUBE_BBOX_EDGE_COUNT;
                edge_vertex_indices = (EdgeVertexIndices*)CUBE_EDGES;
                break;
            case CubeEdgesType::Quad:
                edge_count = CUBE_QUAD_EDGE_COUNT;
                edge_vertex_indices = (EdgeVertexIndices*)CUBE_QUAD_EDGES;
                break;
        }

        aabb = {-1 , +1};
        triangles = CUBE_TRIANGLES;
        bvh.nodes = CUBE_BVH_NODES;
        bvh.node_count = 11;
        bvh.height = 5;
    }
};