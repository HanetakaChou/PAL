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

brx_vk_intermediate_color_attachment_image::brx_vk_intermediate_color_attachment_image() : m_image(VK_NULL_HANDLE), m_device_memory(VK_NULL_HANDLE), m_image_view(VK_NULL_HANDLE)
{
}

void brx_vk_intermediate_color_attachment_image::init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, uint32_t color_transient_attachment_image_memory_index, uint32_t color_attachment_sampled_image_memory_index, BRX_COLOR_ATTACHMENT_IMAGE_FORMAT wrapped_color_attachment_image_format, uint32_t width, uint32_t height, bool allow_sampled_image)
{
	PFN_vkCreateImage const pfn_create_image = reinterpret_cast<PFN_vkCreateImage>(pfn_get_device_proc_addr(device, "vkCreateImage"));
	assert(NULL != pfn_create_image);
	PFN_vkGetImageMemoryRequirements const pfn_get_image_memory_requirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(pfn_get_device_proc_addr(device, "vkGetImageMemoryRequirements"));
	assert(NULL != pfn_get_image_memory_requirements);
	PFN_vkAllocateMemory const pfn_allocate_memory = reinterpret_cast<PFN_vkAllocateMemory>(pfn_get_device_proc_addr(device, "vkAllocateMemory"));
	assert(NULL != pfn_allocate_memory);
	PFN_vkBindImageMemory const pfn_bind_image_memory = reinterpret_cast<PFN_vkBindImageMemory>(pfn_get_device_proc_addr(device, "vkBindImageMemory"));
	assert(NULL != pfn_bind_image_memory);
	PFN_vkCreateImageView const pfn_create_image_view = reinterpret_cast<PFN_vkCreateImageView>(pfn_get_device_proc_addr(device, "vkCreateImageView"));
	assert(NULL != pfn_create_image_view);

	VkFormat format;
	switch (wrapped_color_attachment_image_format)
	{
	case BRX_COLOR_ATTACHMENT_FORMAT_B8G8R8A8_UNORM:
		format = VK_FORMAT_B8G8R8A8_UNORM;
		break;
	case BRX_COLOR_ATTACHMENT_FORMAT_R8G8B8A8_UNORM:
		format = VK_FORMAT_R8G8B8A8_UNORM;
		break;
	case BRX_COLOR_ATTACHMENT_FORMAT_A2B10G10R10_UNORM_PACK32:
		format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		break;
	case BRX_COLOR_ATTACHMENT_FORMAT_A2R10G10B10_UNORM_PACK32:
		format = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		break;
	case BRX_COLOR_ATTACHMENT_FORMAT_R16G16_UNORM:
		format = VK_FORMAT_R16G16_UNORM;
		break;
	default:
		assert(false);
		format = VK_FORMAT_UNDEFINED;
	}

	uint32_t const memory_type_index = allow_sampled_image ? color_attachment_sampled_image_memory_index : color_transient_attachment_image_memory_index;

	VkImageAspectFlags const aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageUsageFlags const usage = allow_sampled_image ? (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT) : (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);

	assert(VK_NULL_HANDLE == this->m_image);
	VkImageCreateInfo const image_create_info = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0U,
		VK_IMAGE_TYPE_2D,
		format,
		{width, height, 1U},
		1U,
		1U,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL,
		VK_IMAGE_LAYOUT_UNDEFINED};

	VkResult const res_create_image = pfn_create_image(device, &image_create_info, allocation_callbacks, &this->m_image);
	assert(VK_SUCCESS == res_create_image);

	assert(VK_NULL_HANDLE == this->m_device_memory);
	VkMemoryRequirements memory_requirements;
	pfn_get_image_memory_requirements(device, this->m_image, &memory_requirements);
	assert(0U != (memory_requirements.memoryTypeBits & (1U << memory_type_index)));

	VkMemoryAllocateInfo const memory_allocate_info = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		NULL,
		memory_requirements.size,
		memory_type_index};
	VkResult const res_allocate_memory = pfn_allocate_memory(device, &memory_allocate_info, allocation_callbacks, &this->m_device_memory);
	assert(VK_SUCCESS == res_allocate_memory);

	VkResult const res_bind_image_memory = pfn_bind_image_memory(device, this->m_image, this->m_device_memory, 0U);
	assert(VK_SUCCESS == res_bind_image_memory);

	assert(VK_NULL_HANDLE == this->m_image_view);
	VkImageViewCreateInfo const image_view_create_info = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0U,
		this->m_image,
		VK_IMAGE_VIEW_TYPE_2D,
		format,
		{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
		{aspect_mask, 0U, 1U, 0U, 1U}};
	VkResult const res_create_image_view = pfn_create_image_view(device, &image_view_create_info, allocation_callbacks, &this->m_image_view);
	assert(VK_SUCCESS == res_create_image_view);
}

