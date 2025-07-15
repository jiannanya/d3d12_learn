#pragma once
#ifndef HELLO_WINDOW_HXX
#define HELLO_WINDOW_HXX

#include "src/app.hxx"

class HelloWindowApplication final : public Application
{
public:
    HelloWindowApplication(const std::string& title);

protected:
    void onInit(GLFWwindow* window) override;
    bool onLoad() override;
    void onRender() override;
    void onUpdate() override;
    void onDestroy() override;
};

#endif // HELLO_WINDOW_HXX