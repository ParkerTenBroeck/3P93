
mkdir -p animation

cd out

commands=(
    #":./rasterizer"

    "mp1:OMP_NUM_THREADS=1 ./rasterizer-mp"
    "mp2:OMP_NUM_THREADS=2 ./rasterizer-mp"
    "mp4:OMP_NUM_THREADS=4 ./rasterizer-mp"
    "mp8:OMP_NUM_THREADS=8 ./rasterizer-mp"
    "mp16:OMP_NUM_THREADS=16 ./rasterizer-mp"

    "mpi1:mpirun --oversubscribe -n 2 rasterizer-mpi"
    "mpi2:mpirun --oversubscribe -n 3 rasterizer-mpi"
    "mpi4:mpirun --oversubscribe -n 5 rasterizer-mpi"
    "mpi8:mpirun --oversubscribe -n 9 rasterizer-mpi"
    "mpi16:mpirun --oversubscribe -n 17 rasterizer-mpi"

    #"mp-mpi:mpirun -n 4 rasterizer-mp-mpi"
)

params=(
    "halo_360:--scene=halo --width=360 --height=240 --write_frames=false --frames=300 --framerate=30"
    "halo_720:--scene=halo --width=720 --height=480 --write_frames=false --frames=300 --framerate=30"
    "halo_1080:--scene=halo --width=1920 --height=1080 --write_frames=false --frames=300 --framerate=30"

    "bricks_360:--scene=bricks --height=360 --width=240 --write_frames=false --frames=300 --framerate=30"
    "bricks_720:--scene=bricks --height=720 --width=480 --write_frames=false --frames=300 --framerate=30"
    "bricks_1080:--scene=bricks --height=1920 --width=1080 --write_frames=false --frames=300 --framerate=30"

    "wavy_360:--scene=test --width=360 --height=240 --write_frames=false --frames=300 --framerate=30"
    "wavy_720:--scene=test --width=720 --height=480 --write_frames=false --frames=300 --framerate=30"
    "wavy_1080:--scene=test --width=1920 --height=1080 --write_frames=false --frames=300 --framerate=30"


#    "halo_360_f:--scene=halo --width=360 --height=240 --write_frames=true --frames=300 --framerate=30"
#    "halo_720_f:--scene=halo --width=720 --height=480 --write_frames=true --frames=300 --framerate=30"
#    "halo_1080_f:--scene=halo --width=1920 --height=1080 --write_frames=true --frames=300 --framerate=30"

#    "bricks_360_f:--scene=bricks --height=360 --width=240 --write_frames=true --frames=300 --framerate=30"
#    "bricks_720_f:--scene=bricks --height=720 --width=480 --write_frames=true --frames=300 --framerate=30"
#    "bricks_1080_f:--scene=bricks --height=1920 --width=1080 --write_frames=true --frames=300 --framerate=30"

#    "wavy_360_f:--scene=test --width=360 --height=240 --write_frames=true --frames=300 --framerate=30"
#    "wavy_720_f:--scene=test --width=720 --height=480 --write_frames=true --frames=300 --framerate=30"
#    "wavy_1080_f:--scene=test --width=1920 --height=1080 --write_frames=true --frames=300 --framerate=30"
)

for par_entry in "${params[@]}"; do
    for cmd_entry in "${commands[@]}"; do
       cmd_id="${cmd_entry%%:*}"
       cmd="${cmd_entry#*:}"

        par_id="${par_entry%%:*}"
        par="${par_entry#*:}"

        start=$(date +%s.%N)

        calc_elapsed=$(bash -c "$cmd $par" 2>&1 >/dev/null)
        calc_elapsed=${calc_elapsed//false/}

        end=$(date +%s.%N)
        total_elapsed=$(echo "${end} - ${start}" | bc)

        echo "${cmd_id}_${par_id}, ${calc_elapsed}, ${total_elapsed}"
    done
done