void brx_vk_intermediate_color_attachment_image::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkDestroyImageView const pfn_destroy_image_view = reinterpret_cast<PFN_vkDestroyImageView>(pfn_get_device_proc_addr(device, "vkDestroyImageView"));
	assert(NULL != pfn_destroy_image_view);
	PFN_vkDestroyImage const pfn_destroy_image = reinterpret_cast<PFN_vkDestroyImage>(pfn_get_device_proc_addr(device, "vkDestroyImage"));
	assert(NULL != pfn_destroy_image);
	PFN_vkFreeMemory const pfn_free_memory = reinterpret_cast<PFN_vkFreeMemory>(pfn_get_device_proc_addr(device, "vkFreeMemory"));
	assert(NULL != pfn_free_memory);

	assert(VK_NULL_HANDLE != this->m_image_view);
	pfn_destroy_image_view(device, this->m_image_view, allocation_callbacks);
	this->m_image_view = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_image);
	pfn_destroy_image(device, this->m_image, allocation_callbacks);
	this->m_image = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_device_memory);
	pfn_free_memory(device, this->m_device_memory, allocation_callbacks);
	this->m_device_memory = VK_NULL_HANDLE;
}

brx_vk_intermediate_color_attachment_image::~brx_vk_intermediate_color_attachment_image()
{
	assert(VK_NULL_HANDLE == this->m_image);
	assert(VK_NULL_HANDLE == this->m_device_memory);
	assert(VK_NULL_HANDLE == this->m_image_view);
}

VkImageView brx_vk_intermediate_color_attachment_image::get_image_view() const
{
	return this->m_image_view;
}

brx_sampled_image const *brx_vk_intermediate_color_attachment_image::get_sampled_image() const
{
	return static_cast<brx_vk_sampled_image const *>(this);
}

brx_vk_intermediate_depth_stencil_attachment_image::brx_vk_intermediate_depth_stencil_attachment_image() : m_image(VK_NULL_HANDLE), m_device_memory(VK_NULL_HANDLE), m_image_view(VK_NULL_HANDLE)
{
}

