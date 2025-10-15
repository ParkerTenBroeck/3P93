

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
            auto perspective_size = std::min( light.radius / cs.w(), 1.f);
            isize size = std::max(static_cast<isize>(1), static_cast<isize>(perspective_size * frame.width()));

            for (isize x = -size; x <= size; ++x) {
                for (isize y = -size; y <= size; ++y) {
                    if (x*x + y*y > size*size) continue;
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
                {ms0, ms1, ms2},
                {ws0, ws1, ws2},
                {cs0, cs1, cs2},
                face.normals,
                face.uvs,

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

        std::array<Vector4<f32>, 3> ms,
        std::array<Vector4<f32>, 3> ws,
        std::array<Vector4<f32>, 3> cs,
        std::array<Vector3<f32>, 3> n,
        std::array<Vector2<f32>, 3> uv,

        ref<Matrix4<f32>> /* model_matrix */,
        ref<Matrix4<f32>> /* proj_view */,
        ref<Matrix3<f32>> normal_matrix,
        ref<Vector2<f32>> screen
        ) {
        const auto w0 = cs[0].w();
        const auto w1 = cs[1].w();
        const auto w2 = cs[2].w();

        auto uv0 = uv[0].extend(1);
        auto uv1 = uv[1].extend(1);
        auto uv2 = uv[2].extend(1);

        const auto ps0 = perspective(cs[0]);
        const auto ps1 = perspective(cs[1]);
        const auto ps2 = perspective(cs[2]);

        const auto ss0 = screen_space(ps0, screen);
        const auto ss1 = screen_space(ps1, screen);
        const auto ss2 = screen_space(ps2, screen);

        const auto n0 = (normal_matrix * n[0]) / w0;
        const auto n1 = (normal_matrix * n[1]) / w1;
        const auto n2 = (normal_matrix * n[2]) / w2;

        Vector3<f32> t0{}, t1{}, t2{};
        Vector3<f32> bt0{}, bt1{}, bt2{};

        if (material.normal_map.has_value()) {
            auto g0 = tangent_bitangent(
                {ms[0].xyz(), ms[1].xyz(), ms[2].xyz()},
                {uv0, uv1, uv2},
                w0,
                normal_matrix
            );
            t0 = g0[0];
            bt0 = g0[1];

            auto g1 = tangent_bitangent(
                {ms[1].xyz(), ms[2].xyz(), ms[0].xyz()},
                {uv1, uv2, uv0},
                w1,
                normal_matrix
            );
            t1 = g1[0];
            bt1 = g1[1];

            auto g2 = tangent_bitangent(
                {ms[2].xyz(), ms[0].xyz(), ms[1].xyz()},
                {uv2, uv0, uv1},
                w2,
                normal_matrix
            );
            t2 = g2[0];
            bt2 = g2[1];
        }

        uv0 = uv0 / w0;
        uv1 = uv1 / w1;
        uv2 = uv2 / w2;


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
               {ss0, ss1, ss2},
                {ws[0].xyz()/w0, ws[1].xyz()/w1, ws[2].xyz()/w2},
                {n0, n1, n2},
                {t0, t1, t2},
            {bt0, bt1, bt2},
            {uv0, uv1, uv2},
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
               {ss0, ss1, ss2},
                {ws[0].xyz()/w0, ws[1].xyz()/w1, ws[2].xyz()/w2},
               {n0, n1, n2},
               {t0, t1, t2},
               {bt0, bt1, bt2},
               {uv0, uv1, uv2},
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

    INLINE static Vector4<f32> perspective(ref<Vector4<f32>> cs) {
        return {
            cs.x() / cs.w(),
            cs.y() / cs.w(),
            cs.z() / cs.w(),
            cs.w(),
       };
    }

    INLINE static Vector3<f32> screen_space(ref<Vector4<f32>> ps, ref<Vector2<f32>> screen) {
        return {
            (ps.x()+0.5f) * screen.x(),
            (-ps.y()+0.5f) * screen.y(),
            ps.z(),
       };
    }

    INLINE static std::array<Vector3<f32>, 2> tangent_bitangent(
        ref<std::array<Vector3<f32>, 3>> ws,
        ref<std::array<Vector3<f32>, 3>> uv,
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

    INLINE static void draw_triangle_filled_textured(
            ref_mut<FrameBuffer> frame,
            std::array<Vector3<f32>, 3> ss,
            std::array<Vector3<f32>, 3> ws,
            std::array<Vector3<f32>, 3> n,
            std::array<Vector3<f32>, 3> t,
            std::array<Vector3<f32>, 3> bt,
            std::array<Vector3<f32>, 3> uv,
            Vector3<f32> ambient,
            Vector3<f32> specular,
            u32 shininess,
            TextureId ambient_map,
            ref<Texture> diffuse_map,
            TextureId specular_map,
            TextureId normal_map
        ) {
        rasterize_triangle(frame, ss, [&](auto pix, auto w0, auto w1, auto w2) {

            if (pix.x() < 0 || pix.x() > frame.width() || pix.y() < 0 || pix.y() > frame.height()) return;
            auto depth = w0 * ss[0].z() + w1 * ss[1].z() + w2 * ss[2].z();
            if (depth > 1 || depth < 0) return;

            auto pix_uv = uv[0] * w0 + uv[1] * w1 + uv[2] * w2;
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
            pixel.normal = (n[0] * w0 + n[1] * w1 + n[2] * w2)/frac_1_w;
            pixel.tangent = (t[0] * w0 + t[1] * w1 + t[2] * w2)/frac_1_w;
            pixel.bitangent = (bt[0] * w0 + bt[1] * w1 + bt[2] * w2)/frac_1_w;
            pixel.position = (ws[0] * w0 + ws[1] * w1 + ws[2] * w2)/frac_1_w;
            pixel.normal_map = normal_map;
            pixel.depth = convert_depth(depth);
            frame[pix].set_smaller_depth(pixel);
        });
    }


    INLINE static void draw_triangle_filled_textured_deferred(
            ref_mut<FrameBuffer> frame,
            std::array<Vector3<f32>, 3> ss,
            std::array<Vector3<f32>, 3> ws,
            std::array<Vector3<f32>, 3> n,
            std::array<Vector3<f32>, 3> t,
            std::array<Vector3<f32>, 3> bt,
            std::array<Vector3<f32>, 3> uv,
            Vector3<f32> ambient,
            Vector3<f32> diffuse,
            Vector3<f32> specular,
            u32 shininess,
            TextureId ambient_map,
            TextureId diffuse_map,
            TextureId specular_map,
            TextureId normal_map
        ) {

        rasterize_triangle(frame, ss, [&](auto pix, auto w0, auto w1, auto w2) {

            if (pix.x() < 0 || pix.x() > frame.width() || pix.y() < 0 || pix.y() > frame.height()) return;
            auto depth = w0 * ss[0].z() + w1 * ss[1].z() + w2 * ss[2].z();
            if (depth > 1 || depth < 0) return;

            auto pix_uv = uv[0] * w0 + uv[1] * w1 + uv[2] * w2;
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
            pixel.normal = ((n[0] * w0 + n[1] * w1 + n[2] * w2)/frac_1_w).xyz();
            pixel.tangent = ((t[0] * w0 + t[1] * w1 + t[2] * w2)/frac_1_w).xyz();
            pixel.bitangent = ((bt[0] * w0 + bt[1] * w1 + bt[2] * w2)/frac_1_w).xyz();
            pixel.position = (ws[0] * w0 + ws[1] * w1 + ws[2] * w2)/frac_1_w;
            pixel.normal_map = normal_map;
            pixel.depth = convert_depth(depth);
            frame[pix].set_smaller_depth(pixel);
        });
    }


    INLINE static u32 convert_depth(f32 depth) {
        return static_cast<u32>(depth * static_cast<f32>(0xFFFFFFFEull));
    }

    template<typename Func>
    INLINE static void rasterize_triangle(
            ref_mut<FrameBuffer> frame,
            std::array<Vector3<f32>, 3> ss,
            Func&& func
        ) {
        auto min_x = std::max(std::min({ss[0].x(), ss[1].x(), ss[2].x()}), 0.f);
        auto max_x = std::min(std::max({ss[0].x(), ss[1].x(), ss[2].x()}), static_cast<f32>(frame.width())-1);
        auto min_y = std::max(std::min({ss[0].y(), ss[1].y(), ss[2].y()}), 0.f);
        auto max_y = std::min(std::max({ss[0].y(), ss[1].y(), ss[2].y()}), static_cast<f32>(frame.height())-1);


        auto denom = (ss[1].y() - ss[2].y()) * (ss[0].x() - ss[2].x()) + (ss[2].x() - ss[1].x()) * (ss[0].y() - ss[2].y());

        if (denom == 0.0f) {
            return;
        }


        for (auto y = static_cast<isize>(min_y); y <= static_cast<isize>(max_y); y++) {
            for (auto x = static_cast<isize>(min_x); x <= static_cast<isize>(max_x); x++) {
                Vector2<f32> pf{static_cast<f32>(x), static_cast<f32>(y)};

                auto w0 = ((ss[1].y() - ss[2].y()) * (pf.x() - ss[2].x()) + (ss[2].x() - ss[1].x()) * (pf.y() - ss[2].y())) / denom;
                auto w1 = ((ss[2].y() - ss[0].y()) * (pf.x() - ss[2].x()) + (ss[0].x() - ss[2].x()) * (pf.y() - ss[2].y())) / denom;
                auto w2 = 1.0f - w0 - w1;

                if (w0 >= 0.0 && w1 >= 0.0 && w2 >= 0.0) {
                    Vector2<usize> pix{static_cast<usize>(x), static_cast<usize>(y)};
                    func(pix, w0, w1, w2);
                }
            }
        }
    }
};

#endif //RENDERER_H
