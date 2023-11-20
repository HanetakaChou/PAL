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

#ifndef _PAL_VK_DEVICE_H_
#define _PAL_VK_DEVICE_H_ 1

#include "../include/pal_device.h"
#if defined(__GNUC__)
#if defined(__linux__) && defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR 1
#else
#error Unknown Platform
#endif
#elif defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX 1
#define VK_USE_PLATFORM_WIN32_KHR 1
#include <sdkddkver.h>
#include <windows.h>
#else
#error Unknown Compiler
#endif
#include "../thirdparty/Vulkan-Headers/include/vulkan/vulkan.h"
#include "../thirdparty/VulkanMemoryAllocator/include/vk_mem_alloc.h"

// TODO: may be used by other stages
extern VkPipelineStageFlags const g_graphics_queue_family_all_supported_shader_stages;
extern VkPipelineStageFlags const g_upload_queue_family_all_supported_shader_stages;

class pal_vk_device : public pal_device
{
	bool m_support_ray_tracing;

	VkAllocationCallbacks *m_allocation_callbacks;

	PFN_vkGetInstanceProcAddr m_pfn_get_instance_proc_addr;
	VkInstance m_instance;

#ifndef NDEBUG
	VkDebugUtilsMessengerEXT m_messenge;
#endif

	VkPhysicalDevice m_physical_device;
	uint32_t m_min_uniform_buffer_offset_alignment;
	uint32_t m_min_storage_buffer_offset_alignment;
	uint32_t m_optimal_buffer_copy_offset_alignment;
	uint32_t m_optimal_buffer_copy_row_pitch_alignment;

	bool m_has_dedicated_upload_queue;
	uint32_t m_graphics_queue_family_index;
	uint32_t m_upload_queue_family_index;

	PFN_vkGetDeviceProcAddr m_pfn_get_device_proc_addr;
	bool m_physical_device_feature_texture_compression_BC;
	bool m_physical_device_feature_texture_compression_ASTC_LDR;
	VkDevice m_device;

	VkQueue m_graphics_queue;
	VkQueue m_upload_queue;

	VkFormat m_depth_attachment_image_format;
	VkFormat m_depth_stencil_attachment_image_format;
	bool m_depth_attachment_image_format_support_sampled_image;
	bool m_depth_stencil_attachment_image_format_support_sampled_image;

	uint32_t m_upload_ring_buffer_memory_index;
	uint32_t m_staging_buffer_memory_index;
	uint32_t m_asset_vertex_position_buffer_memory_index;
	uint32_t m_asset_vertex_varying_buffer_memory_index;
	uint32_t m_asset_index_buffer_memory_index;
	uint32_t m_color_transient_attachment_image_memory_index;
	uint32_t m_color_attachment_sampled_image_memory_index;
	uint32_t m_depth_transient_attachment_image_memory_index;
	uint32_t m_depth_attachment_sampled_image_memory_index;
	uint32_t m_depth_stencil_transient_attachment_image_memory_index;
	uint32_t m_depth_stencil_attachment_sampled_image_memory_index;
	uint32_t m_storage_image_memory_index;
	uint32_t m_asset_sampled_image_memory_index;
	uint32_t m_scratch_buffer_memory_index;
	uint32_t m_staging_non_compacted_bottom_level_acceleration_structure_buffer_memory_index;
	uint32_t m_asset_compacted_bottom_level_acceleration_structure_memory_index;
	uint32_t m_top_level_acceleration_structure_instance_buffer_memory_index;
	uint32_t m_top_level_acceleration_structure_memory_index;

	PFN_vkWaitForFences m_pfn_wait_for_fences;
	PFN_vkResetFences m_pfn_reset_fences;
	PFN_vkResetCommandPool m_pfn_reset_command_pool;
	PFN_vkAcquireNextImageKHR m_pfn_acquire_next_image;
	PFN_vkGetQueryPoolResults m_pfn_get_query_pool_results;