void brx_vk_intermediate_depth_stencil_attachment_image::init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, uint32_t depth_transient_attachment_image_memory_index, uint32_t depth_attachment_sampled_image_memory_index, uint32_t depth_stencil_transient_attachment_image_memory_index, uint32_t depth_stencil_attachment_sampled_image_memory_index, BRX_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT wrapped_depth_stencil_attachment_image_format, uint32_t width, uint32_t height, bool allow_sampled_image)
{
	PFN_vkCreateImage const pfn_create_image = reinterpret_cast<PFN_vkCreateImage>(pfn_get_device_proc_addr(device, "vkCreateImage"));
	assert(NULL != pfn_create_image);
	PFN_vkGetImageMemoryRequirements const pfn_get_image_memory_requirements = reinterpret_cast<PFN_vkGetImageMemoryRequirements>(pfn_get_device_proc_addr(device, "vkGetImageMemoryRequirements"));
	assert(NULL != pfn_get_image_memory_requirements);
	PFN_vkAllocateMemory const pfn_allocate_memory = reinterpret_cast<PFN_vkAllocateMemory>(pfn_get_device_proc_addr(device, "vkAllocateMemory"));
	assert(NULL != pfn_allocate_memory);
	PFN_vkBindImageMemory const pfn_bind_image_memory = reinterpret_cast<PFN_vkBindImageMemory>(pfn_get_device_proc_addr(device, "vkBindImageMemory"));
	assert(NULL != pfn_bind_image_memory);
	PFN_vkCreateImageView const pfn_create_image_view = reinterpret_cast<PFN_vkCreateImageView>(pfn_get_device_proc_addr(device, "vkCreateImageView"));
	assert(NULL != pfn_create_image_view);

	VkFormat format;
	uint32_t memory_type_index;
	VkImageAspectFlags aspect_mask;
	switch (wrapped_depth_stencil_attachment_image_format)
	{
	case BRX_DEPTH_STENCIL_ATTACHMENT_FORMAT_D32_SFLOAT:
		format = VK_FORMAT_D32_SFLOAT;
		memory_type_index = allow_sampled_image ? depth_attachment_sampled_image_memory_index : depth_transient_attachment_image_memory_index;
		aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
		break;
	case BRX_DEPTH_STENCIL_ATTACHMENT_FORMAT_X8_D24_UNORM_PACK32:
		format = VK_FORMAT_X8_D24_UNORM_PACK32;
		memory_type_index = allow_sampled_image ? depth_attachment_sampled_image_memory_index : depth_transient_attachment_image_memory_index;
		aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
		break;
	case BRX_DEPTH_STENCIL_ATTACHMENT_FORMAT_D32_SFLOAT_S8_UINT:
		format = VK_FORMAT_D32_SFLOAT_S8_UINT;
		memory_type_index = allow_sampled_image ? depth_stencil_attachment_sampled_image_memory_index : depth_stencil_transient_attachment_image_memory_index;
		aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		break;
	case BRX_DEPTH_STENCIL_ATTACHMENT_FORMAT_D24_UNORM_S8_UINT:
		format = VK_FORMAT_D24_UNORM_S8_UINT;
		memory_type_index = allow_sampled_image ? depth_stencil_attachment_sampled_image_memory_index : depth_stencil_transient_attachment_image_memory_index;
		aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		break;
	default:
		assert(false);
		format = VK_FORMAT_UNDEFINED;
		memory_type_index = VK_MAX_MEMORY_TYPES;
		aspect_mask = VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
	}

	VkImageUsageFlags const usage = allow_sampled_image ? (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT) : (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);

	assert(VK_NULL_HANDLE == this->m_image);
	VkImageCreateInfo const image_create_info = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0U,
		VK_IMAGE_TYPE_2D,
		format,
		{width, height, 1U},
		1U,
		1U,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL,
		VK_IMAGE_LAYOUT_UNDEFINED};

	VkResult const res_create_image = pfn_create_image(device, &image_create_info, allocation_callbacks, &this->m_image);
	assert(VK_SUCCESS == res_create_image);

	assert(VK_NULL_HANDLE == this->m_device_memory);
	VkMemoryRequirements memory_requirements;
	pfn_get_image_memory_requirements(device, this->m_image, &memory_requirements);
	assert(0U != (memory_requirements.memoryTypeBits & (1U << memory_type_index)));

	VkMemoryAllocateInfo const memory_allocate_info = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		NULL,
		memory_requirements.size,
		memory_type_index};
	VkResult const res_allocate_memory = pfn_allocate_memory(device, &memory_allocate_info, allocation_callbacks, &this->m_device_memory);
	assert(VK_SUCCESS == res_allocate_memory);

	VkResult const res_bind_image_memory = pfn_bind_image_memory(device, this->m_image, this->m_device_memory, 0U);
	assert(VK_SUCCESS == res_bind_image_memory);

	assert(VK_NULL_HANDLE == this->m_image_view);
	VkImageViewCreateInfo const image_view_create_info = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0U,
		this->m_image,
		VK_IMAGE_VIEW_TYPE_2D,
		format,
		{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
		{aspect_mask, 0U, 1U, 0U, 1U}};
	VkResult const res_create_image_view = pfn_create_image_view(device, &image_view_create_info, allocation_callbacks, &this->m_image_view);
	assert(VK_SUCCESS == res_create_image_view);
}

