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

#include "brx_vk_device.h"
#include "brx_format.h"
#include "brx_vector.h"
#include <assert.h>

brx_vk_graphics_command_buffer::brx_vk_graphics_command_buffer()
	: m_command_pool(VK_NULL_HANDLE),
	  m_command_buffer(VK_NULL_HANDLE),
	  m_acquire_next_image_semaphore(VK_NULL_HANDLE),
	  m_queue_submit_semaphore(VK_NULL_HANDLE),
	  m_pfn_begin_command_buffer(NULL),
	  m_pfn_cmd_pipeline_barrier(NULL),
#ifndef NDEBUG
	  m_pfn_cmd_begin_debug_utils_label(NULL),
	  m_pfn_cmd_end_debug_utils_label(NULL),
#endif
	  m_pfn_cmd_begin_render_pass(NULL),
	  m_pfn_cmd_bind_pipeline(NULL),
	  m_pfn_cmd_set_view_port(NULL),
	  m_pfn_cmd_set_scissor(NULL),
	  m_pfn_cmd_bind_descriptor_sets(NULL),
	  m_pfn_cmd_bind_vertex_buffers(NULL),
	  m_pfn_cmd_bind_index_buffer(NULL),
	  m_pfn_cmd_draw(NULL),
	  m_pfn_cmd_draw_indexed(NULL),
	  m_pfn_cmd_end_render_pass(NULL),
	  m_pfn_cmd_dispatch(NULL),
	  m_pfn_cmd_build_acceleration_structure(NULL),
	  m_pfn_end_command_buffer(NULL)
{
}

void brx_vk_graphics_command_buffer::init(bool support_ray_tracing, bool has_dedicated_upload_queue, uint32_t graphics_queue_family_index, uint32_t upload_queue_family_index, PFN_vkGetInstanceProcAddr pfn_get_instance_proc_addr, VkInstance instance, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	this->m_support_ray_tracing = support_ray_tracing;

	this->m_has_dedicated_upload_queue = has_dedicated_upload_queue;
	this->m_graphics_queue_family_index = graphics_queue_family_index;
	this->m_upload_queue_family_index = upload_queue_family_index;

	PFN_vkCreateCommandPool const pfn_create_command_pool = reinterpret_cast<PFN_vkCreateCommandPool>(pfn_get_device_proc_addr(device, "vkCreateCommandPool"));
	assert(NULL != pfn_create_command_pool);
	PFN_vkAllocateCommandBuffers const pfn_allocate_command_buffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(pfn_get_device_proc_addr(device, "vkAllocateCommandBuffers"));
	assert(NULL != pfn_allocate_command_buffers);
	PFN_vkCreateSemaphore const pfn_create_semaphore = reinterpret_cast<PFN_vkCreateSemaphore>(pfn_get_device_proc_addr(device, "vkCreateSemaphore"));
	assert(NULL != pfn_create_semaphore);

	assert(VK_NULL_HANDLE == this->m_command_pool);
	VkCommandPoolCreateInfo const command_pool_create_info = {
		VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		NULL,
		0U,
		graphics_queue_family_index};
	VkResult const res_create_command_pool = pfn_create_command_pool(device, &command_pool_create_info, allocation_callbacks, &this->m_command_pool);
	assert(VK_SUCCESS == res_create_command_pool);

	assert(VK_NULL_HANDLE == this->m_command_buffer);
	VkCommandBufferAllocateInfo const command_buffer_allocate_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		NULL,
		this->m_command_pool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1U};
	VkResult const res_allocate_command_buffers = pfn_allocate_command_buffers(device, &command_buffer_allocate_info, &this->m_command_buffer);
	assert(VK_SUCCESS == res_allocate_command_buffers);

	assert(VK_NULL_HANDLE == this->m_acquire_next_image_semaphore);
	VkSemaphoreCreateInfo const acquire_next_image_semaphore_create_info = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		NULL,
		0U};
	VkResult const res_create_acquire_next_image_semaphore = pfn_create_semaphore(device, &acquire_next_image_semaphore_create_info, allocation_callbacks, &this->m_acquire_next_image_semaphore);
	assert(VK_SUCCESS == res_create_acquire_next_image_semaphore);

	assert(VK_NULL_HANDLE == this->m_queue_submit_semaphore);
	VkSemaphoreCreateInfo const queue_submit_semaphore_create_info = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		NULL,
		0U};
	VkResult const res_create_queue_submit_semaphore = pfn_create_semaphore(device, &queue_submit_semaphore_create_info, allocation_callbacks, &this->m_queue_submit_semaphore);
	assert(VK_SUCCESS == res_create_queue_submit_semaphore);

	assert(NULL == this->m_pfn_begin_command_buffer);
	this->m_pfn_begin_command_buffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(pfn_get_device_proc_addr(device, "vkBeginCommandBuffer"));
	assert(NULL == this->m_pfn_cmd_pipeline_barrier);
	this->m_pfn_cmd_pipeline_barrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(pfn_get_device_proc_addr(device, "vkCmdPipelineBarrier"));
#ifndef NDEBUG
	assert(NULL == this->m_pfn_cmd_begin_debug_utils_label);
	this->m_pfn_cmd_begin_debug_utils_label = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(pfn_get_instance_proc_addr(instance, "vkCmdBeginDebugUtilsLabelEXT"));
	assert(NULL == this->m_pfn_cmd_end_debug_utils_label);
	this->m_pfn_cmd_end_debug_utils_label = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(pfn_get_instance_proc_addr(instance, "vkCmdEndDebugUtilsLabelEXT"));
#endif
	assert(NULL == this->m_pfn_cmd_begin_render_pass);
	this->m_pfn_cmd_begin_render_pass = reinterpret_cast<PFN_vkCmdBeginRenderPass>(pfn_get_device_proc_addr(device, "vkCmdBeginRenderPass"));
	assert(NULL == this->m_pfn_cmd_bind_pipeline);
	this->m_pfn_cmd_bind_pipeline = reinterpret_cast<PFN_vkCmdBindPipeline>(pfn_get_device_proc_addr(device, "vkCmdBindPipeline"));
	assert(NULL == this->m_pfn_cmd_set_view_port);
	this->m_pfn_cmd_set_view_port = reinterpret_cast<PFN_vkCmdSetViewport>(pfn_get_device_proc_addr(device, "vkCmdSetViewport"));
	assert(NULL == this->m_pfn_cmd_set_scissor);
	this->m_pfn_cmd_set_scissor = reinterpret_cast<PFN_vkCmdSetScissor>(pfn_get_device_proc_addr(device, "vkCmdSetScissor"));
	assert(NULL == this->m_pfn_cmd_bind_descriptor_sets);
	this->m_pfn_cmd_bind_descriptor_sets = reinterpret_cast<PFN_vkCmdBindDescriptorSets>(pfn_get_device_proc_addr(device, "vkCmdBindDescriptorSets"));
	assert(NULL == this->m_pfn_cmd_bind_vertex_buffers);
	this->m_pfn_cmd_bind_vertex_buffers = reinterpret_cast<PFN_vkCmdBindVertexBuffers>(pfn_get_device_proc_addr(device, "vkCmdBindVertexBuffers"));
	assert(NULL == this->m_pfn_cmd_bind_index_buffer);
	this->m_pfn_cmd_bind_index_buffer = reinterpret_cast<PFN_vkCmdBindIndexBuffer>(pfn_get_device_proc_addr(device, "vkCmdBindIndexBuffer"));
	assert(NULL == this->m_pfn_cmd_draw);
	this->m_pfn_cmd_draw = reinterpret_cast<PFN_vkCmdDraw>(pfn_get_device_proc_addr(device, "vkCmdDraw"));
	assert(NULL == this->m_pfn_cmd_draw_indexed);
	this->m_pfn_cmd_draw_indexed = reinterpret_cast<PFN_vkCmdDrawIndexed>(pfn_get_device_proc_addr(device, "vkCmdDrawIndexed"));
	assert(NULL == this->m_pfn_cmd_end_render_pass);
	this->m_pfn_cmd_end_render_pass = reinterpret_cast<PFN_vkCmdEndRenderPass>(pfn_get_device_proc_addr(device, "vkCmdEndRenderPass"));
	assert(NULL == this->m_pfn_cmd_dispatch);
	this->m_pfn_cmd_dispatch = reinterpret_cast<PFN_vkCmdDispatch>(pfn_get_device_proc_addr(device, "vkCmdDispatch"));
	assert(NULL == this->m_pfn_cmd_build_acceleration_structure);
	if (this->m_support_ray_tracing)
	{
		this->m_pfn_cmd_build_acceleration_structure = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(pfn_get_device_proc_addr(device, "vkCmdBuildAccelerationStructuresKHR"));
	}
	assert(NULL == this->m_pfn_end_command_buffer);
	this->m_pfn_end_command_buffer = reinterpret_cast<PFN_vkEndCommandBuffer>(pfn_get_device_proc_addr(device, "vkEndCommandBuffer"));
}

void brx_vk_graphics_command_buffer::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkDestroyCommandPool const pfn_destroy_command_pool = reinterpret_cast<PFN_vkDestroyCommandPool>(pfn_get_device_proc_addr(device, "vkDestroyCommandPool"));
	assert(NULL != pfn_destroy_command_pool);
	PFN_vkFreeCommandBuffers const pfn_free_command_buffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(pfn_get_device_proc_addr(device, "vkFreeCommandBuffers"));
	assert(NULL != pfn_free_command_buffers);
	PFN_vkDestroySemaphore const pfn_destroy_semaphore = reinterpret_cast<PFN_vkDestroySemaphore>(pfn_get_device_proc_addr(device, "vkDestroySemaphore"));
	assert(NULL != pfn_destroy_semaphore);

	assert(VK_NULL_HANDLE != this->m_command_buffer);
	pfn_free_command_buffers(device, this->m_command_pool, 1U, &this->m_command_buffer);
	this->m_command_buffer = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_command_pool);
	pfn_destroy_command_pool(device, this->m_command_pool, allocation_callbacks);
	this->m_command_pool = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_acquire_next_image_semaphore);
	pfn_destroy_semaphore(device, this->m_acquire_next_image_semaphore, allocation_callbacks);
	this->m_acquire_next_image_semaphore = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_queue_submit_semaphore);

	pfn_destroy_semaphore(device, this->m_queue_submit_semaphore, allocation_callbacks);
	this->m_queue_submit_semaphore = VK_NULL_HANDLE;
}

brx_vk_graphics_command_buffer::~brx_vk_graphics_command_buffer()
{
	assert(VK_NULL_HANDLE == this->m_command_pool);
	assert(VK_NULL_HANDLE == this->m_command_buffer);
	assert(VK_NULL_HANDLE == this->m_acquire_next_image_semaphore);
	assert(VK_NULL_HANDLE == this->m_queue_submit_semaphore);
}

