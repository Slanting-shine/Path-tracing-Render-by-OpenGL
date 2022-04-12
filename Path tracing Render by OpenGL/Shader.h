#pragma once
#include "Common.h"
#include <string>
#include<fstream>
#include<sstream>
#include <iostream>

struct ShaderProgramSource {
    std::string vertex_source;
    std::string fragment_source;
};

class Shader
{
public:
    Shader(const std::string& filepath);

    //Frist, load the shader file.Just use one file for now
    
    //Second, 
    unsigned int getProgramID() {
        return program_id;
    }

    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(program_id, name.c_str()), value);
    }
    void setMat4(const std::string& name, const glm::mat4& mat) const
    {
        glUniformMatrix4fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
    }
    void use()
    {
        glUseProgram(program_id);
    }
    void setVec3(const std::string& name, float x, float y, float z) const
    {
        glUniform3f(glGetUniformLocation(program_id, name.c_str()), x, y, z);
    }
    void setVec3(const std::string& name, const glm::vec3& vec) const
    {
        glUniform3f(glGetUniformLocation(program_id, name.c_str()), vec.x, vec.y, vec.z);
    }
    void deleteProgram() {
        glDeleteProgram(program_id);
    }

private:
    unsigned int CompileShader(const std::string& source, unsigned int type);
    unsigned int program_id = 0;
    std::string vertex_shader;
    std::string fragment_shader;
};

Shader::Shader(const std::string& filepath) {
    std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                //set mode to vertex
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                //set mode to fragment
                type = ShaderType::FRAGMENT;
            }
        }
        else {
            ss[(int)type] << line << '\n';
        }
    }
    vertex_shader = ss[0].str();
    fragment_shader = ss[1].str();
    if (type == ShaderType::NONE)
        std::cout << "Wrong file type" << std::endl;

    //create program
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(vertex_shader, GL_VERTEX_SHADER);
    unsigned int fs = CompileShader(fragment_shader, GL_FRAGMENT_SHADER);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    int result;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char* msg = (char*)_malloca(length * sizeof(char));
        glGetProgramInfoLog(program, length, &length, msg);
        std::cout << "Failed to create program " << std::endl;
        std::cout << msg << std::endl;

    }
    //glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
    program_id = program;
    
};

unsigned int Shader::CompileShader(const std::string& source, unsigned int type) {
    unsigned int shader_id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader_id, 1, &src, nullptr);
    glCompileShader(shader_id);

    int result;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
        char* msg = (char*)_malloca(length * sizeof(char));
        glGetShaderInfoLog(shader_id, length, &length, msg);
        std::cout << source << " Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
        std::cout << msg << std::endl;
        glDeleteShader(shader_id);
        return 0;
    }

    return shader_id;
}