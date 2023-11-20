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

brx_d3d12_intermediate_color_attachment_image::brx_d3d12_intermediate_color_attachment_image() : m_heap(NULL), m_resource(NULL), m_render_target_view_descriptor_heap(NULL), m_render_target_view_descriptor{0U}
{
}

void brx_d3d12_intermediate_color_attachment_image::init(ID3D12Device *device, bool uma, BRX_COLOR_ATTACHMENT_IMAGE_FORMAT wrapped_color_attachment_image_format, uint32_t width, uint32_t height, bool allow_sampled_image)
{
	DXGI_FORMAT unwrapped_format;
	switch (wrapped_color_attachment_image_format)
	{
	case BRX_COLOR_ATTACHMENT_FORMAT_B8G8R8A8_UNORM:
		unwrapped_format = DXGI_FORMAT_B8G8R8A8_UNORM;
		break;
	case BRX_COLOR_ATTACHMENT_FORMAT_R8G8B8A8_UNORM:
		unwrapped_format = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case BRX_COLOR_ATTACHMENT_FORMAT_A2B10G10R10_UNORM_PACK32:
		assert(false);
		unwrapped_format = static_cast<DXGI_FORMAT>(-1);
		break;
	case BRX_COLOR_ATTACHMENT_FORMAT_A2R10G10B10_UNORM_PACK32:
		unwrapped_format = DXGI_FORMAT_R10G10B10A2_UNORM;
		break;
	case BRX_COLOR_ATTACHMENT_FORMAT_R16G16_UNORM:
		unwrapped_format = DXGI_FORMAT_R16G16_UNORM;
		break;
	default:
		assert(false);
		unwrapped_format = static_cast<DXGI_FORMAT>(-1);
	}

	assert(NULL == this->m_heap);
	assert(NULL == this->m_resource);
	{
		D3D12_RESOURCE_DESC const resource_desc = {
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
			width,
			height,
			1U,
			1U,
			unwrapped_format,
			{1U, 0U},
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET};

		D3D12_RESOURCE_ALLOCATION_INFO const resource_allocation_info = device->GetResourceAllocationInfo(0U, 1U, &resource_desc);

		D3D12_HEAP_DESC const heap_desc = {
			resource_allocation_info.SizeInBytes,
			{D3D12_HEAP_TYPE_CUSTOM,
			 D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE,
			 uma ? D3D12_MEMORY_POOL_L0 : D3D12_MEMORY_POOL_L1,
			 0U,
			 0U},
			D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
			D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES};
		HRESULT const hr_create_heap = device->CreateHeap(&heap_desc, IID_PPV_ARGS(&this->m_heap));
		assert(SUCCEEDED(hr_create_heap));

		D3D12_CLEAR_VALUE const optimized_clear_value = {
			.Format = unwrapped_format,
			.Color = {
				0.0F,
				0.0F,
				0.0F,
				0.0F}};
		HRESULT const hr_create_placed_resource = device->CreatePlacedResource(this->m_heap, 0U, &resource_desc, (!allow_sampled_image) ? D3D12_RESOURCE_STATE_RENDER_TARGET : (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE), &optimized_clear_value, IID_PPV_ARGS(&this->m_resource));
		assert(SUCCEEDED(hr_create_placed_resource));
	}

	assert(NULL == this->m_render_target_view_descriptor_heap);
	{
		D3D12_DESCRIPTOR_HEAP_DESC const descriptor_heap_desc = {
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			1U,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0U};
		HRESULT const hr_create_descriptor_heap = device->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&this->m_render_target_view_descriptor_heap));
		assert(SUCCEEDED(hr_create_descriptor_heap));
	}

	assert(0U == this->m_render_target_view_descriptor.ptr);
	{
		UINT const new_dsv_descriptor_heap_descriptor_increment_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		D3D12_CPU_DESCRIPTOR_HANDLE const new_dsv_descriptor_heap_cpu_descriptor_handle_start = this->m_render_target_view_descriptor_heap->GetCPUDescriptorHandleForHeapStart();

		this->m_render_target_view_descriptor = D3D12_CPU_DESCRIPTOR_HANDLE{new_dsv_descriptor_heap_cpu_descriptor_handle_start.ptr + new_dsv_descriptor_heap_descriptor_increment_size * 0U};

		D3D12_RENDER_TARGET_VIEW_DESC const depth_stencil_view_desc{
			.Format = unwrapped_format,
			.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D,
			.Texture2D = {
				0U,
				0U}};
		device->CreateRenderTargetView(this->m_resource, &depth_stencil_view_desc, this->m_render_target_view_descriptor);
	}

	this->m_shader_resource_view_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{
		.Format = unwrapped_format,
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Texture2D = {
			0U,
			1U,
			0U,
			0.0F}};
}

