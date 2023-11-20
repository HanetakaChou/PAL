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

brx_vk_uniform_upload_buffer::brx_vk_uniform_upload_buffer() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_host_memory_range_base(NULL)
{
}

void brx_vk_uniform_upload_buffer::init(VmaAllocator memory_allocator, VmaPool uniform_upload_buffer_memory_pool, uint32_t size)
{
	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};

	VmaAllocationCreateInfo const allocation_create_info = {
		VMA_ALLOCATION_CREATE_MAPPED_BIT,
		VMA_MEMORY_USAGE_UNKNOWN,
		0U,
		0U,
		0U,
		uniform_upload_buffer_memory_pool,
		NULL,
		1.0F};

	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	VmaAllocationInfo allocation_info;
	VkResult const res_vma_create_buffer = vmaCreateBuffer(memory_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, &allocation_info);
	assert(VK_SUCCESS == res_vma_create_buffer);

	assert(NULL != allocation_info.pMappedData);
	assert(NULL == this->m_host_memory_range_base);
	this->m_host_memory_range_base = allocation_info.pMappedData;
}

void brx_vk_uniform_upload_buffer::uninit(VmaAllocator memory_allocator)
{
	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(memory_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

VkBuffer brx_vk_uniform_upload_buffer::get_buffer() const
{
	return this->m_buffer;
}

brx_vk_uniform_upload_buffer::~brx_vk_uniform_upload_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
}

void *brx_vk_uniform_upload_buffer::get_host_memory_range_base() const
{
	return this->m_host_memory_range_base;
}

brx_vk_staging_upload_buffer::brx_vk_staging_upload_buffer() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_host_memory_range_base(NULL)
{
}

void brx_vk_staging_upload_buffer::init(VmaAllocator memory_allocator, VmaPool staging_upload_buffer_memory_pool, uint32_t size)
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};

	VmaAllocationCreateInfo const allocation_create_info = {
		VMA_ALLOCATION_CREATE_MAPPED_BIT,
		VMA_MEMORY_USAGE_UNKNOWN,
		0U,
		0U,
		0U,
		staging_upload_buffer_memory_pool,
		NULL,
		1.0F};

	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	VmaAllocationInfo allocation_info;
	VkResult const res_vma_create_buffer = vmaCreateBuffer(memory_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, &allocation_info);
	assert(VK_SUCCESS == res_vma_create_buffer);

	assert(NULL != allocation_info.pMappedData);
	assert(NULL == this->m_host_memory_range_base);
	this->m_host_memory_range_base = allocation_info.pMappedData;
}

void brx_vk_staging_upload_buffer::uninit(VmaAllocator memory_allocator)
{
	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(memory_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

brx_vk_staging_upload_buffer::~brx_vk_staging_upload_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
}

VkBuffer brx_vk_staging_upload_buffer::get_buffer() const
{
	return this->m_buffer;
}

void *brx_vk_staging_upload_buffer::get_host_memory_range_base() const
{
	return this->m_host_memory_range_base;
}

brx_vk_intermediate_storage_buffer::brx_vk_intermediate_storage_buffer() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_device_memory_range_base(0U), m_size(static_cast<VkDeviceSize>(-1))
{
}

void brx_vk_intermediate_storage_buffer::init(bool support_ray_tracing, VkDevice device, PFN_vkGetBufferDeviceAddressKHR pfn_get_buffer_device_address, VmaAllocator memory_allocator, VmaPool storage_buffer_memory_pool, uint32_t size, bool allow_vertex_position, bool allow_vertex_varying)
{
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	if (allow_vertex_position)
	{
		usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if (support_ray_tracing)
		{
			usage |= (VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR);
		}
	}
	if (allow_vertex_varying)
	{
		usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}

	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};

	VmaAllocationCreateInfo const allocation_create_info = {
		0U,
		VMA_MEMORY_USAGE_UNKNOWN,
		0U,
		0U,
		0U,
		storage_buffer_memory_pool,
		NULL,
		1.0F};

	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	VkResult const res_vma_create_buffer = vmaCreateBuffer(memory_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, NULL);
	assert(VK_SUCCESS == res_vma_create_buffer);

	assert(0U == this->m_device_memory_range_base);
	if (support_ray_tracing)
	{
		VkBufferDeviceAddressInfo const buffer_device_address_info = {
			VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			NULL,
			this->m_buffer};
		this->m_device_memory_range_base = pfn_get_buffer_device_address(device, &buffer_device_address_info);
	}

	assert(static_cast<VkDeviceSize>(-1) == this->m_size);
	this->m_size = size;
}

