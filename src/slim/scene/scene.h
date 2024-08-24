#pragma once

#include "./mesh.h"
#include "./grid.h"
#include "./box.h"
#include "./camera.h"
#include "./light.h"
#include "./material.h"
#include "./bvh_builder.h"
#include "../core/texture.h"
#include "../core/ray.h"
#include "../core/transform.h"
#include "../serialization/texture.h"
#include "../serialization/mesh.h"

struct SceneCountsData {
    u32 geometries;
    u32 cameras;
    u32 directional_lights;
    u32 point_lights;
    u32 spot_lights;
    u32 materials;
    u32 textures;
    u32 meshes;
    u32 grids;
    u32 boxes;
    u32 tets;
    u32 quads;
    u32 curves;
};

struct SceneCounts : SceneCountsData {
     SceneCounts(
         u32 geometries = 0, 
         u32 cameras = 0, 
         u32 directional_lights = 0, 
         u32 point_lights = 0,
         u32 spot_lights = 0,
         u32 materials = 0, 
         u32 textures = 0,
         u32 meshes = 0, 
         u32 grids = 0, 
         u32 boxes = 0, 
         u32 tets = 0, 
         u32 quads = 0, 
         u32 curves = 0
     ) : SceneCountsData{
         geometries, 
         cameras,
         directional_lights,
         point_lights,
         spot_lights,
         materials,
         textures, 
         meshes, 
         grids, 
         boxes, 
         tets, 
         quads, 
         curves
     } 
     {}
};

#define SCENE_HAD_EMISSIVE_QUADS 1

struct SceneIO {
    String file_path;
    u64 last_io_ticks = 0;
    bool last_io_is_save{false};
};

struct SceneData {
    SceneCountsData counts;
    u16 flags, mesh_stack_size;

    Geometry *geometries;
    Camera *cameras;
    DirectionalLight *directional_lights;
    PointLight *point_lights;
    SpotLight *spot_lights;
    Material *materials;
    Texture *textures;
    Mesh *meshes;
    Grid *grids;
    Box *boxes;
    Curve *curves;

    AABB *aabbs;
    SceneIO *io;
    BVHBuilder *bvh_builder;
    u32 *bvh_leaf_geometry_indices;
    BVH bvh;
};

