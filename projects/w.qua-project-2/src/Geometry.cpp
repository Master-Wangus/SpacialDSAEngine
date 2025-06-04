#include "Geometry.hpp"
#include <Eigen/Dense>

constexpr float kEpsilon = 1e-5f; // Custom epsilon for floating-point comparisons

SideResult ClassifyPointAabb(Vertex const& p, Vertex const& min, Vertex const& max)
{
    const glm::vec3& pos = p.m_Position;
    const glm::vec3& minPos = min.m_Position;
    const glm::vec3& maxPos = max.m_Position;
    
    if (pos.x >= minPos.x && pos.x <= maxPos.x &&
        pos.y >= minPos.y && pos.y <= maxPos.y &&
        pos.z >= minPos.z && pos.z <= maxPos.z)
    {
        if (pos.x > minPos.x && pos.x < maxPos.x &&
            pos.y > minPos.y && pos.y < maxPos.y &&
            pos.z > minPos.z && pos.z < maxPos.z)
        {
            return SideResult::eINSIDE;
        }
        return SideResult::eOVERLAPPING;
    }
    return SideResult::eOUTSIDE;
}

SideResult ClassifyPlanePoint(glm::vec3 const& n, float d, Vertex const& p)
{
    float distance = dot(n, p.m_Position) - d;
    if (distance > kEpsilon) 
    {
        return SideResult::eOUTSIDE;
    }
    else if (distance < -kEpsilon) 
    {
        return SideResult::eINSIDE;
    }
    return SideResult::eOVERLAPPING;
}

SideResult ClassifyPlaneTriangle(glm::vec3 const& n, float d, Vertex const& A, Vertex const& B, Vertex const& C) 
{
    SideResult sideA = ClassifyPlanePoint(n, d, A);
    SideResult sideB = ClassifyPlanePoint(n, d, B);
    SideResult sideC = ClassifyPlanePoint(n, d, C);

    if (sideA == SideResult::eINSIDE && sideB == SideResult::eINSIDE && sideC == SideResult::eINSIDE) {
        return SideResult::eINSIDE;
    }
    else if (sideA == SideResult::eOUTSIDE && sideB == SideResult::eOUTSIDE && sideC == SideResult::eOUTSIDE) {
        return SideResult::eOUTSIDE;
    }
    return SideResult::eOVERLAPPING;
}

SideResult ClassifyPlaneAabb(glm::vec3 const& n, float d, Vertex const& min, Vertex const& max) 
{
    const glm::vec3& minPos = min.m_Position;
    const glm::vec3& maxPos = max.m_Position;
    
    glm::vec3 vertices[8] = 
    {
        {minPos.x, minPos.y, minPos.z},
        {maxPos.x, minPos.y, minPos.z},
        {minPos.x, maxPos.y, minPos.z},
        {maxPos.x, maxPos.y, minPos.z},
        {minPos.x, minPos.y, maxPos.z},
        {maxPos.x, minPos.y, maxPos.z},
        {minPos.x, maxPos.y, maxPos.z},
        {maxPos.x, maxPos.y, maxPos.z}
    };

    bool inside = false;
    bool outside = false;

    for (const glm::vec3& vertex : vertices)
    {
        float distance = dot(n, vertex) - d;
        if (distance > kEpsilon) 
        {
            outside = true;
        }
        else if (distance < -kEpsilon) 
        {
            inside = true;
        }
        if (inside && outside) 
        {
            return SideResult::eOVERLAPPING;
        }
    }

    return inside ? SideResult::eINSIDE : SideResult::eOUTSIDE;
}

SideResult ClassifyPlaneSphere(glm::vec3 const& n, float d, Vertex const& c, float r)
{
    float distance = dot(n, c.m_Position) - d;
    if (distance > r) 
    {
        return SideResult::eOUTSIDE;
    }
    else if (distance < -r)
    {
        return SideResult::eINSIDE;
    }
    return SideResult::eOVERLAPPING;
}

SideResult ClassifyFrustumSphereNaive(glm::vec3 const fn[6], float const fd[6], Vertex const& c, float r) 
{
    bool inside = true;
    for (int i = 0; i < 6; ++i) 
    {
        SideResult side = ClassifyPlaneSphere(fn[i], fd[i], c, r);
        if (side == SideResult::eOUTSIDE)
        {
            return SideResult::eOUTSIDE;
        }
        else if (side == SideResult::eOVERLAPPING) 
        {
            inside = false;
        }
    }
    return inside ? SideResult::eINSIDE : SideResult::eOVERLAPPING;
}

