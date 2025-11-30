#ifndef RECEIVER_H
#define RECEIVER_H
#ifdef USE_OPEN_MPI



namespace mpi {
    class Receiver {
        friend Sender;
        int src;
        TypeTag m_type;

        explicit Receiver(int dest, TypeTag type): src(dest), m_type(type) {
        }

        template<typename T>
        void do_receive(T& value, int count, MPI_Datatype datatype) {
            MPI_Status status;
            MPI_Recv(&value, count, datatype,src, static_cast<int>(Tag::DataTag), MPI_COMM_WORLD, &status);

            std::cout << status.MPI_SOURCE << " -> (" << rank() << ") " << " type: " << type_name<T>() << " count: " << count << std::endl;
        }

    public:
        [[nodiscard]] constexpr TypeTag type() const {
            return m_type;
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

        static Receiver begin() {
            return begin(MPI_ANY_SOURCE);
        }

        static Receiver begin(int receiver) {
            TypeTag type;
            MPI_Status status;
            MPI_Recv(&type,
                1,
                types::u32,
                receiver, static_cast<int>(Tag::TypeTag), MPI_COMM_WORLD, &status);
            std::cout << status.MPI_SOURCE <<  " -> (" << rank() << ") Began" << std::endl;
            return Receiver(status.MPI_SOURCE, type);
        }

        [[nodiscard]] Sender sender() const {
            return Sender(src);
        }
    };
}

#endif
#endif //RECEIVER_H
