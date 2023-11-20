//
// Copyright (C) YuqiaoZhang(HanetakaChou)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef _BRX_D3D12_DEVICE_H_
#define _BRX_D3D12_DEVICE_H_ 1

#include "../include/brx_device.h"
#include "brx_vector.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 1
#include <sdkddkver.h>
#include <windows.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#define D3D12MA_D3D12_HEADERS_ALREADY_INCLUDED 1
#include "../thirdparty/D3D12MemoryAllocator/include/D3D12MemAlloc.h"
#include "brx_d3d12_descriptor_allocator.h"

class brx_d3d12_device : public brx_device
{
	HINSTANCE m_dynamic_library_dxgi;

	HINSTANCE m_dynamic_library_d3d12;

	decltype(D3D12SerializeRootSignature) *m_pfn_d3d12_serialize_root_signature;

	bool m_support_ray_tracing;

	IDXGIFactory2 *m_factory;
	IDXGIAdapter *m_adapter;

	ID3D12Device5 *m_device;

	bool m_uma;
	bool m_cache_coherent_uma;

	ID3D12CommandQueue *m_graphics_queue;
	ID3D12CommandQueue *m_upload_queue;

	D3D12MA::Allocator *m_memory_allocator;
	D3D12MA::Pool *m_uniform_upload_buffer_memory_pool;
	D3D12MA::Pool *m_staging_upload_buffer_memory_pool;
	D3D12MA::Pool *m_storage_buffer_memory_pool;
	D3D12MA::Pool *m_asset_vertex_position_buffer_memory_pool;
	D3D12MA::Pool *m_asset_vertex_varying_buffer_memory_pool;
	D3D12MA::Pool *m_asset_index_buffer_memory_pool;
	D3D12MA::Pool *m_asset_sampled_image_memory_pool;
	D3D12MA::Pool *m_storage_image_memory_pool;
	D3D12MA::Pool *m_scratch_buffer_memory_pool;
	D3D12MA::Pool *m_staging_non_compacted_bottom_level_acceleration_structure_memory_pool;
	D3D12MA::Pool *m_compacted_bottom_level_acceleration_structure_size_query_buffer_memory_pool;
	D3D12MA::Pool *m_asset_compacted_bottom_level_acceleration_structure_memory_pool;
	D3D12MA::Pool *m_top_level_acceleration_structure_instance_upload_buffer_memory_pool;
	D3D12MA::Pool *m_top_level_acceleration_structure_memory_pool;

