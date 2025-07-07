#pragma once

#include "Base/Types.h"
#include <vector>

namespace punk::rhi {

// forwards
class texture;
class vertex_shader;
class pixel_shader;  
class geometry_shader;
class hull_shader;
class domain_shader;
class vertex_declaration;
class blend_state;
class rasterizer_state;
class depth_stencil_state;

// RHI enums
enum class feature_level {
    es2 = 0,
    es3_1,
    sm4,
    sm5,
    sm6
};

enum class backend {
    none = 0,
    opengl,
    directx11,
    directx12,
    vulkan,
    metal
};

enum class buffer_usage_flags : uint32 {
    none = 0,
    static_usage = 1 << 0,
    dynamic = 1 << 1,
    volatile_usage = 1 << 2,
    unordered_access = 1 << 3,
    byte_address_buffer = 1 << 4,
    source_copy = 1 << 5,
    structured_buffer = 1 << 6,
    draw_indirect = 1 << 7,
    shader_resource = 1 << 8,
    keep_cpu_accessible = 1 << 9,
    fast_vram = 1 << 10,
    transient = 1 << 11
};

enum class pixel_format : uint8 {
    unknown = 0,
    a32b32g32r32f,
    b8g8r8a8,
    g8,
    g16,
    dxt1,
    dxt3,
    dxt5,
    uyvy,
    float_rgb,
    float_rgba,
    depth_stencil,
    shadow_depth,
    r32_float,
    g16r16,
    g16r16f,
    g32r32f,
    a2b10g10r10,
    a16b16g16r16,
    d24,
    r16f,
    r16f_filter,
    bc5,
    v8u8,
    a1,
    float_r11g11b10,
    a8,
    r32_uint,
    r32_sint,
    pvrtc2,
    pvrtc4,
    r16_uint,
    r16_sint,
    r16g16b16a16_uint,
    r16g16b16a16_sint,
    r5g6b5_unorm,
    r8g8b8a8,
    a8r8g8b8,
    bc4,
    r8g8,
    atc_rgb,
    atc_rgba_e,
    atc_rgba_i,
    x24_g8,
    etc1,
    etc2_rgb,
    etc2_rgba,
    r32g32b32a32_uint,
    r16g16_uint,
    astc_4x4,
    astc_6x6,
    astc_8x8,
    astc_10x10,
    astc_12x12,
    bc6h,
    bc7,
    r8_uint,
    l8,
    xgxr8,
    r8g8b8a8_uint,
    r8g8b8a8_snorm,
    r16g16b16a16_unorm,
    r16g16b16a16_snorm,
    r32g32_uint,
    r32g32_sint,
    r32g32b32_uint,
    r32g32b32_sint,
    r32g32b32f,
    max
};

enum class shader_frequency : uint8 {
    vertex = 0,
    hull,
    domain,
    pixel,
    geometry,
    compute,
    ray_gen,
    ray_miss,
    ray_hit_group,
    ray_callable,
    amplification,
    mesh,
    num_frequencies
};

enum class primitive_type : uint8 {
    triangles = 0,
    triangle_strip,
    triangle_fan,
    lines,
    line_strip,
    points,
    patches,
    triangle_list = triangles
};

enum class resource_state : uint32 {
    unknown = 0,
    common = 1 << 0,
    vertex_and_constant_buffer = 1 << 1,
    index_buffer = 1 << 2,
    render_target = 1 << 3,
    unordered_access = 1 << 4,
    depth_write = 1 << 5,
    depth_read = 1 << 6,
    non_pixel_shader_resource = 1 << 7,
    pixel_shader_resource = 1 << 8,
    stream_out = 1 << 9,
    indirect_argument = 1 << 10,
    copy_dest = 1 << 11,
    copy_source = 1 << 12,
    resolve_dest = 1 << 13,
    resolve_source = 1 << 14,
    generic_read = (1 << 15) - 1,
    present = 1 << 15,
    predication = 1 << 16,
    shading_rate_source = 1 << 17
};

enum class clear_binding {
    none_bound = 0,
    color_bound = 1,
    depth_stencil_bound = 2
};

enum class texture_create_flags : uint32 {
    none = 0,
    render_targetable = 1 << 0,
    resolve_targetable = 1 << 1,
    depth_stencil_targetable = 1 << 2,
    shader_resource = 1 << 3,
    srgb = 1 << 4,
    cpu_writable = 1 << 5,
    no_tiling = 1 << 6,
    fast_vram = 1 << 7,
    hide_in_visualize_texture = 1 << 8,
    create_texture_3d = 1 << 9,
    uav = 1 << 10,
    presentable = 1 << 11,
    cpu_readback = 1 << 12,
    offline_processed = 1 << 13,
    fast_vram_partial_alloc = 1 << 14,
    disable_dcc = 1 << 15,
    tc_compatible = 1 << 16,
    reduce_memory_with_tiling_mode = 1 << 17,
    virtual_texture = 1 << 18,
    target_array_slices_independently = 1 << 19,
    shared = 1 << 20,
    generate_mip_capable = 1 << 21,
    transient = 1 << 22,
    foveation = 1 << 23,
    not_offline_processed = 1 << 24
};

enum class render_target_load_action : uint8 {
    no_action = 0,
    load,
    clear
};

enum class render_target_store_action : uint8 {
    no_action = 0,
    store,
    multisample_resolve
};

enum class depth_stencil_target_load_action : uint8 {
    no_action = 0,
    load,
    clear
};

enum class depth_stencil_target_store_action : uint8 {
    no_action = 0,
    store
};


struct resource_create_info {
    void* bulk_data = nullptr;
    class resource_bulk_data_interface* resource_bulk_data_interface = nullptr;
    const char* debug_name = nullptr;
    resource_state initial_state = resource_state::common;
    uint32 ext_data = 0;
    uint32 gpu_mask = 1;
};

struct texture_create_desc {
    texture_create_flags flags = texture_create_flags::none;
    pixel_format format = pixel_format::unknown;
    clear_binding clear_value{};
    uint32 extent_3d[3] = {1, 1, 1}; // Width, Height, Depth
    uint16 dimension = 2; // 1D, 2D, 3D, Cube
    uint16 array_size = 1;
    uint8 num_mips = 1;
    uint8 num_samples = 1;
    uint32 gpu_mask = 1;
    const char* debug_name = nullptr;
    resource_state initial_state = resource_state::common;
    