	VmaAllocator m_asset_allocator;

public:
	pal_vk_device();
	void init(bool support_ray_tracing);
	void uninit();
	~pal_vk_device();
	pal_graphics_queue *create_graphics_queue() const override;
	void destroy_graphics_queue(pal_graphics_queue *graphics_queue) const override;
	pal_upload_queue *create_upload_queue() const override;
	void destroy_upload_queue(pal_upload_queue *upload_queue) const override;
	pal_graphics_command_buffer *create_graphics_command_buffer() const override;
	void reset_graphics_command_buffer(pal_graphics_command_buffer *graphics_command_buffer) const override;
	void destroy_graphics_command_buffer(pal_graphics_command_buffer *graphics_command_buffer) const override;
	pal_upload_command_buffer *create_upload_command_buffer() const override;
	void reset_upload_command_buffer(pal_upload_command_buffer *upload_command_buffer) const override;
	void destroy_upload_command_buffer(pal_upload_command_buffer *upload_command_buffer) const override;
	pal_fence *create_fence(bool signaled) const override;
	void wait_for_fence(pal_fence *fence) const override;
	void reset_fence(pal_fence *fence) const override;
	void destroy_fence(pal_fence *fence) const override;
	pal_descriptor_set_layout *create_descriptor_set_layout(uint32_t descriptor_set_binding_count, PAL_DESCRIPTOR_SET_LAYOUT_BINDING const *descriptor_set_bindings) const override;
	void destroy_descriptor_set_layout(pal_descriptor_set_layout *descriptor_set_layout) const override;
	pal_pipeline_layout *create_pipeline_layout(uint32_t descriptor_set_layout_count, pal_descriptor_set_layout const *const *descriptor_set_layouts) const override;
	void destroy_pipeline_layout(pal_pipeline_layout *pipeline_layout) const;
	pal_descriptor_set *create_descriptor_set(pal_descriptor_set_layout const *descriptor_set_layout) override;
	void write_descriptor_set(pal_descriptor_set *descriptor_set, PAL_DESCRIPTOR_TYPE descriptor_type, uint32_t dst_binding, uint32_t dst_array_element, uint32_t src_descriptor_count, pal_upload_ring_buffer const *const *src_dynamic_uniform_buffers, uint32_t const *src_dynamic_uniform_buffers_range, pal_sampled_image const *const *src_sampled_images, pal_sampler const *const *src_samplers, pal_storage_image const *const *src_storage_images, pal_top_level_acceleration_structure const *const *src_top_level_acceleration_structures) const override;
	void destroy_descriptor_set(pal_descriptor_set *descriptor_set) override;
	pal_render_pass *create_render_pass(uint32_t color_attachment_count, PAL_RENDER_PASS_COLOR_ATTACHMENT const *color_attachments, PAL_RENDER_PASS_DEPTH_STENCIL_ATTACHMENT const *depth_stencil_attachment) const override;
	void destroy_render_pass(pal_render_pass *render_pass) const override;
	pal_graphics_pipeline *create_graphics_pipeline(pal_render_pass const *render_pass, pal_pipeline_layout const *pipeline_layout, size_t vertex_shader_module_code_size, void const *vertex_shader_module_code, size_t fragment_shader_module_code_size, void const *fragment_shader_module_code, uint32_t vertex_binding_count, PAL_GRAPHICS_PIPELINE_VERTEX_BINDING const *vertex_bindings, uint32_t vertex_attribute_count, PAL_GRAPHICS_PIPELINE_VERTEX_ATTRIBUTE const *vertex_attributes, bool depth_enable, PAL_GRAPHICS_PIPELINE_COMPARE_OPERATION depth_compare_operation) const override;
	void destroy_graphics_pipeline(pal_graphics_pipeline *graphics_pipeline) const override;
	pal_compute_pipeline *create_compute_pipeline(pal_pipeline_layout const *pipeline_layout, size_t compute_shader_module_code_size, void const *compute_shader_module_code) const override;
	void destroy_compute_pipeline(pal_compute_pipeline *compute_pipeline) const override;
	pal_frame_buffer *create_frame_buffer(pal_render_pass const *render_pass, uint32_t width, uint32_t height, uint32_t color_attachment_count, pal_color_attachment_image const *const *color_attachments, pal_depth_stencil_attachment_image const *depth_stencil_attachment) const override;
	void destroy_frame_buffer(pal_frame_buffer *frame_buffer) const override;
	uint32_t get_upload_ring_buffer_offset_alignment() const override;
	pal_upload_ring_buffer *create_upload_ring_buffer(uint32_t size) const override;
	void destroy_upload_ring_buffer(pal_upload_ring_buffer *upload_ring_buffer) const override;
	uint32_t get_staging_buffer_offset_alignment() const override;
	uint32_t get_staging_buffer_row_pitch_alignment() const override;
	pal_staging_buffer *create_staging_buffer(uint32_t size) const override;
	void destroy_staging_buffer(pal_staging_buffer *staging_buffer) const override;
	pal_asset_vertex_position_buffer *create_asset_vertex_position_buffer(uint32_t size) const override;
	void destroy_asset_vertex_position_buffer(pal_asset_vertex_position_buffer *asset_vertex_position_buffer) const override;
	pal_asset_vertex_varying_buffer *create_asset_vertex_varying_buffer(uint32_t size) const override;
	void destroy_asset_vertex_varying_buffer(pal_asset_vertex_varying_buffer *asset_vertex_varying_buffer) const override;
	pal_asset_index_buffer *create_asset_index_buffer(uint32_t size) const override;
	void destroy_asset_index_buffer(pal_asset_index_buffer *asset_index_buffer) const override;
	pal_color_attachment_image *create_color_attachment_image(PAL_COLOR_ATTACHMENT_IMAGE_FORMAT color_attachment_image_format, uint32_t width, uint32_t height, bool allow_sampled_image) const override;
	void destroy_color_attachment_image(pal_color_attachment_image *color_attachment_image) const override;
	PAL_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT get_depth_attachment_image_format() const override;
	PAL_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT get_depth_stencil_attachment_image_format() const override;
	pal_depth_stencil_attachment_image *create_depth_stencil_attachment_image(PAL_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT depth_stencil_attachment_image_format, uint32_t width, uint32_t height, bool allow_sampled_image) const;
	void destroy_depth_stencil_attachment_image(pal_depth_stencil_attachment_image *depth_stencil_attachment_image) const override;
	pal_storage_image *create_storage_image(PAL_STORAGE_IMAGE_FORMAT storage_image_format, uint32_t width, uint32_t height, bool allow_sampled_image) const override;
	void destroy_storage_image(pal_storage_image *storage_image) const override;
	pal_asset_sampled_image *create_asset_sampled_image(PAL_ASSET_IMAGE_FORMAT asset_sampled_image_format, uint32_t width, uint32_t height, uint32_t mip_levels) const override;
	void destroy_asset_sampled_image(pal_asset_sampled_image *asset_sampled_image) const override;
	pal_sampler *create_sampler(PAL_SAMPLER_FILTER filter) const override;
	void destroy_sampler(pal_sampler *sampler) const override;
	pal_surface *create_surface(void *window) const override;
	void destroy_surface(pal_surface *surface) const override;
	pal_swap_chain *create_swap_chain(pal_surface *surface) const override;
	bool acquire_next_image(pal_graphics_command_buffer *graphics_command_buffer, pal_swap_chain const *swap_chain, uint32_t *out_swap_chain_image_index) const override;
	void destroy_swap_chain(pal_swap_chain *swap_chain) const override;
	uint32_t get_scratch_buffer_offset_alignment() const override;
	pal_scratch_buffer *create_scratch_buffer(uint32_t size) const override;
	void destroy_scratch_buffer(pal_scratch_buffer *scratch_buffer) const override;
	uint32_t get_staging_non_compacted_bottom_level_acceleration_structure_buffer_offset_alignment() const override;
	pal_staging_non_compacted_bottom_level_acceleration_structure_buffer *create_staging_non_compacted_bottom_level_acceleration_structure_buffer(uint32_t size) const override;
	void destroy_staging_non_compacted_bottom_level_acceleration_structure_buffer(pal_staging_non_compacted_bottom_level_acceleration_structure_buffer *staging_non_compacted_bottom_level_acceleration_structure_buffer) const override;
	void get_staging_non_compacted_bottom_level_acceleration_structure_size(uint32_t acceleration_structure_geometry_count, PAL_BOTTOM_LEVEL_ACCELERATION_STRUCTURE_GEOMETRY const *acceleration_structure_geometries, uint32_t *acceleration_structure_size, uint32_t *build_scratch_size) const override;
	pal_staging_non_compacted_bottom_level_acceleration_structure *create_staging_non_compacted_bottom_level_acceleration_structure(pal_staging_non_compacted_bottom_level_acceleration_structure_buffer const *staging_non_compacted_bottom_level_acceleration_structure_buffer, uint32_t offset, uint32_t size) const override;
	void destroy_staging_non_compacted_bottom_level_acceleration_structure(pal_staging_non_compacted_bottom_level_acceleration_structure *staging_non_compacted_bottom_level_acceleration_structure) const override;
	pal_compacted_bottom_level_acceleration_structure_size_query_pool *create_compacted_bottom_level_acceleration_structure_size_query_pool(uint32_t query_count) const override;
	uint32_t get_compacted_bottom_level_acceleration_structure_size_query_pool_result(pal_compacted_bottom_level_acceleration_structure_size_query_pool const *compacted_bottom_level_acceleration_structure_size_query_pool, uint32_t query_index) const override;
	void destroy_compacted_bottom_level_acceleration_structure_size_query_pool(pal_compacted_bottom_level_acceleration_structure_size_query_pool *compacted_bottom_level_acceleration_structure_size_query_pool) const override;
	pal_asset_compacted_bottom_level_acceleration_structure *create_asset_compacted_bottom_level_acceleration_structure(uint32_t size) const override;
	void destroy_asset_compacted_bottom_level_acceleration_structure(pal_asset_compacted_bottom_level_acceleration_structure *asset_compacted_bottom_level_acceleration_structure) const override;
	pal_top_level_acceleration_structure_instance_buffer *create_top_level_acceleration_structure_instance_buffer(uint32_t instance_count) const override;
	void write_top_level_acceleration_structure_instance_buffer(pal_top_level_acceleration_structure_instance_buffer *top_level_acceleration_structure_instance_buffer, uint32_t instance_index, float const transform_matrix[3][4], bool force_closest_hit, bool force_any_hit, bool disable_back_face_cull, bool front_ccw, pal_asset_compacted_bottom_level_acceleration_structure *asset_compacted_bottom_level_acceleration_structure) const override;
	void destroy_top_level_acceleration_structure_instance_buffer(pal_top_level_acceleration_structure_instance_buffer *top_level_acceleration_structure_instance_buffer) const override;
	void get_top_level_acceleration_structure_size(uint32_t top_level_acceleration_structure_instance_count, uint32_t *top_level_acceleration_structure_size, uint32_t *build_scratch_size, uint32_t *update_scratch_size) const override;
	pal_top_level_acceleration_structure *create_top_level_acceleration_structure(uint32_t size) const override;
	void destroy_top_level_acceleration_structure(pal_top_level_acceleration_structure *top_level_acceleration_structure) const override;
};

