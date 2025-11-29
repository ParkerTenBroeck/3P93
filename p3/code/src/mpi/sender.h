#ifndef SENDER_H
#define SENDER_H
#include "mpi.h"

class Sender {
    friend Receiver;
    int dest;
    explicit Sender(int dest)
        : dest(dest) {
    }

    template<typename T>
    void do_send(const T& value, int count, MPI_Datatype type) {
        MPI_Send(&value, count, type, dest, static_cast<int>(Tag::DataTag), MPI_COMM_WORLD);
        std::cout << "(" << rank() << ") -> " << dest << " type: " << type_name<T>() << " count: " << count << std::endl;

    }

public:
    template<typename T>
    Sender& send(const T& value) {
        if constexpr (codec<T>::is_primitive()) {
            do_send<T>(value, codec<T>::count(), codec<T>::type());
        }else {
            codec<T>::send(value, *this);
        }
        return *this;
    }

    template<typename T>
    Sender& send(const T* value, usize count) {
        static_assert(codec<T>::is_primitive());
        do_send<T>(*value, codec<T>::count()*count, codec<T>::type());
        return *this;
    }

    static Sender begin_master(const TypeTag ttag) {
        return begin(MASTER, ttag);
    }

    static Sender begin(const int dest, const TypeTag ttag) {
        MPI_Send(&ttag,
            1,
            types::u32,
            dest, static_cast<int>(Tag::TypeTag), MPI_COMM_WORLD);
        std::cout << "(" << rank() << ") -> " << dest << " Initiated" << std::endl;
        return Sender(dest);
    }

    Receiver receiver() const;
};

#endif //SENDER_H
