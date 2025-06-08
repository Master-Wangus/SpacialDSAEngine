#pragma once
#include "pch.h"
#include "Buffer.hpp"

enum class SideResult 
{
    eINSIDE      = -1,
    eOVERLAPPING = 0,
    eOUTSIDE     = 1
};

SideResult ClassifyPlaneAabb(glm::vec3 const& n, float d, Vertex const& min, Vertex const& max);
SideResult ClassifyPlaneSphere(glm::vec3 const& n, float d, Vertex const& c, float r);
SideResult ClassifyFrustumSphereNaive(glm::vec3 const fn[6], float const fd[6], Vertex const& c, float r);
SideResult ClassifyFrustumAabbNaive(glm::vec3 const fn[6], float const fd[6], Vertex const& min, Vertex const& max);
void TransformAabb(glm::vec3& min, glm::vec3& max, glm::mat4 const& transform);
void FrustumFromVp(glm::mat4 const& vp, glm::vec3 fn[6], float fd[6]);
void CreateAabbBruteForce(Vertex const* vertices, size_t count, Vertex* out_min, Vertex* out_max);
void CreateSphereRitters(Vertex const* vertices, size_t count, Vertex* out_c, float* out_r);
void CreateSphereIterative(Vertex const* vertices, size_t count, int iteration_count, float shrink_ratio, Vertex* out_c, float* out_r);
void CreateSpherePCA(Vertex const* vertices, size_t count, Vertex* out_c, float* out_r);
void CreateObbPCA(Vertex const* vertices, size_t count, glm::vec3* out_center, glm::vec3 out_axes[3], glm::vec3* out_halfExtents);

