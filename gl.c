#include <SDL2/SDL.h>
#include <emscripten.h>
#include <GLES2/gl2.h> // W Emscripten uzywamy GLES2/gl2.h dla WebGL 2.0
#include <stdio.h>

SDL_Window* window = NULL;
SDL_GLContext gl_context;

// Wymiary okna
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

// Funkcja inicjalizujaca SDL i OpenGL
int init_sdl_opengl() {
    // Inicjalizacja SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Nie mozna zainicjalizowac SDL: %s\n", SDL_GetError());
        return 0;
    }

    // Ustawienie atrybutow OpenGL/WebGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2); // Wymus WebGL 2.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Utworzenie okna z flaga SDL_WINDOW_OPENGL
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

    // Utworzenie kontekstu OpenGL/WebGL
    gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL) {
        fprintf(stderr, "Nie mozna utworzyc kontekstu OpenGL: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    // Ustawienie biezacego kontekstu GL
    if (SDL_GL_MakeCurrent(window, gl_context) < 0) {
        fprintf(stderr, "Nie mozna ustawic biezacego kontekstu GL: %s\n", SDL_GetError());
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    // Konfiguracja widoku OpenGL
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // Ciemny turkus na tle

    fprintf(stdout, "SDL i OpenGL zainicjalizowane pomyslnie.\n");
    return 1;
}

// Funkcja renderujaca scene OpenGL
void render_scene() {
    glClear(GL_COLOR_BUFFER_BIT); // Wyczyszczenie bufora koloru

    // Rysowanie prostego trojkata
    glBegin(GL_TRIANGLES);
        glColor3f(1.0f, 0.0f, 0.0f); // Czerwony wierzcholek
        glVertex2f(0.0f, 0.5f);

        glColor3f(0.0f, 1.0f, 0.0f); // Zielony wierzcholek
        glVertex2f(-0.5f, -0.5f);

        glColor3f(0.0f, 0.0f, 1.0f); // Niebieski wierzcholek
        glVertex2f(0.5f, -0.5f);
    glEnd();

    SDL_GL_SwapWindow(window); // Zamiana buforow (wyswietlenie renderowanej sceny)
}

// Glowna petla aplikacji
void main_loop() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            emscripten_cancel_main_loop(); // Anulowanie petli glownej Emscripten
        }
    }

    render_scene();
}

// Funkcja czyszczaca zasoby
void cleanup() {
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

    // Uruchomienie glownej petli Emscripten
    // Drugi argument to liczba FPS (-1 dla nieograniczonej)
    // Trzeci argument (1) oznacza, ze petla powinna byc uruchomiona natychmiast
    emscripten_set_main_loop(main_loop, -1, 1);

    cleanup(); // Wykonane po emscripten_cancel_main_loop()
    return 0;
}
