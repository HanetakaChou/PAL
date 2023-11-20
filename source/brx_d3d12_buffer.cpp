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

#include "brx_d3d12_device.h"
#include <assert.h>

brx_d3d12_uniform_upload_buffer::brx_d3d12_uniform_upload_buffer() : m_resource(NULL), m_allocation(NULL), m_host_memory_range_base(NULL)
{
}

void brx_d3d12_uniform_upload_buffer::init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *uniform_upload_buffer_memory_pool, uint32_t size)
{
	D3D12MA::ALLOCATION_DESC const allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		uniform_upload_buffer_memory_pool,
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
		D3D12_RESOURCE_FLAG_NONE};

	HRESULT const hr_create_resource = memory_allocator->CreateResource(&allocation_desc, &resource_desc, D3D12_RESOURCE_STATE_COPY_SOURCE, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));

	assert(NULL == this->m_host_memory_range_base);
	D3D12_RANGE const read_range = {0U, 0U};
	HRESULT const hr_map = this->m_resource->Map(0U, &read_range, &this->m_host_memory_range_base);
	assert(SUCCEEDED(hr_map));
}

void brx_d3d12_uniform_upload_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

brx_d3d12_uniform_upload_buffer::~brx_d3d12_uniform_upload_buffer()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *brx_d3d12_uniform_upload_buffer::get_resource() const
{
	return this->m_resource;
}

void *brx_d3d12_uniform_upload_buffer::get_host_memory_range_base() const
{
	return this->m_host_memory_range_base;
}

brx_d3d12_staging_upload_buffer::brx_d3d12_staging_upload_buffer() : m_resource(NULL), m_allocation(NULL), m_host_memory_range_base(NULL)
{
}

void brx_d3d12_staging_upload_buffer::init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *staging_upload_buffer_memory_pool, uint32_t size)
{
	D3D12MA::ALLOCATION_DESC const allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		staging_upload_buffer_memory_pool,
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
		D3D12_RESOURCE_FLAG_NONE};

	HRESULT const hr_create_resource = memory_allocator->CreateResource(&allocation_desc, &resource_desc, D3D12_RESOURCE_STATE_COPY_SOURCE, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));

	assert(NULL == this->m_host_memory_range_base);
	D3D12_RANGE const read_range = {0U, 0U};
	HRESULT const hr_map = this->m_resource->Map(0U, &read_range, &this->m_host_memory_range_base);
	assert(SUCCEEDED(hr_map));
}

void brx_d3d12_staging_upload_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

brx_d3d12_staging_upload_buffer::~brx_d3d12_staging_upload_buffer()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *brx_d3d12_staging_upload_buffer::get_resource() const
{
	return this->m_resource;
}

void *brx_d3d12_staging_upload_buffer::get_host_memory_range_base() const
{
	return this->m_host_memory_range_base;
}

brx_d3d12_intermediate_storage_buffer::brx_d3d12_intermediate_storage_buffer() : m_resource(NULL), m_allocation(NULL)
{
}

void brx_d3d12_intermediate_storage_buffer::init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *storage_buffer_memory_pool, uint32_t wrapped_size, bool allow_vertex_position, bool allow_vertex_varying)
{
	uint32_t const num_elements = ((wrapped_size - 1U) / sizeof(uint32_t)) + 1U;
	uint32_t const size_elements = sizeof(uint32_t) * num_elements;

	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);

	D3D12MA::ALLOCATION_DESC allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		storage_buffer_memory_pool,
		NULL};

	D3D12_RESOURCE_DESC resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size_elements,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS};

	HRESULT hr_create_resource = memory_allocator->CreateResource(&allocation_desc, &resource_desc, ((!allow_vertex_position) && (!allow_vertex_varying)) ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : (D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE), NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));

	this->m_unordered_access_view_desc = D3D12_UNORDERED_ACCESS_VIEW_DESC{
		.Format = DXGI_FORMAT_R32_TYPELESS,
		.ViewDimension = D3D12_UAV_DIMENSION_BUFFER,
		.Buffer = {
			0U,
			static_cast<UINT>(num_elements),
			0U,
			0U,
			D3D12_BUFFER_UAV_FLAG_RAW}};

	this->m_shader_resource_view_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{
		.Format = DXGI_FORMAT_R32_TYPELESS,
		.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Buffer = {
			0U,
			static_cast<UINT>(num_elements),
			0U,
			D3D12_BUFFER_SRV_FLAG_RAW}};
}