void brx_vk_intermediate_depth_stencil_attachment_image::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkDestroyImageView const pfn_destroy_image_view = reinterpret_cast<PFN_vkDestroyImageView>(pfn_get_device_proc_addr(device, "vkDestroyImageView"));
	assert(NULL != pfn_destroy_image_view);
	PFN_vkDestroyImage const pfn_destroy_image = reinterpret_cast<PFN_vkDestroyImage>(pfn_get_device_proc_addr(device, "vkDestroyImage"));
	assert(NULL != pfn_destroy_image);
	PFN_vkFreeMemory const pfn_free_memory = reinterpret_cast<PFN_vkFreeMemory>(pfn_get_device_proc_addr(device, "vkFreeMemory"));
	assert(NULL != pfn_free_memory);

	assert(VK_NULL_HANDLE != this->m_image_view);
	pfn_destroy_image_view(device, this->m_image_view, allocation_callbacks);
	this->m_image_view = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_image);
	pfn_destroy_image(device, this->m_image, allocation_callbacks);
	this->m_image = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_device_memory);
	pfn_free_memory(device, this->m_device_memory, allocation_callbacks);
	this->m_device_memory = VK_NULL_HANDLE;
}

brx_vk_intermediate_depth_stencil_attachment_image::~brx_vk_intermediate_depth_stencil_attachment_image()
{
	assert(VK_NULL_HANDLE == this->m_image);
	assert(VK_NULL_HANDLE == this->m_device_memory);
	assert(VK_NULL_HANDLE == this->m_image_view);
}

VkImageView brx_vk_intermediate_depth_stencil_attachment_image::get_image_view() const
{
	return this->m_image_view;
}

brx_sampled_image const *brx_vk_intermediate_depth_stencil_attachment_image::get_sampled_image() const
{
	return static_cast<brx_vk_sampled_image const *>(this);
}

brx_vk_intermediate_storage_image::brx_vk_intermediate_storage_image() : m_image(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_image_view(VK_NULL_HANDLE)
{
}

void brx_vk_intermediate_storage_image::init(VkDevice device, PFN_vkCreateImageView pfn_create_image_view, VkAllocationCallbacks const *allocation_callbacks, VmaAllocator memory_allocator, VmaPool storage_image_memory_pool, VkFormat unwrapped_storage_image_format, uint32_t width, uint32_t height, bool allow_sampled_image)
{
	VkImageAspectFlags const aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageUsageFlags const usage = allow_sampled_image ? (VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT) : VK_IMAGE_USAGE_STORAGE_BIT;

	VkImageCreateInfo const image_create_info = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0U,
		VK_IMAGE_TYPE_2D,
		unwrapped_storage_image_format,
		{width, height, 1U},
		1U,
		1U,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL,
		VK_IMAGE_LAYOUT_UNDEFINED};

	VmaAllocationCreateInfo const allocation_create_info = {
		0U,
		VMA_MEMORY_USAGE_UNKNOWN,
		0U,
		0U,
		0U,
		storage_image_memory_pool,
		NULL,
		1.0F};

	assert(VK_NULL_HANDLE == this->m_image);
	assert(VK_NULL_HANDLE == this->m_allocation);
	VkResult res_vma_create_buffer = vmaCreateImage(memory_allocator, &image_create_info, &allocation_create_info, &this->m_image, &this->m_allocation, NULL);
	assert(VK_SUCCESS == res_vma_create_buffer);

	VkImageViewCreateInfo const image_view_create_info = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0U,
		this->m_image,
		VK_IMAGE_VIEW_TYPE_2D,
		unwrapped_storage_image_format,
		{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
		{aspect_mask, 0U, 1U, 0U, 1U}};

	assert(VK_NULL_HANDLE == this->m_image_view);
	VkResult const res_create_image_view = pfn_create_image_view(device, &image_view_create_info, allocation_callbacks, &this->m_image_view);
	assert(VK_SUCCESS == res_create_image_view);
}