	brx_d3d12_descriptor_allocator m_descriptor_allocator;

public:
	brx_d3d12_device();
	void init(bool support_ray_tracing);
	void uninit();
	~brx_d3d12_device();
	brx_graphics_queue *create_graphics_queue() const override;
	void destroy_graphics_queue(brx_graphics_queue *graphics_queue) const override;
	brx_upload_queue *create_upload_queue() const override;
	void destroy_upload_queue(brx_upload_queue *upload_queue) const override;
	brx_graphics_command_buffer *create_graphics_command_buffer() const override;
	void reset_graphics_command_buffer(brx_graphics_command_buffer *graphics_command_buffer) const override;
	void destroy_graphics_command_buffer(brx_graphics_command_buffer *graphics_command_buffer) const override;
	brx_upload_command_buffer *create_upload_command_buffer() const override;
	void reset_upload_command_buffer(brx_upload_command_buffer *upload_command_buffer) const override;
	void destroy_upload_command_buffer(brx_upload_command_buffer *upload_command_buffer) const override;
	brx_fence *create_fence(bool signaled) const override;
	void wait_for_fence(brx_fence *fence) const override;
	void reset_fence(brx_fence *fence) const override;
	void destroy_fence(brx_fence *fence) const override;
	brx_descriptor_set_layout *create_descriptor_set_layout(uint32_t descriptor_set_binding_count, BRX_DESCRIPTOR_SET_LAYOUT_BINDING const *descriptor_set_bindings) const override;
	void destroy_descriptor_set_layout(brx_descriptor_set_layout *descriptor_set_layout) const override;
	brx_pipeline_layout *create_pipeline_layout(uint32_t descriptor_set_layout_count, brx_descriptor_set_layout const *const *descriptor_set_layouts) const override;
	void destroy_pipeline_layout(brx_pipeline_layout *pipeline_layout) const override;
	brx_descriptor_set *create_descriptor_set(brx_descriptor_set_layout const *descriptor_set_layout) override;
	void write_descriptor_set(brx_descriptor_set *descriptor_set, BRX_DESCRIPTOR_TYPE descriptor_type, uint32_t dst_binding, uint32_t dst_array_element, uint32_t src_descriptor_count, brx_uniform_upload_buffer const *const *src_dynamic_uniform_buffers, uint32_t const *src_dynamic_uniform_buffer_ranges, brx_storage_buffer const *const *src_storage_buffers, brx_sampled_image const *const *src_sampled_images, brx_sampler const *const *src_samplers, brx_storage_image const *const *src_storage_images, brx_top_level_acceleration_structure const *const *src_top_level_acceleration_structures) const override;
	void destroy_descriptor_set(brx_descriptor_set *descriptor_set) override;
	brx_render_pass *create_render_pass(uint32_t color_attachment_count, BRX_RENDER_PASS_COLOR_ATTACHMENT const *color_attachments, BRX_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT const *depth_stencil_attachment) const override;
	void destroy_render_pass(brx_render_pass *render_pass) const override;
	brx_graphics_pipeline *create_graphics_pipeline(brx_render_pass const *render_pass, brx_pipeline_layout const *pipeline_layout, size_t vertex_shader_module_code_size, void const *vertex_shader_module_code, size_t fragment_shader_module_code_size, void const *fragment_shader_module_code, uint32_t vertex_binding_count, BRX_GRAPHICS_PIPELINE_VERTEX_BINDING const *vertex_bindings, uint32_t vertex_attribute_count, BRX_GRAPHICS_PIPELINE_VERTEX_ATTRIBUTE const *vertex_attributes, bool depth_enable, BRX_GRAPHICS_PIPELINE_COMPARE_OPERATION depth_compare_operation) const override;
	void destroy_graphics_pipeline(brx_graphics_pipeline *graphics_pipeline) const override;
	brx_compute_pipeline *create_compute_pipeline(brx_pipeline_layout const *pipeline_layout, size_t compute_shader_module_code_size, void const *compute_shader_module_code) const override;
	void destroy_compute_pipeline(brx_compute_pipeline *compute_pipeline) const override;
	brx_frame_buffer *create_frame_buffer(brx_render_pass const *render_pass, uint32_t width, uint32_t height, uint32_t color_attachment_count, brx_color_attachment_image const *const *color_attachments, brx_depth_stencil_attachment_image const *depth_stencil_attachment) const override;
	void destroy_frame_buffer(brx_frame_buffer *frame_buffer) const override;
	brx_uniform_upload_buffer *create_uniform_upload_buffer(uint32_t size) const override;
	void destroy_uniform_upload_buffer(brx_uniform_upload_buffer *uniform_upload_buffer) const override;
	uint32_t get_staging_upload_buffer_offset_alignment() const override;
	uint32_t get_staging_upload_buffer_row_pitch_alignment() const override;
	brx_staging_upload_buffer *create_staging_upload_buffer(uint32_t size) const override;
	void destroy_staging_upload_buffer(brx_staging_upload_buffer *staging_upload_buffer) const override;
	brx_intermediate_storage_buffer *create_intermediate_storage_buffer(uint32_t size, bool allow_vertex_position, bool allow_vertex_varying) const override;
	void destroy_intermediate_storage_buffer(brx_intermediate_storage_buffer *intermediate_storage_buffer) const override;
	brx_asset_vertex_position_buffer *create_asset_vertex_position_buffer(uint32_t size) const override;
	void destroy_asset_vertex_position_buffer(brx_asset_vertex_position_buffer *asset_vertex_position_buffer) const override;
	brx_asset_vertex_varying_buffer *create_asset_vertex_varying_buffer(uint32_t size) const override;
	void destroy_asset_vertex_varying_buffer(brx_asset_vertex_varying_buffer *asset_vertex_varying_buffer) const override;
	brx_asset_index_buffer *create_asset_index_buffer(uint32_t size) const override;
	void destroy_asset_index_buffer(brx_asset_index_buffer *asset_index_buffer) const override;
	brx_color_attachment_image *create_color_attachment_image(BRX_COLOR_ATTACHMENT_IMAGE_FORMAT color_attachment_image_format, uint32_t width, uint32_t height, bool allow_sampled_image) const override;
	void destroy_color_attachment_image(brx_color_attachment_image *color_attachment_image) const override;
	BRX_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT get_depth_attachment_image_format() const override;
	BRX_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT get_depth_stencil_attachment_image_format() const override;
	brx_depth_stencil_attachment_image *create_depth_stencil_attachment_image(BRX_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT depth_stencil_attachment_image_format, uint32_t width, uint32_t height, bool allow_sampled_image) const override;
	void destroy_depth_stencil_attachment_image(brx_depth_stencil_attachment_image *depth_stencil_attachment_image) const override;
	brx_storage_image *create_storage_image(BRX_STORAGE_IMAGE_FORMAT storage_image_format, uint32_t width, uint32_t height, bool allow_sampled_image) const override;
	void destroy_storage_image(brx_storage_image *storage_image) const override;
	bool is_asset_sampled_image_compression_bc_supported() const override;
	bool is_asset_sampled_image_compression_astc_supported() const override;
	brx_asset_sampled_image *create_asset_sampled_image(BRX_ASSET_IMAGE_FORMAT asset_sampled_image_format, uint32_t width, uint32_t height, uint32_t mip_levels) const override;
	void destroy_asset_sampled_image(brx_asset_sampled_image *asset_sampled_image) const override;
	brx_sampler *create_sampler(BRX_SAMPLER_FILTER filter) const override;
	void destroy_sampler(brx_sampler *sampler) const override;
	brx_surface *create_surface(void *window) const override;
	void destroy_surface(brx_surface *surface) const override;
	brx_swap_chain *create_swap_chain(brx_surface *surface) const override;
	bool acquire_next_image(brx_graphics_command_buffer *graphics_command_buffer, brx_swap_chain const *swap_chain, uint32_t *out_swap_chain_image_index) const override;
	void destroy_swap_chain(brx_swap_chain *swap_chain) const override;
	brx_scratch_buffer *create_scratch_buffer(uint32_t size) const override;
	void destroy_scratch_buffer(brx_scratch_buffer *scratch_buffer) const override;
	void get_staging_non_compacted_bottom_level_acceleration_structure_size(uint32_t bottom_level_acceleration_structure_geometry_count, BRX_BOTTOM_LEVEL_ACCELERATION_STRUCTURE_GEOMETRY const *bottom_level_acceleration_structure_geometries, uint32_t *staging_non_compacted_bottom_level_acceleration_structure_size, uint32_t *build_scratch_size) const override;
	brx_staging_non_compacted_bottom_level_acceleration_structure *create_staging_non_compacted_bottom_level_acceleration_structure(uint32_t size) const override;
	void destroy_staging_non_compacted_bottom_level_acceleration_structure(brx_staging_non_compacted_bottom_level_acceleration_structure *staging_non_compacted_bottom_level_acceleration_structure) const override;
	brx_compacted_bottom_level_acceleration_structure_size_query_pool *create_compacted_bottom_level_acceleration_structure_size_query_pool(uint32_t query_count) const override;
	uint32_t get_compacted_bottom_level_acceleration_structure_size_query_pool_result(brx_compacted_bottom_level_acceleration_structure_size_query_pool const *compacted_bottom_level_acceleration_structure_size_query_pool, uint32_t query_index) const override;
	void destroy_compacted_bottom_level_acceleration_structure_size_query_pool(brx_compacted_bottom_level_acceleration_structure_size_query_pool *compacted_bottom_level_acceleration_structure_size_query_pool) const override;
	brx_asset_compacted_bottom_level_acceleration_structure *create_asset_compacted_bottom_level_acceleration_structure(uint32_t size) const override;
	void destroy_asset_compacted_bottom_level_acceleration_structure(brx_asset_compacted_bottom_level_acceleration_structure *asset_compacted_bottom_level_acceleration_structure) const override;
	brx_top_level_acceleration_structure_instance_upload_buffer *create_top_level_acceleration_structure_instance_upload_buffer(uint32_t instance_count) const override;
	void destroy_top_level_acceleration_structure_instance_upload_buffer(brx_top_level_acceleration_structure_instance_upload_buffer *top_level_acceleration_structure_instance_upload_buffer) const override;
	void get_top_level_acceleration_structure_size(uint32_t top_level_acceleration_structure_instance_count, uint32_t *top_level_acceleration_structure_size, uint32_t *build_scratch_size, uint32_t *update_scratch_size) const override;
	brx_top_level_acceleration_structure *create_top_level_acceleration_structure(uint32_t size) const override;
	void destroy_top_level_acceleration_structure(brx_top_level_acceleration_structure *top_level_acceleration_structure) const override;
};