VkCommandPool brx_vk_graphics_command_buffer::get_command_pool() const
{
	return this->m_command_pool;
}

VkCommandBuffer brx_vk_graphics_command_buffer::get_command_buffer() const
{
	return this->m_command_buffer;
}

VkSemaphore brx_vk_graphics_command_buffer::get_acquire_next_image_semaphore() const
{
	return this->m_acquire_next_image_semaphore;
}

VkSemaphore brx_vk_graphics_command_buffer::get_queue_submit_semaphore() const
{
	return this->m_queue_submit_semaphore;
}

void brx_vk_graphics_command_buffer::begin()
{
	VkCommandBufferBeginInfo command_buffer_begin_info = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		NULL};
	VkResult res_begin_command_buffer = this->m_pfn_begin_command_buffer(this->m_command_buffer, &command_buffer_begin_info);
	assert(VK_SUCCESS == res_begin_command_buffer);
}

void brx_vk_graphics_command_buffer::acquire_asset_vertex_position_buffer(brx_asset_vertex_position_buffer *wrapped_asset_vertex_position_buffer)
{
	assert(NULL != wrapped_asset_vertex_position_buffer);
	VkBuffer const asset_buffer = static_cast<brx_vk_asset_vertex_position_buffer *>(wrapped_asset_vertex_position_buffer)->get_buffer();

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			// acquire operation
			VkBufferMemoryBarrier const acquire_barrier = {

				VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
				NULL,
				0U,
				(!this->m_support_ray_tracing) ? VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT : static_cast<VkAccessFlags>(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_SHADER_READ_BIT),
				this->m_upload_queue_family_index,
				this->m_graphics_queue_family_index,
				asset_buffer,
				0U,
				VK_WHOLE_SIZE};

			this->m_pfn_cmd_pipeline_barrier(this->m_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, (!this->m_support_ray_tracing) ? VK_PIPELINE_STAGE_VERTEX_INPUT_BIT : static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | g_graphics_queue_family_all_supported_shader_stages), 0U, 0U, NULL, 1U, &acquire_barrier, 0U, NULL);
		}
		else
		{
			// do nothing
		}
	}
	else
	{
		// do nothing
	}
}

void brx_vk_graphics_command_buffer::acquire_asset_vertex_varying_buffer(brx_asset_vertex_varying_buffer *wrapped_asset_vertex_varying_buffer)
{
	assert(NULL != wrapped_asset_vertex_varying_buffer);
	VkBuffer const asset_buffer = static_cast<brx_vk_asset_vertex_varying_buffer *>(wrapped_asset_vertex_varying_buffer)->get_buffer();

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			// acquire operation
			VkBufferMemoryBarrier const acquire_barrier = {

				VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
				NULL,
				0U,
				(!this->m_support_ray_tracing) ? VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT : static_cast<VkAccessFlags>(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_SHADER_READ_BIT),
				this->m_upload_queue_family_index,
				this->m_graphics_queue_family_index,
				asset_buffer,
				0U,
				VK_WHOLE_SIZE};

			this->m_pfn_cmd_pipeline_barrier(this->m_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, (!this->m_support_ray_tracing) ? VK_PIPELINE_STAGE_VERTEX_INPUT_BIT : static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | g_graphics_queue_family_all_supported_shader_stages), 0U, 0U, NULL, 1U, &acquire_barrier, 0U, NULL);
		}
		else
		{
			// do nothing
		}
	}
	else
	{
		// do nothing
	}
}

void brx_vk_graphics_command_buffer::acquire_asset_index_buffer(brx_asset_index_buffer *wrapped_asset_index_buffer)
{
	assert(NULL != wrapped_asset_index_buffer);
	VkBuffer const asset_buffer = static_cast<brx_vk_asset_index_buffer *>(wrapped_asset_index_buffer)->get_buffer();

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			// acquire operation
			VkBufferMemoryBarrier const acquire_barrier = {

				VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
				NULL,
				0U,
				(!this->m_support_ray_tracing) ? VK_ACCESS_INDEX_READ_BIT : static_cast<VkAccessFlags>(VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_SHADER_READ_BIT),
				this->m_upload_queue_family_index,
				this->m_graphics_queue_family_index,
				asset_buffer,
				0U,
				VK_WHOLE_SIZE};

			this->m_pfn_cmd_pipeline_barrier(this->m_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, (!this->m_support_ray_tracing) ? VK_PIPELINE_STAGE_VERTEX_INPUT_BIT : static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | g_graphics_queue_family_all_supported_shader_stages), 0U, 0U, NULL, 1U, &acquire_barrier, 0U, NULL);
		}
		else
		{
			// do nothing
		}
	}
	else
	{
		// do nothing
	}
}

void brx_vk_graphics_command_buffer::acquire_asset_sampled_image(brx_asset_sampled_image *wrapped_asset_sampled_image, uint32_t dst_mip_level)
{
	assert(NULL != wrapped_asset_sampled_image);
	VkImage const asset_sampled_image = static_cast<brx_vk_asset_sampled_image *>(wrapped_asset_sampled_image)->get_image();

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			// acquire operation
			VkImageSubresourceRange const load_subresource_range = {VK_IMAGE_ASPECT_COLOR_BIT, dst_mip_level, 1U, 0U, 1U};
			VkImageMemoryBarrier const acquire_barrier = {
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				NULL,
				0,
				VK_ACCESS_SHADER_READ_BIT,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				this->m_upload_queue_family_index,
				this->m_graphics_queue_family_index,
				asset_sampled_image,
				load_subresource_range};
			this->m_pfn_cmd_pipeline_barrier(this->m_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, g_graphics_queue_family_all_supported_shader_stages, 0U, 0U, NULL, 0U, NULL, 1U, &acquire_barrier);
		}
		else
		{
			// do nothing
		}
	}
	else
	{
		// do nothing
	}
}

void brx_vk_graphics_command_buffer::acquire_asset_compacted_bottom_level_acceleration_structure(brx_asset_compacted_bottom_level_acceleration_structure *wrapped_asset_compacted_bottom_level_acceleration_structure)
{
	VkBuffer const asset_buffer = static_cast<brx_vk_asset_compacted_bottom_level_acceleration_structure *>(wrapped_asset_compacted_bottom_level_acceleration_structure)->get_buffer();

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			// acquire operation
			VkBufferMemoryBarrier acquire_barrier = {
				VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
				NULL,
				0,
				VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
				this->m_upload_queue_family_index,
				this->m_graphics_queue_family_index,
				asset_buffer,
				0U,
				VK_WHOLE_SIZE};
			this->m_pfn_cmd_pipeline_barrier(this->m_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, 0U, 0U, NULL, 1U, &acquire_barrier, 0U, NULL);
		}
		else
		{
			// do nothing
		}
	}
	else
	{
		// do nothing
	}
}

void brx_vk_graphics_command_buffer::begin_debug_utils_label(char const *label_name)
{
#ifndef NDEBUG
	VkDebugUtilsLabelEXT debug_utils_label = {VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT, NULL, label_name, {1.0F, 1.0F, 1.0F, 1.0F}};
	this->m_pfn_cmd_begin_debug_utils_label(this->m_command_buffer, &debug_utils_label);
#endif
}

void brx_vk_graphics_command_buffer::end_debug_utils_label()
{
#ifndef NDEBUG
	this->m_pfn_cmd_end_debug_utils_label(this->m_command_buffer);
#endif
}

