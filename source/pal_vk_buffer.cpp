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

#include "pal_vk_device.h"
#include <assert.h>

pal_vk_upload_ring_buffer::pal_vk_upload_ring_buffer() : m_buffer(VK_NULL_HANDLE), m_device_memory(VK_NULL_HANDLE), m_memory_range_base(NULL)
{
}

void pal_vk_upload_ring_buffer::init(uint32_t upload_ring_buffer_memory_index, uint32_t size, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkCreateBuffer const pfn_create_buffer = reinterpret_cast<PFN_vkCreateBuffer>(pfn_get_device_proc_addr(device, "vkCreateBuffer"));
	assert(NULL != pfn_create_buffer);
	PFN_vkGetBufferMemoryRequirements const pfn_get_buffer_memory_requirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(pfn_get_device_proc_addr(device, "vkGetBufferMemoryRequirements"));
	assert(NULL != pfn_get_buffer_memory_requirements);
	PFN_vkAllocateMemory const pfn_allocate_memory = reinterpret_cast<PFN_vkAllocateMemory>(pfn_get_device_proc_addr(device, "vkAllocateMemory"));
	assert(NULL != pfn_allocate_memory);
	PFN_vkBindBufferMemory const pfn_bind_buffer_memory = reinterpret_cast<PFN_vkBindBufferMemory>(pfn_get_device_proc_addr(device, "vkBindBufferMemory"));
	assert(NULL != pfn_bind_buffer_memory);
	PFN_vkMapMemory const pfn_map_memory = reinterpret_cast<PFN_vkMapMemory>(pfn_get_device_proc_addr(device, "vkMapMemory"));
	assert(NULL != pfn_map_memory);

	assert(VK_NULL_HANDLE == this->m_buffer);
	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};
	VkResult const res_create_buffer = pfn_create_buffer(device, &buffer_create_info, allocation_callbacks, &this->m_buffer);
	assert(VK_SUCCESS == res_create_buffer);

	VkMemoryRequirements memory_requirements;
	pfn_get_buffer_memory_requirements(device, this->m_buffer, &memory_requirements);
	assert(0U != (memory_requirements.memoryTypeBits & (1U << upload_ring_buffer_memory_index)));

	assert(VK_NULL_HANDLE == this->m_device_memory);
	// maxMemoryAllocationCount >= 4096
	VkMemoryAllocateInfo const memory_allocate_info = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		NULL,
		memory_requirements.size,
		upload_ring_buffer_memory_index};
	VkResult const res_allocate_memory = pfn_allocate_memory(device, &memory_allocate_info, allocation_callbacks, &this->m_device_memory);
	assert(VK_SUCCESS == res_allocate_memory);

	VkResult const res_bind_buffer_memory = pfn_bind_buffer_memory(device, this->m_buffer, this->m_device_memory, 0U);
	assert(VK_SUCCESS == res_bind_buffer_memory);

	assert(NULL == this->m_memory_range_base);
	VkResult const res_map_memory = pfn_map_memory(device, this->m_device_memory, 0U, size, 0U, &this->m_memory_range_base);
	assert(VK_SUCCESS == res_map_memory);
}

void pal_vk_upload_ring_buffer::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkUnmapMemory const pfn_unmap_memory = reinterpret_cast<PFN_vkUnmapMemory>(pfn_get_device_proc_addr(device, "vkUnmapMemory"));
	assert(NULL != pfn_unmap_memory);
	PFN_vkDestroyBuffer const pfn_destroy_buffer = reinterpret_cast<PFN_vkDestroyBuffer>(pfn_get_device_proc_addr(device, "vkDestroyBuffer"));
	assert(NULL != pfn_destroy_buffer);
	PFN_vkFreeMemory const pfn_free_memory = reinterpret_cast<PFN_vkFreeMemory>(pfn_get_device_proc_addr(device, "vkFreeMemory"));
	assert(NULL != pfn_free_memory);

	pfn_unmap_memory(device, this->m_device_memory);

	assert(VK_NULL_HANDLE != this->m_buffer);
	pfn_destroy_buffer(device, this->m_buffer, allocation_callbacks);
	this->m_buffer = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_device_memory);
	pfn_free_memory(device, this->m_device_memory, allocation_callbacks);
	this->m_device_memory = VK_NULL_HANDLE;
}

VkBuffer pal_vk_upload_ring_buffer::get_buffer() const
{
	return this->m_buffer;
}

pal_vk_upload_ring_buffer::~pal_vk_upload_ring_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_device_memory);
}

void *pal_vk_upload_ring_buffer::get_memory_range_base() const
{
	return this->m_memory_range_base;
}

pal_vk_staging_buffer::pal_vk_staging_buffer() : m_buffer(VK_NULL_HANDLE), m_device_memory(VK_NULL_HANDLE), m_memory_range_base(NULL)
{
}