void brx_d3d12_intermediate_storage_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

brx_d3d12_intermediate_storage_buffer::~brx_d3d12_intermediate_storage_buffer()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *brx_d3d12_intermediate_storage_buffer::get_resource() const
{
	return this->m_resource;
}

D3D12_UNORDERED_ACCESS_VIEW_DESC const *brx_d3d12_intermediate_storage_buffer::get_unordered_access_view_desc() const
{
	return &this->m_unordered_access_view_desc;
}

D3D12_SHADER_RESOURCE_VIEW_DESC const *brx_d3d12_intermediate_storage_buffer::get_shader_resource_view_desc() const
{
	return &this->m_shader_resource_view_desc;
}

brx_vertex_buffer const *brx_d3d12_intermediate_storage_buffer::get_vertex_buffer() const
{
	return static_cast<brx_d3d12_vertex_buffer const *>(this);
}

brx_vertex_position_buffer const *brx_d3d12_intermediate_storage_buffer::get_vertex_position_buffer() const
{
	return static_cast<brx_d3d12_vertex_position_buffer const *>(this);
}

brx_vertex_varying_buffer const *brx_d3d12_intermediate_storage_buffer::get_vertex_varying_buffer() const
{
	return static_cast<brx_d3d12_vertex_varying_buffer const *>(this);
}

brx_storage_buffer const *brx_d3d12_intermediate_storage_buffer::get_storage_buffer() const
{
	return static_cast<brx_d3d12_storage_buffer const *>(this);
}

brx_d3d12_asset_vertex_position_buffer::brx_d3d12_asset_vertex_position_buffer() : m_resource(NULL), m_allocation(NULL)
{
}

void brx_d3d12_asset_vertex_position_buffer::init(bool uma, D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *asset_vertex_position_buffer_memory_pool, uint32_t wrapped_size)
{
	uint32_t const num_elements = ((wrapped_size - 1U) / sizeof(uint32_t)) + 1U;
	uint32_t const size_elements = sizeof(uint32_t) * num_elements;

	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);

	D3D12MA::ALLOCATION_DESC allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		asset_vertex_position_buffer_memory_pool,
		NULL};

	D3D12_RESOURCE_DESC resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size_elements,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE};

	HRESULT hr_create_resource = memory_allocator->CreateResource(&allocation_desc, &resource_desc, (!uma) ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COMMON, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));

	this->m_shader_resource_view_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{
		.Format = DXGI_FORMAT_R32_TYPELESS,
		.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Buffer = {
			0U,
			static_cast<UINT>(num_elements),
			0U,
			D3D12_BUFFER_SRV_FLAG_RAW}};
}

void brx_d3d12_asset_vertex_position_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

brx_d3d12_asset_vertex_position_buffer::~brx_d3d12_asset_vertex_position_buffer()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *brx_d3d12_asset_vertex_position_buffer::get_resource() const
{
	return this->m_resource;
}

D3D12_UNORDERED_ACCESS_VIEW_DESC const *brx_d3d12_asset_vertex_position_buffer::get_unordered_access_view_desc() const
{
	return NULL;
}

D3D12_SHADER_RESOURCE_VIEW_DESC const *brx_d3d12_asset_vertex_position_buffer::get_shader_resource_view_desc() const
{
	return &this->m_shader_resource_view_desc;
}