void brx_vk_intermediate_storage_image::uninit(VkDevice device, PFN_vkDestroyImageView pfn_destroy_image_view, VkAllocationCallbacks const *allocation_callbacks, VmaAllocator memory_allocator)
{
	assert(VK_NULL_HANDLE != this->m_image_view);
	pfn_destroy_image_view(device, this->m_image_view, allocation_callbacks);
	this->m_image_view = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_image);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyImage(memory_allocator, this->m_image, this->m_allocation);

	this->m_image = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

brx_vk_intermediate_storage_image::~brx_vk_intermediate_storage_image()
{
	assert(VK_NULL_HANDLE == this->m_image);
	assert(VK_NULL_HANDLE == this->m_allocation);
	assert(VK_NULL_HANDLE == this->m_image_view);
}
VkImage brx_vk_intermediate_storage_image::get_image() const
{
	return this->m_image;
}

VkImageView brx_vk_intermediate_storage_image::get_image_view() const
{
	return this->m_image_view;
}

brx_sampled_image const *brx_vk_intermediate_storage_image::get_sampled_image() const
{
	return static_cast<brx_vk_sampled_image const *>(this);
}

brx_vk_asset_sampled_image::brx_vk_asset_sampled_image() : m_image(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_image_view(VK_NULL_HANDLE)
{
}

void brx_vk_asset_sampled_image::init(VkDevice device, PFN_vkCreateImageView pfn_create_image_view, VkAllocationCallbacks const *allocation_callbacks, VmaAllocator memory_allocator, VmaPool asset_sampled_image_memory_pool, VkFormat format, uint32_t width, uint32_t height, uint32_t mip_levels)
{
	VkImageCreateInfo const image_create_info = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0U,
		VK_IMAGE_TYPE_2D,
		format,
		width,
		height,
		1U,
		mip_levels,
		1U,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL,
		VK_IMAGE_LAYOUT_UNDEFINED};

	VmaAllocationCreateInfo const allocation_create_info = {
		0U,
		VMA_MEMORY_USAGE_UNKNOWN,
		0U,
		0U,
		0U,
		asset_sampled_image_memory_pool,
		NULL,
		1.0F};

	assert(VK_NULL_HANDLE == this->m_image);
	assert(VK_NULL_HANDLE == this->m_allocation);
	VkResult const res_vma_create_image = vmaCreateImage(memory_allocator, &image_create_info, &allocation_create_info, &this->m_image, &this->m_allocation, NULL);
	assert(VK_SUCCESS == res_vma_create_image);

	VkImageViewCreateInfo const image_view_create_info = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0U,
		this->m_image,
		VK_IMAGE_VIEW_TYPE_2D,
		format,
		{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
		{VK_IMAGE_ASPECT_COLOR_BIT, 0U, mip_levels, 0U, 1U}};

	assert(VK_NULL_HANDLE == this->m_image_view);
	VkResult res_create_image_view = pfn_create_image_view(device, &image_view_create_info, allocation_callbacks, &this->m_image_view);
	assert(VK_SUCCESS == res_create_image_view);
}

void brx_vk_asset_sampled_image::uninit(VkDevice device, PFN_vkDestroyImageView pfn_destroy_image_view, VkAllocationCallbacks const *allocation_callbacks, VmaAllocator memory_allocator)
{
	assert(VK_NULL_HANDLE != this->m_image_view);
	pfn_destroy_image_view(device, this->m_image_view, allocation_callbacks);
	this->m_image_view = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_image);
	assert(VK_NULL_HANDLE != this->m_allocation);
	vmaDestroyImage(memory_allocator, this->m_image, this->m_allocation);
	this->m_image = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

brx_vk_asset_sampled_image::~brx_vk_asset_sampled_image()
{
	assert(VK_NULL_HANDLE == this->m_image);
	assert(VK_NULL_HANDLE == this->m_allocation);
	assert(VK_NULL_HANDLE == this->m_image_view);
}

VkImage brx_vk_asset_sampled_image::get_image() const
{
	return this->m_image;
}

VkImageView brx_vk_asset_sampled_image::get_image_view() const
{
	return this->m_image_view;
}

brx_sampled_image const *brx_vk_asset_sampled_image::get_sampled_image() const
{
	return static_cast<brx_vk_sampled_image const *>(this);
}
