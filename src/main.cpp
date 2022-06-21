#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <IMGUI/imgui.h>
#include <IMGUI/imgui_impl_glfw.h>
#include <IMGUI/imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>

int main()
{
	if (!glfwInit()) {
		std::cout << "Failed to initialize glfw" << std::endl;
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(960, 580, "OpenGL", NULL, NULL);

	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (glewInit()) {
		std::cout << "Failed to initialize glew" << std::endl;
		return -1;
	}

	std::cout << glGetString(GL_VERSION) << std::endl;

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
	glViewport(0, 0, 960, 580);
	glm::vec4 color(0.0f); // Initial clear color.
	glm::vec4 uniColor(0.5f);
	std::vector<char> errorMessage;

	GLfloat vertices[4 * 2] = {
		-0.5f, -0.5f,
	 	-0.5f,  0.5f,
		 0.5f,  0.5f, 
		 0.5f, -0.5f
	};

	GLuint indices[6] = { 0, 1, 2, 2, 3, 0 };

	GLuint vertexArray;
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (const void*)0);
	// Stride is the size (in bytes) between each vertex.
	// offset is the poisition of where the data we are specifying begins in the buffer.
	glEnableVertexAttribArray(0);

	GLuint indexbuffer;
	glGenBuffers(1, &indexbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	std::string vertexSource = R"(
		#version 330 core

		layout (location = 0) in vec3 a_Position; // since this is an attribute, takes the a_ Prefix

		void main()
		{
			gl_Position = vec4(a_Position, 1.0f);			
		}
	)";
	const char* vertexC_str = vertexSource.c_str();

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexC_str, NULL);
	glCompileShader(vertexShader);

	GLint result;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		GLint length;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &length);
		glGetShaderInfoLog(vertexShader, length, &length, &errorMessage[0]);
		
		std::cout << "Failed to compile Vertex Shader!!\n";
		std::cout << errorMessage[0] << std::endl;

		glDeleteShader(vertexShader);
	}

	std::string fragmentSource = R"(
		#version 330 core
		
		layout (location = 0) out vec4 a_Color;

		uniform vec4 color;

		void main()
		{
			a_Color = color; 
		}		
	)";

	const char* fragmentC_str = fragmentSource.c_str();

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentC_str, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		GLint length;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &length);
		glGetShaderInfoLog(fragmentShader, length, &length, &errorMessage[0]);

		std::cout << "Failed to compile Fragment Shader!!\n";
		std::cout << errorMessage[0] << std::endl;

		glDeleteShader(fragmentShader);
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	glValidateProgram(program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glUseProgram(program);

	glUniform4f(glGetUniformLocation(program, "color"), uniColor.r, uniColor.g, uniColor.b, uniColor.a);

	while (!glfwWindowShouldClose(window)) // Render Loop.
	{
		glfwPollEvents();
		glClearColor(color.r, color.g, color.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Colors");
		ImGui::ColorEdit3("Clear Color:", (float*)&color);
		ImGui::ColorEdit3("Uniform Color:", (float*)&uniColor);
		ImGui::End();

		glUniform4f(glGetUniformLocation(program, "color"), uniColor.r, uniColor.g, uniColor.b, uniColor.a);
		glBindVertexArray(vertexArray);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer); // We do not need these since the index buffer is in the VA
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteProgram(program);
	glfwTerminate();
	return 0;

}