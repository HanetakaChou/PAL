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

brx_vk_render_pass::brx_vk_render_pass(VkRenderPass render_pass) : m_render_pass(render_pass)
{
}

VkRenderPass brx_vk_render_pass::get_render_pass() const
{
	return this->m_render_pass;
}

void brx_vk_render_pass::steal(VkRenderPass *out_render_pass)
{
	assert(NULL != out_render_pass);

	(*out_render_pass) = this->m_render_pass;

	this->m_render_pass = VK_NULL_HANDLE;
}

brx_vk_render_pass::~brx_vk_render_pass()
{
	assert(VK_NULL_HANDLE == this->m_render_pass);
}