class pal_vk_graphics_queue : public pal_graphics_queue
{
	VkQueue m_graphics_queue;

	bool m_has_dedicated_upload_queue;
	uint32_t m_upload_queue_family_index;
	uint32_t m_graphics_queue_family_index;

	PFN_vkQueueSubmit m_pfn_queue_submit;
	PFN_vkQueuePresentKHR m_pfn_queue_present;

public:
	pal_vk_graphics_queue(bool has_dedicated_upload_queue, uint32_t upload_queue_family_index, uint32_t graphics_queue_family_index, VkQueue graphics_queue, PFN_vkQueueSubmit pfn_queue_submit, PFN_vkQueuePresentKHR pfn_queue_present);
	void wait_and_submit(pal_upload_command_buffer const *upload_command_buffer, pal_graphics_command_buffer const *graphics_command_buffer, pal_fence *fence) const override;
	bool submit_and_present(pal_graphics_command_buffer *graphics_command_buffer, pal_swap_chain *swap_chain, uint32_t swap_chain_image_index, pal_fence *fence) const override;
	void steal(VkQueue *out_graphics_queue);
	~pal_vk_graphics_queue();
};

class pal_vk_upload_queue : public pal_upload_queue
{
	VkQueue m_upload_queue;

	bool m_has_dedicated_upload_queue;
	uint32_t m_upload_queue_family_index;
	uint32_t m_graphics_queue_family_index;

	PFN_vkQueueSubmit m_pfn_queue_submit;

public:
	pal_vk_upload_queue(bool has_dedicated_upload_queue, uint32_t upload_queue_family_index, uint32_t graphics_queue_family_index, VkQueue upload_queue, PFN_vkQueueSubmit pfn_queue_submit);
	void submit_and_signal(pal_upload_command_buffer const *upload_command_buffer) const override;
	void steal(VkQueue *out_upload_queue);
	~pal_vk_upload_queue();
};

