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

#include "pal_d3d12_device.h"
#include <assert.h>

pal_d3d12_upload_ring_buffer::pal_d3d12_upload_ring_buffer() : m_heap(NULL), m_resource(NULL), m_memory_range_base(NULL)
{
}

void pal_d3d12_upload_ring_buffer::init(ID3D12Device *device, bool cache_coherent_uma, uint32_t size)
{
	D3D12_RESOURCE_DESC const resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE};

	D3D12_RESOURCE_ALLOCATION_INFO const resource_allocation_info = device->GetResourceAllocationInfo(0U, 1U, &resource_desc);

	D3D12_HEAP_DESC const heap_desc = {
		resource_allocation_info.SizeInBytes,
		{D3D12_HEAP_TYPE_CUSTOM,
		 cache_coherent_uma ? D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE : D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
		 D3D12_MEMORY_POOL_L0,
		 0U,
		 0U},
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES};

	assert(NULL == this->m_heap);
	HRESULT const hr_create_heap = device->CreateHeap(&heap_desc, IID_PPV_ARGS(&this->m_heap));
	assert(SUCCEEDED(hr_create_heap));

	assert(NULL == this->m_resource);
	HRESULT const hr_create_placed_resource = device->CreatePlacedResource(this->m_heap, 0U, &resource_desc, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_placed_resource));

	assert(NULL == this->m_memory_range_base);
	D3D12_RANGE const read_range = {0U, 0U};
	HRESULT const hr_map = this->m_resource->Map(0U, &read_range, &this->m_memory_range_base);
	assert(SUCCEEDED(hr_map));
}

void pal_d3d12_upload_ring_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Unmap(0U, NULL);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_heap);
	this->m_heap->Release();
	this->m_heap = NULL;
}

pal_d3d12_upload_ring_buffer::~pal_d3d12_upload_ring_buffer()
{
	assert(NULL == this->m_heap);
	assert(NULL == this->m_resource);
}

ID3D12Resource *pal_d3d12_upload_ring_buffer::get_resource() const
{
	return this->m_resource;
}

void *pal_d3d12_upload_ring_buffer::get_memory_range_base() const
{
	return this->m_memory_range_base;
}

pal_d3d12_staging_buffer::pal_d3d12_staging_buffer() : m_heap(NULL), m_resource(NULL), m_memory_range_base(NULL)
{
}

void pal_d3d12_staging_buffer::init(ID3D12Device *device, bool cache_coherent_uma, uint32_t size)
{
	D3D12_RESOURCE_DESC const resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE};

	D3D12_RESOURCE_ALLOCATION_INFO const resource_allocation_info = device->GetResourceAllocationInfo(0U, 1U, &resource_desc);

	D3D12_HEAP_DESC const heap_desc = {
		resource_allocation_info.SizeInBytes,
		{D3D12_HEAP_TYPE_CUSTOM,
		 cache_coherent_uma ? D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE : D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
		 D3D12_MEMORY_POOL_L0,
		 0U,
		 0U},
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES};

	assert(NULL == this->m_heap);
	HRESULT const hr_create_heap = device->CreateHeap(&heap_desc, IID_PPV_ARGS(&this->m_heap));
	assert(SUCCEEDED(hr_create_heap));

	assert(NULL == this->m_resource);
	HRESULT const hr_create_placed_resource = device->CreatePlacedResource(this->m_heap, 0U, &resource_desc, D3D12_RESOURCE_STATE_COPY_SOURCE, NULL, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_placed_resource));

	assert(NULL == this->m_memory_range_base);
	D3D12_RANGE const read_range = {0U, 0U};
	HRESULT const hr_map = this->m_resource->Map(0U, &read_range, &this->m_memory_range_base);
	assert(SUCCEEDED(hr_map));
}

void pal_d3d12_staging_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Unmap(0U, NULL);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_heap);
	this->m_heap->Release();
	this->m_heap = NULL;
}

pal_d3d12_staging_buffer::~pal_d3d12_staging_buffer()
{
	assert(NULL == this->m_heap);
	assert(NULL == this->m_resource);
}

ID3D12Resource *pal_d3d12_staging_buffer::get_resource() const
{
	return this->m_resource;
}

void *pal_d3d12_staging_buffer::get_memory_range_base() const
{
	return this->m_memory_range_base;
}

