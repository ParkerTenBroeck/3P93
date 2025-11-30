#ifndef RECEIVER_H
#define RECEIVER_H
#ifdef USE_OPEN_MPI

#include <assert.h>
#include <filesystem>
#include <mpi.h>

#include "types.h"

namespace mpi {
    class Receiver {
        friend Sender;
        Rank m_src;
        Tag m_tag;

        explicit Receiver(Rank dest, Tag type): m_src(dest), m_tag(type) {
        }

        template<typename T>
        void do_receive(T& value, int count, MPI_Datatype datatype) {
            MPI_Status status;
            MPI_Recv(&value, count, datatype,m_src, static_cast<int>(m_tag), MPI_COMM_WORLD, &status);
            //std::cout << status.MPI_SOURCE << " -> (" << rank() << ") " << " type: " << type_name<T>() << " count: " << count << std::endl;
        }

    public:
        [[nodiscard]] constexpr Tag type() const {
            return m_tag;
        }

        template<typename T>
        T receive() {
            if constexpr (codec<T>::is_primitive()) {
                T value;
                do_receive<T>(value, codec<T>::count(), codec<T>::type());
                return value;
            }else {
                return codec<T>::receive(*this);
            }
        }

        template<typename T>
        T* receive(usize count) {
            static_assert(codec<T>::is_primitive());
            T* value = new T[count];
            do_receive<T>(*value, codec<T>::count()*count, codec<T>::type());
            return value;
        }

        template<typename T>
        Receiver& receive(T* value) {
            if constexpr (codec<T>::is_primitive()) {
                do_receive<T>(*value, codec<T>::count(), codec<T>::type());
            }else {
                codec<T>::receive(value, *this);
            }
            return *this;
        }

        template<typename T>
        Receiver& receive(T* value, usize count) {
            static_assert(codec<T>::is_primitive());
            do_receive<T>(*value, codec<T>::count()*count, codec<T>::type());
            return *this;
        }

        static Receiver probe(Rank src = MPI_ANY_SOURCE, Tag tag = Tag::ANY) {
            MPI_Status status;
            MPI_Probe(src, static_cast<int>(tag), MPI_COMM_WORLD, &status);
            return begin(status.MPI_SOURCE, static_cast<Tag>(status.MPI_TAG));
        }

        static Receiver begin(const Rank receiver, Tag type) {
            return Receiver(receiver, type);
        }

        [[nodiscard]] Sender sender() const {
            return sender(m_tag);
        }

        [[nodiscard]] Sender sender(const Tag tag) const {
            return Sender(m_src, tag);
        }
    };
}

#endif
#endif //RECEIVER_H
