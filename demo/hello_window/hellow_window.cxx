#include "hello_window.hxx"

HelloWindowApplication::HelloWindowApplication(const std::string& title)
    : Application(title)
{
}
void HelloWindowApplication::onInit(GLFWwindow* window)
{
    // Initialization code can go here if needed
}

bool HelloWindowApplication::onLoad()
{
    return true;
}


void HelloWindowApplication::onUpdate()
{
}

void HelloWindowApplication::onRender()
{
}

void HelloWindowApplication::onDestroy()
{
    // Cleanup code can go here if needed
}