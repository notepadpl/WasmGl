#include <SDL.h>
#include <GLES2/gl2.h>
#include <emscripten.h>
#include <cmath>
#include <stdio.h>
#include "obj_loader.h"

SDL_Window* window;
SDL_GLContext glContext;
Mesh mesh;
GLuint program, vbo, ibo;
float angle = 0.0f;

// Shadery (vertex obraca model w płaszczyźnie XY dla uproszczenia)
const char* vs = R"(
    attribute vec3 aPos;
    uniform float uAngle;
    void main() {
        float c = cos(uAngle), s = sin(uAngle);
        mat3 rot = mat3(c, -s, 0, s, c, 0, 0, 0, 1);
        gl_Position = vec4(rot * aPos, 1.0);
    }
)";
const char* fs = R"(
    precision mediump float;
    void main() { gl_FragColor = vec4(0.8, 0.3, 0.3, 1.0); }
)";

// (funkcja compileShader jak wcześniej)...

bool init() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    window = SDL_CreateWindow("OBJ Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              640, 480, SDL_WINDOW_OPENGL);
    glContext = SDL_GL_CreateContext(window);
    glClearColor(0.2f,0.2f,0.2f,1);

    // wczytaj model
    mesh = loadObjSimple("asserts/cube2.obj");
    printf("Vertices: %zu, Indices: %zu\n", mesh.vertices.size()/3, mesh.indices.size()/3);

    //compile and link shaders...
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size()*sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size()*sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

    return true;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
    glUniform1f(glGetUniformLocation(program, "uAngle"), angle);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    GLint pos = glGetAttribLocation(program, "aPos");
    glEnableVertexAttribArray(pos);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);

    SDL_GL_SwapWindow(window);
}

void loop() {
    angle += 0.01f;
    SDL_Event e;
    while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) emscripten_cancel_main_loop();
    render();
}

int main() {
    init();
    emscripten_set_main_loop(loop, 0, 1);
    return 0;
}
