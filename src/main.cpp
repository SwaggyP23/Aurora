#include "Graphics/Shader.h"
#include "Graphics/Window.h"
#include <stb_image/stb_image.h>

#include <iostream>
#include <vector>

int main()
{
	Window window("OpenGL", 1024, 576);
	window.enable(GL_DEPTH_TEST);

	glm::vec4 color(0.2f, 0.7f, 0.8f, 1.0f); // Initial clear color.
	glm::vec4 uniColor(0.5f);
	std::vector<char> errorMessage;

	//GLfloat vertices[4 * 6 * 9] = {
	//	-0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 0
	// 	-0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 1
	//	 0.5f,  0.5f, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f, // 2
	//	 0.5f, -0.5f, -0.5f,	0.5f, 0.0f, 1.0f, 1.0f,   1.0f, 0.0f, // 3

	//    -0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 4
	//    -0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 5
	//    -0.5f,  0.5f, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f, // 6
	//    -0.5f,  0.5f,  0.5f,	0.5f, 0.0f, 1.0f, 1.0f,   1.0f, 0.0f, // 7

	//    -0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 8
	//    -0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 9
	//     0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f, // 10
	//     0.5f, -0.5f,  0.5f,	0.5f, 0.0f, 1.0f, 1.0f,   1.0f, 0.0f, // 11

	//	 0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 12
	//	 0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 13
	//	 0.5f,  0.5f, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f, // 14
	//	 0.5f,  0.5f,  0.5f,	0.5f, 0.0f, 1.0f, 1.0f,   1.0f, 0.0f, // 15

	//    -0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 16
	//     0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 17
	//    -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f, // 18
	//     0.5f, -0.5f, -0.5f,	0.5f, 0.0f, 1.0f, 1.0f,   1.0f, 0.0f, // 19

	//	-0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // 20
	//	 0.5f, -0.5f,  0.5f,    0.0f, 1.0f, 0.0f, 1.0f,   0.0f, 1.0f, // 21
	//	-0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 1.0f, 1.0f,   1.0f, 1.0f, // 22
	//	 0.5f, -0.5f,  0.5f,	0.5f, 0.0f, 1.0f, 1.0f,   1.0f, 0.0f  // 23
	//};

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	GLuint indices[6] = { 0, 1, 2, 2, 3, 0 };

	GLuint vertexArray;
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)0/*0 * sizeof(GLfloat)*/);
	// Stride is the size (in bytes) between instances of the same vertex attributes.
	// offset is the poisition of where the data we are specifying begins in the buffer.
	glEnableVertexAttribArray(0);
	//glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)12/*3 * sizeof(GLfloat)*/);
	//glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)12/*7 * sizeof(GLfloat)*/);
	glEnableVertexAttribArray(1);

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

	data = stbi_load("res/textures/pack.png", &width, &height, &channels, 0);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
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

	glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1024.0f / 576.0f, 0.1f, 100.0f);

	shader.setUniformMat4("ml_matrix", model);
	shader.setUniformMat4("vw_matrix", view);
	shader.setUniformMat4("pr_matrix", projection);

	float f = 0.0f;
	float ff = 0.0f;
	glm::vec3 fff(0.0f, 0.0f, 0.0f);

	while (!window.closed()) // Render Loop.
	{
		window.clear(color.r, color.g, color.b, 1.0f);

		ImGui::Begin("Colors");
		ImGui::ColorEdit3("Clear Color:", (float*)&color);
		ImGui::ColorEdit3("Uniform Color:", (float*)&uniColor);
		ImGui::SliderFloat("Blend:", &f, 0.0f, 1.0f);
		ImGui::SliderFloat("rotation:", &ff, -10.0f, 10.0f);
		ImGui::SliderFloat3("tranlation:", &fff[0], 10.0f, -5.0f);
		ImGui::End();

		shader.setUniform1f("blend", f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, text1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, text2);

		glBindVertexArray(vertexArray);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);
		for (unsigned int i = 0; i < 10; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i] + fff);
			//model = glm::translate(model, fff);
			//float angle = 20.0f * i;
			model = glm::rotate(model, ff, glm::vec3(1.0f, 0.3f, 0.5f));
			//ourShader.setMat4("model", model);
			shader.setUniformMat4("ml_matrix", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);

		//rotation = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
		//translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.5f, 0.0f));

		//shader.setUniformMat4("rotation", rotation);
		//shader.setUniformMat4("translation", translation);

		//glBindVertexArray(vertexArray);

		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);

		//float scaleAmount = static_cast<float>(sin(glfwGetTime()));
		//scale = glm::scale(glm::mat4(1.0f), glm::vec3(scaleAmount, scaleAmount, scaleAmount));

		//rotation = glm::mat4(1.0f);
		//translation = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.5f, 0.0f));

		//shader.setUniformMat4("scale", scale);
		//shader.setUniformMat4("rotation", rotation);
		//shader.setUniformMat4("translation", translation);

		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);
		//glBindVertexArray(0);


		window.update();
	}
	
	glDeleteBuffers(1, &vertexArray);
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &indexbuffer);

	return 0;
}