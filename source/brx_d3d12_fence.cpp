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

brx_d3d12_fence::brx_d3d12_fence(ID3D12Fence *fence) : m_fence(fence)
{
}

ID3D12Fence *brx_d3d12_fence::get_fence() const
{
	return this->m_fence;
}

void brx_d3d12_fence::steal(ID3D12Fence **out_fence)
{
	assert(NULL != out_fence);

	(*out_fence) = this->m_fence;

	this->m_fence = NULL;
}

brx_d3d12_fence::~brx_d3d12_fence()
{
	assert(NULL == this->m_fence);
}