void pal_vk_staging_buffer::init(uint32_t staging_buffer_memory_index, uint32_t size, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkCreateBuffer const pfn_create_buffer = reinterpret_cast<PFN_vkCreateBuffer>(pfn_get_device_proc_addr(device, "vkCreateBuffer"));
	assert(NULL != pfn_create_buffer);
	PFN_vkGetBufferMemoryRequirements const pfn_get_buffer_memory_requirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(pfn_get_device_proc_addr(device, "vkGetBufferMemoryRequirements"));
	assert(NULL != pfn_get_buffer_memory_requirements);
	PFN_vkAllocateMemory const pfn_allocate_memory = reinterpret_cast<PFN_vkAllocateMemory>(pfn_get_device_proc_addr(device, "vkAllocateMemory"));
	assert(NULL != pfn_allocate_memory);
	PFN_vkBindBufferMemory const pfn_bind_buffer_memory = reinterpret_cast<PFN_vkBindBufferMemory>(pfn_get_device_proc_addr(device, "vkBindBufferMemory"));
	assert(NULL != pfn_bind_buffer_memory);
	PFN_vkMapMemory const pfn_map_memory = reinterpret_cast<PFN_vkMapMemory>(pfn_get_device_proc_addr(device, "vkMapMemory"));
	assert(NULL != pfn_map_memory);

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
	VkResult const res_create_buffer = pfn_create_buffer(device, &buffer_create_info, allocation_callbacks, &this->m_buffer);
	assert(VK_SUCCESS == res_create_buffer);

	VkMemoryRequirements memory_requirements;
	pfn_get_buffer_memory_requirements(device, this->m_buffer, &memory_requirements);
	assert(0U != (memory_requirements.memoryTypeBits & (1U << staging_buffer_memory_index)));

	assert(VK_NULL_HANDLE == this->m_device_memory);
	// maxMemoryAllocationCount >= 4096
	VkMemoryAllocateInfo const memory_allocate_info = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		NULL,
		memory_requirements.size,
		staging_buffer_memory_index};
	VkResult const res_allocate_memory = pfn_allocate_memory(device, &memory_allocate_info, allocation_callbacks, &this->m_device_memory);
	assert(VK_SUCCESS == res_allocate_memory);

	VkResult const res_bind_buffer_memory = pfn_bind_buffer_memory(device, this->m_buffer, this->m_device_memory, 0U);
	assert(VK_SUCCESS == res_bind_buffer_memory);

	assert(NULL == this->m_memory_range_base);
	VkResult const res_map_memory = pfn_map_memory(device, this->m_device_memory, 0U, size, 0U, &this->m_memory_range_base);
	assert(VK_SUCCESS == res_map_memory);
}

void pal_vk_staging_buffer::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkUnmapMemory const pfn_unmap_memory = reinterpret_cast<PFN_vkUnmapMemory>(pfn_get_device_proc_addr(device, "vkUnmapMemory"));
	assert(NULL != pfn_unmap_memory);
	PFN_vkDestroyBuffer const pfn_destroy_buffer = reinterpret_cast<PFN_vkDestroyBuffer>(pfn_get_device_proc_addr(device, "vkDestroyBuffer"));
	assert(NULL != pfn_destroy_buffer);
	PFN_vkFreeMemory const pfn_free_memory = reinterpret_cast<PFN_vkFreeMemory>(pfn_get_device_proc_addr(device, "vkFreeMemory"));
	assert(NULL != pfn_free_memory);

	pfn_unmap_memory(device, this->m_device_memory);

	assert(VK_NULL_HANDLE != this->m_buffer);
	pfn_destroy_buffer(device, this->m_buffer, allocation_callbacks);
	this->m_buffer = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_device_memory);
	pfn_free_memory(device, this->m_device_memory, allocation_callbacks);
	this->m_device_memory = VK_NULL_HANDLE;
}

pal_vk_staging_buffer::~pal_vk_staging_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_device_memory);
}

VkBuffer pal_vk_staging_buffer::get_buffer() const
{
	return this->m_buffer;
}

void *pal_vk_staging_buffer::get_memory_range_base() const
{
	return this->m_memory_range_base;
}

pal_vk_asset_vertex_position_buffer::pal_vk_asset_vertex_position_buffer() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_device_memory_range_base(0U)
{
}

void pal_vk_asset_vertex_position_buffer::init(bool support_ray_tracing, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VmaAllocator asset_allocator, uint32_t asset_vertex_position_buffer_memory_index, uint32_t size)
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);

	VkBufferUsageFlags const usage = (!support_ray_tracing) ? (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT) : (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR);

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
		VMA_MEMORY_USAGE_GPU_ONLY,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0U,
		1U << asset_vertex_position_buffer_memory_index,
		VK_NULL_HANDLE,
		NULL,
		1.0F};

	VkResult res_vma_create_buffer = vmaCreateBuffer(asset_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, NULL);
	assert(VK_SUCCESS == res_vma_create_buffer);

	assert(0U == this->m_device_memory_range_base);
	if (support_ray_tracing)
	{
		PFN_vkGetBufferDeviceAddressKHR const pfn_get_buffer_device_address = reinterpret_cast<PFN_vkGetBufferDeviceAddress>(pfn_get_device_proc_addr(device, "vkGetBufferDeviceAddressKHR"));
		assert(NULL != pfn_get_buffer_device_address);

		VkBufferDeviceAddressInfo const buffer_device_address_info = {
			VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			NULL,
			this->m_buffer};
		this->m_device_memory_range_base = pfn_get_buffer_device_address(device, &buffer_device_address_info);
	}
}

