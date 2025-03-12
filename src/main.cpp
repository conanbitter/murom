#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <algorithm>

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

struct Vec3 {
    float x;
    float y;
    float z;

    Vec3() : x{0.0f}, y{0.0f}, z{0.0f} {}
};

struct Vertex {
    Vec3 pos;
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
    TriEdgeGentle(int x1, int x2, int dx, int dy, int sign) : x{x1}, x2{x2}, sign{sign}, dx2{2 * dx}, dy2{2 * dy}, d{2 * dx - dy} {}
    int next() override {
        int res = x;
        while (d > 0) {
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

void drawTriangle(ScreenPoint a, ScreenPoint b, ScreenPoint c) {
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
    }
    putPixel(a.x, a.y, Color(100, 50, 50));
    putPixel(b.x, b.y, Color(100, 50, 50));
    putPixel(c.x, c.y, Color(100, 50, 50));
}

void drawTriangle2(ScreenPoint a, ScreenPoint b, ScreenPoint c, Color color) {
    int xmin = std::max(std::min({a.x, b.x, c.x}), 0);
    int xmax = std::min(std::max({a.x, b.x, c.x}), SCREEN_WIDTH - 1);
    int ymin = std::max(std::min({a.y, b.y, c.y}), 0);
    int ymax = std::min(std::max({a.y, b.y, c.y}), SCREEN_HEIGHT - 1);

    for (int y = ymin; y <= ymax; y++) {
        for (int x = xmin; x <= xmax; x++) {
            if ((a.x - b.x) * (y - a.y) - (a.y - b.y) * (x - a.x) >= 0 &&
                (b.x - c.x) * (y - b.y) - (b.y - c.y) * (x - b.x) >= 0 &&
                (c.x - a.x) * (y - c.y) - (c.y - a.y) * (x - c.x) >= 0) {
                if (screenBuffer[y][x].b != 100) {
                    putPixel(x, y, color);
                } else {
                    putPixel(x, y, Color(200, 50, 50));
                }
            }
        }
    }
    // putPixel(a.x, a.y, Color(100, 50, 50));
    // putPixel(b.x, b.y, Color(100, 50, 50));
    // putPixel(c.x, c.y, Color(100, 50, 50));
}

void drawTriangle3(const Vertex &a, const Vertex &b, const Vertex &c, Color color) {
    int x1 = a.pos.x * SCREEN_WIDTH;
    int y1 = a.pos.y * SCREEN_HEIGHT;
    int x2 = b.pos.x * SCREEN_WIDTH;
    int y2 = b.pos.y * SCREEN_HEIGHT;
    int x3 = c.pos.x * SCREEN_WIDTH;
    int y3 = c.pos.y * SCREEN_HEIGHT;

    int xmin = std::max(std::min({x1, x2, x3}), 0);
    int xmax = std::min(std::max({x1, x2, x3}), SCREEN_WIDTH - 1);
    int ymin = std::max(std::min({y1, y2, y3}), 0);
    int ymax = std::min(std::max({y1, y2, y3}), SCREEN_HEIGHT - 1);

    int Dx1 = y1 - y2;
    int Dx2 = y2 - y3;
    int Dx3 = y3 - y1;

    int Dy1 = x1 - x2;
    int Dy2 = x2 - x3;
    int Dy3 = x3 - x1;

    int S1 = Dy1 * (ymin - y1) - Dx1 * (xmin - x1);
    int S2 = Dy2 * (ymin - y2) - Dx2 * (xmin - x2);
    int S3 = Dy3 * (ymin - y3) - Dx3 * (xmin - x3);

    if (Dx1 < 0 || (Dx1 == 0 && Dy1 > 0)) S1++;
    if (Dx2 < 0 || (Dx2 == 0 && Dy2 > 0)) S2++;
    if (Dx3 < 0 || (Dx3 == 0 && Dy3 > 0)) S3++;

    for (int y = ymin; y <= ymax; y++) {
        int P1 = S1;
        int P2 = S2;
        int P3 = S3;
        for (int x = xmin; x <= xmax; x++) {
            if (P1 > 0 && P2 > 0 && P3 > 0) {
                if (screenBuffer[y][x].b != 100) {
                    putPixel(x, y, color);
                } else {
                    putPixel(x, y, Color(200, 50, 50));
                }
            }
            P1 -= Dx1;
            P2 -= Dx2;
            P3 -= Dx3;
        }
        S1 += Dy1;
        S2 += Dy2;
        S3 += Dy3;
    }
    putPixel(x1, y1, Color(100, 50, 50));
    putPixel(x2, y2, Color(100, 50, 50));
    putPixel(x3, y3, Color(100, 50, 50));
}

Color bgColor = Color(92, 131, 181);

void clearScreen() {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            screenBuffer[y][x] = bgColor;
        }
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

    ScreenPoint a(30, 10);
    ScreenPoint b(67, 22);
    ScreenPoint c(13, 57);
    ScreenPoint d(13, 57);

    Vertex av, bv, cv, dv;

    double angle = 0.0;

    while (working) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    working = 0;
                    break;
            }
        }

        angle += 0.002;

        a.x = 160 + cos(angle) * 100;
        a.y = 90 + sin(angle) * 50;
        b.x = 160 + cos(angle + 3.0) * 100;
        b.y = 90 + sin(angle + 3.0) * 50;
        c.x = 160 + cos(angle + 1.2) * 100;
        c.y = 90 + sin(angle + 1.2) * 50;
        d.x = 160 + cos(angle + 4) * 100;
        d.y = 90 + sin(angle + 4) * 50;

        av.pos.x = 0.5f + cosf(angle) * 0.3f;
        av.pos.y = 0.5f + sinf(angle) * 0.3f;
        bv.pos.x = 0.5f + cosf(angle + 3.0) * 0.3f;
        bv.pos.y = 0.5f + sinf(angle + 3.0) * 0.3f;
        cv.pos.x = 0.5f + cosf(angle + 1.2) * 0.3f;
        cv.pos.y = 0.5f + sinf(angle + 1.2) * 0.3f;
        dv.pos.x = 0.5f + cosf(angle + 4) * 0.3f;
        dv.pos.y = 0.5f + sinf(angle + 4) * 0.3f;

        clearScreen();
        // drawTriangle2(a, b, c, Color(100, 200, 100));
        //  drawTriangle2(d, b, a, Color(200, 200, 100));
        drawTriangle3(av, bv, cv, Color(100, 200, 100));
        drawTriangle3(dv, bv, av, Color(200, 200, 100));

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