void brx_vk_graphics_command_buffer::begin_render_pass(brx_render_pass const *brx_render_pass, brx_frame_buffer const *brx_frame_buffer, uint32_t width, uint32_t height, uint32_t color_clear_value_count, float const (*color_clear_values)[4], float const *depth_clear_value, uint8_t const *stencil_clear_value)
{
	assert(NULL != brx_render_pass);
	assert(NULL != brx_frame_buffer);
	VkRenderPass render_pass = static_cast<brx_vk_render_pass const *>(brx_render_pass)->get_render_pass();
	VkFramebuffer frame_buffer = static_cast<brx_vk_frame_buffer const *>(brx_frame_buffer)->get_frame_buffer();

	constexpr uint32_t const max_color_clear_value_count = 8U;
	assert(color_clear_value_count < max_color_clear_value_count);
	color_clear_value_count = (color_clear_value_count < max_color_clear_value_count) ? color_clear_value_count : max_color_clear_value_count;

	VkClearValue clear_values[max_color_clear_value_count + 1U];
	for (uint32_t color_clear_value_index = 0U; color_clear_value_index < color_clear_value_count; ++color_clear_value_index)
	{
		clear_values[color_clear_value_index].color.float32[0] = color_clear_values[color_clear_value_index][0];
		clear_values[color_clear_value_index].color.float32[1] = color_clear_values[color_clear_value_index][1];
		clear_values[color_clear_value_index].color.float32[2] = color_clear_values[color_clear_value_index][2];
		clear_values[color_clear_value_index].color.float32[3] = color_clear_values[color_clear_value_index][3];
	}

	if (NULL != depth_clear_value)
	{
		clear_values[color_clear_value_count].depthStencil.depth = (*depth_clear_value);
		if (NULL != stencil_clear_value)
		{
			clear_values[color_clear_value_count].depthStencil.stencil = (*stencil_clear_value);
		}
	}

	VkRenderPassBeginInfo render_pass_begin_info = {
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		NULL,
		render_pass,
		frame_buffer,
		{{0U, 0U}, {width, height}},
		color_clear_value_count + ((NULL != depth_clear_value) ? 1U : 0U),
		clear_values,
	};

	this->m_pfn_cmd_begin_render_pass(this->m_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void brx_vk_graphics_command_buffer::bind_graphics_pipeline(brx_graphics_pipeline const *wrapped_graphics_pipeline)
{
	assert(NULL != wrapped_graphics_pipeline);
	VkPipeline const graphics_pipeline = static_cast<brx_vk_graphics_pipeline const *>(wrapped_graphics_pipeline)->get_pipeline();

	this->m_pfn_cmd_bind_pipeline(this->m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
}

void brx_vk_graphics_command_buffer::set_view_port(uint32_t width, uint32_t height)
{
	// Vulkan Flip Y
	float const view_port_y = static_cast<float>(height);
	float const view_port_height = -static_cast<float>(height);

	VkViewport view_port = {0.0F, view_port_y, static_cast<float>(width), view_port_height, 0.0F, 1.0F};
	this->m_pfn_cmd_set_view_port(this->m_command_buffer, 0U, 1U, &view_port);
}

void brx_vk_graphics_command_buffer::set_scissor(uint32_t width, uint32_t height)
{
	VkRect2D scissor = {{0, 0}, {width, height}};
	this->m_pfn_cmd_set_scissor(this->m_command_buffer, 0U, 1U, &scissor);
}

void brx_vk_graphics_command_buffer::bind_graphics_descriptor_sets(brx_pipeline_layout const *wrapped_pipeline_layout, uint32_t descriptor_set_count, brx_descriptor_set const *const *wrapped_descriptor_sets, uint32_t dynamic_offet_count, uint32_t const *dynamic_offsets)
{
	assert(NULL != wrapped_pipeline_layout);
	assert(NULL != wrapped_descriptor_sets);
	VkPipelineLayout const pipeline_layout = static_cast<brx_vk_pipeline_layout const *>(wrapped_pipeline_layout)->get_pipeline_layout();

	brx_vector<VkDescriptorSet> descriptor_sets(static_cast<size_t>(descriptor_set_count));
	for (uint32_t descriptor_set_index = 0U; descriptor_set_index < descriptor_set_count; ++descriptor_set_index)
	{
		assert(NULL != wrapped_descriptor_sets[descriptor_set_index]);
		descriptor_sets[descriptor_set_index] = static_cast<brx_vk_descriptor_set const *>(wrapped_descriptor_sets[descriptor_set_index])->get_descriptor_set();
	}

	this->m_pfn_cmd_bind_descriptor_sets(this->m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0U, descriptor_set_count, &descriptor_sets[0], dynamic_offet_count, dynamic_offsets);
}

void brx_vk_graphics_command_buffer::bind_vertex_buffers(uint32_t vertex_buffer_count, brx_vertex_buffer const *const *vertex_buffers)
{
	assert(NULL != vertex_buffers);

	constexpr uint32_t const max_vertex_buffer_count = 16U;
	assert(vertex_buffer_count < max_vertex_buffer_count);
	vertex_buffer_count = (vertex_buffer_count < max_vertex_buffer_count) ? vertex_buffer_count : max_vertex_buffer_count;

	VkBuffer buffers[max_vertex_buffer_count];
	VkDeviceSize offsets[max_vertex_buffer_count];
	for (uint32_t vertex_buffer_index = 0U; vertex_buffer_index < vertex_buffer_count; ++vertex_buffer_index)
	{
		assert(NULL != vertex_buffers[vertex_buffer_index]);
		buffers[vertex_buffer_index] = static_cast<brx_vk_vertex_buffer const *>(vertex_buffers[vertex_buffer_index])->get_buffer();
		offsets[vertex_buffer_index] = 0U;
	}

	this->m_pfn_cmd_bind_vertex_buffers(this->m_command_buffer, 0U, vertex_buffer_count, buffers, offsets);
}

void brx_vk_graphics_command_buffer::draw(uint32_t vertex_count, uint32_t instance_count)
{
	this->m_pfn_cmd_draw(this->m_command_buffer, vertex_count, instance_count, 0U, 0U);
}

void brx_vk_graphics_command_buffer::draw_index(brx_index_buffer const *wrapped_index_buffer, BRX_GRAPHICS_PIPELINE_INDEX_TYPE wrapped_index_type, uint32_t index_count, uint32_t instance_count)
{
	assert(NULL != wrapped_index_buffer);
	VkBuffer const index_buffer = static_cast<brx_vk_index_buffer const *>(wrapped_index_buffer)->get_buffer();

	VkIndexType index_type;
	switch (wrapped_index_type)
	{
	case BRX_GRAPHICS_PIPELINE_INDEX_TYPE_UINT32:
		index_type = VK_INDEX_TYPE_UINT32;
		break;
	case BRX_GRAPHICS_PIPELINE_INDEX_TYPE_UINT16:
		index_type = VK_INDEX_TYPE_UINT16;
		break;
	default:
		assert(false);
		index_type = static_cast<VkIndexType>(-1);
	}

	this->m_pfn_cmd_bind_index_buffer(this->m_command_buffer, index_buffer, 0U, index_type);

	this->m_pfn_cmd_draw_indexed(this->m_command_buffer, index_count, instance_count, 0U, 0U, 0U);
}

void brx_vk_graphics_command_buffer::end_render_pass()
{
	this->m_pfn_cmd_end_render_pass(this->m_command_buffer);
}

void brx_vk_graphics_command_buffer::compute_pass_load_storage_image(brx_storage_image const *wrapped_storage_image, BRX_STORAGE_IMAGE_LOAD_OPERATION load_operation)
{
	assert(BRX_STORAGE_IMAGE_LOAD_OPERATION_DONT_CARE == load_operation);

	// load operation
	VkImage const load_image = static_cast<brx_vk_storage_image const *>(wrapped_storage_image)->get_image();
	VkImageSubresourceRange const load_subresource_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0U, 1U, 0U, 1U};
	VkImageMemoryBarrier const load_barrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		0,
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		load_image,
		load_subresource_range};
	this->m_pfn_cmd_pipeline_barrier(this->m_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0U, 0U, NULL, 0U, NULL, 1U, &load_barrier);
}

void brx_vk_graphics_command_buffer::bind_compute_pipeline(brx_compute_pipeline const *wrapped_compute_pipeline)
{
	assert(NULL != wrapped_compute_pipeline);
	VkPipeline const compute_pipeline = static_cast<brx_vk_compute_pipeline const *>(wrapped_compute_pipeline)->get_pipeline();

	this->m_pfn_cmd_bind_pipeline(this->m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline);
}

void brx_vk_graphics_command_buffer::bind_compute_descriptor_sets(brx_pipeline_layout const *wrapped_pipeline_layout, uint32_t descriptor_set_count, brx_descriptor_set const *const *wrapped_descriptor_sets, uint32_t dynamic_offet_count, uint32_t const *dynamic_offsets)
{
	assert(NULL != wrapped_pipeline_layout);
	assert(NULL != wrapped_descriptor_sets);
	VkPipelineLayout const pipeline_layout = static_cast<brx_vk_pipeline_layout const *>(wrapped_pipeline_layout)->get_pipeline_layout();

	brx_vector<VkDescriptorSet> descriptor_sets(static_cast<size_t>(descriptor_set_count));
	for (uint32_t descriptor_set_index = 0U; descriptor_set_index < descriptor_set_count; ++descriptor_set_index)
	{
		assert(NULL != wrapped_descriptor_sets[descriptor_set_index]);
		descriptor_sets[descriptor_set_index] = static_cast<brx_vk_descriptor_set const *>(wrapped_descriptor_sets[descriptor_set_index])->get_descriptor_set();
	}

	this->m_pfn_cmd_bind_descriptor_sets(this->m_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0U, descriptor_set_count, &descriptor_sets[0], dynamic_offet_count, dynamic_offsets);
}

void brx_vk_graphics_command_buffer::dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z)
{
	this->m_pfn_cmd_dispatch(this->m_command_buffer, group_count_x, group_count_y, group_count_z);
}

void brx_vk_graphics_command_buffer::compute_pass_store_storage_image(brx_storage_image const *wrapped_storage_image, BRX_STORAGE_IMAGE_STORE_OPERATION store_operation)
{
	assert(BRX_STORAGE_IMAGE_STORE_OPERATION_FLUSH_FOR_SAMPLED_IMAGE == store_operation);

	// store operation
	VkImage const store_image = static_cast<brx_vk_storage_image const *>(wrapped_storage_image)->get_image();
	VkImageSubresourceRange const store_subresource_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0U, 1U, 0U, 1U};
	VkImageMemoryBarrier const store_barrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		store_image,
		store_subresource_range};
	this->m_pfn_cmd_pipeline_barrier(this->m_command_buffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, g_graphics_queue_family_all_supported_shader_stages, 0U, 0U, NULL, 0U, NULL, 1U, &store_barrier);
}

void brx_vk_graphics_command_buffer::build_top_level_acceleration_structure(brx_top_level_acceleration_structure *wrapped_top_level_acceleration_structure, uint32_t top_level_acceleration_structure_instance_count, brx_top_level_acceleration_structure_instance_upload_buffer *wrapped_top_level_acceleration_structure_instance_upload_buffer, brx_scratch_buffer *wrapped_scratch_buffer)
{
	assert(NULL != wrapped_top_level_acceleration_structure);
	VkAccelerationStructureKHR const destination_acceleration_structure = static_cast<brx_vk_top_level_acceleration_structure *>(wrapped_top_level_acceleration_structure)->get_acceleration_structure();
	VkBuffer const destination_buffer = static_cast<brx_vk_top_level_acceleration_structure *>(wrapped_top_level_acceleration_structure)->get_buffer();

	assert(NULL != wrapped_top_level_acceleration_structure_instance_upload_buffer);
	VkDeviceAddress const top_level_acceleration_structure_instance_upload_buffer_device_memory_range_base = static_cast<brx_vk_top_level_acceleration_structure_instance_upload_buffer *>(wrapped_top_level_acceleration_structure_instance_upload_buffer)->get_device_memory_range_base();

	assert(NULL != wrapped_scratch_buffer);
	VkDeviceAddress const scratch_buffer_device_memory_range_base = static_cast<brx_vk_scratch_buffer *>(wrapped_scratch_buffer)->get_device_memory_range_base();

	VkAccelerationStructureGeometryKHR const acceleration_structure_geometry = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		NULL,
		VK_GEOMETRY_TYPE_INSTANCES_KHR,
		{.instances =
			 {
				 VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
				 NULL,
				 VK_FALSE,
				 top_level_acceleration_structure_instance_upload_buffer_device_memory_range_base,
			 }}};

	VkAccelerationStructureBuildGeometryInfoKHR const acceleration_structure_build_geometry_info = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		NULL,
		VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR,
		VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		VK_NULL_HANDLE,
		destination_acceleration_structure,
		1U,
		&acceleration_structure_geometry,
		NULL,
		{.deviceAddress = scratch_buffer_device_memory_range_base}};

	VkAccelerationStructureBuildRangeInfoKHR const acceleration_structure_build_range_info = {
		top_level_acceleration_structure_instance_count,
		0U,
		0U,
		0U};

	VkAccelerationStructureBuildRangeInfoKHR const *const p_build_range_infos = &acceleration_structure_build_range_info;

	this->m_pfn_cmd_build_acceleration_structure(this->m_command_buffer, 1U, &acceleration_structure_build_geometry_info, &p_build_range_infos);

	static_cast<brx_vk_top_level_acceleration_structure *>(wrapped_top_level_acceleration_structure)->set_instance_count(top_level_acceleration_structure_instance_count);

	VkBufferMemoryBarrier const release_barrier = {
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
		VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		destination_buffer,
		0U,
		VK_WHOLE_SIZE};
	this->m_pfn_cmd_pipeline_barrier(this->m_command_buffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0U, 0U, NULL, 1U, &release_barrier, 0U, NULL);
}