void brx_vk_intermediate_storage_buffer::uninit(VmaAllocator memory_allocator)
{
	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(memory_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

brx_vk_intermediate_storage_buffer::~brx_vk_intermediate_storage_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
}

VkBuffer brx_vk_intermediate_storage_buffer::get_buffer() const
{
	return this->m_buffer;
}

VkDeviceSize brx_vk_intermediate_storage_buffer::get_size() const
{
	return this->m_size;
}

VkDeviceAddress brx_vk_intermediate_storage_buffer::get_device_memory_range_base() const
{
	return this->m_device_memory_range_base;
}

brx_vertex_buffer const *brx_vk_intermediate_storage_buffer::get_vertex_buffer() const
{
	return static_cast<brx_vk_vertex_buffer const *>(this);
}

brx_vertex_position_buffer const *brx_vk_intermediate_storage_buffer::get_vertex_position_buffer() const
{
	return static_cast<brx_vk_vertex_position_buffer const *>(this);
}

brx_vertex_varying_buffer const *brx_vk_intermediate_storage_buffer::get_vertex_varying_buffer() const
{
	return static_cast<brx_vk_vertex_varying_buffer const *>(this);
}

brx_storage_buffer const *brx_vk_intermediate_storage_buffer::get_storage_buffer() const
{
	return static_cast<brx_vk_storage_buffer const *>(this);
}

brx_vk_asset_vertex_position_buffer::brx_vk_asset_vertex_position_buffer() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_device_memory_range_base(0U), m_size(static_cast<VkDeviceSize>(-1))
{
}

void brx_vk_asset_vertex_position_buffer::init(bool support_ray_tracing, VkDevice device, PFN_vkGetBufferDeviceAddressKHR pfn_get_buffer_device_address, VmaAllocator memory_allocator, VmaPool asset_vertex_position_buffer_memory_pool, uint32_t size)
{
	VkBufferUsageFlags const usage = (!support_ray_tracing) ? (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT) : (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR);

	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};

	VmaAllocationCreateInfo const allocation_create_info = {
		0U,
		VMA_MEMORY_USAGE_UNKNOWN,
		0U,
		0U,
		0U,
		asset_vertex_position_buffer_memory_pool,
		NULL,
		1.0F};

	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	VkResult res_vma_create_buffer = vmaCreateBuffer(memory_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, NULL);
	assert(VK_SUCCESS == res_vma_create_buffer);

	assert(0U == this->m_device_memory_range_base);
	if (support_ray_tracing)
	{
		VkBufferDeviceAddressInfo const buffer_device_address_info = {
			VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			NULL,
			this->m_buffer};
		this->m_device_memory_range_base = pfn_get_buffer_device_address(device, &buffer_device_address_info);
	}

	assert(static_cast<VkDeviceSize>(-1) == this->m_size);
	this->m_size = size;
}

void brx_vk_asset_vertex_position_buffer::uninit(VmaAllocator memory_allocator)
{
	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(memory_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

brx_vk_asset_vertex_position_buffer::~brx_vk_asset_vertex_position_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
}

VkBuffer brx_vk_asset_vertex_position_buffer::get_buffer() const
{
	return this->m_buffer;
}

VkDeviceSize brx_vk_asset_vertex_position_buffer::get_size() const
{
	return this->m_size;
}

VkDeviceAddress brx_vk_asset_vertex_position_buffer::get_device_memory_range_base() const
{
	return this->m_device_memory_range_base;
}

brx_vertex_buffer const *brx_vk_asset_vertex_position_buffer::get_vertex_buffer() const
{
	return static_cast<brx_vk_vertex_buffer const *>(this);
}

brx_vertex_position_buffer const *brx_vk_asset_vertex_position_buffer::get_vertex_position_buffer() const
{
	return static_cast<brx_vk_vertex_position_buffer const *>(this);
}

brx_storage_buffer const *brx_vk_asset_vertex_position_buffer::get_storage_buffer() const
{
	return static_cast<brx_vk_storage_buffer const *>(this);
}

brx_vk_asset_vertex_varying_buffer::brx_vk_asset_vertex_varying_buffer() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_size(static_cast<VkDeviceSize>(-1))
{
}

void brx_vk_asset_vertex_varying_buffer::init(bool support_ray_tracing, VmaAllocator memory_allocator, VmaPool asset_vertex_varying_buffer_memory_pool, uint32_t size)
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);

	VkBufferUsageFlags const usage = (!support_ray_tracing) ? (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT) : (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};

	VmaAllocationCreateInfo const allocation_create_info = {
		0U,
		VMA_MEMORY_USAGE_UNKNOWN,
		0U,
		0U,
		0U,
		asset_vertex_varying_buffer_memory_pool,
		NULL,
		1.0F};

	VkResult res_vma_create_buffer = vmaCreateBuffer(memory_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, NULL);
	assert(VK_SUCCESS == res_vma_create_buffer);

	assert(static_cast<VkDeviceSize>(-1) == this->m_size);
	this->m_size = size;
}