pal_d3d12_asset_vertex_position_buffer::pal_d3d12_asset_vertex_position_buffer() : m_resource(NULL), m_allocation(NULL)
{
}

void pal_d3d12_asset_vertex_position_buffer::init(bool uma, D3D12MA::Allocator *asset_allocator, D3D12MA::Pool *asset_buffer_pool, uint32_t size)
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);

	D3D12MA::ALLOCATION_DESC allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		asset_buffer_pool,
		NULL};

	D3D12_RESOURCE_DESC resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE};

	HRESULT hr_create_resource = asset_allocator->CreateResource(&allocation_desc, &resource_desc, (!uma) ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COMMON, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));
}

void pal_d3d12_asset_vertex_position_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

pal_d3d12_asset_vertex_position_buffer::~pal_d3d12_asset_vertex_position_buffer()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *pal_d3d12_asset_vertex_position_buffer::get_resource() const
{
	return this->m_resource;
}

pal_vertex_buffer const *pal_d3d12_asset_vertex_position_buffer::get_vertex_buffer() const
{
	return static_cast<pal_d3d12_vertex_buffer const *>(this);
}

pal_d3d12_asset_vertex_varying_buffer::pal_d3d12_asset_vertex_varying_buffer() : m_resource(NULL), m_allocation(NULL)
{
}

void pal_d3d12_asset_vertex_varying_buffer::init(bool uma, D3D12MA::Allocator *asset_allocator, D3D12MA::Pool *asset_buffer_pool, uint32_t size)
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);

	D3D12MA::ALLOCATION_DESC allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		asset_buffer_pool,
		NULL};

	D3D12_RESOURCE_DESC resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE};

	HRESULT hr_create_resource = asset_allocator->CreateResource(&allocation_desc, &resource_desc, (!uma) ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COMMON, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));
}

void pal_d3d12_asset_vertex_varying_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

pal_d3d12_asset_vertex_varying_buffer::~pal_d3d12_asset_vertex_varying_buffer()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *pal_d3d12_asset_vertex_varying_buffer::get_resource() const
{
	return this->m_resource;
}

pal_vertex_buffer const *pal_d3d12_asset_vertex_varying_buffer::get_vertex_buffer() const
{
	return static_cast<pal_d3d12_vertex_buffer const *>(this);
}

pal_d3d12_asset_index_buffer::pal_d3d12_asset_index_buffer() : m_resource(NULL), m_allocation(NULL)
{
}

void pal_d3d12_asset_index_buffer::init(bool uma, D3D12MA::Allocator *asset_allocator, D3D12MA::Pool *asset_buffer_pool, uint32_t size)
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);

	D3D12MA::ALLOCATION_DESC allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		asset_buffer_pool,
		NULL};

	D3D12_RESOURCE_DESC resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE};

	HRESULT hr_create_resource = asset_allocator->CreateResource(&allocation_desc, &resource_desc, (!uma) ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COMMON, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));
}

void pal_d3d12_asset_index_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

pal_d3d12_asset_index_buffer::~pal_d3d12_asset_index_buffer()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *pal_d3d12_asset_index_buffer::get_resource() const
{
	return this->m_resource;
}

pal_d3d12_scratch_buffer::pal_d3d12_scratch_buffer() : m_heap(NULL), m_resource(NULL)
{
}

void pal_d3d12_scratch_buffer::init(ID3D12Device *device, bool uma, uint32_t size)
{
	D3D12_RESOURCE_DESC const resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS};

	D3D12_RESOURCE_ALLOCATION_INFO const resource_allocation_info = device->GetResourceAllocationInfo(0U, 1U, &resource_desc);

	D3D12_HEAP_DESC const heap_desc = {
		resource_allocation_info.SizeInBytes,
		{D3D12_HEAP_TYPE_CUSTOM,
		 D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE,
		 uma ? D3D12_MEMORY_POOL_L0 : D3D12_MEMORY_POOL_L1,
		 0U,
		 0U},
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES};

	assert(NULL == this->m_heap);
	HRESULT const hr_create_heap = device->CreateHeap(&heap_desc, IID_PPV_ARGS(&this->m_heap));
	assert(SUCCEEDED(hr_create_heap));

	assert(NULL == this->m_resource);
	HRESULT const hr_create_placed_resource = device->CreatePlacedResource(this->m_heap, 0U, &resource_desc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_placed_resource));
}

