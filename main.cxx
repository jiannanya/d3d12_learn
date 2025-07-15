#include "demo/hellow_texture/hellow_texture.hxx"
// #include "demo/hellow_triangle/hello_triangle.hxx"

int main(int argc, char* argv[])
{
    CD3D12Texture application{ "LearnD3D12 - Hello Texture" };
    application.Run();

    return 0;
}