SideResult ClassifyFrustumAabbNaive(glm::vec3 const fn[6], float const fd[6], Vertex const& min, Vertex const& max)
{
    bool inside = true;
    for (int i = 0; i < 6; ++i) {
        SideResult side = ClassifyPlaneAabb(fn[i], fd[i], min, max);
        if (side == SideResult::eOUTSIDE) {
            return SideResult::eOUTSIDE;
        }
        else if (side == SideResult::eOVERLAPPING) {
            inside = false;
        }
    }
    return inside ? SideResult::eINSIDE : SideResult::eOVERLAPPING;
}

bool OverlapPointAabb(Vertex const& p, Vertex const& min, Vertex const& max)
{
    return ClassifyPointAabb(p, min, max) != SideResult::eOUTSIDE;
}

bool OverlapPointSphere(Vertex const& p, Vertex const& c, float r)
{
    glm::vec3 difference = p.m_Position - c.m_Position;
    return dot(difference, difference) <= r * r;
}

bool OverlapAabbAabb(Vertex const& min1, Vertex const& max1, Vertex const& min2, Vertex const& max2) 
{
    const glm::vec3& min1Pos = min1.m_Position;
    const glm::vec3& max1Pos = max1.m_Position;
    const glm::vec3& min2Pos = min2.m_Position;
    const glm::vec3& max2Pos = max2.m_Position;
    
    return (min1Pos.x <= max2Pos.x && max1Pos.x >= min2Pos.x) &&
           (min1Pos.y <= max2Pos.y && max1Pos.y >= min2Pos.y) &&
           (min1Pos.z <= max2Pos.z && max1Pos.z >= min2Pos.z);
}

bool OverlapSphereSphere(Vertex const& c1, float r1, Vertex const& c2, float r2)
{
    glm::vec3 difference = c1.m_Position - c2.m_Position;
    float distanceSquared = dot(difference, difference);
    float radiusSum = r1 + r2;
    return distanceSquared <= radiusSum * radiusSum;
}

bool OverlapSegmentPlane(Vertex const& s, Vertex const& e, glm::vec3 const& n, float d) 
{
    float distanceStart = dot(n, s.m_Position) - d;
    float distanceEnd = dot(n, e.m_Position) - d;
    return (distanceStart * distanceEnd <= 0.0f);
}

bool OverlapSegmentTriangle(Vertex const& s, Vertex const& e, Vertex const& A, Vertex const& B, Vertex const& C)
{
    const glm::vec3& APos = A.m_Position;
    const glm::vec3& BPos = B.m_Position;
    const glm::vec3& CPos = C.m_Position;
    const glm::vec3& sPos = s.m_Position;
    const glm::vec3& ePos = e.m_Position;
    
    glm::vec3 AB = BPos - APos;
    glm::vec3 AC = CPos - APos;
    glm::vec3 normal = cross(AB, AC);

    // Check if the segment is parallel to the triangle plane
    glm::vec3 segment = ePos - sPos;
    if (glm::abs(glm::dot(normal, segment)) < kEpsilon) {
        return false;
    }

    // Compute the intersection point
    float d = glm::dot(normal, APos);
    float t = (d - glm::dot(normal, sPos)) / glm::dot(normal, segment);
    if (t < 0.0f || t > 1.0f) {
        return false;
    }

    glm::vec3 intersection = sPos + t * segment;

    // Check if the intersection point is inside the triangle
    glm::vec3 AP = intersection - APos;
    float u = glm::dot(cross(AB, AP), normal);
    float v = glm::dot(cross(AP, AC), normal);

    return (u >= 0.0f && v >= 0.0f && u + v <= glm::dot(normal, normal));
}

