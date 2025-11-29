#ifndef MPI_CODEC_H
#define MPI_CODEC_H

#include <util/types.h>

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

#endif //MPI_CODECS_H
