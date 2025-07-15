#include "src/hello_triangle.hxx"

int main(int argc, char* argv[])
{
    CD3D12Triangle application{ "LearnD3D12 - Hello Window" };
    application.Run();

    return 0;
}