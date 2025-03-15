#pragma once
#include <SDL.h>

namespace mrm {

class Scene {
   public:
    Scene() : loaded{false} {}
    virtual void load();
    virtual void update(float delta) {}
    virtual void draw(float alpha) {}

   private:
    void tryLoad();
    bool loaded;

    friend class App;
};

class App {
   public:
    static App& getInstance();
    void init(const char* title, int scale = 1);
    void run();
    void requestExit();
    void setScene(Scene* newScene);

   private:
    bool m_initComplete;
    bool m_running;
    int m_scale;
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_screen;
    SDL_Rect m_screen_rect;
    Scene* m_currentScene;
    Scene* m_nextScene;

    static Scene dummyScene;

    App() : m_initComplete{false}, m_currentScene{&dummyScene}, m_nextScene{nullptr} {};
    App(const App&) = delete;
    App& operator=(const App&) = delete;
    ~App();
};

}  // namespace mrm