class brx_d3d12_graphics_queue : public brx_graphics_queue
{
	ID3D12CommandQueue *m_graphics_queue;
	bool m_uma;
	bool m_support_ray_tracing;

public:
	brx_d3d12_graphics_queue();
	void init(ID3D12CommandQueue *graphics_queue, bool uma, bool support_ray_tracing);
	void uninit(ID3D12CommandQueue *graphics_queue);
	~brx_d3d12_graphics_queue();
	void wait_and_submit(brx_upload_command_buffer const *upload_command_buffer_to_wait, brx_graphics_command_buffer const *graphics_command_buffer_to_submit, brx_fence *fence_to_signal) const override;
	bool submit_and_present(brx_graphics_command_buffer *graphics_command_buffer_to_submit, brx_swap_chain *swap_chain_to_present, uint32_t swap_chain_image_index, brx_fence *fence_to_signal) const override;
};

class brx_d3d12_upload_queue : public brx_upload_queue
{
	ID3D12CommandQueue *m_upload_queue;
	bool m_uma;
	bool m_support_ray_tracing;

public:
	brx_d3d12_upload_queue();
	void init(ID3D12CommandQueue *upload_queue, bool uma, bool support_ray_tracing);
	void uninit(ID3D12CommandQueue *upload_queue);
	~brx_d3d12_upload_queue();
	void submit_and_signal(brx_upload_command_buffer const *upload_command_buffer_to_submit_and_signal) const override;
};

class brx_d3d12_graphics_command_buffer : public brx_graphics_command_buffer
{
	bool m_uma;
	bool m_support_ray_tracing;
	ID3D12CommandAllocator *m_command_allocator;
	ID3D12GraphicsCommandList4 *m_command_list;
	brx_d3d12_descriptor_allocator *m_descriptor_allocator;
	class brx_d3d12_render_pass const *m_current_render_pass;
	class brx_d3d12_frame_buffer const *m_current_frame_buffer;
	brx_vector<uint32_t> m_current_vertex_buffer_strides;

public:
	brx_d3d12_graphics_command_buffer();
	void init(ID3D12Device *device, bool uma, bool support_ray_tracing, brx_d3d12_descriptor_allocator *descriptor_allocator);
	void uninit();
	~brx_d3d12_graphics_command_buffer();
	ID3D12CommandAllocator *get_command_allocator() const;
	ID3D12GraphicsCommandList4 *get_command_list() const;
	void begin() override;
	void acquire_asset_vertex_position_buffer(brx_asset_vertex_position_buffer *asset_vertex_position_buffer) override;
	void acquire_asset_vertex_varying_buffer(brx_asset_vertex_varying_buffer *asset_vertex_varying_buffer) override;
	void acquire_asset_index_buffer(brx_asset_index_buffer *asset_index_buffer) override;
	void acquire_asset_sampled_image(brx_asset_sampled_image *asset_sampled_image, uint32_t dst_mip_level) override;
	void acquire_asset_compacted_bottom_level_acceleration_structure(brx_asset_compacted_bottom_level_acceleration_structure *asset_compacted_bottom_level_acceleration_structure) override;
	void begin_debug_utils_label(char const *label_name) override;
	void end_debug_utils_label() override;
	void begin_render_pass(brx_render_pass const *render_pass, brx_frame_buffer const *frame_buffer, uint32_t width, uint32_t height, uint32_t color_clear_value_count, float const (*color_clear_values)[4], float const *depth_clear_value, uint8_t const *stencil_clear_value) override;
	void bind_graphics_pipeline(brx_graphics_pipeline const *graphics_pipeline) override;
	void set_view_port(uint32_t width, uint32_t height) override;
	void set_scissor(uint32_t width, uint32_t height) override;
	void bind_graphics_descriptor_sets(brx_pipeline_layout const *pipeline_layout, uint32_t descriptor_set_count, brx_descriptor_set const *const *descriptor_sets, uint32_t dynamic_offet_count, uint32_t const *dynamic_offsets) override;
	void bind_vertex_buffers(uint32_t vertex_buffer_count, brx_vertex_buffer const *const *vertex_buffers) override;
	void draw(uint32_t vertex_count, uint32_t instance_count) override;
	void draw_index(brx_index_buffer const *index_buffer, BRX_GRAPHICS_PIPELINE_INDEX_TYPE index_type, uint32_t index_count, uint32_t instance_count) override;
	void end_render_pass() override;
	void compute_pass_load_storage_image(brx_storage_image const *storage_image, BRX_STORAGE_IMAGE_LOAD_OPERATION load_operation) override;
	void bind_compute_pipeline(brx_compute_pipeline const *compute_pipeline) override;
	void bind_compute_descriptor_sets(brx_pipeline_layout const *pipeline_layout, uint32_t descriptor_set_count, brx_descriptor_set const *const *descriptor_sets, uint32_t dynamic_offet_count, uint32_t const *dynamic_offsets) override;
	void dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z) override;
	void compute_pass_store_storage_image(brx_storage_image const *storage_image, BRX_STORAGE_IMAGE_STORE_OPERATION store_operation) override;
	void build_top_level_acceleration_structure(brx_top_level_acceleration_structure *top_level_acceleration_structure, uint32_t top_level_acceleration_structure_instance_count, brx_top_level_acceleration_structure_instance_upload_buffer *top_level_acceleration_structure_instance_upload_buffer, brx_scratch_buffer *scratch_buffer) override;
	void update_top_level_acceleration_structure(brx_top_level_acceleration_structure *top_level_acceleration_structure, brx_top_level_acceleration_structure_instance_upload_buffer *top_level_acceleration_structure_instance_upload_buffer, brx_scratch_buffer *scratch_buffer) override;
	void acceleration_structure_pass_store_top_level(brx_top_level_acceleration_structure *top_level_acceleration_structure) override;
	void end() override;
};

