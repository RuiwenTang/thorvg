/*
 * Copyright (c) 2020 - 2023 the ThorVG project. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "tvgMath.h"
#include "tvgBezier.h"

#define BEZIER_EPSILON 1e-4f

/************************************************************************/
/* Internal Class Implementation                                        */
/************************************************************************/

static float _lineLength(const Point& pt1, const Point& pt2)
{
    /* approximate sqrt(x*x + y*y) using alpha max plus beta min algorithm.
       With alpha = 1, beta = 3/8, giving results with the largest error less
       than 7% compared to the exact value. */
    Point diff = {pt2.x - pt1.x, pt2.y - pt1.y};
    if (diff.x < 0) diff.x = -diff.x;
    if (diff.y < 0) diff.y = -diff.y;
    return (diff.x > diff.y) ? (diff.x + diff.y * 0.375f) : (diff.y + diff.x * 0.375f);
}


/************************************************************************/
/* External Class Implementation                                        */
/************************************************************************/

namespace tvg
{

void bezSplit(const Bezier& cur, Bezier& left, Bezier& right)
{
    auto c = (cur.ctrl1.x + cur.ctrl2.x) * 0.5f;
    left.ctrl1.x = (cur.start.x + cur.ctrl1.x) * 0.5f;
    right.ctrl2.x = (cur.ctrl2.x + cur.end.x) * 0.5f;
    left.start.x = cur.start.x;
    right.end.x = cur.end.x;
    left.ctrl2.x = (left.ctrl1.x + c) * 0.5f;
    right.ctrl1.x = (right.ctrl2.x + c) * 0.5f;
    left.end.x = right.start.x = (left.ctrl2.x + right.ctrl1.x) * 0.5f;

    c = (cur.ctrl1.y + cur.ctrl2.y) * 0.5f;
    left.ctrl1.y = (cur.start.y + cur.ctrl1.y) * 0.5f;
    right.ctrl2.y = (cur.ctrl2.y + cur.end.y) * 0.5f;
    left.start.y = cur.start.y;
    right.end.y = cur.end.y;
    left.ctrl2.y = (left.ctrl1.y + c) * 0.5f;
    right.ctrl1.y = (right.ctrl2.y + c) * 0.5f;
    left.end.y = right.start.y = (left.ctrl2.y + right.ctrl1.y) * 0.5f;
}


float bezLength(const Bezier& cur)
{
    Bezier left, right;
    auto len = _lineLength(cur.start, cur.ctrl1) + _lineLength(cur.ctrl1, cur.ctrl2) + _lineLength(cur.ctrl2, cur.end);
    auto chord = _lineLength(cur.start, cur.end);

    if (fabsf(len - chord) > BEZIER_EPSILON) {
        bezSplit(cur, left, right);
        return bezLength(left) + bezLength(right);
    }
    return len;
}


void bezSplitLeft(Bezier& cur, float at, Bezier& left)
{
    left.start = cur.start;

    left.ctrl1.x = cur.start.x + at * (cur.ctrl1.x - cur.start.x);
    left.ctrl1.y = cur.start.y + at * (cur.ctrl1.y - cur.start.y);

    left.ctrl2.x = cur.ctrl1.x + at * (cur.ctrl2.x - cur.ctrl1.x);  // temporary holding spot
    left.ctrl2.y = cur.ctrl1.y + at * (cur.ctrl2.y - cur.ctrl1.y);  // temporary holding spot

    cur.ctrl2.x = cur.ctrl2.x + at * (cur.end.x - cur.ctrl2.x);
    cur.ctrl2.y = cur.ctrl2.y + at * (cur.end.y - cur.ctrl2.y);

    cur.ctrl1.x = left.ctrl2.x + at * (cur.ctrl2.x - left.ctrl2.x);
    cur.ctrl1.y = left.ctrl2.y + at * (cur.ctrl2.y - left.ctrl2.y);

    left.ctrl2.x = left.ctrl1.x + at * (left.ctrl2.x - left.ctrl1.x);
    left.ctrl2.y = left.ctrl1.y + at * (left.ctrl2.y - left.ctrl1.y);

    left.end.x = cur.start.x = left.ctrl2.x + at * (cur.ctrl1.x - left.ctrl2.x);
    left.end.y = cur.start.y = left.ctrl2.y + at * (cur.ctrl1.y - left.ctrl2.y);
}


float bezAt(const Bezier& bz, float at)
{
    auto len = bezLength(bz);
    auto biggest = 1.0f;
    auto smallest = 0.0f;
    auto t = 0.5f;

    // just in case to prevent an infinite loop
    if (at <= 0) return 0.0f;
    if (at >= len) return len;

    while (true) {
        auto   right = bz;
        Bezier left;
        bezSplitLeft(right, t, left);
        len = bezLength(left);
        if (fabsf(len - at) < BEZIER_EPSILON || fabsf(smallest - biggest) < BEZIER_EPSILON) {
            break;
        }
        if (len < at) {
            smallest = t;
            t = (t + biggest) * 0.5f;
        } else {
            biggest = t;
            t = (smallest + t) * 0.5f;
        }
    }
    return t;
}


void bezSplitAt(const Bezier& cur, float at, Bezier& left, Bezier& right)
{
    right = cur;
    auto t = bezAt(right, at);
    bezSplitLeft(right, t, left);
}


Point bezPointAt(const Bezier& bz, float t)
{
    Point cur;
    auto  it = 1.0f - t;

    auto ax = bz.start.x * it + bz.ctrl1.x * t;
    auto bx = bz.ctrl1.x * it + bz.ctrl2.x * t;
    auto cx = bz.ctrl2.x * it + bz.end.x * t;
    ax = ax * it + bx * t;
    bx = bx * it + cx * t;
    cur.x = ax * it + bx * t;

    float ay = bz.start.y * it + bz.ctrl1.y * t;
    float by = bz.ctrl1.y * it + bz.ctrl2.y * t;
    float cy = bz.ctrl2.y * it + bz.end.y * t;
    ay = ay * it + by * t;
    by = by * it + cy * t;
    cur.y = ay * it + by * t;

    return cur;
}

bool bezIsFlatten(const Bezier& bz)
{
    float diff1_x = fabs((bz.ctrl1.x * 3.f) - (bz.start.x * 2.f) - bz.end.x);
    float diff1_y = fabs((bz.ctrl1.y * 3.f) - (bz.start.y * 2.f) - bz.end.y);
    float diff2_x = fabs((bz.ctrl2.x * 3.f) - (bz.end.x * 2.f) - bz.start.x);
    float diff2_y = fabs((bz.ctrl2.y * 3.f) - (bz.end.y * 2.f) - bz.start.y);

    if (diff1_x < diff2_x) diff1_x = diff2_x;
    if (diff1_y < diff2_y) diff1_y = diff2_y;

    if (diff1_x + diff1_y <= 0.5f) return true;

    return false;
}

Bezier bezFromArc(const Point& start, const Point& end, float radius)
{
    // Calculate the angle between the start and end points
    float angle = atan2(end.y - start.y, end.x - start.x);

    // Calculate the control points of the cubic bezier curve
    float c = radius * 0.552284749831;  // c = radius * (4/3) * tan(pi/8)

    Bezier bz;

    bz.start = start;
    bz.ctrl1 = Point{start.x + radius * cos(angle), start.y + radius * sin(angle)};
    bz.ctrl2 = {end.x - c * cos(angle), end.y - c * sin(angle)};
    bz.end = end;

    return bz;
}

Bezier bezFromArc(const Point& start, const Point& end, const Point& center)
{
    Point start_tangent{center.x - start.x, center.y - start.y};
    Point end_tangent{end.x - center.x, end.y - center.y};

    Point control1{start.x + (1.0f / 3.0f) * start_tangent.x, start.y + (1.0f / 3.0f) * start_tangent.y};
    Point control2{end.x - (1.0f / 3.0f) * end_tangent.x, end.y - (1.0f / 3.0f) * end_tangent.y};

    Bezier bz;
    bz.start = start;
    bz.ctrl1 = control1;
    bz.ctrl2 = control2;
    bz.end = end;

    return bz;   
}

}
