#include "Graphics/Shader.h"
#include <IMGUI/imgui.h>
#include <IMGUI/imgui_impl_glfw.h>
#include <IMGUI/imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <stb_image/stb_image.h>

#include <iostream>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main()
{
	if (!glfwInit()) {
		std::cout << "Failed to initialize glfw" << std::endl;
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(1024, 576, "OpenGL", NULL, NULL);

	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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
	glm::vec4 color(0.2f, 0.7f, 0.8f, 1.0f); // Initial clear color.
	glm::vec4 uniColor(0.5f);
	std::vector<char> errorMessage;

	GLfloat vertices[4 * 8] = {
		-0.7f, -0.7f,    1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
	 	-0.7f,  0.7f,    0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f,
		 0.7f,  0.7f,    0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f,
		 0.7f, -0.7f,    0.5f, 0.0f, 1.0f, 1.0f,   1.0f, 0.0f
	};

	GLuint indices[6] = { 0, 1, 2, 2, 3, 0 };

	GLuint vertexArray;
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)0/*0 * sizeof(GLfloat)*/);
	// Stride is the size (in bytes) between instances of the same vertex attributes.
	// offset is the poisition of where the data we are specifying begins in the buffer.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)8/*2 * sizeof(GLfloat)*/);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)24/*6 * sizeof(GLfloat)*/);
	glEnableVertexAttribArray(2);

	GLuint indexbuffer;
	glGenBuffers(1, &indexbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	Shader shader("res/shaders/Basic.shader");

	GLuint text1, text2;
	glGenTextures(1, &text1);
	glBindTexture(GL_TEXTURE_2D, text1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load image
	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	GLubyte* data = stbi_load("res/textures/lufi.png", &width, &height, &channels, 0);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Second texture
	glGenTextures(1, &text2);
	glBindTexture(GL_TEXTURE_2D, text2);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load("res/textures/awesomefaceTexture.png", &width, &height, &channels, 0);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);

	shader.bind();
	shader.setUniform1i("texture1", 0);
	shader.setUniform1i("texture2", 1);

	//glUniform4f(glGetUniformLocation(program, "color"), uniColor.r, uniColor.g, uniColor.b, uniColor.a);

	float f = 0.0f;

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
		ImGui::SliderFloat("Blend:", &f, 0.0f, 1.0f);
		ImGui::End();

		//glUniform4f(glGetUniformLocation(program, "color"), uniColor.r, uniColor.g, uniColor.b, uniColor.a);
		shader.setUniform1f("blend", f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, text1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, text2);

		shader.bind();
		glBindVertexArray(vertexArray);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer); // We do not need these since the index buffer is in the VA
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}
	
	glDeleteBuffers(1, &vertexArray);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &indexbuffer);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;

}