void pal_vk_asset_vertex_position_buffer::uninit(VmaAllocator asset_allocator)
{
	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(asset_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

pal_vk_asset_vertex_position_buffer::~pal_vk_asset_vertex_position_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
}

VkBuffer pal_vk_asset_vertex_position_buffer::get_buffer() const
{
	return this->m_buffer;
}

VkDeviceAddress pal_vk_asset_vertex_position_buffer::get_device_memory_range_base() const
{
	return this->m_device_memory_range_base;
}

pal_vertex_buffer const *pal_vk_asset_vertex_position_buffer::get_vertex_buffer() const
{
	return static_cast<pal_vk_vertex_buffer const *>(this);
}

pal_vk_asset_vertex_varying_buffer::pal_vk_asset_vertex_varying_buffer() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE)
{
}

void pal_vk_asset_vertex_varying_buffer::init(bool support_ray_tracing, VmaAllocator asset_allocator, uint32_t asset_vertex_varying_buffer_memory_index, uint32_t size)
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
		VMA_MEMORY_USAGE_GPU_ONLY,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0U,
		1U << asset_vertex_varying_buffer_memory_index,
		VK_NULL_HANDLE,
		NULL,
		1.0F};

	VkResult res_vma_create_buffer = vmaCreateBuffer(asset_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, NULL);
	assert(VK_SUCCESS == res_vma_create_buffer);
}

void pal_vk_asset_vertex_varying_buffer::uninit(VmaAllocator asset_allocator)
{
	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(asset_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

pal_vk_asset_vertex_varying_buffer::~pal_vk_asset_vertex_varying_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
}

VkBuffer pal_vk_asset_vertex_varying_buffer::get_buffer() const
{
	return this->m_buffer;
}

pal_vertex_buffer const *pal_vk_asset_vertex_varying_buffer::get_vertex_buffer() const
{
	return static_cast<pal_vk_vertex_buffer const *>(this);
}

pal_vk_asset_index_buffer::pal_vk_asset_index_buffer() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_device_memory_range_base(0U)
{
}

void pal_vk_asset_index_buffer::init(bool support_ray_tracing, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VmaAllocator asset_allocator, uint32_t asset_index_buffer_memory_index, uint32_t size)
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);

	VkBufferUsageFlags const usage = (!support_ray_tracing) ? (VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT) : (VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR);
	uint32_t const memory_type_bits = 1U << asset_index_buffer_memory_index;

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
		VMA_MEMORY_USAGE_GPU_ONLY,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0U,
		memory_type_bits,
		VK_NULL_HANDLE,
		NULL,
		1.0F};

	VkResult res_vma_create_buffer = vmaCreateBuffer(asset_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, NULL);
	assert(VK_SUCCESS == res_vma_create_buffer);

	if (support_ray_tracing)
	{
		PFN_vkGetBufferDeviceAddressKHR const pfn_get_buffer_device_address = reinterpret_cast<PFN_vkGetBufferDeviceAddress>(pfn_get_device_proc_addr(device, "vkGetBufferDeviceAddressKHR"));
		assert(NULL != pfn_get_buffer_device_address);

		VkBufferDeviceAddressInfo const buffer_device_address_info = {
			VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			NULL,
			this->m_buffer};
		this->m_device_memory_range_base = pfn_get_buffer_device_address(device, &buffer_device_address_info);
	}
}