class pal_vk_graphics_command_buffer : public pal_graphics_command_buffer
{
	bool m_support_ray_tracing;

	bool m_has_dedicated_upload_queue;
	uint32_t m_graphics_queue_family_index;
	uint32_t m_upload_queue_family_index;

	VkCommandPool m_command_pool;
	VkCommandBuffer m_command_buffer;

	VkSemaphore m_acquire_next_image_semaphore;
	VkSemaphore m_queue_submit_semaphore;

	PFN_vkBeginCommandBuffer m_pfn_begin_command_buffer;
	PFN_vkCmdPipelineBarrier m_pfn_cmd_pipeline_barrier;
#ifndef NDEBUG
	PFN_vkCmdBeginDebugUtilsLabelEXT m_pfn_cmd_begin_debug_utils_label;
	PFN_vkCmdEndDebugUtilsLabelEXT m_pfn_cmd_end_debug_utils_label;
#endif
	PFN_vkCmdBeginRenderPass m_pfn_cmd_begin_render_pass;
	PFN_vkCmdBindPipeline m_pfn_cmd_bind_pipeline;
	PFN_vkCmdSetViewport m_pfn_cmd_set_view_port;
	PFN_vkCmdSetScissor m_pfn_cmd_set_scissor;
	PFN_vkCmdBindDescriptorSets m_pfn_cmd_bind_descriptor_sets;
	PFN_vkCmdBindVertexBuffers m_pfn_cmd_bind_vertex_buffers;
	PFN_vkCmdBindIndexBuffer m_pfn_cmd_bind_index_buffer;
	PFN_vkCmdDraw m_pfn_cmd_draw;
	PFN_vkCmdDrawIndexed m_pfn_cmd_draw_indexed;
	PFN_vkCmdEndRenderPass m_pfn_cmd_end_render_pass;
	PFN_vkCmdDispatch m_pfn_cmd_dispatch;
	PFN_vkCmdBuildAccelerationStructuresKHR m_pfn_cmd_build_acceleration_structure;
	PFN_vkEndCommandBuffer m_pfn_end_command_buffer;

public:
	pal_vk_graphics_command_buffer();
	void init(bool support_ray_tracing, bool has_dedicated_upload_queue, uint32_t graphics_queue_family_index, uint32_t upload_queue_family_index, PFN_vkGetInstanceProcAddr pfn_get_instance_proc_addr, VkInstance instance, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_graphics_command_buffer();
	VkCommandPool get_command_pool() const;
	VkCommandBuffer get_command_buffer() const;
	VkSemaphore get_acquire_next_image_semaphore() const;
	VkSemaphore get_queue_submit_semaphore() const;
	void begin() override;
	void acquire_asset_vertex_position_buffer(pal_asset_vertex_position_buffer *asset_vertex_position_buffer) override;
	void acquire_asset_vertex_varying_buffer(pal_asset_vertex_varying_buffer *asset_vertex_varying_buffer) override;
	void acquire_asset_index_buffer(pal_asset_index_buffer *asset_index_buffer) override;
	void acquire_asset_sampled_image(pal_asset_sampled_image *asset_sampled_image, uint32_t dst_mip_level) override;
	void acquire_asset_compacted_bottom_level_acceleration_structure(pal_asset_compacted_bottom_level_acceleration_structure *asset_compacted_bottom_level_acceleration_structure) override;
	void begin_debug_utils_label(char const *label_name) override;
	void end_debug_utils_label() override;
	void begin_render_pass(pal_render_pass const *render_pass, pal_frame_buffer const *frame_buffer, uint32_t width, uint32_t height, uint32_t color_clear_value_count, float const (*color_clear_values)[4], float const *depth_clear_value, uint8_t const *stencil_clear_value) override;
	void bind_graphics_pipeline(pal_graphics_pipeline const *graphics_pipeline) override;
	void set_view_port(uint32_t width, uint32_t height) override;
	void set_scissor(uint32_t width, uint32_t height) override;
	void bind_graphics_descriptor_sets(pal_pipeline_layout const *pipeline_layout, uint32_t descriptor_set_count, pal_descriptor_set const *const *descriptor_sets, uint32_t dynamic_offet_count, uint32_t const *dynamic_offsets) override;
	void bind_vertex_buffers(uint32_t vertex_buffer_count, pal_vertex_buffer const *const *vertex_buffers) override;
	void draw(uint32_t vertex_count, uint32_t instance_count) override;
	void draw_index(pal_asset_index_buffer const *index_buffer, PAL_GRAPHICS_PIPELINE_INDEX_TYPE index_type, uint32_t index_count, uint32_t instance_count) override;
	void end_render_pass() override;
	void compute_pass_load_storage_image(pal_storage_image const *storage_image, PAL_STORAGE_IMAGE_LOAD_OPERATION load_operation) override;
	void bind_compute_pipeline(pal_compute_pipeline const *compute_pipeline) override;
	void bind_compute_descriptor_sets(pal_pipeline_layout const *pipeline_layout, uint32_t descriptor_set_count, pal_descriptor_set const *const *descriptor_sets, uint32_t dynamic_offet_count, uint32_t const *dynamic_offsets) override;
	void dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z) override;
	void compute_pass_store_storage_image(pal_storage_image const *storage_image, PAL_STORAGE_IMAGE_STORE_OPERATION store_operation) override;
	void build_top_level_acceleration_structure(pal_scratch_buffer *scratch_buffer, uint32_t scratch_buffer_offset, uint32_t top_level_acceleration_structure_instance_count, pal_top_level_acceleration_structure_instance_buffer *top_level_acceleration_structure_instance_buffer, uint32_t top_level_acceleration_structure_start_instance_index, pal_top_level_acceleration_structure *top_level_acceleration_structure) override;
	void update_top_level_acceleration_structure(pal_scratch_buffer *scratch_buffer, uint32_t scratch_buffer_offset, uint32_t top_level_acceleration_structure_instance_count, pal_top_level_acceleration_structure_instance_buffer *top_level_acceleration_structure_instance_buffer, uint32_t top_level_acceleration_structure_start_instance_index, pal_top_level_acceleration_structure *top_level_acceleration_structure) override;
	void end() override;
};