void pal_d3d12_scratch_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_heap);
	this->m_heap->Release();
	this->m_heap = NULL;
}

pal_d3d12_scratch_buffer::~pal_d3d12_scratch_buffer()
{
	assert(NULL == this->m_heap);
	assert(NULL == this->m_resource);
}

ID3D12Resource *pal_d3d12_scratch_buffer::get_resource() const
{
	return this->m_resource;
}

pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure_buffer::pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure_buffer() : m_heap(NULL), m_resource(NULL)
{
}

void pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure_buffer::init(ID3D12Device *device, bool uma, uint32_t size)
{
	D3D12_RESOURCE_DESC const resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS};

	D3D12_RESOURCE_ALLOCATION_INFO const resource_allocation_info = device->GetResourceAllocationInfo(0U, 1U, &resource_desc);

	D3D12_HEAP_DESC const heap_desc = {
		resource_allocation_info.SizeInBytes,
		{D3D12_HEAP_TYPE_CUSTOM,
		 D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE,
		 uma ? D3D12_MEMORY_POOL_L0 : D3D12_MEMORY_POOL_L1,
		 0U,
		 0U},
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES};

	assert(NULL == this->m_heap);
	HRESULT const hr_create_heap = device->CreateHeap(&heap_desc, IID_PPV_ARGS(&this->m_heap));
	assert(SUCCEEDED(hr_create_heap));

	assert(NULL == this->m_resource);
	HRESULT const hr_create_placed_resource = device->CreatePlacedResource(this->m_heap, 0U, &resource_desc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_placed_resource));
}

void pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_heap);
	this->m_heap->Release();
	this->m_heap = NULL;
}

pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure_buffer::~pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure_buffer()
{
	assert(NULL == this->m_heap);
	assert(NULL == this->m_resource);
}

ID3D12Resource *pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure_buffer::get_resource() const
{
	return this->m_resource;
}

pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure::pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure() : m_device_memory_range_base(0U)
{
}

void pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure::init(pal_staging_non_compacted_bottom_level_acceleration_structure_buffer const *wrapped_staging_non_compacted_bottom_level_acceleration_structure_buffer, uint32_t offset, uint32_t size)
{
	assert(NULL != wrapped_staging_non_compacted_bottom_level_acceleration_structure_buffer);
	pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure_buffer const *const unwrapped_staging_non_compacted_bottom_level_acceleration_structure_buffer = static_cast<pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure_buffer const *>(wrapped_staging_non_compacted_bottom_level_acceleration_structure_buffer);

	assert(0U == this->m_device_memory_range_base);
	this->m_device_memory_range_base = unwrapped_staging_non_compacted_bottom_level_acceleration_structure_buffer->get_resource()->GetGPUVirtualAddress() + offset;
}

void pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure::uninit()
{
}

pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure::~pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure()
{
}

D3D12_GPU_VIRTUAL_ADDRESS pal_d3d12_staging_non_compacted_bottom_level_acceleration_structure::get_device_memory_range_base() const
{
	return this->m_device_memory_range_base;
}

pal_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool::pal_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool() : m_heap(NULL), m_resource(NULL), m_memory_range_base(NULL)
{
}

void pal_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool::init(ID3D12Device *device, uint32_t query_count)
{
	uint32_t const size = sizeof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC) * query_count;

	static_assert(0U == (D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT % alignof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC)), "");
	D3D12_RESOURCE_DESC const resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS};

	D3D12_RESOURCE_ALLOCATION_INFO const resource_allocation_info = device->GetResourceAllocationInfo(0U, 1U, &resource_desc);

	D3D12_HEAP_DESC const heap_desc = {
		resource_allocation_info.SizeInBytes,
		{D3D12_HEAP_TYPE_CUSTOM,
		 D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
		 D3D12_MEMORY_POOL_L0,
		 0U,
		 0U},
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES};

	assert(NULL == this->m_heap);
	HRESULT const hr_create_heap = device->CreateHeap(&heap_desc, IID_PPV_ARGS(&this->m_heap));
	assert(SUCCEEDED(hr_create_heap));

	assert(NULL == this->m_resource);
	HRESULT const hr_create_placed_resource = device->CreatePlacedResource(this->m_heap, 0U, &resource_desc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_placed_resource));

	void *memory_range_base = NULL;
	D3D12_RANGE const read_range = {0U, size};
	HRESULT const hr_map = this->m_resource->Map(0U, &read_range, &memory_range_base);
	assert(SUCCEEDED(hr_map));

	assert(NULL == this->m_memory_range_base);
	this->m_memory_range_base = static_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC volatile *>(const_cast<void volatile *>(memory_range_base));
}

