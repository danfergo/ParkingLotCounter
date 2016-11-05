//
// Created by danfergo on 02-11-2016.
//

#include "Geometry.h"

float Geometry::slopeAngle(Vec4i points) {
    return fastAtan2(points[2] - points[0], points[3] - points[1]);
}

Vec2i Geometry::intersect(Vec4i edge1, Vec4i edge2) {
    float x1 = edge1[0], y1 = edge1[1], x2 = edge1[2] , y2 = edge1[3];
    float x3 = edge2[0], y3 = edge2[1], x4 = edge2[2] , y4 = edge2[3];

    float x = ((x1*y2 - y1*x2)*(x3 - x4) - (x1 - x2)*(x3*y4 - y3*x4))
            / ((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4));

    float y = ((x1*y2 - y1*x2)*(y3 - y4) - (y1-y2)*(x3*y4 - y3*x4))
            / ((x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4));

    return cv::Vec2i(x, y);
}

float Geometry::distance(Vec2i x, Vec2i y) {
    return sqrt(pow(y[0]-x[0],2) + pow(y[1]-x[1],2));
}