void pal_vk_asset_index_buffer::uninit(VmaAllocator asset_allocator)
{
	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(asset_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

pal_vk_asset_index_buffer::~pal_vk_asset_index_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
}

VkBuffer pal_vk_asset_index_buffer::get_buffer() const
{
	return this->m_buffer;
}

VkDeviceAddress pal_vk_asset_index_buffer::get_device_memory_range_base() const
{
	return this->m_device_memory_range_base;
}

pal_vk_scratch_buffer::pal_vk_scratch_buffer() : m_buffer(VK_NULL_HANDLE), m_device_memory(VK_NULL_HANDLE), m_device_memory_range_base(0U)
{
}

void pal_vk_scratch_buffer::init(uint32_t scratch_buffer_memory_index, uint32_t size, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkCreateBuffer const pfn_create_buffer = reinterpret_cast<PFN_vkCreateBuffer>(pfn_get_device_proc_addr(device, "vkCreateBuffer"));
	assert(NULL != pfn_create_buffer);
	PFN_vkGetBufferMemoryRequirements const pfn_get_buffer_memory_requirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(pfn_get_device_proc_addr(device, "vkGetBufferMemoryRequirements"));
	assert(NULL != pfn_get_buffer_memory_requirements);
	PFN_vkAllocateMemory const pfn_allocate_memory = reinterpret_cast<PFN_vkAllocateMemory>(pfn_get_device_proc_addr(device, "vkAllocateMemory"));
	assert(NULL != pfn_allocate_memory);
	PFN_vkBindBufferMemory const pfn_bind_buffer_memory = reinterpret_cast<PFN_vkBindBufferMemory>(pfn_get_device_proc_addr(device, "vkBindBufferMemory"));
	assert(NULL != pfn_bind_buffer_memory);
	PFN_vkGetBufferDeviceAddressKHR const pfn_get_buffer_device_address = reinterpret_cast<PFN_vkGetBufferDeviceAddress>(pfn_get_device_proc_addr(device, "vkGetBufferDeviceAddressKHR"));
	assert(NULL != pfn_get_buffer_device_address);

	assert(VK_NULL_HANDLE == this->m_buffer);
	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};
	VkResult const res_create_buffer = pfn_create_buffer(device, &buffer_create_info, allocation_callbacks, &this->m_buffer);
	assert(VK_SUCCESS == res_create_buffer);

	VkMemoryRequirements memory_requirements;
	pfn_get_buffer_memory_requirements(device, this->m_buffer, &memory_requirements);
	assert(0U != (memory_requirements.memoryTypeBits & (1U << scratch_buffer_memory_index)));

	assert(VK_NULL_HANDLE == this->m_device_memory);
	// maxMemoryAllocationCount >= 4096
	VkMemoryAllocateFlagsInfoKHR const memory_allocate_flags_info = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR,
		NULL,
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
		0U};
	VkMemoryAllocateInfo const memory_allocate_info = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		&memory_allocate_flags_info,
		memory_requirements.size,
		scratch_buffer_memory_index};
	VkResult const res_allocate_memory = pfn_allocate_memory(device, &memory_allocate_info, allocation_callbacks, &this->m_device_memory);
	assert(VK_SUCCESS == res_allocate_memory);

	VkResult const res_bind_buffer_memory = pfn_bind_buffer_memory(device, this->m_buffer, this->m_device_memory, 0U);
	assert(VK_SUCCESS == res_bind_buffer_memory);

	assert(0U == this->m_device_memory_range_base);
	VkBufferDeviceAddressInfo const buffer_device_address_info = {
		VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		NULL,
		this->m_buffer};
	this->m_device_memory_range_base = pfn_get_buffer_device_address(device, &buffer_device_address_info);
}

void pal_vk_scratch_buffer::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkDestroyBuffer const pfn_destroy_buffer = reinterpret_cast<PFN_vkDestroyBuffer>(pfn_get_device_proc_addr(device, "vkDestroyBuffer"));
	assert(NULL != pfn_destroy_buffer);
	PFN_vkFreeMemory const pfn_free_memory = reinterpret_cast<PFN_vkFreeMemory>(pfn_get_device_proc_addr(device, "vkFreeMemory"));
	assert(NULL != pfn_free_memory);

	assert(VK_NULL_HANDLE != this->m_buffer);
	pfn_destroy_buffer(device, this->m_buffer, allocation_callbacks);
	this->m_buffer = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_device_memory);
	pfn_free_memory(device, this->m_device_memory, allocation_callbacks);
	this->m_device_memory = VK_NULL_HANDLE;
}

pal_vk_scratch_buffer::~pal_vk_scratch_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_device_memory);
}

VkBuffer pal_vk_scratch_buffer::get_buffer() const
{
	return this->m_buffer;
}

VkDeviceAddress pal_vk_scratch_buffer::get_device_memory_range_base() const
{
	return this->m_device_memory_range_base;
}

pal_vk_staging_non_compacted_bottom_level_acceleration_structure_buffer::pal_vk_staging_non_compacted_bottom_level_acceleration_structure_buffer() : m_buffer(VK_NULL_HANDLE), m_device_memory(VK_NULL_HANDLE)
{
}