    // Helper functions
    static texture_create_desc create_2d(uint32 width, uint32 height, pixel_format format) {
        texture_create_desc desc;
        desc.extent_3d[0] = width;
        desc.extent_3d[1] = height;
        desc.format = format;
        return desc;
    }
};

struct buffer_create_desc {
    uint32 size = 0;
    uint32 stride = 0;
    buffer_usage_flags usage = buffer_usage_flags::none;
    const char* debug_name = nullptr;
    uint32 gpu_mask = 1;
    resource_state initial_state = resource_state::common;
    
    static buffer_create_desc create_vertex_buffer(uint32 size) {
        buffer_create_desc desc;
        desc.size = size;
        desc.usage = buffer_usage_flags::static_usage;
        desc.initial_state = resource_state::vertex_and_constant_buffer;
        return desc;
    }
    
    static buffer_create_desc create_index_buffer(uint32 size) {
        buffer_create_desc desc;
        desc.size = size;
        desc.usage = buffer_usage_flags::static_usage;
        desc.initial_state = resource_state::index_buffer;
        return desc;
    }
    
    static buffer_create_desc create_uniform_buffer(uint32 size) {
        buffer_create_desc desc;
        desc.size = size;
        desc.usage = buffer_usage_flags::dynamic;
        desc.initial_state = resource_state::vertex_and_constant_buffer;
        return desc;
    }
};

struct render_pass_info {
    struct color_entry {
        class render_target_view* render_target = nullptr;
        class render_target_view* resolve_target = nullptr;
        render_target_load_action load_action = render_target_load_action::no_action;
        render_target_store_action store_action = render_target_store_action::no_action;
        float clear_color[4] = {0, 0, 0, 0};
    } color_render_targets[8];
    
    struct depth_stencil_entry {
        class depth_stencil_view* depth_stencil_target = nullptr;
        depth_stencil_target_load_action depth_load_action = depth_stencil_target_load_action::no_action;
        depth_stencil_target_store_action depth_store_action = depth_stencil_target_store_action::no_action;
        depth_stencil_target_load_action stencil_load_action = depth_stencil_target_load_action::no_action;
        depth_stencil_target_store_action stencil_store_action = depth_stencil_target_store_action::no_action;
        float clear_depth = 1.0f;
        uint32 clear_stencil = 0;
        bool depth_read_only = false;
        bool stencil_read_only = false;
    } depth_stencil_render_target;
    
