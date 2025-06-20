#include "Geometry.hpp"
#include <Eigen/Dense>

constexpr float kEpsilon = 1e-5f; // Custom epsilon for floating-point comparisons



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
    for (int i = 0; i < 6; ++i) 
    {
        SideResult side = ClassifyPlaneAabb(fn[i], fd[i], min, max);
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

    // Write results back
    min = newMin;
    max = newMax;
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
    for (int i = 0; i < 6; ++i) 
    {
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



void CreateSphereRitters(Vertex const* vertices, size_t count, Vertex* out_c, float* out_r)
{
    if (count == 0 || !vertices || !out_c || !out_r) return;
    
    // Optimized approach: instead of O(n²) search, use approximate method
    // Find extremal points along each axis (much faster)
    size_t minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0;
    
    for (size_t i = 1; i < count; ++i) 
    {
        const glm::vec3& pos = vertices[i].m_Position;
        
        if (pos.x < vertices[minX].m_Position.x) minX = i;
        if (pos.x > vertices[maxX].m_Position.x) maxX = i;
        if (pos.y < vertices[minY].m_Position.y) minY = i;
        if (pos.y > vertices[maxY].m_Position.y) maxY = i;
        if (pos.z < vertices[minZ].m_Position.z) minZ = i;
        if (pos.z > vertices[maxZ].m_Position.z) maxZ = i;
    }
    
    // Find the pair with maximum distance among the extremal points
    size_t maxI = minX, maxJ = maxX;
    float maxDistanceSq = glm::length2(vertices[maxX].m_Position - vertices[minX].m_Position);
    
    float distY = glm::length2(vertices[maxY].m_Position - vertices[minY].m_Position);
    if (distY > maxDistanceSq) {
        maxDistanceSq = distY;
        maxI = minY;
        maxJ = maxY;
    }
    
    float distZ = glm::length2(vertices[maxZ].m_Position - vertices[minZ].m_Position);
    if (distZ > maxDistanceSq) {
        maxDistanceSq = distZ;
        maxI = minZ;
        maxJ = maxZ;
    }
    
    // Initial sphere from most distant extremal pair
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


