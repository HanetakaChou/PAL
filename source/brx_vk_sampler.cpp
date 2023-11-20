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

brx_vk_sampler::brx_vk_sampler(VkSampler sampler) : m_sampler(sampler)
{
}

VkSampler brx_vk_sampler::get_sampler() const
{
	return this->m_sampler;
}

void brx_vk_sampler::steal(VkSampler *out_sampler)
{
	assert(NULL != out_sampler);

	(*out_sampler) = this->m_sampler;

	this->m_sampler = VK_NULL_HANDLE;
}

brx_vk_sampler::~brx_vk_sampler()
{
	assert(VK_NULL_HANDLE == this->m_sampler);
}