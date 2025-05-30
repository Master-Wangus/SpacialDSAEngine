#include "Geometry.hpp"

    constexpr float kEpsilon = 1e-5f; // Custom epsilon for floating-point comparisons

    SideResult ClassifyPointAabb(glm::vec3 const& p, glm::vec3 const& min, glm::vec3 const& max)
    {
        if (p.x >= min.x && p.x <= max.x &&
            p.y >= min.y && p.y <= max.y &&
            p.z >= min.z && p.z <= max.z)
        {
            if (p.x > min.x && p.x < max.x &&
                p.y > min.y && p.y < max.y &&
                p.z > min.z && p.z < max.z)
            {
                return SideResult::eINSIDE;
            }
            return SideResult::eOVERLAPPING;
        }
        return SideResult::eOUTSIDE;
    }

    SideResult ClassifyPlanePoint(glm::vec3 const& n, float d, glm::vec3 const& p)
    {
        float distance = dot(n, p) - d;
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

    SideResult ClassifyPlaneTriangle(glm::vec3 const& n, float d, glm::vec3 const& A, glm::vec3 const& B, glm::vec3 const& C) 
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

    SideResult ClassifyPlaneAabb(glm::vec3 const& n, float d, glm::vec3 const& min, glm::vec3 const& max) 
    {
        glm::vec3 vertices[8] = 
        {
            {min.x, min.y, min.z},
            {max.x, min.y, min.z},
            {min.x, max.y, min.z},
            {max.x, max.y, min.z},
            {min.x, min.y, max.z},
            {max.x, min.y, max.z},
            {min.x, max.y, max.z},
            {max.x, max.y, max.z}
        };

        bool inside = false;
        bool outside = false;

        for (const glm::vec3& vertex : vertices)
        {
            SideResult side = ClassifyPlanePoint(n, d, vertex);
            if (side == SideResult::eINSIDE) 
            {
                inside = true;
            }
            else if (side == SideResult::eOUTSIDE) 
            {
                outside = true;
            }
            if (inside && outside) 
            {
                return SideResult::eOVERLAPPING;
            }
        }

        return inside ? SideResult::eINSIDE : SideResult::eOUTSIDE;
    }

    SideResult ClassifyPlaneSphere(glm::vec3 const& n, float d, glm::vec3 const& c, float r)
    {
        float distance = dot(n, c) - d;
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

    SideResult ClassifyFrustumSphereNaive(glm::vec3 const fn[6], float const fd[6], glm::vec3 const& c, float r) 
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

    SideResult ClassifyFrustumAabbNaive(glm::vec3 const fn[6], float const fd[6], glm::vec3 const& min, glm::vec3 const& max)
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

    bool OverlapPointAabb(glm::vec3 const& p, glm::vec3 const& min, glm::vec3 const& max)
    {
        return ClassifyPointAabb(p, min, max) != SideResult::eOUTSIDE;
    }

    bool OverlapPointSphere(glm::vec3 const& p, glm::vec3 const& c, float r)
    {
        glm::vec3 difference = p - c;
        return dot(difference, difference) <= r * r;
    }

    bool OverlapAabbAabb(glm::vec3 const& min1, glm::vec3 const& max1, glm::vec3 const& min2, glm::vec3 const& max2) 
    {
        return (min1.x <= max2.x && max1.x >= min2.x) &&
            (min1.y <= max2.y && max1.y >= min2.y) &&
            (min1.z <= max2.z && max1.z >= min2.z);
    }

    bool OverlapSphereSphere(glm::vec3 const& c1, float r1, glm::vec3 const& c2, float r2)
    {
        glm::vec3 difference = c1 - c2;
        float distanceSquared = dot(difference, difference);
        float radiusSum = r1 + r2;
        return distanceSquared <= radiusSum * radiusSum;
    }

    bool OverlapSegmentPlane(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& n, float d) 
    {
        float distanceStart = dot(n, s) - d;
        float distanceEnd = dot(n, e) - d;
        return (distanceStart * distanceEnd <= 0.0f);
    }

    bool OverlapSegmentTriangle(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& A, glm::vec3 const& B, glm::vec3 const& C)
    {
        glm::vec3 AB = B - A;
        glm::vec3 AC = C - A;
        glm::vec3 normal = cross(AB, AC);

        // Check if the segment is parallel to the triangle plane
        glm::vec3 segment = e - s;
        if (glm::abs(glm::dot(normal, segment)) < kEpsilon) {
            return false;
        }

        // Compute the intersection point
        float d = glm::dot(normal, A);
        float t = (d - glm::dot(normal, s)) / glm::dot(normal, segment);
        if (t < 0.0f || t > 1.0f) {
            return false;
        }

        glm::vec3 intersection = s + t * segment;

        // Check if the intersection point is inside the triangle
        glm::vec3 AP = intersection - A;
        float u = glm::dot(cross(AB, AP), normal);
        float v = glm::dot(cross(AP, AC), normal);

        return (u >= 0.0f && v >= 0.0f && u + v <= glm::dot(normal, normal));
    }

    bool OverlapSegmentAabb(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& min, glm::vec3 const& max) 
    {
        float tmin = 0.0f;
        float tmax = 1.0f;

        glm::vec3 segment = e - s;

        for (int i = 0; i < 3; ++i) 
        {
            if (abs(static_cast<float>(segment[i])) < kEpsilon) 
            {
                if (s[i] < min[i] || s[i] > max[i]) 
                {
                    return false;
                }
            }
            else {
                float invDir = 1.0f / segment[i];
                float t1 = (min[i] - s[i]) * invDir;
                float t2 = (max[i] - s[i]) * invDir;

                if (t1 > t2) {
                    std::swap(t1, t2);
                }

                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);

                if (tmin > tmax) 
                {
                    return false;
                }
            }
        }

        return true;
    }

    bool OverlapSegmentSphere(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& c, float r)
    {
        glm::vec3 segment = e - s;
        glm::vec3 centerToStart = s - c;

        float a = dot(segment, segment);
        float b = 2.0f * dot(centerToStart, segment);
        float cDiscrimnant = dot(centerToStart, centerToStart) - r * r;

        float discriminant = b * b - 4.0f * a * cDiscrimnant;
        if (discriminant < 0.0f) 
        {
            return false;
        }

        float sqrtDiscriminant = glm::sqrt(discriminant);
        float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
        float t2 = (-b + sqrtDiscriminant) / (2.0f * a);

        return (t1 >= 0.0f && t1 <= 1.0f) || (t2 >= 0.0f && t2 <= 1.0f);
    }

    float IntersectionTimeRayPlane(glm::vec3 const& s, glm::vec3 const& dir, glm::vec3 const& n, float d) 
    {
        float denominator = dot(n, dir);
        if (glm::abs(denominator) > kEpsilon)
        {
            float t = (d - dot(n, s)) / denominator;
            if (t >= 0.0f) 
            {
                return t;
            }
        }
        return  - 1.0f;
    }

    float IntersectionTimeRayTriangle(glm::vec3 const& s, glm::vec3 const& dir, glm::vec3 const& A, glm::vec3 const& B, glm::vec3 const& C) 
    {
        glm::vec3 AB = B - A;
        glm::vec3 AC = C - A;
        glm::vec3 normal = cross(AB, AC);

        float d = glm::dot(normal, A);
        float denominator = glm::dot(normal, dir);

        if (glm::abs(denominator) > kEpsilon) {
            float t = (d - glm::dot(normal, s)) / denominator;
            if (t >= 0.0f) {
                glm::vec3 intersection = s + t * dir;

                glm::vec3 AP = intersection - A;
                float u = glm::dot(cross(AB, AP), normal);
                float v = glm::dot(cross(AP, AC), normal);

                if (u >= 0.0f && v >= 0.0f && u + v <= glm::dot(normal, normal)) {
                    return t;
                }
            }
        }

        return -1.0f;
    }

    float IntersectionTimeRayAabb(glm::vec3 const& s, glm::vec3 const& dir, glm::vec3 const& min, glm::vec3 const& max) 
    {
        float tmin = 0.0f;
        float tmax = std::numeric_limits<float>::max();

        for (int i = 0; i < 3; ++i) 
        {
            if (glm::abs(dir[i]) < kEpsilon)
            {
                if (s[i] < min[i] || s[i] > max[i]) 
                {
                    return -1.0f;
                }
            }
            else 
            {
                float invDir = 1.0f / dir[i];
                float t1 = (min[i] - s[i]) * invDir;
                float t2 = (max[i] - s[i]) * invDir;

                if (t1 > t2) 
                {
                    std::swap(t1, t2);
                }

                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);

                if (tmin > tmax) 
                {
                    return -1.0f;
                }
            }
        }

        return tmin;
    }

    float IntersectionTimeRayObb(glm::vec3 const& s, glm::vec3 const& dir, glm::vec3 const& min, glm::vec3 const& max, glm::mat4 const& m2w)
    {
        glm::mat4 w2m = inverse(m2w);
        glm::vec3 localStart = TransformPoint(w2m, s);
        glm::vec3 localDir = TransformVector(w2m, dir);

        return IntersectionTimeRayAabb(localStart, localDir, min, max);
    }

    float IntersectionTimeRaySphere(glm::vec3 const& s, glm::vec3 const& dir, glm::vec3 const& c, float r)
    {

        if (OverlapPointSphere(s, c, r)) 
        {
            return 0.0f;
        }
        glm::vec3 sc = s - c; 


        float a = glm::dot(dir, dir); 
        float b = 2.0f * glm::dot(dir, sc); 
        float c_val = glm::dot(sc, sc) - r * r; 

        float discriminant = b * b - 4.0f * a * c_val;

        if (discriminant < 0.0f)
        {
            // No intersection
            return -1.0f;
        }
        else if (discriminant == 0.0f)
        {
            // Tangent to the sphere, one intersection point
            float t = -b / (2.0f * a);
            return t;
        }
        else
        {
            // Two points of intersection
            float sqrtDiscriminant = sqrt(discriminant);
            float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
            float t2 = (-b + sqrtDiscriminant) / (2.0f * a);

            // Return the smallest non-negative t value
            if (t1 >= 0.0f && t2 >= 0.0f)
            {
                return std::min(t1, t2);
            }
            else if (t1 >= 0.0f)
            {
                return t1;
            }
            else if (t2 >= 0.0f)
            {
                return t2;
            }
            else
            {
                // Both t values are negative, ray originates inside the sphere
                return -1.0f;
            }
        }
    }

    float IntersectionTimeSegmentPlane(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& n, float d) 
    {
        float tStart = dot(n, s) - d;
        float tEnd = dot(n, e) - d;

        if (tStart * tEnd >= 0.0f) 
        {
            return -1.0f;
        }

        return tStart / (tStart - tEnd);
    }

    float IntersectionTimeSegmentTriangle(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& A, glm::vec3 const& B, glm::vec3 const& C) 
    {
        glm::vec3 AB = B - A;
        glm::vec3 AC = C - A;
        glm::vec3 normal = glm::cross(AB, AC);

        float d = glm::dot(normal, A);
        glm::vec3 segment = e - s;
        float denominator = glm::dot(normal, segment);

        if (glm::abs(denominator) > kEpsilon)
        {
            float t = (d - glm::dot(normal, s)) / denominator;
            if (t >= 0.0f && t <= 1.0f) 
            {
                glm::vec3 intersection = s + t * segment;

                glm::vec3 AP = intersection - A;
                float u = glm::dot(cross(AB, AP), normal);
                float v = glm::dot(cross(AP, AC), normal);
                if (u >= 0.0f && v >= 0.0f && u + v <= glm::dot(normal, normal)) {
                    return t;
                }
            }
        }

        return -1.0f;
    }

    float IntersectionTimeSegmentAabb(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& min, glm::vec3 const& max)
    {
        float tmin = 0.0f;
        float tmax = 1.0f;

        glm::vec3 segment = e - s;

        for (int i = 0; i < 3; ++i) {
            if (glm::abs(segment[i]) < kEpsilon) {
                if (s[i] < min[i] || s[i] > max[i]) 
                {
                    return -1.0f;
                }
            }
            else {
                float invDir = 1.0f / segment[i];
                float t1 = (min[i] - s[i]) * invDir;
                float t2 = (max[i] - s[i]) * invDir;

                if (t1 > t2) {
                    std::swap(t1, t2);
                }

                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);

                if (tmin > tmax) 
                {
                    return -1.0f;
                }
            }
        }

        return tmin;
    }

    float IntersectionTimeSegmentSphere(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& c, float r) 
    {
        glm::vec3 segment = e - s;
        glm::vec3 centerToStart = s - c;

        float a = glm::dot(segment, segment);
        float b = 2.0f * dot(centerToStart, segment);
        float cDiscrimnant = dot(centerToStart, centerToStart) - r * r;

        float discriminant = b * b - 4.0f * a * cDiscrimnant;
        if (discriminant < 0.0f) 
        {
            return -1.0f;
        }

        float sqrtDiscriminant = glm::sqrt(discriminant);
        float t1 = (-b - sqrtDiscriminant) / (2.0f * a);
        float t2 = (-b + sqrtDiscriminant) / (2.0f * a);

        if (t1 >= 0.0f && t1 <= 1.0f) 
        {
            return t1;
        }
        else if (t2 >= 0.0f && t2 <= 1.0f) 
        {
            return t2;
        }

        return -1.0f;
    }

    bool AreCollinear(glm::vec3 const& a, glm::vec3 const& b, glm::vec3 const& c) 
    {
        glm::vec3 ab = b - a;
        glm::vec3 ac = c - a;
        glm::vec3 cross_product = cross(ab, ac);
        return length(cross_product) < kEpsilon;
    }

    float DistancePointPlane(glm::vec3 const& p, glm::vec3 const& n, float d) 
    {
        return dot(n, p) - d;
    }

    float DistanceSegmentPoint(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& pt) 
    {
        glm::vec3 segment = e - s;
        glm::vec3 startToPoint = pt - s;
        float t = dot(startToPoint, segment) / dot(segment, segment);
        t = glm::clamp(t, 0.0f, 1.0f);
        glm::vec3 projection = s + t * segment;
        return length(pt - projection);
    }

    glm::vec3 ClosestPointPlane(glm::vec3 const& p, glm::vec3 const& n, float d) {
        float distance = dot(n, p) - d;
        return p - distance * n;
    }

    void ClosestSegmentSegment(glm::vec3 const& s1, glm::vec3 const& e1, glm::vec3 const& s2, glm::vec3 const& e2, glm::vec3* p1, glm::vec3* p2) 
    {
        glm::vec3 d1 = e1 - s1; // Direction vector of segment S1
        glm::vec3 d2 = e2 - s2; // Direction vector of segment S2
        glm::vec3 r = s1 - s2;
        float a = dot(d1,d1); // Squared length of segment S1
        float e = dot(d2,d2); // Squared length of segment S2
        float f = dot(d2,r);
        float c = dot(d1, r);
        float b = dot(d1, d2);
        float denom = a * e - b * b; // Always nonnegative

        float s{};
        float t{};

        bool degenerateS1 = a <= kEpsilon;
        bool degenerateS2 = e <= kEpsilon;

        if (degenerateS1 && degenerateS2) 
        {
            *p1 = s1;
            *p2 = s2;
            if (glm::length(r) <= kEpsilon) 
            {
                *p1 = *p2 = s1;
            }
            return;
        }

        if (degenerateS1) 
        {
            t = glm::clamp(f / e, 0.0f, 1.0f);
        }
        else if (degenerateS2) {
            s = glm::clamp(-c / a, 0.0f, 1.0f);
        }
        else 
        {
            if (glm::abs(denom) > kEpsilon)
            {
                s = glm::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
            }
            t = (b * s + f) / e;
        }

        // If t in [0,1] done. Else clamp t, recompute s for the new value
        // of t using s = Dot((P2 + D2*t) - P1,D1) / Dot(D1,D1)= (t*b - c) / a
        // and clamp s to [0, 1]
        if (t < 0.0f) 
        {
            t = 0.0f;
            s = glm::clamp(-c / a, 0.0f, 1.0f);
        }
        else if (t > 1.0f) 
        {
            t = 1.0f;
            s = glm::clamp((b - c) / a, 0.0f, 1.0f);
        }

        *p1 = s1 + s * d1;
        *p2 = s2 + t * d2;

        if (glm::length(*p1 - *p2) <= kEpsilon && glm::length(s1 - e2) <= kEpsilon && glm::length(e1 - s2) <= kEpsilon) 
        {
            *p1 = e1;
            *p2 = s2;
        }
    }

    glm::vec3 IntersectionPlanePlanePlane(glm::vec3 const& n1, float d1, glm::vec3 const& n2, float d2, glm::vec3 const& n3, float d3) 
    {
        glm::mat3 A(n1, n2, n3);
        glm::vec3 b(d1, d2, d3);
        return glm::inverse(A) * b;
    }

    glm::vec3 TransformPoint(glm::mat4 const& transform, glm::vec3 const& point)
    {
        glm::vec4 homogeneous_point(point, 1.0f);
        glm::vec4 transformed_point = transform * homogeneous_point;
        return glm::vec3(transformed_point) / transformed_point.w;
    }

    glm::vec3 TransformVector(glm::mat4 const& m, glm::vec3 const& v)
    {
        glm::vec4 homogeneous_vector(v, 0.0f);
        glm::vec4 transformed_vector = m * homogeneous_vector;
        return glm::vec3(transformed_vector);
    }

    void TransformAabb(glm::vec3 const& min, glm::vec3 const& max, glm::mat4 const& transform, glm::vec3* out_min, glm::vec3* out_max) 
    {
        glm::vec3 vertices[8] = 
        {
            {min.x, min.y, min.z},
            {max.x, min.y, min.z},
            {min.x, max.y, min.z},
            {max.x, max.y, min.z},
            {min.x, min.y, max.z},
            {max.x, min.y, max.z},
            {min.x, max.y, max.z},
            {max.x, max.y, max.z}
        };

        *out_min = TransformPoint(transform, vertices[0]);
        *out_max = *out_min;

        for (const glm::vec3& vertex : vertices) 
        {
            glm::vec3 transformedVertex = TransformPoint(transform, vertex);
            *out_min = glm::min(*out_min, transformedVertex);
            *out_max = glm::max(*out_max, transformedVertex);
        }
    }

    void FrustumFromVp(glm::mat4 const& vp, glm::vec3 fn[6], float fd[6]) 
    {
        fn[0] = glm::vec3(vp[0][3] + vp[0][0], vp[1][3] + vp[1][0], vp[2][3] + vp[2][0]); // Left plane
        fn[1] = glm::vec3(vp[0][3] - vp[0][0], vp[1][3] - vp[1][0], vp[2][3] - vp[2][0]); // Right plane
        fn[2] = glm::vec3(vp[0][3] + vp[0][1], vp[1][3] + vp[1][1], vp[2][3] + vp[2][1]); // Bottom plane
        fn[3] = glm::vec3(vp[0][3] - vp[0][1], vp[1][3] - vp[1][1], vp[2][3] - vp[2][1]); // Top plane
        fn[4] = glm::vec3(vp[0][3] + vp[0][2], vp[1][3] + vp[1][2], vp[2][3] + vp[2][2]); // Near plane
        fn[5] = glm::vec3(vp[0][3] - vp[0][2], vp[1][3] - vp[1][2], vp[2][3] - vp[2][2]); // Far plane

        fd[0] = vp[3][3] + vp[3][0];
        fd[1] = vp[3][3] - vp[3][0];
        fd[2] = vp[3][3] + vp[3][1];
        fd[3] = vp[3][3] - vp[3][1];
        fd[4] = vp[3][3] + vp[3][2];
        fd[5] = vp[3][3] - vp[3][2];

        for (int i = 0; i < 6; ++i) 
        {
            float invLength = 1.0f / length(fn[i]);
            fn[i] *= invLength;
            fd[i] *= invLength;
        }
    }

    void CreateAabbBruteForce(glm::vec3 const* vertices, size_t count, glm::vec3* out_min, glm::vec3* out_max) {
        *out_min = vertices[0];
        *out_max = vertices[0];

        for (size_t i = 1; i < count; ++i) 
        {
            *out_min = min(*out_min, vertices[i]);
            *out_max = max(*out_max, vertices[i]);
        }
    }

    void CreateSphereCentroid(glm::vec3 const* vertices, size_t count, glm::vec3* out_c, float* out_r) {
        glm::vec3 centroid(0.0f);
        for (size_t i = 0; i < count; ++i) 
        {
            centroid += vertices[i];
        }
        centroid /= static_cast<float>(count);

        float maxDistanceSquared = 0.0f;
        for (size_t i = 0; i < count; ++i) {
            float distanceSquared = glm::length2(vertices[i] - centroid);
            maxDistanceSquared = std::max(maxDistanceSquared, distanceSquared);
        }

        *out_c = centroid;
        *out_r = glm::sqrt(maxDistanceSquared);
    }

    void CreateSphereRitters(glm::vec3 const* vertices, size_t count, glm::vec3* out_c, float* out_r) {
        glm::vec3 min_point = vertices[0];
        glm::vec3 max_point = vertices[0];

        for (size_t i = 1; i < count; ++i) {
            min_point = min(min_point, vertices[i]);
            max_point = max(max_point, vertices[i]);
        }

        glm::vec3 center = (min_point + max_point) * 0.5f;
        float radius = length(max_point - center);

        for (size_t i = 0; i < count; ++i) {
            float distance = length(vertices[i] - center);
            if (distance > radius) {
                glm::vec3 direction = normalize(vertices[i] - center);
                glm::vec3 new_center = (center + vertices[i] - direction * radius) * 0.5f;
                float new_radius = length(vertices[i] - new_center);
                center = new_center;
                radius = new_radius;
            }
        }

        *out_c = center;
        *out_r = radius;
    }

    void CreateSphereIterative(glm::vec3 const* vertices, size_t count, int iteration_count, float shrink_ratio, glm::vec3* out_c, float* out_r) {
        glm::vec3 center(0.0f);
        float radius = 0.0f;

        // Find the initial bounding sphere using Ritter's method
        CreateSphereRitters(vertices, count, &center, &radius);

        for (int iteration = 0; iteration < iteration_count; ++iteration) {
            // Find the vertex furthest from the current center
            glm::vec3 furthest_vertex = vertices[0];
            float max_distance = glm::length2(vertices[0] - center);

            for (size_t i = 1; i < count; ++i) {
                float distance = length2(vertices[i] - center);
                if (distance > max_distance) {
                    max_distance = distance;
                    furthest_vertex = vertices[i];
                }
            }

            // Adjust the sphere's radius and center based on the furthest vertex
            float new_radius = glm::sqrt(max_distance);
            glm::vec3 direction = glm::normalize(furthest_vertex - center);
            glm::vec3 new_center = center + direction * (new_radius - radius) * shrink_ratio;

            center = new_center;
            radius = new_radius;
        }

        *out_c = center;
        *out_r = radius;
    }

    void CreateSpherePCA(glm::vec3 const* vertices, size_t count, glm::vec3* out_c, float* out_r) {
        // Calculate centroid
        glm::vec3 centroid(0.0f);
        for (size_t i = 0; i < count; ++i) {
            centroid += vertices[i];
        }
        centroid /= static_cast<float>(count);

        // Calculate covariance matrix
        glm::mat3 covariance(0.0f);
        for (size_t i = 0; i < count; ++i) {
            glm::vec3 diff = vertices[i] - centroid;
            covariance[0][0] += diff.x * diff.x;
            covariance[0][1] += diff.x * diff.y;
            covariance[0][2] += diff.x * diff.z;
            covariance[1][0] += diff.y * diff.x;
            covariance[1][1] += diff.y * diff.y;
            covariance[1][2] += diff.y * diff.z;
            covariance[2][0] += diff.z * diff.x;
            covariance[2][1] += diff.z * diff.y;
            covariance[2][2] += diff.z * diff.z;
        }
        covariance /= static_cast<float>(count - 1);

        // For simplicity, we'll use the largest eigenvalue direction
        // In a full implementation, you'd want to compute all eigenvalues/eigenvectors
        // Here we'll approximate by finding the direction of maximum variance
        glm::vec3 eigenVector(1.0f, 0.0f, 0.0f);
        
        // Power iteration to find dominant eigenvector (simplified)
        for (int iter = 0; iter < 10; ++iter) {
            glm::vec3 newVector = covariance * eigenVector;
            float length = glm::length(newVector);
            if (length > kEpsilon) {
                eigenVector = newVector / length;
            }
        }

        // Project all points onto the dominant eigenvector to find extremes
        float minProj = glm::dot(vertices[0] - centroid, eigenVector);
        float maxProj = minProj;
        
        for (size_t i = 1; i < count; ++i) {
            float proj = glm::dot(vertices[i] - centroid, eigenVector);
            minProj = std::min(minProj, proj);
            maxProj = std::max(maxProj, proj);
        }

        // Calculate sphere center and radius based on PCA
        glm::vec3 center = centroid + eigenVector * (minProj + maxProj) * 0.5f;
        float radius = (maxProj - minProj) * 0.5f;

        // Ensure all points are within the sphere
        for (size_t i = 0; i < count; ++i) {
            float distance = glm::length(vertices[i] - center);
            radius = std::max(radius, distance);
        }

        *out_c = center;
        *out_r = radius;
    }

    bool OverlapSphereAabb(glm::vec3 const& sphere_center, float sphere_radius, glm::vec3 const& aabb_min, glm::vec3 const& aabb_max) {
        // Find the closest point on the AABB to the sphere center
        glm::vec3 closest_point;
        closest_point.x = glm::clamp(sphere_center.x, aabb_min.x, aabb_max.x);
        closest_point.y = glm::clamp(sphere_center.y, aabb_min.y, aabb_max.y);
        closest_point.z = glm::clamp(sphere_center.z, aabb_min.z, aabb_max.z);

        // Calculate the distance from the sphere center to this closest point
        glm::vec3 distance_vec = sphere_center - closest_point;
        float distance_squared = glm::dot(distance_vec, distance_vec);

        // Check if the distance is within the sphere's radius
        return distance_squared <= (sphere_radius * sphere_radius);
    }


