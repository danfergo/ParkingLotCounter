//
// Created by danfergo on 02-11-2016.
//

#ifndef DISPLAYIMAGE_GEOMETRY_H
#define DISPLAYIMAGE_GEOMETRY_H

#include <opencv2/core/core.hpp>

using namespace cv;

class Geometry {
public:
     static float slopeAngle(Vec4i edge);
     static Vec2i intersect(Vec4i edge1, Vec4i edge2);
     static float distance(Vec2i x, Vec2i y);
};


#endif //DISPLAYIMAGE_GEOMETRY_H