void brx_vk_graphics_command_buffer::update_top_level_acceleration_structure(brx_top_level_acceleration_structure *wrapped_top_level_acceleration_structure, brx_top_level_acceleration_structure_instance_upload_buffer *wrapped_top_level_acceleration_structure_instance_upload_buffer, brx_scratch_buffer *wrapped_scratch_buffer)
{
	assert(NULL != wrapped_top_level_acceleration_structure);
	VkAccelerationStructureKHR const destination_acceleration_structure = static_cast<brx_vk_top_level_acceleration_structure *>(wrapped_top_level_acceleration_structure)->get_acceleration_structure();

	uint32_t const top_level_acceleration_structure_instance_count = static_cast<brx_vk_top_level_acceleration_structure *>(wrapped_top_level_acceleration_structure)->get_instance_count();

	assert(NULL != wrapped_top_level_acceleration_structure_instance_upload_buffer);
	VkDeviceAddress const top_level_acceleration_structure_instance_upload_buffer_device_memory_range_base = static_cast<brx_vk_top_level_acceleration_structure_instance_upload_buffer *>(wrapped_top_level_acceleration_structure_instance_upload_buffer)->get_device_memory_range_base();

	VkAccelerationStructureGeometryKHR const acceleration_structure_geometry = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		NULL,
		VK_GEOMETRY_TYPE_INSTANCES_KHR,
		{.instances =
			 {
				 VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
				 NULL,
				 VK_FALSE,
				 top_level_acceleration_structure_instance_upload_buffer_device_memory_range_base,
			 }}};

	assert(NULL != wrapped_scratch_buffer);
	VkDeviceAddress const scratch_buffer_device_memory_range_base = static_cast<brx_vk_scratch_buffer *>(wrapped_scratch_buffer)->get_device_memory_range_base();

	VkAccelerationStructureBuildGeometryInfoKHR const acceleration_structure_build_geometry_info = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		NULL,
		VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR,
		VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR,
		destination_acceleration_structure,
		destination_acceleration_structure,
		1U,
		&acceleration_structure_geometry,
		NULL,
		{.deviceAddress = scratch_buffer_device_memory_range_base}};

	VkAccelerationStructureBuildRangeInfoKHR const acceleration_structure_build_range_info = {
		top_level_acceleration_structure_instance_count,
		0U,
		0U,
		0U};

	VkAccelerationStructureBuildRangeInfoKHR const *const p_build_range_infos = &acceleration_structure_build_range_info;

	this->m_pfn_cmd_build_acceleration_structure(this->m_command_buffer, 1U, &acceleration_structure_build_geometry_info, &p_build_range_infos);
}

void brx_vk_graphics_command_buffer::acceleration_structure_pass_store_top_level(brx_top_level_acceleration_structure *wrapped_top_level_acceleration_structure)
{
	assert(NULL != wrapped_top_level_acceleration_structure);
	VkBuffer const destination_buffer = static_cast<brx_vk_top_level_acceleration_structure *>(wrapped_top_level_acceleration_structure)->get_buffer();

	VkBufferMemoryBarrier const release_barrier = {
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
		VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		destination_buffer,
		0U,
		VK_WHOLE_SIZE};
	this->m_pfn_cmd_pipeline_barrier(this->m_command_buffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0U, 0U, NULL, 1U, &release_barrier, 0U, NULL);
}

void brx_vk_graphics_command_buffer::end()
{
	VkResult res_end_command_buffer = this->m_pfn_end_command_buffer(this->m_command_buffer);
	assert(VK_SUCCESS == res_end_command_buffer);
}

brx_vk_upload_command_buffer::brx_vk_upload_command_buffer()
	: m_graphics_command_pool(VK_NULL_HANDLE),
	  m_graphics_command_buffer(VK_NULL_HANDLE),
	  m_upload_command_pool(VK_NULL_HANDLE),
	  m_upload_command_buffer(VK_NULL_HANDLE),
	  m_upload_queue_submit_semaphore(VK_NULL_HANDLE),
	  m_pfn_begin_command_buffer(NULL),
	  m_pfn_cmd_pipeline_barrier(NULL),
	  m_pfn_cmd_copy_buffer(NULL),
	  m_pfn_cmd_copy_buffer_to_image(NULL),
	  m_pfn_cmd_build_acceleration_structure(NULL),
	  m_pfn_cmd_reset_query_pool(NULL),
	  m_pfn_cmd_write_acceleration_structures_properties(NULL),
	  m_pfn_cmd_copy_acceleration_structure(NULL),
	  m_pfn_end_command_buffer(NULL)
{
}

void brx_vk_upload_command_buffer::init(bool support_ray_tracing, bool has_dedicated_upload_queue, uint32_t graphics_queue_family_index, uint32_t upload_queue_family_index, PFN_vkGetInstanceProcAddr pfn_get_instance_proc_addr, VkInstance instance, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	this->m_support_ray_tracing = support_ray_tracing;
	this->m_has_dedicated_upload_queue = has_dedicated_upload_queue;
	this->m_graphics_queue_family_index = graphics_queue_family_index;
	this->m_upload_queue_family_index = upload_queue_family_index;

	assert(VK_NULL_HANDLE == this->m_upload_command_pool);
	assert(VK_NULL_HANDLE == this->m_upload_command_buffer);
	assert(VK_NULL_HANDLE == this->m_graphics_command_pool);
	assert(VK_NULL_HANDLE == this->m_graphics_command_buffer);
	assert(VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);

	PFN_vkCreateCommandPool const pfn_create_command_pool = reinterpret_cast<PFN_vkCreateCommandPool>(pfn_get_device_proc_addr(device, "vkCreateCommandPool"));
	assert(NULL != pfn_create_command_pool);
	PFN_vkAllocateCommandBuffers const pfn_allocate_command_buffers = reinterpret_cast<PFN_vkAllocateCommandBuffers>(pfn_get_device_proc_addr(device, "vkAllocateCommandBuffers"));
	assert(NULL != pfn_allocate_command_buffers);
	PFN_vkCreateSemaphore const pfn_create_semaphore = reinterpret_cast<PFN_vkCreateSemaphore>(pfn_get_device_proc_addr(device, "vkCreateSemaphore"));
	assert(NULL != pfn_create_semaphore);

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_graphics_queue_family_index != this->m_upload_queue_family_index)
		{
			VkCommandPoolCreateInfo upload_command_pool_create_info = {
				VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				NULL,
				0U,
				this->m_upload_queue_family_index};
			VkResult res_upload_create_command_pool = pfn_create_command_pool(device, &upload_command_pool_create_info, allocation_callbacks, &this->m_upload_command_pool);
			assert(VK_SUCCESS == res_upload_create_command_pool);

			VkCommandBufferAllocateInfo upload_command_buffer_allocate_info = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				NULL,
				this->m_upload_command_pool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1U};
			VkResult res_upload_allocate_command_buffers = pfn_allocate_command_buffers(device, &upload_command_buffer_allocate_info, &this->m_upload_command_buffer);
			assert(VK_SUCCESS == res_upload_allocate_command_buffers);

			VkSemaphoreCreateInfo upload_queue_submit_semaphore_create_info = {
				VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				NULL,
				0U};
			VkResult res_create_semaphore = pfn_create_semaphore(device, &upload_queue_submit_semaphore_create_info, allocation_callbacks, &this->m_upload_queue_submit_semaphore);
			assert(VK_SUCCESS == res_create_semaphore);
		}
		else
		{
			VkCommandPoolCreateInfo upload_command_pool_create_info = {
				VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				NULL,
				0U,
				this->m_upload_queue_family_index};
			VkResult res_upload_create_command_pool = pfn_create_command_pool(device, &upload_command_pool_create_info, allocation_callbacks, &this->m_upload_command_pool);
			assert(VK_SUCCESS == res_upload_create_command_pool);

			VkCommandBufferAllocateInfo upload_command_buffer_allocate_info = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				NULL,
				this->m_upload_command_pool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1U};
			VkResult res_upload_allocate_command_buffers = pfn_allocate_command_buffers(device, &upload_command_buffer_allocate_info, &this->m_upload_command_buffer);
			assert(VK_SUCCESS == res_upload_allocate_command_buffers);

			VkSemaphoreCreateInfo upload_queue_submit_semaphore_create_info = {
				VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				NULL,
				0U};
			VkResult res_create_semaphore = pfn_create_semaphore(device, &upload_queue_submit_semaphore_create_info, allocation_callbacks, &this->m_upload_queue_submit_semaphore);
			assert(VK_SUCCESS == res_create_semaphore);
		}
	}
	else
	{
		VkCommandPoolCreateInfo graphics_command_pool_create_info = {
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			NULL,
			0U,
			this->m_graphics_queue_family_index};
		VkResult res_graphics_create_command_pool = pfn_create_command_pool(device, &graphics_command_pool_create_info, allocation_callbacks, &this->m_graphics_command_pool);
		assert(VK_SUCCESS == res_graphics_create_command_pool);

		VkCommandBufferAllocateInfo graphics_command_buffer_allocate_info = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			NULL,
			this->m_graphics_command_pool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1U};
		VkResult res_graphics_allocate_command_buffers = pfn_allocate_command_buffers(device, &graphics_command_buffer_allocate_info, &this->m_graphics_command_buffer);
		assert(VK_SUCCESS == res_graphics_allocate_command_buffers);
	}

	assert(NULL == this->m_pfn_begin_command_buffer);
	this->m_pfn_begin_command_buffer = reinterpret_cast<PFN_vkBeginCommandBuffer>(pfn_get_device_proc_addr(device, "vkBeginCommandBuffer"));
	assert(NULL == this->m_pfn_cmd_pipeline_barrier);
	this->m_pfn_cmd_pipeline_barrier = reinterpret_cast<PFN_vkCmdPipelineBarrier>(pfn_get_device_proc_addr(device, "vkCmdPipelineBarrier"));
	assert(NULL == this->m_pfn_cmd_copy_buffer);
	this->m_pfn_cmd_copy_buffer = reinterpret_cast<PFN_vkCmdCopyBuffer>(pfn_get_device_proc_addr(device, "vkCmdCopyBuffer"));
	assert(NULL == this->m_pfn_cmd_copy_buffer_to_image);
	this->m_pfn_cmd_copy_buffer_to_image = reinterpret_cast<PFN_vkCmdCopyBufferToImage>(pfn_get_device_proc_addr(device, "vkCmdCopyBufferToImage"));
	assert(NULL == this->m_pfn_cmd_build_acceleration_structure);
	assert(NULL == this->m_pfn_cmd_reset_query_pool);
	assert(NULL == this->m_pfn_cmd_write_acceleration_structures_properties);
	assert(NULL == this->m_pfn_cmd_copy_acceleration_structure);
	if (this->m_support_ray_tracing)
	{
		this->m_pfn_cmd_build_acceleration_structure = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(pfn_get_device_proc_addr(device, "vkCmdBuildAccelerationStructuresKHR"));
		this->m_pfn_cmd_reset_query_pool = reinterpret_cast<PFN_vkCmdResetQueryPool>(pfn_get_device_proc_addr(device, "vkCmdResetQueryPool"));
		this->m_pfn_cmd_write_acceleration_structures_properties = reinterpret_cast<PFN_vkCmdWriteAccelerationStructuresPropertiesKHR>(pfn_get_device_proc_addr(device, "vkCmdWriteAccelerationStructuresPropertiesKHR"));
		this->m_pfn_cmd_copy_acceleration_structure = reinterpret_cast<PFN_vkCmdCopyAccelerationStructureKHR>(pfn_get_device_proc_addr(device, "vkCmdCopyAccelerationStructureKHR"));
	}
	assert(NULL == this->m_pfn_end_command_buffer);
	this->m_pfn_end_command_buffer = reinterpret_cast<PFN_vkEndCommandBuffer>(pfn_get_device_proc_addr(device, "vkEndCommandBuffer"));
}

