//
// Created by may on 05/10/25.
//

#ifndef RESOURCE_STORE_H
#define RESOURCE_STORE_H
#include <map>
#include <vector>

#include "scene.h"

class ResourceStore {
    std::vector<std::shared_ptr<const Texture>> textures{};
    std::map<std::string, std::shared_ptr<const Texture>> textures_map{};

public:
    ResourceStore(){}

    std::shared_ptr<const Texture> get_texture(const std::string& path) {
        auto it = textures_map.find(path);
        if (it == textures_map.end()) {
            textures_map[path] = std::make_shared<Texture>(path, TextureId(textures.size()));
            textures.emplace_back(textures_map[path]);
        }
        return textures_map[path];
    }

    ref<std::shared_ptr<const Texture>> operator[](const TextureId texture) const {
        return this->textures[texture.id];
    }
};

#endif //RESOURCE_STORE_H
