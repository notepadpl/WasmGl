#include <SDL.h>
#include <SDL_opengl.h>
#include <emscripten.h>

// Wymiary okna
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window* gWindow = nullptr;
SDL_GLContext gContext;

// Inicjalizacja SDL i OpenGL
bool init() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("SDL nie mogło się zainicjalizować! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // Ustawienie wersji kontekstu OpenGL (WebGL 1.0 = OpenGL ES 2.0)
#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    // Tworzenie okna
    gWindow = SDL_CreateWindow("Blue OpenGL Window",
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               SCREEN_WIDTH, SCREEN_HEIGHT,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (gWindow == nullptr) {
        printf("Nie można utworzyć okna! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // Tworzenie kontekstu OpenGL
    gContext = SDL_GL_CreateContext(gWindow);
    if (gContext == nullptr) {
        printf("Nie można utworzyć kontekstu OpenGL! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    // Inicjalizacja OpenGL
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f); // niebieskie tło

    return true;
}

// Funkcja renderująca
void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(gWindow);
}

// Sprzątanie zasobów
void close() {
    SDL_GL_DeleteContext(gContext);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
}

// Główna pętla aplikacji
void main_loop() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            emscripten_cancel_main_loop();
        }
    }

    render();
}

// Punkt wejścia
int main(int argc, char* args[]) {
    if (!init()) {
        printf("Inicjalizacja nie powiodła się!\n");
        return 1;
    }

    atexit(close); // rejestruje close() przy końcu działania
    emscripten_set_main_loop(main_loop, 0, 1); // główna pętla
    return 0;
}
