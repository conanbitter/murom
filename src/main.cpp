#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>

using std::cout;
using std::endl;

const int SCREEN_WIDTH = 320;   // 640
const int SCREEN_HEIGHT = 180;  // 360
const int SCREEN_SCALE = 4;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *screen;
SDL_Rect screen_rect = {0, 0, SCREEN_WIDTH *SCREEN_SCALE, SCREEN_HEIGHT *SCREEN_SCALE};

struct alignas(uint32_t) Color {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;

    Color() : r{0}, g{0}, b{0}, a{255} {}
    Color(uint8_t r, uint8_t g, uint8_t b) : r{r}, g{g}, b{b}, a{255} {}
};

Color screenBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];

void putPixel(int x, int y, Color color) {
    screenBuffer[y][x] = color;
}

struct ScreenPoint {
    int x;
    int y;

    ScreenPoint() : x{0}, y{0} {}
    ScreenPoint(int x, int y) : x{x}, y{y} {}
};

class ITriEdge {
   public:
    virtual int next() { return 0; }
};

class TriEdgeLine : public ITriEdge {
   public:
    TriEdgeLine(int x) : x{x} {}
    int next() override {
        return x;
    }

   private:
    int x;
};

class TriEdgeGentle : public ITriEdge {
   public:
    TriEdgeGentle(int x1, int x2, int dx, int dy, int sign) : x{x1}, x2{x2}, sign{sign}, dx2{2 * dx}, dy2{2 * dy}, d{2 * dy - dx} {}
    int next() override {
        while (d <= 0) {
            if (x == x2) break;
            d += dy2;
            x += sign;
        }
        d -= dx2;
        return x;
    }

   private:
    int d;
    int dx2;
    int dy2;
    int x;
    int x2;
    int sign;
};

class TriEdgeSteep : public ITriEdge {
   public:
    TriEdgeSteep(int x, int dx, int dy, int sign) : x{x}, sign{sign}, dx2{2 * dx}, dy2{2 * dy}, d{2 * dx - dy} {}
    int next() override {
        int res = x;
        if (d > 0) {
            x += sign;
            d -= dy2;
        }
        d += dx2;
        return res;
    }

   private:
    int d;
    int dx2;
    int dy2;
    int x;
    int sign;
};

typedef std::unique_ptr<ITriEdge> TriEdge;

TriEdge getEdge(ScreenPoint a, ScreenPoint b) {
    int dx = b.x - a.x;
    int dy = b.y - a.y;

    if (dx == 0 || dy == 0) {
        return std::make_unique<TriEdgeLine>(b.x);
    }
    int sign = 1;
    if (dx < 0) {
        sign = -1;
        dx = -dx;
    }
    if (dx > dy) {
        return std::make_unique<TriEdgeGentle>(a.x, b.x, dx, dy, sign);
    } else {
        return std::make_unique<TriEdgeSteep>(a.x, dx, dy, sign);
    }
}

int max(int a, int b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
}

void sortPointsByY(ScreenPoint &a, ScreenPoint &b, ScreenPoint &c) {
    if (b.y < a.y) std::swap(a, b);
    if (c.y < a.y) std::swap(a, c);
    if (c.y < b.y) std::swap(b, c);
}

void drawScanLine(int left, int right, int y) {
    for (int x = left; x <= right; x++) {
        putPixel(x, y, Color(100, 200, 100));
    }
}

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    window = SDL_CreateWindow(
        "Murom",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH * SCREEN_SCALE,
        SCREEN_HEIGHT * SCREEN_SCALE,
        SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    cout << "Renderer name: " << info.name << endl;
    cout << "Texture formats: " << endl;
    for (Uint32 i = 0; i < info.num_texture_formats; i++) {
        cout << SDL_GetPixelFormatName(info.texture_formats[i]) << endl;
    }
    Uint32 format;
    SDL_QueryTexture(screen, &format, NULL, NULL, NULL);
    cout << "Screen texture format: " << SDL_GetPixelFormatName(format) << endl;
    if (format != SDL_PIXELFORMAT_ARGB8888) {
        cout << "Error in texture format" << endl;
    }

    SDL_Event event;
    bool working = true;

    Color bgColor = Color(92, 131, 181);
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            screenBuffer[y][x] = bgColor;
        }
    }

    // putPixel(10, 20, Color(255, 0, 0));

    ScreenPoint a(30, 10);
    ScreenPoint b(67, 22);
    ScreenPoint c(13, 57);

    sortPointsByY(a, b, c);

    TriEdge ab = getEdge(a, b);
    TriEdge ac = getEdge(a, c);
    TriEdge bc = getEdge(b, c);

    for (int y = a.y; y <= c.y; y++) {
        if (y < 0) continue;
        if (y >= SCREEN_HEIGHT) break;
        int e1 = y < b.y ? ab->next() : bc->next();
        int e2 = ac->next();
        int left = max(0, min(e1, e2));
        int right = min(SCREEN_WIDTH, max(e1, e2));
        drawScanLine(left, right, y);
        // putPixel(left, y, Color(100, 200, 100));
        // putPixel(right, y, Color(100, 200, 100));
    }
    putPixel(a.x, a.y, Color(100, 50, 50));
    putPixel(b.x, b.y, Color(100, 50, 50));
    putPixel(c.x, c.y, Color(100, 50, 50));

    while (working) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    working = 0;
                    break;
            }
        }

        SDL_UpdateTexture(screen, NULL, screenBuffer, SCREEN_WIDTH * sizeof(Color));
        SDL_RenderCopy(renderer, screen, NULL, &screen_rect);
        SDL_RenderPresent(renderer);

        SDL_Delay(5);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}