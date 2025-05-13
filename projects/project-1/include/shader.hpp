#pragma once

#include <GL/glew.h>

/**
 * @brief Loads and compiles vertex and fragment shaders from given file paths, links them into a shader program.
 *
 * @param vertex_file_path Path to the vertex shader source file.
 * @param fragment_file_path Path to the fragment shader source file.
 * @return GLuint ID of the created shader program.
 */
GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path);

/**
 * @brief Loads separable vertex and fragment shaders and sets up a program pipeline.
 *
 * @param vertex_file_path Path to the vertex shader source file.
 * @param fragment_file_path Path to the fragment shader source file.
 * @param programIDs Output array to store vertex and fragment shader program IDs (size must be 2).
 * @return GLuint ID of the OpenGL pipeline object.
 */
GLuint LoadPipeline(const char* vertex_file_path, const char* fragment_file_path, GLuint* programIDs);