class brx_d3d12_upload_command_buffer : public brx_upload_command_buffer
{
	bool m_uma;
	bool m_support_ray_tracing;

	ID3D12CommandAllocator *m_command_allocator;
	ID3D12GraphicsCommandList4 *m_command_list;
	ID3D12Fence *m_upload_queue_submit_fence;

public:
	brx_d3d12_upload_command_buffer();
	void init(ID3D12Device *device, bool uma, bool support_ray_tracing);
	void uninit();
	~brx_d3d12_upload_command_buffer();
	ID3D12CommandAllocator *get_command_allocator() const;
	ID3D12GraphicsCommandList4 *get_command_list() const;
	ID3D12Fence *get_upload_queue_submit_fence() const;
	void begin() override;
	void upload_from_staging_upload_buffer_to_asset_vertex_position_buffer(brx_asset_vertex_position_buffer *asset_vertex_position_buffer, uint64_t dst_offset, brx_staging_upload_buffer *staging_upload_buffer, uint64_t src_offset, uint32_t src_size) override;
	void upload_from_staging_upload_buffer_to_asset_vertex_varying_buffer(brx_asset_vertex_varying_buffer *asset_vertex_varying_buffer, uint64_t dst_offset, brx_staging_upload_buffer *staging_upload_buffer, uint64_t src_offset, uint32_t src_size) override;
	void upload_from_staging_upload_buffer_to_asset_index_buffer(brx_asset_index_buffer *asset_index_buffer, uint64_t dst_offset, brx_staging_upload_buffer *staging_upload_buffer, uint64_t src_offset, uint32_t src_size) override;
	void upload_from_staging_upload_buffer_to_asset_sampled_image(brx_asset_sampled_image *asset_sampled_image, BRX_ASSET_IMAGE_FORMAT asset_sampled_image_format, uint32_t asset_sampled_image_width, uint32_t asset_sampled_image_height, uint32_t dst_mip_level, brx_staging_upload_buffer *staging_upload_buffer, uint64_t src_offset, uint32_t src_row_pitch, uint32_t src_row_count) override;
	void build_staging_non_compacted_bottom_level_acceleration_structure(brx_staging_non_compacted_bottom_level_acceleration_structure *staging_non_compacted_bottom_level_acceleration_structure, uint32_t bottom_level_acceleration_structure_geometry_count, BRX_BOTTOM_LEVEL_ACCELERATION_STRUCTURE_GEOMETRY const *bottom_level_acceleration_structure_geometries, brx_scratch_buffer *scratch_buffer, brx_compacted_bottom_level_acceleration_structure_size_query_pool *compacted_bottom_level_acceleration_structure_size_query_pool, uint32_t query_index) override;
	void compact_bottom_level_acceleration_structure(brx_asset_compacted_bottom_level_acceleration_structure *destination_asset_compacted_bottom_level_acceleration_structure, brx_staging_non_compacted_bottom_level_acceleration_structure *source_staging_non_compacted_bottom_level_acceleration_structure) override;
	void release_asset_vertex_position_buffer(brx_asset_vertex_position_buffer *asset_vertex_position_buffer) override;
	void release_asset_vertex_varying_buffer(brx_asset_vertex_varying_buffer *asset_vertex_varying_buffer) override;
	void release_asset_index_buffer(brx_asset_index_buffer *asset_index_buffer) override;
	void release_asset_sampled_image(brx_asset_sampled_image *asset_sampled_image, uint32_t dst_mip_level) override;
	void release_asset_compacted_bottom_level_acceleration_structure(brx_asset_compacted_bottom_level_acceleration_structure *asset_compacted_bottom_level_acceleration_structure) override;
	void end() override;
};

class brx_d3d12_fence : public brx_fence
{
	ID3D12Fence *m_fence;

public:
	brx_d3d12_fence(ID3D12Fence *fence);
	ID3D12Fence *get_fence() const;
	void steal(ID3D12Fence **out_fence);
	~brx_d3d12_fence();
};

struct brx_d3d12_descriptor_layout
{
	BRX_DESCRIPTOR_TYPE root_parameter_type;
	uint32_t root_parameter_shader_register;
	uint32_t root_descriptor_table_num_descriptors;
};

class brx_d3d12_descriptor_set_layout : public brx_descriptor_set_layout
{
	brx_vector<brx_d3d12_descriptor_layout> m_descriptor_layouts;

public:
	brx_d3d12_descriptor_set_layout();
	void init(uint32_t descriptor_set_binding_count, BRX_DESCRIPTOR_SET_LAYOUT_BINDING const *wrapped_descriptor_set_bindingst);
	void uninit();
	uint32_t get_descriptor_layout_count() const;
	brx_d3d12_descriptor_layout const *get_descriptor_layouts() const;
};

class brx_d3d12_pipeline_layout : public brx_pipeline_layout
{
	ID3D12RootSignature *m_root_signature;

public:
	brx_d3d12_pipeline_layout();
	void init(ID3D12Device *device, decltype(D3D12SerializeRootSignature) *pfn_d3d12_serialize_root_signature, uint32_t descriptor_set_layout_count, brx_descriptor_set_layout const *const *descriptor_set_layouts);
	void uninit();
	~brx_d3d12_pipeline_layout();
	ID3D12RootSignature *get_root_signature() const;
};

