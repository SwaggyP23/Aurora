#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

struct ShaderSources {
	std::string vertexSource;
	std::string fragmentSource;
};

class Core {
public:
	Core() = default;

	void createBuffer(GLenum type, unsigned int* val) {
		glGenBuffers(1, val);
		glBindBuffer(type, *val);
	}

	void setBufferData(GLenum type, GLsizeiptr size, const void* data, GLenum usage = GL_STATIC_DRAW) {
		glBufferData(type, size, data, usage);
        glBindBuffer(type, 0);
	}

    ShaderSources parseShader(const std::string& filePath) {
        std::ifstream stream(filePath);

        std::string line;
        std::stringstream ss[2];
        ShaderType type = ShaderType::NONE;

        while (std::getline(stream, line)) {
            if (line.find("shader") != std::string::npos) {
                if (line.find("vertex") != std::string::npos)
                    // set mode to vertex
                    type = ShaderType::VERTEX;
                else if (line.find("fragment") != std::string::npos)
                    // set mode to fragment
                    type = ShaderType::FRAGMENT;
            }
            else {
                ss[(int)type] << line << '\n';
            }
        }

        return { ss[0].str(), ss[1].str() };
    }

    unsigned int createShader(const std::string& vertexShader, const std::string& fragmentShader) {
        unsigned int program = glCreateProgram();
        unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
        unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);
        glValidateProgram(program);

        glDeleteShader(vs);
        glDeleteShader(fs);

        return program;
    }

    unsigned int compileShader(unsigned int type, const std::string& source) {
        unsigned int id = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);

        // Error handling...

        int result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);

        if (result == GL_FALSE) {
            int length;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)alloca(length * sizeof(char));
            glGetShaderInfoLog(id, length, &length, message);

            std::cout << "Failed to compile shader " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " Shader!" << std::endl;
            std::cout << message << std::endl;

            glDeleteShader(id);
            return 0;
        }

        // Error handling...

        return id;
    }

private:
	enum ShaderType {
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

};