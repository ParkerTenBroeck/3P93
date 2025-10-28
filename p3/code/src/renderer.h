

#ifndef RENDERER_H
#define RENDERER_H

#include "scene.h"
#include "vec_math.h"
#include "frame_buffer.h"
#include "obj.h"

#include <variant>
#include <algorithm>
#include <cmath>

struct Renderer {

    static void render(ref_mut<FrameBuffer> frame, ref<Scene> scene, ref<ResourceStore> resources) {
        clear(frame);
        render_scene(frame, scene);
        fragment(frame, scene, resources);
        render_lights(frame, scene);
    }

    static void render_lights(ref_mut<FrameBuffer> frame, ref<Scene> scene) {
        Vector2<f32> screen{static_cast<f32>(frame.width()), static_cast<f32>(frame.height())};
        auto proj_view = scene.proj_view(screen);

        auto size_projection = 1.f/std::tan(scene.m_camera.fov/2)* frame.width();

        for (const auto& light : scene.m_lights) {
            if (light.global) continue;
            auto cs = proj_view*light.position_or_direction.extend(1);

            if (cs.z() > cs.w()) {
                continue;
            }
            if (cs.z() < -cs.w()) {
                continue;
            }
            if (cs.x() < -cs.w()) {
                continue;
            }
            if (cs.x() > cs.w()) {
                continue;
            }
            if (cs.y() < -cs.w()) {
                continue;
            }
            if (cs.y() > cs.w()) {
                continue;
            }

            auto ps = perspective(cs);
            auto ss = screen_space(ps, screen);
            const auto perspective_size = std::clamp( light.radius / cs.w() * size_projection, 0.f, (f32)frame.width());
            const auto size = static_cast<isize>(perspective_size);

            for (isize x = -size; x <= size; ++x) {
                for (isize y = -size; y <= size; ++y) {
                    if (x*x + y*y > perspective_size*perspective_size) continue;
                    auto pos = ss.xy() + Vector2<f32>({static_cast<f32>(x), static_cast<f32>(y)});

                    if (pos.x() < 0 || pos.x() > frame.width() || pos.y() < 0 || pos.y() > frame.height()) continue;
                    Vector2<usize> pixel_cord{static_cast<usize>(pos.x()), static_cast<usize>(pos.y())};
                    if (frame[pixel_cord].depth >= convert_depth(ps.z())) {
                        frame[pixel_cord].diffuse = light.color;
                        frame[pixel_cord].normal = {1,1,1}; // lmao
                    }
                }
            }
        }

    }

    static void fragment(ref_mut<FrameBuffer> frame, ref<Scene> scene, ref<ResourceStore> resources) {
        #ifdef PAR
        #pragma omp parallel for
        #endif
        for (usize i = 0; i < frame.size(); i ++) {
            frame[i] = frame[i].fragment_shader(scene, resources);
        }
    }

    static void clear(ref_mut<FrameBuffer> frame) {
        #ifdef PAR
        #pragma omp parallel for
        #endif
        for (usize i = 0; i < frame.size(); i ++) {
            frame[i] = Pixel();
        }
    }

