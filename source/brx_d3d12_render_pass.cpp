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

brx_d3d12_render_pass::brx_d3d12_render_pass(
	brx_vector<BRX_COLOR_ATTACHMENT_IMAGE_FORMAT> &&color_attachment_formats,
	brx_vector<uint32_t> &&color_attachment_clear_indices,
	brx_vector<uint32_t> &&color_attachment_flush_for_sampled_image_indices,
	brx_vector<uint32_t> &&color_attachment_flush_for_present_indices,
	brx_vector<BRX_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT> &&depth_stencil_attachment_format,
	bool depth_stencil_attachment_clear,
	bool depth_stencil_attachment_flush_for_sampled_image)
	: m_color_attachment_formats(std::move(color_attachment_formats)),
	  m_color_attachment_clear_indices(std::move(color_attachment_clear_indices)),
	  m_color_attachment_flush_for_sampled_image_indices(std::move(color_attachment_flush_for_sampled_image_indices)),
	  m_color_attachment_flush_for_present_indices(std::move(color_attachment_flush_for_present_indices)),
	  m_depth_stencil_attachment_format(std::move(depth_stencil_attachment_format)),
	  m_depth_stencil_attachment_clear(depth_stencil_attachment_clear),
	  m_depth_stencil_attachment_flush_for_sampled_image(depth_stencil_attachment_flush_for_sampled_image)
{
	assert(0U == this->m_depth_stencil_attachment_format.size() || 1U == this->m_depth_stencil_attachment_format.size());
	assert((!this->m_depth_stencil_attachment_clear) || 1U == this->m_depth_stencil_attachment_format.size());
	assert((!this->m_depth_stencil_attachment_flush_for_sampled_image) || 1U == this->m_depth_stencil_attachment_format.size());
}

uint32_t brx_d3d12_render_pass::get_color_attachment_count() const
{
	return static_cast<uint32_t>(this->m_color_attachment_formats.size());
}

BRX_COLOR_ATTACHMENT_IMAGE_FORMAT const *brx_d3d12_render_pass::get_color_attachment_formats() const
{
	BRX_COLOR_ATTACHMENT_IMAGE_FORMAT const *color_attachment_formats;

	if (this->m_color_attachment_formats.size() > 0U)
	{
		color_attachment_formats = &this->m_color_attachment_formats[0];
	}
	else
	{
		color_attachment_formats = NULL;
	}

	return color_attachment_formats;
}

uint32_t brx_d3d12_render_pass::get_color_attachments_clear_count() const
{
	return static_cast<uint32_t>(this->m_color_attachment_clear_indices.size());
}

uint32_t const *brx_d3d12_render_pass::get_color_attachment_clear_indices() const
{
	uint32_t const *color_attachment_clear_indices;

	if (this->m_color_attachment_clear_indices.size() > 0U)
	{
		color_attachment_clear_indices = &this->m_color_attachment_clear_indices[0];
	}
	else
	{
		color_attachment_clear_indices = NULL;
	}

	return color_attachment_clear_indices;
}

uint32_t brx_d3d12_render_pass::get_color_attachment_flush_for_sampled_image_count() const
{
	return static_cast<uint32_t>(this->m_color_attachment_flush_for_sampled_image_indices.size());
}

uint32_t const *brx_d3d12_render_pass::get_color_attachment_flush_for_sampled_image_indices() const
{
	uint32_t const *color_attachment_flush_for_sampled_image_indices;

	if (this->m_color_attachment_flush_for_sampled_image_indices.size() > 0U)
	{
		color_attachment_flush_for_sampled_image_indices = &this->m_color_attachment_flush_for_sampled_image_indices[0];
	}
	else
	{
		color_attachment_flush_for_sampled_image_indices = NULL;
	}

	return color_attachment_flush_for_sampled_image_indices;
}

uint32_t brx_d3d12_render_pass::get_color_attachment_flush_for_present_count() const
{
	return static_cast<uint32_t>(this->m_color_attachment_flush_for_present_indices.size());
}

uint32_t const *brx_d3d12_render_pass::get_color_attachment_flush_for_present_indices() const
{
	uint32_t const *color_attachment_flush_for_present_indices;

	if (this->m_color_attachment_flush_for_present_indices.size() > 0U)
	{
		color_attachment_flush_for_present_indices = &this->m_color_attachment_flush_for_present_indices[0];
	}
	else
	{
		color_attachment_flush_for_present_indices = NULL;
	}

	return color_attachment_flush_for_present_indices;
}

BRX_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT const *brx_d3d12_render_pass::get_depth_stencil_attachment_format() const
{
	BRX_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT const *depth_stencil_attachment_format;

	if (this->m_depth_stencil_attachment_format.size() > 0U)
	{
		assert(1U == this->m_depth_stencil_attachment_format.size());
		depth_stencil_attachment_format = &this->m_depth_stencil_attachment_format[0];
	}
	else
	{
		depth_stencil_attachment_format = NULL;
	}

	return depth_stencil_attachment_format;
}

bool brx_d3d12_render_pass::get_depth_stencil_attachment_clear() const
{
	return this->m_depth_stencil_attachment_clear;
}

bool brx_d3d12_render_pass::get_depth_stencil_attachment_flush_for_sampled_image() const
{
	return this->m_depth_stencil_attachment_flush_for_sampled_image;
}