class pal_vk_upload_command_buffer : public pal_upload_command_buffer
{
	bool m_support_ray_tracing;

	bool m_has_dedicated_upload_queue;
	uint32_t m_graphics_queue_family_index;
	uint32_t m_upload_queue_family_index;

	VkCommandPool m_graphics_command_pool;
	VkCommandBuffer m_graphics_command_buffer;
	VkCommandPool m_upload_command_pool;
	VkCommandBuffer m_upload_command_buffer;

	VkSemaphore m_upload_queue_submit_semaphore;

	PFN_vkBeginCommandBuffer m_pfn_begin_command_buffer;
	PFN_vkCmdPipelineBarrier m_pfn_cmd_pipeline_barrier;
	PFN_vkCmdCopyBuffer m_pfn_cmd_copy_buffer;
	PFN_vkCmdCopyBufferToImage m_pfn_cmd_copy_buffer_to_image;
	PFN_vkCmdBuildAccelerationStructuresKHR m_pfn_cmd_build_acceleration_structure;
	PFN_vkCmdResetQueryPool m_pfn_cmd_reset_query_pool;
	PFN_vkCmdWriteAccelerationStructuresPropertiesKHR m_pfn_cmd_write_acceleration_structures_properties;
	PFN_vkCmdCopyAccelerationStructureKHR m_pfn_cmd_copy_acceleration_structure;
	PFN_vkEndCommandBuffer m_pfn_end_command_buffer;

public:
	pal_vk_upload_command_buffer();
	void init(bool support_ray_tracing, bool has_dedicated_upload_queue, uint32_t graphics_queue_family_index, uint32_t upload_queue_family_index, PFN_vkGetInstanceProcAddr pfn_get_instance_proc_addr, VkInstance instance, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_upload_command_buffer();
	VkCommandPool get_upload_command_pool() const;
	VkCommandBuffer get_upload_command_buffer() const;
	VkCommandPool get_graphics_command_pool() const;
	VkCommandBuffer get_graphics_command_buffer() const;
	VkSemaphore get_upload_queue_submit_semaphore() const;
	void begin() override;
	void upload_from_staging_buffer_to_asset_vertex_position_buffer(pal_asset_vertex_position_buffer *asset_vertex_position_buffer, uint64_t dst_offset, pal_staging_buffer *staging_buffer, uint64_t src_offset, uint32_t src_size) override;
	void upload_from_staging_buffer_to_asset_vertex_varying_buffer(pal_asset_vertex_varying_buffer *asset_vertex_varying_buffer, uint64_t dst_offset, pal_staging_buffer *staging_buffer, uint64_t src_offset, uint32_t src_size) override;
	void upload_from_staging_buffer_to_asset_index_buffer(pal_asset_index_buffer *asset_index_buffer, uint64_t dst_offset, pal_staging_buffer *staging_buffer, uint64_t src_offset, uint32_t src_size) override;
	void upload_from_staging_buffer_to_asset_sampled_image(pal_asset_sampled_image *asset_sampled_image, PAL_ASSET_IMAGE_FORMAT asset_sampled_image_format, uint32_t asset_sampled_image_width, uint32_t asset_sampled_image_height, uint32_t dst_mip_level, pal_staging_buffer *staging_buffer, uint64_t src_offset, uint32_t src_row_pitch, uint32_t src_row_count) override;
	void build_staging_non_compacted_bottom_level_acceleration_structure(pal_scratch_buffer *scratch_buffer, uint32_t scratch_buffer_offset, pal_staging_non_compacted_bottom_level_acceleration_structure *staging_non_compacted_bottom_level_acceleration_structure, uint32_t acceleration_structure_geometry_count, PAL_BOTTOM_LEVEL_ACCELERATION_STRUCTURE_GEOMETRY const *acceleration_structure_geometries, pal_compacted_bottom_level_acceleration_structure_size_query_pool *compacted_bottom_level_acceleration_structure_size_query_pool, uint32_t query_index) override;
	void compact_bottom_level_acceleration_structure(pal_asset_compacted_bottom_level_acceleration_structure *destination_asset_compacted_bottom_level_acceleration_structure, pal_staging_non_compacted_bottom_level_acceleration_structure *source_staging_non_compacted_bottom_level_acceleration_structure) override;
	void release_asset_vertex_position_buffer(pal_asset_vertex_position_buffer *asset_vertex_position_buffer) override;
	void release_asset_vertex_varying_buffer(pal_asset_vertex_varying_buffer *asset_vertex_varying_buffer) override;
	void release_asset_index_buffer(pal_asset_index_buffer *asset_index_buffer) override;
	void release_asset_sampled_image(pal_asset_sampled_image *asset_sampled_image, uint32_t dst_mip_level) override;
	void release_asset_compacted_bottom_level_acceleration_structure(pal_asset_compacted_bottom_level_acceleration_structure *asset_compacted_bottom_level_acceleration_structure) override;
	void end() override;
};

class pal_vk_fence : public pal_fence
{
	VkFence m_fence;

public:
	pal_vk_fence(VkFence fence);
	VkFence get_fence() const;
	void steal(VkFence *out_fence);
	~pal_vk_fence();
};

class pal_vk_descriptor_set_layout : public pal_descriptor_set_layout
{
	VkDescriptorSetLayout m_descriptor_set_layout;
	uint32_t m_dynamic_uniform_buffer_descriptor_count;
	uint32_t m_sampled_image_descriptor_count;
	uint32_t m_sampler_descriptor_count;
	uint32_t m_storage_image_descriptor_count;
	uint32_t m_top_level_acceleration_structure_descriptor_count;

public:
	pal_vk_descriptor_set_layout();
	void init(uint32_t descriptor_set_binding_count, PAL_DESCRIPTOR_SET_LAYOUT_BINDING const *descriptor_set_bindings, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_descriptor_set_layout();
	VkDescriptorSetLayout get_descriptor_set_layout() const;
	uint32_t get_dynamic_uniform_buffer_descriptor_count() const;
	uint32_t get_sampled_image_descriptor_count() const;
	uint32_t get_sampler_descriptor_count() const;
	uint32_t get_storage_image_descriptor_count() const;
	uint32_t get_top_level_acceleration_structure_descriptor_count() const;
};

class pal_vk_pipeline_layout : public pal_pipeline_layout
{
	VkPipelineLayout m_pipeline_layout;

public:
	pal_vk_pipeline_layout(VkPipelineLayout pipeline_layout);
	VkPipelineLayout get_pipeline_layout() const;
	void steal(VkPipelineLayout *out_pipeline_layout);
	~pal_vk_pipeline_layout();
};

class pal_vk_descriptor_set : public pal_descriptor_set
{
	VkDescriptorPool m_descriptor_pool;
	VkDescriptorSet m_descriptor_set;

public:
	pal_vk_descriptor_set();
	void init(pal_descriptor_set_layout const *descriptor_set_layout, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_descriptor_set();
	void write_descriptor(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, PAL_DESCRIPTOR_TYPE descriptor_type, uint32_t dst_binding, uint32_t dst_array_element, uint32_t src_descriptor_count, pal_upload_ring_buffer const *const *src_dynamic_uniform_buffers, uint32_t const *src_dynamic_uniform_buffers_range, pal_sampled_image const *const *src_sampled_images, pal_sampler const *const *src_samplers, pal_storage_image const *const *src_storage_images, pal_top_level_acceleration_structure const *const *src_top_level_acceleration_structures);
	VkDescriptorSet get_descriptor_set() const;
};

class pal_vk_render_pass : public pal_render_pass
{
	VkRenderPass m_render_pass;

public:
	pal_vk_render_pass(VkRenderPass render_pass);
	VkRenderPass get_render_pass() const;
	void steal(VkRenderPass *out_render_pass);
	~pal_vk_render_pass();
};

class pal_vk_graphics_pipeline : public pal_graphics_pipeline
{
	VkPipeline m_pipeline;

public:
	pal_vk_graphics_pipeline();
	void init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, pal_render_pass const *render_pass, pal_pipeline_layout const *pipeline_layout, size_t vertex_shader_module_code_size, void const *vertex_shader_module_code, size_t fragment_shader_module_code_size, void const *fragment_shader_module_code, uint32_t vertex_binding_count, PAL_GRAPHICS_PIPELINE_VERTEX_BINDING const *vertex_bindings, uint32_t vertex_attribute_count, PAL_GRAPHICS_PIPELINE_VERTEX_ATTRIBUTE const *vertex_attributes, bool depth_enable, PAL_GRAPHICS_PIPELINE_COMPARE_OPERATION depth_compare_operation);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_graphics_pipeline();
	VkPipeline get_pipeline() const;
};

class pal_vk_compute_pipeline : public pal_compute_pipeline
{
	VkPipeline m_pipeline;

public:
	pal_vk_compute_pipeline();
	void init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, pal_pipeline_layout const *pipeline_layout, size_t compute_shader_module_code_size, void const *compute_shader_module_code);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_compute_pipeline();
	VkPipeline get_pipeline() const;
};

class pal_vk_frame_buffer : public pal_frame_buffer
{
	VkFramebuffer m_frame_buffer;

public:
	pal_vk_frame_buffer(VkFramebuffer frame_buffer);
	VkFramebuffer get_frame_buffer() const;
	void steal(VkFramebuffer *out_frame_buffer);
	~pal_vk_frame_buffer();
};

class pal_vk_upload_ring_buffer : public pal_upload_ring_buffer
{
	VkBuffer m_buffer;
	VkDeviceMemory m_device_memory;
	void *m_memory_range_base;

public:
	pal_vk_upload_ring_buffer();
	void init(uint32_t upload_ring_buffer_memory_index, uint32_t size, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_upload_ring_buffer();
	VkBuffer get_buffer() const;
	void *get_memory_range_base() const override;
};

class pal_vk_staging_buffer : public pal_staging_buffer
{
	VkBuffer m_buffer;
	VkDeviceMemory m_device_memory;
	void *m_memory_range_base;

public:
	pal_vk_staging_buffer();
	void init(uint32_t staging_buffer_memory_index, uint32_t size, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	VkBuffer get_buffer() const;
	void *get_memory_range_base() const override;
	~pal_vk_staging_buffer();
};

class pal_vk_vertex_buffer : public pal_vertex_buffer
{
public:
	virtual VkBuffer get_buffer() const = 0;
};

class pal_vk_asset_vertex_position_buffer : public pal_asset_vertex_position_buffer, pal_vk_vertex_buffer
{
	VkBuffer m_buffer;
	VmaAllocation m_allocation;
	VkDeviceAddress m_device_memory_range_base;

public:
	pal_vk_asset_vertex_position_buffer();
	void init(bool support_ray_tracing, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VmaAllocator asset_allocator, uint32_t asset_vertex_position_buffer_memory_index, uint32_t size);
	void uninit(VmaAllocator asset_allocator);
	~pal_vk_asset_vertex_position_buffer();
	VkBuffer get_buffer() const override;
	VkDeviceAddress get_device_memory_range_base() const;
	pal_vertex_buffer const *get_vertex_buffer() const override;
};

class pal_vk_asset_vertex_varying_buffer : public pal_asset_vertex_varying_buffer, pal_vk_vertex_buffer
{
	VkBuffer m_buffer;
	VmaAllocation m_allocation;

public:
	pal_vk_asset_vertex_varying_buffer();
	void init(bool support_ray_tracing, VmaAllocator asset_allocator, uint32_t asset_vertex_varying_buffer_memory_index, uint32_t size);
	void uninit(VmaAllocator asset_allocator);
	~pal_vk_asset_vertex_varying_buffer();
	VkBuffer get_buffer() const override;
	pal_vertex_buffer const *get_vertex_buffer() const override;
};

class pal_vk_asset_index_buffer : public pal_asset_index_buffer
{
	VkBuffer m_buffer;
	VmaAllocation m_allocation;
	VkDeviceAddress m_device_memory_range_base;

public:
	pal_vk_asset_index_buffer();
	void init(bool support_ray_tracing, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VmaAllocator asset_allocator, uint32_t asset_index_buffer_memory_index, uint32_t size);
	void uninit(VmaAllocator asset_allocator);
	~pal_vk_asset_index_buffer();
	VkBuffer get_buffer() const;
	VkDeviceAddress get_device_memory_range_base() const;
};

class pal_vk_sampled_image : public pal_sampled_image
{
public:
	virtual VkImageView get_image_view() const = 0;
};

class pal_vk_color_attachment_image : public pal_color_attachment_image
{
public:
	virtual VkImageView get_image_view() const = 0;
};

class pal_vk_depth_stencil_attachment_image : public pal_depth_stencil_attachment_image
{
public:
	virtual VkImageView get_image_view() const = 0;
};

class pal_vk_storage_image : public pal_storage_image
{
public:
	virtual VkImage get_image() const = 0;
	virtual VkImageView get_image_view() const = 0;
};

class pal_vk_concrete_color_attachment_image : public pal_vk_color_attachment_image, pal_vk_sampled_image
{
	VkImage m_image;
	VkDeviceMemory m_device_memory;
	VkImageView m_image_view;

public:
	pal_vk_concrete_color_attachment_image();
	void init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, uint32_t color_transient_attachment_image_memory_index, uint32_t color_attachment_sampled_image_memory_index, PAL_COLOR_ATTACHMENT_IMAGE_FORMAT color_attachment_image_format, uint32_t width, uint32_t height, bool allow_sampled_image);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_concrete_color_attachment_image();
	VkImageView get_image_view() const override;
	pal_sampled_image const *get_sampled_image() const override;
};

class pal_vk_concrete_depth_stencil_attachment_image : public pal_vk_depth_stencil_attachment_image, pal_vk_sampled_image
{
	VkImage m_image;
	VkDeviceMemory m_device_memory;
	VkImageView m_image_view;

public:
	pal_vk_concrete_depth_stencil_attachment_image();
	void init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, uint32_t depth_transient_attachment_image_memory_index, uint32_t depth_attachment_sampled_image_memory_index, uint32_t depth_stencil_transient_attachment_image_memory_index, uint32_t depth_stencil_attachment_sampled_image_memory_index, PAL_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT depth_stencil_attachment_image_format, uint32_t width, uint32_t height, bool allow_sampled_image);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_concrete_depth_stencil_attachment_image();
	VkImageView get_image_view() const override;
	pal_sampled_image const *get_sampled_image() const override;
};

class pal_vk_concrete_storage_image : public pal_vk_storage_image, pal_vk_sampled_image
{
	VkImage m_image;
	VkDeviceMemory m_device_memory;
	VkImageView m_image_view;

public:
	pal_vk_concrete_storage_image();
	void init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, uint32_t storage_image_memory_index, PAL_STORAGE_IMAGE_FORMAT storage_image_format, uint32_t width, uint32_t height, bool allow_sampled_image);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_concrete_storage_image();
	VkImage get_image() const override;
	VkImageView get_image_view() const override;
	pal_sampled_image const *get_sampled_image() const override;
};

