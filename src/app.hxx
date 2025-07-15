#pragma once
#ifndef APP_HXX
#define APP_HXX 

#include <string>
#include <iostream>

struct GLFWwindow;

class Application
{
public:
    Application(const std::string& title);
    virtual ~Application();
    void Run();

protected:

    virtual void Cleanup();
    virtual bool Initialize();

    virtual void onInit(GLFWwindow* window) =0;
    virtual bool onLoad() = 0;
    virtual void onRender() = 0;
    virtual void onUpdate() = 0;
    virtual void onDestroy() = 0;


private:
    GLFWwindow* _window = nullptr;
    int32_t _width = 0;
    int32_t _height = 0;
    std::string _title;
};

#endif // APP_HXX