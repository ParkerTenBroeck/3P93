#ifndef MPI_CODEC_H
#define MPI_CODEC_H
#ifdef USE_OPEN_MPI

#include <util/types.h>
#include <mpi/types.h>

namespace mpi{
    class Sender;
    class Receiver;

    struct codec_base {
        static constexpr bool is_primitive() noexcept {return false;}
        static constexpr usize count() noexcept { return 1;}
    };

    template<typename T>
    struct codec : codec_base {
        static constexpr const types::type& type() noexcept = delete;

        static void send(const T&, Sender&) {
            static_assert(false, "no codec defined for this type");
        }
        static T receive(Receiver&) {
            static_assert(false, "no codec defined for this type");
        };
        static void receive(T*, Receiver&) {
            static_assert(false, "no codec defined for this type");
        };
    };
}

#endif
#endif //MPI_CODEC_H
