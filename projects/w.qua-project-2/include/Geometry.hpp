#pragma once
#include "pch.h"
#include "Buffer.hpp"

enum class SideResult 
{
    eINSIDE      = -1,
    eOVERLAPPING = 0,
    eOUTSIDE     = 1
};

// New functions using Vertex struct
SideResult ClassifyPointAabb(Vertex const& p, Vertex const& min, Vertex const& max);
SideResult ClassifyPlanePoint(glm::vec3 const& n, float d, Vertex const& p);
SideResult ClassifyPlaneTriangle(glm::vec3 const& n, float d, Vertex const& A, Vertex const& B, Vertex const& C);
SideResult ClassifyPlaneAabb(glm::vec3 const& n, float d, Vertex const& min, Vertex const& max);
SideResult ClassifyPlaneSphere(glm::vec3 const& n, float d, Vertex const& c, float r);
SideResult ClassifyFrustumSphereNaive(glm::vec3 const fn[6], float const fd[6], Vertex const& c, float r);
SideResult ClassifyFrustumAabbNaive(glm::vec3 const fn[6], float const fd[6], Vertex const& min, Vertex const& max);
bool OverlapPointAabb(Vertex const& p, Vertex const& min, Vertex const& max);
bool OverlapPointSphere(Vertex const& p, Vertex const& c, float r);
bool OverlapAabbAabb(Vertex const& min1, Vertex const& max1, Vertex const& min2, Vertex const& max2);
bool OverlapSphereSphere(Vertex const& c1, float r1, Vertex const& c2, float r2);
bool OverlapSegmentPlane(Vertex const& s, Vertex const& e, glm::vec3 const& n, float d);
bool OverlapSegmentTriangle(Vertex const& s, Vertex const& e, Vertex const& A, Vertex const& B, Vertex const& C);
bool OverlapSegmentAabb(Vertex const& s, Vertex const& e, Vertex const& min, Vertex const& max);
bool OverlapSegmentSphere(Vertex const& s, Vertex const& e, Vertex const& c, float r);
float IntersectionTimeRayPlane(Vertex const& s, glm::vec3 const& dir, glm::vec3 const& n, float d);
float IntersectionTimeRayTriangle(Vertex const& s, glm::vec3 const& dir, Vertex const& A, Vertex const& B, Vertex const& C);
float IntersectionTimeRayAabb(Vertex const& s, glm::vec3 const& dir, Vertex const& min, Vertex const& max);
float IntersectionTimeRayObb(Vertex const& s, glm::vec3 const& dir, Vertex const& min, Vertex const& max, glm::mat4 const& m2w);
float IntersectionTimeSegmentPlane(Vertex const& s, Vertex const& e, glm::vec3 const& n, float d);
float IntersectionTimeSegmentTriangle(Vertex const& s, Vertex const& e, Vertex const& A, Vertex const& B, Vertex const& C);
float IntersectionTimeSegmentAabb(Vertex const& s, Vertex const& e, Vertex const& min, Vertex const& max);
float IntersectionTimeSegmentSphere(Vertex const& s, Vertex const& e, Vertex const& c, float r);
bool AreCollinear(Vertex const& a, Vertex const& b, Vertex const& c);
float DistancePointPlane(Vertex const& p, glm::vec3 const& n, float d);
float DistanceSegmentPoint(Vertex const& s, Vertex const& e, Vertex const& pt);
glm::vec3 ClosestPointPlane(Vertex const& p, glm::vec3 const& n, float d);
void ClosestSegmentSegment(Vertex const& s1, Vertex const& e1, Vertex const& s2, Vertex const& e2, Vertex* p1, Vertex* p2);
glm::vec3 IntersectionPlanePlanePlane(glm::vec3 const& n1, float d1, glm::vec3 const& n2, float d2, glm::vec3 const& n3, float d3);
Vertex TransformPoint(glm::mat4 const& transform, Vertex const& point);
glm::vec3 TransformVector(glm::mat4 const& m, glm::vec3 const& v);
void TransformAabb(glm::vec3& min, glm::vec3& max, glm::mat4 const& transform);
void FrustumFromVp(glm::mat4 const& vp, glm::vec3 fn[6], float fd[6]);
void CreateAabbBruteForce(Vertex const* vertices, size_t count, Vertex* out_min, Vertex* out_max);
void CreateSphereCentroid(Vertex const* vertices, size_t count, Vertex* out_c, float* out_r);
void CreateSphereRitters(Vertex const* vertices, size_t count, Vertex* out_c, float* out_r);
void CreateSphereIterative(Vertex const* vertices, size_t count, int iteration_count, float shrink_ratio, Vertex* out_c, float* out_r);
void CreateSpherePCA(Vertex const* vertices, size_t count, Vertex* out_c, float* out_r);
void CreateObbPCA(Vertex const* vertices, size_t count, glm::vec3* out_center, glm::vec3 out_axes[3], glm::vec3* out_halfExtents);

// Sphere-AABB overlap test
bool OverlapSphereAabb(Vertex const& sphere_center, float sphere_radius, Vertex const& aabb_min, Vertex const& aabb_max);