class pal_vk_asset_sampled_image : public pal_asset_sampled_image, pal_vk_sampled_image
{
	VkImage m_image;
	VmaAllocation m_allocation;
	VkImageView m_image_view;

public:
	pal_vk_asset_sampled_image();
	void init(VmaAllocator asset_allocator, uint32_t asset_sampled_image_memory_index, PFN_vkCreateImageView pfn_create_image_view, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, VkFormat format, uint32_t width, uint32_t height, uint32_t mip_levels);
	void uninit(VmaAllocator asset_allocator, PFN_vkDestroyImageView pfn_destroy_image_view, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_asset_sampled_image();
	VkImage get_image() const;
	VkImageView get_image_view() const override;
	pal_sampled_image const *get_sampled_image() const override;
};

class pal_vk_sampler : public pal_sampler
{
	VkSampler m_sampler;

public:
	pal_vk_sampler(VkSampler sampler);
	VkSampler get_sampler() const;
	void steal(VkSampler *out_sampler);
	~pal_vk_sampler();
};

class pal_vk_surface : public pal_surface
{
	VkSurfaceKHR m_surface;

public:
	pal_vk_surface(VkSurfaceKHR surface);
	VkSurfaceKHR get_surface() const;
	void steal(VkSurfaceKHR *out_surface);
	~pal_vk_surface();
};

class pal_vk_swap_chain_image_view : public pal_vk_color_attachment_image
{
	VkImageView m_image_view;

public:
	pal_vk_swap_chain_image_view(VkImageView image_view);
	VkImageView get_image_view() const override;
	pal_sampled_image const *get_sampled_image() const override;
	void steal(VkImageView *out_image_view);
	~pal_vk_swap_chain_image_view();
};

class pal_vk_swap_chain : public pal_swap_chain
{
	VkSwapchainKHR m_swap_chain;
	VkFormat m_image_format;
	uint32_t m_image_width;
	uint32_t m_image_height;
	uint32_t m_image_count;
	pal_vk_swap_chain_image_view *m_image_views;

public:
	pal_vk_swap_chain(VkSwapchainKHR swap_chain, VkFormat image_format, uint32_t image_width, uint32_t image_height, uint32_t image_count, pal_vk_swap_chain_image_view *image_views);
	VkSwapchainKHR get_swap_chain() const;
	PAL_COLOR_ATTACHMENT_IMAGE_FORMAT get_image_format() const override;
	uint32_t get_image_width() const override;
	uint32_t get_image_height() const override;
	uint32_t get_image_count() const override;
	pal_color_attachment_image const *get_image(uint32_t swap_chain_image_index) const override;
	void steal(VkSwapchainKHR *out_swap_chain, uint32_t *out_image_count, pal_vk_swap_chain_image_view **out_image_views);
	~pal_vk_swap_chain();
};

class pal_vk_scratch_buffer : public pal_scratch_buffer
{
	VkBuffer m_buffer;
	VkDeviceMemory m_device_memory;
	VkDeviceAddress m_device_memory_range_base;

public:
	pal_vk_scratch_buffer();
	void init(uint32_t scratch_buffer_memory_index, uint32_t size, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_scratch_buffer();
	VkBuffer get_buffer() const;
	VkDeviceAddress get_device_memory_range_base() const;
};

class pal_vk_staging_non_compacted_bottom_level_acceleration_structure_buffer : public pal_staging_non_compacted_bottom_level_acceleration_structure_buffer
{
	VkBuffer m_buffer;
	VkDeviceMemory m_device_memory;

public:
	pal_vk_staging_non_compacted_bottom_level_acceleration_structure_buffer();
	void init(uint32_t staging_non_compacted_bottom_level_acceleration_structure_buffer_memory_index, uint32_t size, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_staging_non_compacted_bottom_level_acceleration_structure_buffer();
	VkBuffer get_buffer() const;
};

class pal_vk_staging_non_compacted_bottom_level_acceleration_structure : public pal_staging_non_compacted_bottom_level_acceleration_structure
{
	VkAccelerationStructureKHR m_acceleration_structure;

public:
	pal_vk_staging_non_compacted_bottom_level_acceleration_structure();
	void init(pal_staging_non_compacted_bottom_level_acceleration_structure_buffer const *wrapped_staging_non_compacted_bottom_level_acceleration_structure_buffer, uint32_t offset, uint32_t size, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_staging_non_compacted_bottom_level_acceleration_structure();
	VkAccelerationStructureKHR get_acceleration_structure() const;
};

class pal_vk_compacted_bottom_level_acceleration_structure_size_query_pool : public pal_compacted_bottom_level_acceleration_structure_size_query_pool
{
	VkQueryPool m_query_pool;

public:
	pal_vk_compacted_bottom_level_acceleration_structure_size_query_pool();
	void init(uint32_t query_count, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_compacted_bottom_level_acceleration_structure_size_query_pool();
	VkQueryPool get_query_pool() const;
};

class pal_vk_asset_compacted_bottom_level_acceleration_structure : public pal_asset_compacted_bottom_level_acceleration_structure
{
	VkBuffer m_buffer;
	VmaAllocation m_allocation;
	VkAccelerationStructureKHR m_acceleration_structure;
	VkDeviceAddress m_device_memory_range_base;

public:
	pal_vk_asset_compacted_bottom_level_acceleration_structure();
	void init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, uint32_t asset_compacted_bottom_level_acceleration_structure_memory_index, VmaAllocator asset_allocator, uint32_t size);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, VmaAllocator asset_allocator);
	~pal_vk_asset_compacted_bottom_level_acceleration_structure();
	VkBuffer get_buffer() const;
	VkAccelerationStructureKHR get_acceleration_structure() const;
	VkDeviceAddress get_device_memory_range_base() const;
};

class pal_vk_top_level_acceleration_structure_instance_buffer : public pal_top_level_acceleration_structure_instance_buffer
{
	VkBuffer m_buffer;
	VkDeviceMemory m_device_memory;
	VkDeviceAddress m_device_memory_range_base;
	VkAccelerationStructureInstanceKHR *m_memory_range_base;

public:
	pal_vk_top_level_acceleration_structure_instance_buffer();
	void init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, uint32_t top_level_acceleration_structure_instance_buffer_memory_index, uint32_t instance_count);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks);
	~pal_vk_top_level_acceleration_structure_instance_buffer();
	void write_instance(uint32_t instance_index, float const transform_matrix[3][4], bool force_closest_hit, bool force_any_hit, bool disable_back_face_cull, bool front_ccw, pal_asset_compacted_bottom_level_acceleration_structure *asset_compacted_bottom_level_acceleration_structure);
	VkBuffer get_buffer() const;
	VkDeviceAddress get_device_memory_range_base() const;
};

class pal_vk_top_level_acceleration_structure : public pal_top_level_acceleration_structure
{
	VkBuffer m_buffer;
	VmaAllocation m_allocation;
	VkAccelerationStructureKHR m_acceleration_structure;

public:
	pal_vk_top_level_acceleration_structure();
	void init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, uint32_t top_level_acceleration_structure_memory_index, VmaAllocator asset_allocator, uint32_t size);
	void uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, VmaAllocator asset_allocator);
	~pal_vk_top_level_acceleration_structure();
	VkBuffer get_buffer() const;
	VkAccelerationStructureKHR get_acceleration_structure() const;
};

#endif