brx_vertex_buffer const *brx_d3d12_asset_vertex_position_buffer::get_vertex_buffer() const
{
	return static_cast<brx_d3d12_vertex_buffer const *>(this);
}

brx_vertex_position_buffer const *brx_d3d12_asset_vertex_position_buffer::get_vertex_position_buffer() const
{
	return static_cast<brx_d3d12_vertex_position_buffer const *>(this);
}

brx_storage_buffer const *brx_d3d12_asset_vertex_position_buffer::get_storage_buffer() const
{
	return static_cast<brx_d3d12_storage_buffer const *>(this);
}

brx_d3d12_asset_vertex_varying_buffer::brx_d3d12_asset_vertex_varying_buffer() : m_resource(NULL), m_allocation(NULL)
{
}

void brx_d3d12_asset_vertex_varying_buffer::init(bool uma, D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *asset_vertex_varying_buffer_memory_pool, uint32_t wrapped_size)
{
	uint32_t const num_elements = ((wrapped_size - 1U) / sizeof(uint32_t)) + 1U;
	uint32_t const size_elements = sizeof(uint32_t) * num_elements;

	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);

	D3D12MA::ALLOCATION_DESC allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		asset_vertex_varying_buffer_memory_pool,
		NULL};

	D3D12_RESOURCE_DESC resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size_elements,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE};

	HRESULT hr_create_resource = memory_allocator->CreateResource(&allocation_desc, &resource_desc, (!uma) ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COMMON, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));

	this->m_shader_resource_view_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{
		.Format = DXGI_FORMAT_R32_TYPELESS,
		.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Buffer = {
			0U,
			static_cast<UINT>(num_elements),
			0U,
			D3D12_BUFFER_SRV_FLAG_RAW}};
}

void brx_d3d12_asset_vertex_varying_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

brx_d3d12_asset_vertex_varying_buffer::~brx_d3d12_asset_vertex_varying_buffer()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *brx_d3d12_asset_vertex_varying_buffer::get_resource() const
{
	return this->m_resource;
}

D3D12_UNORDERED_ACCESS_VIEW_DESC const *brx_d3d12_asset_vertex_varying_buffer::get_unordered_access_view_desc() const
{
	return NULL;
}

D3D12_SHADER_RESOURCE_VIEW_DESC const *brx_d3d12_asset_vertex_varying_buffer::get_shader_resource_view_desc() const
{
	return &this->m_shader_resource_view_desc;
}

brx_vertex_buffer const *brx_d3d12_asset_vertex_varying_buffer::get_vertex_buffer() const
{
	return static_cast<brx_d3d12_vertex_buffer const *>(this);
}

brx_vertex_varying_buffer const *brx_d3d12_asset_vertex_varying_buffer::get_vertex_varying_buffer() const
{
	return static_cast<brx_d3d12_vertex_varying_buffer const *>(this);
}

brx_storage_buffer const *brx_d3d12_asset_vertex_varying_buffer::get_storage_buffer() const
{
	return static_cast<brx_d3d12_storage_buffer const *>(this);
}

brx_d3d12_asset_index_buffer::brx_d3d12_asset_index_buffer() : m_resource(NULL), m_allocation(NULL)
{
}

void brx_d3d12_asset_index_buffer::init(bool uma, D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *asset_index_buffer_memory_pool, uint32_t wrapped_size)
{
	uint32_t const num_elements = ((wrapped_size - 1U) / sizeof(uint32_t)) + 1U;
	uint32_t const size_elements = sizeof(uint32_t) * num_elements;

	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);

	D3D12MA::ALLOCATION_DESC allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		asset_index_buffer_memory_pool,
		NULL};

	D3D12_RESOURCE_DESC resource_desc = {
		D3D12_RESOURCE_DIMENSION_BUFFER,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		size_elements,
		1U,
		1U,
		1U,
		DXGI_FORMAT_UNKNOWN,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		D3D12_RESOURCE_FLAG_NONE};

	HRESULT hr_create_resource = memory_allocator->CreateResource(&allocation_desc, &resource_desc, (!uma) ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COMMON, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));

	this->m_shader_resource_view_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{
		.Format = DXGI_FORMAT_R32_TYPELESS,
		.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Buffer = {
			0U,
			static_cast<UINT>(num_elements),
			0U,
			D3D12_BUFFER_SRV_FLAG_RAW}};
}