void pal_vk_staging_non_compacted_bottom_level_acceleration_structure_buffer::init(uint32_t staging_non_compacted_bottom_level_acceleration_structure_buffer_memory_index, uint32_t size, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkCreateBuffer const pfn_create_buffer = reinterpret_cast<PFN_vkCreateBuffer>(pfn_get_device_proc_addr(device, "vkCreateBuffer"));
	assert(NULL != pfn_create_buffer);
	PFN_vkGetBufferMemoryRequirements const pfn_get_buffer_memory_requirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(pfn_get_device_proc_addr(device, "vkGetBufferMemoryRequirements"));
	assert(NULL != pfn_get_buffer_memory_requirements);
	PFN_vkAllocateMemory const pfn_allocate_memory = reinterpret_cast<PFN_vkAllocateMemory>(pfn_get_device_proc_addr(device, "vkAllocateMemory"));
	assert(NULL != pfn_allocate_memory);
	PFN_vkBindBufferMemory const pfn_bind_buffer_memory = reinterpret_cast<PFN_vkBindBufferMemory>(pfn_get_device_proc_addr(device, "vkBindBufferMemory"));
	assert(NULL != pfn_bind_buffer_memory);

	assert(VK_NULL_HANDLE == this->m_buffer);
	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};
	VkResult const res_create_buffer = pfn_create_buffer(device, &buffer_create_info, allocation_callbacks, &this->m_buffer);
	assert(VK_SUCCESS == res_create_buffer);

	VkMemoryRequirements memory_requirements;
	pfn_get_buffer_memory_requirements(device, this->m_buffer, &memory_requirements);
	assert(0U != (memory_requirements.memoryTypeBits & (1U << staging_non_compacted_bottom_level_acceleration_structure_buffer_memory_index)));

	assert(VK_NULL_HANDLE == this->m_device_memory);
	// maxMemoryAllocationCount >= 4096
	VkMemoryAllocateInfo const memory_allocate_info = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		NULL,
		memory_requirements.size,
		staging_non_compacted_bottom_level_acceleration_structure_buffer_memory_index};
	VkResult const res_allocate_memory = pfn_allocate_memory(device, &memory_allocate_info, allocation_callbacks, &this->m_device_memory);
	assert(VK_SUCCESS == res_allocate_memory);

	VkResult const res_bind_buffer_memory = pfn_bind_buffer_memory(device, this->m_buffer, this->m_device_memory, 0U);
	assert(VK_SUCCESS == res_bind_buffer_memory);
}

void pal_vk_staging_non_compacted_bottom_level_acceleration_structure_buffer::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkDestroyBuffer const pfn_destroy_buffer = reinterpret_cast<PFN_vkDestroyBuffer>(pfn_get_device_proc_addr(device, "vkDestroyBuffer"));
	assert(NULL != pfn_destroy_buffer);
	PFN_vkFreeMemory const pfn_free_memory = reinterpret_cast<PFN_vkFreeMemory>(pfn_get_device_proc_addr(device, "vkFreeMemory"));
	assert(NULL != pfn_free_memory);

	assert(VK_NULL_HANDLE != this->m_buffer);
	pfn_destroy_buffer(device, this->m_buffer, allocation_callbacks);
	this->m_buffer = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_device_memory);
	pfn_free_memory(device, this->m_device_memory, allocation_callbacks);
	this->m_device_memory = VK_NULL_HANDLE;
}

pal_vk_staging_non_compacted_bottom_level_acceleration_structure_buffer::~pal_vk_staging_non_compacted_bottom_level_acceleration_structure_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_device_memory);
}

VkBuffer pal_vk_staging_non_compacted_bottom_level_acceleration_structure_buffer::get_buffer() const
{
	return this->m_buffer;
}

pal_vk_staging_non_compacted_bottom_level_acceleration_structure::pal_vk_staging_non_compacted_bottom_level_acceleration_structure() : m_acceleration_structure(VK_NULL_HANDLE)
{
}

void pal_vk_staging_non_compacted_bottom_level_acceleration_structure::init(pal_staging_non_compacted_bottom_level_acceleration_structure_buffer const *wrapped_staging_non_compacted_bottom_level_acceleration_structure_buffer, uint32_t offset, uint32_t size, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	assert(NULL != wrapped_staging_non_compacted_bottom_level_acceleration_structure_buffer);
	VkBuffer const staging_non_compacted_bottom_level_acceleration_structure_buffer = static_cast<pal_vk_staging_non_compacted_bottom_level_acceleration_structure_buffer const *>(wrapped_staging_non_compacted_bottom_level_acceleration_structure_buffer)->get_buffer();

	PFN_vkCreateAccelerationStructureKHR const pfn_create_acceleration_structure = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(pfn_get_device_proc_addr(device, "vkCreateAccelerationStructureKHR"));
	assert(NULL != pfn_create_acceleration_structure);

	VkAccelerationStructureCreateInfoKHR const acceleration_structure_create_info = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		NULL,
		0U,
		staging_non_compacted_bottom_level_acceleration_structure_buffer,
		offset,
		size,
		VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		0U};

	assert(VK_NULL_HANDLE == this->m_acceleration_structure);
	pfn_create_acceleration_structure(device, &acceleration_structure_create_info, allocation_callbacks, &this->m_acceleration_structure);
}

void pal_vk_staging_non_compacted_bottom_level_acceleration_structure::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkDestroyAccelerationStructureKHR const pfn_destroy_acceleration_structure = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(pfn_get_device_proc_addr(device, "vkDestroyAccelerationStructureKHR"));
	assert(NULL != pfn_destroy_acceleration_structure);

	assert(VK_NULL_HANDLE != this->m_acceleration_structure);

	pfn_destroy_acceleration_structure(device, this->m_acceleration_structure, allocation_callbacks);

	this->m_acceleration_structure = VK_NULL_HANDLE;
}

pal_vk_staging_non_compacted_bottom_level_acceleration_structure::~pal_vk_staging_non_compacted_bottom_level_acceleration_structure()
{
	assert(VK_NULL_HANDLE == this->m_acceleration_structure);
}

