#ifndef MPI_CODECS_H
#define MPI_CODECS_H
#ifdef USE_OPEN_MPI

#include <args.h>
#include <mpi/sender.h>
#include <mpi/receiver.h>
#include <mpi/codecs.h>
#include <util/types.h>
#include <resources/texture.h>


namespace mpi {
    #define BASE_CODEC(ty, mpi_ty) template<>struct codec<ty> : codec_base {\
        static constexpr bool is_primitive() noexcept { return true;}\
        static constexpr const types::type& type() noexcept{return mpi_ty;}\
        static constexpr usize count() noexcept{return 1;}\
    };

    BASE_CODEC(f32, types::f32);
    BASE_CODEC(f64, types::f64);
    BASE_CODEC(i8, types::i8);
    BASE_CODEC(i16, types::i16);
    BASE_CODEC(i32, types::i32);
    BASE_CODEC(i64, types::i64);
    BASE_CODEC(u8, types::u8);
    BASE_CODEC(u16, types::u16);
    BASE_CODEC(u32, types::u32);
    BASE_CODEC(u64, types::u64);
    BASE_CODEC(char, types::character);
    BASE_CODEC(bool, types::boolean);
    BASE_CODEC(TextureId, types::u32);

#undef BASE_CODEC

    template<typename T, usize R, usize C>
    struct codec<Matrix<T, R, C>> {
        static constexpr bool is_primitive() noexcept { return codec<T>::is_primitive();}
        static constexpr const types::type& type() noexcept{return codec<T>::type();}
        static constexpr usize count() noexcept {
            return codec<T>::count() * R * C;
        }
    };

    template<typename T, usize L>
    struct codec<std::array<T, L>> {
        static constexpr bool is_primitive() noexcept { return codec<T>::is_primitive();}
        static constexpr const types::type& type() noexcept{return codec<T>::type();}
        static usize count() noexcept {
            return codec<T>::count() * L;
        }
    };


    template<>
    struct codec<std::string> : codec_base {
        static void send(const std::string& value, Sender& sender) {
            sender.send(static_cast<u32>(value.size()));
            sender.send(value.data(), value.size());
        }

        static std::string receive(Receiver& receiver) {
            std::string string{};
            receive(&string, receiver);
            return string;
        }

        static void receive(std::string* value, Receiver& receiver) {
            const auto size = receiver.receive<u32>();
            value->resize(size);
            receiver.receive(value->data(), size);
        }
    };

    template<>
    struct codec<Texture> : codec_base {
        static void send(const Texture& value, Sender& sender) {
            const u32 width = value.m_width, height = value.m_height;
            sender
                .send(width)
                .send(height)
                .send(value.m_transparent)
                .send(value.m_widthf)
                .send(value.m_heightf)
                .send(value.m_id)
                .send(value.m_pixels, width*height);
        }

        static Texture receive(Receiver& receiver) {
            Texture texture;
            receive(&texture, receiver);
            return texture;
        }

        static void receive(Texture* value, Receiver& receiver) {
            u32 height, width;
            receiver
                .receive(&width)
                .receive(&height)
                .receive(&value->m_transparent)
                .receive(&value->m_widthf)
                .receive(&value->m_heightf)
                .receive(&value->m_id);
            value->m_width = width;
            value->m_height = height;
            value->m_pixels = new Vector4<f32>[height*width];
            receiver.receive(value->m_pixels, width*height);
        }
    };

    template<>
    struct codec<Scenes> : codec_base {
        static void send(const Scenes& value, Sender& sender) {
            sender
                .send((u32)value);
        }

        static Scenes receive(Receiver& receiver) {
            Scenes scene = Scenes::Brick;
            receive(&scene, receiver);
            return scene;
        }

        static void receive(Scenes* value, Receiver& receiver) {
            receiver
                .receive((u32*)(value));
        }
    };

    template<>
    struct codec<Arguments> : codec_base {
        static void send(const Arguments& value, Sender& sender) {
            sender
                .send(value.height)
                .send(value.width)
                .send(value.write_frames)
                .send(value.frames)
                .send(value.scene);
        }

        static Arguments receive(Receiver& receiver) {
            Arguments args;
            receive(&args, receiver);
            return args;
        }

        static void receive(Arguments* value, Receiver& receiver) {
            receiver
                .receive(&value->height)
                .receive(&value->width)
                .receive(&value->write_frames)
                .receive(&value->frames)
                .receive(&value->scene);
        }
    };


    template<>
    struct codec<Empty> : codec_base {
        static void send(const Empty&, Sender& sender) {
            sender.send("", 0);
        }

        static Empty receive(Receiver& receiver) {
            receiver.receive<char>(nullptr, 0);
            return Empty{};
        }

        static void receive(Empty*, Receiver& receiver) {
            receiver.receive<char>(nullptr, 0);
        }
    };
}

#endif
#endif //MPI_CODECS_H
