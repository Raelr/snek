rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
platformpth = $(subst /,$(PATHSEP),$1)

runVendorCmd = $(CD) $(call platformpth,vendor/$1) $(THEN) $2
exportEnv = export $1=$2

buildDir := bin
executable := app
target := $(buildDir)/$(executable)
sources := $(call rwildcard,src/,*.cpp)
objects := $(patsubst src/%, $(buildDir)/%, $(patsubst %.cpp, %.o, $(sources)))
depends := $(patsubst %.o, %.d, $(objects))

submoduleDir := vendor

updateSubmodule = git submodule update --init $(submoduleDir)/$1
clone = git clone $1

includes = -I include -I vendor/glfw/include -I include/volk -I $(vulkanIncludes) 
linkFlags = -L lib/$(platform) -lglfw3
compileFlags = -std=c++17 $(includes)

ifeq ($(OS),Windows_NT)
    platformpth = "$(subst /,$(PATHSEP),$1)"
    platform := Windows
    CXX ?= g++
    linkFlags += -Wl,--allow-multiple-definition -pthread -lopengl32 -lgdi32 -lwinmm -mwindows -static -static-libgcc -static-libstdc++
    CLI_SHELL := cmd.exe /c
    THEN := &&
    PATHSEP := \$(BLANK)
    MKDIR = md $1 
    RMDIR = rmdir /s /q $1
    directories = $(sort $(dir $(wildcard ./$1/*/.)))
    CLEAN_FOLDER = $(foreach dir,$(call directories,$1),$(call RMDIR,$(call platformpth,$(dir))),) 
    CLEAN_FILES = del /s /q $1
    SHELL_CMD = $(CLI_SHELL) "$1"
    MAKE ?= mingw32-make

    COPY = -robocopy $(call platformpth,$1) $(call platformpth,$2) $3
    libSuffix = dll

    generator := "MinGW Makefiles"
    volkDefines = VK_USE_PLATFORM_WIN32_KHR

    loaderInstallDir := vendor/Vulkan-Loader/build/loader/Release
    validationLayersInstallDir := vendor/Vulkan-ValidationLayers/build/layers/Release
    validationLayersDllInstallDir := $(validationLayersInstallDir)

    export GLSLC $(call platformpth,$(VULKAN_SDK)/bin/glslangValidator)

    clean:
		$(call CLEAN_FOLDER,$(buildDir))
		$(call CLEAN_FILES,$(buildDir))

    clean-all: clean
		-$(call RMDIR,lib)
		-$(call RMDIR,include)
		$(call CLEAN_FOLDER,vendor)
else 
    UNAMEOS := $(shell uname)
    ifeq ($(UNAMEOS), Linux)

        platform := Linux
        CXX ?= g++
        linkFlags += -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
        libSuffix = so
        NUMBER_OF_PROCESSORS := $(shell nproc)

        volkDefines = VK_USE_PLATFORM_XLIB_KHR
    endif
    ifeq ($(UNAMEOS), Darwin)

        platform := macOS
        CXX ?= clang++
        linkFlags += -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
        libSuffix = dylib
        NUMBER_OF_PROCESSORS := $(shell sysctl -n hw.ncpu)
        
        volkDefines = VK_USE_PLATFORM_MACOS_MVK

    endif # end of MacOS check

    layerPath=$(VULKAN_SDK)/etc/explicit_layer.d

    export GLSLC=$(VULKAN_SDK)/bin/glslangValidator

    generator := "Unix Makefiles"
    loaderInstallDir := vendor/Vulkan-Loader/build/loader
    validationLayersInstallDir := vendor/Vulkan-ValidationLayers/build/share/vulkan/explicit_layer.d
    validationLayersDllInstallDir := vendor/Vulkan-ValidationLayers/build/layers/

    PATHSEP := /
    MKDIR = mkdir -p $1
    COPY = cp $1$(PATHSEP)$3 $2
    THEN = ;
    RM = rm -rf
    SHELL_CMD = $1

    clean:
		$(RM) $(call platformpth,$(buildDir)/*)

    clean-all: clean
		$(RM) $(call platformpth,lib)
		$(RM) $(call platformpth,include)
		$(RM) $(call platformpth,vendor/**)
endif

# Lists phony targets for Makefile
.PHONY: all setup submodules execute clean

all: $(target) execute clean

ifndef VULKAN_SDK

    vulkanIncludes := include/vulkan

	ifndef DYLD_LIBRARY_PATH
        export DYLD_LIBRARY_PATH=$(CURDIR)/lib/$(platform)
	endif 

    # This is only required when using validation layers
	ifndef VK_LAYER_PATH
        export VK_LAYER_PATH=$(CURDIR)/include/vulkan/explicit_layer.d
	endif 

    # MacOS requires the extra step of seting up MoltenVK 
	ifeq ($(UNAMEOS), Darwin)
        setup-moltenVk:
			$(call updateSubmodule,MoltenVK)

			$(call runVendorCmd,MoltenVK,./fetchDependencies --macos)
			$(call runVendorCmd,MoltenVK,$(MAKE) macos)

			$(call MKDIR, $(call platformpth,include/vulkan/icd.d))

			$(call COPY,vendor/MoltenVK/Package/Latest/MoltenVK/dylib/macOS,include/vulkan/icd.d,**)
			$(call COPY,vendor/MoltenVK/Package/Latest/MoltenVK/dylib/macOS,lib/$(platform),**.dylib)
		
        # This is required regardless of the build type 
        ifndef VK_ICD_FILENAMES
            export VK_ICD_FILENAMES=$(CURDIR)/include/vulkan/icd.d/MoltenVK_icd.json
        endif 
	endif
    
    ifdef DEBUG 

        # If we're using a Debug build, then we want to build the validation layers
        setup-validation-layers:
			$(call updateSubmodule,Vulkan-ValidationLayers)

			-$(call MKDIR,$(call platformpth,vendor/Vulkan-ValidationLayers/build))
			$(call runVendorCmd,Vulkan-ValidationLayers/build,$(call platformpth,../scripts/update_deps.py --dir ../external --config release))
			$(call runVendorCmd,Vulkan-ValidationLayers/build,cmake -C ../external/helper.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./ ..)
			$(call runVendorCmd,Vulkan-ValidationLayers/build,cmake --build . --config Release)
			$(call runVendorCmd,Vulkan-ValidationLayers/build,cmake --install .)

			-$(call MKDIR,$(call platformpth,include/vulkan/explicit_layer.d))

			$(call COPY,vendor/Vulkan-ValidationLayers/external/Vulkan-Headers/include/vulkan,include/vulkan,**.h)
			$(call COPY,vendor/Vulkan-ValidationLayers/external/Vulkan-Headers/include/vulkan,include/vulkan,**.hpp)
			$(call COPY,$(validationLayersInstallDir),include/vulkan/explicit_layer.d,**.json)
			$(call COPY,$(validationLayersDllInstallDir),lib/$(platform),**.$(libSuffix))

        # MacOS requires the extra step of seting up MoltenVK 
        ifeq ($(UNAMEOS), Darwin)

            setup: setup-glfw setup-volk setup-moltenVk setup-vulkan-loader setup-validation-layers 

            install-vulkan: 
				$(info ---- Installing Vulkan Dependencies ----)

				$(call COPY,lib/$(platform),/usr/local/lib,libvulkan**.dylib)

				$(info ---- Copying MoltenVk libs ----)
				$(call COPY,vendor/MoltenVK/Package/Latest/MoltenVK/dylib/macOS,/usr/local/lib,libMoltenVK.dylib)

				$(info ---- Copying MoltenVk icd ----)
				$(call MKDIR, $(call platformpth, /usr/local/share/vulkan/icd.d))
				$(call COPY,include/vulkan/icd.d,/usr/local/share/vulkan/icd.d,**)

				$(info ---- Copying Vulkan Validation Layers ----)
				$(call COPY,vendor/Vulkan-ValidationLayers/build/lib,/usr/local/lib,libVkLayer_khronos_validation.dylib)
				$(call MKDIR, $(call platformpth, /usr/local/share/vulkan/explicit_layer.d))
				$(call COPY,vendor/Vulkan-ValidationLayers/build/share/vulkan/explicit_layer.d,/usr/local/share/vulkan/explicit_layer.d,**.json)

				$(info ---- Copying Vulkan Headers ----)
				$(call COPY, include/vulkan,/usr/local/include,**.h)
				$(call COPY, include/vulkan,/usr/local/include,**.hpp)

        endif # end of MacOS check

        ifneq ($(OS),Windows_NT)
            export GLSLC=vendor/Vulkan-ValidationLayers/external/glslang/build/StandAlone/glslangValidator.exe
        else 
            export GLSLC=$(call platformpth,vendor/Vulkan-ValidationLayers/external/glslang/build/StandAlone/glslangValidator.exe)
        endif # end of windows check

        setup: setup-glfw setup-volk setup-vulkan-loader setup-validation-layers
		
    else # if it isnt DEBUG

        # Building GLSLANG and pulling in the Vulkan headers is only relevant when 
        # we aren't using the SDK and don't want to use validation layers 
        setup-glslang:
			$(call SHELL_CMD,$(call updateSubmodule,glslang))
			-$(call SHELL_CMD,$(call MKDIR,$(call platformpth,vendor/glslang/build)))
			$(call SHELL_CMD,cd $(call platformpth,vendor/glslang/build) $(THEN) cmake -G $(generator) -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$(pwd)/install" ..)
			cd $(call platformpth,vendor/glslang/build/StandAlone) $(THEN) "$(MAKE)" -j$(NUMBER_OF_PROCESSORS)
			-$(call SHELL_CMD,$(call MKDIR,$(call platformpth, lib/$(platform))))
			$(call COPY,vendor/glslang/build/glslang,lib/$(platform),libglslang.a)

        setup-vulkan-headers:
			$(call SHELL_CMD,$(call updateSubmodule,Vulkan-Headers))
			cd $(call platformpth,vendor/Vulkan-Headers) $(THEN) git fetch --all --tags $(THEN) git checkout tags/v1.3.207
			-$(call SHELL_CMD,$(call MKDIR,$(call platformpth,vendor/Vulkan-Headers/build)))
			ls $(call platformpth,vendor/Vulkan-Headers)

			cd $(call platformpth,vendor/Vulkan-Headers/build) $(THEN) cmake -DCMAKE_INSTALL_PREFIX=install -G $(generator) ..
			cd $(call platformpth,vendor/Vulkan-Headers/build) $(THEN) cmake --build . --target install

			-$(call SHELL_CMD,$(call MKDIR,$(call platformpth,include/vulkan)))
			-$(call COPY,vendor/Vulkan-Headers/include/vulkan,include/vulkan,**.h)

        setup: setup-glfw setup-volk setup-vulkan-headers setup-vulkan-loader setup-glslang

        setup-vulkan-loader:
			$(call SHELL_CMD, cd vendor $(THEN) $(call clone,https://github.com/KhronosGroup/Vulkan-Loader.git))
			-$(call SHELL_CMD,$(call MKDIR,$(call platformpth,vendor/Vulkan-Loader/build)))

			cd $(call platformpth,vendor/Vulkan-Loader/build) $(THEN) cmake -DVULKAN_HEADERS_INSTALL_DIR=$(CURDIR)/vendor/Vulkan-Headers/build/install ..
			cd $(call platformpth,vendor/Vulkan-Loader/build) $(THEN) cmake --build . --config Release

			-$(call SHELL_CMD,$(call MKDIR,$(call platformpth,lib/$(platform))))
			$(call COPY,$(loaderInstallDir),lib/$(platform),**.$(libSuffix))
    endif
else # If VULKAN_SDK is defined
    vulkanIncludes := $(VULKAN_SDK)/include
    setup: setup-glfw setup-volk
endif #End of VULKAN_SDK check

# Volk and GLFW are relevant for all builds and platforms, therefore
# we make these targets available for everyone
setup-glfw:
	$(call updateSubmodule,glfw)

	cd $(call platformpth,vendor/glfw) $(THEN) cmake -G $(generator) . $(THEN) "$(MAKE)" -j$(NUMBER_OF_PROCESSORS)
	-$(call SHELL_CMD,$(call MKDIR,$(call platformpth,lib/$(platform))))
	$(call COPY,vendor/glfw/src,lib/$(platform),libglfw3.a)

setup-volk:
	$(call updateSubmodule,volk)

	-$(call SHELL_CMD,$(call MKDIR,$(call platformpth,include/volk)))

	$(call COPY,vendor/volk,include/volk,volk.h)
	$(call COPY,vendor/volk,include/volk,volk.c)

# Link the program and create the executable
$(target): $(objects)
	$(CXX) $(objects) -o $(target) $(linkFlags)

$(buildDir)/%.spv: % 
	-$(call SHELL_CMD,$(call MKDIR,$(call platformpth,$(@D))))
	${GLSLC} $< -V -o $@

# Add all rules from dependency files
-include $(depends)

# Compile objects to the build directory
$(buildDir)/%.o: src/%.cpp Makefile
	-$(call SHELL_CMD,$(call MKDIR,$(call platformpth,$(@D))))
	$(CXX) -MMD -MP -c $(compileFlags) $< -o $@ $(CXXFLAGS) -D$(volkDefines)

execute: 
	$(target) $(ARGS)