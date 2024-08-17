#pragma once

#include "../math/vec3.h"

union BoxVertices {
    struct Corners {
        vec3 front_top_left;
        vec3 front_top_right;
        vec3 front_bottom_left;
        vec3 front_bottom_right;
        vec3 back_top_left;
        vec3 back_top_right;
        vec3 back_bottom_left;
        vec3 back_bottom_right;
    } corners;
    vec3 array[BOX__VERTEX_COUNT];

    BoxVertices() : BoxVertices({-1, -1, -1}, {1, 1, 1}) {}
    BoxVertices(const AABB &aabb) : BoxVertices(aabb.min, aabb.max){}
    BoxVertices(const vec3 &min, const vec3 &max) : corners{
        {min.x, max.y, max.z},
        {max.x, max.y, max.z},
        {min.x, min.y, max.z},
        {max.x, min.y, max.z},
        {min.x, max.y, min.z},
        {max.x, max.y, min.z},
        {min.x, min.y, min.z},
        {max.x, min.y, min.z}
    }
    {}
};

union BoxEdges {
    struct Sides {
        Edge front_top,
             front_bottom,
             front_left,
             front_right,
             back_top,
             back_bottom,
             back_left,
             back_right,
             left_bottom,
             left_top,
             right_bottom,
             right_top;
    } sides;
    Edge array[BOX__EDGE_COUNT];

    BoxEdges(const BoxVertices &vertices = BoxVertices{}) { setFrom(vertices); }
    void setFrom(const BoxVertices &vertices) {
        sides.front_top.from    = vertices.corners.front_top_left;
        sides.front_top.to      = vertices.corners.front_top_right;
        sides.front_bottom.from = vertices.corners.front_bottom_left;
        sides.front_bottom.to   = vertices.corners.front_bottom_right;
        sides.front_left.from   = vertices.corners.front_bottom_left;
        sides.front_left.to     = vertices.corners.front_top_left;
        sides.front_right.from  = vertices.corners.front_bottom_right;
        sides.front_right.to    = vertices.corners.front_top_right;

        sides.back_top.from     = vertices.corners.back_top_left;
        sides.back_top.to       = vertices.corners.back_top_right;
        sides.back_bottom.from  = vertices.corners.back_bottom_left;
        sides.back_bottom.to    = vertices.corners.back_bottom_right;
        sides.back_left.from    = vertices.corners.back_bottom_left;
        sides.back_left.to      = vertices.corners.back_top_left;
        sides.back_right.from   = vertices.corners.back_bottom_right;
        sides.back_right.to     = vertices.corners.back_top_right;

        sides.left_bottom.from  = vertices.corners.front_bottom_left;
        sides.left_bottom.to    = vertices.corners.back_bottom_left;
        sides.left_top.from     = vertices.corners.front_top_left;
        sides.left_top.to       = vertices.corners.back_top_left;
        sides.right_bottom.from = vertices.corners.front_bottom_right;
        sides.right_bottom.to   = vertices.corners.back_bottom_right;
        sides.right_top.from    = vertices.corners.front_top_right;
        sides.right_top.to      = vertices.corners.back_top_right;
    }
};

struct Box {
    BoxVertices vertices;
    BoxEdges edges;

    Box() : vertices{}, edges{vertices} {}
};
