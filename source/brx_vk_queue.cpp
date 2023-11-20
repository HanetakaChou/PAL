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
#include <assert.h>

brx_vk_graphics_queue::brx_vk_graphics_queue(
	bool has_dedicated_upload_queue,
	uint32_t upload_queue_family_index,
	uint32_t graphics_queue_family_index,
	VkQueue graphics_queue,
	PFN_vkQueueSubmit pfn_queue_submit,
	PFN_vkQueuePresentKHR pfn_queue_present)
	: m_has_dedicated_upload_queue(has_dedicated_upload_queue),
	  m_upload_queue_family_index(upload_queue_family_index),
	  m_graphics_queue_family_index(graphics_queue_family_index),
	  m_graphics_queue(graphics_queue),
	  m_pfn_queue_submit(pfn_queue_submit),
	  m_pfn_queue_present(pfn_queue_present)
{
}

void brx_vk_graphics_queue::wait_and_submit(brx_upload_command_buffer const *brx_upload_command_buffer, brx_graphics_command_buffer const *brx_graphics_command_buffer, brx_fence *brx_fence) const
{
	assert(NULL != brx_upload_command_buffer);
	assert(NULL != brx_fence);
	VkCommandBuffer upload_upload_command_buffer = static_cast<brx_vk_upload_command_buffer const *>(brx_upload_command_buffer)->get_upload_command_buffer();
	VkCommandBuffer upload_graphics_command_buffer = static_cast<brx_vk_upload_command_buffer const *>(brx_upload_command_buffer)->get_graphics_command_buffer();
	VkSemaphore upload_queue_submit_semaphore = static_cast<brx_vk_upload_command_buffer const *>(brx_upload_command_buffer)->get_upload_queue_submit_semaphore();
	VkCommandBuffer graphics_command_buffer = static_cast<brx_vk_graphics_command_buffer const *>(brx_graphics_command_buffer)->get_command_buffer();
	VkFence fence = static_cast<brx_vk_fence const *>(brx_fence)->get_fence();

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != upload_upload_command_buffer && VK_NULL_HANDLE == upload_graphics_command_buffer && VK_NULL_HANDLE != upload_queue_submit_semaphore);

			// graphics_command_buffer
			//
			// queue family ownership transfer
			// acquire operation
			//
			VkPipelineStageFlags wait_dst_stage_mask[1] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
			VkSubmitInfo submit_info{
				VK_STRUCTURE_TYPE_SUBMIT_INFO,
				NULL,
				1U,
				&upload_queue_submit_semaphore,
				wait_dst_stage_mask,
				1U,
				&graphics_command_buffer,
				0U,
				NULL};
			VkResult res_queue_submit = this->m_pfn_queue_submit(this->m_graphics_queue, 1U, &submit_info, fence);
			assert(VK_SUCCESS == res_queue_submit);
		}
		else
		{
			assert(VK_NULL_HANDLE != upload_upload_command_buffer && VK_NULL_HANDLE == upload_graphics_command_buffer && VK_NULL_HANDLE != upload_queue_submit_semaphore);

			VkPipelineStageFlags wait_dst_stage_mask[1] = {VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
			VkSubmitInfo submit_info{
				VK_STRUCTURE_TYPE_SUBMIT_INFO,
				NULL,
				1U,
				&upload_queue_submit_semaphore,
				wait_dst_stage_mask,
				0U,
				NULL,
				0U,
				NULL};
			VkResult res_queue_submit = this->m_pfn_queue_submit(this->m_graphics_queue, 1U, &submit_info, fence);
			assert(VK_SUCCESS == res_queue_submit);
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == upload_upload_command_buffer && VK_NULL_HANDLE != upload_graphics_command_buffer && VK_NULL_HANDLE == upload_queue_submit_semaphore);

		VkSubmitInfo submit_info{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			NULL,
			0U,
			NULL,
			NULL,
			1U,
			&upload_graphics_command_buffer,
			0U,
			NULL};
		VkResult res_queue_submit = this->m_pfn_queue_submit(this->m_graphics_queue, 1U, &submit_info, fence);
		assert(VK_SUCCESS == res_queue_submit);
	}
}

