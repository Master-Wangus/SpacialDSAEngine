/**
 * @file shader.cpp
 * @brief Helper functions to load, compile, and link GLSL shaders (vertex, fragment, geometry).
 */

#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <cstring>

#include <GL/glew.h>
#include <shader.hpp>

#if USE_CSD3151_AUTOMATION == 1
 // This automation hook reads the shader from the submission tutorial's shader directory as a string literal.
 // It requires an automation script to convert the shader files from file format to string literal format.
 // After conversion, the file names must be changed to my-shader.vert and my-shader.frag.
std::string const assignment_vs = {
  #include "../shaders/my-shader.vert"
};
std::string const assignment_fs = {
  #include "../shaders/my-shader.frag"
};
#endif

 //////////////////////////////////////////////////////////////////////////

 /**
  * @brief Loads and compiles vertex and fragment shaders from files, then links them into a shader program.
  *
  * @param vertex_file_path Path to the vertex shader source file.
  * @param fragment_file_path Path to the fragment shader source file.
  * @return GLuint Program ID of the compiled and linked shader program.
  */
GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // --- Read Vertex Shader ---
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::string Line;
        while (getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    else {
        printf("Unable to open %s. Are you in the correct directory?\n", vertex_file_path);
        getchar();
        return 0;
    }

    // --- Read Fragment Shader ---
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::string Line;
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // --- Compile Vertex Shader ---
    printf("Compiling shader: %s\n", vertex_file_path);

#if USE_CSD3151_AUTOMATION == 1
    const char* VertexSourcePointer = assignment_vs.c_str();
#else
    const char* VertexSourcePointer = VertexShaderCode.c_str();
#endif
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
    glCompileShader(VertexShaderID);

    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ErrorMsg(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &ErrorMsg[0]);
        printf("%s\n", &ErrorMsg[0]);
    }

    // --- Compile Fragment Shader ---
    printf("Compiling shader: %s\n", fragment_file_path);
#if USE_CSD3151_AUTOMATION == 1
    const char* FragmentSourcePointer = assignment_fs.c_str();
#else
    const char* FragmentSourcePointer = FragmentShaderCode.c_str();
#endif
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
    glCompileShader(FragmentShaderID);

    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ErrorMsg(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &ErrorMsg[0]);
        printf("%s\n", &ErrorMsg[0]);
    }

    // --- Link Program ---
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramError(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramError[0]);
        printf("%s\n", &ProgramError[0]);
    }

    // Clean up
    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

//////////////////////////////////////////////////////////////////////////

/**
 * @brief Loads shaders and sets up a separable shader pipeline (useful for modern OpenGL).
 *
 * @param vertex_file_path Path to vertex shader.
 * @param fragment_file_path Path to fragment shader.
 * @param programIDs Output array to hold created program IDs (index 0: vertex, index 1: fragment).
 * @return GLuint ID of created program pipeline object.
 */
GLuint LoadPipeline(const char* vertex_file_path, const char* fragment_file_path, GLuint* programIDs)
{
    // --- Vertex Shader ---
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::string Line;
        while (getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    else {
        printf("Unable to open %s\n", vertex_file_path);
        getchar();
        return 0;
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    printf("Compiling shader: %s\n", vertex_file_path);
#if USE_CSD3151_AUTOMATION == 1
    const char* VertexSourcePointer = assignment_vs.c_str();
#else
    const char* VertexSourcePointer = VertexShaderCode.c_str();
#endif

    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
    glCompileShader(VertexShaderID);

    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ErrorMsg(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &ErrorMsg[0]);
        printf("%s\n", &ErrorMsg[0]);
    }

    programIDs[0] = glCreateProgram();
    glAttachShader(programIDs[0], VertexShaderID);
    glProgramParameteri(programIDs[0], GL_PROGRAM_SEPARABLE, GL_TRUE);
    glLinkProgram(programIDs[0]);

    // Check program link
    glGetProgramiv(programIDs[0], GL_LINK_STATUS, &Result);
    glGetProgramiv(programIDs[0], GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ErrorMsg(InfoLogLength + 1);
        glGetProgramInfoLog(programIDs[0], InfoLogLength, nullptr, &ErrorMsg[0]);
        printf("%s\n", &ErrorMsg[0]);
    }

    // --- Fragment Shader ---
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::string Line;
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    printf("Compiling shader: %s\n", fragment_file_path);
#if USE_CSD3151_AUTOMATION == 1
    const char* FragmentSourcePointer = assignment_fs.c_str();
#else
    const char* FragmentSourcePointer = FragmentShaderCode.c_str();
#endif
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
    glCompileShader(FragmentShaderID);

    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ErrorMsg(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &ErrorMsg[0]);
        printf("%s\n", &ErrorMsg[0]);
    }

    programIDs[1] = glCreateProgram();
    glAttachShader(programIDs[1], FragmentShaderID);
    glProgramParameteri(programIDs[1], GL_PROGRAM_SEPARABLE, GL_TRUE);
    glLinkProgram(programIDs[1]);

    GLuint pipeLineID;
    glGenProgramPipelines(1, &pipeLineID);

    glUseProgramStages(pipeLineID, GL_VERTEX_SHADER_BIT, programIDs[0]);
    glUseProgramStages(pipeLineID, GL_FRAGMENT_SHADER_BIT, programIDs[1]);

    return pipeLineID;
}