rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
platformpth = $(subst /,$(PATHSEP),$1)

runVendorCmd = cd vendor/$1 $(THEN) $2
exportEnv = export $1=$2

buildDir := bin
executable := app
target := $(buildDir)/$(executable)
sources := $(call rwildcard,src/,*.cpp)
objects := $(patsubst src/%, $(buildDir)/%, $(patsubst %.cpp, %.o, $(sources)))
depends := $(patsubst %.o, %.d, $(objects))
submoduleDir := vendor

updateSubmodule = git submodule update --init $(submoduleDir)/$1

includes = -I include -I vendor/glfw/include -I include/volk -I $(vulkanIncludes) 
linkFlags = -L lib/$(platform) -lglfw3
compileFlags = -std=c++17 $(includes)

ifeq ($(OS),Windows_NT)
    platform := Windows
    CXX ?= g++
    linkFlags += -Wl,--allow-multiple-definition -pthread -lopengl32 -lgdi32 -lwinmm -mwindows -static -static-libgcc -static-libstdc++
    THEN := &&
    PATHSEP := \$(BLANK)
    MKDIR := -mkdir -p
    RM := -del /q
    COPY = -robocopy "$(call platformpth,$1)" "$(call platformpth,$2)" $3

    generator = MinGW Makefiles
    volkDefines = VK_USE_PLATFORM_WIN32_KHR

    export GLSLC $(call platformpth,$(VULKAN_SDK)/bin/glslangValidator)
else 
    UNAMEOS := $(shell uname)
    ifeq ($(UNAMEOS), Linux)

        platform := Linux
        CXX ?= g++
        linkFlags += -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

        volkDefines = VK_USE_PLATFORM_XLIB_KHR
    endif
    ifeq ($(UNAMEOS), Darwin)

        platform := macOS
        CXX ?= clang++
        linkFlags += -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
        
        volkDefines = VK_USE_PLATFORM_MACOS_MVK

        export VK_ICD_FILENAMES=$(VULKAN_SDK)/share/vulkan/icd.d/MoltenVK_icd.json

    endif # end of MacOS check

    export VK_LAYER_PATH=$(VULKAN_SDK)/etc/explicit_layer.d
    export GLSLC=$(VULKAN_SDK)/bin/glslangValidator

    generator = Unix Makefiles

    PATHSEP := /
    MKDIR = mkdir -p
    COPY = cp $1$(PATHSEP)$3 $2
    THEN = ;
    RM := rm -rf
endif

# Lists phony targets for Makefile
.PHONY: all setup submodules execute clean

all: $(target) execute clean

