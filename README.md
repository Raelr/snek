# Vk-Sandbox (to be renamed)

Vk-Sandbox is an experimental repository, intended to explore the Vulkan API. The intent is to turn this into a renderer which can be extended to other applications.

### Current Compatibility

| OS          | Default Compiler | Last Manual Build  | Compile Status                                                                                                                                                                                 |
| ----------- | ---------------- | ------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **macOS**   | Clang++          | `Big Sur 11.6.3`   | [![macOS](https://github.com/CapsCollective/vulkan-cpp-starter/actions/workflows/macOS.yml/badge.svg)](https://github.com/CapsCollective/vulkan-cpp-starter/actions/workflows/macOS.yml)       |
| **Linux**   | G++              | `Ubuntu-20.04.2.0` | [![Ubuntu](https://github.com/CapsCollective/vulkan-cpp-starter/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/CapsCollective/vulkan-cpp-starter/actions/workflows/ubuntu.yml)    |
| **Windows** | MinGW (G++)      | `Windows 10 19041` | [![Windows](https://github.com/CapsCollective/vulkan-cpp-starter/actions/workflows/windows.yml/badge.svg)](https://github.com/CapsCollective/vulkan-cpp-starter/actions/workflows/windows.yml) |

## Getting Started

### Dependencies and Pre-Requisites

Before building the respository, you'll need the following:

1. [CMAKE](https://cmake.org/) - required for building `glfw`.
2. **WINDOWS ONLY**: This project relies primarily on Makefiles for compilation. Our tool of choice for compiling cpp on Windows is [Mingw64](http://mingw-w64.org/doku.php), since it's fully compatible with other operating system compilation methods. A comprehensive guide for installing Mingw64 can be found [here](https://www.youtube.com/watch?v=aXF4A5UeSeM). Once installed, you should be able to pull in your compiler of choice (we recommend gcc or clang).
3. Vulkan - This can be configured either via the SDK, or can be built manually. For a quick start, we recommend setting up the Vulkan SDK before building the project, as this will reduce compile times significantly. Manually building the SDK takes time and is only recommended for release builds. Setup for the SDK can be found below:

   3.1 Vulkan - the SDK can be downloaded from the [LunarG website](https://vulkan.lunarg.com/).

   3.2. Once the Vulkan SDK is installed, you'll want to make sure that both the `VULKAN_SDK` and `VK_VERSION` environment variables are set on your system. Platform specific instructions can be found below:

   - **macOS & Linux:**
     Both MacOS and Linux have a similar process for setting their environment variables. The only difference between the two is their SDK directory structure. The two directory structures have been included below:

     ```bash
     # For macOS
     $ export VULKAN_SDK=<VULKAN_INSTALL_DIR>/VulkanSDK/<VERSION>/macOS

     # for Linux
     $ export VULKAN_SDK=<VULKAN_INSTALL_DIR>/VulkanSDK/<VERSION>/x86_64
     ```

     Remember to substitute `<VULKAN_INSTALL_DIR>` with the directory Vulkan was installed into, and `<VERSION>` with your Vulkan version, i.e: 1.2.176.1.

   - **Windows:**
     For Windows, the SDK should be automatically installed to your `C:` directory. To set these variables, you can set the following:
     ```shell
     > setx VULKAN_SDK = <VULKAN_INSTALL_DIR>/VulkanSDK/<VERSION>
     ```
     For more information about this syntax, please check the following [guide](https://www.shellhacks.com/windows-set-environment-variable-cmd-powershell/). Environment variables can also be set manually using Windows system properties.
     A guide on this process can be found [here](https://www.alphr.com/environment-variables-windows-10/).

### Dependencies

This project relies on the following dependencies, depending on your setup configuration:

#### Vulkan SDK

No further dependencies are required beyond the SDK itself. Please refer to the section above for further details on setting up all appropriate environment variables.

#### Debug

- [Vulkan-ValidationLayers](https://github.com/KhronosGroup/Vulkan-ValidationLayers) - an official repository for building the essential validation layers provided by Khronos.
- [MoltenVk](https://github.com/KhronosGroup/MoltenVK) (MacOS only) - A library containing MacOS specific builds of Vulkan. In our case we require it for the ICD files it provides.

#### Non-Debug

- [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers) - an official collection of all Vulkan headers. These are pulled into the project and linked with no added setup required.
- [glslang](https://github.com/KhronosGroup/glslang) - Khronos's official glsl utility library for cross compiling glsl to SPIRV.

All builds share the following libraries:

- [glfw](https://github.com/glfw/glfw) - the library used for displaying windows.
- [volk](https://github.com/zeux/volk) - the library used to load in Vulkan without requiring explicit linking to Vulkan's dynamic libraries.

#### Setting up Dependencies

You can set up dependencies by simply running the following:

```
// macOS & Linux
$ make setup

// Windows
> mingw32-make setup
```

These will search for the Vulkan SDK. If found, it simply uses the SDK's contents. If not, it runs a non-debug setup.

If you want to build the project in DEBUG mode, then please use the following:

```
// Linux & MacOS
$ make DEBUG=true setup

// Windows
mingw32-make DEBUG=true setup
```

**NOTE**: The debug config is intended for when you want to ship your project without relying on the SDK. Building validation layers takes a while to and is therefore not recommended during development.

These commands will pull down the all external libraries (such as Vulkan), and build them. All relevant headers will be copied and moved to the generated _include_ folder.

#### Running the project

Provided all dependencies are installed correctly, the project should then be able to run with the follwing command:

```
// macOS and Linux

// Release
$ make bin/app; make execute

// Debug - with validation layers
$ make CXXFLAGS=-DDEBUG bin/app; make execute

// Windows

// Release
> mingw32-make bin/app; mingw32-make execute

// Debug - with validation layers
> mingw-make CXXFLAGS=-DDEBUG bin/app; mingw32-make execute
```

## Contributing

### How do I contribute?

It's pretty simple actually:

1. Fork it from [here](https://github.com/CapsCollective/raylib-cpp-starter/fork)
2. Create your feature branch (git checkout -b cool-new-feature)
3. Commit your changes (git commit -m "Added some feature")
4. Push to the branch (git push origin cool-new-feature)
5. Create a new pull request for it!

### Contributors

- [J-Mo63](https://github.com/J-Mo63) Jonathan Moallem - co-creator, maintainer
- [Raelr](https://github.com/Raelr) Aryeh Zinn - co-creator, maintainer

## Licence

This project is licenced under an unmodified zlib/libpng licence, which is an OSI-certified, BSD-like licence that allows static linking with closed source software. Check [`LICENCE`](LICENSE) for further details.