VkAccelerationStructureKHR pal_vk_staging_non_compacted_bottom_level_acceleration_structure::get_acceleration_structure() const
{
	return this->m_acceleration_structure;
}

pal_vk_compacted_bottom_level_acceleration_structure_size_query_pool::pal_vk_compacted_bottom_level_acceleration_structure_size_query_pool() : m_query_pool(VK_NULL_HANDLE)
{
}

void pal_vk_compacted_bottom_level_acceleration_structure_size_query_pool::init(uint32_t query_count, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
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

void pal_vk_compacted_bottom_level_acceleration_structure_size_query_pool::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkDestroyQueryPool const pfn_destroy_query_pool = reinterpret_cast<PFN_vkDestroyQueryPool>(pfn_get_device_proc_addr(device, "vkDestroyQueryPool"));
	assert(NULL != pfn_destroy_query_pool);

	assert(VK_NULL_HANDLE != this->m_query_pool);
	pfn_destroy_query_pool(device, this->m_query_pool, allocation_callbacks);
	this->m_query_pool = VK_NULL_HANDLE;
}

pal_vk_compacted_bottom_level_acceleration_structure_size_query_pool::~pal_vk_compacted_bottom_level_acceleration_structure_size_query_pool()
{
	assert(VK_NULL_HANDLE == this->m_query_pool);
}

VkQueryPool pal_vk_compacted_bottom_level_acceleration_structure_size_query_pool::get_query_pool() const
{
	return this->m_query_pool;
}

pal_vk_asset_compacted_bottom_level_acceleration_structure::pal_vk_asset_compacted_bottom_level_acceleration_structure() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_acceleration_structure(VK_NULL_HANDLE), m_device_memory_range_base(0U)
{
}

void pal_vk_asset_compacted_bottom_level_acceleration_structure::init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, uint32_t asset_compacted_bottom_level_acceleration_structure_memory_index, VmaAllocator asset_allocator, uint32_t size)
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
		VMA_MEMORY_USAGE_GPU_ONLY,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0U,
		1U << asset_compacted_bottom_level_acceleration_structure_memory_index,
		VK_NULL_HANDLE,
		NULL,
		1.0F};

	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	VkResult const res_vma_create_buffer = vmaCreateBuffer(asset_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, NULL);
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

