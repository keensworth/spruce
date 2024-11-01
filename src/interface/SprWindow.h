#pragma once
#include <string>
#include <stdint.h>

#include "InputHandler.h"

namespace spr {
class SprWindow {
public:
    SprWindow();
    SprWindow(std::string title);
    SprWindow(std::string title, uint32 width, uint32 height);
    ~SprWindow() {
        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

    void init();
    void init(uint32 flags);

    void update();

    SDL_Window* getHandle();
    uint32 getFlags();

    int setFullscreen();
    void setWindowed();
    bool isFullscreen();
    bool isWindowed();
    bool isMinimzed();

    void setBorderless();
    void setBordered();
    bool isBorderless();

    bool resized();
    void resizeHandled();

    void setTitle(std::string title);
    void setResolution(uint32 width, uint32 height);

    void setCursorPos(uint32 x, uint32 y);
    void showCursor();
    void hideCursor();
    bool isCursorVisible();
    void setRelativeMouse(bool enable);
    bool isRelativeMouse();

    std::string title();
    uint32 width();
    uint32 height();

    InputManager& getInputManager();

    void addEventListener(std::function<void (SDL_Event* e)> func);

    bool isAlive();

private:
    std::string m_title;
    uint32 m_height;
    uint32 m_width;

    bool m_fullscreen;
    bool m_borderless;
    bool m_resized;
    bool m_relativeMouse;

    struct SDL_Window* m_window{ nullptr };

    InputHandler input;

    void updateResolution();
};
}