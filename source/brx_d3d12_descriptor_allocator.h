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

#ifndef _BRX_D3D12_DESCRIPTOR_ALLOCATOR_H_
#define _BRX_D3D12_DESCRIPTOR_ALLOCATOR_H_ 1

#include "brx_map.h"

class brx_d3d12_descriptor_allocator
{
	ID3D12Device *m_device;

	UINT m_cbv_srv_uav_descriptor_heap_descriptor_increment_size;
	ID3D12DescriptorHeap *m_shader_visible_cbv_srv_uav_descriptor_heap;
	ID3D12DescriptorHeap *m_non_shader_visible_cbv_srv_uav_descriptor_heap;
	brx_map<uint32_t, uint32_t> m_cbv_srv_uav_descriptor_heap_free_list;
	brx_map<uint32_t, uint32_t> m_cbv_srv_uav_descriptor_heap_busy_list;
	uint32_t m_cbv_srv_uav_descriptor_heap_num_alloced;
	uint32_t m_cbv_srv_uav_descriptor_heap_next_free_index;

	UINT m_sampler_descriptor_heap_descriptor_increment_size;
	ID3D12DescriptorHeap *m_shader_visible_sampler_descriptor_heap;
	ID3D12DescriptorHeap *m_non_shader_visible_sampler_descriptor_heap;
	brx_map<uint32_t, uint32_t> m_sampler_descriptor_heap_free_list;
	brx_map<uint32_t, uint32_t> m_sampler_descriptor_heap_busy_list;
	uint32_t m_sampler_descriptor_heap_num_alloced;
	uint32_t m_sampler_descriptor_heap_next_free_index;

public:
	brx_d3d12_descriptor_allocator();
	void init(ID3D12Device *new_device);
	void uninit();
	~brx_d3d12_descriptor_allocator();
	// TODO: remove output "alloced_num_descriptors"
	uint32_t alloc_cbv_srv_uav_descriptor(uint32_t num_descriptors, uint32_t *alloced_num_descriptors);
	void free_cbv_srv_uav_descriptor(uint32_t base_descriptor_heap_index, uint32_t alloced_num_descriptors);
	D3D12_CPU_DESCRIPTOR_HANDLE get_shader_visible_cbv_srv_uav_cpu_descriptor_handle(uint32_t descriptor_heap_index) const;
	D3D12_CPU_DESCRIPTOR_HANDLE get_non_shader_visible_cbv_srv_uav_cpu_descriptor_handle(uint32_t descriptor_heap_index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE get_cbv_srv_uav_gpu_descriptor_handle(uint32_t descriptor_heap_index) const;

	uint32_t alloc_sampler_descriptor(uint32_t num_descriptors, uint32_t *alloced_num_descriptors);
	void free_sampler_descriptor(uint32_t base_descriptor_heap_index, uint32_t alloced_num_descriptors);
	D3D12_CPU_DESCRIPTOR_HANDLE get_shader_visible_sampler_cpu_descriptor_handle(uint32_t descriptor_heap_index) const;
	D3D12_CPU_DESCRIPTOR_HANDLE get_non_shader_visible_sampler_cpu_descriptor_handle(uint32_t descriptor_heap_index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE get_sampler_gpu_descriptor_handle(uint32_t descriptor_heap_index) const;

	void bind_command_list(ID3D12GraphicsCommandList *m_command_list) const;
};

#endif