#pragma once
#ifndef HELLO_WINDOW_HXX
#define HELLO_WINDOW_HXX

#include "app.hxx"

class HelloWindowApplication final : public Application
{
public:
    HelloWindowApplication(const std::string& title);

protected:
    bool Load() override;
    void Render() override;
    void Update() override;
};

#endif // HELLO_WINDOW_HXX