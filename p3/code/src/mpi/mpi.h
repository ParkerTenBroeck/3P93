
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
        return receiver(m_tag);
    }
    inline Receiver Sender::receiver(Tag type) const {
        return Receiver(m_dest, type);
    }

    template<typename T>
    void coordinator_broadcast(const T& value, const Tag type) {
        const auto workers = worker_count();
        for (Rank i = 0; i < workers; i ++) {
            Sender::begin(i+1, type).send<T>(value);
        }
    }
}

#endif
#endif //MPI_DEFS_H
