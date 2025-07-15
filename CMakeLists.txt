cmake_minimum_required(VERSION 3.20)

# if(DEFINED $ENV{VCPKG_TOOLCHAIN_FILE})
#   # set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
#   #   CACHE STRING "")

# endif()

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_TOOLCHAIN_FILE}" 
    CACHE STRING "")

project(d3d12_learn)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    main.cxx
    src/app.cxx
    src/hello_triangle.cxx
    src/hello_triangle.hxx
    src/app.hxx
)



target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glad::glad
    glm::glm
    spdlog::spdlog
    imgui::imgui
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    d3d12 
    dxgi 
    d3dcompiler 
    dxguid
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    src 
    dx_include/directx
    dx_include/dxguids
    dx_include/wsl
)


target_include_directories(${PROJECT_NAME} PRIVATE
    ${GLFW3_INCLUDE_DIRS}
    ${GLAD_INCLUDE_DIRS}
    ${Stb_INCLUDE_DIR}
)