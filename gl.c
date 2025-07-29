#include <SDL2/SDL.h>
#include <emscripten.h>
#include <GLES2/gl2.h> // Uzywamy GLES2/gl2.h dla WebGL 2.0
#include <stdio.h>
#include <stdlib.h> // Dla malloc, free
#include <string.h> // Dla strlen

SDL_Window* window = NULL;
SDL_GLContext gl_context;

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

// Identyfikatory programu shaderowego i bufora wierzcholkow
GLuint shader_program = 0;
GLuint VBO = 0;

// --- Shadery w postaci lancuchow znakow ---
// Vertex Shader: Definiuje pozycje wierzcholkow i przekazuje kolory do Fragment Shadera
const char* vertex_shader_source =
    "attribute vec2 a_position;\n" // Atrybut dla pozycji wierzcholka (x, y)
    "attribute vec3 a_color;\n"    // Atrybut dla koloru wierzcholka (r, g, b)
    "varying vec3 v_color;\n"      // Varying (przekazywana) zmienna koloru do Fragment Shadera
    "void main() {\n"
    "   gl_Position = vec4(a_position, 0.0, 1.0);\n" // Ustawiamy pozycje wierzcholka (z=0, w=1 dla 2D)
    "   v_color = a_color;\n"                      // Przypisujemy kolor do przekazania
    "}\n";

// Fragment Shader: Koloruje piksele na podstawie interpolowanych kolorow wierzcholkow
const char* fragment_shader_source =
    "precision mediump float;\n" // Precyzja dla zmiennych float (wymagane w GLSL ES)
    "varying vec3 v_color;\n"    // Odbieramy interpolowany kolor
    "void main() {\n"
    "   gl_FragColor = vec4(v_color, 1.0);\n" // Ustawiamy koncowy kolor piksela (alpha=1.0)
    "}\n";

// --- Funkcje pomocnicze do kompilacji shaderow ---
GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Blad kompilacji shadera (%s): %s\n",
                (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"), infoLog);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint create_shader_program(const char* vs_source, const char* fs_source) {
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vs_source);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fs_source);

    if (!vertex_shader || !fragment_shader) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "Blad linkowania programu shaderowego: %s\n", infoLog);
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertex_shader); // Shadery sa juz zalinkowane do programu, mozna je usunac
    glDeleteShader(fragment_shader);
    return program;
}
// --- Koniec funkcji pomocniczych ---


// Funkcja inicjalizujaca SDL i OpenGL
int init_sdl_opengl() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Nie mozna zainicjalizowac SDL: %s\n", SDL_GetError());
        return 0;
    }

    // Ustawienie atrybutow OpenGL/WebGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2); // Wymus WebGL 2.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window = SDL_CreateWindow(
        "SDL2 OpenGL Emscripten",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    if (window == NULL) {
        fprintf(stderr, "Nie mozna utworzyc okna SDL: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL) {
        fprintf(stderr, "Nie mozna utworzyc kontekstu OpenGL: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    if (SDL_GL_MakeCurrent(window, gl_context) < 0) {
        fprintf(stderr, "Nie mozna ustawic biezacego kontekstu GL: %s\n", SDL_GetError());
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    // --- Inicjalizacja shaderow i buforow ---
    shader_program = create_shader_program(vertex_shader_source, fragment_shader_source);
    if (!shader_program) {
        fprintf(stderr, "Nie mozna utworzyc programu shaderowego.\n");
        return 0;
    }
    glUseProgram(shader_program); // Aktywuj program shaderowy

    // Dane wierzcholkow: pozycja (x,y) i kolor (r,g,b)
    // float vertices[] = {
    //    // Pozycje            // Kolory
    //     0.0f,  0.5f,        1.0f, 0.0f, 0.0f, // Wierzcholek gorny (czerwony)
    //    -0.5f, -0.5f,        0.0f, 1.0f, 0.0f, // Wierzcholek lewy (zielony)
    //     0.5f, -0.5f,        0.0f, 0.0f, 1.0f  // Wierzcholek prawy (niebieski)
    // };
    // Zaktualizowano dla lepszej widocznosci kolorów w WebGL (niektóre przeglądarki mogą mieć problemy z 0.0f/0.0f)
    float vertices[] = {
        // Pozycje            // Kolory
        0.0f,  0.5f,        1.0f, 0.0f, 0.0f, // Czerwony
       -0.5f, -0.5f,        0.0f, 1.0f, 0.0f, // Zielony
        0.5f, -0.5f,        0.0f, 0.0f, 1.0f  // Niebieski
    };


    glGenBuffers(1, &VBO); // Generowanie bufora wierzcholkow
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Aktywacja bufora
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Zaladowanie danych

    // Ustawienie atrybutow wierzcholkow (pozycja i kolor)
    // Pozycja
    GLint pos_attrib = glGetAttribLocation(shader_program, "a_position");
    glEnableVertexAttribArray(pos_attrib);
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // Kolor
    GLint color_attrib = glGetAttribLocation(shader_program, "a_color");
    glEnableVertexAttribArray(color_attrib);
    glVertexAttribPointer(color_attrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    // --- Koniec inicjalizacji shaderow i buforow ---


    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Ciemny turkus na tle

    fprintf(stdout, "SDL i OpenGL zainicjalizowane pomyslnie.\n");
    return 1;
}

// Funkcja renderujaca scene OpenGL
void render_scene() {
    glClear(GL_COLOR_BUFFER_BIT); // Wyczyszczenie bufora koloru

    // Uzyj wlasnego programu shaderowego
    glUseProgram(shader_program);

    // Rysowanie trojkata z bufora wierzcholkow
    glDrawArrays(GL_TRIANGLES, 0, 3); // Rysuj 3 wierzcholki (jeden trojkat)

    SDL_GL_SwapWindow(window); // Zamiana buforow (wyswietlenie renderowanej sceny)
}

// Glowna petla aplikacji
void main_loop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            emscripten_cancel_main_loop();
        }
    }

    render_scene();
}

// Funkcja czyszczaca zasoby
void cleanup() {
    glDeleteProgram(shader_program); // Usun program shaderowy
    glDeleteBuffers(1, &VBO);       // Usun bufor wierzcholkow

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    fprintf(stdout, "Zasoby wyczyszczone.\n");
}

int main() {
    if (!init_sdl_opengl()) {
        fprintf(stderr, "Blad inicjalizacji! Zamykam aplikacje.\n");
        return 1;
    }

    emscripten_set_main_loop(main_loop, -1, 1);

    cleanup();
    return 0;
}