bool OverlapSegmentAabb(Vertex const& s, Vertex const& e, Vertex const& min, Vertex const& max) 
{
    const glm::vec3& sPos = s.m_Position;
    const glm::vec3& ePos = e.m_Position;
    const glm::vec3& minPos = min.m_Position;
    const glm::vec3& maxPos = max.m_Position;
    
    float tmin = 0.0f;
    float tmax = 1.0f;

    glm::vec3 dir = ePos - sPos;

    for (int i = 0; i < 3; ++i)
    {
        if (glm::abs(dir[i]) < kEpsilon)
        {
            // Segment is parallel to slab
            if (sPos[i] < minPos[i] || sPos[i] > maxPos[i])
                return false;
        }
        else
        {
            float invDir = 1.0f / dir[i];
            float t1 = (minPos[i] - sPos[i]) * invDir;
            float t2 = (maxPos[i] - sPos[i]) * invDir;

            if (t1 > t2) std::swap(t1, t2);

            tmin = glm::max(tmin, t1);
            tmax = glm::min(tmax, t2);

            if (tmin > tmax) return false;
        }
    }

    return tmin <= tmax;
}

bool OverlapSegmentSphere(Vertex const& s, Vertex const& e, Vertex const& c, float r)
{
    const glm::vec3& sPos = s.m_Position;
    const glm::vec3& ePos = e.m_Position;
    const glm::vec3& cPos = c.m_Position;
    
    glm::vec3 segment = ePos - sPos;
    glm::vec3 toSphere = cPos - sPos;
    
    float segmentLength = glm::length(segment);
    if (segmentLength < kEpsilon) {
        return glm::length(toSphere) <= r;
    }
    
    glm::vec3 segmentNorm = segment / segmentLength;
    float projection = glm::dot(toSphere, segmentNorm);
    
    projection = glm::clamp(projection, 0.0f, segmentLength);
    glm::vec3 closestPoint = sPos + projection * segmentNorm;
    
    return glm::length(cPos - closestPoint) <= r;
}

float IntersectionTimeRayPlane(Vertex const& s, glm::vec3 const& dir, glm::vec3 const& n, float d)
{
    float denominator = glm::dot(n, dir);
    if (glm::abs(denominator) < kEpsilon) {
        return -1.0f; // Ray is parallel to plane
    }
    
    float t = (d - glm::dot(n, s.m_Position)) / denominator;
    return (t >= 0.0f) ? t : -1.0f;
}

float IntersectionTimeRayTriangle(Vertex const& s, glm::vec3 const& dir, Vertex const& A, Vertex const& B, Vertex const& C)
{
    const glm::vec3& APos = A.m_Position;
    const glm::vec3& BPos = B.m_Position;
    const glm::vec3& CPos = C.m_Position;
    const glm::vec3& sPos = s.m_Position;
    
    glm::vec3 AB = BPos - APos;
    glm::vec3 AC = CPos - APos;
    glm::vec3 normal = cross(AB, AC);
    
    float denominator = glm::dot(normal, dir);
    if (glm::abs(denominator) < kEpsilon) {
        return -1.0f; // Ray is parallel to triangle
    }
    
    float d = glm::dot(normal, APos);
    float t = (d - glm::dot(normal, sPos)) / denominator;
    if (t < 0.0f) {
        return -1.0f; // Intersection behind ray origin
    }
    
    glm::vec3 intersection = sPos + t * dir;
    
    // Check if intersection point is inside triangle using barycentric coordinates
    glm::vec3 AP = intersection - APos;
    float u = glm::dot(cross(AB, AP), normal);
    float v = glm::dot(cross(AP, AC), normal);
    float normalSq = glm::dot(normal, normal);
    
    return (u >= 0.0f && v >= 0.0f && u + v <= normalSq) ? t : -1.0f;
}

float IntersectionTimeRayAabb(Vertex const& s, glm::vec3 const& dir, Vertex const& min, Vertex const& max)
{
    const glm::vec3& sPos = s.m_Position;
    const glm::vec3& minPos = min.m_Position;
    const glm::vec3& maxPos = max.m_Position;
    
    float tmin = 0.0f;
    float tmax = std::numeric_limits<float>::max();

    for (int i = 0; i < 3; ++i)
    {
        if (glm::abs(dir[i]) < kEpsilon)
        {
            if (sPos[i] < minPos[i] || sPos[i] > maxPos[i])
                return -1.0f;
        }
        else
        {
            float invDir = 1.0f / dir[i];
            float t1 = (minPos[i] - sPos[i]) * invDir;
            float t2 = (maxPos[i] - sPos[i]) * invDir;

            if (t1 > t2) std::swap(t1, t2);

            tmin = glm::max(tmin, t1);
            tmax = glm::min(tmax, t2);

            if (tmin > tmax) return -1.0f;
        }
    }

    return (tmin >= 0.0f) ? tmin : -1.0f;
}

