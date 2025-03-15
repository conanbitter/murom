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
    bool isInitComplete;
    bool isRunning;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* screen;
    SDL_Rect screen_rect;
    Scene* currentScene;

    static Scene dummyScene;

    App() : isInitComplete{false}, currentScene{&dummyScene} {};
    App(const App&) = delete;
    App& operator=(const App&) = delete;
    ~App();
};

}  // namespace mrm