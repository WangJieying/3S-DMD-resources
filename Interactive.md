Please find the source code [here](./code/interactive-3sdmd).

# Building

The software needs a C++ compiler, OpenGL, the CUDA, and QT to build. 
Under a Debian based linux they can be installed using the following command.

* install

sudo apt-get install cuda zpaq libzstd-dev libsnappy-dev gcc git make cmake libboost-all-dev freeglut3-dev libglew-dev ragel libvala-0.40-dev glmark2 valac liblz4-dev liblzma-dev libbz2-1.0

you also need to install the dependency manager: Conan.

Before building, please install the librarys of [Skel](https://github.com/WangJieying/Skel), [Spline](https://github.com/WangJieying/Spline), and [ImageViewerWidget](https://github.com/WangJieying/ImageViewerWidget).

* to build

cd build

conan install ..

cmake ..

make

* Running

./bin/interactive-dmd

# Other remarks

Find more detailed explanation of the code, see [here](./Non-interactive.md).