float IntersectionTimeRayObb(Vertex const& s, glm::vec3 const& dir, Vertex const& min, Vertex const& max, glm::mat4 const& m2w)
{
    // Transform ray to OBB local space
    glm::mat4 w2m = glm::inverse(m2w);
    glm::vec3 localOrigin = glm::vec3(w2m * glm::vec4(s.m_Position, 1.0f));
    glm::vec3 localDir = glm::vec3(w2m * glm::vec4(dir, 0.0f));
    
    // Create temporary vertices for AABB test in local space
    Vertex localS, localMin, localMax;
    localS.m_Position = localOrigin;
    localMin.m_Position = min.m_Position;
    localMax.m_Position = max.m_Position;
    
    return IntersectionTimeRayAabb(localS, localDir, localMin, localMax);
}

float IntersectionTimeSegmentPlane(Vertex const& s, Vertex const& e, glm::vec3 const& n, float d)
{
    const glm::vec3& sPos = s.m_Position;
    const glm::vec3& ePos = e.m_Position;
    
    glm::vec3 segment = ePos - sPos;
    float denominator = glm::dot(n, segment);
    
    if (glm::abs(denominator) < kEpsilon) {
        return -1.0f; // Segment is parallel to plane
    }
    
    float t = (d - glm::dot(n, sPos)) / denominator;
    return (t >= 0.0f && t <= 1.0f) ? t : -1.0f;
}

float IntersectionTimeSegmentTriangle(Vertex const& s, Vertex const& e, Vertex const& A, Vertex const& B, Vertex const& C)
{
    const glm::vec3& sPos = s.m_Position;
    const glm::vec3& ePos = e.m_Position;
    
    glm::vec3 segment = ePos - sPos;
    float t = IntersectionTimeRayTriangle(s, segment, A, B, C);
    
    return (t >= 0.0f && t <= 1.0f) ? t : -1.0f;
}

float IntersectionTimeSegmentAabb(Vertex const& s, Vertex const& e, Vertex const& min, Vertex const& max)
{
    const glm::vec3& sPos = s.m_Position;
    const glm::vec3& ePos = e.m_Position;
    
    glm::vec3 segment = ePos - sPos;
    float t = IntersectionTimeRayAabb(s, segment, min, max);
    
    return (t >= 0.0f && t <= 1.0f) ? t : -1.0f;
}

float IntersectionTimeSegmentSphere(Vertex const& s, Vertex const& e, Vertex const& c, float r)
{
    const glm::vec3& sPos = s.m_Position;
    const glm::vec3& ePos = e.m_Position;
    const glm::vec3& cPos = c.m_Position;
    
    glm::vec3 segment = ePos - sPos;
    glm::vec3 toSphere = sPos - cPos;
    
    float a = glm::dot(segment, segment);
    float b = 2.0f * glm::dot(toSphere, segment);
    float c_coeff = glm::dot(toSphere, toSphere) - r * r;
    
    float discriminant = b * b - 4.0f * a * c_coeff;
    if (discriminant < 0.0f) {
        return -1.0f; // No intersection
    }
    
    float sqrtDisc = glm::sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);
    
    if (t1 >= 0.0f && t1 <= 1.0f) return t1;
    if (t2 >= 0.0f && t2 <= 1.0f) return t2;
    
    return -1.0f;
}

bool AreCollinear(Vertex const& a, Vertex const& b, Vertex const& c)
{
    glm::vec3 ab = b.m_Position - a.m_Position;
    glm::vec3 ac = c.m_Position - a.m_Position;
    glm::vec3 crossProduct = cross(ab, ac);
    return glm::length(crossProduct) < kEpsilon;
}

float DistancePointPlane(Vertex const& p, glm::vec3 const& n, float d)
{
    return glm::abs(glm::dot(n, p.m_Position) - d);
}

