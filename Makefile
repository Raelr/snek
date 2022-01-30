rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
platformpth = $(subst /,$(PATHSEP),$1)

runVendorInstallCmd = cd vendor/$1 $(THEN) $2
exportEnv = export $1=$2

buildDir := bin
executable := app
target := $(buildDir)/$(executable)
sources := $(call rwildcard,src/,*.cpp)
objects := $(patsubst src/%, $(buildDir)/%, $(patsubst %.cpp, %.o, $(sources)))
depends := $(patsubst %.o, %.d, $(objects))
submoduleDir := vendor

updateSubmodule = git submodule update --init $(submoduleDir)/$1

# TODO: 
# Need to build and link the repository in the following way:
# 1) Check for VULKAN_SDK env variable. If it's set, then the SDK is available and we can use the packages in it. 
# 2) If it isn't then we build the other repositories. 
# 3) If we're using macOS, then pull and build MoltenVK, then specify the ICD directory in VK_ICD_FILENAMES env variable
# 4) Pull and build Vulkan-ValidationLayers - Add explicit-layer.d to VK_LAYER_PATH env variable
# 5) Pull and build glslang - add glslangValidator path to GLSLC env variable
# 6) Pull and build Vulkan Headers - add include path to vulkanIncludes variable

includes = -I vendor/glfw/include -I vendor/volk -I $(vulkanIncludes) 
linkFlags = -L lib/$(platform) -lglfw3
compileFlags = -std=c++17 $(includes)

ifeq ($(OS),Windows_NT)

	CMAKE_CMD = cmake -G "MinGW Makefiles" .

	platform := Windows
	CXX ?= g++
	linkFlags += -Wl,--allow-multiple-definition -pthread -lopengl32 -lgdi32 -lwinmm -mwindows -static -static-libgcc -static-libstdc++
	THEN := &&
	PATHSEP := \$(BLANK)
	MKDIR := -mkdir -p
	RM := -del /q
	COPY = -robocopy "$(call platformpth,$1)" "$(call platformpth,$2)" $3

	volkDefines = VK_USE_PLATFORM_WIN32_KHR
else 
	UNAMEOS := $(shell uname)
	ifeq ($(UNAMEOS), Linux)

		vulkanExports := export VK_LAYER_PATH=$(VULKAN_SDK)/etc/explicit_layer.d

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
	endif

	CMAKE_CMD = cmake .

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
ifdef DEBUG 
	export GLSLC=vendor/Vulkan-ValidationLayers/external/glslang/build/StandAlone/glslangValidator
	export VK_LAYER_PATH=vendor/Vulkan-ValidationLayers/build/share/vulkan/explicit_layer.d
	export VK_ICD_FILENAMES=vendor/MoltenVK/Package/Latest/MoltenVK/dylib/MoltenVK_icd.json
vulkanIncludes := vendor/Vulkan-ValidationLayers/external/Vulkan-Headers/include/vulkan
setup-macos: setup-glfw setup-volk setup-validation-layers setup-moltenVk
else
	export GLSLC=vendor/glslang/build/StandAlone/glslangValidator
vulkanIncludes := vendor/Vulkan-Headers/build/install/include/vulkan
setup-macos: setup-glfw setup-volk setup-glslang setup-vulkan-headers 
endif
else
vulkanIncludes := $(VULKAN_SDK)/include
ifdef DEBUG
	export VK_LAYER_PATH=$(VULKAN_SDK)/etc/explicit_layer.d
	export VK_ICD_FILENAMES=$(VULKAN_SDK)/share/vulkan/icd.d/MoltenVK_icd.json
	export GLSLC=$(VULKAN_SDK)/bin/glslangValidator
endif
setup-macos: setup-glfw setup-volk
endif

setup-validation-layers:
	$(call updateSubmodule,Vulkan-ValidationLayers)
	$(call runVendorInstallCmd,Vulkan-ValidationLayers,$(MKDIR) $(call platformpth, build))
	$(call runVendorInstallCmd,Vulkan-ValidationLayers/build,../scripts/update_deps.py --dir ../external --config release)
	$(call runVendorInstallCmd,Vulkan-ValidationLayers/build,cmake -C ../external/helper.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./ ..)
	$(call runVendorInstallCmd,Vulkan-ValidationLayers/build,cmake --build . --config Release)
	$(call runVendorInstallCmd,Vulkan-ValidationLayers/build,cmake --install .)

setup-moltenVk:
	$(call updateSubmodule,MoltenVK)
	$(call runVendorInstallCmd,MoltenVK,./fetchDependencies --macos)
	$(call runVendorInstallCmd,MoltenVK,$(MAKE) macos)

setup-glslang:
	$(call updateSubmodule,glslang)
	$(MKDIR) $(call platformpth,vendor/glslang/build)
	$(call runVendorInstallCmd,glslang/build,cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$(pwd)/install" ..)
	$(call runVendorInstallCmd,glslang/build/StandAlone,$(MAKE))
	$(MKDIR) $(call platformpth, lib/$(platform))
	$(call COPY,vendor/glslang/build/glslang,lib/$(platform),libglslang.a)

setup-vulkan-headers:
	$(call updateSubmodule,Vulkan-Headers)
	$(MKDIR) $(call platformpth,vendor/Vulkan-Headers/build)
	$(call runVendorInstallCmd,Vulkan-Headers/build,cmake -DCMAKE_INSTALL_PREFIX=install ..)
	$(call runVendorInstallCmd,Vulkan-Headers/build,make install)

setup-glfw:
	$(call updateSubmodule,glfw)
	cd vendor/glfw $(THEN) $(CMAKE_CMD) $(THEN) "$(MAKE)" 
	$(MKDIR) $(call platformpth, lib/$(platform))
	$(call COPY,vendor/glfw/src,lib/$(platform),libglfw3.a)

setup-volk:
	$(call updateSubmodule,volk)

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

clear: 
	clear;

execute: 
	$(target) $(ARGS)

clean: 
	$(RM) $(call platformpth, $(buildDir)/*)

clean-all: clean
	$(RM) $(call platformpth, lib)
	$(RM) $(call platformpth, vendor/**)

