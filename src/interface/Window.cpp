#include "Window.h"
#include "SDL_stdinc.h"
#include <string>

#include "../../external/volk/volk.h"

#define VK_NO_PROTOTYPES
#include <SDL2/SDL_vulkan.h>

namespace spr {
Window::Window(){
    m_title = "Spruce Application";
    m_width = 1280;
    m_height = 720;

    m_fullscreen = false;
    m_borderless = false;
    m_resized = false;
    m_relativeMouse = false;
}

Window::Window(std::string title){
    m_title = title;
    m_width = 1280;
    m_height = 720;

    m_fullscreen = false;
    m_borderless = false;
    m_resized = false;
    m_relativeMouse = false;
}

Window::Window(std::string title, uint32 width, uint32 height){
    m_title = title;
    m_width = width;
    m_height = height;

    m_fullscreen = false;
    m_borderless = false;
    m_resized = false;
    m_relativeMouse = false;
}

void Window::init(){
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

void Window::init(uint32 flags){
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

void Window::update(){
    input.update();
}

SDL_Window* Window::getHandle(){
    return m_window;
}

uint32 Window::getFlags(){
    return SDL_GetWindowFlags( m_window );
}

std::string Window::title(){
    return m_title;
}

uint32 Window::width(){
    return m_width;
}

uint32 Window::height(){
    return m_height;
}


void delay(int ms){
    int currTime = SDL_GetTicks();
    while (SDL_GetTicks() - currTime < ms){}
}


int Window::setFullscreen(){
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

void Window::setWindowed(){
    if (!m_fullscreen)
        return;

    m_fullscreen = false;

    SDL_SetWindowFullscreen(m_window, 0);
    delay(50);
    updateResolution();
}

void Window::setBorderless(){
    if (m_fullscreen)
        return; 

    m_borderless = true;
    SDL_SetWindowBordered(
        m_window,
        SDL_FALSE
    );
}

void Window::setBordered(){
    if (m_fullscreen)
        return; 

    m_borderless = false;
    SDL_SetWindowBordered(
        m_window,
        SDL_TRUE
    );
}

void Window::setResolution(uint32 width, uint32 height){
    if (m_fullscreen)
        return;

    SDL_SetWindowSize(
        m_window, 
        width, 
        height
    );

    updateResolution();
}

void Window::updateResolution(){
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

void Window::setTitle(std::string title){
    m_title = title;
    SDL_SetWindowTitle(m_window, title.c_str());
}


InputManager& Window::getInputManager(){
    return input.getInputManager();
}

void Window::addEventListener(std::function<void (SDL_Event* e)> func){
    input.addEventListener(func);
}

bool Window::isAlive(){
    return !input.quit;
}

bool Window::isFullscreen(){
    return m_fullscreen;
}

bool Window::isWindowed(){
    return !m_fullscreen;
}

bool Window::isBorderless(){
    return m_borderless;
}

bool Window::isMinimzed(){
    return SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED;
}

void Window::showCursor(){
    SDL_ShowCursor(SDL_ENABLE);
}

void Window::hideCursor(){
    SDL_ShowCursor(SDL_DISABLE);
}

bool Window::isCursorVisible(){
    return SDL_ShowCursor(SDL_QUERY);
}

void Window::setCursorPos(uint32 x, uint32 y){
    SDL_WarpMouseInWindow(m_window, x, y);
}

void Window::setRelativeMouse(bool enable){
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, enable ? "1" : "0", SDL_HINT_OVERRIDE);
    enable ? hideCursor() : showCursor();
    SDL_SetRelativeMouseMode((SDL_bool)enable);
    m_relativeMouse = enable;
}

bool Window::isRelativeMouse(){
    return m_relativeMouse;
}

bool Window::resized(){
    return m_resized;
}

void Window::resizeHandled(){
    m_resized = false;
}

}