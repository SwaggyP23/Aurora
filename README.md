# Aurora 
[![License: MIT](https://img.shields.io/badge/License-MIT-blue)](https://github.com/SwaggyP23/Aurora/blob/main/LICENSE)
![Aurora Logo](/Resources/Icons/Auroraicon1.png?raw=true "Aurora")

- Aurora Engine is a very primitive 3D engine based on [Hazel 2D Engine](https://github.com/TheCherno/Hazel). 

- First was based on the book called [LearnOpenGL - Graphics Programming](https://learnopengl.com/) made by Joey de Vries.

***

## <ins>Building the project: </ins>
- Start by cloning the repository or forking it, and you will need to have the latest version of [premake5](https://premake.github.io/download/) downloaded

```
git clone --recursive https://github.com/SwaggyP23/Aurora.git
```

- If you do not have premake5 and you are on windows, the Premake5.exe is included in the repository in Tools/BuildSystem/PremakeBin

- And it is adviced to have [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) downloaded to easily generate the project files through premake5

- After setting up premake5, run the GenerateProject.bat file found in the Scripts file and this will generate the project files.

- If you are not using Visual Studio you will have to build the source files one by one or make your own make file and you will need to compile the libraries included. Or another option is to download their binaries from their corresponding sites!

## Profiling

1. First run the engine in profile mode so that it can collect all the data about most of the function and scopes specified.

2. After you end the profile session by exiting the program, go to the root directory of the project (*the one with your .sln file*) and from there go to SandBox/Luna -> Profiling.

3. Aurora provides two ways of profiling:
- - There is the way of profiling and visualizing using `chrome://tracing/` which provides a limited amount of data but is not bad.
- - There is also the more usable way of profiling with **Optick**!

### Using Optick

- Optick is a separate application that is also provided with the repository (*inside Tools folder*). If you are not on windows you will need to get the executable for your platform from [their repo}(https://github.com/bombomby/optick)
- Open the Optick app and then load the .opt files found in the Profiling/Optick folder and you will be able to visualize all your profiling data!

## <ins>Libraries Used:</ins>

- [assimp/zlib](https://github.com/assimp/assimp).
- [choc](https://github.com/Tracktion/choc).
- [entt](https://github.com/skypjack/entt).
- [Glad](https://github.com/Dav1dde/glad/tree/master).
- [glfw](https://github.com/glfw/glfw).
- [glm](https://github.com/g-truc/glm).
- [ImGui](https://github.com/ocornut/imgui).
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo).
- [optick](https://github.com/bombomby/optick).
- [spdlog](https://github.com/gabime/spdlog).
- [stb](https://github.com/nothings/stb).
- [yaml-cpp](https://github.com/jbeder/yaml-cpp).