void brx_d3d12_intermediate_color_attachment_image::uninit()
{
	assert(NULL != this->m_render_target_view_descriptor_heap);
	this->m_render_target_view_descriptor_heap->Release();
	this->m_render_target_view_descriptor_heap = NULL;

	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_heap);
	this->m_heap->Release();
	this->m_heap = NULL;
}

brx_d3d12_intermediate_color_attachment_image::~brx_d3d12_intermediate_color_attachment_image()
{
	assert(NULL == this->m_render_target_view_descriptor_heap);
	assert(NULL == this->m_resource);
	assert(NULL == this->m_heap);
}

ID3D12Resource *brx_d3d12_intermediate_color_attachment_image::get_resource() const
{
	return this->m_resource;
}

D3D12_CPU_DESCRIPTOR_HANDLE brx_d3d12_intermediate_color_attachment_image::get_render_target_view_descriptor() const
{
	return this->m_render_target_view_descriptor;
}

D3D12_SHADER_RESOURCE_VIEW_DESC const *brx_d3d12_intermediate_color_attachment_image::get_shader_resource_view_desc() const
{
	return &this->m_shader_resource_view_desc;
}

brx_sampled_image const *brx_d3d12_intermediate_color_attachment_image::get_sampled_image() const
{
	return static_cast<brx_d3d12_sampled_image const *>(this);
}

brx_d3d12_intermediate_depth_stencil_attachment_image::brx_d3d12_intermediate_depth_stencil_attachment_image() : m_heap(NULL), m_resource(NULL), m_depth_stencil_view_descriptor_heap(NULL), m_depth_stencil_view_descriptor{0U}
{
}

