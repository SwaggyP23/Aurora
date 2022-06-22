#include "Graphics/Shader.h"
#include <IMGUI/imgui.h>
#include <IMGUI/imgui_impl_glfw.h>
#include <IMGUI/imgui_impl_opengl3.h>
#include <glm/glm.hpp>

#include <iostream>
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

	GLfloat vertices[4 * 6] = {
		-0.5f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f,
	 	-0.5f,  0.5f,    0.0f, 1.0f, 0.0f, 1.0f,
		 0.5f,  0.5f,    0.0f, 0.0f, 1.0f, 1.0f,
		 0.5f, -0.5f,    0.5f, 0.0f, 1.0f, 1.0f
	};

	GLuint indices[6] = { 0, 1, 2, 2, 3, 0 };

	GLuint vertexArray;
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const void*)0/*0 * sizeof(GLfloat)*/);
	// Stride is the size (in bytes) between instances of the same vertex attributes.
	// offset is the poisition of where the data we are specifying begins in the buffer.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (const void*)8/*2 * sizeof(GLfloat)*/);
	glEnableVertexAttribArray(1);

	GLuint indexbuffer;
	glGenBuffers(1, &indexbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	Shader shader("res/shaders/Basic.shader");

	shader.bind();

	//glUniform4f(glGetUniformLocation(program, "color"), uniColor.r, uniColor.g, uniColor.b, uniColor.a);

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

		//glUniform4f(glGetUniformLocation(program, "color"), uniColor.r, uniColor.g, uniColor.b, uniColor.a);
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

	glfwTerminate();
	return 0;

}