bool brx_vk_graphics_queue::submit_and_present(brx_graphics_command_buffer *brx_graphics_command_buffer, brx_swap_chain *brx_swap_chain, uint32_t swap_chain_image_index, brx_fence *brx_fence) const
{
	assert(NULL != brx_graphics_command_buffer);
	assert(NULL != brx_swap_chain);
	assert(NULL != brx_fence);
	VkCommandBuffer command_buffer = static_cast<brx_vk_graphics_command_buffer const *>(brx_graphics_command_buffer)->get_command_buffer();
	VkSemaphore acquire_next_image_semaphore = static_cast<brx_vk_graphics_command_buffer const *>(brx_graphics_command_buffer)->get_acquire_next_image_semaphore();
	VkSemaphore queue_submit_semaphore = static_cast<brx_vk_graphics_command_buffer const *>(brx_graphics_command_buffer)->get_queue_submit_semaphore();
	VkSwapchainKHR swap_chain = static_cast<brx_vk_swap_chain const *>(brx_swap_chain)->get_swap_chain();
	VkFence fence = static_cast<brx_vk_fence const *>(brx_fence)->get_fence();

	VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submit_info = {
		VK_STRUCTURE_TYPE_SUBMIT_INFO,
		NULL,
		1U,
		&acquire_next_image_semaphore,
		&wait_dst_stage_mask,
		1U,
		&command_buffer,
		1U,
		&queue_submit_semaphore};
	VkResult res_queue_submit = this->m_pfn_queue_submit(this->m_graphics_queue, 1U, &submit_info, fence);
	assert(VK_SUCCESS == res_queue_submit);

	VkPresentInfoKHR present_info = {
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		NULL,
		1U,
		&queue_submit_semaphore,
		1U,
		&swap_chain,
		&swap_chain_image_index,
		NULL};
	VkResult res_queue_present = this->m_pfn_queue_present(this->m_graphics_queue, &present_info);
	switch (res_queue_present)
	{
	case VK_SUCCESS:
		return true;
	case VK_SUBOPTIMAL_KHR:
		return false;
	case VK_ERROR_OUT_OF_DATE_KHR:
		return false;
	default:
		assert(false);
		return false;
	}
}

void brx_vk_graphics_queue::steal(VkQueue *out_graphics_queue)
{
	assert(NULL != out_graphics_queue);

	(*out_graphics_queue) = this->m_graphics_queue;

	this->m_graphics_queue = VK_NULL_HANDLE;
}

brx_vk_graphics_queue::~brx_vk_graphics_queue()
{
	assert(VK_NULL_HANDLE == this->m_graphics_queue);
}

brx_vk_upload_queue::brx_vk_upload_queue(
	bool has_dedicated_upload_queue,
	uint32_t upload_queue_family_index,
	uint32_t graphics_queue_family_index,
	VkQueue upload_queue,
	PFN_vkQueueSubmit pfn_queue_submit)
	: m_has_dedicated_upload_queue(has_dedicated_upload_queue),
	  m_upload_queue_family_index(upload_queue_family_index),
	  m_graphics_queue_family_index(graphics_queue_family_index),
	  m_upload_queue(upload_queue),
	  m_pfn_queue_submit(pfn_queue_submit)
{
}

void brx_vk_upload_queue::submit_and_signal(brx_upload_command_buffer const *brx_upload_command_buffer) const
{
	assert(NULL != brx_upload_command_buffer);
	VkCommandBuffer upload_command_buffer = static_cast<brx_vk_upload_command_buffer const *>(brx_upload_command_buffer)->get_upload_command_buffer();
	VkCommandBuffer graphics_command_buffer = static_cast<brx_vk_upload_command_buffer const *>(brx_upload_command_buffer)->get_graphics_command_buffer();
	VkSemaphore upload_queue_submit_semaphore = static_cast<brx_vk_upload_command_buffer const *>(brx_upload_command_buffer)->get_upload_queue_submit_semaphore();

	if (this->m_has_dedicated_upload_queue)
	{
		if (this->m_upload_queue_family_index != this->m_graphics_queue_family_index)
		{
			assert(VK_NULL_HANDLE != upload_command_buffer && VK_NULL_HANDLE == graphics_command_buffer && VK_NULL_HANDLE != upload_queue_submit_semaphore);

			VkSubmitInfo submit_info{
				VK_STRUCTURE_TYPE_SUBMIT_INFO,
				NULL,
				0U,
				NULL,
				NULL,
				1U,
				&upload_command_buffer,
				1U,
				&upload_queue_submit_semaphore};
			VkResult res_queue_submit = this->m_pfn_queue_submit(this->m_upload_queue, 1U, &submit_info, VK_NULL_HANDLE);
			assert(VK_SUCCESS == res_queue_submit);
		}
		else
		{
			assert(VK_NULL_HANDLE != upload_command_buffer && VK_NULL_HANDLE == graphics_command_buffer && VK_NULL_HANDLE != upload_queue_submit_semaphore);

			VkSubmitInfo submit_info{
				VK_STRUCTURE_TYPE_SUBMIT_INFO,
				NULL,
				0U,
				NULL,
				NULL,
				1U,
				&upload_command_buffer,
				1U,
				&upload_queue_submit_semaphore};
			VkResult res_upload_queue_submit = this->m_pfn_queue_submit(this->m_upload_queue, 1U, &submit_info, VK_NULL_HANDLE);
			assert(VK_SUCCESS == res_upload_queue_submit);
		}
	}
	else
	{
		assert(VK_NULL_HANDLE == upload_command_buffer && VK_NULL_HANDLE != graphics_command_buffer && VK_NULL_HANDLE == upload_queue_submit_semaphore);
	}
}

void brx_vk_upload_queue::steal(VkQueue *out_upload_queue)
{
	assert(NULL != out_upload_queue);

	(*out_upload_queue) = this->m_upload_queue;

	this->m_upload_queue = VK_NULL_HANDLE;
}

brx_vk_upload_queue::~brx_vk_upload_queue()
{
	assert(VK_NULL_HANDLE == this->m_upload_queue);
}