float DistanceSegmentPoint(Vertex const& s, Vertex const& e, Vertex const& pt)
{
    const glm::vec3& sPos = s.m_Position;
    const glm::vec3& ePos = e.m_Position;
    const glm::vec3& ptPos = pt.m_Position;
    
    glm::vec3 segment = ePos - sPos;
    glm::vec3 toPoint = ptPos - sPos;
    
    float segmentLengthSq = glm::dot(segment, segment);
    if (segmentLengthSq < kEpsilon) {
        return glm::length(toPoint);
    }
    
    float t = glm::clamp(glm::dot(toPoint, segment) / segmentLengthSq, 0.0f, 1.0f);
    glm::vec3 closestPoint = sPos + t * segment;
    
    return glm::length(ptPos - closestPoint);
}

glm::vec3 ClosestPointPlane(Vertex const& p, glm::vec3 const& n, float d)
{
    float distance = glm::dot(n, p.m_Position) - d;
    return p.m_Position - distance * n;
}

void ClosestSegmentSegment(Vertex const& s1, Vertex const& e1, Vertex const& s2, Vertex const& e2, Vertex* p1, Vertex* p2)
{
    const glm::vec3& s1Pos = s1.m_Position;
    const glm::vec3& e1Pos = e1.m_Position;
    const glm::vec3& s2Pos = s2.m_Position;
    const glm::vec3& e2Pos = e2.m_Position;
    
    glm::vec3 d1 = e1Pos - s1Pos;
    glm::vec3 d2 = e2Pos - s2Pos;
    glm::vec3 r = s1Pos - s2Pos;
    
    float a = glm::dot(d1, d1);
    float e = glm::dot(d2, d2);
    float f = glm::dot(d2, r);
    
    float s = 0.0f, t = 0.0f;
    
    if (a <= kEpsilon && e <= kEpsilon) {
        // Both segments are points
        s = t = 0.0f;
    } else if (a <= kEpsilon) {
        // First segment is a point
        s = 0.0f;
        t = glm::clamp(f / e, 0.0f, 1.0f);
    } else {
        float c = glm::dot(d1, r);
        if (e <= kEpsilon) {
            // Second segment is a point
            t = 0.0f;
            s = glm::clamp(-c / a, 0.0f, 1.0f);
        } else {
            // General case
            float b = glm::dot(d1, d2);
            float denom = a * e - b * b;
            
            if (denom != 0.0f) {
                s = glm::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
            } else {
                s = 0.0f;
            }
            
            t = (b * s + f) / e;
            
            if (t < 0.0f) {
                t = 0.0f;
                s = glm::clamp(-c / a, 0.0f, 1.0f);
            } else if (t > 1.0f) {
                t = 1.0f;
                s = glm::clamp((b - c) / a, 0.0f, 1.0f);
            }
        }
    }
    
    if (p1) {
        p1->m_Position = s1Pos + s * d1;
        p1->m_Color = s1.m_Color;
        p1->m_Normal = s1.m_Normal;
        p1->m_UV = s1.m_UV;
    }
    
    if (p2) {
        p2->m_Position = s2Pos + t * d2;
        p2->m_Color = s2.m_Color;
        p2->m_Normal = s2.m_Normal;
        p2->m_UV = s2.m_UV;
    }
}

glm::vec3 IntersectionPlanePlanePlane(glm::vec3 const& n1, float d1, glm::vec3 const& n2, float d2, glm::vec3 const& n3, float d3)
{
    glm::mat3 matrix(n1, n2, n3);
    glm::vec3 rhs(d1, d2, d3);
    
    float det = glm::determinant(matrix);
    if (glm::abs(det) < kEpsilon) {
        return glm::vec3(0.0f); // Planes don't intersect at a single point
    }
    
    return glm::inverse(matrix) * rhs;
}

Vertex TransformPoint(glm::mat4 const& transform, Vertex const& point)
{
    Vertex result = point;
    glm::vec4 transformedPos = transform * glm::vec4(point.m_Position, 1.0f);
    result.m_Position = glm::vec3(transformedPos);
    
    // Transform normal (use inverse transpose for proper normal transformation)
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
    result.m_Normal = glm::normalize(normalMatrix * point.m_Normal);
    
    return result;
}

glm::vec3 TransformVector(glm::mat4 const& m, glm::vec3 const& v)
{
    return glm::vec3(m * glm::vec4(v, 0.0f));
}