void brx_vk_asset_vertex_varying_buffer::uninit(VmaAllocator memory_allocator)
{
	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(memory_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

brx_vk_asset_vertex_varying_buffer::~brx_vk_asset_vertex_varying_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
}

VkBuffer brx_vk_asset_vertex_varying_buffer::get_buffer() const
{
	return this->m_buffer;
}

VkDeviceSize brx_vk_asset_vertex_varying_buffer::get_size() const
{
	return this->m_size;
}

brx_vertex_buffer const *brx_vk_asset_vertex_varying_buffer::get_vertex_buffer() const
{
	return static_cast<brx_vk_vertex_buffer const *>(this);
}

brx_vertex_varying_buffer const *brx_vk_asset_vertex_varying_buffer::get_vertex_varying_buffer() const
{
	return static_cast<brx_vk_vertex_varying_buffer const *>(this);
}

brx_storage_buffer const *brx_vk_asset_vertex_varying_buffer::get_storage_buffer() const
{
	return static_cast<brx_vk_storage_buffer const *>(this);
}

brx_vk_asset_index_buffer::brx_vk_asset_index_buffer() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_device_memory_range_base(0U), m_size(static_cast<VkDeviceSize>(-1))
{
}

void brx_vk_asset_index_buffer::init(bool support_ray_tracing, VkDevice device, PFN_vkGetBufferDeviceAddressKHR pfn_get_buffer_device_address, VmaAllocator memory_allocator, VmaPool asset_index_buffer_memory_pool, uint32_t size)
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);

	VkBufferUsageFlags const usage = (!support_ray_tracing) ? (VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT) : (VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR);

	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};

	VmaAllocationCreateInfo const allocation_create_info = {
		0U,
		VMA_MEMORY_USAGE_UNKNOWN,
		0U,
		0U,
		0U,
		asset_index_buffer_memory_pool,
		NULL,
		1.0F};

	VkResult res_vma_create_buffer = vmaCreateBuffer(memory_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, NULL);
	assert(VK_SUCCESS == res_vma_create_buffer);

	if (support_ray_tracing)
	{
		VkBufferDeviceAddressInfo const buffer_device_address_info = {
			VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			NULL,
			this->m_buffer};
		this->m_device_memory_range_base = pfn_get_buffer_device_address(device, &buffer_device_address_info);
	}

	assert(static_cast<VkDeviceSize>(-1) == this->m_size);
	this->m_size = size;
}

void brx_vk_asset_index_buffer::uninit(VmaAllocator memory_allocator)
{
	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(memory_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

brx_vk_asset_index_buffer::~brx_vk_asset_index_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
}

VkDeviceAddress brx_vk_asset_index_buffer::get_device_memory_range_base() const
{
	return this->m_device_memory_range_base;
}

VkBuffer brx_vk_asset_index_buffer::get_buffer() const
{
	return this->m_buffer;
}

VkDeviceSize brx_vk_asset_index_buffer::get_size() const
{
	return this->m_size;
}

brx_index_buffer const *brx_vk_asset_index_buffer::get_index_buffer() const
{
	return static_cast<brx_vk_index_buffer const *>(this);
}

brx_storage_buffer const *brx_vk_asset_index_buffer::get_storage_buffer() const
{
	return static_cast<brx_vk_storage_buffer const *>(this);
}

brx_vk_scratch_buffer::brx_vk_scratch_buffer() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_device_memory_range_base(0U)
{
}

void brx_vk_scratch_buffer::init(VkDevice device, PFN_vkGetBufferDeviceAddressKHR pfn_get_buffer_device_address, VmaAllocator memory_allocator, VmaPool scratch_buffer_memory_pool, uint32_t size)
{
	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};

	VmaAllocationCreateInfo const allocation_create_info = {
		0U,
		VMA_MEMORY_USAGE_UNKNOWN,
		0U,
		0U,
		0U,
		scratch_buffer_memory_pool,
		NULL,
		1.0F};

	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	VkResult const res_vma_create_buffer = vmaCreateBuffer(memory_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, NULL);
	assert(VK_SUCCESS == res_vma_create_buffer);

	assert(0U == this->m_device_memory_range_base);
	VkBufferDeviceAddressInfo const buffer_device_address_info = {
		VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		NULL,
		this->m_buffer};
	this->m_device_memory_range_base = pfn_get_buffer_device_address(device, &buffer_device_address_info);
}