void brx_vk_upload_command_buffer::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkDestroyCommandPool const pfn_destroy_command_pool = reinterpret_cast<PFN_vkDestroyCommandPool>(pfn_get_device_proc_addr(device, "vkDestroyCommandPool"));
	assert(NULL != pfn_destroy_command_pool);
	PFN_vkFreeCommandBuffers const pfn_free_command_buffers = reinterpret_cast<PFN_vkFreeCommandBuffers>(pfn_get_device_proc_addr(device, "vkFreeCommandBuffers"));
	assert(NULL != pfn_free_command_buffers);
	PFN_vkDestroySemaphore const pfn_destroy_semaphore = reinterpret_cast<PFN_vkDestroySemaphore>(pfn_get_device_proc_addr(device, "vkDestroySemaphore"));
	assert(NULL != pfn_destroy_semaphore);

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_buffer);
			pfn_free_command_buffers(device, this->m_upload_command_pool, 1U, &this->m_upload_command_buffer);
			this->m_upload_command_buffer = VK_NULL_HANDLE;

			assert(VK_NULL_HANDLE != this->m_upload_command_pool);
			pfn_destroy_command_pool(device, this->m_upload_command_pool, allocation_callbacks);
			this->m_upload_command_pool = VK_NULL_HANDLE;

			assert(VK_NULL_HANDLE == this->m_graphics_command_buffer);

			assert(VK_NULL_HANDLE == this->m_graphics_command_pool);

			assert(VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);
			pfn_destroy_semaphore(device, this->m_upload_queue_submit_semaphore, allocation_callbacks);
			this->m_upload_queue_submit_semaphore = VK_NULL_HANDLE;
		}
		else
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_buffer);
			pfn_free_command_buffers(device, this->m_upload_command_pool, 1U, &this->m_upload_command_buffer);
			this->m_upload_command_buffer = VK_NULL_HANDLE;

			assert(VK_NULL_HANDLE != this->m_upload_command_pool);
			pfn_destroy_command_pool(device, this->m_upload_command_pool, allocation_callbacks);
			this->m_upload_command_pool = VK_NULL_HANDLE;

			assert(VK_NULL_HANDLE == this->m_graphics_command_buffer);

			assert(VK_NULL_HANDLE == this->m_graphics_command_pool);

			assert(VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);
			pfn_destroy_semaphore(device, this->m_upload_queue_submit_semaphore, allocation_callbacks);
			this->m_upload_queue_submit_semaphore = VK_NULL_HANDLE;
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == this->m_upload_command_buffer);

		assert(VK_NULL_HANDLE == this->m_upload_command_pool);

		assert(VK_NULL_HANDLE != this->m_graphics_command_buffer);
		pfn_free_command_buffers(device, this->m_graphics_command_pool, 1U, &this->m_graphics_command_buffer);
		this->m_graphics_command_buffer = VK_NULL_HANDLE;

		assert(VK_NULL_HANDLE != this->m_graphics_command_pool);
		pfn_destroy_command_pool(device, this->m_graphics_command_pool, allocation_callbacks);
		this->m_graphics_command_pool = VK_NULL_HANDLE;

		assert(VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);
	}
}

brx_vk_upload_command_buffer::~brx_vk_upload_command_buffer()
{
	assert(VK_NULL_HANDLE == this->m_upload_command_pool);
	assert(VK_NULL_HANDLE == this->m_upload_command_buffer);
	assert(VK_NULL_HANDLE == this->m_graphics_command_pool);
	assert(VK_NULL_HANDLE == this->m_graphics_command_buffer);
	assert(VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);
}

VkCommandPool brx_vk_upload_command_buffer::get_upload_command_pool() const
{
	return this->m_upload_command_pool;
}

VkCommandBuffer brx_vk_upload_command_buffer::get_upload_command_buffer() const
{
	return this->m_upload_command_buffer;
}

VkCommandPool brx_vk_upload_command_buffer::get_graphics_command_pool() const
{
	return this->m_graphics_command_pool;
}

VkCommandBuffer brx_vk_upload_command_buffer::get_graphics_command_buffer() const
{
	return this->m_graphics_command_buffer;
}

VkSemaphore brx_vk_upload_command_buffer::get_upload_queue_submit_semaphore() const
{
	return this->m_upload_queue_submit_semaphore;
}

void brx_vk_upload_command_buffer::begin()
{
	if (this->m_has_dedicated_upload_queue)
	{
		assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer);

		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			VkCommandBufferBeginInfo upload_command_buffer_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL};
			VkResult res_begin_upload_command_buffer = this->m_pfn_begin_command_buffer(this->m_upload_command_buffer, &upload_command_buffer_begin_info);
			assert(VK_SUCCESS == res_begin_upload_command_buffer);
		}
		else
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			VkCommandBufferBeginInfo upload_command_buffer_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL};
			VkResult res_begin_upload_command_buffer = this->m_pfn_begin_command_buffer(this->m_upload_command_buffer, &upload_command_buffer_begin_info);
			assert(VK_SUCCESS == res_begin_upload_command_buffer);
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == this->m_upload_command_pool && VK_NULL_HANDLE == this->m_upload_command_buffer && VK_NULL_HANDLE != this->m_graphics_command_pool && VK_NULL_HANDLE != this->m_graphics_command_buffer && VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);

		VkCommandBufferBeginInfo graphics_command_buffer_begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, NULL, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL};
		VkResult res_begin_graphics_command_buffer = this->m_pfn_begin_command_buffer(this->m_graphics_command_buffer, &graphics_command_buffer_begin_info);
		assert(VK_SUCCESS == res_begin_graphics_command_buffer);
	}
}

void brx_vk_upload_command_buffer::upload_from_staging_upload_buffer_to_asset_vertex_position_buffer(brx_asset_vertex_position_buffer *wrapped_asset_vertex_position_buffer, uint64_t dst_offset, brx_staging_upload_buffer *wrapped_staging_upload_buffer, uint64_t src_offset, uint32_t src_size)
{
	assert(NULL != wrapped_asset_vertex_position_buffer);
	VkBuffer const asset_buffer = static_cast<brx_vk_asset_vertex_position_buffer *>(wrapped_asset_vertex_position_buffer)->get_buffer();

	assert(NULL != wrapped_staging_upload_buffer);
	VkBuffer const staging_upload_buffer = static_cast<brx_vk_staging_upload_buffer *>(wrapped_staging_upload_buffer)->get_buffer();

	VkBufferCopy const region = {src_offset, dst_offset, src_size};

	VkBufferMemoryBarrier const upload_queue_family_store_barrier = {
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		(!this->m_support_ray_tracing) ? 0U : VK_ACCESS_SHADER_READ_BIT,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		asset_buffer,
		0U,
		VK_WHOLE_SIZE};
	VkPipelineStageFlags const upload_queue_family_store_source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkPipelineStageFlags const upload_queue_family_store_destination_stage = (!this->m_support_ray_tracing) ? VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT : VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;

	VkBufferMemoryBarrier const graphics_queue_family_store_barrier = {
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		(!this->m_support_ray_tracing) ? VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT : static_cast<VkAccessFlags>(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_SHADER_READ_BIT),
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		asset_buffer,
		0U,
		VK_WHOLE_SIZE};
	VkPipelineStageFlags const graphics_queue_family_store_source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkPipelineStageFlags const graphics_queue_family_store_destination_stage = (!this->m_support_ray_tracing) ? VK_PIPELINE_STAGE_VERTEX_INPUT_BIT : static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR);

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			this->m_pfn_cmd_copy_buffer(this->m_upload_command_buffer, staging_upload_buffer, asset_buffer, 1U, &region);

			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, upload_queue_family_store_source_stage, upload_queue_family_store_destination_stage, 0U, 0U, NULL, 1U, &upload_queue_family_store_barrier, 0U, NULL);
		}
		else
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			this->m_pfn_cmd_copy_buffer(this->m_upload_command_buffer, staging_upload_buffer, asset_buffer, 1U, &region);

			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, graphics_queue_family_store_source_stage, graphics_queue_family_store_destination_stage, 0U, 0U, NULL, 1U, &graphics_queue_family_store_barrier, 0U, NULL);
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == this->m_upload_command_pool && VK_NULL_HANDLE == this->m_upload_command_buffer && VK_NULL_HANDLE != this->m_graphics_command_pool && VK_NULL_HANDLE != this->m_graphics_command_buffer && VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);

		this->m_pfn_cmd_copy_buffer(this->m_graphics_command_buffer, staging_upload_buffer, asset_buffer, 1U, &region);

		this->m_pfn_cmd_pipeline_barrier(this->m_graphics_command_buffer, graphics_queue_family_store_source_stage, graphics_queue_family_store_destination_stage, 0U, 0U, NULL, 1U, &graphics_queue_family_store_barrier, 0U, NULL);
	}
}

