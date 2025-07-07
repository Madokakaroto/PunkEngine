#pragma once

#include "Base/Types.h"
#include "RHI/RHIDefinitions.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace punk {
namespace rhi {

// 前向声明
class buffer;
class texture;
class shader;
class render_pass;
class pipeline_state;
class command_list;
class command_context;
class viewport;
class uniform_buffer;
class vertex_declaration;
class sampler_state;
class render_target_view;
class depth_stencil_view;
class shader_resource_view;
class unordered_access_view;
class compute_shader;
class fence;

// 前置结构体定义 (详细定义在 RHIDefinitions.h 中)
struct resource_create_info;
struct texture_create_desc;
struct buffer_create_desc;
struct render_pass_info;

// RHI 核心接口
class dynamic_rhi {
public:
    virtual ~dynamic_rhi() = default;

    // 初始化和销毁
    virtual bool init() = 0;
    virtual void post_init() = 0;
    virtual void shutdown() = 0;

    // 设备信息
    virtual backend get_backend() const = 0;
    virtual feature_level get_feature_level() const = 0;
    virtual std::string get_adapter_name() const = 0;
    virtual uint64 get_available_gpu_memory_size() const = 0;

    // 命令上下文
    virtual command_context* get_default_context() = 0;
    virtual command_context* get_default_async_compute_context() = 0;

    // 资源创建 - 缓冲区
    virtual buffer* create_buffer(const buffer_create_desc& create_desc, const resource_create_info& create_info) = 0;
    virtual void* lock_buffer(command_list* cmd_list, buffer* buffer, uint32 offset, uint32 size, bool is_write_only) = 0;
    virtual void unlock_buffer(command_list* cmd_list, buffer* buffer) = 0;
    virtual void copy_buffer(buffer* source_buffer, buffer* dest_buffer) = 0;

    // 资源创建 - 纹理
    virtual texture* create_texture(const texture_create_desc& create_desc, const resource_create_info& create_info) = 0;
    virtual texture* create_aliased_texture(texture* source_texture) = 0;
    virtual void* lock_texture_2d(command_list* cmd_list, texture* texture, uint32 mip_index, bool is_write_only, uint32& dest_stride, bool lock_within_miptail = false) = 0;
    virtual void unlock_texture_2d(command_list* cmd_list, texture* texture, uint32 mip_index, bool lock_within_miptail = false) = 0;

    // 着色器
    virtual shader* create_shader(shader_frequency frequency, const std::vector<uint8>& code, const std::string& debug_name) = 0;
    virtual compute_shader* create_compute_shader(const std::vector<uint8>& code, const std::string& debug_name) = 0;

    // 视图创建
    virtual shader_resource_view* create_shader_resource_view(buffer* buffer, uint32 stride, uint8 format) = 0;
    virtual shader_resource_view* create_shader_resource_view(texture* texture, uint8 mip_level = 0, uint8 num_mip_levels = 1, uint8 format = 255, uint32 first_array_slice = 0, uint32 num_array_slices = 1) = 0;
    virtual unordered_access_view* create_unordered_access_view(buffer* buffer, uint8 format) = 0;
    virtual unordered_access_view* create_unordered_access_view(texture* texture, uint32 mip_level = 0, uint8 format = 255, uint32 first_array_slice = 0, uint32 num_array_slices = 1) = 0;
    virtual render_target_view* create_render_target_view(texture* texture, uint32 mip_index = 0, uint32 array_slice_index = 0) = 0;
    virtual depth_stencil_view* create_depth_stencil_view(texture* texture, bool is_read_only = false) = 0;

    // Pipeline State
    virtual pipeline_state* create_graphics_pipeline_state(const struct graphics_pipeline_state_initializer& initializer) = 0;
    virtual pipeline_state* create_compute_pipeline_state(compute_shader* compute_shader) = 0;

    // 同步
    virtual fence* create_fence(const std::string& name) = 0;
    virtual void signal_fence(fence* fence, uint64 value) = 0;
    virtual void wait_for_fence(fence* fence, uint64 value, uint32 timeout_ms = 0xFFFFFFFF) = 0;

    // 查询
    virtual bool get_query_result(class render_query* render_query, uint64& out_result, bool wait) = 0;

    // 实用函数
    virtual void update_texture_2d(command_list* cmd_list, texture* texture, uint32 mip_index, const struct update_texture_region_2d& update_region, uint32 source_pitch, const uint8* source_data) = 0;
    virtual void copy_texture(command_list* cmd_list, texture* source_texture, texture* dest_texture, const struct copy_texture_info& copy_info) = 0;

    // 统计信息
    virtual void get_texture_memory_stats(struct texture_memory_stats& out_stats) = 0;
    virtual bool get_texture_memory_visualize_data(struct texture_memory_visualize_data& out_data) = 0;

    // 静态工厂方法
    static std::unique_ptr<dynamic_rhi> create(backend backend);
};

// 命令列表接口
class command_list {
public:
    virtual ~command_list() = default;

    // 命令列表状态
    virtual void begin() = 0;
    virtual void end() = 0;
    virtual void reset() = 0;

    // 渲染通道
    virtual void begin_render_pass(const render_pass_info& info, const std::string& name) = 0;
    virtual void end_render_pass() = 0;
    virtual void next_subpass() = 0;

    // Pipeline 绑定
    virtual void set_graphics_pipeline_state(pipeline_state* graphics_pipeline_state, uint32 stencil_ref = 0) = 0;
    virtual void set_compute_pipeline_state(pipeline_state* compute_pipeline_state) = 0;