void brx_vk_scratch_buffer::uninit(VmaAllocator memory_allocator)
{
	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(memory_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

brx_vk_scratch_buffer::~brx_vk_scratch_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
}

VkBuffer brx_vk_scratch_buffer::get_buffer() const
{
	return this->m_buffer;
}

VkDeviceAddress brx_vk_scratch_buffer::get_device_memory_range_base() const
{
	return this->m_device_memory_range_base;
}

brx_vk_staging_non_compacted_bottom_level_acceleration_structure::brx_vk_staging_non_compacted_bottom_level_acceleration_structure() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_acceleration_structure(VK_NULL_HANDLE)
{
}

void brx_vk_staging_non_compacted_bottom_level_acceleration_structure::init(VmaAllocator memory_allocator, VmaPool staging_non_compacted_bottom_level_acceleration_structure_memory_pool, uint32_t size, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkCreateAccelerationStructureKHR const pfn_create_acceleration_structure = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(pfn_get_device_proc_addr(device, "vkCreateAccelerationStructureKHR"));
	assert(NULL != pfn_create_acceleration_structure);

	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};

	VmaAllocationCreateInfo const allocation_create_info = {
		0U,
		VMA_MEMORY_USAGE_UNKNOWN,
		0U,
		0U,
		0U,
		staging_non_compacted_bottom_level_acceleration_structure_memory_pool,
		NULL,
		1.0F};

	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	VkResult const res_vma_create_buffer = vmaCreateBuffer(memory_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, NULL);
	assert(VK_SUCCESS == res_vma_create_buffer);

	VkAccelerationStructureCreateInfoKHR const acceleration_structure_create_info = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		NULL,
		0U,
		this->m_buffer,
		0U,
		size,
		VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		0U};

	assert(VK_NULL_HANDLE == this->m_acceleration_structure);
	pfn_create_acceleration_structure(device, &acceleration_structure_create_info, allocation_callbacks, &this->m_acceleration_structure);
}

