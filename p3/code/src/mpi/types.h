#ifndef MPI_TYPES_H
#define MPI_TYPES_H


#include <util/types.h>

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

#endif //TYPES_H
