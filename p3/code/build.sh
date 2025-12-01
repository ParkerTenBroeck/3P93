
mkdir -p animation
mkdir -p out

mkdir -p cmake-build-rel
cd cmake-build-rel
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_OPENMP=OFF -DENABLE_OPENMPI=OFF -DENABLE_OPENGL=OFF ./..
cmake --build .
cp ./rasterizer ../out/rasterizer
cd ..

mkdir -p cmake-build-rel-mp
cd cmake-build-rel-mp
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_OPENMP=ON -DENABLE_OPENMPI=OFF -DENABLE_OPENGL=OFF ./..
cmake --build .
cp ./rasterizer ../out/rasterizer-mp
cd ..

mkdir -p cmake-build-rel-mpi
cd cmake-build-rel-mpi
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_OPENMP=OFF -DENABLE_OPENMPI=ON -DENABLE_OPENGL=OFF ./..
cmake --build .
cp ./rasterizer ../out/rasterizer-mpi
cd ..

mkdir -p cmake-build-rel-mp-mpi
cd cmake-build-rel-mp-mpi
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_OPENMP=ON -DENABLE_OPENMPI=ON -DENABLE_OPENGL=OFF ./..
cmake --build .
cp ./rasterizer ../out/rasterizer-mp-mpi
cd ..

mkdir -p cmake-build-rel-gui
cd cmake-build-rel-gui
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_OPENMP=ON -DENABLE_OPENMPI=OFF -DENABLE_OPENGL=ON ./..
cmake --build .
cp ./rasterizer ../out/rasterizer-gui
cd ..