void brx_d3d12_asset_index_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

brx_d3d12_asset_index_buffer::~brx_d3d12_asset_index_buffer()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *brx_d3d12_asset_index_buffer::get_resource() const
{
	return this->m_resource;
}

D3D12_UNORDERED_ACCESS_VIEW_DESC const *brx_d3d12_asset_index_buffer::get_unordered_access_view_desc() const
{
	return NULL;
}

D3D12_SHADER_RESOURCE_VIEW_DESC const *brx_d3d12_asset_index_buffer::get_shader_resource_view_desc() const
{
	return &this->m_shader_resource_view_desc;
}

brx_index_buffer const *brx_d3d12_asset_index_buffer::get_index_buffer() const
{
	return static_cast<brx_d3d12_index_buffer const *>(this);
}

brx_storage_buffer const *brx_d3d12_asset_index_buffer::get_storage_buffer() const
{
	return static_cast<brx_d3d12_storage_buffer const *>(this);
}

brx_d3d12_scratch_buffer::brx_d3d12_scratch_buffer() : m_resource(NULL), m_allocation(NULL)
{
}

void brx_d3d12_scratch_buffer::init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *scratch_buffer_memory_pool, uint32_t size)
{
	D3D12MA::ALLOCATION_DESC const allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		scratch_buffer_memory_pool,
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

	HRESULT const hr_create_resource = memory_allocator->CreateResource(&allocation_desc, &resource_desc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));
}

void brx_d3d12_scratch_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

brx_d3d12_scratch_buffer::~brx_d3d12_scratch_buffer()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *brx_d3d12_scratch_buffer::get_resource() const
{
	return this->m_resource;
}

brx_d3d12_staging_non_compacted_bottom_level_acceleration_structure::brx_d3d12_staging_non_compacted_bottom_level_acceleration_structure() : m_resource(NULL), m_allocation(NULL)
{
}

void brx_d3d12_staging_non_compacted_bottom_level_acceleration_structure::init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *staging_non_compacted_bottom_level_acceleration_structure_memory_pool, uint32_t size)
{
	D3D12MA::ALLOCATION_DESC const allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		staging_non_compacted_bottom_level_acceleration_structure_memory_pool,
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

	HRESULT const hr_create_resource = memory_allocator->CreateResource(&allocation_desc, &resource_desc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));
}

void brx_d3d12_staging_non_compacted_bottom_level_acceleration_structure::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

brx_d3d12_staging_non_compacted_bottom_level_acceleration_structure::~brx_d3d12_staging_non_compacted_bottom_level_acceleration_structure()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *brx_d3d12_staging_non_compacted_bottom_level_acceleration_structure::get_resource() const
{
	return this->m_resource;
}

brx_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool::brx_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool() : m_resource(NULL), m_allocation(NULL), m_host_memory_range_base(NULL)
{
}

void brx_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool::init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *compacted_bottom_level_acceleration_structure_size_query_buffer_memory_pool, uint32_t query_count)
{
	uint32_t const size = sizeof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC) * query_count;

	D3D12MA::ALLOCATION_DESC const allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		compacted_bottom_level_acceleration_structure_size_query_buffer_memory_pool,
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

	HRESULT const hr_create_resource = memory_allocator->CreateResource(&allocation_desc, &resource_desc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));

	void *host_memory_range_base = NULL;
	D3D12_RANGE const read_range = {0U, size};
	HRESULT const hr_map = this->m_resource->Map(0U, &read_range, &host_memory_range_base);
	assert(SUCCEEDED(hr_map));

	assert(NULL == this->m_host_memory_range_base);
	this->m_host_memory_range_base = static_cast<D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC *>(host_memory_range_base);
}

