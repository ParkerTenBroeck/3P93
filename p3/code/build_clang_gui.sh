export CC=clang
export CXX=clang++

mkdir build
mkdir animation

cd build

cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_OPENGL=ON ..

cmake --build .
