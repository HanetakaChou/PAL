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

brx_vk_fence::brx_vk_fence(VkFence fence) : m_fence(fence)
{
}

VkFence brx_vk_fence::get_fence() const
{
	return this->m_fence;
}

void brx_vk_fence::steal(VkFence *out_fence)
{
	assert(NULL != out_fence);

	(*out_fence) = this->m_fence;

	this->m_fence = VK_NULL_HANDLE;
}

brx_vk_fence::~brx_vk_fence()
{
	assert(VK_NULL_HANDLE == this->m_fence);
}