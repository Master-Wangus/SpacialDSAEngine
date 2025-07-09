#pragma once
#include "pch.h"
#include "Buffer.hpp"

enum class SideResult 
{
    eINSIDE      = -1,
    eOVERLAPPING = 0,
    eOUTSIDE     = 1
};

/**
 * @brief Classifies an AABB against a plane.
 * @param n Plane normal vector
 * @param d Plane distance from origin
 * @param min Minimum vertex of the AABB
 * @param max Maximum vertex of the AABB
 * @return Classification result (inside, outside, or overlapping)
 */
SideResult ClassifyPlaneAabb(glm::vec3 const& n, float d, Vertex const& min, Vertex const& max);

/**
 * @brief Classifies a sphere against a plane.
 * @param n Plane normal vector
 * @param d Plane distance from origin
 * @param c Center vertex of the sphere
 * @param r Radius of the sphere
 * @return Classification result (inside, outside, or overlapping)
 */
SideResult ClassifyPlaneSphere(glm::vec3 const& n, float d, Vertex const& c, float r);

/**
 * @brief Classifies a sphere against a frustum using naive approach.
 * @param fn Array of 6 frustum plane normals
 * @param fd Array of 6 frustum plane distances
 * @param c Center vertex of the sphere
 * @param r Radius of the sphere
 * @return Classification result (inside, outside, or overlapping)
 */
SideResult ClassifyFrustumSphereNaive(glm::vec3 const fn[6], float const fd[6], Vertex const& c, float r);

/**
 * @brief Classifies an AABB against a frustum using naive approach.
 * @param fn Array of 6 frustum plane normals
 * @param fd Array of 6 frustum plane distances
 * @param min Minimum vertex of the AABB
 * @param max Maximum vertex of the AABB
 * @return Classification result (inside, outside, or overlapping)
 */
SideResult ClassifyFrustumAabbNaive(glm::vec3 const fn[6], float const fd[6], Vertex const& min, Vertex const& max);

/**
 * @brief Transforms an AABB by a transformation matrix.
 * @param min Minimum point of the AABB (input/output)
 * @param max Maximum point of the AABB (input/output)
 * @param transform Transformation matrix to apply
 */
void TransformAabb(glm::vec3& min, glm::vec3& max, glm::mat4 const& transform);

/**
 * @brief Extracts frustum planes from a view-projection matrix.
 * @param vp View-projection matrix
 * @param fn Output array for 6 frustum plane normals
 * @param fd Output array for 6 frustum plane distances
 */
void FrustumFromVp(glm::mat4 const& vp, glm::vec3 fn[6], float fd[6]);

/**
 * @brief Creates an AABB using brute force approach.
 * @param vertices Array of vertices to process
 * @param count Number of vertices
 * @param out_min Output minimum vertex
 * @param out_max Output maximum vertex
 */
void CreateAabbBruteForce(Vertex const* vertices, size_t count, Vertex* out_min, Vertex* out_max);

/**
 * @brief Creates a bounding sphere using Principal Component Analysis.
 * @param vertices Array of vertices to process
 * @param count Number of vertices
 * @param out_c Output center vertex
 * @param out_r Output radius
 */
void CreateSpherePCA(Vertex const* vertices, size_t count, Vertex* out_c, float* out_r);

/**
 * @brief Creates an oriented bounding box using Principal Component Analysis.
 * @param vertices Array of vertices to process
 * @param count Number of vertices
 * @param out_center Output center position
 * @param out_axes Output array of 3 orientation axes
 * @param out_halfExtents Output half-extents along each axis
 */
void CreateObbPCA(Vertex const* vertices, size_t count, glm::vec3* out_center, glm::vec3 out_axes[3], glm::vec3* out_halfExtents);