struct brx_d3d12_descriptor
{
	BRX_DESCRIPTOR_TYPE root_parameter_type;
	uint32_t root_parameter_shader_register;
	union
	{
		struct
		{
			D3D12_GPU_VIRTUAL_ADDRESS address_base;
		} root_constant_buffer_view;
		struct
		{
			uint32_t base_descriptor_heap_index;
			uint32_t num_descriptors;
			uint32_t alloced_num_descriptors;
		} root_descriptor_table;
	};
};

class brx_d3d12_descriptor_set : public brx_descriptor_set
{
	brx_vector<brx_d3d12_descriptor> m_descriptors;

public:
	brx_d3d12_descriptor_set();
	void init(brx_d3d12_descriptor_allocator *descriptor_allocator, brx_descriptor_set_layout const *descriptor_set_layout);
	void uninit(brx_d3d12_descriptor_allocator *descriptor_allocator);
	~brx_d3d12_descriptor_set();
	void write_descriptor(ID3D12Device *device, brx_d3d12_descriptor_allocator const *descriptor_allocator, BRX_DESCRIPTOR_TYPE descriptor_type, uint32_t dst_binding, uint32_t dst_array_element, uint32_t src_descriptor_count, brx_uniform_upload_buffer const *const *src_dynamic_uniform_buffers, uint32_t const *src_dynamic_uniform_buffer_ranges, brx_storage_buffer const *const *src_storage_buffers, brx_sampled_image const *const *src_sampled_images, brx_sampler const *const *src_samplers, brx_storage_image const *const *src_storage_images, brx_top_level_acceleration_structure const *const *src_top_level_acceleration_structures);
	uint32_t get_descriptor_count() const;
	brx_d3d12_descriptor const *get_descriptors() const;
};

class brx_d3d12_render_pass : public brx_render_pass
{
	brx_vector<BRX_COLOR_ATTACHMENT_IMAGE_FORMAT> m_color_attachment_formats;
	brx_vector<uint32_t> m_color_attachment_clear_indices;
	brx_vector<uint32_t> m_color_attachment_flush_for_sampled_image_indices;
	brx_vector<uint32_t> m_color_attachment_flush_for_present_indices;
	brx_vector<BRX_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT> m_depth_stencil_attachment_format;
	bool m_depth_stencil_attachment_clear;
	bool m_depth_stencil_attachment_flush_for_sampled_image;

public:
	brx_d3d12_render_pass(
		brx_vector<BRX_COLOR_ATTACHMENT_IMAGE_FORMAT> &&color_attachment_formats,
		brx_vector<uint32_t> &&color_attachment_clear_indices,
		brx_vector<uint32_t> &&color_attachment_flush_for_sampled_image_indices,
		brx_vector<uint32_t> &&color_attachment_flush_for_present_indices,
		brx_vector<BRX_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT> &&depth_stencil_attachment_format,
		bool depth_stencil_attachment_clear,
		bool depth_stencil_attachment_flush_for_sampled_image);
	uint32_t get_color_attachment_count() const;
	BRX_COLOR_ATTACHMENT_IMAGE_FORMAT const *get_color_attachment_formats() const;
	uint32_t get_color_attachments_clear_count() const;
	uint32_t const *get_color_attachment_clear_indices() const;
	uint32_t get_color_attachment_flush_for_sampled_image_count() const;
	uint32_t const *get_color_attachment_flush_for_sampled_image_indices() const;
	uint32_t get_color_attachment_flush_for_present_count() const;
	uint32_t const *get_color_attachment_flush_for_present_indices() const;
	BRX_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT const *get_depth_stencil_attachment_format() const;
	bool get_depth_stencil_attachment_clear() const;
	bool get_depth_stencil_attachment_flush_for_sampled_image() const;
};

class brx_d3d12_graphics_pipeline : public brx_graphics_pipeline
{
	brx_vector<uint32_t> m_vertex_buffer_strides;
	D3D12_PRIMITIVE_TOPOLOGY m_primitive_topology;
	ID3D12PipelineState *m_pipeline_state;

public:
	brx_d3d12_graphics_pipeline();
	void init(ID3D12Device *device, brx_render_pass const *render_pass, brx_pipeline_layout const *pipeline_layout, size_t vertex_shader_module_code_size, void const *vertex_shader_module_code, size_t fragment_shader_module_code_size, void const *fragment_shader_module_code, uint32_t vertex_binding_count, BRX_GRAPHICS_PIPELINE_VERTEX_BINDING const *vertex_bindings, uint32_t vertex_attribute_count, BRX_GRAPHICS_PIPELINE_VERTEX_ATTRIBUTE const *vertex_attributes, bool depth_enable, BRX_GRAPHICS_PIPELINE_COMPARE_OPERATION wrapped_depth_compare_operation);
	void uninit();
	~brx_d3d12_graphics_pipeline();
	uint32_t get_vertex_buffer_count() const;
	uint32_t const *get_vertex_buffer_strides() const;
	D3D12_PRIMITIVE_TOPOLOGY get_primitive_topology() const;
	ID3D12PipelineState *get_pipeline() const;
};

class brx_d3d12_compute_pipeline : public brx_compute_pipeline
{
	ID3D12PipelineState *m_pipeline_state;

public:
	brx_d3d12_compute_pipeline();
	void init(ID3D12Device *device, brx_pipeline_layout const *pipeline_layout, size_t compute_shader_module_code_size, void const *compute_shader_module_code);
	void uninit();
	~brx_d3d12_compute_pipeline();
	ID3D12PipelineState *get_pipeline() const;
};

