#include <SDL.h>
#include <GLES2/gl2.h>
#include <emscripten.h>
#include <stdio.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window* gWindow = NULL;
SDL_GLContext gContext;

GLuint shaderProgram;
GLuint vbo;

bool init();
void render();
void close();
void main_loop();

// Shader sources
const char* vertexShaderSource = R"(
    attribute vec2 aPos;
    attribute vec3 aColor;
    varying vec3 vColor;
    void main() {
        gl_Position = vec4(aPos, 0.0, 1.0);
        vColor = aColor;
    }
)";

const char* fragmentShaderSource = R"(
    precision mediump float;
    varying vec3 vColor;
    void main() {
        gl_FragColor = vec4(vColor, 1.0);
    }
)";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compile error: %s\n", infoLog);
    }

    return shader;
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init error: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2); // WebGL 1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    gWindow = SDL_CreateWindow("Trójkąt OpenGL ES 2.0",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               SCREEN_WIDTH, SCREEN_HEIGHT,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!gWindow) {
        printf("Window error: %s\n", SDL_GetError());
        return false;
    }

    gContext = SDL_GL_CreateContext(gWindow);
    if (!gContext) {
        printf("GL context error: %s\n", SDL_GetError());
        return false;
    }

    // OpenGL setup
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f); // Niebieskie tło

    // Compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Triangle data: [x, y, r, g, b]
    float vertices[] = {
        // Pozycja   // Kolor
        0.0f,  0.5f, 1.0f, 0.0f, 0.0f, // góra (czerwony)
       -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // lewo (zielony)
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f  // prawo (niebieski)
    };

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    return true;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "aPos");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    GLint colAttrib = glGetAttribLocation(shaderProgram, "aColor");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    glDrawArrays(GL_TRIANGLES, 0, 3);
    SDL_GL_SwapWindow(gWindow);
}

void close() {
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &vbo);
    SDL_GL_DeleteContext(gContext);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
}

void main_loop() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            emscripten_cancel_main_loop();
        }
    }

    render();
}

int main(int argc, char* args[]) {
    if (!init()) {
        printf("Inicjalizacja nie powiodła się!\n");
        return 1;
    }

    atexit(close);
    emscripten_set_main_loop(main_loop, 0, 1);
    return 0;
}