void brx_vk_upload_command_buffer::upload_from_staging_upload_buffer_to_asset_vertex_varying_buffer(brx_asset_vertex_varying_buffer *wrapped_asset_vertex_varying_buffer, uint64_t dst_offset, brx_staging_upload_buffer *wrapped_staging_upload_buffer, uint64_t src_offset, uint32_t src_size)
{
	assert(NULL != wrapped_asset_vertex_varying_buffer);
	VkBuffer const asset_buffer = static_cast<brx_vk_asset_vertex_varying_buffer *>(wrapped_asset_vertex_varying_buffer)->get_buffer();

	assert(NULL != wrapped_staging_upload_buffer);
	VkBuffer const staging_upload_buffer = static_cast<brx_vk_staging_upload_buffer *>(wrapped_staging_upload_buffer)->get_buffer();

	VkBufferCopy const region = {src_offset, dst_offset, src_size};

	VkBufferMemoryBarrier const upload_queue_family_store_barrier = {
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		(!this->m_support_ray_tracing) ? 0U : VK_ACCESS_SHADER_READ_BIT,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		asset_buffer,
		0U,
		VK_WHOLE_SIZE};
	VkPipelineStageFlags const upload_queue_family_store_source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkPipelineStageFlags const upload_queue_family_store_destination_stage = (!this->m_support_ray_tracing) ? VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT : g_upload_queue_family_all_supported_shader_stages;

	VkBufferMemoryBarrier const graphics_queue_family_store_barrier = {
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		(!this->m_support_ray_tracing) ? VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT : static_cast<VkAccessFlags>(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_SHADER_READ_BIT),
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		asset_buffer,
		0U,
		VK_WHOLE_SIZE};
	VkPipelineStageFlags const graphics_queue_family_store_source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkPipelineStageFlags const graphics_queue_family_store_destination_stage = (!this->m_support_ray_tracing) ? VK_PIPELINE_STAGE_VERTEX_INPUT_BIT : static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | g_graphics_queue_family_all_supported_shader_stages);

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			this->m_pfn_cmd_copy_buffer(this->m_upload_command_buffer, staging_upload_buffer, asset_buffer, 1U, &region);

			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, upload_queue_family_store_source_stage, upload_queue_family_store_destination_stage, 0U, 0U, NULL, 1U, &upload_queue_family_store_barrier, 0U, NULL);
		}
		else
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			this->m_pfn_cmd_copy_buffer(this->m_upload_command_buffer, staging_upload_buffer, asset_buffer, 1U, &region);

			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, graphics_queue_family_store_source_stage, graphics_queue_family_store_destination_stage, 0U, 0U, NULL, 1U, &graphics_queue_family_store_barrier, 0U, NULL);
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == this->m_upload_command_pool && VK_NULL_HANDLE == this->m_upload_command_buffer && VK_NULL_HANDLE != this->m_graphics_command_pool && VK_NULL_HANDLE != this->m_graphics_command_buffer && VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);

		this->m_pfn_cmd_copy_buffer(this->m_graphics_command_buffer, staging_upload_buffer, asset_buffer, 1U, &region);

		this->m_pfn_cmd_pipeline_barrier(this->m_graphics_command_buffer, graphics_queue_family_store_source_stage, graphics_queue_family_store_destination_stage, 0U, 0U, NULL, 1U, &graphics_queue_family_store_barrier, 0U, NULL);
	}
}

void brx_vk_upload_command_buffer::upload_from_staging_upload_buffer_to_asset_index_buffer(brx_asset_index_buffer *wrapped_asset_index_buffer, uint64_t dst_offset, brx_staging_upload_buffer *wrapped_staging_upload_buffer, uint64_t src_offset, uint32_t src_size)
{
	assert(NULL != wrapped_asset_index_buffer);
	VkBuffer const asset_buffer = static_cast<brx_vk_asset_index_buffer *>(wrapped_asset_index_buffer)->get_buffer();

	assert(NULL != wrapped_staging_upload_buffer);
	VkBuffer const staging_upload_buffer = static_cast<brx_vk_staging_upload_buffer *>(wrapped_staging_upload_buffer)->get_buffer();

	VkBufferCopy const region = {src_offset, dst_offset, src_size};

	VkBufferMemoryBarrier const upload_queue_family_store_barrier = {
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		(!this->m_support_ray_tracing) ? 0U : VK_ACCESS_SHADER_READ_BIT,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		asset_buffer,
		0U,
		VK_WHOLE_SIZE};

	VkPipelineStageFlags const upload_queue_family_store_source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkPipelineStageFlags const upload_queue_family_store_destination_stage = (!this->m_support_ray_tracing) ? VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT : VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;

	VkBufferMemoryBarrier const graphics_queue_family_store_barrier = {
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		(!this->m_support_ray_tracing) ? VK_ACCESS_INDEX_READ_BIT : static_cast<VkAccessFlags>(VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_SHADER_READ_BIT),
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		asset_buffer,
		0U,
		VK_WHOLE_SIZE};
	VkPipelineStageFlags const graphics_queue_family_store_source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkPipelineStageFlags const graphics_queue_family_store_destination_stage = (!this->m_support_ray_tracing) ? VK_PIPELINE_STAGE_VERTEX_INPUT_BIT : static_cast<VkPipelineStageFlags>(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR);

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			this->m_pfn_cmd_copy_buffer(this->m_upload_command_buffer, staging_upload_buffer, asset_buffer, 1U, &region);

			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, upload_queue_family_store_source_stage, upload_queue_family_store_destination_stage, 0U, 0U, NULL, 1U, &upload_queue_family_store_barrier, 0U, NULL);
		}
		else
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			this->m_pfn_cmd_copy_buffer(this->m_upload_command_buffer, staging_upload_buffer, asset_buffer, 1U, &region);

			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, graphics_queue_family_store_source_stage, graphics_queue_family_store_destination_stage, 0U, 0U, NULL, 1U, &graphics_queue_family_store_barrier, 0U, NULL);
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == this->m_upload_command_pool && VK_NULL_HANDLE == this->m_upload_command_buffer && VK_NULL_HANDLE != this->m_graphics_command_pool && VK_NULL_HANDLE != this->m_graphics_command_buffer && VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);

		this->m_pfn_cmd_copy_buffer(this->m_graphics_command_buffer, staging_upload_buffer, asset_buffer, 1U, &region);

		this->m_pfn_cmd_pipeline_barrier(this->m_graphics_command_buffer, graphics_queue_family_store_source_stage, graphics_queue_family_store_destination_stage, 0U, 0U, NULL, 1U, &graphics_queue_family_store_barrier, 0U, NULL);
	}
}

void brx_vk_upload_command_buffer::upload_from_staging_upload_buffer_to_asset_sampled_image(brx_asset_sampled_image *wrapped_asset_sampled_image, BRX_ASSET_IMAGE_FORMAT wrapped_asset_sampled_image_format, uint32_t asset_sampled_image_width, uint32_t asset_sampled_image_height, uint32_t dst_mip_level, brx_staging_upload_buffer *wrapped_staging_upload_buffer, uint64_t src_offset, uint32_t src_row_pitch, uint32_t src_row_count)
{
	assert(NULL != wrapped_asset_sampled_image);
	VkImage const asset_sampled_image = static_cast<brx_vk_asset_sampled_image *>(wrapped_asset_sampled_image)->get_image();

	assert(NULL != wrapped_staging_upload_buffer);
	VkBuffer const staging_upload_buffer = static_cast<brx_vk_staging_upload_buffer *>(wrapped_staging_upload_buffer)->get_buffer();

	uint32_t const block_size = brx_get_format_block_size(wrapped_asset_sampled_image_format);
	uint32_t const block_width = brx_get_format_block_width(wrapped_asset_sampled_image_format);
	uint32_t const block_height = brx_get_format_block_height(wrapped_asset_sampled_image_format);

	assert(0U == (src_row_pitch % block_size));
	uint32_t const buffer_row_length = (src_row_pitch / block_size) * block_width;
	uint32_t const buffer_image_height = src_row_count * block_height;

	VkBufferImageCopy const region = {src_offset, buffer_row_length, buffer_image_height, {VK_IMAGE_ASPECT_COLOR_BIT, dst_mip_level, 0U, 1U}, {0U, 0U, 0U}, {asset_sampled_image_width >> dst_mip_level, asset_sampled_image_height >> dst_mip_level, 1U}};

	VkImageSubresourceRange const asset_sampled_image_subresource_range = {VK_IMAGE_ASPECT_COLOR_BIT, dst_mip_level, 1U, 0U, 1U};

	VkImageMemoryBarrier const load_barrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		0U,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		asset_sampled_image,
		asset_sampled_image_subresource_range};

	VkPipelineStageFlags const load_source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	VkPipelineStageFlags const load_destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

	VkImageMemoryBarrier const upload_queue_family_store_barrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		asset_sampled_image,
		asset_sampled_image_subresource_range};

	VkPipelineStageFlags const upload_queue_family_source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

	VkPipelineStageFlags const upload_queue_family_store_destination_stage = g_upload_queue_family_all_supported_shader_stages;

	VkImageMemoryBarrier const graphics_queue_family_store_barrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		asset_sampled_image,
		asset_sampled_image_subresource_range};

	VkPipelineStageFlags const graphics_queue_family_source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

	VkPipelineStageFlags const graphics_queue_family_store_destination_stage = g_graphics_queue_family_all_supported_shader_stages;

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, load_source_stage, load_destination_stage, 0U, 0U, NULL, 0U, NULL, 1U, &load_barrier);

			this->m_pfn_cmd_copy_buffer_to_image(this->m_upload_command_buffer, staging_upload_buffer, asset_sampled_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &region);

			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, upload_queue_family_source_stage, upload_queue_family_store_destination_stage, 0U, 0U, NULL, 0U, NULL, 1U, &upload_queue_family_store_barrier);
		}
		else
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, load_source_stage, load_destination_stage, 0U, 0U, NULL, 0U, NULL, 1U, &load_barrier);

			this->m_pfn_cmd_copy_buffer_to_image(this->m_upload_command_buffer, staging_upload_buffer, asset_sampled_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &region);

			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, graphics_queue_family_source_stage, graphics_queue_family_store_destination_stage, 0U, 0U, NULL, 0U, NULL, 1U, &graphics_queue_family_store_barrier);
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == this->m_upload_command_pool && VK_NULL_HANDLE == this->m_upload_command_buffer && VK_NULL_HANDLE != this->m_graphics_command_pool && VK_NULL_HANDLE != this->m_graphics_command_buffer && VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);

		this->m_pfn_cmd_pipeline_barrier(this->m_graphics_command_buffer, load_source_stage, load_destination_stage, 0U, 0U, NULL, 0U, NULL, 1U, &load_barrier);

		this->m_pfn_cmd_copy_buffer_to_image(this->m_graphics_command_buffer, staging_upload_buffer, asset_sampled_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &region);

		this->m_pfn_cmd_pipeline_barrier(this->m_graphics_command_buffer, graphics_queue_family_source_stage, graphics_queue_family_store_destination_stage, 0U, 0U, NULL, 0U, NULL, 1U, &graphics_queue_family_store_barrier);
	}
}