class brx_d3d12_frame_buffer : public brx_frame_buffer
{
	brx_vector<ID3D12Resource *> m_render_target_resources;
	brx_vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_render_target_view_descriptors;
	ID3D12Resource *m_depth_stencil_resource;
	brx_vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_depth_stencil_view_descriptor;

public:
	brx_d3d12_frame_buffer(brx_vector<ID3D12Resource *> &&render_target_resources, brx_vector<D3D12_CPU_DESCRIPTOR_HANDLE> &&render_target_view_descriptors, ID3D12Resource *depth_stencil_resource, brx_vector<D3D12_CPU_DESCRIPTOR_HANDLE> &&depth_stencil_view_descriptor);
	uint32_t get_num_render_targets() const;
	ID3D12Resource *const *get_render_target_view_resources() const;
	D3D12_CPU_DESCRIPTOR_HANDLE const *get_render_target_view_descriptors() const;
	ID3D12Resource *get_depth_stencil_resource() const;
	D3D12_CPU_DESCRIPTOR_HANDLE const *get_depth_stencil_view_descriptor() const;
};

class brx_d3d12_uniform_upload_buffer : public brx_uniform_upload_buffer
{
	ID3D12Resource *m_resource = NULL;
	D3D12MA::Allocation *m_allocation;
	void *m_host_memory_range_base = NULL;

public:
	brx_d3d12_uniform_upload_buffer();
	void init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *uniform_upload_buffer_memory_pool, uint32_t size);
	void uninit();
	~brx_d3d12_uniform_upload_buffer();
	ID3D12Resource *get_resource() const;
	void *get_host_memory_range_base() const override;
};

class brx_d3d12_staging_upload_buffer : public brx_staging_upload_buffer
{
	ID3D12Resource *m_resource = NULL;
	D3D12MA::Allocation *m_allocation;
	void *m_host_memory_range_base = NULL;

public:
	brx_d3d12_staging_upload_buffer();
	void init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *staging_upload_buffer_memory_pool, uint32_t size);
	void uninit();
	~brx_d3d12_staging_upload_buffer();
	ID3D12Resource *get_resource() const;
	void *get_host_memory_range_base() const override;
};

class brx_d3d12_vertex_buffer : public brx_vertex_buffer
{
public:
	virtual ID3D12Resource *get_resource() const = 0;
};

class brx_d3d12_vertex_position_buffer : public brx_vertex_position_buffer
{
public:
	virtual ID3D12Resource *get_resource() const = 0;
};

class brx_d3d12_vertex_varying_buffer : public brx_vertex_varying_buffer
{
public:
	virtual ID3D12Resource *get_resource() const = 0;
};

class brx_d3d12_index_buffer : public brx_index_buffer
{
public:
	virtual ID3D12Resource *get_resource() const = 0;
};

class brx_d3d12_storage_buffer : public brx_storage_buffer
{
public:
	virtual ID3D12Resource *get_resource() const = 0;
	virtual D3D12_UNORDERED_ACCESS_VIEW_DESC const *get_unordered_access_view_desc() const = 0;
	virtual D3D12_SHADER_RESOURCE_VIEW_DESC const *get_shader_resource_view_desc() const = 0;
};

class brx_d3d12_intermediate_storage_buffer : public brx_intermediate_storage_buffer, brx_d3d12_storage_buffer, brx_d3d12_vertex_position_buffer, brx_d3d12_vertex_varying_buffer, brx_d3d12_vertex_buffer
{
	ID3D12Resource *m_resource;
	D3D12MA::Allocation *m_allocation;
	D3D12_UNORDERED_ACCESS_VIEW_DESC m_unordered_access_view_desc;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_shader_resource_view_desc;

public:
	brx_d3d12_intermediate_storage_buffer();
	void init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *storage_buffer_memory_pool, uint32_t size, bool allow_vertex_position, bool allow_vertex_varying);
	void uninit();
	~brx_d3d12_intermediate_storage_buffer();
	ID3D12Resource *get_resource() const override;
	D3D12_UNORDERED_ACCESS_VIEW_DESC const *get_unordered_access_view_desc() const override;
	D3D12_SHADER_RESOURCE_VIEW_DESC const *get_shader_resource_view_desc() const override;
	brx_vertex_buffer const *get_vertex_buffer() const override;
	brx_vertex_position_buffer const *get_vertex_position_buffer() const override;
	brx_vertex_varying_buffer const *get_vertex_varying_buffer() const override;
	brx_storage_buffer const *get_storage_buffer() const override;
};

class brx_d3d12_asset_vertex_position_buffer : public brx_asset_vertex_position_buffer, brx_d3d12_vertex_position_buffer, brx_d3d12_vertex_buffer, brx_d3d12_storage_buffer
{
	ID3D12Resource *m_resource;
	D3D12MA::Allocation *m_allocation;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_shader_resource_view_desc;

public:
	brx_d3d12_asset_vertex_position_buffer();
	void init(bool uma, D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *asset_vertex_position_buffer_memory_pool, uint32_t size);
	void uninit();
	~brx_d3d12_asset_vertex_position_buffer();
	ID3D12Resource *get_resource() const override;
	D3D12_UNORDERED_ACCESS_VIEW_DESC const *get_unordered_access_view_desc() const override;
	D3D12_SHADER_RESOURCE_VIEW_DESC const *get_shader_resource_view_desc() const override;
	brx_vertex_buffer const *get_vertex_buffer() const override;
	brx_vertex_position_buffer const *get_vertex_position_buffer() const override;
	brx_storage_buffer const *get_storage_buffer() const override;
};

class brx_d3d12_asset_vertex_varying_buffer : public brx_asset_vertex_varying_buffer, brx_d3d12_vertex_varying_buffer, brx_d3d12_vertex_buffer, brx_d3d12_storage_buffer
{
	ID3D12Resource *m_resource;
	D3D12MA::Allocation *m_allocation;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_shader_resource_view_desc;

public:
	brx_d3d12_asset_vertex_varying_buffer();
	void init(bool uma, D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *asset_vertex_varying_buffer_memory_pool, uint32_t size);
	void uninit();
	~brx_d3d12_asset_vertex_varying_buffer();
	ID3D12Resource *get_resource() const override;
	D3D12_UNORDERED_ACCESS_VIEW_DESC const *get_unordered_access_view_desc() const override;
	D3D12_SHADER_RESOURCE_VIEW_DESC const *get_shader_resource_view_desc() const override;
	brx_vertex_buffer const *get_vertex_buffer() const override;
	brx_vertex_varying_buffer const *get_vertex_varying_buffer() const override;
	brx_storage_buffer const *get_storage_buffer() const override;
};

