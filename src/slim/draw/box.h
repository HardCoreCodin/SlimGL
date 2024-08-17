#pragma once

#include "./edge.h"
#include "../core/transform.h"
#include "../scene/box.h"

void drawBox(const Box &box, const Transform &transform, const Viewport &viewport,
             const Color &color = White, f32 opacity = 1.0f, u8 line_width = 1, u8 sides = BOX__ALL_SIDES) {

    static Box view_space_box;

    // Transform vertices positions from local-space to world-space and then to view-space:
    for (u8 i = 0; i < BOX__VERTEX_COUNT; i++)
        view_space_box.vertices.array[i] = viewport.camera->internPos(transform.externPos(box.vertices.array[i]));

    // Distribute transformed vertices positions to edges:
    view_space_box.edges.setFrom(view_space_box.vertices);

    if (sides == BOX__ALL_SIDES) for (const auto &edge : view_space_box.edges.array)
        drawEdge(edge, viewport, color, opacity, line_width);
    else {
        if (sides & BoxSide_Front | sides & BoxSide_Top   ) drawEdge(view_space_box.edges.sides.front_top,    viewport, color, opacity, line_width);
        if (sides & BoxSide_Front | sides & BoxSide_Bottom) drawEdge(view_space_box.edges.sides.front_bottom, viewport, color, opacity, line_width);
        if (sides & BoxSide_Front | sides & BoxSide_Left  ) drawEdge(view_space_box.edges.sides.front_left,   viewport, color, opacity, line_width);
        if (sides & BoxSide_Front | sides & BoxSide_Right ) drawEdge(view_space_box.edges.sides.front_right,  viewport, color, opacity, line_width);
        if (sides & BoxSide_Back  | sides & BoxSide_Top   ) drawEdge(view_space_box.edges.sides.back_top,     viewport, color, opacity, line_width);
        if (sides & BoxSide_Back  | sides & BoxSide_Bottom) drawEdge(view_space_box.edges.sides.back_bottom,  viewport, color, opacity, line_width);
        if (sides & BoxSide_Back  | sides & BoxSide_Left  ) drawEdge(view_space_box.edges.sides.back_left,    viewport, color, opacity, line_width);
        if (sides & BoxSide_Back  | sides & BoxSide_Right ) drawEdge(view_space_box.edges.sides.back_right,   viewport, color, opacity, line_width);
        if (sides & BoxSide_Left  | sides & BoxSide_Top   ) drawEdge(view_space_box.edges.sides.left_top,     viewport, color, opacity, line_width);
        if (sides & BoxSide_Left  | sides & BoxSide_Bottom) drawEdge(view_space_box.edges.sides.left_bottom,  viewport, color, opacity, line_width);
        if (sides & BoxSide_Right | sides & BoxSide_Top   ) drawEdge(view_space_box.edges.sides.right_top,    viewport, color, opacity, line_width);
        if (sides & BoxSide_Right | sides & BoxSide_Bottom) drawEdge(view_space_box.edges.sides.right_bottom, viewport, color, opacity, line_width);
    }
}