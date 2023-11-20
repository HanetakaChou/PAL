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

brx_vk_frame_buffer::brx_vk_frame_buffer(VkFramebuffer frame_buffer) : m_frame_buffer(frame_buffer)
{
}

VkFramebuffer brx_vk_frame_buffer::get_frame_buffer() const
{
	return this->m_frame_buffer;
}

void brx_vk_frame_buffer::steal(VkFramebuffer *out_frame_buffer)
{
	assert(NULL != out_frame_buffer);

	(*out_frame_buffer) = this->m_frame_buffer;

	this->m_frame_buffer = VK_NULL_HANDLE;
}

brx_vk_frame_buffer::~brx_vk_frame_buffer()
{
	assert(VK_NULL_HANDLE == this->m_frame_buffer);
}
