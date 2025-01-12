#pragma once

#include "./utils.h"
#include "../core/transform2d.h"

mat3 Mat3(const Transform2D &transform) { return Mat3(transform.position, transform.rotation, transform.scale); }
