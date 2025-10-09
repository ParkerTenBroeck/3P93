//
// Created by may on 05/10/25.
//

#ifndef OBJ_H
#define OBJ_H

#include <filesystem>
#include <memory>
#include <optional>
#include <utility>
#include <vector>
#include <variant>
#include <algorithm>

#include <tiny_obj_loader.h>

#include "texture.h"
#include "vec_math.h"
#include "resource_store.h"

class Face {
public:
    std::array<Vector3<f32>, 3> points;
    std::array<Vector3<f32>, 3> normals;
    std::array<Vector2<f32>, 3> uvs;
};

class Material {
public:
    Vector3<f32> ambient;
    Vector3<f32> diffuse;
    Vector3<f32> specular;
    u32 shininess{0};
    std::optional<std::shared_ptr<const Texture>> ambient_map;
    std::optional<std::shared_ptr<const Texture>> diffuse_map;
    std::optional<std::shared_ptr<const Texture>> specular_map;
    std::optional<std::shared_ptr<const Texture>> normal_map;
};

class Mesh {
public:
    std::string name{};
    std::vector<Face> m_faces{};
    Material m_material{};

    Mesh(std::string name, std::vector<Face> faces, Material material) : name {std::move(name)}, m_faces{std::move(faces)}, m_material{std::move(material)} {}
    Mesh()= default;
};

class Object {
public:
    Vector3<f32> m_position{0, 0, 0};
    Vector3<f32> m_rotation{0, 0, 0};
    Vector3<f32> m_scale{1, 1, 1};
    std::variant<Mesh, std::vector<Object>> m_kind;

    explicit Object(Mesh&& mesh) : m_kind{mesh} {}
    explicit Object(std::vector<Object>&& children) : m_kind{children} {}

    explicit Object(int _cpp_par_);

    static Object load(std::string&& path, ref_mut<ResourceStore> resource_store) {

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> objmaterials;
        std::string err;

        auto parent = std::filesystem::path(path).parent_path().string() + "/";
        bool success = tinyobj::LoadObj(&attrib, &shapes, &objmaterials, &err,
            path.c_str(),
            parent.c_str(),
            true);

        if (!success) {
            std::cout << "Failed to load OBJ file '" << err << "': " << path << std::endl;
            return Object(Mesh{});
        }
        if (!err.empty()) {
            std::cout << err << std::endl;
        }

        std::vector<Mesh> meshes{};
        meshes.emplace_back(Mesh{});
        for (const auto& mat : objmaterials) {

            auto ambient = mat.ambient_texname.empty()
                ? std::nullopt :
                std::optional{resource_store.get_texture(parent + mat.ambient_texname)};

            auto diffuse = mat.diffuse_texname.empty()
                ? std::nullopt :
                std::optional{resource_store.get_texture(parent + mat.diffuse_texname)};

            auto specular = mat.specular_texname.empty()
                ? std::nullopt :
                std::optional{resource_store.get_texture(parent + mat.specular_texname)};

            auto normal = mat.normal_texname.empty()
                ? std::nullopt :
                std::optional{resource_store.get_texture(parent + mat.normal_texname)};

            Material material{
                {mat.ambient[0], mat.ambient[1], mat.ambient[2]},
                {mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]},
                {mat.specular[0], mat.specular[1], mat.specular[2]},
                static_cast<u32>(mat.shininess),
                ambient,
                diffuse,
                specular,
                normal,
            };

            meshes.push_back(Mesh{
                "",
                {},
                material,
            });
        }



        for (const auto& mesh: shapes) {
            if (mesh.mesh.indices.empty())continue;

            for (usize i = 0; i < mesh.mesh.indices.size(); i += 3) {

                const auto i0 = mesh.mesh.indices[i];
                const auto i1 = mesh.mesh.indices[i + 1];
                const auto i2 = mesh.mesh.indices[i + 2];

                std::array<Vector3<f32>, 3> positions{
                    Vector3<f32>{attrib.vertices[3*i0.vertex_index], attrib.vertices[3*i0.vertex_index+1], attrib.vertices[3*i0.vertex_index+2]},
                    Vector3<f32>{attrib.vertices[3*i1.vertex_index], attrib.vertices[3*i1.vertex_index+1], attrib.vertices[3*i1.vertex_index+2]},
                    Vector3<f32>{attrib.vertices[3*i2.vertex_index], attrib.vertices[3*i2.vertex_index+1], attrib.vertices[3*i2.vertex_index+2]}
                };

                std::array<Vector3<f32>, 3> normals{
                    Vector3<f32>{attrib.normals[3*i0.normal_index], attrib.normals[3*i0.normal_index+1], attrib.normals[3*i0.normal_index+2]},
                    Vector3<f32>{attrib.normals[3*i1.normal_index], attrib.normals[3*i1.normal_index+1], attrib.normals[3*i1.normal_index+2]},
                    Vector3<f32>{attrib.normals[3*i2.normal_index], attrib.normals[3*i2.normal_index+1], attrib.normals[3*i2.normal_index+2]}
                };
                std::array<Vector2<f32>, 3> uvs{
                    Vector2<f32>{attrib.texcoords[2*i0.texcoord_index], 1.f-attrib.texcoords[2*i0.texcoord_index+1]},
                    Vector2<f32>{attrib.texcoords[2*i1.texcoord_index], 1.f-attrib.texcoords[2*i1.texcoord_index+1]},
                    Vector2<f32>{attrib.texcoords[2*i2.texcoord_index], 1.f-attrib.texcoords[2*i2.texcoord_index+1]}
                };
                auto idx = mesh.mesh.material_ids[i/3]+1;

                meshes[idx].m_faces.push_back(Face{
                    positions,
                    normals,
                    uvs,
                });
                if (meshes[idx].name.empty()) {
                    meshes[idx].name = mesh.name;
                }
            }
        }

        meshes.erase(std::remove_if(meshes.begin(), meshes.end(), [](const auto& mesh) {
            if (mesh.m_faces.empty()) return true;
            std::cout << "Mesh " << mesh.name << " loaded with " << mesh.m_faces.size() << " faces" << "\n";
            return false;
        }), meshes.end());

        if (meshes.empty()) {
            std::cout << "Failed to load OBJ file: " << path << std::endl;
            return Object(Mesh{});
        }
        std::cout << "Loaded OBJ file: " << path << std::endl;
        if (meshes.size() == 1) {
            return Object(std::move(meshes[0]));
        }

        std::vector<Object> children;
        for (auto& child: meshes) {
            children.emplace_back(std::move(child));
        }
        return Object(std::move(children));
    }

    [[nodiscard]]
    Matrix4<f32> model() const {
        const Matrix4<f32> rotation{Matrix3<f32>::rotation(m_rotation)};
        const Matrix4<f32> scale{
            m_scale.x(), 0, 0, 0,
            0, m_scale.y(), 0, 0,
            0, 0, m_scale.z(), 0,
            0, 0, 0, 1
        };
        auto model = rotation*scale;
        model[{0, 3}] = m_position.x();
        model[{1, 3}] = m_position.y();
        model[{2, 3}] = m_position.z();
        model[{3, 3}] = 1;
        return model;
    }
};

#endif //OBJ_H
