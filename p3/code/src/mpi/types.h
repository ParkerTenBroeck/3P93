#ifndef MPI_TYPES_H
#define MPI_TYPES_H
#ifdef USE_OPEN_MPI

#include <mpi.h>
#include <util/types.h>

namespace mpi {
    namespace types {
        class type {public:virtual operator MPI_Datatype() const = 0;};
#define TTY(repr, name, MPI) class : public type {public:virtual operator MPI_Datatype() const {return MPI;}} constexpr name;
#define TY(repr, MPI) TTY(repr, repr, MPI)
        TY(f32, MPI_FLOAT)
        TY(f64, MPI_DOUBLE)
        TY(i64, MPI_INT64_T)
        TY(i32, MPI_INT32_T)
        TY(i16, MPI_INT16_T)
        TY(i8, MPI_INT8_T)
        TY(u64, MPI_UINT64_T)
        TY(u32, MPI_UINT32_T)
        TY(u16, MPI_UINT16_T)
        TY(u8, MPI_UINT8_T)
        TY(character, MPI_CHARACTER)
        TTY(bool, boolean, MPI_CXX_BOOL)
    #undef TY
    #undef TTY
    }


    inline int rank() {
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        return rank;
    }

    inline int worker_count() {
        int count;
        MPI_Comm_size(MPI_COMM_WORLD, &count);
        return count-1;
    }

    inline bool is_coordinator() {
        return rank() == 0;
    }

    inline bool is_worker() {
        return !is_coordinator();
    }

    typedef int Rank;
    constexpr Rank COORDINATOR = 0;

    enum class Tag : int {
        Arguments,
        FrameComplete,
        WorkerReady,
        BeginFrame,
        LoadFile,
        ANY = MPI_ANY_TAG,
    };

    struct Empty{};

}

#endif
#endif //MPI_TYPES_H
