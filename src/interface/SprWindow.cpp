#include "SprWindow.h"
#include "SDL_stdinc.h"
#include <string>

#include "../../external/volk/volk.h"

#define VK_NO_PROTOTYPES
#include <SDL2/SDL_vulkan.h>

namespace spr {
SprWindow::SprWindow(){
    m_title = "Spruce Application";
    m_width = 1280;
    m_height = 720;

    m_fullscreen = false;
    m_borderless = false;
    m_resized = false;
    m_relativeMouse = false;
}

SprWindow::SprWindow(std::string title){
    m_title = title;
    m_width = 1280;
    m_height = 720;

    m_fullscreen = false;
    m_borderless = false;
    m_resized = false;
    m_relativeMouse = false;
}

SprWindow::SprWindow(std::string title, uint32 width, uint32 height){
    m_title = title;
    m_width = width;
    m_height = height;

    m_fullscreen = false;
    m_borderless = false;
    m_resized = false;
    m_relativeMouse = false;
}

void SprWindow::init(){
    SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
	
	m_window = SDL_CreateWindow(
		m_title.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		m_width,
		m_height,
		flags
	);

    SDL_SetWindowResizable(m_window, SDL_FALSE);
}

void SprWindow::init(uint32 flags){
    SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags windowFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | flags);
	
	m_window = SDL_CreateWindow(
		m_title.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		m_width,
		m_height,
		windowFlags
	);

    SDL_SetWindowResizable(m_window, SDL_FALSE);
}

void SprWindow::update(){
    input.update();
}

SDL_Window* SprWindow::getHandle(){
    return m_window;
}

uint32 SprWindow::getFlags(){
    return SDL_GetWindowFlags( m_window );
}

std::string SprWindow::title(){
    return m_title;
}

uint32 SprWindow::width(){
    return m_width;
}

uint32 SprWindow::height(){
    return m_height;
}


void delay(int ms){
    int currTime = SDL_GetTicks();
    while (SDL_GetTicks() - currTime < ms){}
}


int SprWindow::setFullscreen(){
    m_fullscreen = true;
    int full;
    
    if (m_borderless){ // borderless
        full = SDL_SetWindowFullscreen(
            m_window, 
            SDL_WINDOW_FULLSCREEN
        );
    } else {           // bordered
        full = SDL_SetWindowFullscreen(
            m_window, 
            SDL_WINDOW_FULLSCREEN_DESKTOP
        );
    }

    updateResolution();

    return full;
}

void SprWindow::setWindowed(){
    if (!m_fullscreen)
        return;

    m_fullscreen = false;

    SDL_SetWindowFullscreen(m_window, 0);
    delay(50);
    updateResolution();
}

void SprWindow::setBorderless(){
    if (m_fullscreen)
        return; 

    m_borderless = true;
    SDL_SetWindowBordered(
        m_window,
        SDL_FALSE
    );
}

void SprWindow::setBordered(){
    if (m_fullscreen)
        return; 

    m_borderless = false;
    SDL_SetWindowBordered(
        m_window,
        SDL_TRUE
    );
}

void SprWindow::setResolution(uint32 width, uint32 height){
    if (m_fullscreen)
        return;

    SDL_SetWindowSize(
        m_window, 
        width, 
        height
    );

    updateResolution();
}

void SprWindow::updateResolution(){
    SDL_PumpEvents();

    // get fullscreen resolution
    int h,w;
    SDL_Vulkan_GetDrawableSize(m_window, &w, &h);
    if (h==m_height && w == m_width)
        return;
    
    // update window size
    m_width = w;
    m_height = h;

    // mark window as dirty
    if (!m_resized)
        m_resized = true;
}

void SprWindow::setTitle(std::string title){
    m_title = title;
    SDL_SetWindowTitle(m_window, title.c_str());
}


InputManager& SprWindow::getInputManager(){
    return input.getInputManager();
}

void SprWindow::addEventListener(std::function<void (SDL_Event* e)> func){
    input.addEventListener(func);
}

bool SprWindow::isAlive(){
    return !input.quit;
}

bool SprWindow::isFullscreen(){
    return m_fullscreen;
}

bool SprWindow::isWindowed(){
    return !m_fullscreen;
}

bool SprWindow::isBorderless(){
    return m_borderless;
}

bool SprWindow::isMinimzed(){
    return SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED;
}

void SprWindow::showCursor(){
    SDL_ShowCursor(SDL_ENABLE);
}

void SprWindow::hideCursor(){
    SDL_ShowCursor(SDL_DISABLE);
}

bool SprWindow::isCursorVisible(){
    return SDL_ShowCursor(SDL_QUERY);
}

void SprWindow::setCursorPos(uint32 x, uint32 y){
    SDL_WarpMouseInWindow(m_window, x, y);
}

void SprWindow::setRelativeMouse(bool enable){
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, enable ? "1" : "0", SDL_HINT_OVERRIDE);
    enable ? hideCursor() : showCursor();
    SDL_SetRelativeMouseMode((SDL_bool)enable);
    m_relativeMouse = enable;
}

bool SprWindow::isRelativeMouse(){
    return m_relativeMouse;
}

bool SprWindow::resized(){
    return m_resized;
}

void SprWindow::resizeHandled(){
    m_resized = false;
}

}