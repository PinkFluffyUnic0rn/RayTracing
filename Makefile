CCLinux=gcc
CCWindows=i586-mingw32msvc-gcc
CCLinuxObjArgsR=-Wall -fpic -c -lOpenCL
CCLinuxObjArgsD=-Wall -fpic -c -lOpenCL -g
CCLinuxExArgsR=-Wall `pkg-config gtk+-3.0 --cflags --libs`                                                     \
-lm -pthread -lrt -lOpenCL
BuildPath=./Build/
BuildReleasePath=$(BuildPath)Release/
BuildDebugPath=$(BuildPath)Debug/
BuildWindowsPath=$(BuildPath)Windows/

all: dir main

dir:
	mkdir -p $(BuildReleasePath) 
	mkdir -p $(BuildDebugPath)

main: rtR rtD
	$(CCLinux) -L/home/qwerty/RayTracing/$(BuildReleasePath)                                               \
-Wl,-rpath=/home/qwerty/RayTracing/$(BuildReleasePath)                                                         \
main.c -o "$(BuildReleasePath)rt" $(CCLinuxExArgsR)

rtR: $(BuildReleasePath)rt_debug_output.o                                                                      \
$(BuildReleasePath)rt_funcs_math.o $(BuildReleasePath)rt_funcs_primitives.o                                    \
$(BuildReleasePath)rt_funcs_render_pipe.o $(BuildReleasePath)rt_meshes.o
	cp -rd CL $(BuildReleasePath)
	$(CCLinux) -shared -o $(BuildReleasePath)librt.so $(BuildReleasePath)rt_debug_output.o                 \
$(BuildReleasePath)rt_funcs_math.o $(BuildReleasePath)rt_funcs_primitives.o                                    \
$(BuildReleasePath)rt_funcs_render_pipe.o $(BuildReleasePath)rt_meshes.o

$(BuildReleasePath)rt_debug_output.o:
	$(CCLinux) rt_debug_output.c $(CCLinuxObjArgsR) -o $(BuildReleasePath)rt_debug_output.o

$(BuildReleasePath)rt_funcs_math.o:
	$(CCLinux) rt_funcs_math.c $(CCLinuxObjArgsR) -o $(BuildReleasePath)rt_funcs_math.o

$(BuildReleasePath)rt_funcs_primitives.o:
	$(CCLinux) rt_funcs_primitives.c $(CCLinuxObjArgsR) -o $(BuildReleasePath)rt_funcs_primitives.o

$(BuildReleasePath)rt_funcs_render_pipe.o:
	$(CCLinux) rt_funcs_render_pipe.c $(CCLinuxObjArgsR) -o $(BuildReleasePath)rt_funcs_render_pipe.o

$(BuildReleasePath)rt_meshes.o:
	$(CCLinux) rt_meshes.c $(CCLinuxObjArgsR) -o $(BuildReleasePath)rt_meshes.o

rtD:
	cp -rd CL $(BuildDebugPath)
	$(CCLinux) -pg -g -o $(BuildDebugPath)rt rt_debug_output.c rt_funcs_math.c                             \
rt_funcs_primitives.c rt_funcs_render_pipe.c rt_meshes.c main.c                                                \
`pkg-config gtk+-3.0 --cflags --libs` -lm -pthread -lOpenCL
	
clean:
	rm -rd $(BuildPath)