void brx_vk_upload_command_buffer::build_staging_non_compacted_bottom_level_acceleration_structure(brx_staging_non_compacted_bottom_level_acceleration_structure *wrapped_staging_non_compacted_bottom_level_acceleration_structure, uint32_t bottom_level_acceleration_structure_geometry_count, BRX_BOTTOM_LEVEL_ACCELERATION_STRUCTURE_GEOMETRY const *wrapped_bottom_level_acceleration_structure_geometries, brx_scratch_buffer *wrapped_scratch_buffer, brx_compacted_bottom_level_acceleration_structure_size_query_pool *wrapped_compacted_bottom_level_acceleration_structure_size_query_pool, uint32_t query_index)
{
	assert(NULL != wrapped_staging_non_compacted_bottom_level_acceleration_structure);
	VkAccelerationStructureKHR const destination_acceleration_structure = static_cast<brx_vk_staging_non_compacted_bottom_level_acceleration_structure *>(wrapped_staging_non_compacted_bottom_level_acceleration_structure)->get_acceleration_structure();

	assert(NULL != wrapped_bottom_level_acceleration_structure_geometries);
	brx_vector<VkBufferMemoryBarrier> vertex_position_buffer_load_barriers;
	brx_vector<VkBufferMemoryBarrier> index_buffer_load_barriers;
	VkPipelineStageFlags const vertex_index_buffer_load_source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkPipelineStageFlags const vertex_index_buffer_load_destination_stage = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
	// we do not need to care "store" barriers since we do not write the buffers and there is no "layout" transitions for the buffers
	brx_vector<VkAccelerationStructureGeometryKHR> acceleration_structure_geometries;
	brx_vector<VkAccelerationStructureBuildRangeInfoKHR> acceleration_structure_build_range_infos;
	acceleration_structure_geometries.reserve(bottom_level_acceleration_structure_geometry_count);
	acceleration_structure_build_range_infos.reserve(bottom_level_acceleration_structure_geometry_count);
	for (uint32_t bottom_level_acceleration_structure_geometry_index = 0U; bottom_level_acceleration_structure_geometry_index < bottom_level_acceleration_structure_geometry_count; ++bottom_level_acceleration_structure_geometry_index)
	{
		BRX_BOTTOM_LEVEL_ACCELERATION_STRUCTURE_GEOMETRY const &wrapped_bottom_level_acceleration_structure_geometry = wrapped_bottom_level_acceleration_structure_geometries[bottom_level_acceleration_structure_geometry_index];

		brx_vk_vertex_position_buffer const *const unwrapped_vertex_position_buffer = static_cast<brx_vk_vertex_position_buffer const *>(wrapped_bottom_level_acceleration_structure_geometry.vertex_position_buffer);

		VkBufferMemoryBarrier const vertex_position_buffer_load_barrier = {
			VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			NULL,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			unwrapped_vertex_position_buffer->get_buffer(),
			0U,
			VK_WHOLE_SIZE};
		vertex_position_buffer_load_barriers.push_back(vertex_position_buffer_load_barrier);

		brx_vk_index_buffer const *const unwrapped_index_buffer = (BRX_GRAPHICS_PIPELINE_INDEX_TYPE_NONE != wrapped_bottom_level_acceleration_structure_geometry.index_type) ? static_cast<brx_vk_index_buffer const *>(wrapped_bottom_level_acceleration_structure_geometry.index_buffer) : NULL;

		if (BRX_GRAPHICS_PIPELINE_INDEX_TYPE_NONE != wrapped_bottom_level_acceleration_structure_geometry.index_type)
		{
			VkBufferMemoryBarrier const index_buffer_load_barrier = {
				VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
				NULL,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_SHADER_READ_BIT,
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				unwrapped_index_buffer->get_buffer(),
				0U,
				VK_WHOLE_SIZE};
			index_buffer_load_barriers.push_back(index_buffer_load_barrier);
		}

		VkFormat vertex_position_attribute_format;
		switch (wrapped_bottom_level_acceleration_structure_geometry.vertex_position_attribute_format)
		{
		case BRX_GRAPHICS_PIPELINE_VERTEX_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT:
			vertex_position_attribute_format = VK_FORMAT_R32G32B32_SFLOAT;
			break;
		default:
			// VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR
			assert(false);
			vertex_position_attribute_format = static_cast<VkFormat>(-1);
			break;
		}

		VkDeviceAddress const vertex_position_buffer_device_memory_range_base = unwrapped_vertex_position_buffer->get_device_memory_range_base();

		VkIndexType index_type;
		switch (wrapped_bottom_level_acceleration_structure_geometry.index_type)
		{
		case BRX_GRAPHICS_PIPELINE_INDEX_TYPE_UINT32:
			index_type = VK_INDEX_TYPE_UINT32;
			break;
		case BRX_GRAPHICS_PIPELINE_INDEX_TYPE_UINT16:
			index_type = VK_INDEX_TYPE_UINT16;
			break;
		case BRX_GRAPHICS_PIPELINE_INDEX_TYPE_NONE:
			index_type = VK_INDEX_TYPE_NONE_KHR;
			break;
		default:
			assert(false);
			index_type = static_cast<VkIndexType>(-1);
		}

		VkDeviceAddress const index_buffer_device_memory_range_base = (BRX_GRAPHICS_PIPELINE_INDEX_TYPE_NONE != wrapped_bottom_level_acceleration_structure_geometry.index_type) ? unwrapped_index_buffer->get_device_memory_range_base() : NULL;

		VkAccelerationStructureGeometryKHR const acceleration_structure_geometry = {
			VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
			NULL,
			VK_GEOMETRY_TYPE_TRIANGLES_KHR,
			{.triangles =
				 {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
				  NULL,
				  vertex_position_attribute_format,
				  {.deviceAddress = vertex_position_buffer_device_memory_range_base},
				  wrapped_bottom_level_acceleration_structure_geometry.vertex_position_binding_stride,
				  (BRX_GRAPHICS_PIPELINE_INDEX_TYPE_NONE != wrapped_bottom_level_acceleration_structure_geometry.index_type) ? (wrapped_bottom_level_acceleration_structure_geometry.index_count - 1U) : (wrapped_bottom_level_acceleration_structure_geometry.vertex_count - 1U),
				  index_type,
				  {.deviceAddress = index_buffer_device_memory_range_base},
				  {.deviceAddress = 0U}}},
			wrapped_bottom_level_acceleration_structure_geometry.force_closest_hit ? VK_GEOMETRY_OPAQUE_BIT_KHR : 0U};

		acceleration_structure_geometries.push_back(acceleration_structure_geometry);

		assert(0U == ((VK_INDEX_TYPE_NONE_KHR != index_type) ? (wrapped_bottom_level_acceleration_structure_geometry.index_count % 3U) : (wrapped_bottom_level_acceleration_structure_geometry.vertex_count % 3U)));
		uint32_t const primitive_count = (VK_INDEX_TYPE_NONE_KHR == index_type) ? (wrapped_bottom_level_acceleration_structure_geometry.vertex_count / 3U) : (wrapped_bottom_level_acceleration_structure_geometry.index_count / 3U);

		VkAccelerationStructureBuildRangeInfoKHR const acceleration_structure_build_range_info = {
			primitive_count,
			0U,
			0U,
			0U};

		acceleration_structure_build_range_infos.push_back(acceleration_structure_build_range_info);
	}
	assert(bottom_level_acceleration_structure_geometry_count == acceleration_structure_geometries.size());
	assert(bottom_level_acceleration_structure_geometry_count == acceleration_structure_build_range_infos.size());

	assert(NULL != wrapped_scratch_buffer);
	VkDeviceAddress const scratch_buffer_device_memory_range_base = static_cast<brx_vk_scratch_buffer *>(wrapped_scratch_buffer)->get_device_memory_range_base();

	assert(NULL != wrapped_compacted_bottom_level_acceleration_structure_size_query_pool);
	VkQueryPool const query_pool = static_cast<brx_vk_compacted_bottom_level_acceleration_structure_size_query_pool *>(wrapped_compacted_bottom_level_acceleration_structure_size_query_pool)->get_query_pool();

	VkAccelerationStructureBuildGeometryInfoKHR const acceleration_structure_build_geometry_info = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		NULL,
		VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
		VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		VK_NULL_HANDLE,
		destination_acceleration_structure,
		static_cast<uint32_t>(acceleration_structure_geometries.size()),
		&acceleration_structure_geometries[0],
		NULL,
		{.deviceAddress = scratch_buffer_device_memory_range_base}};

	VkAccelerationStructureBuildRangeInfoKHR const *const p_build_range_infos = &acceleration_structure_build_range_infos[0];

	assert(bottom_level_acceleration_structure_geometry_count == vertex_position_buffer_load_barriers.size());
	assert(bottom_level_acceleration_structure_geometry_count >= index_buffer_load_barriers.size());
	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, vertex_index_buffer_load_source_stage, vertex_index_buffer_load_destination_stage, 0U, 0U, NULL, bottom_level_acceleration_structure_geometry_count, &vertex_position_buffer_load_barriers[0], 0U, NULL);

			if (index_buffer_load_barriers.size() > 0U)
			{
				this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, vertex_index_buffer_load_source_stage, vertex_index_buffer_load_destination_stage, 0U, 0U, NULL, static_cast<uint32_t>(index_buffer_load_barriers.size()), &index_buffer_load_barriers[0], 0U, NULL);
			}

			this->m_pfn_cmd_build_acceleration_structure(this->m_upload_command_buffer, 1U, &acceleration_structure_build_geometry_info, &p_build_range_infos);

			this->m_pfn_cmd_reset_query_pool(this->m_upload_command_buffer, query_pool, query_index, 1U);

			this->m_pfn_cmd_write_acceleration_structures_properties(this->m_upload_command_buffer, 1U, &destination_acceleration_structure, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, query_pool, query_index);
		}
		else
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, vertex_index_buffer_load_source_stage, vertex_index_buffer_load_destination_stage, 0U, 0U, NULL, bottom_level_acceleration_structure_geometry_count, &vertex_position_buffer_load_barriers[0], 0U, NULL);

			if (index_buffer_load_barriers.size() > 0U)
			{
				this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, vertex_index_buffer_load_source_stage, vertex_index_buffer_load_destination_stage, 0U, 0U, NULL, static_cast<uint32_t>(index_buffer_load_barriers.size()), &index_buffer_load_barriers[0], 0U, NULL);
			}

			this->m_pfn_cmd_build_acceleration_structure(this->m_upload_command_buffer, 1U, &acceleration_structure_build_geometry_info, &p_build_range_infos);

			this->m_pfn_cmd_reset_query_pool(this->m_upload_command_buffer, query_pool, query_index, 1U);

			this->m_pfn_cmd_write_acceleration_structures_properties(this->m_upload_command_buffer, 1U, &destination_acceleration_structure, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, query_pool, query_index);
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == this->m_upload_command_pool && VK_NULL_HANDLE == this->m_upload_command_buffer && VK_NULL_HANDLE != this->m_graphics_command_pool && VK_NULL_HANDLE != this->m_graphics_command_buffer && VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);

		this->m_pfn_cmd_pipeline_barrier(this->m_graphics_command_buffer, vertex_index_buffer_load_source_stage, vertex_index_buffer_load_destination_stage, 0U, 0U, NULL, bottom_level_acceleration_structure_geometry_count, &vertex_position_buffer_load_barriers[0], 0U, NULL);

		if (index_buffer_load_barriers.size() > 0U)
		{
			this->m_pfn_cmd_pipeline_barrier(this->m_graphics_command_buffer, vertex_index_buffer_load_source_stage, vertex_index_buffer_load_destination_stage, 0U, 0U, NULL, static_cast<uint32_t>(index_buffer_load_barriers.size()), &index_buffer_load_barriers[0], 0U, NULL);
		}

		this->m_pfn_cmd_build_acceleration_structure(this->m_graphics_command_buffer, 1U, &acceleration_structure_build_geometry_info, &p_build_range_infos);

		this->m_pfn_cmd_reset_query_pool(this->m_graphics_command_buffer, query_pool, query_index, 1U);

		this->m_pfn_cmd_write_acceleration_structures_properties(this->m_graphics_command_buffer, 1U, &destination_acceleration_structure, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, query_pool, query_index);
	}
}