struct Scene : SceneData {
    Scene(
        SceneCounts counts, 
        Geometry *geometries = nullptr, 
        Camera *cameras = nullptr,
        DirectionalLight *directional_lights = nullptr, 
        PointLight *point_lights = nullptr,
        SpotLight *spot_lights = nullptr,
        Material *materials = nullptr,
        Texture *textures = nullptr, 
        String *texture_files = nullptr,
        Mesh *meshes = nullptr, 
        String *mesh_files = nullptr,
        Grid *grids = nullptr, 
        Box *boxes = nullptr, 
        Curve *curves = nullptr,
        SceneIO *scene_io = nullptr,
        
        memory::MonotonicAllocator *memory_allocator = nullptr
    ) : SceneData{
        counts, 0, 0,
        geometries, 
        cameras, 
        directional_lights, 
        point_lights, 
        spot_lights, 
        materials, 
        textures, 
        meshes, 
        grids, 
        boxes, 
        curves
    } {
        bvh.node_count = counts.geometries * 2;
        bvh.height = (u8)counts.geometries;

        memory::MonotonicAllocator temp_allocator;
        u32 capacity = sizeof(BVHBuilder) + (sizeof(u32) + sizeof(AABB) + sizeof(RectI)) * counts.geometries;
        u32 bvh_nodes_capacity = sizeof(BVHNode) * bvh.node_count;

        if (counts.directional_lights && !directional_lights) capacity += sizeof(DirectionalLight) * counts.point_lights;
        if (counts.point_lights && !point_lights) capacity += sizeof(PointLight) * counts.point_lights;
        if (counts.spot_lights && !spot_lights) capacity += sizeof(SpotLight) * counts.spot_lights;
        if (counts.materials && !materials) capacity += sizeof(Material) * counts.materials;
        if (counts.geometries && !geometries) capacity += sizeof(Geometry) * counts.geometries;
        if (counts.boxes && !boxes) capacity += sizeof(Box) * counts.boxes;
        if (counts.curves && !curves) capacity += sizeof(Curve) * counts.curves;

        if (counts.textures) {
            if (!textures) capacity += sizeof(Texture) * counts.textures;
            capacity += getTotalMemoryForTextures(texture_files, counts.textures);
        }
        u32 max_triangle_count = 0;
        if (counts.meshes) {
            if (!meshes) capacity += sizeof(Mesh) * counts.meshes;
            capacity += getTotalMemoryForMeshes(mesh_files, counts.meshes, &max_triangle_count, &bvh_nodes_capacity);
            capacity += sizeof(u32) * (2 * counts.meshes);
        }
        u32 max_leaf_node_count = Max(max_triangle_count, counts.geometries);
        capacity += BVHBuilder::getSizeInBytes(max_leaf_node_count);

        if (!memory_allocator) {
            temp_allocator = memory::MonotonicAllocator{bvh_nodes_capacity + capacity};
            memory_allocator = &temp_allocator;
        }
        memory::MonotonicAllocator bvh_nodes_allocator;
        bvh_nodes_allocator.address = (u8*)memory_allocator->allocate(bvh_nodes_capacity);
        bvh_nodes_allocator.capacity = (u64)bvh_nodes_capacity;

        bvh.nodes = (BVHNode*)bvh_nodes_allocator.allocate(sizeof(BVHNode) * bvh.node_count);
        bvh_leaf_geometry_indices = (u32*)memory_allocator->allocate(sizeof(u32) * counts.geometries);
        bvh_builder = (BVHBuilder*)memory_allocator->allocate(sizeof(BVHBuilder));
        *bvh_builder = BVHBuilder{max_leaf_node_count, memory_allocator};

        aabbs = (AABB*)memory_allocator->allocate(sizeof(AABB) * counts.geometries);

        if (counts.geometries && !geometries) {
            geometries = (Geometry*)memory_allocator->allocate(sizeof(Geometry) * counts.geometries);
            for (u32 i = 0; i < counts.geometries; i++) geometries[i] = Geometry{};
        }
        if (counts.boxes && !boxes) {
            boxes = (Box*)memory_allocator->allocate(sizeof(Box) * counts.boxes);
            for (u32 i = 0; i < counts.boxes; i++) boxes[i] = Box{};
        }
        if (counts.curves && !curves) {
            curves = (Curve*)memory_allocator->allocate(sizeof(Curve) * counts.curves);
            for (u32 i = 0; i < counts.curves; i++) curves[i] = Curve{};
        }
        if (counts.directional_lights && !directional_lights) {
            directional_lights = (DirectionalLight*)memory_allocator->allocate(sizeof(DirectionalLight) * counts.directional_lights);
            for (u32 i = 0; i < counts.directional_lights; i++) directional_lights[i] = DirectionalLight{};
        }
        if (counts.point_lights && !point_lights) {
            point_lights = (PointLight*)memory_allocator->allocate(sizeof(PointLight) * counts.point_lights);
            for (u32 i = 0; i < counts.point_lights; i++) point_lights[i] = PointLight{};
        }
        if (counts.spot_lights && !spot_lights) {
            spot_lights = (SpotLight*)memory_allocator->allocate(sizeof(SpotLight) * counts.spot_lights);
            for (u32 i = 0; i < counts.spot_lights; i++) spot_lights[i] = SpotLight{};
        }
        if (counts.materials && !materials) {
            materials = (Material*)memory_allocator->allocate(sizeof(Material) * counts.materials);
            for (u32 i = 0; i < counts.materials; i++) materials[i] = Material{};
        }
        if (counts.textures && texture_files) {
            if (!textures) textures = (Texture*)memory_allocator->allocate(sizeof(Texture) * counts.textures);
            for (u32 i = 0; i < counts.textures; i++)
                load(textures[i], texture_files[i].char_ptr, memory_allocator);
        }
        if (counts.meshes && mesh_files) {
            if (!meshes) meshes = (Mesh*)memory_allocator->allocate(sizeof(Mesh) * counts.meshes);
            for (u32 i = 0; i < counts.meshes; i++) meshes[i] = Mesh{};

            for (u32 i = 0; i < counts.meshes; i++) {
                load(meshes[i], mesh_files[i].char_ptr, memory_allocator, &bvh_nodes_allocator);
                mesh_stack_size = Max(mesh_stack_size, meshes[i].bvh.height);
            }
            mesh_stack_size += 2;
        }

        for (u32 i = 0; i < counts.geometries; i++)
            if (geometries[i].type == GeometryType_Quad && materials[geometries[i].material_id].isEmissive()) {
                flags = SCENE_HAD_EMISSIVE_QUADS;
            }

        updateAABBs();
        updateBVH();
    }

    void updateAABB(AABB &aabb, const Geometry &geo, u8 sphere_steps = 255) {
        if (geo.type == GeometryType_Mesh) {
            aabb = meshes[geo.id].aabb;
        } else {
            aabb.max = geo.type == GeometryType_Tet ? TET_MAX : 1.0f;
            aabb.min = -aabb.max.x;
            if (geo.type == GeometryType_Quad) aabb.min.y = aabb.max.y = 0.0f;
        }
        aabb = geo.transform.externAABB(aabb);
    }

    void updateAABBs() {
        for (u32 i = 0; i < counts.geometries; i++)
            updateAABB(aabbs[i], geometries[i]);
    }

    void updateBVH(u16 max_leaf_size = 1) {
        for (u32 i = 0; i < counts.geometries; i++) {
            bvh_builder->nodes[i].aabb = aabbs[i];
            bvh_builder->nodes[i].first_index = bvh_builder->node_ids[i] = i;
        }

        bvh_builder->build(bvh, counts.geometries, max_leaf_size);

        for (u32 i = 0; i < counts.geometries; i++)
            bvh_leaf_geometry_indices[i] = bvh_builder->leaf_ids[i];
    }
};