#ifndef FS_H
#define FS_H

#include <fstream>
#include <mpi/types.h>
#include <memory>
#include <sstream>
#include <mpi/sender.h>
#include <mpi/receiver.h>

inline std::string file_load_string(const std::string& path) {
#ifdef USE_OPEN_MPI
    if (mpi::is_slave()) {
        return mpi::Sender::begin_master(mpi::TypeTag::M2S_LoadFile).send(path).receiver().receive<std::string>();
    }
#endif

    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Failed to open file: " + std::string(path));

    file.seekg(0, std::ios::end);
    const size_t size = file.tellg();
    std::string buffer(size, '\0');
    file.seekg(0);
    file.read(buffer.data(), size);

    return buffer;
}

inline std::unique_ptr<std::istream> file_load_istream(const std::string& path) {
#ifdef USE_OPEN_MPI
    if (mpi::is_slave()) {
        return std::make_unique<std::istringstream>(file_load_string(path));
    }
#endif
    return std::make_unique<std::ifstream>(path);
}

#endif //FS_H