    uint32 num_color_render_targets = 0;
    bool occlusion_queries = false;
    bool generating_mips = false;
    uint8 multi_view_count = 0;
    bool too_many_uavs = false;
    bool is_msaa = false;
};

// 着色器参数结构
struct shader_parameter {
    uint32 buffer_index = 0;
    uint32 base_index = 0;
    uint32 size = 0;
    shader_frequency shader_freq = shader_frequency::vertex;
    std::string name;
};

// 顶点元素描述
struct vertex_element {
    uint8 stream_index = 0;
    uint8 offset = 0;
    uint8 type = 0; // 对应具体的顶点格式枚举
    uint8 attribute_index = 0;
    uint16 stride = 0;
    bool use_instance_index = false;
    uint32 instance_step_rate = 0;
};

// 图形管线状态初始化器
struct graphics_pipeline_state_initializer {
    vertex_declaration* bound_shader_state = nullptr;
    vertex_shader* vertex_shader = nullptr;
    pixel_shader* pixel_shader = nullptr;
    geometry_shader* geometry_shader = nullptr;
    hull_shader* hull_shader = nullptr;
    domain_shader* domain_shader = nullptr;
    
    blend_state* blend_state = nullptr;
    rasterizer_state* rasterizer_state = nullptr;
    depth_stencil_state* depth_stencil_state = nullptr;
    
    primitive_type primitive_type_val = primitive_type::triangles;
    uint32 num_render_targets = 1;
    pixel_format render_target_formats[8] = {pixel_format::b8g8r8a8};
    uint32 render_target_flags[8] = {0};
    pixel_format depth_stencil_target_format = pixel_format::depth_stencil;
    uint32 depth_stencil_target_flag = 0;
    uint16 num_samples = 1;
    uint32 subpass_hint = 0;
    uint32 subpass_index = 0;
    bool depth_bounds = false;
    uint8 multi_view_count = 0;
    bool has_fragment_density_attachment = false;
};

// 混合状态描述
struct blend_state_initializer_render_target {
    bool blend_enable = false;
    uint8 color_blend_op = 0;  // Add, Subtract, etc.
    uint8 color_src_blend = 0; // One, Zero, SrcAlpha, etc.
    uint8 color_dest_blend = 0;
    uint8 alpha_blend_op = 0;
    uint8 alpha_src_blend = 0;
    uint8 alpha_dest_blend = 0;
    uint8 color_write_mask = 0x0F; // RGBA
};

struct blend_state_initializer {
    bool use_independent_render_target_blend_states = false;
    blend_state_initializer_render_target render_targets[8];
    bool use_alpha_to_coverage = false;
};

// 光栅化状态描述
struct rasterizer_state_initializer {
    uint8 fill_mode = 0; // Solid, Wireframe
    uint8 cull_mode = 0; // None, Front, Back
    float depth_bias = 0.0f;
    float slope_scale_depth_bias = 0.0f;
    bool allow_msaa = true;
    bool enable_line_aa = false;
};

// 深度模板状态描述
struct depth_stencil_state_initializer {
    bool enable_depth_write = true;
    uint8 depth_test = 0; // Never, Less, Equal, etc.
    bool enable_front_face_stencil = false;
    uint8 front_face_stencil_test = 0;
    uint8 front_face_stencil_fail_stencil_op = 0;
    uint8 front_face_depth_fail_stencil_op = 0;
    uint8 front_face_pass_stencil_op = 0;
    bool enable_back_face_stencil = false;
    uint8 back_face_stencil_test = 0;
    uint8 back_face_stencil_fail_stencil_op = 0;
    uint8 back_face_depth_fail_stencil_op = 0;
    uint8 back_face_pass_stencil_op = 0;
    uint8 stencil_read_mask = 0xFF;
    uint8 stencil_write_mask = 0xFF;
};

// 采样器状态描述
struct sampler_state_initializer {
    uint8 filter = 0; // Point, Linear, Anisotropic
    uint8 address_u = 0; // Wrap, Mirror, Clamp, Border
    uint8 address_v = 0;
    uint8 address_w = 0;
    float mip_bias = 0.0f;
    uint32 max_anisotropy = 1;
    uint32 border_color = 0;
    float min_mip_level = 0.0f;
    float max_mip_level = 3.402823466e+38f; // FLT_MAX
    uint8 comparison_func = 0; // Never, Less, Equal, etc.
};

// 纹理更新区域
struct update_texture_region_2d {
    uint32 dest_x = 0;
    uint32 dest_y = 0;
    int32 src_x = 0;
    int32 src_y = 0;
    uint32 width = 0;
    uint32 height = 0;
};

struct update_texture_region_3d {
    uint32 dest_x = 0;
    uint32 dest_y = 0;
    uint32 dest_z = 0;
    int32 src_x = 0;
    int32 src_y = 0;
    int32 src_z = 0;
    uint32 width = 0;
    uint32 height = 0;
    uint32 depth = 0;
};

// 纹理复制信息
struct copy_texture_info {
    uint32 size[3] = {0, 0, 0}; // Width, Height, Depth
    uint32 source_position[3] = {0, 0, 0};
    uint32 dest_position[3] = {0, 0, 0};
    uint32 source_slice_index = 0;
    uint32 dest_slice_index = 0;
    uint32 num_slices = 1;
    uint32 source_mip_index = 0;
    uint32 dest_mip_index = 0;
    uint32 num_mips = 1;
};

// 解析参数
struct resolve_params {
    resolve_params() = default;
    resolve_params(const struct resolve_rect& in_source_rect, const struct resolve_rect& in_dest_rect)
        : source_rect(in_source_rect), dest_rect(in_dest_rect) {}
        
