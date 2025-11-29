
#ifndef MPI_DEFS_H
#ifdef USE_OPEN_MPI
#define MPI_DEFS_H

#include <mpi.h>
#include <util/types.h>

namespace mpi {

    constexpr int MASTER = 0;

    enum class Tag : int {
        DataTag = 0,
        TypeTag = 1,
    };

    enum class TypeTag : u32 {
        Null = 0,
        S2M_Ready,
        S2M_FrameComplete,

        M2S_BeginFrame,

        S2M_RequestNormalTexture,
        S2M_RequestMapTexture,
        S2M_RequestRgbGammaCorrectedTexture,
        S2M_RequestRgbaTexture,
        M2S_RequestedTexture,
        S2M_RequestMesh,
        M2S_RequestedMesh,
    };

    inline int rank() {
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        return rank;
    }

    inline bool is_master() {
        return rank() == 0;
    }

    inline bool is_slave() {
        return !is_master();
    }

    template<class T>
    struct codec;

    #include <mpi/types.h>
    #include <mpi/codec.h>
    #include <mpi/sender.h>
    #include <mpi/receiver.h>
    #include <mpi/codecs.h>



    inline Receiver Sender::receiver() const {
        return Receiver(dest, TypeTag::Null);
    }

    namespace s {
        inline Texture request_normal_texture(ref<std::string> path) {
            return Sender::begin_master(TypeTag::S2M_RequestNormalTexture)
                .send(path)
                .receiver()
                .receive<Texture>();
        }

        inline double slave_await_begin_frame() {
            return Sender::begin_master(TypeTag::S2M_Ready).receiver().receive<f64>();
        }
    }
}

#endif
#endif //MPI_DEFS_H