void brx_vk_staging_non_compacted_bottom_level_acceleration_structure::uninit(VmaAllocator memory_allocator, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkDestroyAccelerationStructureKHR const pfn_destroy_acceleration_structure = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(pfn_get_device_proc_addr(device, "vkDestroyAccelerationStructureKHR"));
	assert(NULL != pfn_destroy_acceleration_structure);

	assert(VK_NULL_HANDLE != this->m_acceleration_structure);

	pfn_destroy_acceleration_structure(device, this->m_acceleration_structure, allocation_callbacks);

	this->m_acceleration_structure = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(memory_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

brx_vk_staging_non_compacted_bottom_level_acceleration_structure::~brx_vk_staging_non_compacted_bottom_level_acceleration_structure()
{
	assert(VK_NULL_HANDLE == this->m_acceleration_structure);
}

VkAccelerationStructureKHR brx_vk_staging_non_compacted_bottom_level_acceleration_structure::get_acceleration_structure() const
{
	return this->m_acceleration_structure;
}

brx_vk_compacted_bottom_level_acceleration_structure_size_query_pool::brx_vk_compacted_bottom_level_acceleration_structure_size_query_pool() : m_query_pool(VK_NULL_HANDLE)
{
}

void brx_vk_compacted_bottom_level_acceleration_structure_size_query_pool::init(uint32_t query_count, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkCreateQueryPool const pfn_create_query_pool = reinterpret_cast<PFN_vkCreateQueryPool>(pfn_get_device_proc_addr(device, "vkCreateQueryPool"));
	assert(NULL != pfn_create_query_pool);

	VkQueryPoolCreateInfo const query_pool_create_info =
		{
			VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
			NULL,
			0U,
			VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
			query_count};

	assert(VK_NULL_HANDLE == this->m_query_pool);
	VkResult const res_create_query_pool = pfn_create_query_pool(device, &query_pool_create_info, allocation_callbacks, &this->m_query_pool);
	assert(VK_SUCCESS == res_create_query_pool);
}

void brx_vk_compacted_bottom_level_acceleration_structure_size_query_pool::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkDestroyQueryPool const pfn_destroy_query_pool = reinterpret_cast<PFN_vkDestroyQueryPool>(pfn_get_device_proc_addr(device, "vkDestroyQueryPool"));
	assert(NULL != pfn_destroy_query_pool);

	assert(VK_NULL_HANDLE != this->m_query_pool);
	pfn_destroy_query_pool(device, this->m_query_pool, allocation_callbacks);
	this->m_query_pool = VK_NULL_HANDLE;
}

brx_vk_compacted_bottom_level_acceleration_structure_size_query_pool::~brx_vk_compacted_bottom_level_acceleration_structure_size_query_pool()
{
	assert(VK_NULL_HANDLE == this->m_query_pool);
}

VkQueryPool brx_vk_compacted_bottom_level_acceleration_structure_size_query_pool::get_query_pool() const
{
	return this->m_query_pool;
}

brx_vk_asset_compacted_bottom_level_acceleration_structure::brx_vk_asset_compacted_bottom_level_acceleration_structure() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_acceleration_structure(VK_NULL_HANDLE), m_device_memory_range_base(0U)
{
}

void brx_vk_asset_compacted_bottom_level_acceleration_structure::init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, VmaAllocator memory_allocator, VmaPool asset_compacted_bottom_level_acceleration_structure_memory_pool, uint32_t size)
{
	PFN_vkCreateAccelerationStructureKHR const pfn_create_acceleration_structure = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(pfn_get_device_proc_addr(device, "vkCreateAccelerationStructureKHR"));
	assert(NULL != pfn_create_acceleration_structure);
	PFN_vkGetAccelerationStructureDeviceAddressKHR const pfn_get_acceleration_structure_device_address = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(pfn_get_device_proc_addr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
	assert(NULL != pfn_get_acceleration_structure_device_address);

	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};

	VmaAllocationCreateInfo const allocation_create_info = {
		0U,
		VMA_MEMORY_USAGE_UNKNOWN,
		0U,
		0U,
		0U,
		asset_compacted_bottom_level_acceleration_structure_memory_pool,
		NULL,
		1.0F};

	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	VkResult const res_vma_create_buffer = vmaCreateBuffer(memory_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, NULL);
	assert(VK_SUCCESS == res_vma_create_buffer);

	VkAccelerationStructureCreateInfoKHR const acceleration_structure_create_info = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		NULL,
		0U,
		this->m_buffer,
		0U,
		size,
		VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		0U};

	assert(VK_NULL_HANDLE == this->m_acceleration_structure);
	pfn_create_acceleration_structure(device, &acceleration_structure_create_info, allocation_callbacks, &this->m_acceleration_structure);

	VkAccelerationStructureDeviceAddressInfoKHR const acceleration_structure_device_address_info =
		{
			VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			NULL,
			this->m_acceleration_structure};
	assert(0U == this->m_device_memory_range_base);
	this->m_device_memory_range_base = pfn_get_acceleration_structure_device_address(device, &acceleration_structure_device_address_info);
}