void pal_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Unmap(0U, NULL);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_heap);
	this->m_heap->Release();
	this->m_heap = NULL;
}

pal_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool::~pal_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool()
{
	assert(NULL == this->m_heap);
	assert(NULL == this->m_resource);
}

ID3D12Resource *pal_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool::get_resource() const
{
	return this->m_resource;
}

D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC volatile *pal_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool::get_memory_range_base() const
{
	return this->m_memory_range_base;
}

pal_d3d12_asset_compacted_bottom_level_acceleration_structure::pal_d3d12_asset_compacted_bottom_level_acceleration_structure() : m_resource(NULL), m_allocation(NULL)
{
}

void pal_d3d12_asset_compacted_bottom_level_acceleration_structure::init(D3D12MA::Allocator *asset_allocator, D3D12MA::Pool *asset_compacted_bottom_level_acceleration_structure_pool, uint32_t size)
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);

	D3D12MA::ALLOCATION_DESC const allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		asset_compacted_bottom_level_acceleration_structure_pool,
		NULL};

	D3D12_RESOURCE_DESC const resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS};

	HRESULT const hr_create_resource = asset_allocator->CreateResource(&allocation_desc, &resource_desc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));
}

void pal_d3d12_asset_compacted_bottom_level_acceleration_structure::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

pal_d3d12_asset_compacted_bottom_level_acceleration_structure::~pal_d3d12_asset_compacted_bottom_level_acceleration_structure()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *pal_d3d12_asset_compacted_bottom_level_acceleration_structure::get_resource() const
{
	return this->m_resource;
}

pal_d3d12_top_level_acceleration_structure_instance_buffer::pal_d3d12_top_level_acceleration_structure_instance_buffer() : m_heap(NULL), m_resource(NULL), m_memory_range_base(NULL)
{
}

void pal_d3d12_top_level_acceleration_structure_instance_buffer::init(ID3D12Device *device, uint32_t instance_count)
{
	uint32_t const size = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instance_count;

	static_assert(0U == (D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT % alignof(D3D12_RAYTRACING_INSTANCE_DESC)), "");
	D3D12_RESOURCE_DESC const resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE};

	D3D12_RESOURCE_ALLOCATION_INFO const resource_allocation_info = device->GetResourceAllocationInfo(0U, 1U, &resource_desc);

	D3D12_HEAP_DESC const heap_desc = {
		resource_allocation_info.SizeInBytes,
		{D3D12_HEAP_TYPE_CUSTOM,
		 D3D12_CPU_PAGE_PROPERTY_WRITE_BACK,
		 D3D12_MEMORY_POOL_L0,
		 0U,
		 0U},
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES};

	assert(NULL == this->m_heap);
	HRESULT const hr_create_heap = device->CreateHeap(&heap_desc, IID_PPV_ARGS(&this->m_heap));
	assert(SUCCEEDED(hr_create_heap));

	assert(NULL == this->m_resource);
	HRESULT const hr_create_placed_resource = device->CreatePlacedResource(this->m_heap, 0U, &resource_desc, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, NULL, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_placed_resource));

	void *memory_range_base = NULL;
	D3D12_RANGE const read_range = {0U, size};
	HRESULT const hr_map = this->m_resource->Map(0U, &read_range, &memory_range_base);
	assert(SUCCEEDED(hr_map));

	assert(NULL == this->m_memory_range_base);
	this->m_memory_range_base = static_cast<D3D12_RAYTRACING_INSTANCE_DESC *>(memory_range_base);
}

void pal_d3d12_top_level_acceleration_structure_instance_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Unmap(0U, NULL);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_heap);
	this->m_heap->Release();
	this->m_heap = NULL;
}

pal_d3d12_top_level_acceleration_structure_instance_buffer::~pal_d3d12_top_level_acceleration_structure_instance_buffer()
{
	assert(NULL == this->m_heap);
	assert(NULL == this->m_resource);
}

