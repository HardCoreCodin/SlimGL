#pragma once

#include "./mesh.h"
#include "../core/ray.h"

struct MeshTracer {
    u32 *stack = nullptr;

    mutable RayHit triangle_hit;

    INLINE_XPU explicit MeshTracer(u32 *stack) : stack{stack} {}

    explicit MeshTracer(u32 stack_size, memory::MonotonicAllocator *memory_allocator = nullptr) {
        memory::MonotonicAllocator temp_allocator;
        if (!memory_allocator) {
            temp_allocator = memory::MonotonicAllocator{sizeof(u32) * stack_size};
            memory_allocator = &temp_allocator;
        }

        stack = stack_size ? (u32*)memory_allocator->allocate(sizeof(u32) * stack_size) : nullptr;
    }

    INLINE_XPU bool hitTriangles(Triangle *triangles, u32 triangle_count, f32 closest_distance, const Ray &ray, RayHit &hit, bool any_hit) const {
        vec3 UV;
        bool found_triangle = false;
        Triangle *triangle = triangles;
        closest_distance = Min(closest_distance, hit.distance);
        for (u32 i = 0; i < triangle_count; i++, triangle++) {
            if (ray.hitsPlane(triangle->position, triangle->normal, triangle_hit)) {
                UV = triangle->local_to_tangent * (triangle_hit.position - triangle->position);
                if (UV.x < 0 || UV.y < 0 || (UV.x + UV.y) > 1 || triangle_hit.distance >= closest_distance)
                    continue;

                closest_distance = triangle_hit.distance;
                hit = triangle_hit;
                hit.uv.x = UV.x;
                hit.uv.y = UV.y;
                hit.uv_coverage = triangle->uv_coverage;
                hit.id = i;

                found_triangle = true;

                if (any_hit)
                    break;
            }
        }

        return found_triangle;
    }

    INLINE_XPU bool trace(const Mesh &mesh, Ray &ray, RayHit &hit, bool any_hit) {
        bool hit_left, hit_right, found = false;
        f32 left_near_distance, right_near_distance, left_far_distance, right_far_distance;

        if (!(ray.hitsAABB(mesh.bvh.nodes->aabb, left_near_distance, left_far_distance) && left_near_distance < hit.distance))
            return false;

        if (unlikely(mesh.bvh.nodes->leaf_count))
            return hitTriangles(mesh.triangles, mesh.triangle_count, left_far_distance, ray, hit, any_hit);

        BVHNode *left_node = mesh.bvh.nodes + mesh.bvh.nodes->first_index;
        BVHNode *right_node, *tmp_node;
        u32 top = 0;

        while (true) {
            right_node = left_node + 1;

            hit_left  = ray.hitsAABB(left_node->aabb, left_near_distance, left_far_distance) && left_near_distance < hit.distance;
            hit_right = ray.hitsAABB(right_node->aabb, right_near_distance, right_far_distance) && right_near_distance < hit.distance;

            if (hit_left) {
                if (unlikely(left_node->leaf_count)) {
                    if (hitTriangles(mesh.triangles + left_node->first_index, left_node->leaf_count, left_far_distance, ray, hit, any_hit)) {
                        hit.id += left_node->first_index;
                        found = true;
                        if (any_hit)
                            break;
                    }

                    left_node = nullptr;
                }
            } else
                left_node = nullptr;

            if (hit_right) {
                if (unlikely(right_node->leaf_count)) {
                    if (hitTriangles(mesh.triangles + right_node->first_index, right_node->leaf_count, right_far_distance, ray, hit, any_hit)) {
                        hit.id += right_node->first_index;
                        found = true;
                        if (any_hit)
                            break;
                    }

                    right_node = nullptr;
                }
            } else
                right_node = nullptr;

            if (left_node) {
                if (right_node) {
                    if (!any_hit && left_near_distance > right_near_distance) {
                        tmp_node = left_node;
                        left_node = right_node;
                        right_node = tmp_node;
                    }
                    stack[top++] = right_node->first_index;
                }
                left_node = mesh.bvh.nodes + left_node->first_index;
            } else if (right_node) {
                left_node = mesh.bvh.nodes + right_node->first_index;
            } else {
                if (top == 0) break;
                left_node = mesh.bvh.nodes + stack[--top];
            }
        }

        if (found && !any_hit && mesh.normals_count | mesh.uvs_count) {
            Triangle &triangle = mesh.triangles[hit.id];
            f32 a = hit.uv.u;
            f32 b = hit.uv.v;
            f32 c = 1 - a - b;
            if (mesh.uvs_count) {
                hit.uv.x = fast_mul_add(triangle.uv3.x, a, fast_mul_add(triangle.uv2.u, b, triangle.uv1.u * c));
                hit.uv.y = fast_mul_add(triangle.uv3.y, a, fast_mul_add(triangle.uv2.v, b, triangle.uv1.v * c));
            }
            if (mesh.normals_count) {
                hit.normal.x = fast_mul_add(triangle.n3.x, a, fast_mul_add(triangle.n2.x, b, triangle.n1.x * c));
                hit.normal.y = fast_mul_add(triangle.n3.y, a, fast_mul_add(triangle.n2.y, b, triangle.n1.y * c));
                hit.normal.z = fast_mul_add(triangle.n3.z, a, fast_mul_add(triangle.n2.z, b, triangle.n1.z * c));
            }
        }

        return found;
    }
};