#include "app.hpp"
#include <iostream>

namespace mrm {

using std::cout;
using std::endl;

const int SCREEN_WIDTH = 320;   // 640
const int SCREEN_HEIGHT = 180;  // 360

Scene App::dummyScene;

void Scene::load() {
    loaded = true;
}

void Scene::tryLoad() {
    if (!loaded) load();
}

App& App::getInstance() {
    static App theInstance;
    return theInstance;
}

void App::init(const char* title, int scale) {
    if (m_initComplete) return;

    m_scale = scale;
    m_screen_rect = {0, 0, SCREEN_WIDTH * scale, SCREEN_HEIGHT * scale};

    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    m_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * m_scale,
        SCREEN_HEIGHT * m_scale,
        SDL_WINDOW_RESIZABLE);
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    m_screen = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_RendererInfo info;
    SDL_GetRendererInfo(m_renderer, &info);
    cout << "Renderer name: " << info.name << endl;
    cout << "Texture formats: " << endl;
    for (Uint32 i = 0; i < info.num_texture_formats; i++) {
        cout << SDL_GetPixelFormatName(info.texture_formats[i]) << endl;
    }
    Uint32 format;
    SDL_QueryTexture(m_screen, &format, NULL, NULL, NULL);
    cout << "Screen texture format: " << SDL_GetPixelFormatName(format) << endl;
    if (format != SDL_PIXELFORMAT_ARGB8888) {
        cout << "Error in texture format" << endl;
    }

    m_initComplete = true;
}

void App::run() {
    m_running = true;

    SDL_Event event;
    while (m_running) {
        if (m_nextScene != nullptr) {
            m_currentScene = m_nextScene;
            m_nextScene = nullptr;
            m_currentScene->tryLoad();
        }

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    m_running = 0;
                    break;
            }
        }

        m_currentScene->update(0.0f);
        m_currentScene->draw(0.0f);

        // SDL_UpdateTexture(m_screen, NULL, screenBuffer, SCREEN_WIDTH * sizeof(Color));
        SDL_RenderCopy(m_renderer, m_screen, NULL, &m_screen_rect);
        SDL_RenderPresent(m_renderer);

        SDL_Delay(5);
    }
}

void App::requestExit() {
    m_running = false;
}

void App::setScene(Scene* newScene) {
    if (!m_initComplete) return;
    if (newScene != nullptr) {
        m_nextScene = newScene;
    } else {
        m_nextScene = &dummyScene;
    }
}

App::~App() {
    if (!m_initComplete) return;
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

}  // namespace mrm
