\page Started Getting Started

# Getting Started

This article describes all the neccessary steps to compile your first application with the Madgine.


## Dependencies

The Madgine is built using CMake. It can be downloaded [here](https://cmake.org/download/). 
All source and library dependencies are included in the repository itself.
A compiler supporting C++20 is necessary.


## Building The Madgine

Download the source code from [GitHub](https://github.com/MadManRises/Madgine).

Even though you can compile the code directly from within the repository, it is recommended to use the Madgine as a library from source. All the necessary utility scripts are provided. To setup the library as source library, create your own CMakeLists.txt for your application and add the following:
```cmake
include(<root folder of local madgine repository>/cmake/MadgineSDK.cmake)
```
To create the executable add the following line to the end of your CmakeLists.txt:
```cmake
create_launcher(<name>)
```

Generate the build files using CMake for your preferred compiler/IDE. There are no additional CMake settings necessary. Build the solution and start the executable (it is called what you provided to `create_launcher` or MadgineLauncher if building from the repository itself), which can be found in the `bin` folder of your build directory. 
Congratulations, you successfully compiled the Madgine Framework. 


## Running The Madgine 

When you start the engine for the first time you are presented with a blank screen and only two items in the menu bar. By default all plugins are disabled, except those that are dependencies of the launcher itself. To get started you can open the Plugin Manager tool and enable all plugins you intend to use. Afterwards, you should create/load your project by providing the engine with the root folder of your game. For further information please refer to the [User Manual](@ref Manual).
