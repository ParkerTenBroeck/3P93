
#ifndef MPI_DEFS_H
#define MPI_DEFS_H
#ifdef USE_OPEN_MPI

#include <mpi.h>
#include <util/types.h>


#include <mpi/types.h>
#include <mpi/codec.h>
#include <mpi/sender.h>
#include <mpi/receiver.h>
#include <mpi/codecs.h>

namespace mpi {
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

        inline Texture request_map_texture(ref<std::string> path) {
            return Sender::begin_master(TypeTag::S2M_RequestMapTexture)
                .send(path)
                .receiver()
                .receive<Texture>();
        }

        inline Texture request_rgba_gamma_corrected_texture(ref<std::string> path) {
            return Sender::begin_master(TypeTag::S2M_RequestRgbaGammaCorrectedTexture)
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
