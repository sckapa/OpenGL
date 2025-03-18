#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>

#define ASSERT(x) if(!(x)) __debugbreak()
#define GLCall(x) GLClearErrors();\
        x;\
        ASSERT(GLPrintErrors(#x, __FILE__, __LINE__))

static bool GLPrintErrors(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] : " << error << " Function : " << function << " Path : " << file << " Line : " << line;
        return false;
    }
    return true;
}

static void GLClearErrors()
{
    while (glGetError() != GL_NO_ERROR);
}

static unsigned int CompileShader(unsigned int type, std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to load " << (type == GL_FRAGMENT_SHADER ? "fragment" : "vertex") << " shader!" << std::endl;
        std::cout << message << std::endl;

        glDeleteShader(id);
        return 0;
    }

    return id;
}

static unsigned int CreateShader(std::string& vertexShader, std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

static std::unordered_map<GLenum, std::string> ParseShader(const char* path)
{
    std::unordered_map<GLenum, std::string> result;

    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string src = buffer.str();

    const char* token = "#type";
    int length = strlen(token);

    int pos = src.find(token);
    while (pos != std::string::npos)
    {
        if (src.find("vertex", pos + length) == pos + length + 1)
        {
            size_t start = pos + length + 7;
            size_t end = src.find(token, pos + length);

            if (end == std::string::npos)
            {
                end = src.size();
            }
            result.emplace(GL_VERTEX_SHADER, src.substr(start, end - start));
        }
        else if (src.find("fragment", pos + length) == pos + length + 1)
        {
            size_t start = pos + length + 9;
            size_t end = src.find(token, pos + length);

            if (end == std::string::npos)
            {
                end = src.size();
            }
            result.emplace(GL_FRAGMENT_SHADER, src.substr(start, end - start));
        }
        else
        {
            std::cout << "Error parsing shader file!";
            break; // TODO : create break point here
        }

        pos = src.find(token, pos + length);
    }

    return result;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "Error!" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    float positions[] =
    {
        -0.5f,-0.5f,
         0.5f,-0.5f,
         0.5f, 0.5f,
        -0.5f, 0.5f
    };

    unsigned int indices[] =
    {
        0,1,2,2,3,0
    };

    unsigned int vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    std::unordered_map<GLenum, std::string> map = ParseShader("shaders/Texture.glsl");
    unsigned int program = CreateShader(map[GL_VERTEX_SHADER], map[GL_FRAGMENT_SHADER]);
    glUseProgram(program);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(program);

    glfwTerminate();
    return 0;
}