    struct resolve_rect source_rect;
    struct resolve_rect dest_rect;
    uint32 mip_index = 0;
    uint32 source_array_index = 0;
    uint32 dest_array_index = 0;
    uint8 resolve_type = 0; // Average, Min, Max
};

struct resolve_rect {
    int32 x1 = 0;
    int32 y1 = 0;
    int32 x2 = 0;
    int32 y2 = 0;
    
    resolve_rect() = default;
    resolve_rect(int32 in_x1, int32 in_y1, int32 in_x2, int32 in_y2)
        : x1(in_x1), y1(in_y1), x2(in_x2), y2(in_y2) {}
};

// GPU统计信息
struct texture_memory_stats {
    int64 dedicated_video_memory = 0;
    int64 dedicated_system_memory = 0;
    int64 shared_system_memory = 0;
    int64 allocated_memory_size = 0;
    int64 largest_contiguous_allocation = 0;
    int64 texture_pool_size = 0;
    int64 pending_memory_adjustment = 0;
    uint32 memory_pressure = 0;
    bool is_amd_pre_gcn_architecture = false;
};

struct texture_memory_visualize_data {
    struct texture_entry {
        std::string name;
        uint32 size_x = 0;
        uint32 size_y = 0;
        pixel_format format = pixel_format::unknown;
        uint32 size = 0;
        bool is_streaming = false;
        bool is_render_target = false;
    };
    std::vector<texture_entry> textures;
    uint32 total_size = 0;
};

// 资源批量数据接口
class resource_bulk_data_interface {
public:
    virtual ~resource_bulk_data_interface() = default;
    virtual const void* get_resource_bulk_data() const = 0;
    virtual uint32 get_resource_bulk_data_size() const = 0;
    virtual void discard() = 0;
};

// 查询相关
class RHIRenderQuery {
public:
    virtual ~RHIRenderQuery() = default;
    virtual void Begin() = 0;
    virtual void End() = 0;
    virtual bool GetResult(uint64& OutResult, bool bWait) = 0;
};

// 状态对象基类
class RHIBlendState {
public:
    virtual ~RHIBlendState() = default;
};

class RHIRasterizerState {
public:
    virtual ~RHIRasterizerState() = default;
};

class RHIDepthStencilState {
public:
    virtual ~RHIDepthStencilState() = default;
};

// 操作符重载用于标志枚举
inline buffer_usage_flags operator|(buffer_usage_flags a, buffer_usage_flags b) {
    return static_cast<buffer_usage_flags>(static_cast<uint32>(a) | static_cast<uint32>(b));
}

inline buffer_usage_flags operator&(buffer_usage_flags a, buffer_usage_flags b) {
    return static_cast<buffer_usage_flags>(static_cast<uint32>(a) & static_cast<uint32>(b));
}

inline texture_create_flags operator|(texture_create_flags a, texture_create_flags b) {
    return static_cast<texture_create_flags>(static_cast<uint32>(a) | static_cast<uint32>(b));
}

inline texture_create_flags operator&(texture_create_flags a, texture_create_flags b) {
    return static_cast<texture_create_flags>(static_cast<uint32>(a) & static_cast<uint32>(b));
}

inline resource_state operator|(resource_state a, resource_state b) {
    return static_cast<resource_state>(static_cast<uint32>(a) | static_cast<uint32>(b));
}

inline resource_state operator&(resource_state a, resource_state b) {
    return static_cast<resource_state>(static_cast<uint32>(a) & static_cast<uint32>(b));
}

} // namespace rhi
} // namespace punk 