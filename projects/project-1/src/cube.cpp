#include <cube.hpp>

/**
 * @brief Constructor for Cube class.
 */
Cube::Cube()
    : programID(0), vertexbuffer(0), VertexArrayID(0) {
    geometryBuffer.clear();
}

/**
 * @brief Destructor for Cube class.
 * Calls clean-up routine to release OpenGL resources.
 */
Cube::~Cube() {
    cleanUp();
}

/**
 * @brief Releases GPU resources used by this cube instance.
 */
void Cube::cleanUp() {
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID);
}

/**
 * @brief Sets up vertex buffers and configures vertex attribute pointers.
 *
 * The layout used here:
 * - Position: 3 floats
 * - Color: 3 floats
 */
void Cube::SetupBuffers() {
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER,
        geometryBuffer.size() * sizeof(GLfloat),
        geometryBuffer.data(),
        GL_STATIC_DRAW);

    // Vertex position attribute (x, y, z)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);

    // Vertex color attribute (r, g, b)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
}

/**
 * @brief Initializes shader program and cube geometry.
 * Loads shaders and populates vertex buffer data for a colored cube.
 */
void Cube::initialize() {
    // Load vertex and fragment shaders
    programID = LoadShaders(
        "../projects/project-1/shaders/ColorVertexShader.vert",
        "../projects/project-1/shaders/ColorFragmentShader.frag");

    // Define 36 vertices (12 triangles) with positions and colors
    geometryBuffer = {
        // Back face
        -0.5f, -0.5f, -0.5f, 1, 0, 0,
         0.5f, -0.5f, -0.5f, 0, 1, 0,
         0.5f,  0.5f, -0.5f, 0, 0, 1,
         0.5f,  0.5f, -0.5f, 0, 0, 1,
        -0.5f,  0.5f, -0.5f, 1, 1, 0,
        -0.5f, -0.5f, -0.5f, 1, 0, 0,

        // Front face
        -0.5f, -0.5f,  0.5f, 0, 1, 1,
         0.5f, -0.5f,  0.5f, 1, 0, 1,
         0.5f,  0.5f,  0.5f, 1, 1, 1,
         0.5f,  0.5f,  0.5f, 1, 1, 1,
        -0.5f,  0.5f,  0.5f, 0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f,  0.5f, 0, 1, 1,

        // Left face
        -0.5f,  0.5f,  0.5f, 1, 0, 0,
        -0.5f,  0.5f, -0.5f, 0, 1, 0,
        -0.5f, -0.5f, -0.5f, 0, 0, 1,
        -0.5f, -0.5f, -0.5f, 0, 0, 1,
        -0.5f, -0.5f,  0.5f, 1, 1, 0,
        -0.5f,  0.5f,  0.5f, 1, 0, 0,

        // Right face
         0.5f,  0.5f,  0.5f, 0, 1, 1,
         0.5f,  0.5f, -0.5f, 1, 0, 1,
         0.5f, -0.5f, -0.5f, 1, 1, 1,
         0.5f, -0.5f, -0.5f, 1, 1, 1,
         0.5f, -0.5f,  0.5f, 0.5f, 0.5f, 0.5f,
         0.5f,  0.5f,  0.5f, 0, 1, 1,

         // Bottom face
         -0.5f, -0.5f, -0.5f, 1, 0, 0,
          0.5f, -0.5f, -0.5f, 0, 1, 0,
          0.5f, -0.5f,  0.5f, 0, 0, 1,
          0.5f, -0.5f,  0.5f, 0, 0, 1,
         -0.5f, -0.5f,  0.5f, 1, 1, 0,
         -0.5f, -0.5f, -0.5f, 1, 0, 0,

         // Top face
         -0.5f,  0.5f, -0.5f, 0, 1, 1,
          0.5f,  0.5f, -0.5f, 1, 0, 1,
          0.5f,  0.5f,  0.5f, 1, 1, 1,
          0.5f,  0.5f,  0.5f, 1, 1, 1,
         -0.5f,  0.5f,  0.5f, 0.5f, 0.5f, 0.5f,
         -0.5f,  0.5f, -0.5f, 0, 1, 1
    };

    SetupBuffers();
}

/**
 * @brief Renders the cube to the screen.
 * Applies rotation and draws the geometry using the active shader.
 */
void Cube::render(const glm::mat4& modelMat) {
    glUseProgram(programID); // Use shader program
    glBindVertexArray(VertexArrayID); // Bind cube VAO

    // Pass transform to shader
    GLint vTransformLoc = glGetUniformLocation(programID, "vertexTransform");
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &modelMat[0][0]);

    // Draw the cube using 36 vertices (12 triangles)
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