class brx_d3d12_asset_index_buffer : public brx_asset_index_buffer, brx_d3d12_index_buffer, brx_d3d12_storage_buffer
{
	ID3D12Resource *m_resource;
	D3D12MA::Allocation *m_allocation;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_shader_resource_view_desc;

public:
	brx_d3d12_asset_index_buffer();
	void init(bool uma, D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *asset_index_buffer_memory_pool, uint32_t size);
	void uninit();
	~brx_d3d12_asset_index_buffer();
	ID3D12Resource *get_resource() const override;
	virtual D3D12_UNORDERED_ACCESS_VIEW_DESC const *get_unordered_access_view_desc() const override;
	virtual D3D12_SHADER_RESOURCE_VIEW_DESC const *get_shader_resource_view_desc() const override;
	brx_index_buffer const *get_index_buffer() const override;
	brx_storage_buffer const *get_storage_buffer() const override;
};

class brx_d3d12_sampled_image : public brx_sampled_image
{
public:
	virtual ID3D12Resource *get_resource() const = 0;
	virtual D3D12_SHADER_RESOURCE_VIEW_DESC const *get_shader_resource_view_desc() const = 0;
};

class brx_d3d12_color_attachment_image : public brx_color_attachment_image
{
public:
	virtual ID3D12Resource *get_resource() const = 0;
	virtual D3D12_CPU_DESCRIPTOR_HANDLE get_render_target_view_descriptor() const = 0;
};

class brx_d3d12_depth_stencil_attachment_image : public brx_depth_stencil_attachment_image
{
public:
	virtual ID3D12Resource *get_resource() const = 0;
	virtual D3D12_CPU_DESCRIPTOR_HANDLE get_depth_stencil_view_descriptor() const = 0;
};

class brx_d3d12_storage_image : public brx_storage_image
{
public:
	virtual ID3D12Resource *get_resource() const = 0;
	virtual D3D12_UNORDERED_ACCESS_VIEW_DESC const *get_unordered_access_view_desc() const = 0;
};

class brx_d3d12_intermediate_color_attachment_image : public brx_d3d12_color_attachment_image, brx_d3d12_sampled_image
{
	ID3D12Heap *m_heap;
	ID3D12Resource *m_resource;
	ID3D12DescriptorHeap *m_render_target_view_descriptor_heap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_render_target_view_descriptor;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_shader_resource_view_desc;

public:
	brx_d3d12_intermediate_color_attachment_image();
	void init(ID3D12Device *device, bool uma, BRX_COLOR_ATTACHMENT_IMAGE_FORMAT color_attachment_image_format, uint32_t width, uint32_t height, bool allow_sampled_image);
	void uninit();
	~brx_d3d12_intermediate_color_attachment_image();
	ID3D12Resource *get_resource() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE get_render_target_view_descriptor() const override;
	D3D12_SHADER_RESOURCE_VIEW_DESC const *get_shader_resource_view_desc() const override;
	brx_sampled_image const *get_sampled_image() const override;
};

class brx_d3d12_intermediate_depth_stencil_attachment_image : public brx_d3d12_depth_stencil_attachment_image, brx_d3d12_sampled_image
{
	ID3D12Heap *m_heap;
	ID3D12Resource *m_resource;
	ID3D12DescriptorHeap *m_depth_stencil_view_descriptor_heap;
	D3D12_CPU_DESCRIPTOR_HANDLE m_depth_stencil_view_descriptor;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_shader_resource_view_desc;

public:
	brx_d3d12_intermediate_depth_stencil_attachment_image();
	void init(ID3D12Device *device, bool uma, BRX_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT depth_stencil_attachment_image_format, uint32_t width, uint32_t height, bool allow_sampled_image);
	void uninit();
	~brx_d3d12_intermediate_depth_stencil_attachment_image();
	ID3D12Resource *get_resource() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE get_depth_stencil_view_descriptor() const override;
	D3D12_SHADER_RESOURCE_VIEW_DESC const *get_shader_resource_view_desc() const override;
	brx_sampled_image const *get_sampled_image() const override;
};

class brx_d3d12_intermediate_storage_image : public brx_d3d12_storage_image, brx_d3d12_sampled_image
{
	ID3D12Resource *m_resource;
	D3D12MA::Allocation *m_allocation;
	D3D12_UNORDERED_ACCESS_VIEW_DESC m_unordered_access_view_desc;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_shader_resource_view_desc;

public:
	brx_d3d12_intermediate_storage_image();
	void init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *storage_image_memory_pool, DXGI_FORMAT storage_image_format, uint32_t width, uint32_t height, bool allow_sampled_image);
	void uninit();
	~brx_d3d12_intermediate_storage_image();
	ID3D12Resource *get_resource() const override;
	D3D12_UNORDERED_ACCESS_VIEW_DESC const *get_unordered_access_view_desc() const override;
	D3D12_SHADER_RESOURCE_VIEW_DESC const *get_shader_resource_view_desc() const override;
	brx_sampled_image const *get_sampled_image() const override;
};

class brx_d3d12_asset_sampled_image : public brx_asset_sampled_image, brx_d3d12_sampled_image
{
	ID3D12Resource *m_resource;
	D3D12MA::Allocation *m_allocation;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_shader_resource_view_desc;

public:
	brx_d3d12_asset_sampled_image();
	void init(bool uma, D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *asset_sampled_image_memory_pool, DXGI_FORMAT asset_sampled_image_format, uint32_t width, uint32_t height, uint32_t mip_levels);
	void uninit();
	~brx_d3d12_asset_sampled_image();
	ID3D12Resource *get_resource() const override;
	D3D12_SHADER_RESOURCE_VIEW_DESC const *get_shader_resource_view_desc() const override;
	brx_sampled_image const *get_sampled_image() const override;
};