void TransformAabb(glm::vec3& min, glm::vec3& max, glm::mat4 const& transform)
{
    const glm::vec3 minPos = min;
    const glm::vec3 maxPos = max;
    
    // Generate all 8 corners of the AABB
    glm::vec3 corners[8] = {
        {minPos.x, minPos.y, minPos.z},
        {maxPos.x, minPos.y, minPos.z},
        {minPos.x, maxPos.y, minPos.z},
        {maxPos.x, maxPos.y, minPos.z},
        {minPos.x, minPos.y, maxPos.z},
        {maxPos.x, minPos.y, maxPos.z},
        {minPos.x, maxPos.y, maxPos.z},
        {maxPos.x, maxPos.y, maxPos.z}
    };
    
    // Transform first corner to initialize min/max
    glm::vec4 transformedCorner = transform * glm::vec4(corners[0], 1.0f);
    glm::vec3 newMin = glm::vec3(transformedCorner);
    glm::vec3 newMax = newMin;
    
    // Transform remaining corners and update min/max
    for (int i = 1; i < 8; ++i) {
        transformedCorner = transform * glm::vec4(corners[i], 1.0f);
        glm::vec3 corner = glm::vec3(transformedCorner);
        
        newMin = glm::min(newMin, corner);
        newMax = glm::max(newMax, corner);
    }
}

void FrustumFromVp(glm::mat4 const& vp, glm::vec3 fn[6], float fd[6])
{
    // Extract frustum planes from view-projection matrix
    // Left plane
    fn[0] = glm::vec3(vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0]);
    fd[0] = vp[3][3] + vp[3][0];
    
    // Right plane
    fn[1] = glm::vec3(vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0]);
    fd[1] = vp[3][3] - vp[3][0];
    
    // Bottom plane
    fn[2] = glm::vec3(vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1]);
    fd[2] = vp[3][3] + vp[3][1];
    
    // Top plane
    fn[3] = glm::vec3(vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1]);
    fd[3] = vp[3][3] - vp[3][1];
    
    // Near plane
    fn[4] = glm::vec3(vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2]);
    fd[4] = vp[3][3] + vp[3][2];
    
    // Far plane
    fn[5] = glm::vec3(vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2]);
    fd[5] = vp[3][3] - vp[3][2];
    
    // Normalize planes
    for (int i = 0; i < 6; ++i) {
        float length = glm::length(fn[i]);
        if (length > kEpsilon) {
            fn[i] /= length;
            fd[i] /= length;
        }
    }
}

void CreateAabbBruteForce(Vertex const* vertices, size_t count, Vertex* out_min, Vertex* out_max)
{
    if (count == 0 || !vertices || !out_min || !out_max) return;
    
    glm::vec3 minPos = vertices[0].m_Position;
    glm::vec3 maxPos = vertices[0].m_Position;
    
    for (size_t i = 1; i < count; ++i)
     {
        minPos = glm::min(minPos, vertices[i].m_Position);
        maxPos = glm::max(maxPos, vertices[i].m_Position);
    }
    
    out_min->m_Position = minPos;
    out_min->m_Color = vertices[0].m_Color;
    out_min->m_Normal = vertices[0].m_Normal;
    out_min->m_UV = vertices[0].m_UV;
    
    out_max->m_Position = maxPos;
    out_max->m_Color = vertices[0].m_Color;
    out_max->m_Normal = vertices[0].m_Normal;
    out_max->m_UV = vertices[0].m_UV;
}

void CreateSphereCentroid(Vertex const* vertices, size_t count, Vertex* out_c, float* out_r)
{
    if (count == 0 || !vertices || !out_c || !out_r) return;
    
    // Calculate centroid
    glm::vec3 centroid(0.0f);
    for (size_t i = 0; i < count; ++i) 
    {
        centroid += vertices[i].m_Position;
    }
    centroid /= static_cast<float>(count);
    
    // Find maximum distance from centroid
    float maxDistanceSq = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        float distanceSq = glm::length2(vertices[i].m_Position - centroid);
        maxDistanceSq = glm::max(maxDistanceSq, distanceSq);
    }
    
    out_c->m_Position = centroid;
    out_c->m_Color = vertices[0].m_Color;
    out_c->m_Normal = vertices[0].m_Normal;
    out_c->m_UV = vertices[0].m_UV;
    *out_r = glm::sqrt(maxDistanceSq);
}

