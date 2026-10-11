#include <iostream>

#define SDL_MAIN_USE_CALLBACKS 1

#include <algorithm>
#include <format>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <glm/glm.hpp>

struct Paddle {
    Paddle(glm::vec2 pos, float width, float height)
    : pos(pos), width(width), height(height) {}

    glm::vec2 pos;
    float width = 15.0f, height = 100.0f;

    bool isUpPressed{}, isDownPressed{};

    void Render(SDL_Renderer *renderer) {
        SDL_FRect rect;
        rect.h = height;
        rect.w = width;
        rect.x = pos.x;
        rect.y = pos.y;

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        SDL_RenderFillRect(renderer, &rect);
    }
    void ApplyVelocity(float deltaTime) {
        float paddleSpeed = 0;
        if (isDownPressed) {
            paddleSpeed += 1.0f;
        }
        if (isUpPressed) {
            paddleSpeed -= 1.0f;
        }
        paddleSpeed = paddleSpeed * 300;
        pos.y += paddleSpeed * deltaTime;
    }
    void HandleInput(SDL_Keycode input, bool isPressed) {
        if (!UpInput || !DownInput) {
            std::cout << "No inputs set!" << std::endl;
            return;
        }
        if (input == UpInput) {
            isUpPressed = isPressed;
        }
        if (input == DownInput) {
            isDownPressed = isPressed;
        }
    }

    void SetInput(SDL_Keycode Up, SDL_Keycode Down) {
        UpInput = Up;
        DownInput = Down;
    }

    void CheckBounds(int wHeight) {
        if (pos.y < 0) {
            pos.y = 0;
        } else if (pos.y > wHeight - height) {
            pos.y = wHeight - height;
        }
    };
private:
    SDL_Keycode UpInput, DownInput;
};

struct Ball {
    glm::vec2 pos;
    int radius = 10;

    void Render(SDL_Renderer *renderer) {
        int x = radius - 1;
        int y = 0;
        int dx = 1;
        int dy = 1;
        int err = dx - (radius << 1);
        while (x >= y) {
            SDL_RenderPoint(renderer, pos.x + x, pos.y + y);
            SDL_RenderPoint(renderer, pos.x + y, pos.y + x);
            SDL_RenderPoint(renderer, pos.x - y, pos.y + x);
            SDL_RenderPoint(renderer, pos.x - x, pos.y + y);
            SDL_RenderPoint(renderer, pos.x - x, pos.y - y);
            SDL_RenderPoint(renderer, pos.x - y, pos.y - x);
            SDL_RenderPoint(renderer, pos.x + y, pos.y - x);
            SDL_RenderPoint(renderer, pos.x + x, pos.y - y);
            if (err <= 0) {
                y++;
                err += dy;
                dy += 2;
            }
            if (err > 0) {
                x--;
                dx += 2;
                err += (-radius << 1) + dx;
            }
        }
    }
};

struct AppState {
    SDL_Window *window{};
    SDL_Renderer *renderer{};

    std::vector<Paddle> paddles{};
    Ball ball{};

    uint64_t lastTime = 0;
};

SDL_AppResult SDL_AppIterate(void *appstate) {
    auto *as = static_cast<AppState *>(appstate);
    SDL_SetRenderDrawColor(as->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(as->renderer);

    uint64_t now = SDL_GetTicksNS();
    uint64_t elapsed = now - as->lastTime;
    float deltaTime = elapsed / 1e9f;
    as->lastTime = now;

    if (as->paddles.empty()) {
        std::cout << "No paddles!" << std::endl;
        return SDL_APP_CONTINUE;
    }

    for (Paddle &paddle: as->paddles) {
        paddle.ApplyVelocity(deltaTime);
    }

    /*
     * Check Bounds
     */
    int height = 0;
    if (!SDL_GetWindowSize(as->window, nullptr, &height))
        std::cout << "Failed to get window size: " << SDL_GetError() << std::endl;

    for (Paddle &paddle : as->paddles) {
        paddle.CheckBounds(height);
    }

    /*
     * Draw Paddle
     */
    for (Paddle &paddle : as->paddles) {
        paddle.Render(as->renderer);
    }

    as->ball.Render(as->renderer);

    int radius = 10;
    int centerX = 150;
    int centerY = 150;

    SDL_RenderPresent(as->renderer);
    return SDL_APP_CONTINUE;
}

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

    Paddle paddle = Paddle{{50.0f, 50.0f}, 15.0f, 100.0f};
    paddle.SetInput(SDLK_W, SDLK_S);
    as->paddles.push_back(paddle);

    int width;
    SDL_GetWindowSize(as->window, &width, nullptr);

    paddle = Paddle{{width - 50.0f, 200.0f}, 15.0f, 100.0f};
    paddle.SetInput(SDLK_UP, SDLK_DOWN);
    as->paddles.push_back(paddle);

    as->ball = Ball{{640, 420}, 10};

    SDL_SetRenderVSync(as->renderer, SDL_RENDERER_VSYNC_DISABLED);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    auto *as = static_cast<AppState *>(appstate);

    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;

        case SDL_EVENT_KEY_DOWN:
            if (event->key.key == SDLK_ESCAPE) {
                return SDL_APP_SUCCESS;
            }

            for (Paddle &paddle : as->paddles) {
                paddle.HandleInput(event->key.key, true);
            }

            break;

        case SDL_EVENT_KEY_UP:
            for (Paddle &paddle : as->paddles) {
                paddle.HandleInput(event->key.key, false);
            }
            break;

        default:
            return SDL_APP_CONTINUE;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    auto *as = static_cast<AppState *>(appstate);
    if (as) {
        SDL_DestroyRenderer(as->renderer);
        SDL_DestroyWindow(as->window);
        delete as;
    }
}