void brx_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

brx_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool::~brx_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *brx_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool::get_resource() const
{
	return this->m_resource;
}

D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC volatile *brx_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool::get_host_memory_range_base() const
{
	return this->m_host_memory_range_base;
}

brx_d3d12_asset_compacted_bottom_level_acceleration_structure::brx_d3d12_asset_compacted_bottom_level_acceleration_structure() : m_resource(NULL), m_allocation(NULL)
{
}

void brx_d3d12_asset_compacted_bottom_level_acceleration_structure::init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *asset_compacted_bottom_level_acceleration_structure_memory_pool, uint32_t size)
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);

	D3D12MA::ALLOCATION_DESC const allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		asset_compacted_bottom_level_acceleration_structure_memory_pool,
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

	HRESULT const hr_create_resource = memory_allocator->CreateResource(&allocation_desc, &resource_desc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));
}

void brx_d3d12_asset_compacted_bottom_level_acceleration_structure::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

brx_d3d12_asset_compacted_bottom_level_acceleration_structure::~brx_d3d12_asset_compacted_bottom_level_acceleration_structure()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *brx_d3d12_asset_compacted_bottom_level_acceleration_structure::get_resource() const
{
	return this->m_resource;
}

brx_d3d12_top_level_acceleration_structure_instance_upload_buffer::brx_d3d12_top_level_acceleration_structure_instance_upload_buffer() : m_resource(NULL), m_allocation(NULL), m_host_memory_range_base(NULL)
{
}

void brx_d3d12_top_level_acceleration_structure_instance_upload_buffer::init(uint32_t instance_count, D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *top_level_acceleration_structure_instance_upload_buffer_memory_pool)
{
	uint32_t const size = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instance_count;

	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);

	D3D12MA::ALLOCATION_DESC const allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		top_level_acceleration_structure_instance_upload_buffer_memory_pool,
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

	HRESULT const hr_create_resource = memory_allocator->CreateResource(&allocation_desc, &resource_desc, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));

	void *host_memory_range_base = NULL;
	D3D12_RANGE const read_range = {0U, 0U};
	HRESULT const hr_map = this->m_resource->Map(0U, &read_range, &host_memory_range_base);
	assert(SUCCEEDED(hr_map));

	assert(NULL == this->m_host_memory_range_base);
	this->m_host_memory_range_base = static_cast<D3D12_RAYTRACING_INSTANCE_DESC *>(host_memory_range_base);
}

void brx_d3d12_top_level_acceleration_structure_instance_upload_buffer::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

brx_d3d12_top_level_acceleration_structure_instance_upload_buffer::~brx_d3d12_top_level_acceleration_structure_instance_upload_buffer()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