void CreateSphereRitters(Vertex const* vertices, size_t count, Vertex* out_c, float* out_r)
{
    if (count == 0 || !vertices || !out_c || !out_r) return;
    
    // Find the most distant pair of points
    size_t maxI = 0, maxJ = 1;
    float maxDistanceSq = 0.0f;
    
    for (size_t i = 0; i < count; ++i) 
    {
        for (size_t j = i + 1; j < count; ++j) 
        {
            float distanceSq = glm::length2(vertices[i].m_Position - vertices[j].m_Position);
            if (distanceSq > maxDistanceSq) 
            {
                maxDistanceSq = distanceSq;
                maxI = i;
                maxJ = j;
            }
        }
    }
    
    // Initial sphere from most distant pair
    glm::vec3 center = (vertices[maxI].m_Position + vertices[maxJ].m_Position) * 0.5f;
    float radius = glm::length(vertices[maxI].m_Position - vertices[maxJ].m_Position) * 0.5f;
    
    // Expand sphere to include all points
    for (size_t i = 0; i < count; ++i) 
    {
        glm::vec3 toPoint = vertices[i].m_Position - center;
        float distance = glm::length(toPoint);
        
        if (distance > radius) 
        {
            float newRadius = (radius + distance) * 0.5f;
            glm::vec3 direction = toPoint / distance;
            center = center + direction * (newRadius - radius);
            radius = newRadius;
        }
    }
    
    out_c->m_Position = center;
    out_c->m_Color = vertices[0].m_Color;
    out_c->m_Normal = vertices[0].m_Normal;
    out_c->m_UV = vertices[0].m_UV;
    *out_r = radius;
}

void CreateSphereIterative(Vertex const* vertices, size_t count, int iteration_count, float shrink_ratio, Vertex* out_c, float* out_r)
{
    if (count == 0 || !vertices || !out_c || !out_r) return;
    
    // Start with Ritter's algorithm
    CreateSphereRitters(vertices, count, out_c, out_r);
    
    glm::vec3 center = out_c->m_Position;
    float radius = *out_r;
    
    // Iterative improvement
    for (int iter = 0; iter < iteration_count; ++iter) 
    {
        glm::vec3 newCenter(0.0f);
        float totalWeight = 0.0f;
        
        // Weighted centroid of points outside current sphere
        for (size_t i = 0; i < count; ++i) 
        {
            float distance = glm::length(vertices[i].m_Position - center);
            if (distance > radius) 
            {
                float weight = distance - radius;
                newCenter += vertices[i].m_Position * weight;
                totalWeight += weight;
            }
        }
        
        if (totalWeight > kEpsilon) 
        {
            newCenter /= totalWeight;
            center = glm::mix(center, newCenter, shrink_ratio);
        }
        
        // Recalculate radius
        radius = 0.0f;
        for (size_t i = 0; i < count; ++i) 
        {
            float distance = glm::length(vertices[i].m_Position - center);
            radius = glm::max(radius, distance);
        }
    }
    
    out_c->m_Position = center;
    *out_r = radius;
}

void CreateSpherePCA(Vertex const* vertices, size_t count, Vertex* out_c, float* out_r)
{
    if (count == 0 || !vertices || !out_c || !out_r) return;
    
    // Create Eigen matrix from vertex data
    Eigen::MatrixXf pointMatrix(count, 3);
    for (size_t i = 0; i < count; ++i) {
        pointMatrix(i, 0) = vertices[i].m_Position.x;
        pointMatrix(i, 1) = vertices[i].m_Position.y;
        pointMatrix(i, 2) = vertices[i].m_Position.z;
    }
    
    // Step 1: Compute Centroid
    Eigen::Vector3f centroid = pointMatrix.colwise().mean();
    
    // Step 2: Center the data
    Eigen::MatrixXf centered = pointMatrix.rowwise() - centroid.transpose();
    
    // Step 3: Compute Covariance Matrix
    Eigen::Matrix3f covariance = (centered.adjoint() * centered) / float(count);
    
    // Step 4: Eigen Decomposition
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> eigensolver(covariance);
    Eigen::Vector3f eigenvalues = eigensolver.eigenvalues();
    Eigen::Matrix3f eigenvectors = eigensolver.eigenvectors();
    
    // Step 5: Radius Estimation
    // Find the extents along each principal axis
    Eigen::Vector3f minExtents = Eigen::Vector3f::Constant(std::numeric_limits<float>::max());
    Eigen::Vector3f maxExtents = Eigen::Vector3f::Constant(-std::numeric_limits<float>::max());
    
    for (size_t i = 0; i < count; ++i) {
        // Project centered point onto each eigenvector (principal axis)
        Eigen::Vector3f point;
        point << centered(i, 0), centered(i, 1), centered(i, 2);
        Eigen::Vector3f projections = eigenvectors.transpose() * point;
        
        minExtents = minExtents.cwiseMin(projections);
        maxExtents = maxExtents.cwiseMax(projections);
    }
    
    // Calculate the optimal sphere center in principal component space
    Eigen::Vector3f centerInPCA = (minExtents + maxExtents) * 0.5f;
    
    // Transform back to world space
    Eigen::Vector3f optimalCenter = centroid + eigenvectors * centerInPCA;
    
    // Calculate radius - farthest point defines the radius
    float radius = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        Eigen::Vector3f pos;
        pos << vertices[i].m_Position.x, vertices[i].m_Position.y, vertices[i].m_Position.z;
        float distance = (pos - optimalCenter).norm();
        radius = std::max(radius, distance);
    }
    
    out_c->m_Position = glm::vec3(optimalCenter(0), optimalCenter(1), optimalCenter(2));
    out_c->m_Color = vertices[0].m_Color;
    out_c->m_Normal = vertices[0].m_Normal;
    out_c->m_UV = vertices[0].m_UV;
    *out_r = radius;
}

