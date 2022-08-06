# Aurora
- Aurora Engine is a very primitive 3D engine based on [Hazel 2D Engine](https://github.com/TheCherno/Hazel). 

- First was based on the book called [LearnOpenGL - Graphics Programming](https://learnopengl.com/) made by Joey de Vries.

## <ins>Building the project: </ins>
- Start by cloning the repository or forking it, and you will need to have the latest version of [premake5](https://premake.github.io/download/) downloaded

- If you do not have premake5 and you are on windows, its .exe is included in the repository in Tools/BuildSystem/PremakeBin

- And it is adviced to have [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) downloaded to easily generate the project files through premake5

- After donwloading premake5 run the GenerateProject batch file found in the main directory and this will create the project files.

- If you are not using Visual Studio you will have to build the source files one by one or make your own make file and you will need to compile the libraries included. Or another option is to download their binaries from their corresponding sites!

- Optick.exe is a tool that is used to profile and time for the engine. To use it you will need to load the .opt files that will be produced when you run the engine in profile mode