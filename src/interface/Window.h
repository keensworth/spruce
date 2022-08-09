#pragma once
#include <string>
#include <stdint.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace spr {
class Window {
public:
    Window();
    Window(std::string title);
    Window(std::string title, int width, int height);
    ~Window() {
        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    void init();
    void init(uint32_t flags);

    SDL_Window* getHandle();
    uint32_t getFlags();

    int setFullscreen();
    int setWindowed();

    void setBorderless();
    void setBordered();

    void setTitle(std::string title);
    void setResolution(int width, int height);

    std::string title();
    int width();
    int height();

private:
    std::string m_title;
    int m_height;
    int m_width;

    bool m_fullscreen;
    bool m_borderless;

    struct SDL_Window* m_window{ nullptr };

    void updateResolution();
};
}