    // 资源绑定
    virtual void set_shader_texture(shader_frequency shader_freq, uint32 texture_index, texture* new_texture) = 0;
    virtual void set_shader_sampler(shader_frequency shader_freq, uint32 sampler_index, sampler_state* new_sampler) = 0;
    virtual void set_shader_resource_view_parameter(shader_frequency shader_freq, uint32 sampler_index, shader_resource_view* srv) = 0;
    virtual void set_uav_parameter(shader_frequency shader_freq, uint32 uav_index, unordered_access_view* uav) = 0;
    virtual void set_shader_uniform_buffer(shader_frequency shader_freq, uint32 buffer_index, uniform_buffer* buffer) = 0;

    // 顶点数据
    virtual void set_stream_source(uint32 stream_index, buffer* vertex_buffer, uint32 stride, uint32 offset) = 0;
    virtual void set_vertex_declaration(vertex_declaration* vertex_declaration) = 0;

    // 绘制命令
    virtual void draw_primitive(uint32 base_vertex_index, uint32 num_primitives, uint32 num_instances) = 0;
    virtual void draw_indexed_primitive(buffer* index_buffer, uint32 base_vertex_index, uint32 first_instance, uint32 num_vertices, uint32 start_index, uint32 num_primitives, uint32 num_instances) = 0;
    virtual void draw_primitive_indirect(buffer* argument_buffer, uint32 argument_offset) = 0;
    virtual void draw_indexed_indirect(buffer* index_buffer, buffer* argument_buffer, uint32 argument_offset) = 0;

    // 计算着色器
    virtual void dispatch_compute_shader(uint32 thread_group_count_x, uint32 thread_group_count_y, uint32 thread_group_count_z) = 0;
    virtual void dispatch_indirect_compute_shader(buffer* argument_buffer, uint32 argument_offset) = 0;

    // 资源状态转换
    virtual void transition_resource(texture* texture, resource_state before_state, resource_state after_state) = 0;
    virtual void transition_resource(buffer* buffer, resource_state before_state, resource_state after_state) = 0;

    // 复制操作
    virtual void copy_to_resolve_target(texture* source_texture, texture* dest_texture, const struct resolve_params& resolve_params) = 0;
    virtual void copy_texture(texture* source_texture, texture* dest_texture, const struct copy_texture_info& copy_info) = 0;

    // 查询
    virtual void begin_occlusion_query_batch(uint32 num_queries_in_batch) = 0;
    virtual void end_occlusion_query_batch() = 0;
    virtual void begin_render_query(class render_query* render_query) = 0;
    virtual void end_render_query(class render_query* render_query) = 0;

    // 调试
    virtual void push_event(const std::string& name, uint32 color) = 0;
    virtual void pop_event() = 0;
    virtual void set_marker(const std::string& name) = 0;

    // 提交
    virtual void submit_commands_hint() = 0;
};

// 命令上下文接口
class command_context {
public:
    virtual ~command_context() = default;
    virtual command_list& get_command_list() = 0;
    virtual void begin_frame() = 0;
    virtual void end_frame() = 0;
    virtual void begin_scene() = 0;
    virtual void end_scene() = 0;
};

// 资源基类
class resource {
public:
    virtual ~resource() = default;
    virtual uint32 get_ref_count() const = 0;
    virtual uint32 add_ref() const = 0;
    virtual uint32 release() const = 0;
    virtual void set_debug_name(const std::string& name) = 0;
    virtual std::string get_debug_name() const = 0;
};

class buffer : public resource {
public:
    virtual uint32 get_size() const = 0;
    virtual buffer_usage_flags get_usage() const = 0;
    virtual uint32 get_stride() const = 0;
};

class texture : public resource {
public:
    virtual uint32 get_size_x() const = 0;
    virtual uint32 get_size_y() const = 0;
    virtual uint32 get_size_z() const = 0;
    virtual pixel_format get_format() const = 0;
    virtual uint32 get_num_mips() const = 0;
    virtual uint32 get_num_samples() const = 0;
    virtual texture_create_flags get_flags() const = 0;
    virtual void* get_native_resource() const = 0;
    virtual void* get_native_shader_resource_view() const = 0;
    virtual void get_write_mask_properties(void*& out_data, uint32& out_size) = 0;
};

// 着色器相关
class shader : public resource {
public:
    virtual shader_frequency get_frequency() const = 0;
    virtual const std::vector<uint8>& get_code() const = 0;
};

class vertex_shader : public shader {};
class pixel_shader : public shader {};
class geometry_shader : public shader {};  
class compute_shader : public shader {};
class hull_shader : public shader {};
class domain_shader : public shader {};

// 视图相关
class shader_resource_view : public resource {};
class unordered_access_view : public resource {};
class render_target_view : public resource {};
class depth_stencil_view : public resource {};

// 其他资源
class sampler_state : public resource {};
class uniform_buffer : public resource {
public:
    virtual const void* get_contents() const = 0;
};
class vertex_declaration : public resource {};
class pipeline_state : public resource {};
class fence : public resource {};

// 全局RHI访问
extern std::unique_ptr<dynamic_rhi> g_dynamic_rhi;
extern command_list* g_command_list;

// 辅助宏
#define ENQUEUE_RENDER_COMMAND(TypeName) \
    struct TypeName { \
        static void do_task(command_list& cmd_list)

} // namespace rhi
} // namespace punk 