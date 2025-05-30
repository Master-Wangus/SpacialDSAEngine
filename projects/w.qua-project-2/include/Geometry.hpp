#pragma once
#include "pch.h"

enum class SideResult 
{
    eINSIDE      = -1,
    eOVERLAPPING = 0,
    eOUTSIDE     = 1
};

SideResult ClassifyPointAabb(glm::vec3 const& p, glm::vec3 const& min, glm::vec3 const& max);
SideResult ClassifyPlanePoint(glm::vec3 const& n, float d, glm::vec3 const& p);
SideResult ClassifyPlaneTriangle(glm::vec3 const& n, float d, glm::vec3 const& A, glm::vec3 const& B, glm::vec3 const& C);
SideResult ClassifyPlaneAabb(glm::vec3 const& n, float d, glm::vec3 const& min, glm::vec3 const& max);
SideResult ClassifyPlaneSphere(glm::vec3 const& n, float d, glm::vec3 const& c, float r);
SideResult ClassifyFrustumSphereNaive(glm::vec3 const fn[6], float const fd[6], glm::vec3 const& c, float r);
SideResult ClassifyFrustumAabbNaive(glm::vec3 const fn[6], float const fd[6], glm::vec3 const& min, glm::vec3 const& max);
bool OverlapPointAabb(glm::vec3 const& p, glm::vec3 const& min, glm::vec3 const& max);
bool OverlapPointSphere(glm::vec3 const& p, glm::vec3 const& c, float r);
bool OverlapAabbAabb(glm::vec3 const& min1, glm::vec3 const& max1, glm::vec3 const& min2, glm::vec3 const& max2);
bool OverlapSphereSphere(glm::vec3 const& c1, float r1, glm::vec3 const& c2, float r2);
bool OverlapSegmentPlane(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& n, float d);
bool OverlapSegmentTriangle(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& A, glm::vec3 const& B, glm::vec3 const& C);
bool OverlapSegmentAabb(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& min, glm::vec3 const& max);
bool OverlapSegmentSphere(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& c, float r);
float IntersectionTimeRayPlane(glm::vec3 const& s, glm::vec3 const& dir, glm::vec3 const& n, float d);
float IntersectionTimeRayTriangle(glm::vec3 const& s, glm::vec3 const& dir, glm::vec3 const& A, glm::vec3 const& B, glm::vec3 const& C);
float IntersectionTimeRayAabb(glm::vec3 const& s, glm::vec3 const& dir, glm::vec3 const& min, glm::vec3 const& max);
float IntersectionTimeRayObb(glm::vec3 const& s, glm::vec3 const& dir, glm::vec3 const& min, glm::vec3 const& max, glm::mat4 const& m2w);
float IntersectionTimeSegmentPlane(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& n, float d);
float IntersectionTimeSegmentTriangle(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& A, glm::vec3 const& B, glm::vec3 const& C);
float IntersectionTimeSegmentAabb(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& min, glm::vec3 const& max);
float IntersectionTimeSegmentSphere(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& c, float r);
bool AreCollinear(glm::vec3 const& a, glm::vec3 const& b, glm::vec3 const& c);
float DistancePointPlane(glm::vec3 const& p, glm::vec3 const& n, float d);
float DistanceSegmentPoint(glm::vec3 const& s, glm::vec3 const& e, glm::vec3 const& pt);
glm::vec3 ClosestPointPlane(glm::vec3 const& p, glm::vec3 const& n, float d);
void ClosestSegmentSegment(glm::vec3 const& s1, glm::vec3 const& e1, glm::vec3 const& s2, glm::vec3 const& e2, glm::vec3* p1, glm::vec3* p2);
glm::vec3 IntersectionPlanePlanePlane(glm::vec3 const& n1, float d1, glm::vec3 const& n2, float d2, glm::vec3 const& n3, float d3);
glm::vec3 TransformPoint(glm::mat4 const& transform, glm::vec3 const& point);
glm::vec3 TransformVector(glm::mat4 const& m, glm::vec3 const& v);
void TransformAabb(glm::vec3 const& min, glm::vec3 const& max, glm::mat4 const& transform, glm::vec3* out_min, glm::vec3* out_max);
void FrustumFromVp(glm::mat4 const& vp, glm::vec3 fn[6], float fd[6]);
void CreateAabbBruteForce(glm::vec3 const* vertices, size_t count, glm::vec3* out_min, glm::vec3* out_max);
void CreateSphereCentroid(glm::vec3 const* vertices, size_t count, glm::vec3* out_c, float* out_r);
void CreateSphereRitters(glm::vec3 const* vertices, size_t count, glm::vec3* out_c, float* out_r);
void CreateSphereIterative(glm::vec3 const* vertices, size_t count, int iteration_count, float shrink_ratio, glm::vec3* out_c, float* out_r);
void CreateSpherePCA(glm::vec3 const* vertices, size_t count, glm::vec3* out_c, float* out_r);

// Sphere-AABB overlap test
bool OverlapSphereAabb(glm::vec3 const& sphere_center, float sphere_radius, glm::vec3 const& aabb_min, glm::vec3 const& aabb_max);