void brx_vk_upload_command_buffer::compact_bottom_level_acceleration_structure(brx_asset_compacted_bottom_level_acceleration_structure *wrapped_destination_asset_compacted_bottom_level_acceleration_structure, brx_staging_non_compacted_bottom_level_acceleration_structure *wrapped_source_staging_non_compacted_bottom_level_acceleration_structure)
{
	// NOTE: we don't need the barrier to wait for the building of the source acceleration structure, since we already use the fence to wait before we get the size of the compacted acceleration structure
	assert(NULL != wrapped_source_staging_non_compacted_bottom_level_acceleration_structure);
	VkAccelerationStructureKHR const source_acceleration_structure = static_cast<brx_vk_staging_non_compacted_bottom_level_acceleration_structure *>(wrapped_source_staging_non_compacted_bottom_level_acceleration_structure)->get_acceleration_structure();

	assert(NULL != wrapped_destination_asset_compacted_bottom_level_acceleration_structure);
	VkAccelerationStructureKHR const destination_acceleration_structure = static_cast<brx_vk_asset_compacted_bottom_level_acceleration_structure *>(wrapped_destination_asset_compacted_bottom_level_acceleration_structure)->get_acceleration_structure();
	VkBuffer const destination_buffer = static_cast<brx_vk_asset_compacted_bottom_level_acceleration_structure *>(wrapped_destination_asset_compacted_bottom_level_acceleration_structure)->get_buffer();

	VkCopyAccelerationStructureInfoKHR const copy_acceleration_structure_info = {
		VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR,
		NULL,
		source_acceleration_structure,
		destination_acceleration_structure,
		VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR};

	VkBufferMemoryBarrier const store_barrier = {
		VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
		VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		destination_buffer,
		0U,
		VK_WHOLE_SIZE};

	VkPipelineStageFlags const store_source_stage = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;

	// build top level acceleration structure
	VkPipelineStageFlags const store_destination_stage = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			this->m_pfn_cmd_copy_acceleration_structure(this->m_upload_command_buffer, &copy_acceleration_structure_info);

			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, store_source_stage, store_destination_stage, 0U, 0U, NULL, 1U, &store_barrier, 0U, NULL);
		}
		else
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			this->m_pfn_cmd_copy_acceleration_structure(this->m_upload_command_buffer, &copy_acceleration_structure_info);

			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, store_source_stage, store_destination_stage, 0U, 0U, NULL, 1U, &store_barrier, 0U, NULL);
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == this->m_upload_command_pool && VK_NULL_HANDLE == this->m_upload_command_buffer && VK_NULL_HANDLE != this->m_graphics_command_pool && VK_NULL_HANDLE != this->m_graphics_command_buffer && VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);

		this->m_pfn_cmd_copy_acceleration_structure(this->m_graphics_command_buffer, &copy_acceleration_structure_info);

		this->m_pfn_cmd_pipeline_barrier(this->m_graphics_command_buffer, store_source_stage, store_destination_stage, 0U, 0U, NULL, 1U, &store_barrier, 0U, NULL);
	}
}

void brx_vk_upload_command_buffer::release_asset_vertex_position_buffer(brx_asset_vertex_position_buffer *wrapped_asset_vertex_position_buffer)
{
	assert(NULL != wrapped_asset_vertex_position_buffer);
	VkBuffer const asset_buffer = static_cast<brx_vk_asset_vertex_position_buffer *>(wrapped_asset_vertex_position_buffer)->get_buffer();

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			VkBufferMemoryBarrier const release_barrier =
				{
					VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
					NULL,
					VK_ACCESS_TRANSFER_WRITE_BIT,
					0U,
					this->m_upload_queue_family_index,
					this->m_graphics_queue_family_index,
					asset_buffer,
					0U,
					VK_WHOLE_SIZE};
			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0U, 0U, NULL, 1U, &release_barrier, 0U, NULL);
		}
		else
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			// do nothing
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == this->m_upload_command_pool && VK_NULL_HANDLE == this->m_upload_command_buffer && VK_NULL_HANDLE != this->m_graphics_command_pool && VK_NULL_HANDLE != this->m_graphics_command_buffer && VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);

		// do nothing
	}
}

void brx_vk_upload_command_buffer::release_asset_vertex_varying_buffer(brx_asset_vertex_varying_buffer *wrapped_asset_vertex_varying_buffer)
{
	assert(NULL != wrapped_asset_vertex_varying_buffer);
	VkBuffer const asset_buffer = static_cast<brx_vk_asset_vertex_varying_buffer *>(wrapped_asset_vertex_varying_buffer)->get_buffer();

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			VkBufferMemoryBarrier const release_barrier =
				{
					VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
					NULL,
					VK_ACCESS_TRANSFER_WRITE_BIT,
					0U,
					this->m_upload_queue_family_index,
					this->m_graphics_queue_family_index,
					asset_buffer,
					0U,
					VK_WHOLE_SIZE};
			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0U, 0U, NULL, 1U, &release_barrier, 0U, NULL);
		}
		else
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			// do nothing
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == this->m_upload_command_pool && VK_NULL_HANDLE == this->m_upload_command_buffer && VK_NULL_HANDLE != this->m_graphics_command_pool && VK_NULL_HANDLE != this->m_graphics_command_buffer && VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);

		// do nothing
	}
}

void brx_vk_upload_command_buffer::release_asset_index_buffer(brx_asset_index_buffer *wrapped_asset_index_buffer)
{
	assert(NULL != wrapped_asset_index_buffer);
	VkBuffer const asset_buffer = static_cast<brx_vk_asset_index_buffer *>(wrapped_asset_index_buffer)->get_buffer();

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			VkBufferMemoryBarrier const release_barrier =
				{
					VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
					NULL,
					VK_ACCESS_TRANSFER_WRITE_BIT,
					0U,
					this->m_upload_queue_family_index,
					this->m_graphics_queue_family_index,
					asset_buffer,
					0U,
					VK_WHOLE_SIZE};
			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0U, 0U, NULL, 1U, &release_barrier, 0U, NULL);
		}
		else
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			// do nothing
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == this->m_upload_command_pool && VK_NULL_HANDLE == this->m_upload_command_buffer && VK_NULL_HANDLE != this->m_graphics_command_pool && VK_NULL_HANDLE != this->m_graphics_command_buffer && VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);

		// do nothing
	}
}

void brx_vk_upload_command_buffer::release_asset_sampled_image(brx_asset_sampled_image *wrapped_asset_sampled_image, uint32_t dst_mip_level)
{
	assert(NULL != wrapped_asset_sampled_image);
	VkImage const asset_sampled_image = static_cast<brx_vk_asset_sampled_image *>(wrapped_asset_sampled_image)->get_image();

	VkImageSubresourceRange const asset_sampled_image_subresource_range = {VK_IMAGE_ASPECT_COLOR_BIT, dst_mip_level, 1U, 0U, 1U};

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			VkImageMemoryBarrier const release_barrier = {
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				NULL,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				0,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				this->m_upload_queue_family_index,
				this->m_graphics_queue_family_index,
				asset_sampled_image,
				asset_sampled_image_subresource_range};
			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0U, 0U, NULL, 0U, NULL, 1U, &release_barrier);
		}
		else
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			// do nothing
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == this->m_upload_command_pool && VK_NULL_HANDLE == this->m_upload_command_buffer && VK_NULL_HANDLE != this->m_graphics_command_pool && VK_NULL_HANDLE != this->m_graphics_command_buffer && VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);

		// do nothing
	}
}

void brx_vk_upload_command_buffer::release_asset_compacted_bottom_level_acceleration_structure(brx_asset_compacted_bottom_level_acceleration_structure *wrapped_asset_compacted_bottom_level_acceleration_structure)
{
	assert(NULL != wrapped_asset_compacted_bottom_level_acceleration_structure);
	VkBuffer const asset_buffer = static_cast<brx_vk_asset_compacted_bottom_level_acceleration_structure *>(wrapped_asset_compacted_bottom_level_acceleration_structure)->get_buffer();

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			VkBufferMemoryBarrier const release_barrier =
				{
					VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
					NULL,
					VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
					0U,
					this->m_upload_queue_family_index,
					this->m_graphics_queue_family_index,
					asset_buffer,
					0U,
					VK_WHOLE_SIZE};
			this->m_pfn_cmd_pipeline_barrier(this->m_upload_command_buffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0U, 0U, NULL, 1U, &release_barrier, 0U, NULL);
		}
		else
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			// do nothing
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == this->m_upload_command_pool && VK_NULL_HANDLE == this->m_upload_command_buffer && VK_NULL_HANDLE != this->m_graphics_command_pool && VK_NULL_HANDLE != this->m_graphics_command_buffer && VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);

		// do nothing
	}
}

void brx_vk_upload_command_buffer::end()
{
	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			VkResult res_end_upload_command_buffer = this->m_pfn_end_command_buffer(this->m_upload_command_buffer);
			assert(VK_SUCCESS == res_end_upload_command_buffer);
		}
		else
		{
			assert(VK_NULL_HANDLE != this->m_upload_command_pool && VK_NULL_HANDLE != this->m_upload_command_buffer && VK_NULL_HANDLE == this->m_graphics_command_pool && VK_NULL_HANDLE == this->m_graphics_command_buffer && VK_NULL_HANDLE != this->m_upload_queue_submit_semaphore);

			VkResult res_end_upload_command_buffer = this->m_pfn_end_command_buffer(this->m_upload_command_buffer);
			assert(VK_SUCCESS == res_end_upload_command_buffer);
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == this->m_upload_command_pool && VK_NULL_HANDLE == this->m_upload_command_buffer && VK_NULL_HANDLE != this->m_graphics_command_pool && VK_NULL_HANDLE != this->m_graphics_command_buffer && VK_NULL_HANDLE == this->m_upload_queue_submit_semaphore);

		VkResult res_end_graphics_command_buffer = this->m_pfn_end_command_buffer(this->m_graphics_command_buffer);
		assert(VK_SUCCESS == res_end_graphics_command_buffer);
	}
}