void brx_d3d12_intermediate_depth_stencil_attachment_image::init(ID3D12Device *device, bool uma, BRX_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT wrapped_depth_stencil_attachment_image_format, uint32_t width, uint32_t height, bool allow_sampled_image)
{
	DXGI_FORMAT unwrapped_resource_format;
	DXGI_FORMAT unwrapped_depth_stencil_view_format;
	DXGI_FORMAT unwrapped_shader_resource_view_format;
	switch (wrapped_depth_stencil_attachment_image_format)
	{
	case BRX_DEPTH_STENCIL_ATTACHMENT_FORMAT_D32_SFLOAT:
		unwrapped_resource_format = DXGI_FORMAT_R32_TYPELESS;
		unwrapped_depth_stencil_view_format = DXGI_FORMAT_D32_FLOAT;
		unwrapped_shader_resource_view_format = DXGI_FORMAT_R32_FLOAT;
		break;
	case BRX_DEPTH_STENCIL_ATTACHMENT_FORMAT_D32_SFLOAT_S8_UINT:
		unwrapped_resource_format = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
		unwrapped_depth_stencil_view_format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		unwrapped_shader_resource_view_format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		break;
	case BRX_DEPTH_STENCIL_ATTACHMENT_FORMAT_D24_UNORM_S8_UINT:
		unwrapped_resource_format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
		unwrapped_depth_stencil_view_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		unwrapped_shader_resource_view_format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		break;
	default:
		assert(false);
		unwrapped_resource_format = static_cast<DXGI_FORMAT>(-1);
		unwrapped_depth_stencil_view_format = static_cast<DXGI_FORMAT>(-1);
		unwrapped_shader_resource_view_format = static_cast<DXGI_FORMAT>(-1);
	}

	assert(NULL == this->m_heap);
	assert(NULL == this->m_resource);
	{
		D3D12_RESOURCE_DESC const resource_desc = {
			D3D12_RESOURCE_DIMENSION_TEXTURE2D,
			D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
			width,
			height,
			1U,
			1U,
			unwrapped_resource_format,
			{1U, 0U},
			D3D12_TEXTURE_LAYOUT_UNKNOWN,
			(!allow_sampled_image) ? (D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE) : D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL};

		D3D12_RESOURCE_ALLOCATION_INFO const resource_allocation_info = device->GetResourceAllocationInfo(0U, 1U, &resource_desc);

		D3D12_HEAP_DESC const heap_desc = {
			resource_allocation_info.SizeInBytes,
			{D3D12_HEAP_TYPE_CUSTOM,
			 D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE,
			 uma ? D3D12_MEMORY_POOL_L0 : D3D12_MEMORY_POOL_L1,
			 0U,
			 0U},
			D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
			D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES};
		HRESULT const hr_create_heap = device->CreateHeap(&heap_desc, IID_PPV_ARGS(&this->m_heap));
		assert(SUCCEEDED(hr_create_heap));

		D3D12_CLEAR_VALUE const optimized_clear_value = {
			.Format = unwrapped_depth_stencil_view_format,
			.DepthStencil = {
				0.0F,
				0U}};
		HRESULT const hr_create_placed_resource = device->CreatePlacedResource(this->m_heap, 0U, &resource_desc, (!allow_sampled_image) ? D3D12_RESOURCE_STATE_DEPTH_WRITE : (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE), &optimized_clear_value, IID_PPV_ARGS(&this->m_resource));
		assert(SUCCEEDED(hr_create_placed_resource));
	}

	assert(NULL == this->m_depth_stencil_view_descriptor_heap);
	{
		D3D12_DESCRIPTOR_HEAP_DESC const descriptor_heap_desc = {
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
			1U,
			D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			0U};
		HRESULT const hr_create_descriptor_heap = device->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(&this->m_depth_stencil_view_descriptor_heap));
		assert(SUCCEEDED(hr_create_descriptor_heap));
	}

	assert(0U == this->m_depth_stencil_view_descriptor.ptr);
	{
		UINT const new_dsv_descriptor_heap_descriptor_increment_size = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		D3D12_CPU_DESCRIPTOR_HANDLE const new_dsv_descriptor_heap_cpu_descriptor_handle_start = this->m_depth_stencil_view_descriptor_heap->GetCPUDescriptorHandleForHeapStart();

		this->m_depth_stencil_view_descriptor = D3D12_CPU_DESCRIPTOR_HANDLE{new_dsv_descriptor_heap_cpu_descriptor_handle_start.ptr + new_dsv_descriptor_heap_descriptor_increment_size * 0U};

		D3D12_DEPTH_STENCIL_VIEW_DESC const depth_stencil_view_desc{
			.Format = unwrapped_depth_stencil_view_format,
			.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
			.Flags = D3D12_DSV_FLAG_NONE,
			.Texture2D = {
				0U}};
		device->CreateDepthStencilView(this->m_resource, &depth_stencil_view_desc, this->m_depth_stencil_view_descriptor);
	}

	this->m_shader_resource_view_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{
		.Format = unwrapped_shader_resource_view_format,
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Texture2D = {
			0U,
			1U,
			0U,
			0.0F}};
}

void brx_d3d12_intermediate_depth_stencil_attachment_image::uninit()
{
	assert(NULL != this->m_depth_stencil_view_descriptor_heap);
	this->m_depth_stencil_view_descriptor_heap->Release();
	this->m_depth_stencil_view_descriptor_heap = NULL;

	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_heap);
	this->m_heap->Release();
	this->m_heap = NULL;
}

brx_d3d12_intermediate_depth_stencil_attachment_image::~brx_d3d12_intermediate_depth_stencil_attachment_image()
{
	assert(NULL == this->m_depth_stencil_view_descriptor_heap);
	assert(NULL == this->m_resource);
	assert(NULL == this->m_heap);
}

ID3D12Resource *brx_d3d12_intermediate_depth_stencil_attachment_image::get_resource() const
{
	return this->m_resource;
}

D3D12_CPU_DESCRIPTOR_HANDLE brx_d3d12_intermediate_depth_stencil_attachment_image::get_depth_stencil_view_descriptor() const
{
	return this->m_depth_stencil_view_descriptor;
}

D3D12_SHADER_RESOURCE_VIEW_DESC const *brx_d3d12_intermediate_depth_stencil_attachment_image::get_shader_resource_view_desc() const
{
	return &this->m_shader_resource_view_desc;
}

brx_sampled_image const *brx_d3d12_intermediate_depth_stencil_attachment_image::get_sampled_image() const
{
	return static_cast<brx_d3d12_sampled_image const *>(this);
}

brx_d3d12_intermediate_storage_image::brx_d3d12_intermediate_storage_image() : m_resource(NULL), m_allocation(NULL)
{
}

