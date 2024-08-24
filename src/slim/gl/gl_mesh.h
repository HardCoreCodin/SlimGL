#pragma once

#include "./gl_base.h"
#include "../scene/mesh.h"

struct TriangleVertex {
    vec3 position;
    vec2 uv;
    vec3 normal;
//    vec3 tangent;
};


template <typename Vertex = TriangleVertex>
void loadVertices(const Mesh &mesh, Vertex *vertices, bool flip_winding_order = false) {
    for (u32 triangle_index = 0; triangle_index < mesh.triangle_count; triangle_index++) {
        for (u32 vertex_num = 0; vertex_num < 3; vertex_num++, vertices++) {
            u32 v = vertex_num == 0 || !flip_winding_order ? vertex_num : (vertex_num == 2 ? 1 : 2);
            vertices->position = mesh.vertex_positions[mesh.vertex_position_indices[triangle_index].ids[v]];
            if (mesh.uvs_count)      vertices->uv       = mesh.vertex_uvs[      mesh.vertex_uvs_indices[     triangle_index].ids[v]];
            if (mesh.normals_count)  vertices->normal   = mesh.vertex_normals[  mesh.vertex_normal_indices[  triangle_index].ids[v]];
//            if (mesh.tangents_count) vertices->tangent  = mesh.vertex_tangents[ mesh.vertex_tangent_indices[ triangle_index].ids[v]];
        }
    }
}

struct GLMesh {
    GLuint VAO = 0, VBO = 0, IBO = 0;
    GLsizei index_count = 0;

    GLMesh() = default;

    template <typename Vertex = TriangleVertex>
    explicit GLMesh(const Mesh &mesh) {
        create(mesh);
    }

    template <typename Vertex = TriangleVertex>
    explicit GLMesh(Vertex *vertices, u32 count) {
        create(vertices, count);
    }

    template <typename Vertex = TriangleVertex>
    explicit GLMesh(Vertex *vertices, u32 vertex_count, TriangleVertexIndices *indices = nullptr, u32 indices_count = 0) {
        create(vertices, vertex_count, indices, indices_count);
    }


    template <typename Vertex = TriangleVertex>
    void create(const Mesh &mesh) {
//        u32 edge_vertex_count = mesh.edge_count * 2;
        u32 vertex_count = mesh.triangle_count * 3;
        auto *vertices = new Vertex[vertex_count];
//        auto *edges = new Edge[mesh.edge_count];
        loadVertices<Vertex>(mesh, vertices);
//        mesh.loadEdges(edges);
        /*
        auto& indices = mesh.vertex_position_indices;
        for (u32 triangle_index = 0; triangle_index < mesh.triangle_count; triangle_index++) {
            u32 id1 = indices[triangle_index].ids[0];
            u32 id2 = indices[triangle_index].ids[1];
            u32 id3 = indices[triangle_index].ids[2];
            vec3 v1 = vertices[id1].position;
            vec3 v2 = vertices[id2].position;
            vec3 v3 = vertices[id3].position;
            vec3 e1 = v2 - v1;
            vec3 e2 = v3 - v1;
            vec3 n = e1.cross(e2).normalized();
            vertices[id1].normal += n;
            vertices[id2].normal += n;
            vertices[id3].normal += n;
        }
        for (u32 vertex_index = 0; vertex_index < vertex_count; vertex_index++) {
            vertices[vertex_index].normal = -vertices[vertex_index].normal.normalized();
        }
        */
        create(vertices, vertex_count);

        delete[] vertices;
//        delete[] edges;
    }

//    template <typename Vertex = TriangleVertex>
    void create(TriangleVertex *vertices, u32 vertex_count, TriangleVertexIndices *indices = nullptr, u32 indices_count = 0) {


        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        if (indices && indices_count) {
            index_count = (GLsizei)(indices_count * 3);
            auto buffer_size = (GLsizei)(sizeof(TriangleVertexIndices) * indices_count);
            glGenBuffers(1, &IBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size, indices, GL_STATIC_DRAW);
        } else {
            IBO = 0;
            index_count = (GLsizei)(vertex_count * 3);
        }

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleVertex) * vertex_count, vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex), (void*)(sizeof(vec3)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(TriangleVertex), (void*)(sizeof(vec3) + sizeof(vec2)));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        if (IBO)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }

    void render() const
    {
        glBindVertexArray(VAO);

        if (IBO) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
            glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        } else {
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glDrawArrays(GL_TRIANGLES, 0, index_count);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(0);
    }

    void destroy() {
        if (IBO != 0)
        {
            glDeleteBuffers(1, &IBO);
            IBO = 0;
        }

        if (VBO != 0)
        {
            glDeleteBuffers(1, &VBO);
            VBO = 0;
        }

        if (VAO != 0)
        {
            glDeleteVertexArrays(1, &VAO);
            VAO = 0;
        }

        index_count = 0;
    }
};