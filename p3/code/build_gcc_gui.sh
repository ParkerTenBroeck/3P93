export CC=gcc
export CXX=g++

mkdir build
mkdir animation

cd build

cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_OPENGL=ON ..

cmake --build .