void brx_vk_asset_compacted_bottom_level_acceleration_structure::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, VmaAllocator memory_allocator)
{
	PFN_vkDestroyAccelerationStructureKHR const pfn_destroy_acceleration_structure = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(pfn_get_device_proc_addr(device, "vkDestroyAccelerationStructureKHR"));
	assert(NULL != pfn_destroy_acceleration_structure);

	assert(VK_NULL_HANDLE != this->m_acceleration_structure);

	pfn_destroy_acceleration_structure(device, this->m_acceleration_structure, allocation_callbacks);

	this->m_acceleration_structure = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(memory_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

brx_vk_asset_compacted_bottom_level_acceleration_structure::~brx_vk_asset_compacted_bottom_level_acceleration_structure()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	assert(VK_NULL_HANDLE == this->m_acceleration_structure);
}

VkBuffer brx_vk_asset_compacted_bottom_level_acceleration_structure::get_buffer() const
{
	return this->m_buffer;
}

VkAccelerationStructureKHR brx_vk_asset_compacted_bottom_level_acceleration_structure::get_acceleration_structure() const
{
	return this->m_acceleration_structure;
}

VkDeviceAddress brx_vk_asset_compacted_bottom_level_acceleration_structure::get_device_memory_range_base() const
{
	return this->m_device_memory_range_base;
}

brx_vk_top_level_acceleration_structure_instance_upload_buffer::brx_vk_top_level_acceleration_structure_instance_upload_buffer() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_device_memory_range_base(0U), m_host_memory_range_base(NULL)
{
}

void brx_vk_top_level_acceleration_structure_instance_upload_buffer::init(VkDevice device, PFN_vkGetBufferDeviceAddressKHR pfn_get_buffer_device_address, VmaAllocator memory_allocator, VmaPool top_level_acceleration_structure_instance_upload_buffer_memory_pool, uint32_t instance_count)
{
	uint32_t const size = sizeof(VkAccelerationStructureInstanceKHR) * instance_count;

	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};

	VmaAllocationCreateInfo const allocation_create_info = {
		VMA_ALLOCATION_CREATE_MAPPED_BIT,
		VMA_MEMORY_USAGE_UNKNOWN,
		0U,
		0U,
		0U,
		top_level_acceleration_structure_instance_upload_buffer_memory_pool,
		NULL,
		1.0F};

	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	VmaAllocationInfo allocation_info;
	VkResult const res_vma_create_buffer = vmaCreateBuffer(memory_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, &allocation_info);
	assert(VK_SUCCESS == res_vma_create_buffer);

	assert(0U == this->m_device_memory_range_base);
	VkBufferDeviceAddressInfo const buffer_device_address_info = {
		VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		NULL,
		this->m_buffer};
	this->m_device_memory_range_base = pfn_get_buffer_device_address(device, &buffer_device_address_info);

	assert(NULL != allocation_info.pMappedData);
	assert(NULL == this->m_host_memory_range_base);
	this->m_host_memory_range_base = static_cast<VkAccelerationStructureInstanceKHR *>(allocation_info.pMappedData);
}

