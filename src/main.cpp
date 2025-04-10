#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <algorithm>

#include "app.hpp"

using mrm::SCREEN_HEIGHT;
using mrm::SCREEN_WIDTH;

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

struct Vec3 {
    float x;
    float y;
    float z;

    Vec3() : x{0.0f}, y{0.0f}, z{0.0f} {}
    Vec3(float x, float y, float z) : x{x}, y{y}, z{z} {}
    Color toColor() const {
        return Color(
            std::min((int)(x * 256.0), 255),
            std::min((int)(y * 256.0), 255),
            std::min((int)(z * 256.0), 255));
    }
};

struct Vec2 {
    float x;
    float y;

    Vec2() : x{0.0f}, y{0.0f} {}
    Vec2(float x, float y) : x{x}, y{y} {}
    Vec2(const Vec3 &other) : x{other.x}, y{other.y} {};
};

Vec2 operator-(const Vec2 &a, const Vec2 &b) {
    return Vec2(a.x - b.x, a.y - b.y);
}

Vec2 operator+(const Vec2 &a, const Vec2 &b) {
    return Vec2(a.x + b.x, a.y + b.y);
}

Vec2 operator*(const Vec2 &a, const float &k) {
    return Vec2(a.x * k, a.y * k);
}

Vec2 operator*(const float &k, const Vec2 &a) {
    return Vec2(a.x * k, a.y * k);
}

Vec3 operator+(const Vec3 &a, const Vec3 &b) {
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec3 operator*(const Vec3 &a, const float &k) {
    return Vec3(a.x * k, a.y * k, a.z * k);
}

Vec3 operator*(const float &k, const Vec3 &a) {
    return Vec3(a.x * k, a.y * k, a.z * k);
}

float dot(const Vec2 &a, const Vec2 &b) {
    return a.x * b.x + a.y * b.y;
}

struct Vertex {
    Vec3 pos;
    Vec3 color;
    Vec2 uv;
};

void barycentric(const Vec2 &p, const Vec2 &a, const Vec2 &b, const Vec2 &c, float &u, float &v, float &w) {
    Vec2 v0 = b - a, v1 = c - a, v2 = p - a;
    float d00 = dot(v0, v0);
    float d01 = dot(v0, v1);
    float d11 = dot(v1, v1);
    float d20 = dot(v2, v0);
    float d21 = dot(v2, v1);
    float denom = d00 * d11 - d01 * d01;
    v = (d11 * d20 - d01 * d21) / denom;
    w = (d00 * d21 - d01 * d20) / denom;
    u = 1.0f - v - w;
    u = std::clamp(u, 0.0f, 1.0f);
    v = std::clamp(v, 0.0f, 1.0f);
    w = std::clamp(w, 0.0f, 1.0f);
}

void drawPixel(int x, int y, const Vertex &a, const Vertex &b, const Vertex &c) {
    Vec2 a2d(a.pos);
    Vec2 b2d(b.pos);
    Vec2 c2d(c.pos);
    Vec2 pos(((float)x + 0.5f) / (float)SCREEN_WIDTH,
             ((float)y + 0.5f) / (float)SCREEN_HEIGHT);
    float u, v, w;
    barycentric(pos, a2d, b2d, c2d, u, v, w);

    Vec3 color = a.color * u + b.color * v + c.color * w;
    Vec2 uv = a.uv * u + b.uv * v + c.uv * w;
    putPixel(x, y, Color(255 * uv.x, 255 * uv.y, 0));  // col.toColor()
}

void drawTriangle(const Vertex &a, const Vertex &b, const Vertex &c) {
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
                drawPixel(x, y, a, b, c);
            }
            P1 -= Dx1;
            P2 -= Dx2;
            P3 -= Dx3;
        }
        S1 += Dy1;
        S2 += Dy2;
        S3 += Dy3;
    }
}

Color bgColor = Color(92, 131, 181);

void clearScreen() {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            screenBuffer[y][x] = bgColor;
        }
    }
}

class TestScene : public mrm::Scene {
   public:
    void load() override {
        a.color = Vec3(0.3, 0.8, 0.3);
        b.color = Vec3(0.8, 0.3, 0.3);
        c.color = Vec3(0.3, 0.8, 0.8);
        d.color = Vec3(0.8, 0.8, 0.3);
        a.uv = Vec2(0.0, 1.0);
        b.uv = Vec2(1.0, 0.0);
        c.uv = Vec2(0.0, 0.0);
        d.uv = Vec2(1.0, 1.0);
    }

    void update(float delta) override {
        angle += 0.002;

        a.pos.x = 0.5f + cosf(angle) * 0.3f;
        a.pos.y = 0.5f + sinf(angle) * 0.3f;
        b.pos.x = 0.5f + cosf(angle + 3.0) * 0.3f;
        b.pos.y = 0.5f + sinf(angle + 3.0) * 0.3f;
        c.pos.x = 0.5f + cosf(angle + 1.2) * 0.3f;
        c.pos.y = 0.5f + sinf(angle + 1.2) * 0.3f;
        d.pos.x = 0.5f + cosf(angle + 4) * 0.3f;
        d.pos.y = 0.5f + sinf(angle + 4) * 0.3f;
    }

    void draw(float alpha) override {
        clearScreen();
        drawTriangle(a, b, c);
        drawTriangle(d, b, a);
    }

   private:
    Vertex a, b, c, d;
    double angle = 0.0;
};

int main(int argc, char *argv[]) {
    TestScene scene;
    mrm::App &app = mrm::App::getInstance();
    app.init("Murom", 4);
    app.setScene(&scene);
    app.run();
    return 0;
}