void pal_vk_asset_compacted_bottom_level_acceleration_structure::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, VmaAllocator asset_allocator)
{
	PFN_vkDestroyAccelerationStructureKHR const pfn_destroy_acceleration_structure = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(pfn_get_device_proc_addr(device, "vkDestroyAccelerationStructureKHR"));
	assert(NULL != pfn_destroy_acceleration_structure);

	assert(VK_NULL_HANDLE != this->m_acceleration_structure);

	pfn_destroy_acceleration_structure(device, this->m_acceleration_structure, allocation_callbacks);

	this->m_acceleration_structure = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(asset_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

pal_vk_asset_compacted_bottom_level_acceleration_structure::~pal_vk_asset_compacted_bottom_level_acceleration_structure()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	assert(VK_NULL_HANDLE == this->m_acceleration_structure);
}

VkBuffer pal_vk_asset_compacted_bottom_level_acceleration_structure::get_buffer() const
{
	return this->m_buffer;
}

VkAccelerationStructureKHR pal_vk_asset_compacted_bottom_level_acceleration_structure::get_acceleration_structure() const
{
	return this->m_acceleration_structure;
}

VkDeviceAddress pal_vk_asset_compacted_bottom_level_acceleration_structure::get_device_memory_range_base() const
{
	return this->m_device_memory_range_base;
}

pal_vk_top_level_acceleration_structure_instance_buffer::pal_vk_top_level_acceleration_structure_instance_buffer() : m_buffer(VK_NULL_HANDLE), m_device_memory(VK_NULL_HANDLE), m_device_memory_range_base(0U), m_memory_range_base(NULL)
{
}

void pal_vk_top_level_acceleration_structure_instance_buffer::init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, uint32_t top_level_acceleration_structure_instance_buffer_memory_index, uint32_t instance_count)
{
	uint32_t const size = sizeof(VkAccelerationStructureInstanceKHR) * instance_count;

	PFN_vkCreateBuffer const pfn_create_buffer = reinterpret_cast<PFN_vkCreateBuffer>(pfn_get_device_proc_addr(device, "vkCreateBuffer"));
	assert(NULL != pfn_create_buffer);
	PFN_vkGetBufferMemoryRequirements const pfn_get_buffer_memory_requirements = reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(pfn_get_device_proc_addr(device, "vkGetBufferMemoryRequirements"));
	assert(NULL != pfn_get_buffer_memory_requirements);
	PFN_vkAllocateMemory const pfn_allocate_memory = reinterpret_cast<PFN_vkAllocateMemory>(pfn_get_device_proc_addr(device, "vkAllocateMemory"));
	assert(NULL != pfn_allocate_memory);
	PFN_vkBindBufferMemory const pfn_bind_buffer_memory = reinterpret_cast<PFN_vkBindBufferMemory>(pfn_get_device_proc_addr(device, "vkBindBufferMemory"));
	assert(NULL != pfn_bind_buffer_memory);
	PFN_vkGetBufferDeviceAddressKHR const pfn_get_buffer_device_address = reinterpret_cast<PFN_vkGetBufferDeviceAddress>(pfn_get_device_proc_addr(device, "vkGetBufferDeviceAddressKHR"));
	assert(NULL != pfn_get_buffer_device_address);
	PFN_vkMapMemory const pfn_map_memory = reinterpret_cast<PFN_vkMapMemory>(pfn_get_device_proc_addr(device, "vkMapMemory"));
	assert(NULL != pfn_map_memory);

	assert(VK_NULL_HANDLE == this->m_buffer);
	VkBufferCreateInfo const buffer_create_info = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0U,
		size,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR,
		VK_SHARING_MODE_EXCLUSIVE,
		0U,
		NULL};
	VkResult const res_create_buffer = pfn_create_buffer(device, &buffer_create_info, allocation_callbacks, &this->m_buffer);
	assert(VK_SUCCESS == res_create_buffer);

	VkMemoryRequirements memory_requirements;
	pfn_get_buffer_memory_requirements(device, this->m_buffer, &memory_requirements);
	assert(0U != (memory_requirements.memoryTypeBits & (1U << top_level_acceleration_structure_instance_buffer_memory_index)));

	assert(VK_NULL_HANDLE == this->m_device_memory);
	// maxMemoryAllocationCount >= 4096
	VkMemoryAllocateFlagsInfoKHR const memory_allocate_flags_info = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR,
		NULL,
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
		0U};
	VkMemoryAllocateInfo const memory_allocate_info = {
		VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		&memory_allocate_flags_info,
		memory_requirements.size,
		top_level_acceleration_structure_instance_buffer_memory_index};
	VkResult const res_allocate_memory = pfn_allocate_memory(device, &memory_allocate_info, allocation_callbacks, &this->m_device_memory);
	assert(VK_SUCCESS == res_allocate_memory);

	VkResult const res_bind_buffer_memory = pfn_bind_buffer_memory(device, this->m_buffer, this->m_device_memory, 0U);
	assert(VK_SUCCESS == res_bind_buffer_memory);

	assert(0U == this->m_device_memory_range_base);
	VkBufferDeviceAddressInfo const buffer_device_address_info = {
		VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		NULL,
		this->m_buffer};
	this->m_device_memory_range_base = pfn_get_buffer_device_address(device, &buffer_device_address_info);

	void *memory_range_base = NULL;
	VkResult const res_map_memory = pfn_map_memory(device, this->m_device_memory, 0U, size, 0U, &memory_range_base);
	assert(VK_SUCCESS == res_map_memory);

	assert(NULL == this->m_memory_range_base);
	this->m_memory_range_base = static_cast<VkAccelerationStructureInstanceKHR *>(memory_range_base);
}

void pal_vk_top_level_acceleration_structure_instance_buffer::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkUnmapMemory const pfn_unmap_memory = reinterpret_cast<PFN_vkUnmapMemory>(pfn_get_device_proc_addr(device, "vkUnmapMemory"));
	assert(NULL != pfn_unmap_memory);
	PFN_vkDestroyBuffer const pfn_destroy_buffer = reinterpret_cast<PFN_vkDestroyBuffer>(pfn_get_device_proc_addr(device, "vkDestroyBuffer"));
	assert(NULL != pfn_destroy_buffer);
	PFN_vkFreeMemory const pfn_free_memory = reinterpret_cast<PFN_vkFreeMemory>(pfn_get_device_proc_addr(device, "vkFreeMemory"));
	assert(NULL != pfn_free_memory);

	pfn_unmap_memory(device, this->m_device_memory);

	assert(VK_NULL_HANDLE != this->m_buffer);
	pfn_destroy_buffer(device, this->m_buffer, allocation_callbacks);
	this->m_buffer = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_device_memory);
	pfn_free_memory(device, this->m_device_memory, allocation_callbacks);
	this->m_device_memory = VK_NULL_HANDLE;
}

pal_vk_top_level_acceleration_structure_instance_buffer::~pal_vk_top_level_acceleration_structure_instance_buffer()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_device_memory);
}

