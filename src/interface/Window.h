#pragma once
#include <string>
#include <stdint.h>

#include "InputHandler.h"

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

    void update();

    SDL_Window* getHandle();
    uint32_t getFlags();

    int setFullscreen();
    int setWindowed();
    bool isFullscreen();
    bool isWindowed();

    void setBorderless();
    void setBordered();
    bool isBorderless();

    void setTitle(std::string title);
    void setResolution(int width, int height);

    void setCursorPos(int x, int y);
    void showCursor();
    void hideCursor();
    bool isCursorVisible();

    std::string title();
    int width();
    int height();

    InputManager& getInputManager();

    bool isAlive();

private:
    std::string m_title;
    int m_height;
    int m_width;

    bool m_fullscreen;
    bool m_borderless;

    struct SDL_Window* m_window{ nullptr };

    InputHandler input;

    void updateResolution();
};
}