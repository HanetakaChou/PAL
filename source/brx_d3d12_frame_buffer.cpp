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

brx_d3d12_frame_buffer::brx_d3d12_frame_buffer(brx_vector<ID3D12Resource *> &&render_target_resources, brx_vector<D3D12_CPU_DESCRIPTOR_HANDLE> &&render_target_view_descriptors, ID3D12Resource *depth_stencil_resource, brx_vector<D3D12_CPU_DESCRIPTOR_HANDLE> &&depth_stencil_view_descriptor) : m_render_target_resources(std::move(render_target_resources)), m_render_target_view_descriptors(std::move(render_target_view_descriptors)), m_depth_stencil_resource(depth_stencil_resource), m_depth_stencil_view_descriptor(std::move(depth_stencil_view_descriptor))
{
	assert(this->m_render_target_resources.size() == this->m_render_target_view_descriptors.size());
	assert(((NULL != this->m_depth_stencil_resource) ? 1U : 0U) == this->m_depth_stencil_view_descriptor.size());
}

uint32_t brx_d3d12_frame_buffer::get_num_render_targets() const
{
	assert(this->m_render_target_resources.size() == this->m_render_target_view_descriptors.size());
	return static_cast<uint32_t>(this->m_render_target_resources.size());
}

ID3D12Resource *const *brx_d3d12_frame_buffer::get_render_target_view_resources() const
{
	ID3D12Resource *const *render_target_resource;

	if (this->m_render_target_resources.size() > 0U)
	{
		assert(1U == this->m_render_target_resources.size());
		render_target_resource = &this->m_render_target_resources[0];
	}
	else
	{
		render_target_resource = NULL;
	}

	return render_target_resource;
}

D3D12_CPU_DESCRIPTOR_HANDLE const *brx_d3d12_frame_buffer::get_render_target_view_descriptors() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE const *render_target_view_descriptor;

	if (this->m_render_target_view_descriptors.size() > 0U)
	{
		assert(1U == this->m_render_target_view_descriptors.size());
		render_target_view_descriptor = &this->m_render_target_view_descriptors[0];
	}
	else
	{
		render_target_view_descriptor = NULL;
	}

	return render_target_view_descriptor;
}

ID3D12Resource *brx_d3d12_frame_buffer::get_depth_stencil_resource() const
{
	return this->m_depth_stencil_resource;
}

D3D12_CPU_DESCRIPTOR_HANDLE const *brx_d3d12_frame_buffer::get_depth_stencil_view_descriptor() const
{
	D3D12_CPU_DESCRIPTOR_HANDLE const *depth_stencil_view_descriptor;

	if (this->m_depth_stencil_view_descriptor.size() > 0U)
	{
		assert(1U == this->m_depth_stencil_view_descriptor.size());
		depth_stencil_view_descriptor = &this->m_depth_stencil_view_descriptor[0];
	}
	else
	{
		depth_stencil_view_descriptor = NULL;
	}

	return depth_stencil_view_descriptor;
}
