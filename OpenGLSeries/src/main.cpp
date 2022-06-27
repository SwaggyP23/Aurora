

bool firstMouse = true;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
bool running = true;

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
Camera camera(cameraPos);

bool onWindowCloseEvent(WindowCloseEvent& e)
{
	running = false;
	return true;
}

void onEvent(Event& e)
{
	EventDispatcher dispatcher(e);
	dispatcher.dispatch<WindowCloseEvent>(std::bind(onWindowCloseEvent, std::placeholders::_1));
	LOG_INFO("{0}", e);
}

int main()
{
	Window window("OpenGL", 1280, 720);
	window.enable(GL_DEPTH_TEST);
	window.SetVSync(false);
	window.SetEventCallback(std::bind(&onEvent, std::placeholders::_1));

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

	//glm::vec3 cubePositions[] = {
	//	glm::vec3(0.0f,  0.0f,  0.0f),
	//	glm::vec3(2.0f,  5.0f, -15.0f),
	//	glm::vec3(-1.5f, -2.2f, -2.5f),
	//	glm::vec3(-3.8f, -2.0f, -12.3f),
	//	glm::vec3(2.4f, -0.4f, -3.5f),
	//	glm::vec3(-1.7f,  3.0f, -7.5f),
	//	glm::vec3(1.3f, -2.0f, -2.5f),
	//	glm::vec3(1.5f,  2.0f, -2.5f),
	//	glm::vec3(1.5f,  0.2f, -1.5f),
	//	glm::vec3(-1.3f,  1.0f, -1.5f)
	//};

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(0.0f,  0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, -2.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f,  0.0f, -1.0f),
		glm::vec3(-1.0f, 0.0f, -2.0f),
		glm::vec3(1.0f,  0.0f, -1.0f),
		glm::vec3(1.0f,  0.0f, -2.0f),
		glm::vec3(0.0f,  1.0f, -1.0f)
	};

	GLuint indices[6] = { 0, 1, 2, 2, 3, 0 };

	VertexArray vertexArray;

	BufferLayout layout = {
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float2, "a_TexCoord" }
	};

	VertexBuffer vertexBuffer(vertices, sizeof(vertices));
	vertexBuffer.bind();
	vertexBuffer.setLayout(layout);
	vertexArray.addVertexBuffer(vertexBuffer);


	IndexBuffer indexBuffer(indices, sizeof(indices) / sizeof(GLuint));
	indexBuffer.bind();
	vertexArray.setIndexBuffer(indexBuffer);

	vertexBuffer.unBind();
	indexBuffer.unBind();

	Shader shader("res/shaders/Basic.shader");

	// Creating textures
	Texture text1("res/textures/minecraftDirt.png");
	text1.bind();
	text1.setTextureWrapping(GL_REPEAT);
	text1.setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text1.loadTextureData(GL_RGB, GL_RGB);
	text1.unBind();

	Texture text2("res/textures/minecraftTNT.png");
	text2.bind();
	text2.setTextureWrapping(GL_REPEAT);
	text2.setTextureFiltering(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	text2.loadTextureData(GL_RGBA, GL_RGBA);
	text2.unBind();

	// Vector of textures for easily adding and binding textures in render loop
	std::vector<std::unique_ptr<Texture>> textures;
	textures.push_back(std::make_unique<Texture>(text1));
	textures.push_back(std::make_unique<Texture>(text2));
	
	shader.bind();
	shader.setUniform1i("texture1", 0);
	shader.setUniform1i("texture2", 1);

	// Just some hard coded program start MVP matrices
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1024.0f / 576.0f, 0.1f, 100.0f);

	shader.setUniformMat4("ml_matrix", model);
	shader.setUniformMat4("vw_matrix", view);
	shader.setUniformMat4("pr_matrix", projection);

	//float fov = 45.0f;
	float blend = 0.0f;
	float rotation = 0.0f;
	glm::vec3 translation(0.0f, 0.0f, 0.0f);

	// timing
	float deltaTime = 0.0f;	// time between current frame and last frame
	float lastFrame = 0.0f;
	bool isRpressed = false;

	while (running) // Render Loop.
	{
		float currentFrame = (float)(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//processInput(window.getWindowPointer(), deltaTime);

		window.clear(color.r, color.g, color.b, 1.0f);

		ImGui::Begin("Colors");
		ImGui::ColorEdit3("Clear Color:", (float*)&color);
		ImGui::ColorEdit3("Uniform Color:", (float*)&uniColor);
		ImGui::SliderFloat("FOV:", &(camera.getZoom()), 10.0f, 90.0f);
		ImGui::SliderFloat("Blend:", &blend, 0.0f, 1.0f);
		ImGui::SliderFloat("rotation:", &rotation, -10.0f, 10.0f);
		ImGui::SliderFloat3("transforms:", &translation[0], 10.0f, -5.0f);
		ImGui::Text("Average FrameRate: %.3f", ImGui::GetIO().Framerate);
		ImGui::End();

		int i = 0;
		for (const auto& texture : textures)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			texture->bind();
			i++;
		}

		//if (window.isKeyPressed(GLFW_KEY_B))
		//	if(blend < 0.990f) blend += 0.01f;
		//if (window.isKeyPressed(GLFW_KEY_V))
		//	if(blend > 0.010f) blend -= 0.01f;

		shader.setUniform1f("blend", blend);

		glm::mat4 view = camera.GetViewMatrix();
		shader.setUniformMat4("vw_matrix", view);

		projection = glm::perspective(glm::radians(camera.getZoom()), 1024.0f / 576.0f, 0.1f, 100.0f);
		shader.setUniformMat4("pr_matrix", projection);

		//if (window.isKeyPressed(GLFW_KEY_R))
		//	isRpressed = !isRpressed;

		vertexArray.bind();
		for (unsigned int i = 0; i < 10; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			model = glm::translate(model, cubePositions[i] + translation);
			float angle = 20.0f * i;
			if (isRpressed) {
				model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.3f, 0.5f));
			}
			else
				model = glm::rotate(model, rotation, glm::vec3(1.0f, 0.3f, 0.5f));
			shader.setUniformMat4("ml_matrix", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		vertexArray.unBind();

		window.update();
	}

	return 0;
}