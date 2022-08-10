#include "Window.h"

namespace spr {
Window::Window(){
    m_title = "Spruce Application";
    m_width = 1280;
    m_height = 720;

    m_fullscreen = false;
    m_borderless = false;
}

Window::Window(std::string title){
    m_title = title;
    m_width = 1280;
    m_height = 720;

    m_fullscreen = false;
    m_borderless = false;
}

Window::Window(std::string title, int width, int height){
    m_title = title;
    m_width = width;
    m_height = height;

    m_fullscreen = false;
    m_borderless = false;
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
}

void Window::init(uint32_t flags){
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
}


void Window::update(){
    input.update();
}

SDL_Window* Window::getHandle(){
    return m_window;
}

uint32_t Window::getFlags(){
    return SDL_GetWindowFlags( m_window );
}

std::string Window::title(){
    return m_title;
}

int Window::width(){
    return m_width;
}

int Window::height(){
    return m_height;
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

int Window::setWindowed(){
    if (!m_fullscreen)
        return 0;

    m_fullscreen = false;

    // fake fullscreen
    int full = SDL_SetWindowFullscreen(
            m_window, 
            SDL_WINDOW_FULLSCREEN_DESKTOP
    );

    updateResolution();

    return full;
}

void Window::updateResolution(){
    // get fullscreen resolution
    SDL_DisplayMode DM;
    if (SDL_GetCurrentDisplayMode(0, &DM) != 0) {
        SDL_Log("SDL_GetCurrentDisplayMode failed: %s", SDL_GetError());
        return;
    };
    m_width = DM.w;
    m_height = DM.h;
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

void Window::setTitle(std::string title){
    m_title = title;
    SDL_SetWindowTitle(m_window, title.c_str());
}

void Window::setResolution(int width, int height){
    if (m_fullscreen)
        return;
    
    m_width = width;
    m_height = height;

    SDL_SetWindowSize(
        m_window, 
        m_width, 
        m_height
    );
}

InputManager& Window::getInputManager(){
    return input.getInputManager();
}

bool Window::isAlive(){
    return !input.quit;
}

bool Window::isFullscreen(){
    return SDL_GetWindowFlags(m_window) & SDL_WINDOW_FULLSCREEN;;
}

bool Window::isWindowed(){
    return !isFullscreen();
}

bool Window::isBorderless(){
    return SDL_GetWindowFlags(m_window) & SDL_WINDOW_BORDERLESS;
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

void Window::setCursorPos(int x, int y){
    SDL_WarpMouseInWindow(m_window, x, y);
}

}