void brx_d3d12_intermediate_storage_image::init(D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *storage_image_memory_pool, DXGI_FORMAT unwrapped_storage_image_format, uint32_t width, uint32_t height, bool allow_sampled_image)
{
	D3D12MA::ALLOCATION_DESC allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		storage_image_memory_pool,
		NULL};

	D3D12_RESOURCE_DESC const resource_desc = {
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		width,
		height,
		1U,
		1U,
		unwrapped_storage_image_format,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS};

	HRESULT hr_create_resource = memory_allocator->CreateResource(&allocation_desc, &resource_desc, (!allow_sampled_image) ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : (D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE), NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));

	this->m_unordered_access_view_desc = D3D12_UNORDERED_ACCESS_VIEW_DESC{
		.Format = unwrapped_storage_image_format,
		.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D,
		.Texture2D = {
			0U,
			0U}};

	this->m_shader_resource_view_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{
		.Format = unwrapped_storage_image_format,
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Texture2D = {
			0U,
			1U,
			0U,
			0.0F}};
}

void brx_d3d12_intermediate_storage_image::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

brx_d3d12_intermediate_storage_image::~brx_d3d12_intermediate_storage_image()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *brx_d3d12_intermediate_storage_image::get_resource() const
{
	return this->m_resource;
}

D3D12_UNORDERED_ACCESS_VIEW_DESC const *brx_d3d12_intermediate_storage_image::get_unordered_access_view_desc() const
{
	return &this->m_unordered_access_view_desc;
}

D3D12_SHADER_RESOURCE_VIEW_DESC const *brx_d3d12_intermediate_storage_image::get_shader_resource_view_desc() const
{
	return &this->m_shader_resource_view_desc;
}

brx_sampled_image const *brx_d3d12_intermediate_storage_image::get_sampled_image() const
{
	return static_cast<brx_d3d12_sampled_image const *>(this);
}

brx_d3d12_asset_sampled_image::brx_d3d12_asset_sampled_image() : m_resource(NULL), m_allocation(NULL)
{
}

void brx_d3d12_asset_sampled_image::init(bool uma, D3D12MA::Allocator *memory_allocator, D3D12MA::Pool *asset_sampled_image_memory_pool, DXGI_FORMAT unwrapped_asset_sampled_image_format, uint32_t width, uint32_t height, uint32_t mip_levels)
{
	D3D12MA::ALLOCATION_DESC allocation_desc = {
		D3D12MA::ALLOCATION_FLAG_NONE,
		D3D12_HEAP_TYPE_CUSTOM,
		D3D12_HEAP_FLAG_NONE,
		asset_sampled_image_memory_pool,
		NULL};

	D3D12_RESOURCE_DESC resource_desc = {
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT,
		width,
		height,
		1U,
		static_cast<UINT16>(mip_levels),
		unwrapped_asset_sampled_image_format,
		{1U, 0U},
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_NONE};

	HRESULT hr_create_resource = memory_allocator->CreateResource(&allocation_desc, &resource_desc, (!uma) ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COMMON, NULL, &this->m_allocation, IID_PPV_ARGS(&this->m_resource));
	assert(SUCCEEDED(hr_create_resource));

	this->m_shader_resource_view_desc = D3D12_SHADER_RESOURCE_VIEW_DESC{
		.Format = unwrapped_asset_sampled_image_format,
		.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
		.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
		.Texture2D = {
			0U,
			mip_levels,
			0U,
			0.0F}};
}

void brx_d3d12_asset_sampled_image::uninit()
{
	assert(NULL != this->m_resource);
	this->m_resource->Release();
	this->m_resource = NULL;

	assert(NULL != this->m_allocation);
	this->m_allocation->Release();
	this->m_allocation = NULL;
}

brx_d3d12_asset_sampled_image::~brx_d3d12_asset_sampled_image()
{
	assert(NULL == this->m_resource);
	assert(NULL == this->m_allocation);
}

ID3D12Resource *brx_d3d12_asset_sampled_image::get_resource() const
{
	return this->m_resource;
}

D3D12_SHADER_RESOURCE_VIEW_DESC const *brx_d3d12_asset_sampled_image::get_shader_resource_view_desc() const
{
	return &this->m_shader_resource_view_desc;
}

brx_sampled_image const *brx_d3d12_asset_sampled_image::get_sampled_image() const
{
	return static_cast<brx_d3d12_sampled_image const *>(this);
}