class brx_d3d12_sampler : public brx_sampler
{
	D3D12_SAMPLER_DESC m_sampler_desc;

public:
	brx_d3d12_sampler(D3D12_SAMPLER_DESC const *sampler_desc);
	D3D12_SAMPLER_DESC const *get_sampler_desc() const;
};

class brx_d3d12_swap_chain_image : public brx_d3d12_color_attachment_image
{
	ID3D12Resource *m_resource;
	D3D12_CPU_DESCRIPTOR_HANDLE m_render_target_view_descriptor;

public:
	brx_d3d12_swap_chain_image(ID3D12Resource *resource, D3D12_CPU_DESCRIPTOR_HANDLE render_target_view_descriptor);
	void steal(ID3D12Resource **out_resource);
	ID3D12Resource *get_resource() const override;
	D3D12_CPU_DESCRIPTOR_HANDLE get_render_target_view_descriptor() const override;
	brx_sampled_image const *get_sampled_image() const override;
};

class brx_d3d12_swap_chain : public brx_swap_chain
{
	IDXGISwapChain3 *m_swap_chain;
	DXGI_FORMAT m_image_format;
	uint32_t m_image_width;
	uint32_t m_image_height;
	uint32_t m_image_count;
	ID3D12DescriptorHeap *m_rtv_descriptor_heap;
	brx_vector<brx_d3d12_swap_chain_image> m_images;

public:
	brx_d3d12_swap_chain(IDXGISwapChain3 *swap_chain, DXGI_FORMAT image_format, uint32_t image_width, uint32_t image_height, uint32_t image_count, ID3D12DescriptorHeap *rtv_descriptor_heap, brx_vector<brx_d3d12_swap_chain_image> &&m_images);
	IDXGISwapChain3 *get_swap_chain() const;
	BRX_COLOR_ATTACHMENT_IMAGE_FORMAT get_image_format() const override;
	uint32_t get_image_width() const override;
	uint32_t get_image_height() const override;
	uint32_t get_image_count() const override;
	brx_color_attachment_image const *get_image(uint32_t swap_chain_image_index) const override;
	void steal(IDXGISwapChain3 **out_swap_chain, ID3D12DescriptorHeap **out_rtv_descriptor_heap, brx_vector<brx_d3d12_swap_chain_image> &out_images);
	~brx_d3d12_swap_chain();
};

class brx_d3d12_scratch_buffer : public brx_scratch_buffer
{
	ID3D12Resource *m_resource;
	D3D12MA::Allocation *m_allocation;

public:
	brx_d3d12_scratch_buffer();
	void init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *scratch_buffer_memory_pool, uint32_t size);
	void uninit();
	~brx_d3d12_scratch_buffer();
	ID3D12Resource *get_resource() const;
};

class brx_d3d12_staging_non_compacted_bottom_level_acceleration_structure : public brx_staging_non_compacted_bottom_level_acceleration_structure
{
	ID3D12Resource *m_resource;
	D3D12MA::Allocation *m_allocation;

public:
	brx_d3d12_staging_non_compacted_bottom_level_acceleration_structure();
	void init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *staging_non_compacted_bottom_level_acceleration_structure_memory_pool, uint32_t size);
	void uninit();
	~brx_d3d12_staging_non_compacted_bottom_level_acceleration_structure();
	ID3D12Resource *get_resource() const;
};

class brx_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool : public brx_compacted_bottom_level_acceleration_structure_size_query_pool
{
	ID3D12Resource *m_resource;
	D3D12MA::Allocation *m_allocation;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC volatile *m_host_memory_range_base;

public:
	brx_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool();
	void init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *compacted_bottom_level_acceleration_structure_size_query_buffer_memory_pool, uint32_t query_count);
	void uninit();
	~brx_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool();
	ID3D12Resource *get_resource() const;
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC volatile *get_host_memory_range_base() const;
};

class brx_d3d12_asset_compacted_bottom_level_acceleration_structure : public brx_asset_compacted_bottom_level_acceleration_structure
{
	ID3D12Resource *m_resource;
	D3D12MA::Allocation *m_allocation;

public:
	brx_d3d12_asset_compacted_bottom_level_acceleration_structure();
	void init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *asset_compacted_bottom_level_acceleration_structure_memory_pool, uint32_t size);
	void uninit();
	~brx_d3d12_asset_compacted_bottom_level_acceleration_structure();
	ID3D12Resource *get_resource() const;
};

class brx_d3d12_top_level_acceleration_structure_instance_upload_buffer : public brx_top_level_acceleration_structure_instance_upload_buffer
{
	ID3D12Resource *m_resource;
	D3D12MA::Allocation *m_allocation;
	D3D12_RAYTRACING_INSTANCE_DESC *m_host_memory_range_base;

public:
	brx_d3d12_top_level_acceleration_structure_instance_upload_buffer();
	void init(uint32_t instance_count, D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *top_level_acceleration_structure_instance_upload_buffer_memory_pool);
	void uninit();
	~brx_d3d12_top_level_acceleration_structure_instance_upload_buffer();
	void write_instance(uint32_t instance_index, BRX_TOP_LEVEL_ACCELERATION_STRUCTURE_INSTANCE const *bottom_top_acceleration_structure_instance) override;
	ID3D12Resource *get_resource() const;
};

class brx_d3d12_top_level_acceleration_structure : public brx_top_level_acceleration_structure
{
	ID3D12Resource *m_resource;
	D3D12MA::Allocation *m_allocation;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_shader_resource_view_desc;
	uint32_t m_instance_count;

public:
	brx_d3d12_top_level_acceleration_structure();
	void init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *top_level_acceleration_structure_memory_pool, uint32_t size);
	void uninit();
	~brx_d3d12_top_level_acceleration_structure();
	ID3D12Resource *get_resource() const;
	D3D12_SHADER_RESOURCE_VIEW_DESC const *get_shader_resource_view_desc() const;
	void set_instance_count(uint32_t instance_count);
	uint32_t get_instance_count() const;
};

#endif