void pal_d3d12_top_level_acceleration_structure_instance_buffer::write_instance(uint32_t instance_index, float const transform_matrix[3][4], bool force_closest_hit, bool force_any_hit, bool disable_back_face_cull, bool front_ccw, pal_asset_compacted_bottom_level_acceleration_structure *wrapped_asset_compacted_bottom_level_acceleration_structure)
{
	pal_d3d12_asset_compacted_bottom_level_acceleration_structure *const unwrapped_asset_compacted_bottom_level_acceleration_structure = static_cast<pal_d3d12_asset_compacted_bottom_level_acceleration_structure *>(wrapped_asset_compacted_bottom_level_acceleration_structure);
	D3D12_GPU_VIRTUAL_ADDRESS const asset_compacted_bottom_level_acceleration_structure_device_memory_range_base = unwrapped_asset_compacted_bottom_level_acceleration_structure->get_resource()->GetGPUVirtualAddress();

	D3D12_RAYTRACING_INSTANCE_DESC &destination_top_level_acceleration_structure_instance = this->m_memory_range_base[instance_index];

	destination_top_level_acceleration_structure_instance.Transform[0][0] = transform_matrix[0][0];
	destination_top_level_acceleration_structure_instance.Transform[0][1] = transform_matrix[0][1];
	destination_top_level_acceleration_structure_instance.Transform[0][2] = transform_matrix[0][2];
	destination_top_level_acceleration_structure_instance.Transform[0][3] = transform_matrix[0][3];
	destination_top_level_acceleration_structure_instance.Transform[1][0] = transform_matrix[1][0];
	destination_top_level_acceleration_structure_instance.Transform[1][1] = transform_matrix[1][1];
	destination_top_level_acceleration_structure_instance.Transform[1][2] = transform_matrix[1][2];
	destination_top_level_acceleration_structure_instance.Transform[1][3] = transform_matrix[1][3];
	destination_top_level_acceleration_structure_instance.Transform[2][0] = transform_matrix[2][0];
	destination_top_level_acceleration_structure_instance.Transform[2][1] = transform_matrix[2][1];
	destination_top_level_acceleration_structure_instance.Transform[2][2] = transform_matrix[2][2];
	destination_top_level_acceleration_structure_instance.Transform[2][3] = transform_matrix[2][3];
	destination_top_level_acceleration_structure_instance.InstanceID = 0U;
	destination_top_level_acceleration_structure_instance.InstanceMask = 0XFFU;
	destination_top_level_acceleration_structure_instance.InstanceContributionToHitGroupIndex = 0U;
	destination_top_level_acceleration_structure_instance.Flags = (force_closest_hit ? D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE : 0U) | (force_any_hit ? D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_NON_OPAQUE : 0U) | (disable_back_face_cull ? D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE : 0U) | (front_ccw ? D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE : 0U);
	destination_top_level_acceleration_structure_instance.AccelerationStructure = asset_compacted_bottom_level_acceleration_structure_device_memory_range_base;
}

ID3D12Resource *pal_d3d12_top_level_acceleration_structure_instance_buffer::get_resource() const
{
	return this->m_resource;
}

pal_d3d12_top_level_acceleration_structure::pal_d3d12_top_level_acceleration_structure() : m_resource(NULL), m_allocation(NULL)
{
}

void pal_d3d12_top_level_acceleration_structure::init(D3D12MA::Allocator *asset_allocator, D3D12MA::Pool *top_level_acceleration_structure_pool, uint32_t size)
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);

	D3D12MA::ALLOCATION_DESC const allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		top_level_acceleration_structure_pool,
		NULL};

	D3D12_RESOURCE_DESC const resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS};

	HRESULT const hr_create_resource = asset_allocator->CreateResource(&allocation_desc, &resource_desc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));

	this->m_shader_resource_view_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{
		.Format = DXGI_FORMAT_UNKNOWN,
		.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.RaytracingAccelerationStructure = {
			this->m_resource->GetGPUVirtualAddress()}};
}

void pal_d3d12_top_level_acceleration_structure::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

pal_d3d12_top_level_acceleration_structure::~pal_d3d12_top_level_acceleration_structure()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *pal_d3d12_top_level_acceleration_structure::get_resource() const
{
	return this->m_resource;
}

D3D12_SHADER_RESOURCE_VIEW_DESC const *pal_d3d12_top_level_acceleration_structure::get_shader_resource_view_desc() const
{
	return &this->m_shader_resource_view_desc;
}
