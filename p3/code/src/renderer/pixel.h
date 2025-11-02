#ifndef PIXEL_H
#define PIXEL_H

#include <renderer/scene.h>
#include <util/types.h>

#include <atomic>
#include <cstring>
#include <stdatomic.h>

struct Pixel {
    Vector3<f32> ambient;
    Vector3<f32> diffuse;
    Vector3<f32> specular;

    TextureId ambient_map;
    TextureId diffuse_map;
    TextureId specular_map;
    TextureId normal_map;

    i32 shininess{};

    Vector2<f32> uv;

    Vector3<f32> normal;
    Vector3<f32> tangent;
    Vector3<f32> bitangent;

    Vector3<f32> position;

    u32 depth{0xFFFFFFFE};

    INLINE void set_smaller_depth(Pixel pixel) {
#ifdef USE_OPEN_MP
        auto ptr = (std::atomic<u32>*)(&this->depth);
        const u32 LOCK_VALUE = 0xFFFFFFFF;
        u32 depth;
        while ((depth = ptr->exchange(LOCK_VALUE, std::memory_order_acquire)) == 0xFFFFFFFF){}
        if (depth > pixel.depth) {
            depth = pixel.depth;
            std::memcpy(this, &pixel, sizeof(Pixel)-sizeof(pixel.depth));
        }
        ptr->store(depth, std::memory_order_release);
#else
        if (pixel.depth < this->depth) *this = pixel;
#endif
    }

    INLINE Pixel fragment_shader(ref<Scene> scene, ref<ResourceStore> resources) const;
};




#include <algorithm>

INLINE inline f32 power(f32 base, i32 exp) {
    f32 res = 1;
    while (exp > 0) {
        if (exp % 2 == 1) res *= base;
        base *= base;
        exp /= 2;
    }
    return res;
}

INLINE inline f32 DistributionGGX(Vector3<f32> N, Vector3<f32> H, f32 roughness)
{
    const f32 a = roughness*roughness;
    const f32 a2 = a*a;
    const f32 NdotH = std::max(N.dot(H), 0.0f);
    const f32 NdotH2 = NdotH*NdotH;

    const f32 nom   = a2;
    f32 denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = M_PIf * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
INLINE inline f32 GeometrySchlickGGX(f32 NdotV, f32 roughness)
{
    const f32 r = (roughness + 1.0f);
    const f32 k = (r*r) / 8.0f;

    const f32 nom   = NdotV;
    const f32 denom = NdotV * (1.0f - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
INLINE inline float GeometrySmith(Vector3<f32> N, Vector3<f32> V, Vector3<f32> L, f32 roughness)
{
    const f32 NdotV = std::max(N.dot(V), 0.0f);
    const f32 NdotL = std::max(N.dot(L), 0.0f);
    const f32 ggx2 = GeometrySchlickGGX(NdotV, roughness);
    const f32 ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
INLINE inline Vector3<f32> fresnelSchlick(f32 cosTheta, Vector3<f32> F0)
{
    return F0 + (Vector3<f32>{1.0f,1.0f,1.0f} - F0) * power(std::clamp(1.0f - cosTheta, 0.0f, 1.0f), 5);
}

[[clang::always_inline]]
INLINE inline Pixel Pixel::fragment_shader(ref<Scene> scene, ref<ResourceStore> resources) const {
    auto pixel = *this;
    if (pixel.normal.magnitude_squared() == 0.) return pixel;


    if (pixel.normal_map.exists()) {
        auto n = resources[pixel.normal_map]->resolve_uv_wrapping(pixel.uv).xyz();

        pixel.normal = pixel.normal.normalize();
        pixel.tangent = pixel.tangent.normalize();
        pixel.bitangent = pixel.bitangent.normalize();
        Matrix3<f32> tbn{
            pixel.tangent.x(), pixel.bitangent.x(), pixel.normal.x(),
            pixel.tangent.y(), pixel.bitangent.y(), pixel.normal.y(),
            pixel.tangent.z(), pixel.bitangent.z(), pixel.normal.z(),
        };

        // auto view_dir = (tbn*scene.m_camera.position-tbn*pixel.position).normalize();
        // auto height = resources[pixel.specular_map]->resolve_uv_wrapping(pixel.uv).x();
        // pixel.uv = pixel.uv - view_dir.xy() / view_dir.z()*height*0.5;

        pixel.normal = tbn * n;
    }
    pixel.normal = pixel.normal.normalize();

    if (pixel.ambient_map.exists()) {
        pixel.ambient = resources[pixel.ambient_map]->resolve_uv_wrapping(pixel.uv).xyz();
    }

    if (pixel.diffuse_map.exists() && !resources[pixel.diffuse_map]->transparent()) {
        pixel.diffuse = resources[pixel.diffuse_map]->resolve_uv_wrapping(pixel.uv).xyz();
    }

    auto shine = pixel.shininess;
    auto metalic = 0.0f;
    auto ambient = 0.1f;
    if (pixel.specular_map.exists()) {
        auto val = resources[pixel.specular_map]->resolve_uv_wrapping(pixel.uv).xyz();
        ambient = val.x();
        auto rough = val.y()*val.y();
        rough = rough*rough;
        shine = static_cast<i32>(1.f / (0.000001 + rough));
        metalic = val.z();
    }
    const auto view_dir = (scene.m_camera.position-pixel.position).normalize();

    Vector3<f32> specular_light{};
    Vector3<f32> diffuse_light{};
    for (const auto& light: scene.m_lights) {
        Vector3<f32> light_dir;
        f32 distance_squared;
        if (light.global) {
            distance_squared = 1;
            light_dir = light.position_or_direction;
        }else {
            light_dir = light.position_or_direction-pixel.position;
            distance_squared = light_dir.magnitude_squared();
            light_dir = light_dir/std::sqrt(distance_squared);
        }

        auto lambertian = std::max(0.f, light_dir.dot(pixel.normal));

        auto light_power = light.intensity/distance_squared;

        if (lambertian > 0.0) {
            auto half_dir = (light_dir + view_dir).normalize();

            auto specular = power(std::max(0.f, pixel.normal.dot(half_dir)), shine);

            auto color = light.color*metalic+pixel.diffuse.mult_components(light.color)*(1.f-metalic);
            specular_light = specular_light+color*(specular*light_power);

        }

        diffuse_light = diffuse_light+light.color*(lambertian*light_power);
    }


    auto color =
        (pixel.diffuse) * (ambient * (1.f-metalic))
        + pixel.diffuse.mult_components(diffuse_light)
        + (specular_light)
    ;


    if (pixel.specular_map.exists()) {
        pixel.specular = resources[pixel.specular_map]->resolve_uv_wrapping(pixel.uv).xyz();
    }

    pixel.diffuse = color;
    return pixel;
}

#endif //PIXEL_H
