#ifndef SENDER_H
#define SENDER_H
#ifdef USE_OPEN_MPI
#include "types.h"

namespace mpi {
    class Sender {
        friend Receiver;
        Rank m_dest;
        Tag m_tag;
        explicit Sender(const Rank dest, const Tag tag)
            : m_dest(dest), m_tag(tag) {
        }

        template<typename T>
        void do_send(const T& value, int count, MPI_Datatype type) {
            MPI_Send(&value, count, type, m_dest, static_cast<int>(m_tag), MPI_COMM_WORLD);
            // std::cout << "(" << rank() << ") -> " << m_dest << " type: " << type_name<T>() << " count: " << count << std::endl;
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

        static Sender begin_coordinator(const Tag ttag) {
            return begin(COORDINATOR, ttag);
        }

        static Sender begin(const int dest, const Tag tag) {
            // std::cout << "(" << rank() << ") -> " << dest << " Initiated" << std::endl;
            return Sender(dest, tag);
        }

        [[nodiscard]] Receiver receiver() const;
        [[nodiscard]] Receiver receiver(Tag type) const;
    };
}

#endif
#endif //SENDER_H