bool OverlapSphereAabb(Vertex const& sphere_center, float sphere_radius, Vertex const& aabb_min, Vertex const& aabb_max)
{
    const glm::vec3& center = sphere_center.m_Position;
    const glm::vec3& minPos = aabb_min.m_Position;
    const glm::vec3& maxPos = aabb_max.m_Position;
    
    glm::vec3 closestPoint = glm::clamp(center, minPos, maxPos);
    glm::vec3 difference = center - closestPoint;
    
    return glm::dot(difference, difference) <= sphere_radius * sphere_radius;
}

void CreateObbPCA(Vertex const* vertices, size_t count, glm::vec3* out_center, glm::vec3 out_axes[3], glm::vec3* out_halfExtents)
{
    if (count == 0 || !vertices || !out_center || !out_axes || !out_halfExtents) return;
    
    // Step 1: Convert vertices to a matrix for computation
    Eigen::MatrixXf points(count, 3);
    for (size_t i = 0; i < count; ++i) {
        points(i, 0) = vertices[i].m_Position.x;
        points(i, 1) = vertices[i].m_Position.y;
        points(i, 2) = vertices[i].m_Position.z;
    }
    
    // Step 2: Compute the centroid (mean position)
    Eigen::Vector3f centroid = points.colwise().mean();
    
    // Step 3: Center the data by subtracting the centroid
    Eigen::MatrixXf centered = points.rowwise() - centroid.transpose();
    
    // Step 4: Compute the covariance matrix
    Eigen::Matrix3f covariance = (centered.adjoint() * centered) / float(count);
    
    // Step 5: Compute eigenvalues and eigenvectors of the covariance matrix
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(covariance);
    
    Eigen::Matrix3f eigenVectors = solver.eigenvectors();
        
    // Step 6: Set OBB axes (principal directions)
    for (int i = 0; i < 3; ++i) 
    {
        Eigen::Vector3f axis = eigenVectors.col(i);
        out_axes[i] = glm::normalize(glm::vec3(axis(0), axis(1), axis(2)));
    }
        
    // Step 7: Rotate points into the PCA-aligned frame
    Eigen::MatrixXf rotated = centered * eigenVectors;
        
    // Step 8: Compute extents in the PCA-aligned space
    Eigen::Vector3f minExtents = rotated.colwise().minCoeff();
    Eigen::Vector3f maxExtents = rotated.colwise().maxCoeff();
    Eigen::Vector3f halfExtents = (maxExtents - minExtents) * 0.5f;
        
    // Step 9: Set half-extents
    *out_halfExtents = glm::vec3(halfExtents(0), halfExtents(1), halfExtents(2));
        
    // Step 10: Compute OBB center in world space
    Eigen::Vector3f centerOffset = eigenVectors * (minExtents + halfExtents);
    Eigen::Vector3f obbCenter = centroid + centerOffset;
    *out_center = glm::vec3(obbCenter(0), obbCenter(1), obbCenter(2));
    
}


