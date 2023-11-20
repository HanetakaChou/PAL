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

brx_d3d12_sampler::brx_d3d12_sampler(D3D12_SAMPLER_DESC const *sampler_desc) : m_sampler_desc(*sampler_desc)
{
}

D3D12_SAMPLER_DESC const *brx_d3d12_sampler::get_sampler_desc() const
{
	return &this->m_sampler_desc;
}