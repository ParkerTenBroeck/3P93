#ifndef RESOURCE_STORE_H
#define RESOURCE_STORE_H

#include <map>
#include <vector>

#include <resources/texture.h>

#include "util/fs.h"

/**
 * Stores already loaded textures to prevent loading the same textures multiple times and also allow us to get a texture from a texture_id
 */
class ResourceStore {
    std::vector<std::shared_ptr<const Texture>> textures{};
    std::map<std::string, std::shared_ptr<const Texture>> map_textures{};
    std::map<std::string, std::shared_ptr<const Texture>> normal_textures{};
    std::map<std::string, std::shared_ptr<const Texture>> rgba_gamma_corrected_textures{};
    friend struct Texture;

public:
    ResourceStore() = default;

    std::shared_ptr<const Texture> normal_map(ref<std::string> path) {
        if (normal_textures.count(path) != 0) {
            return normal_textures[path];
        }

        auto contents = file_load_string(path);
        i32 width, height, channels;
        auto result = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(contents.data()), (int)contents.size(), &width, &height, &channels, 4);

        if (!result)
            std::cout << "Failed to load texture: " << path << std::endl;
        if (width == 0)
            std::cout << "Texture width cannot be zero: " << path << std::endl;
        if (height == 0)
            std::cout << "Texture height cannot be zero: " << path << std::endl;

        std::shared_ptr<Texture> shared;
        if (!result || width == 0 || height == 0) {
            Texture texture{1, 1, false, new Vector4<f32>[1]};
            texture.m_pixels[0] = {0, 0, 1.};

            shared = std::make_shared<Texture>(std::move(texture));
        }else{
            auto w = static_cast<usize>(width);
            auto h = static_cast<usize>(height);
            Texture texture{w, h, false, new Vector4<f32>[w*h]};

            for (usize i = 0; i < w*h; i++) {
                texture.m_pixels[i] = {result[i*4]/255.f*2-1, result[i*4+1]/255.f*2-1, result[i*4+2]/255.f*2-1, result[i*4+3]/255.f*2-1};
                texture.m_transparent |= result[i*4+3] != 1.;
            }

            std::cout << "Loaded normal map texture: " << path << " width: " << texture.width() << " height: " << texture.height() << " transparent: " << texture.transparent() << std::endl;

            shared = std::make_shared<Texture>(std::move(texture));
        }

        stbi_image_free(result);

        shared->m_id = TextureId(textures.size()+1);
        normal_textures[path] = shared;
        textures.emplace_back(normal_textures[path]);

        return shared;
    }

    std::shared_ptr<const Texture> map(ref<std::string> path) {
        if (map_textures.count(path) != 0) {
            return map_textures[path];
        }

        auto contents = file_load_string(path);
        i32 width, height, channels;
        auto result = stbi_load_from_memory(reinterpret_cast<stbi_uc const *>(contents.data()), (int)contents.size(), &width, &height, &channels, 4);

        if (!result)
            std::cout << "Failed to load texture: " << path << std::endl;
        if (width == 0)
            std::cout << "Texture width cannot be zero: " << path << std::endl;
        if (height == 0)
            std::cout << "Texture height cannot be zero: " << path << std::endl;

        std::shared_ptr<Texture> shared;
        if (!result || width == 0 || height == 0) {
            Texture texture{1, 1, false, new Vector4<f32>[1]};
            texture.m_pixels[0] = {0, 0, 1.};

            shared = std::make_shared<Texture>(std::move(texture));
        }else{
            auto w = static_cast<usize>(width);
            auto h = static_cast<usize>(height);
            Texture texture{w, h, false, new Vector4<f32>[w*h]};

            for (usize i = 0; i < w*h; i++) {
                texture.m_pixels[i] = {result[i*4]/255.f, result[i*4+1]/255.f, result[i*4+2]/255.f, result[i*4+3]/255.f};
                texture.m_transparent |= result[i*4+3] != 1.;
            }

            std::cout << "Loaded map texture: " << path << " width: " << texture.width() << " height: " << texture.height() << " transparent: " << texture.transparent() << std::endl;

            shared = std::make_shared<Texture>(std::move(texture));
        }

        stbi_image_free(result);

        shared->m_id = TextureId(textures.size()+1);
        map_textures[path] = shared;
        textures.emplace_back(map_textures[path]);

        return shared;
    }

    std::shared_ptr<const Texture> rgba_gamma_corrected(ref<std::string> path) {
        if (rgba_gamma_corrected_textures.count(path) != 0) {
            return rgba_gamma_corrected_textures[path];
        }

        auto contents = file_load_string(path);
        i32 width, height, channels;
        auto result = stbi_loadf_from_memory(reinterpret_cast<stbi_uc const *>(contents.data()), (int)contents.size(), &width, &height, &channels, 4);

        if (!result)
            std::cout << "Failed to load texture: " << path << std::endl;
        if (width == 0)
            std::cout << "Texture width cannot be zero: " << path << std::endl;
        if (height == 0)
            std::cout << "Texture height cannot be zero: " << path << std::endl;

        std::shared_ptr<Texture> shared;
        if (!result || width == 0 || height == 0) {
            Texture texture{1, 1, false, new Vector4<f32>[1]};
            texture.m_pixels[0] = {0, 0, 1.};

            shared = std::make_shared<Texture>(std::move(texture));
        }else{
            auto w = static_cast<usize>(width);
            auto h = static_cast<usize>(height);
            Texture texture{w, h, false, new Vector4<f32>[w*h]};

            for (usize i = 0; i < w*h; i++) {
                texture.m_pixels[i] = {result[i*4], result[i*4+1], result[i*4+2], result[i*4+3]};
                texture.m_transparent |= result[i*4+3] != 1.f;
            }

            std::cout << "Loaded rgba gamma corrected texture: " << path << " width: " << texture.width() << " height: " << texture.height() << " transparent: " << texture.transparent() << std::endl;

            shared = std::make_shared<Texture>(std::move(texture));
        }

        stbi_image_free(result);

        shared->m_id = TextureId(textures.size()+1);
        rgba_gamma_corrected_textures[path] = shared;
        textures.emplace_back(rgba_gamma_corrected_textures[path]);

        return shared;
    }

    std::shared_ptr<const Texture> rgba(ref<std::string> path) {
        return map(path);
    }

    ref<std::shared_ptr<const Texture>> operator[](const TextureId texture) const {
        return this->textures[texture.id-1];
    }
};

#endif //RESOURCE_STORE_H