void pal_vk_top_level_acceleration_structure_instance_buffer::write_instance(uint32_t instance_index, float const transform_matrix[3][4], bool force_closest_hit, bool force_any_hit, bool disable_back_face_cull, bool front_ccw, pal_asset_compacted_bottom_level_acceleration_structure *wrapped_asset_compacted_bottom_level_acceleration_structure)
{
	pal_vk_asset_compacted_bottom_level_acceleration_structure *const unwrapped_asset_compacted_bottom_level_acceleration_structure = static_cast<pal_vk_asset_compacted_bottom_level_acceleration_structure *>(wrapped_asset_compacted_bottom_level_acceleration_structure);
	VkDeviceAddress const asset_compacted_bottom_level_acceleration_structure_device_memory_range_base = unwrapped_asset_compacted_bottom_level_acceleration_structure->get_device_memory_range_base();

	VkAccelerationStructureInstanceKHR &destination_top_level_acceleration_structure_instance = this->m_memory_range_base[instance_index];

	destination_top_level_acceleration_structure_instance.transform.matrix[0][0] = transform_matrix[0][0];
	destination_top_level_acceleration_structure_instance.transform.matrix[0][1] = transform_matrix[0][1];
	destination_top_level_acceleration_structure_instance.transform.matrix[0][2] = transform_matrix[0][2];
	destination_top_level_acceleration_structure_instance.transform.matrix[0][3] = transform_matrix[0][3];
	destination_top_level_acceleration_structure_instance.transform.matrix[1][0] = transform_matrix[1][0];
	destination_top_level_acceleration_structure_instance.transform.matrix[1][1] = transform_matrix[1][1];
	destination_top_level_acceleration_structure_instance.transform.matrix[1][2] = transform_matrix[1][2];
	destination_top_level_acceleration_structure_instance.transform.matrix[1][3] = transform_matrix[1][3];
	destination_top_level_acceleration_structure_instance.transform.matrix[2][0] = transform_matrix[2][0];
	destination_top_level_acceleration_structure_instance.transform.matrix[2][1] = transform_matrix[2][1];
	destination_top_level_acceleration_structure_instance.transform.matrix[2][2] = transform_matrix[2][2];
	destination_top_level_acceleration_structure_instance.transform.matrix[2][3] = transform_matrix[2][3];
	destination_top_level_acceleration_structure_instance.instanceCustomIndex = 0U;
	destination_top_level_acceleration_structure_instance.mask = 0XFFU;
	destination_top_level_acceleration_structure_instance.instanceShaderBindingTableRecordOffset = 0U;
	destination_top_level_acceleration_structure_instance.flags = (force_closest_hit ? VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR : 0U) | (force_any_hit ? VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR : 0U) | (disable_back_face_cull ? VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR : 0U) | (front_ccw ? VK_GEOMETRY_INSTANCE_TRIANGLE_FLIP_FACING_BIT_KHR : 0U);
	destination_top_level_acceleration_structure_instance.accelerationStructureReference = asset_compacted_bottom_level_acceleration_structure_device_memory_range_base;
}

VkBuffer pal_vk_top_level_acceleration_structure_instance_buffer::get_buffer() const
{
	return this->m_buffer;
}

VkDeviceAddress pal_vk_top_level_acceleration_structure_instance_buffer::get_device_memory_range_base() const
{
	return this->m_device_memory_range_base;
}

pal_vk_top_level_acceleration_structure::pal_vk_top_level_acceleration_structure() : m_buffer(VK_NULL_HANDLE), m_allocation(VK_NULL_HANDLE), m_acceleration_structure(VK_NULL_HANDLE)
{
}

void pal_vk_top_level_acceleration_structure::init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, uint32_t top_level_acceleration_structure_memory_index, VmaAllocator asset_allocator, uint32_t size)
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
		VMA_MEMORY_USAGE_GPU_ONLY,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		0U,
		1U << top_level_acceleration_structure_memory_index,
		VK_NULL_HANDLE,
		NULL,
		1.0F};

	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	VkResult const res_vma_create_buffer = vmaCreateBuffer(asset_allocator, &buffer_create_info, &allocation_create_info, &this->m_buffer, &this->m_allocation, NULL);
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

void pal_vk_top_level_acceleration_structure::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, VmaAllocator asset_allocator)
{
	PFN_vkDestroyAccelerationStructureKHR const pfn_destroy_acceleration_structure = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(pfn_get_device_proc_addr(device, "vkDestroyAccelerationStructureKHR"));
	assert(NULL != pfn_destroy_acceleration_structure);

	assert(VK_NULL_HANDLE != this->m_acceleration_structure);

	pfn_destroy_acceleration_structure(device, this->m_acceleration_structure, allocation_callbacks);

	this->m_acceleration_structure = VK_NULL_HANDLE;

	assert(VK_NULL_HANDLE != this->m_buffer);
	assert(VK_NULL_HANDLE != this->m_allocation);

	vmaDestroyBuffer(asset_allocator, this->m_buffer, this->m_allocation);

	this->m_buffer = VK_NULL_HANDLE;
	this->m_allocation = VK_NULL_HANDLE;
}

pal_vk_top_level_acceleration_structure::~pal_vk_top_level_acceleration_structure()
{
	assert(VK_NULL_HANDLE == this->m_buffer);
	assert(VK_NULL_HANDLE == this->m_allocation);
	assert(VK_NULL_HANDLE == this->m_acceleration_structure);
}

VkBuffer pal_vk_top_level_acceleration_structure::get_buffer() const
{
	return this->m_buffer;
}

VkAccelerationStructureKHR pal_vk_top_level_acceleration_structure::get_acceleration_structure() const
{
	return this->m_acceleration_structure;
}