void brx_d3d12_top_level_acceleration_structure_instance_upload_buffer::write_instance(uint32_t instance_index, BRX_TOP_LEVEL_ACCELERATION_STRUCTURE_INSTANCE const *wrapped_bottom_top_acceleration_structure_instance)
{
	assert(wrapped_bottom_top_acceleration_structure_instance->instance_id < 0X1000000U);

	brx_d3d12_asset_compacted_bottom_level_acceleration_structure *const unwrapped_asset_compacted_bottom_level_acceleration_structure = static_cast<brx_d3d12_asset_compacted_bottom_level_acceleration_structure *>(wrapped_bottom_top_acceleration_structure_instance->asset_compacted_bottom_level_acceleration_structure);
	D3D12_GPU_VIRTUAL_ADDRESS const asset_compacted_bottom_level_acceleration_structure_device_memory_range_base = unwrapped_asset_compacted_bottom_level_acceleration_structure->get_resource()->GetGPUVirtualAddress();

	this->m_host_memory_range_base[instance_index].Transform[0][0] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[0][0];
	this->m_host_memory_range_base[instance_index].Transform[0][1] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[0][1];
	this->m_host_memory_range_base[instance_index].Transform[0][2] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[0][2];
	this->m_host_memory_range_base[instance_index].Transform[0][3] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[0][3];
	this->m_host_memory_range_base[instance_index].Transform[1][0] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[1][0];
	this->m_host_memory_range_base[instance_index].Transform[1][1] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[1][1];
	this->m_host_memory_range_base[instance_index].Transform[1][2] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[1][2];
	this->m_host_memory_range_base[instance_index].Transform[1][3] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[1][3];
	this->m_host_memory_range_base[instance_index].Transform[2][0] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[2][0];
	this->m_host_memory_range_base[instance_index].Transform[2][1] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[2][1];
	this->m_host_memory_range_base[instance_index].Transform[2][2] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[2][2];
	this->m_host_memory_range_base[instance_index].Transform[2][3] = wrapped_bottom_top_acceleration_structure_instance->transform_matrix[2][3];
	this->m_host_memory_range_base[instance_index].InstanceID = wrapped_bottom_top_acceleration_structure_instance->instance_id;
	this->m_host_memory_range_base[instance_index].InstanceMask = wrapped_bottom_top_acceleration_structure_instance->instance_mask;
	this->m_host_memory_range_base[instance_index].InstanceContributionToHitGroupIndex = 0U;
	this->m_host_memory_range_base[instance_index].Flags = (wrapped_bottom_top_acceleration_structure_instance->force_closest_hit ? D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE : 0U) | (wrapped_bottom_top_acceleration_structure_instance->force_any_hit ? D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_NON_OPAQUE : 0U) | (wrapped_bottom_top_acceleration_structure_instance->disable_back_face_cull ? D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE : 0U) | (wrapped_bottom_top_acceleration_structure_instance->front_ccw ? D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE : 0U);
	this->m_host_memory_range_base[instance_index].AccelerationStructure = asset_compacted_bottom_level_acceleration_structure_device_memory_range_base;
}

ID3D12Resource *brx_d3d12_top_level_acceleration_structure_instance_upload_buffer::get_resource() const
{
	return this->m_resource;
}

brx_d3d12_top_level_acceleration_structure::brx_d3d12_top_level_acceleration_structure() : m_resource(NULL), m_allocation(NULL), m_instance_count(static_cast<uint32_t>(-1))
{
}

void brx_d3d12_top_level_acceleration_structure::init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *top_level_acceleration_structure_memory_pool, uint32_t size)
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);

	D3D12MA::ALLOCATION_DESC const allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		top_level_acceleration_structure_memory_pool,
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

	HRESULT const hr_create_resource = memory_allocator->CreateResource(&allocation_desc, &resource_desc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));

	this->m_shader_resource_view_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{
		.Format = DXGI_FORMAT_UNKNOWN,
		.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.RaytracingAccelerationStructure = {
			this->m_resource->GetGPUVirtualAddress()}};
}

void brx_d3d12_top_level_acceleration_structure::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

brx_d3d12_top_level_acceleration_structure::~brx_d3d12_top_level_acceleration_structure()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *brx_d3d12_top_level_acceleration_structure::get_resource() const
{
	return this->m_resource;
}

D3D12_SHADER_RESOURCE_VIEW_DESC const *brx_d3d12_top_level_acceleration_structure::get_shader_resource_view_desc() const
{
	return &this->m_shader_resource_view_desc;
}

void brx_d3d12_top_level_acceleration_structure::set_instance_count(uint32_t instance_count)
{
	assert(static_cast<uint32_t>(-1) == this->m_instance_count);
	this->m_instance_count = instance_count;
}

uint32_t brx_d3d12_top_level_acceleration_structure::get_instance_count() const
{
	return this->m_instance_count;
}
