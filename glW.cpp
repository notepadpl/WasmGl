#include <SDL.h>
#include <GLES2/gl2.h>
#include <emscripten.h>
#include <stdio.h>
#include <math.h>

// Wymiary okna
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// Globalne wskaźniki
SDL_Window* window = nullptr;
SDL_GLContext glContext;

// Shader program (połączony vertex + fragment shader)
GLuint shaderProgram;
GLuint vbo;

// Czas używany do obrotu trójkąta
float timeElapsed = 0.0f;

// Vertex Shader – obraca trójkąt w czasie
const char* vertexSource = R"(
    attribute vec2 aPos;
    attribute vec3 aColor;
    uniform float uTime;
    varying vec3 vColor;

    void main() {
        float angle = uTime;
        mat2 rotation = mat2(
            cos(angle), -sin(angle),
            sin(angle),  cos(angle)
        );
        gl_Position = vec4(rotation * aPos, 0.0, 1.0);
        vColor = aColor;
    }
)";

// Fragment Shader – rysuje trójkąt na podstawie koloru z vertex shadera
const char* fragmentSource = R"(
    precision mediump float;
    varying vec3 vColor;

    void main() {
        gl_FragColor = vec4(vColor, 1.0);
    }
)";

// Tworzy i kompiluje pojedynczy shader (vertex lub fragment)
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Sprawdzenie błędów kompilacji
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Błąd kompilacji shaderów:\n%s\n", infoLog);
    }

    return shader;
}

// Inicjalizacja SDL, OpenGL i shaderów
bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Błąd SDL: %s\n", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2); // OpenGL ES 2.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    window = SDL_CreateWindow("Obracany Trójkąt", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

    if (!window) {
        printf("Błąd tworzenia okna: %s\n", SDL_GetError());
        return false;
    }

    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        printf("Błąd tworzenia kontekstu OpenGL: %s\n", SDL_GetError());
        return false;
    }

    // Kolor tła: zielony (R,G,B,A)
    glClearColor(0.0f, 0.5f, 0.0f, 1.0f);

    // Kompilacja shaderów
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    // Tworzymy program shaderowy i łączymy oba shadery
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Dane wierzchołków: [x, y, r, g, b]
    float vertices[] = {
        // pozycja   // kolor
        0.0f,  0.5f,  1.0f, 0.0f, 0.0f, // czerwony
       -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, // zielony
        0.5f, -0.5f,  0.0f, 0.0f, 1.0f  // niebieski
    };

    // Tworzymy Vertex Buffer Object (VBO)
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    return true;
}

// Główna funkcja renderująca
void render() {
    glClear(GL_COLOR_BUFFER_BIT); // Czyści ekran do koloru tła

    glUseProgram(shaderProgram);

    // Ustawiamy uniform z czasem (na potrzeby obrotu)
    GLint timeLoc = glGetUniformLocation(shaderProgram, "uTime");
    glUniform1f(timeLoc, timeElapsed);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Pozycja (vec2) → aPos
    GLint posAttrib = glGetAttribLocation(shaderProgram, "aPos");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // Kolor (vec3) → aColor
    GLint colAttrib = glGetAttribLocation(shaderProgram, "aColor");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    // Rysowanie trójkąta
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SDL_GL_SwapWindow(window);
}

// Sprzątanie zasobów
void cleanup() {
    glDeleteProgram(shaderProgram);
    glDeleteBuffers(1, &vbo);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
void main_loop() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            emscripten_cancel_main_loop();
        }
        else if (e.type == SDL_MOUSEBUTTONDOWN) {
            if (e.button.button == SDL_BUTTON_LEFT) {
                // Zmieniamy stan animacji (pauza <-> play)
                isAnimating = !isAnimating;
            }
        }
    }

    // Jeśli animacja jest aktywna, zwiększamy czas
    if (isAnimating) {
        timeElapsed += 0.01f;
    }

    render();
}

// Główna pętla aplikacji
void main_loop2() {
    // Aktualizacja czasu (w radianach)
    timeElapsed += 0.01f;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            emscripten_cancel_main_loop();
        }
    }

    render();
}

int main() {
    if (!init()) {
        return 1;
    }

    atexit(cleanup);
    emscripten_set_main_loop(main_loop, 0, 1); // pętla główna
    return 0;
}
