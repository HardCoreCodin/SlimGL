#pragma once

#include "../core/string.h"
#include "../scene/mesh.h"
#include "./bvh.h"


u32 getSizeInBytes(const Mesh &mesh, u32 *bvh_nodes_size = nullptr) {
    u32 memory_size = getSizeInBytes(mesh.bvh);
    if (bvh_nodes_size) {
        *bvh_nodes_size += memory_size;
        memory_size = 0;
    }

    memory_size += sizeof(Triangle) * mesh.triangle_count;
    memory_size += sizeof(vec3) * mesh.vertex_count;
    memory_size += sizeof(TriangleVertexIndices) * mesh.triangle_count;
    memory_size += sizeof(EdgeVertexIndices) * mesh.edge_count;

    if (mesh.uvs_count) {
        memory_size += sizeof(vec2) * mesh.uvs_count;
        memory_size += sizeof(TriangleVertexIndices) * mesh.triangle_count;
    }
    if (mesh.normals_count) {
        memory_size += sizeof(vec3) * mesh.normals_count;
        memory_size += sizeof(TriangleVertexIndices) * mesh.triangle_count;
    }
    if (mesh.tangents_count) {
        memory_size += sizeof(vec3) * mesh.tangents_count;
        memory_size += sizeof(TriangleVertexIndices) * mesh.triangle_count;
    }
    return memory_size;
}

bool allocateMemory(Mesh &mesh, memory::MonotonicAllocator *memory_allocator, memory::MonotonicAllocator *memory_allocator_for_bvh_nodes = nullptr) {
    if (memory_allocator_for_bvh_nodes) {
        u32 bvh_nodes_size;
        if (getSizeInBytes(mesh, &bvh_nodes_size) > (memory_allocator->capacity - memory_allocator->occupied)) return false;
        allocateMemory(mesh.bvh, memory_allocator_for_bvh_nodes);
    } else {
        if (getSizeInBytes(mesh) > (memory_allocator->capacity - memory_allocator->occupied)) return false;
        allocateMemory(mesh.bvh, memory_allocator);
    }
    mesh.triangles               = (Triangle*             )memory_allocator->allocate(sizeof(Triangle)              * mesh.triangle_count);
    mesh.vertex_positions        = (vec3*                 )memory_allocator->allocate(sizeof(vec3)                  * mesh.vertex_count);
    mesh.vertex_position_indices = (TriangleVertexIndices*)memory_allocator->allocate(sizeof(TriangleVertexIndices) * mesh.triangle_count);
    mesh.edge_vertex_indices     = (EdgeVertexIndices*    )memory_allocator->allocate(sizeof(EdgeVertexIndices)     * mesh.edge_count);
    if (mesh.uvs_count) {
        mesh.vertex_uvs         = (vec2*                 )memory_allocator->allocate(sizeof(vec2)                  * mesh.uvs_count);
        mesh.vertex_uvs_indices = (TriangleVertexIndices*)memory_allocator->allocate(sizeof(TriangleVertexIndices) * mesh.triangle_count);
    }
    if (mesh.normals_count) {
        mesh.vertex_normals          = (vec3*                 )memory_allocator->allocate(sizeof(vec3)                  * mesh.normals_count);
        mesh.vertex_normal_indices   = (TriangleVertexIndices*)memory_allocator->allocate(sizeof(TriangleVertexIndices) * mesh.triangle_count);
    }
    if (mesh.tangents_count) {
        mesh.vertex_tangents          = (vec3*                 )memory_allocator->allocate(sizeof(vec3)                  * mesh.tangents_count);
        mesh.vertex_tangent_indices   = (TriangleVertexIndices*)memory_allocator->allocate(sizeof(TriangleVertexIndices) * mesh.triangle_count);
    }
    return true;
}

void writeHeader(const Mesh &mesh, void *file) {
    os::writeToFile((void*)&mesh.vertex_count,   sizeof(u32),  file);
    os::writeToFile((void*)&mesh.triangle_count, sizeof(u32),  file);
    os::writeToFile((void*)&mesh.edge_count,     sizeof(u32),  file);
    os::writeToFile((void*)&mesh.uvs_count,      sizeof(u32),  file);
    os::writeToFile((void*)&mesh.normals_count,  sizeof(u32),  file);
    os::writeToFile((void*)&mesh.tangents_count, sizeof(u32),  file);
    writeHeader(mesh.bvh, file);
}
void readHeader(Mesh &mesh, void *file) {
    os::readFromFile(&mesh.vertex_count,   sizeof(u32),  file);
    os::readFromFile(&mesh.triangle_count, sizeof(u32),  file);
    os::readFromFile(&mesh.edge_count,     sizeof(u32),  file);
    os::readFromFile(&mesh.uvs_count,      sizeof(u32),  file);
    os::readFromFile(&mesh.normals_count,  sizeof(u32),  file);
    os::readFromFile(&mesh.tangents_count, sizeof(u32),  file);
    readHeader(mesh.bvh, file);
}

bool saveHeader(const Mesh &mesh, char *file_path) {
    void *file = os::openFileForWriting(file_path);
    if (!file) return false;
    writeHeader(mesh, file);
    os::closeFile(file);
    return true;
}

bool loadHeader(Mesh &mesh, char *file_path) {
    void *file = os::openFileForReading(file_path);
    if (!file) return false;
    readHeader(mesh, file);
    os::closeFile(file);
    return true;
}

