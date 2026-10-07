#include <iostream>

#define SDL_MAIN_USE_CALLBACKS 1
#include <algorithm>
#include <format>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <glm/glm.hpp>

struct Paddle {
    glm::vec2 pos;
    float width = 15.0f, height = 100.0f;
};

struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;

    Paddle paddle{{30.0f, 50.0f}, 15.0f, 100.0f};

    bool isUpPressed, isDownPressed;
    uint64_t lastTime = 0;
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return SDL_APP_FAILURE;
    }

    auto *as = new AppState();
    *appstate = as;

    if (!SDL_CreateWindowAndRenderer("Hi", 640, 480, SDL_WINDOW_RESIZABLE, &as->window, &as->renderer)) {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderVSync(as->renderer, SDL_RENDERER_VSYNC_DISABLED);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    auto *as = static_cast<AppState *>(appstate);

    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
            break;

        case SDL_EVENT_KEY_DOWN:
            if (event->key.key == SDLK_W) {
                as->isUpPressed = true;
            }
            if (event->key.key == SDLK_S) {
                as->isDownPressed = true;
            }
            if (event->key.key == SDLK_ESCAPE) {
                return SDL_APP_SUCCESS;
            }
            break;

        case SDL_EVENT_KEY_UP:
            if (event->key.key == SDLK_W) {
                as->isUpPressed = false;
            }
            if (event->key.key == SDLK_S) {
                as->isDownPressed = false;
            }
            break;

        default:
            return SDL_APP_CONTINUE;
            break;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto *as = static_cast<AppState *>(appstate);
    SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(as->renderer);

    uint64_t now = SDL_GetTicksNS();
    uint64_t elapsed = now - as->lastTime;
    float deltaTime = elapsed / 1e9f;
    as->lastTime = now;

    float paddleSpeed = 0;
    if (as->isUpPressed) { paddleSpeed -= 1.0f;}
    if (as->isDownPressed) { paddleSpeed += 1.0f;}
    paddleSpeed = paddleSpeed * 300;

    if (paddleSpeed != 0.0f) {
        as->paddle.pos.y += paddleSpeed * deltaTime;
    }

    if (as->paddle.pos.y <= 0) {
        as->paddle.pos.y = 0;
    } else if (as->paddle.pos.y > 900) {
        as->paddle.pos.y = 900;
    }

    // std::cout << as->paddle.pos.x << " " << as->paddle.pos.y << std::endl;

    SDL_FRect rect;
    rect.h = as->paddle.height;
    rect.w = as->paddle.width;
    rect.x = as->paddle.pos.x;
    rect.y = as->paddle.pos.y;

    SDL_SetRenderDrawColor(as->renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderFillRect(as->renderer, &rect);

    SDL_RenderPresent(as->renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    auto *as = static_cast<AppState *>(appstate);
    if (as) {
        SDL_DestroyRenderer(as->renderer);
        SDL_DestroyWindow(as->window);
        SDL_free(as);
    }
}