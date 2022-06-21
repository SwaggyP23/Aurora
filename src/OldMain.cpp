#include "Core.h"

int main()
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "OpenGL Series", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
        std::cout << "Error" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    Core reda;

    GLfloat position[8] = {
        -0.5f, -0.5f,
        -0.5f,  0.5f,
         0.5f,  0.5f,
         0.5f, -0.5f
    };

    GLuint buffer;
    reda.createBuffer(GL_ARRAY_BUFFER, &buffer);
    reda.setBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), position);

    GLuint indexbuffer;
    GLuint indices[6] = { 0, 1, 2, 2, 3, 0 };
    reda.createBuffer(GL_ELEMENT_ARRAY_BUFFER, &indexbuffer);
    reda.setBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), indices);
 
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    ShaderSources source = reda.parseShader("resources/shaders/Basic.shader");

    unsigned int shader = reda.createShader(source.vertexSource, source.fragmentSource);

    glUseProgram(shader);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexbuffer);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}