void readContent(Mesh &mesh, void *file) {
    os::readFromFile(&mesh.aabb.min,       sizeof(vec3), file);
    os::readFromFile(&mesh.aabb.max,       sizeof(vec3), file);
    os::readFromFile(mesh.triangles,       sizeof(Triangle) * mesh.triangle_count, file);
    os::readFromFile(mesh.vertex_positions,             sizeof(vec3)                  * mesh.vertex_count,   file);
    os::readFromFile(mesh.vertex_position_indices,      sizeof(TriangleVertexIndices) * mesh.triangle_count, file);
    os::readFromFile(mesh.edge_vertex_indices,          sizeof(EdgeVertexIndices)     * mesh.edge_count,     file);
    if (mesh.uvs_count) {
        os::readFromFile(mesh.vertex_uvs,               sizeof(vec2)                  * mesh.uvs_count,      file);
        os::readFromFile(mesh.vertex_uvs_indices,       sizeof(TriangleVertexIndices) * mesh.triangle_count, file);
    }
    if (mesh.normals_count) {
        os::readFromFile(mesh.vertex_normals,                sizeof(vec3)                  * mesh.normals_count,  file);
        os::readFromFile(mesh.vertex_normal_indices,         sizeof(TriangleVertexIndices) * mesh.triangle_count, file);
    }
    if (mesh.tangents_count) {
        os::readFromFile(mesh.vertex_tangents,                sizeof(vec3)                  * mesh.tangents_count, file);
        os::readFromFile(mesh.vertex_tangent_indices,         sizeof(TriangleVertexIndices) * mesh.triangle_count, file);
    }
    readContent(mesh.bvh, file);
}
void writeContent(const Mesh &mesh, void *file) {
    os::writeToFile((void*)&mesh.aabb.min,       sizeof(vec3), file);
    os::writeToFile((void*)&mesh.aabb.max,       sizeof(vec3), file);
    os::writeToFile((void*)mesh.triangles,               sizeof(Triangle)              * mesh.triangle_count, file);
    os::writeToFile((void*)mesh.vertex_positions,        sizeof(vec3)                  * mesh.vertex_count,   file);
    os::writeToFile((void*)mesh.vertex_position_indices, sizeof(TriangleVertexIndices) * mesh.triangle_count, file);
    os::writeToFile((void*)mesh.edge_vertex_indices,     sizeof(EdgeVertexIndices)     * mesh.edge_count,     file);
    if (mesh.uvs_count) {
        os::writeToFile(mesh.vertex_uvs,          sizeof(vec2)                  * mesh.uvs_count,      file);
        os::writeToFile(mesh.vertex_uvs_indices,  sizeof(TriangleVertexIndices) * mesh.triangle_count, file);
    }
    if (mesh.normals_count) {
        os::writeToFile(mesh.vertex_normals,        sizeof(vec3)                  * mesh.normals_count,  file);
        os::writeToFile(mesh.vertex_normal_indices, sizeof(TriangleVertexIndices) * mesh.triangle_count, file);
    }
    if (mesh.tangents_count) {
        os::writeToFile(mesh.vertex_tangents,        sizeof(vec3)                  * mesh.tangents_count,  file);
        os::writeToFile(mesh.vertex_tangent_indices, sizeof(TriangleVertexIndices) * mesh.triangle_count, file);
    }
    writeContent(mesh.bvh, file);
}

bool saveContent(const Mesh &mesh, char *file_path) {
    void *file = os::openFileForWriting(file_path);
    if (!file) return false;
    writeContent(mesh, file);
    os::closeFile(file);
    return true;
}

bool loadContent(Mesh &mesh, char *file_path) {
    void *file = os::openFileForReading(file_path);
    if (!file) return false;
    readContent(mesh, file);
    os::closeFile(file);
    return true;
}

bool save(const Mesh &mesh, char* file_path) {
    void *file = os::openFileForWriting(file_path);
    if (!file) return false;
    writeHeader(mesh, file);
    writeContent(mesh, file);
    os::closeFile(file);
    return true;
}

bool load(Mesh &mesh, char *file_path,
          memory::MonotonicAllocator *memory_allocator = nullptr,
          memory::MonotonicAllocator *memory_allocator_for_bvh_nodes = nullptr) {
    void *file = os::openFileForReading(file_path);
    if (!file) return false;

    if (memory_allocator) {
        mesh = Mesh{};
        readHeader(mesh, file);
        if (!allocateMemory(mesh, memory_allocator, memory_allocator_for_bvh_nodes)) return false;
    } else if (!mesh.vertex_positions) return false;
    readContent(mesh, file);
    os::closeFile(file);
    return true;
}

u32 getTotalMemoryForMeshes(String *mesh_files, u32 mesh_count, u32 *max_triangle_count = nullptr, u32 *bvh_nodes_size = nullptr) {
    u32 memory_size = 0;
    if (max_triangle_count) *max_triangle_count = 0;
    for (u32 i = 0; i < mesh_count; i++) {
        Mesh mesh;
        loadHeader(mesh, mesh_files[i].char_ptr);
        if (max_triangle_count) *max_triangle_count = Max(*max_triangle_count, mesh.triangle_count);
        memory_size += getSizeInBytes(mesh, bvh_nodes_size);
    }

    return memory_size;
}