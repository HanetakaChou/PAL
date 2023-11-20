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

brx_d3d12_swap_chain_image::brx_d3d12_swap_chain_image(ID3D12Resource *resource, D3D12_CPU_DESCRIPTOR_HANDLE render_target_view_descriptor) : m_resource(resource), m_render_target_view_descriptor(render_target_view_descriptor)
{
}

ID3D12Resource *brx_d3d12_swap_chain_image::get_resource() const
{
	return this->m_resource;
}

D3D12_CPU_DESCRIPTOR_HANDLE brx_d3d12_swap_chain_image::get_render_target_view_descriptor() const
{
	return this->m_render_target_view_descriptor;
}

brx_sampled_image const *brx_d3d12_swap_chain_image::get_sampled_image() const
{
	return NULL;
}

void brx_d3d12_swap_chain_image::steal(ID3D12Resource **out_resource)
{
	assert(NULL != out_resource);

	(*out_resource) = this->m_resource;

	this->m_resource = NULL;
}

brx_d3d12_swap_chain::brx_d3d12_swap_chain(
	IDXGISwapChain3 *swap_chain,
	DXGI_FORMAT image_format,
	uint32_t image_width,
	uint32_t image_height,
	uint32_t image_count,
	ID3D12DescriptorHeap *rtv_descriptor_heap,
	brx_vector<brx_d3d12_swap_chain_image> &&images)
	: m_swap_chain(swap_chain),
	  m_image_format(image_format),
	  m_image_width(image_width),
	  m_image_height(image_height),
	  m_image_count(image_count),
	  m_rtv_descriptor_heap(rtv_descriptor_heap),
	  m_images(std::move(images))
{
}

IDXGISwapChain3 *brx_d3d12_swap_chain::get_swap_chain() const
{
	return this->m_swap_chain;
}

BRX_COLOR_ATTACHMENT_IMAGE_FORMAT brx_d3d12_swap_chain::get_image_format() const
{
	BRX_COLOR_ATTACHMENT_IMAGE_FORMAT brx_color_attachment_image_format;
	switch (this->m_image_format)
	{
	case DXGI_FORMAT_B8G8R8A8_UNORM:
		brx_color_attachment_image_format = BRX_COLOR_ATTACHMENT_FORMAT_B8G8R8A8_UNORM;
		break;
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		brx_color_attachment_image_format = BRX_COLOR_ATTACHMENT_FORMAT_R8G8B8A8_UNORM;
		break;
	case DXGI_FORMAT_R10G10B10A2_UNORM:
		brx_color_attachment_image_format = BRX_COLOR_ATTACHMENT_FORMAT_A2R10G10B10_UNORM_PACK32;
		break;
	default:
		assert(false);
		brx_color_attachment_image_format = static_cast<BRX_COLOR_ATTACHMENT_IMAGE_FORMAT>(-1);
	}
	return brx_color_attachment_image_format;
}

uint32_t brx_d3d12_swap_chain::get_image_width() const
{
	return this->m_image_width;
}

uint32_t brx_d3d12_swap_chain::get_image_height() const
{
	return this->m_image_height;
}

uint32_t brx_d3d12_swap_chain::get_image_count() const
{
	return this->m_image_count;
}

brx_color_attachment_image const *brx_d3d12_swap_chain::get_image(uint32_t swap_chain_image_index) const
{
	return &this->m_images[swap_chain_image_index];
}

void brx_d3d12_swap_chain::steal(IDXGISwapChain3 **out_swap_chain, ID3D12DescriptorHeap **out_rtv_descriptor_heap, brx_vector<brx_d3d12_swap_chain_image> &out_images)
{
	assert(NULL != out_swap_chain);
	assert(NULL != out_rtv_descriptor_heap);

	(*out_swap_chain) = this->m_swap_chain;
	(*out_rtv_descriptor_heap) = this->m_rtv_descriptor_heap;
	out_images = std::move(this->m_images);

	this->m_swap_chain = NULL;
	this->m_rtv_descriptor_heap = NULL;
}

brx_d3d12_swap_chain::~brx_d3d12_swap_chain()
{
	assert(NULL == this->m_swap_chain);
	assert(NULL == this->m_rtv_descriptor_heap);
	assert(0U == this->m_images.size());
}