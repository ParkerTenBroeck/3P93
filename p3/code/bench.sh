
mkdir -p animation

cd out

commands=(
    ":./rasterizer"
    "mp:./rasterizer-mp"
    "mpi:mpirun -n 4 rasterizer-mpi"
    "mp-mpi:mpirun -n 4 rasterizer-mp-mpi"
)

params=(
    "halo720:--scene=halo --width=720 --height=480 --write_frames=false --frames=300 --framerate=30 --freecam=false"
    "halo1080:--scene=halo --width=1920 --height=1080 --write_frames=false --frames=300 --framerate=30 --freecam=false"
    "bricks720:--scene=bricks --height=720 --width=480 --write_frames=false --frames=300 --framerate=30 --freecam=false"
    "bricks1080:--scene=bricks --height=1920 --width=1080 --write_frames=false --frames=300 --framerate=30 --freecam=false"
    "wavy720:--scene=test --width=720 --height=480 --write_frames=false --frames=300 --framerate=30 --freecam=false"
    "wavy1080:--scene=test --width=1920 --height=1080 --write_frames=false --frames=300 --framerate=30 --freecam=false"
    "halo720-f:--scene=halo --width=720 --height=480 --write_frames=true --frames=300 --framerate=30 --freecam=false"
    "halo1080-f:--scene=halo --width=1920 --height=1080 --write_frames=true --frames=300 --framerate=30 --freecam=false"
    "bricks720-f:--scene=bricks --height=720 --width=480 --write_frames=true --frames=300 --framerate=30 --freecam=false"
    "bricks1080-f:--scene=bricks --height=1920 --width=1080 --write_frames=true --frames=300 --framerate=30 --freecam=false"
    "wavy720-f:--scene=test --width=720 --height=480 --write_frames=true --frames=300 --framerate=30 --freecam=false"
    "wavy1080-f:--scene=test --width=1920 --height=1080 --write_frames=true --frames=300 --framerate=30 --freecam=false"
)

for cmd_entry in "${commands[@]}"; do
    cmd_id="${cmd_entry%%:*}"
    cmd="${cmd_entry#*:}"

    for par_entry in "${params[@]}"; do
        par_id="${par_entry%%:*}"
        par="${par_entry#*:}"

        start=$(date +%s.%N)

        bash -c "$cmd $par" >/dev/null 2>&1

        end=$(date +%s.%N)
        elapsed=$(echo "$end - $start" | bc)

        echo "${cmd_id}-${par_id} $elapsed"
    done
done
