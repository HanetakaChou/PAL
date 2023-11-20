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

pal_vk_surface::pal_vk_surface(VkSurfaceKHR surface) : m_surface(surface)
{
}

VkSurfaceKHR pal_vk_surface::get_surface() const
{
	return this->m_surface;
}

void pal_vk_surface::steal(VkSurfaceKHR *out_surface)
{
	assert(NULL != out_surface);

	(*out_surface) = this->m_surface;

	this->m_surface = VK_NULL_HANDLE;
}

pal_vk_surface::~pal_vk_surface()
{
	assert(VK_NULL_HANDLE == this->m_surface);
}

pal_vk_swap_chain_image_view::pal_vk_swap_chain_image_view(VkImageView image_view) : m_image_view(image_view)
{
}

VkImageView pal_vk_swap_chain_image_view::get_image_view() const
{
	return this->m_image_view;
}

pal_sampled_image const *pal_vk_swap_chain_image_view::get_sampled_image() const
{
	return NULL;
}

void pal_vk_swap_chain_image_view::steal(VkImageView *out_image_view)
{
	assert(NULL != out_image_view);

	(*out_image_view) = this->m_image_view;

	this->m_image_view = VK_NULL_HANDLE;
}

pal_vk_swap_chain_image_view::~pal_vk_swap_chain_image_view()
{
	assert(VK_NULL_HANDLE == this->m_image_view);
}

pal_vk_swap_chain::pal_vk_swap_chain(VkSwapchainKHR swap_chain, VkFormat image_format, uint32_t image_width, uint32_t image_height, uint32_t image_count, pal_vk_swap_chain_image_view *image_views) : m_swap_chain(swap_chain), m_image_format(image_format), m_image_width(image_width), m_image_height(image_height), m_image_count(image_count), m_image_views(image_views)
{
}

VkSwapchainKHR pal_vk_swap_chain::get_swap_chain() const
{
	return this->m_swap_chain;
}

PAL_COLOR_ATTACHMENT_IMAGE_FORMAT pal_vk_swap_chain::get_image_format() const
{
	PAL_COLOR_ATTACHMENT_IMAGE_FORMAT pal_color_attachment_image_format;
	switch (this->m_image_format)
	{
	case VK_FORMAT_B8G8R8A8_UNORM:
		pal_color_attachment_image_format = PAL_COLOR_ATTACHMENT_FORMAT_B8G8R8A8_UNORM;
		break;
	case VK_FORMAT_R8G8B8A8_UNORM:
		pal_color_attachment_image_format = PAL_COLOR_ATTACHMENT_FORMAT_R8G8B8A8_UNORM;
		break;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		pal_color_attachment_image_format = PAL_COLOR_ATTACHMENT_FORMAT_A2B10G10R10_UNORM_PACK32;
		break;
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
		pal_color_attachment_image_format = PAL_COLOR_ATTACHMENT_FORMAT_A2R10G10B10_UNORM_PACK32;
		break;
	default:
		assert(false);
		pal_color_attachment_image_format = static_cast<PAL_COLOR_ATTACHMENT_IMAGE_FORMAT>(-1);
	}
	return pal_color_attachment_image_format;
}

uint32_t pal_vk_swap_chain::get_image_width() const
{
	return this->m_image_width;
}

uint32_t pal_vk_swap_chain::get_image_height() const
{
	return this->m_image_height;
}

uint32_t pal_vk_swap_chain::get_image_count() const
{
	return this->m_image_count;
}

pal_color_attachment_image const *pal_vk_swap_chain::get_image(uint32_t swap_chain_image_index) const
{
	return this->m_image_views + swap_chain_image_index;
}

void pal_vk_swap_chain::steal(VkSwapchainKHR *out_swap_chain, uint32_t *out_image_count, pal_vk_swap_chain_image_view **out_image_views)
{
	assert(NULL != out_swap_chain);
	assert(NULL != out_image_views);

	(*out_swap_chain) = this->m_swap_chain;
	(*out_image_count) = this->m_image_count;
	(*out_image_views) = this->m_image_views;

	this->m_swap_chain = VK_NULL_HANDLE;
	this->m_image_views = NULL;
}

pal_vk_swap_chain::~pal_vk_swap_chain()
{
	assert(VK_NULL_HANDLE == this->m_swap_chain);
	assert(NULL == this->m_image_views);
}