    static void render_scene(ref_mut<FrameBuffer> frame, ref<Scene> scene) {
        Vector2<f32> screen{static_cast<f32>(frame.width()), static_cast<f32>(frame.height())};
        auto proj_view = scene.proj_view(screen);
        auto model = Matrix4<f32>{
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        for (auto& object: scene.objects()) {
            render_object(frame, object, model, proj_view);
        }
    }

    static void render_object(ref_mut<FrameBuffer> frame, ref<Object> object, ref<Matrix4<f32>> model_matrix, ref<Matrix4<f32>> proj_view) {
        if (object.m_kind.index() == 0){
            render_mesh(frame, std::get<Mesh>(object.m_kind), model_matrix*object.model(), proj_view);
        }else {
            for (const auto& child: std::get<std::vector<Object>>(object.m_kind)) {
                render_object(frame, child, model_matrix*object.model(), proj_view);
            }
        }
    }

    static void render_mesh(ref_mut<FrameBuffer> frame, ref<Mesh> mesh, ref<Matrix4<f32>> model_matrix, ref<Matrix4<f32>> proj_view) {
        Vector2<f32> screen{static_cast<f32>(frame.width()), static_cast<f32>(frame.height())};
        Matrix3<f32> normal_matrix{model_matrix.inverse().transpose()};
        #ifdef PAR
        #pragma omp parallel for
        #endif
        for (const auto& face: mesh.m_faces) {
            auto ms0 = face.points[0].extend(1);
            auto ms1 = face.points[1].extend(1);
            auto ms2 = face.points[2].extend(1);

            auto ws0 = model_matrix * ms0;
            auto ws1 = model_matrix * ms1;
            auto ws2 = model_matrix * ms2;

            auto cs0 = proj_view * ws0;
            auto cs1 = proj_view * ws1;
            auto cs2 = proj_view * ws2;

            auto norm = (cs1.xyz() - cs0.xyz())
                .cross(cs2.xyz() - cs0.xyz())
                .normalize();
            //   backface culling
            if (cs0.xyz().dot(norm) <= 0.0 ){
                continue;
            }

            if (cs0.z() > cs0.w() && cs1.z() > cs2.w() && cs2.z() > cs2.w()) {
                continue;
            }
            if (cs0.x() < -cs0.w() && cs1.x() < -cs1.w() && cs2.x() < -cs2.w()) {
                continue;
            }
            if (cs0.x() > cs0.w() && cs1.x() > cs1.w() && cs2.x() > cs2.w()) {
                continue;
            }
            if (cs0.y() < -cs0.w() && cs1.y() < -cs1.w() && cs2.y() < -cs2.w()) {
                continue;
            }
            if (cs0.y() > cs0.w() && cs1.y() > cs1.w() && cs2.y() > cs2.w()) {
                continue;
            }

            auto b0 = cs0.z() < -cs0.w();
            auto b1 = cs1.z() < -cs1.w();
            auto b2 = cs2.z() < -cs2.w();

            if (b0||b1||b2)continue;

            render_triangle(
                frame,
                mesh.m_material,
                ms0, ms1, ms2,
                ws0, ws1, ws2,
                cs0, cs1, cs2,
                face.normals[0], face.normals[1], face.normals[2],
                face.uvs[0], face.uvs[1], face.uvs[2],

                model_matrix,
                proj_view,
                normal_matrix,
                screen
            );
        }
    }

    INLINE static void render_triangle(
        ref_mut<FrameBuffer> frame,
        ref<Material> material,

        ref<Vector4<f32>> ms0, ref<Vector4<f32>> ms1, ref<Vector4<f32>> ms2,
        ref<Vector4<f32>> ws0, ref<Vector4<f32>> ws1, ref<Vector4<f32>> ws2,
        ref<Vector4<f32>> cs0, ref<Vector4<f32>> cs1, ref<Vector4<f32>> cs2,
        ref<Vector3<f32>> n0, ref<Vector3<f32>> n1, ref<Vector3<f32>> n2,
        ref<Vector2<f32>> uv0, ref<Vector2<f32>> uv1, ref<Vector2<f32>> uv2,

        ref<Matrix4<f32>> /* model_matrix */,
        ref<Matrix4<f32>> /* proj_view */,
        ref<Matrix3<f32>> normal_matrix,
        ref<Vector2<f32>> screen
        ) {
        const auto w0 = cs0.w();
        const auto w1 = cs1.w();
        const auto w2 = cs2.w();

        auto uv0_e = uv0.extend(1);
        auto uv1_e = uv1.extend(1);
        auto uv2_e = uv2.extend(1);

        const auto ps0 = perspective(cs0);
        const auto ps1 = perspective(cs1);
        const auto ps2 = perspective(cs2);

        const auto ss0 = screen_space(ps0, screen);
        const auto ss1 = screen_space(ps1, screen);
        const auto ss2 = screen_space(ps2, screen);

        const auto n0_c = (normal_matrix * n0) / w0;
        const auto n1_c = (normal_matrix * n1) / w1;
        const auto n2_c = (normal_matrix * n2) / w2;

        Vector3<f32> t0{}, t1{}, t2{};
        Vector3<f32> bt0{}, bt1{}, bt2{};

        if (material.normal_map.has_value()) {
            auto g0 = tangent_bitangent(
                {ms0.xyz(), ms1.xyz(), ms2.xyz()},
                {uv0_e, uv1_e, uv2_e},
                w0,
                normal_matrix
            );
            t0 = g0[0];
            bt0 = g0[1];

            auto g1 = tangent_bitangent(
                {ms1.xyz(), ms2.xyz(), ms0.xyz()},
                {uv1_e, uv2_e, uv0_e},
                w1,
                normal_matrix
            );
            t1 = g1[0];
            bt1 = g1[1];

            auto g2 = tangent_bitangent(
                {ms2.xyz(), ms0.xyz(), ms1.xyz()},
                {uv2_e, uv0_e, uv1_e},
                w2,
                normal_matrix
            );
            t2 = g2[0];
            bt2 = g2[1];
        }

        uv0_e = uv0_e / w0;
        uv1_e = uv1_e / w1;
        uv2_e = uv2_e / w2;


        TextureId ambient_map{};
        if (material.ambient_map.has_value()) {
            ambient_map = material.ambient_map.value()->get_id();
        }

        TextureId diffuse_map{};
        if (material.diffuse_map.has_value()) {
            diffuse_map = material.diffuse_map.value()->get_id();
        }

        TextureId specular_map{};
        if (material.specular_map.has_value()) {
            specular_map = material.specular_map.value()->get_id();
        }

        TextureId normal_map{};
        if (material.normal_map.has_value()) {
            normal_map = material.normal_map.value()->get_id();
        }

        if (material.diffuse_map.has_value() && material.diffuse_map->get()->transparent()) {
            draw_triangle_filled_textured(
               frame,
               ss0, ss1, ss2,
                ws0.xyz()/w0, ws1.xyz()/w1, ws2.xyz()/w2,
                n0_c, n1_c, n2_c,
                t0, t1, t2,
                bt0, bt1, bt2,
                uv0_e, uv1_e, uv2_e,
               material.ambient,
               material.specular,
                material.shininess,
               ambient_map,
               **material.diffuse_map,
               specular_map,
               normal_map
           );
        }else {
            draw_triangle_filled_textured_deferred(
               frame,
               ss0, ss1, ss2,
                ws0.xyz()/w0, ws1.xyz()/w1, ws2.xyz()/w2,
               n0_c, n1_c, n2_c,
               t0, t1, t2,
               bt0, bt1, bt2,
               uv0_e, uv1_e, uv2_e,
               material.ambient,
               material.diffuse,
               material.specular,
               material.shininess,
               ambient_map,
               diffuse_map,
               specular_map,
               normal_map
           );
        }
    }

    [[clang::always_inline]]
    INLINE static Vector4<f32> perspective(ref<Vector4<f32>> cs) {
        return {
            cs.x() / cs.w(),
            cs.y() / cs.w(),
            cs.z() / cs.w(),
            cs.w(),
       };
    }

    [[clang::always_inline]]
    INLINE static Vector3<f32> screen_space(ref<Vector4<f32>> ps, ref<Vector2<f32>> screen) {
        return {
            (ps.x()+0.5f) * screen.x(),
            (-ps.y()+0.5f) * screen.y(),
            ps.z(),
       };
    }

    [[clang::always_inline]]
    INLINE static std::array<Vector3<f32>, 2> tangent_bitangent(
        std::array<Vector3<f32>, 3> ws,
        std::array<Vector3<f32>, 3> uv,
        const f32 w,
        ref<Matrix3<f32>> normal_matrix
        )  {
        auto edge1 = ws[1] - ws[0];
        auto edge2 = ws[2] - ws[0];
        auto delta_uv1 = uv[1] - uv[0];
        auto delta_uv2 = uv[2] - uv[0];

        auto f = 1.0f / (delta_uv1.x() * delta_uv2.y() - delta_uv2.x() * delta_uv1.y());

        Vector3<f32> tangent{};

        tangent.x() = f * (delta_uv2.y() * edge1.x() - delta_uv1.y() * edge2.x());
        tangent.y() = f * (delta_uv2.y() * edge1.y() - delta_uv1.y() * edge2.y());
        tangent.z() = f * (delta_uv2.y() * edge1.z() - delta_uv1.y() * edge2.z());

        Vector3<f32> bitangent{};

        bitangent.x() = f * (-delta_uv2.x() * edge1.x() + delta_uv1.x() * edge2.x());
        bitangent.y() = f * (-delta_uv2.x() * edge1.y() + delta_uv1.x() * edge2.y());
        bitangent.z() = f * (-delta_uv2.x() * edge1.z() + delta_uv1.x() * edge2.z());

        return {
            (normal_matrix * tangent) / w,
            (normal_matrix * bitangent) / w
        };
    }


    #define rasterize_triangle(func) \
        const auto min_x = std::max(std::min({ss0.x(), ss1.x(), ss2.x()}), 0.f);\
        const auto max_x = std::min(std::max({ss0.x(), ss1.x(), ss2.x()}), static_cast<f32>(frame.width())-1);\
        const auto min_y = std::max(std::min({ss0.y(), ss1.y(), ss2.y()}), 0.f);\
        const auto max_y = std::min(std::max({ss0.y(), ss1.y(), ss2.y()}), static_cast<f32>(frame.height())-1);\
\
\
        auto denom = (ss1.y() - ss2.y()) * (ss0.x() - ss2.x()) + (ss2.x() - ss1.x()) * (ss0.y() - ss2.y());\
\
        if (denom == 0.0f) {\
            return;\
        }\
\
\
        for (auto y = static_cast<isize>(min_y); y <= static_cast<isize>(max_y); y++) {\
            for (auto x = static_cast<isize>(min_x); x <= static_cast<isize>(max_x); x++) {\
                Vector2<f32> pf{static_cast<f32>(x), static_cast<f32>(y)};\
\
                auto w0 = ((ss1.y() - ss2.y()) * (pf.x() - ss2.x()) + (ss2.x() - ss1.x()) * (pf.y() - ss2.y())) / denom;\
                auto w1 = ((ss2.y() - ss0.y()) * (pf.x() - ss2.x()) + (ss0.x() - ss2.x()) * (pf.y() - ss2.y())) / denom;\
                auto w2 = 1.0f - w0 - w1;\
\
                if (w0 >= 0.0 && w1 >= 0.0 && w2 >= 0.0) {\
                    Vector2<usize> pix{static_cast<usize>(x), static_cast<usize>(y)};\
                    func\
                }\
            }\
        }\

    [[clang::always_inline]]
    INLINE static void draw_triangle_filled_textured(
            ref_mut<FrameBuffer> frame,
            Vector3<f32> ss0, Vector3<f32> ss1, Vector3<f32> ss2,
            Vector3<f32> ws0, Vector3<f32> ws1, Vector3<f32> ws2,
            Vector3<f32> n0, Vector3<f32> n1, Vector3<f32> n2,
            Vector3<f32> t0, Vector3<f32> t1, Vector3<f32> t2,
            Vector3<f32> bt0, Vector3<f32> bt1, Vector3<f32> bt2,
            Vector3<f32> uv0, Vector3<f32> uv1, Vector3<f32> uv2,
            Vector3<f32> ambient,
            Vector3<f32> specular,
            u32 shininess,
            TextureId ambient_map,
            ref<Texture> diffuse_map,
            TextureId specular_map,
            TextureId normal_map
        ) {
        rasterize_triangle({

            if (pix.x() < 0 || pix.x() > frame.width() || pix.y() < 0 || pix.y() > frame.height()) return;
            auto depth = w0 * ss0.z() + w1 * ss1.z() + w2 * ss2.z();
            if (depth > 1 || depth < 0) return;

            auto pix_uv = uv0 * w0 + uv1 * w1 + uv2 * w2;
            auto frac_1_w = pix_uv.z();
            pix_uv = pix_uv/frac_1_w;

            auto color = diffuse_map.resolve_uv_wrapping(pix_uv.xy());
            if (color.w() == 0) {
               return;
            }

            Pixel pixel;
            pixel.ambient = ambient;
            pixel.specular = specular;
            pixel.diffuse = color.xyz();
            pixel.ambient_map = ambient_map;
            pixel.specular_map = specular_map;
            pixel.shininess = shininess;
            pixel.uv = pix_uv.xy();
            pixel.normal = (n0 * w0 + n1 * w1 + n2 * w2)/frac_1_w;
            pixel.tangent = (t0 * w0 + t1 * w1 + t2 * w2)/frac_1_w;
            pixel.bitangent = (bt0 * w0 + bt1 * w1 + bt2 * w2)/frac_1_w;
            pixel.position = (ws0 * w0 + ws1 * w1 + ws2 * w2)/frac_1_w;
            pixel.normal_map = normal_map;
            pixel.depth = convert_depth(depth);
            frame[pix].set_smaller_depth(pixel);
        });
    }

    [[clang::always_inline]]
    INLINE static void draw_triangle_filled_textured_deferred(
            ref_mut<FrameBuffer> frame,
            Vector3<f32> ss0, Vector3<f32> ss1, Vector3<f32> ss2,
            Vector3<f32> ws0, Vector3<f32> ws1, Vector3<f32> ws2,
            Vector3<f32> n0, Vector3<f32> n1, Vector3<f32> n2,
            Vector3<f32> t0, Vector3<f32> t1, Vector3<f32> t2,
            Vector3<f32> bt0, Vector3<f32> bt1, Vector3<f32> bt2,
            Vector3<f32> uv0, Vector3<f32> uv1, Vector3<f32> uv2,
            Vector3<f32> ambient,
            Vector3<f32> diffuse,
            Vector3<f32> specular,
            u32 shininess,
            TextureId ambient_map,
            TextureId diffuse_map,
            TextureId specular_map,
            TextureId normal_map
        ) {

        rasterize_triangle({

            if (pix.x() < 0 || pix.x() > frame.width() || pix.y() < 0 || pix.y() > frame.height()) return;
            auto depth = w0 * ss0.z() + w1 * ss1.z() + w2 * ss2.z();
            if (depth > 1 || depth < 0) return;

            auto pix_uv = uv0 * w0 + uv1 * w1 + uv2 * w2;
            auto frac_1_w = pix_uv.z();
            pix_uv = pix_uv/frac_1_w;

            Pixel pixel;
            pixel.ambient = ambient;
            pixel.diffuse = diffuse;
            pixel.specular = specular;
            pixel.ambient_map = ambient_map;
            pixel.diffuse_map = diffuse_map;
            pixel.specular_map = specular_map;
            pixel.shininess = shininess;
            pixel.uv = pix_uv.xy();
            pixel.normal = ((n0 * w0 + n1 * w1 + n2 * w2)/frac_1_w).xyz();
            pixel.tangent = ((t0 * w0 + t1 * w1 + t2 * w2)/frac_1_w).xyz();
            pixel.bitangent = ((bt0 * w0 + bt1 * w1 + bt2 * w2)/frac_1_w).xyz();
            pixel.position = (ws0 * w0 + ws1 * w1 + ws2 * w2)/frac_1_w;
            pixel.normal_map = normal_map;
            pixel.depth = convert_depth(depth);
            frame[pix].set_smaller_depth(pixel);
        });
    }

    [[clang::always_inline]]
    INLINE static u32 convert_depth(f32 depth) {
        return static_cast<u32>(depth * static_cast<f32>(0xFFFFFFFEull));
    }
};

#endif //RENDERER_H