ifndef VULKAN_SDK

    vulkanIncludes := include/vulkan
    
    ifdef DEBUG 

        # If we're using a Debug build, then we want to build the validation layers
        setup-validation-layers:
            $(call updateSubmodule,Vulkan-ValidationLayers)

            $(call runVendorCmd,Vulkan-ValidationLayers,$(MKDIR) $(call platformpth, build))
            $(call runVendorCmd,Vulkan-ValidationLayers/build,../scripts/update_deps.py --dir ../external --config release)
            $(call runVendorCmd,Vulkan-ValidationLayers/build,cmake -C ../external/helper.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./ ..)
            $(call runVendorCmd,Vulkan-ValidationLayers/build,cmake --build . --config Release)
            $(call runVendorCmd,Vulkan-ValidationLayers/build,cmake --install .)

            $(MKDIR) $(call platformpth,include/vulkan/explicit_layer.d)

            $(call COPY,vendor/Vulkan-ValidationLayers/external/Vulkan-Headers/include/vulkan,include/vulkan,**.h)
            $(call COPY,vendor/Vulkan-ValidationLayers/build/share/vulkan/explicit_layer.d,include/vulkan/explicit_layer.d,**.json)

        # MacOS requires the extra step of seting up MoltenVK 
        ifeq ($(UNAMEOS), Darwin)

            # Only relevant for DEBUG builds - used to get the MoltenVk ICD
            setup-moltenVk:
                $(call updateSubmodule,MoltenVK)
    
                $(call runVendorCmd,MoltenVK,./fetchDependencies --macos)
                $(call runVendorCmd,MoltenVK,$(MAKE) macos)

                $(MKDIR) $(call platformpth,include/vulkan/icd.d)

                $(call COPY,vendor/MoltenVK/Package/Latest/MoltenVK/dylib/macOS,include/vulkan/icd.d,**)

            export VK_ICDFILENAMES=include/vulkan/icd.d/MoltenVK_icd.json

            setup: setup-glfw setup-volk setup-validation-layers setup-moltenVk

        endif # end of MacOS check

        ifneq ($(OS),Windows_NT)
            export VK_LAYER_PATH=include/vulkan/explicit_layer.d
            export GLSLC=vendor/Vulkan-ValidationLayers/external/glslang/build/StandAlone/glslangValidator
        else 
            export GLSLC=$(call platformpth,vendor/Vulkan-ValidationLayers/external/glslang/build/StandAlone/glslangValidator)
        endif # end of windows check

        setup: setup-glfw setup-volk setup-validation-layers

    else # if it isnt DEBUG

        # Building GLSLANG and pulling in the Vulkan headers is only relevant when 
        # we aren't using the SDK and don't want to use validation layers 
        setup-glslang:
            $(call updateSubmodule,glslang)
            $(MKDIR) $(call platformpth,vendor/glslang/build)
            $(call runVendorCmd,glslang/build,cmake -G="$(generator)" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$(pwd)/install" ..)
            $(call runVendorCmd,glslang/build/StandAlone,$(MAKE))
            $(MKDIR) $(call platformpth, lib/$(platform))
            $(call COPY,vendor/glslang/build/glslang,lib/$(platform),libglslang.a)

        setup-vulkan-headers:
            $(call updateSubmodule,Vulkan-Headers)
            $(MKDIR) $(call platformpth,include/vulkan)
            $(call COPY,vendor/Vulkan-Headers/include/vulkan,include/vulkan,**.h)

        setup: setup-glfw setup-volk setup-vulkan-headers setup-glslang
    endif
else # If VULKAN_SDK is defined
    vulkanIncludes := $(VULKAN_SDK)/include
    setup: setup-glfw setup-volk
endif #End of VULKAN_SDK check

# Volk and GLFW are relevant for all builds and platforms, therefore
# we make these targets available for everyone
setup-glfw:
    $(call updateSubmodule,glfw)
    cd vendor/glfw $(THEN) cmake -G "$(generator)" . $(THEN) "$(MAKE)" 
    $(MKDIR) $(call platformpth, lib/$(platform))
    $(call COPY,vendor/glfw/src,lib/$(platform),libglfw3.a)

setup-volk:
    $(call updateSubmodule,volk)
    $(MKDIR) $(call platformpth, include/volk)
    $(call COPY,vendor/volk,include/volk,volk.h)
    $(call COPY,vendor/volk,include/volk,volk.c)

# Link the program and create the executable
$(target): $(objects)
    $(CXX) $(objects) -o $(target) $(linkFlags)

$(buildDir)/%.spv: % 
    $(MKDIR) $(call platformpth, $(@D))
    ${GLSLC} $< -V -o $@

# Add all rules from dependency files
-include $(depends)

# Compile objects to the build directory
$(buildDir)/%.o: src/%.cpp Makefile
    $(MKDIR) $(call platformpth, $(@D))
    $(CXX) -MMD -MP -c $(compileFlags) $< -o $@ $(CXXFLAGS) -D$(volkDefines)

execute: 
    $(target) $(ARGS)

clean: 
    $(RM) $(call platformpth, $(buildDir)/*)

clean-all: clean
    $(RM) $(call platformpth, lib)
    $(RM) $(call platformpth, include)
    $(RM) $(call platformpth, vendor/**)