void brx_vk_top_level_acceleration_structure_instance_upload_buffer::uninit(VmaAllocator memory_allocator)
{
	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(memory_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

brx_vk_top_level_acceleration_structure_instance_upload_buffer::~brx_vk_top_level_acceleration_structure_instance_upload_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
}

void brx_vk_top_level_acceleration_structure_instance_upload_buffer::write_instance(uint32_t instance_index, BRX_TOP_LEVEL_ACCELERATION_STRUCTURE_INSTANCE const *wrapped_bottom_top_acceleration_structure_instance)
{
	assert(wrapped_bottom_top_acceleration_structure_instance->instance_id < 0X1000000U);

	brx_vk_asset_compacted_bottom_level_acceleration_structure *const unwrapped_asset_compacted_bottom_level_acceleration_structure = static_cast<brx_vk_asset_compacted_bottom_level_acceleration_structure *>(wrapped_bottom_top_acceleration_structure_instance->asset_compacted_bottom_level_acceleration_structure);
	VkDeviceAddress const asset_compacted_bottom_level_acceleration_structure_device_memory_range_base = unwrapped_asset_compacted_bottom_level_acceleration_structure->get_device_memory_range_base();

	this->m_host_memory_range_base[instance_index].transform.matrix[0][0] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[0][0];
	this->m_host_memory_range_base[instance_index].transform.matrix[0][1] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[0][1];
	this->m_host_memory_range_base[instance_index].transform.matrix[0][2] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[0][2];
	this->m_host_memory_range_base[instance_index].transform.matrix[0][3] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[0][3];
	this->m_host_memory_range_base[instance_index].transform.matrix[1][0] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[1][0];
	this->m_host_memory_range_base[instance_index].transform.matrix[1][1] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[1][1];
	this->m_host_memory_range_base[instance_index].transform.matrix[1][2] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[1][2];
	this->m_host_memory_range_base[instance_index].transform.matrix[1][3] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[1][3];
	this->m_host_memory_range_base[instance_index].transform.matrix[2][0] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[2][0];
	this->m_host_memory_range_base[instance_index].transform.matrix[2][1] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[2][1];
	this->m_host_memory_range_base[instance_index].transform.matrix[2][2] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[2][2];
	this->m_host_memory_range_base[instance_index].transform.matrix[2][3] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[2][3];
	this->m_host_memory_range_base[instance_index].instanceCustomIndex = wrapped_bottom_top_acceleration_structure_instance->instance_id;
	this->m_host_memory_range_base[instance_index].mask = wrapped_bottom_top_acceleration_structure_instance->instance_mask;
	this->m_host_memory_range_base[instance_index].instanceShaderBindingTableRecordOffset = 0U;
	this->m_host_memory_range_base[instance_index].flags = (wrapped_bottom_top_acceleration_structure_instance->force_closest_hit ? VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR : 0U) | (wrapped_bottom_top_acceleration_structure_instance->force_any_hit ? VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR : 0U) | (wrapped_bottom_top_acceleration_structure_instance->disable_back_face_cull ? VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR : 0U) | (wrapped_bottom_top_acceleration_structure_instance->front_ccw ? VK_GEOMETRY_INSTANCE_TRIANGLE_FLIP_FACING_BIT_KHR : 0U);
	this->m_host_memory_range_base[instance_index].accelerationStructureReference = asset_compacted_bottom_level_acceleration_structure_device_memory_range_base;
}

VkBuffer brx_vk_top_level_acceleration_structure_instance_upload_buffer::get_buffer() const
{
	return this->m_buffer;
}

VkDeviceAddress brx_vk_top_level_acceleration_structure_instance_upload_buffer::get_device_memory_range_base() const
{
	return this->m_device_memory_range_base;
}

brx_vk_top_level_acceleration_structure::brx_vk_top_level_acceleration_structure() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_acceleration_structure(VK_NULL_HANDLE), m_instance_count(static_cast<uint32_t>(-1))
{
}

void brx_vk_top_level_acceleration_structure::init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, VmaAllocator memory_allocator, VmaPool top_level_acceleration_structure_memory_pool, uint32_t size)
{
	PFN_vkCreateAccelerationStructureKHR const pfn_create_acceleration_structure = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(pfn_get_device_proc_addr(device, "vkCreateAccelerationStructureKHR"));
	assert(NULL != pfn_create_acceleration_structure);
	PFN_vkGetAccelerationStructureDeviceAddressKHR const pfn_get_acceleration_structure_device_address = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(pfn_get_device_proc_addr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
	assert(NULL != pfn_get_acceleration_structure_device_address);

	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};

	VmaAllocationCreateInfo const allocation_create_info = {
		0U,
		VMA_MEMORY_USAGE_UNKNOWN,
		0U,
		0U,
		0U,
		top_level_acceleration_structure_memory_pool,
		NULL,
		1.0F};

	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	VkResult const res_vma_create_buffer = vmaCreateBuffer(memory_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, NULL);
	assert(VK_SUCCESS == res_vma_create_buffer);

	VkAccelerationStructureCreateInfoKHR const acceleration_structure_create_info = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		NULL,
		0U,
		this->m_buffer,
		0U,
		size,
		VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		0U};

	assert(VK_NULL_HANDLE == this->m_acceleration_structure);
	pfn_create_acceleration_structure(device, &acceleration_structure_create_info, allocation_callbacks, &this->m_acceleration_structure);
}

void brx_vk_top_level_acceleration_structure::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, VmaAllocator memory_allocator)
{
	PFN_vkDestroyAccelerationStructureKHR const pfn_destroy_acceleration_structure = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(pfn_get_device_proc_addr(device, "vkDestroyAccelerationStructureKHR"));
	assert(NULL != pfn_destroy_acceleration_structure);

	assert(VK_NULL_HANDLE != this->m_acceleration_structure);

	pfn_destroy_acceleration_structure(device, this->m_acceleration_structure, allocation_callbacks);

	this->m_acceleration_structure = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(memory_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

brx_vk_top_level_acceleration_structure::~brx_vk_top_level_acceleration_structure()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	assert(VK_NULL_HANDLE == this->m_acceleration_structure);
}

VkBuffer brx_vk_top_level_acceleration_structure::get_buffer() const
{
	return this->m_buffer;
}

VkAccelerationStructureKHR brx_vk_top_level_acceleration_structure::get_acceleration_structure() const
{
	return this->m_acceleration_structure;
}

void brx_vk_top_level_acceleration_structure::set_instance_count(uint32_t instance_count)
{
	assert(static_cast<uint32_t>(-1) == this->m_instance_count);
	this->m_instance_count = instance_count;
}

uint32_t brx_vk_top_level_acceleration_structure::get_instance_count() const
{
	return this->m_instance_count;
}
