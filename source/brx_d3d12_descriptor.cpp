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

brx_d3d12_descriptor_set_layout::brx_d3d12_descriptor_set_layout()
{
}

void brx_d3d12_descriptor_set_layout::init(uint32_t descriptor_set_binding_count, BRX_DESCRIPTOR_SET_LAYOUT_BINDING const *wrapped_descriptor_set_bindings)
{
	assert(0U == this->m_descriptor_layouts.size());
	this->m_descriptor_layouts.resize(descriptor_set_binding_count);
	for (uint32_t binding_index = 0U; binding_index < descriptor_set_binding_count; ++binding_index)
	{
		switch (wrapped_descriptor_set_bindings[binding_index].descriptor_type)
		{
		case BRX_DESCRIPTOR_TYPE_DYNAMIC_UNIFORM_BUFFER:
		{
			this->m_descriptor_layouts[binding_index].root_parameter_type = BRX_DESCRIPTOR_TYPE_DYNAMIC_UNIFORM_BUFFER;
			this->m_descriptor_layouts[binding_index].root_parameter_shader_register = wrapped_descriptor_set_bindings[binding_index].binding;
			this->m_descriptor_layouts[binding_index].root_descriptor_table_num_descriptors = -1;
			assert(1U == wrapped_descriptor_set_bindings[binding_index].descriptor_count);
		}
		break;
		case BRX_DESCRIPTOR_TYPE_READ_ONLY_STORAGE_BUFFER:
		{
			this->m_descriptor_layouts[binding_index].root_parameter_type = BRX_DESCRIPTOR_TYPE_READ_ONLY_STORAGE_BUFFER;
			this->m_descriptor_layouts[binding_index].root_parameter_shader_register = wrapped_descriptor_set_bindings[binding_index].binding;
			this->m_descriptor_layouts[binding_index].root_descriptor_table_num_descriptors = wrapped_descriptor_set_bindings[binding_index].descriptor_count;
		}
		break;
		case BRX_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		{
			this->m_descriptor_layouts[binding_index].root_parameter_type = BRX_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			this->m_descriptor_layouts[binding_index].root_parameter_shader_register = wrapped_descriptor_set_bindings[binding_index].binding;
			this->m_descriptor_layouts[binding_index].root_descriptor_table_num_descriptors = wrapped_descriptor_set_bindings[binding_index].descriptor_count;
		}
		break;
		case BRX_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		{
			this->m_descriptor_layouts[binding_index].root_parameter_type = BRX_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			this->m_descriptor_layouts[binding_index].root_parameter_shader_register = wrapped_descriptor_set_bindings[binding_index].binding;
			this->m_descriptor_layouts[binding_index].root_descriptor_table_num_descriptors = wrapped_descriptor_set_bindings[binding_index].descriptor_count;
		}
		break;
		case BRX_DESCRIPTOR_TYPE_SAMPLER:
		{
			this->m_descriptor_layouts[binding_index].root_parameter_type = BRX_DESCRIPTOR_TYPE_SAMPLER;
			this->m_descriptor_layouts[binding_index].root_parameter_shader_register = wrapped_descriptor_set_bindings[binding_index].binding;
			this->m_descriptor_layouts[binding_index].root_descriptor_table_num_descriptors = wrapped_descriptor_set_bindings[binding_index].descriptor_count;
		}
		break;
		case BRX_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		{
			this->m_descriptor_layouts[binding_index].root_parameter_type = BRX_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			this->m_descriptor_layouts[binding_index].root_parameter_shader_register = wrapped_descriptor_set_bindings[binding_index].binding;
			this->m_descriptor_layouts[binding_index].root_descriptor_table_num_descriptors = wrapped_descriptor_set_bindings[binding_index].descriptor_count;
		}
		break;
		case BRX_DESCRIPTOR_TYPE_TOP_LEVEL_ACCELERATION_STRUCTURE:
		{
			this->m_descriptor_layouts[binding_index].root_parameter_type = BRX_DESCRIPTOR_TYPE_TOP_LEVEL_ACCELERATION_STRUCTURE;
			this->m_descriptor_layouts[binding_index].root_parameter_shader_register = wrapped_descriptor_set_bindings[binding_index].binding;
			this->m_descriptor_layouts[binding_index].root_descriptor_table_num_descriptors = wrapped_descriptor_set_bindings[binding_index].descriptor_count;
		}
		break;
		default:
		{
			assert(false);
			this->m_descriptor_layouts[binding_index].root_parameter_type = static_cast<BRX_DESCRIPTOR_TYPE>(-1);
		}
		}
	}
}

void brx_d3d12_descriptor_set_layout::uninit()
{
}

uint32_t brx_d3d12_descriptor_set_layout::get_descriptor_layout_count() const
{
	return static_cast<uint32_t>(this->m_descriptor_layouts.size());
}

brx_d3d12_descriptor_layout const *brx_d3d12_descriptor_set_layout::get_descriptor_layouts() const
{
	brx_d3d12_descriptor_layout const *descriptor_layouts;

	if (this->m_descriptor_layouts.size() > 0U)
	{
		descriptor_layouts = &this->m_descriptor_layouts[0];
	}
	else
	{
		descriptor_layouts = NULL;
	}

	return descriptor_layouts;
}

brx_d3d12_pipeline_layout::brx_d3d12_pipeline_layout() : m_root_signature(NULL)
{
}

void brx_d3d12_pipeline_layout::init(ID3D12Device *device, decltype(D3D12SerializeRootSignature) *pfn_d3d12_serialize_root_signature, uint32_t descriptor_set_layout_count, brx_descriptor_set_layout const *const *descriptor_set_layouts)
{
	brx_vector<D3D12_ROOT_PARAMETER> root_parameters;
	brx_vector<D3D12_DESCRIPTOR_RANGE> root_descriptor_table_ranges;
	for (uint32_t set_index = 0U; set_index < descriptor_set_layout_count; ++set_index)
	{
		uint32_t binding_count = static_cast<brx_d3d12_descriptor_set_layout const *>(descriptor_set_layouts[set_index])->get_descriptor_layout_count();
		brx_d3d12_descriptor_layout const *descriptor_layouts = static_cast<brx_d3d12_descriptor_set_layout const *>(descriptor_set_layouts[set_index])->get_descriptor_layouts();
		for (uint32_t binding_index = 0U; binding_index < binding_count; ++binding_index)
		{
			switch (descriptor_layouts[binding_index].root_parameter_type)
			{
			case BRX_DESCRIPTOR_TYPE_DYNAMIC_UNIFORM_BUFFER:
			{
				uint32_t const new_root_parameter_index = static_cast<uint32_t>(root_parameters.size());
				root_parameters.emplace_back();

				root_parameters[new_root_parameter_index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
				root_parameters[new_root_parameter_index].Descriptor.ShaderRegister = descriptor_layouts[binding_index].root_parameter_shader_register;
				root_parameters[new_root_parameter_index].Descriptor.RegisterSpace = set_index;
				root_parameters[new_root_parameter_index].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			}
			break;
			case BRX_DESCRIPTOR_TYPE_READ_ONLY_STORAGE_BUFFER:
			{
				uint32_t const new_root_descriptor_table_range_index = static_cast<uint32_t>(root_descriptor_table_ranges.size());
				root_descriptor_table_ranges.emplace_back();

				root_descriptor_table_ranges[new_root_descriptor_table_range_index].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].NumDescriptors = descriptor_layouts[binding_index].root_descriptor_table_num_descriptors;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].BaseShaderRegister = descriptor_layouts[binding_index].root_parameter_shader_register;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].RegisterSpace = set_index;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].OffsetInDescriptorsFromTableStart = 0U;

				uint32_t const new_root_parameter_index = static_cast<uint32_t>(root_parameters.size());
				root_parameters.emplace_back();

				root_parameters[new_root_parameter_index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				root_parameters[new_root_parameter_index].DescriptorTable.NumDescriptorRanges = 1U;
				root_parameters[new_root_parameter_index].DescriptorTable.pDescriptorRanges = reinterpret_cast<D3D12_DESCRIPTOR_RANGE *>(static_cast<uintptr_t>(new_root_descriptor_table_range_index));
				root_parameters[new_root_parameter_index].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			}
			break;
			case BRX_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			{
				uint32_t const new_root_descriptor_table_range_index = static_cast<uint32_t>(root_descriptor_table_ranges.size());
				root_descriptor_table_ranges.emplace_back();

				root_descriptor_table_ranges[new_root_descriptor_table_range_index].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].NumDescriptors = descriptor_layouts[binding_index].root_descriptor_table_num_descriptors;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].BaseShaderRegister = descriptor_layouts[binding_index].root_parameter_shader_register;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].RegisterSpace = set_index;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].OffsetInDescriptorsFromTableStart = 0U;

				uint32_t const new_root_parameter_index = static_cast<uint32_t>(root_parameters.size());
				root_parameters.emplace_back();

				root_parameters[new_root_parameter_index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				root_parameters[new_root_parameter_index].DescriptorTable.NumDescriptorRanges = 1U;
				root_parameters[new_root_parameter_index].DescriptorTable.pDescriptorRanges = reinterpret_cast<D3D12_DESCRIPTOR_RANGE *>(static_cast<uintptr_t>(new_root_descriptor_table_range_index));
				root_parameters[new_root_parameter_index].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			}
			break;
			case BRX_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			{
				uint32_t const new_root_descriptor_table_range_index = static_cast<uint32_t>(root_descriptor_table_ranges.size());
				root_descriptor_table_ranges.emplace_back();

				root_descriptor_table_ranges[new_root_descriptor_table_range_index].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].NumDescriptors = descriptor_layouts[binding_index].root_descriptor_table_num_descriptors;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].BaseShaderRegister = descriptor_layouts[binding_index].root_parameter_shader_register;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].RegisterSpace = set_index;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].OffsetInDescriptorsFromTableStart = 0U;

				uint32_t const new_root_parameter_index = static_cast<uint32_t>(root_parameters.size());
				root_parameters.emplace_back();

				root_parameters[new_root_parameter_index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				root_parameters[new_root_parameter_index].DescriptorTable.NumDescriptorRanges = 1U;
				root_parameters[new_root_parameter_index].DescriptorTable.pDescriptorRanges = reinterpret_cast<D3D12_DESCRIPTOR_RANGE *>(static_cast<uintptr_t>(new_root_descriptor_table_range_index));
				root_parameters[new_root_parameter_index].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			}
			break;
			case BRX_DESCRIPTOR_TYPE_SAMPLER:
			{
				uint32_t const new_root_descriptor_table_range_index = static_cast<uint32_t>(root_descriptor_table_ranges.size());
				root_descriptor_table_ranges.emplace_back();

				root_descriptor_table_ranges[new_root_descriptor_table_range_index].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].NumDescriptors = descriptor_layouts[binding_index].root_descriptor_table_num_descriptors;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].BaseShaderRegister = descriptor_layouts[binding_index].root_parameter_shader_register;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].RegisterSpace = set_index;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].OffsetInDescriptorsFromTableStart = 0U;

				uint32_t const new_root_parameter_index = static_cast<uint32_t>(root_parameters.size());
				root_parameters.emplace_back();

				root_parameters[new_root_parameter_index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				root_parameters[new_root_parameter_index].DescriptorTable.NumDescriptorRanges = 1U;
				root_parameters[new_root_parameter_index].DescriptorTable.pDescriptorRanges = reinterpret_cast<D3D12_DESCRIPTOR_RANGE *>(static_cast<uintptr_t>(new_root_descriptor_table_range_index));
				root_parameters[new_root_parameter_index].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			}
			break;
			case BRX_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			{
				uint32_t const new_root_descriptor_table_range_index = static_cast<uint32_t>(root_descriptor_table_ranges.size());
				root_descriptor_table_ranges.emplace_back();

				root_descriptor_table_ranges[new_root_descriptor_table_range_index].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].NumDescriptors = descriptor_layouts[binding_index].root_descriptor_table_num_descriptors;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].BaseShaderRegister = descriptor_layouts[binding_index].root_parameter_shader_register;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].RegisterSpace = set_index;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].OffsetInDescriptorsFromTableStart = 0U;

				uint32_t const new_root_parameter_index = static_cast<uint32_t>(root_parameters.size());
				root_parameters.emplace_back();

				root_parameters[new_root_parameter_index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				root_parameters[new_root_parameter_index].DescriptorTable.NumDescriptorRanges = 1U;
				root_parameters[new_root_parameter_index].DescriptorTable.pDescriptorRanges = reinterpret_cast<D3D12_DESCRIPTOR_RANGE *>(static_cast<uintptr_t>(new_root_descriptor_table_range_index));
				root_parameters[new_root_parameter_index].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			}
			break;
			case BRX_DESCRIPTOR_TYPE_TOP_LEVEL_ACCELERATION_STRUCTURE:
			{
				uint32_t const new_root_descriptor_table_range_index = static_cast<uint32_t>(root_descriptor_table_ranges.size());
				root_descriptor_table_ranges.emplace_back();

				root_descriptor_table_ranges[new_root_descriptor_table_range_index].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].NumDescriptors = descriptor_layouts[binding_index].root_descriptor_table_num_descriptors;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].BaseShaderRegister = descriptor_layouts[binding_index].root_parameter_shader_register;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].RegisterSpace = set_index;
				root_descriptor_table_ranges[new_root_descriptor_table_range_index].OffsetInDescriptorsFromTableStart = 0U;

				uint32_t const new_root_parameter_index = static_cast<uint32_t>(root_parameters.size());
				root_parameters.emplace_back();

				root_parameters[new_root_parameter_index].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				root_parameters[new_root_parameter_index].DescriptorTable.NumDescriptorRanges = 1U;
				root_parameters[new_root_parameter_index].DescriptorTable.pDescriptorRanges = reinterpret_cast<D3D12_DESCRIPTOR_RANGE *>(static_cast<uintptr_t>(new_root_descriptor_table_range_index));
				root_parameters[new_root_parameter_index].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			}
			break;
			default:
			{
				assert(false);
			}
			}
		}
	}

	// The memory address within "root_descriptor_table_ranges" may change when "emplace_back"
	uint32_t const num_root_parameters = static_cast<uint32_t>(root_parameters.size());
	for (uint32_t root_parameter_index = 0U; root_parameter_index < num_root_parameters; ++root_parameter_index)
	{
		if (D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE == root_parameters[root_parameter_index].ParameterType)
		{
			uint32_t const new_root_descriptor_table_range_index = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(root_parameters[root_parameter_index].DescriptorTable.pDescriptorRanges));
			root_parameters[root_parameter_index].DescriptorTable.pDescriptorRanges = &root_descriptor_table_ranges[new_root_descriptor_table_range_index];
		}
	}

	D3D12_ROOT_SIGNATURE_DESC const root_signature_desc = {
		num_root_parameters,
		&root_parameters[0],
		0U,
		NULL,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT};

	ID3DBlob *new_blob_with_root_signature = NULL;
	HRESULT const hr_serialize_root_signature = pfn_d3d12_serialize_root_signature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &new_blob_with_root_signature, NULL);
	assert(SUCCEEDED(hr_serialize_root_signature));

	assert(NULL == this->m_root_signature);
	HRESULT const hr_create_root_signature = device->CreateRootSignature(0U, new_blob_with_root_signature->GetBufferPointer(), new_blob_with_root_signature->GetBufferSize(), IID_PPV_ARGS(&this->m_root_signature));
	assert(SUCCEEDED(hr_create_root_signature));

	new_blob_with_root_signature->Release();
}

void brx_d3d12_pipeline_layout::uninit()
{
	assert(NULL != this->m_root_signature);

	this->m_root_signature->Release();

	this->m_root_signature = NULL;
}

brx_d3d12_pipeline_layout::~brx_d3d12_pipeline_layout()
{
	assert(NULL == this->m_root_signature);
}

ID3D12RootSignature *brx_d3d12_pipeline_layout::get_root_signature() const
{
	return this->m_root_signature;
}

brx_d3d12_descriptor_set::brx_d3d12_descriptor_set()
{
}

void brx_d3d12_descriptor_set::init(brx_d3d12_descriptor_allocator *descriptor_allocator, brx_descriptor_set_layout const *wrapped_descriptor_set_layout)
{
	assert(NULL != wrapped_descriptor_set_layout);
	brx_d3d12_descriptor_set_layout const *const unwrapped_descriptor_set_layout = static_cast<brx_d3d12_descriptor_set_layout const *>(wrapped_descriptor_set_layout);

	uint32_t const descriptor_layout_count = unwrapped_descriptor_set_layout->get_descriptor_layout_count();
	brx_d3d12_descriptor_layout const *unwrapped_descriptor_layouts = unwrapped_descriptor_set_layout->get_descriptor_layouts();

	assert(0U == this->m_descriptors.size());
	this->m_descriptors.resize(descriptor_layout_count);
	for (uint32_t descriptor_layout_index = 0U; descriptor_layout_index < descriptor_layout_count; ++descriptor_layout_index)
	{
		brx_d3d12_descriptor_layout const &unwrapped_descriptor_layout = unwrapped_descriptor_layouts[descriptor_layout_index];
		brx_d3d12_descriptor &new_descriptor = this->m_descriptors[descriptor_layout_index];

		switch (unwrapped_descriptor_layout.root_parameter_type)
		{
		case BRX_DESCRIPTOR_TYPE_DYNAMIC_UNIFORM_BUFFER:
		{
			new_descriptor.root_parameter_type = unwrapped_descriptor_layout.root_parameter_type;
			new_descriptor.root_parameter_shader_register = unwrapped_descriptor_layout.root_parameter_shader_register;
		}
		break;
		case BRX_DESCRIPTOR_TYPE_READ_ONLY_STORAGE_BUFFER:
		{
			new_descriptor.root_parameter_type = unwrapped_descriptor_layout.root_parameter_type;
			new_descriptor.root_parameter_shader_register = unwrapped_descriptor_layout.root_parameter_shader_register;
			new_descriptor.root_descriptor_table.num_descriptors = unwrapped_descriptor_layout.root_descriptor_table_num_descriptors;
			new_descriptor.root_descriptor_table.base_descriptor_heap_index = descriptor_allocator->alloc_cbv_srv_uav_descriptor(new_descriptor.root_descriptor_table.num_descriptors, &new_descriptor.root_descriptor_table.alloced_num_descriptors);
			assert(new_descriptor.root_descriptor_table.num_descriptors <= new_descriptor.root_descriptor_table.alloced_num_descriptors);
		}
		break;
		case BRX_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		{
			new_descriptor.root_parameter_type = unwrapped_descriptor_layout.root_parameter_type;
			new_descriptor.root_parameter_shader_register = unwrapped_descriptor_layout.root_parameter_shader_register;
			new_descriptor.root_descriptor_table.num_descriptors = unwrapped_descriptor_layout.root_descriptor_table_num_descriptors;
			new_descriptor.root_descriptor_table.base_descriptor_heap_index = descriptor_allocator->alloc_cbv_srv_uav_descriptor(new_descriptor.root_descriptor_table.num_descriptors, &new_descriptor.root_descriptor_table.alloced_num_descriptors);
			assert(new_descriptor.root_descriptor_table.num_descriptors <= new_descriptor.root_descriptor_table.alloced_num_descriptors);
		}
		break;
		case BRX_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		{
			new_descriptor.root_parameter_type = unwrapped_descriptor_layout.root_parameter_type;
			new_descriptor.root_parameter_shader_register = unwrapped_descriptor_layout.root_parameter_shader_register;
			new_descriptor.root_descriptor_table.num_descriptors = unwrapped_descriptor_layout.root_descriptor_table_num_descriptors;
			new_descriptor.root_descriptor_table.base_descriptor_heap_index = descriptor_allocator->alloc_cbv_srv_uav_descriptor(new_descriptor.root_descriptor_table.num_descriptors, &new_descriptor.root_descriptor_table.alloced_num_descriptors);
			assert(new_descriptor.root_descriptor_table.num_descriptors <= new_descriptor.root_descriptor_table.alloced_num_descriptors);
		}
		break;
		case BRX_DESCRIPTOR_TYPE_SAMPLER:
		{
			new_descriptor.root_parameter_type = unwrapped_descriptor_layout.root_parameter_type;
			new_descriptor.root_parameter_shader_register = unwrapped_descriptor_layout.root_parameter_shader_register;
			new_descriptor.root_descriptor_table.num_descriptors = unwrapped_descriptor_layout.root_descriptor_table_num_descriptors;
			new_descriptor.root_descriptor_table.base_descriptor_heap_index = descriptor_allocator->alloc_sampler_descriptor(new_descriptor.root_descriptor_table.num_descriptors, &new_descriptor.root_descriptor_table.alloced_num_descriptors);
			assert(new_descriptor.root_descriptor_table.num_descriptors <= new_descriptor.root_descriptor_table.alloced_num_descriptors);
		}
		break;
		case BRX_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		{
			new_descriptor.root_parameter_type = unwrapped_descriptor_layout.root_parameter_type;
			new_descriptor.root_parameter_shader_register = unwrapped_descriptor_layout.root_parameter_shader_register;
			new_descriptor.root_descriptor_table.num_descriptors = unwrapped_descriptor_layout.root_descriptor_table_num_descriptors;
			new_descriptor.root_descriptor_table.base_descriptor_heap_index = descriptor_allocator->alloc_cbv_srv_uav_descriptor(new_descriptor.root_descriptor_table.num_descriptors, &new_descriptor.root_descriptor_table.alloced_num_descriptors);
			assert(new_descriptor.root_descriptor_table.num_descriptors <= new_descriptor.root_descriptor_table.alloced_num_descriptors);
		}
		break;
		case BRX_DESCRIPTOR_TYPE_TOP_LEVEL_ACCELERATION_STRUCTURE:
		{
			new_descriptor.root_parameter_type = unwrapped_descriptor_layout.root_parameter_type;
			new_descriptor.root_parameter_shader_register = unwrapped_descriptor_layout.root_parameter_shader_register;
			new_descriptor.root_descriptor_table.num_descriptors = unwrapped_descriptor_layout.root_descriptor_table_num_descriptors;
			new_descriptor.root_descriptor_table.base_descriptor_heap_index = descriptor_allocator->alloc_cbv_srv_uav_descriptor(new_descriptor.root_descriptor_table.num_descriptors, &new_descriptor.root_descriptor_table.alloced_num_descriptors);
			assert(new_descriptor.root_descriptor_table.num_descriptors <= new_descriptor.root_descriptor_table.alloced_num_descriptors);
		}
		break;
		default:
		{
			assert(false);
			new_descriptor.root_parameter_type = static_cast<BRX_DESCRIPTOR_TYPE>(-1);
		}
		}
	}
}

void brx_d3d12_descriptor_set::uninit(brx_d3d12_descriptor_allocator *descriptor_allocator)
{
	for (brx_d3d12_descriptor const &delete_descriptor : this->m_descriptors)
	{
		switch (delete_descriptor.root_parameter_type)
		{
		case BRX_DESCRIPTOR_TYPE_DYNAMIC_UNIFORM_BUFFER:
		{
		}
		break;
		case BRX_DESCRIPTOR_TYPE_READ_ONLY_STORAGE_BUFFER:
		{
			descriptor_allocator->free_cbv_srv_uav_descriptor(delete_descriptor.root_descriptor_table.base_descriptor_heap_index, delete_descriptor.root_descriptor_table.alloced_num_descriptors);
		}
		break;
		case BRX_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		{
			descriptor_allocator->free_cbv_srv_uav_descriptor(delete_descriptor.root_descriptor_table.base_descriptor_heap_index, delete_descriptor.root_descriptor_table.alloced_num_descriptors);
		}
		break;
		case BRX_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		{
			descriptor_allocator->free_cbv_srv_uav_descriptor(delete_descriptor.root_descriptor_table.base_descriptor_heap_index, delete_descriptor.root_descriptor_table.alloced_num_descriptors);
		}
		break;
		case BRX_DESCRIPTOR_TYPE_SAMPLER:
		{
			descriptor_allocator->free_sampler_descriptor(delete_descriptor.root_descriptor_table.base_descriptor_heap_index, delete_descriptor.root_descriptor_table.alloced_num_descriptors);
		}
		break;
		case BRX_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		{
			descriptor_allocator->free_cbv_srv_uav_descriptor(delete_descriptor.root_descriptor_table.base_descriptor_heap_index, delete_descriptor.root_descriptor_table.alloced_num_descriptors);
		}
		break;
		case BRX_DESCRIPTOR_TYPE_TOP_LEVEL_ACCELERATION_STRUCTURE:
		{
			descriptor_allocator->free_cbv_srv_uav_descriptor(delete_descriptor.root_descriptor_table.base_descriptor_heap_index, delete_descriptor.root_descriptor_table.alloced_num_descriptors);
		}
		break;
		default:
			assert(false);
		}
	}

	this->m_descriptors.clear();
}

brx_d3d12_descriptor_set::~brx_d3d12_descriptor_set()
{
	assert(0U == this->m_descriptors.size());
}

void brx_d3d12_descriptor_set::write_descriptor(ID3D12Device *device, brx_d3d12_descriptor_allocator const *descriptor_allocator, BRX_DESCRIPTOR_TYPE wrapped_descriptor_type, uint32_t dst_binding, uint32_t dst_array_element, uint32_t src_descriptor_count, brx_uniform_upload_buffer const *const *src_dynamic_uniform_buffers, uint32_t const *src_dynamic_uniform_buffer_ranges, brx_storage_buffer const *const *src_storage_buffers, brx_sampled_image const *const *src_sampled_images, brx_sampler const *const *src_samplers, brx_storage_image const *const *src_storage_images, brx_top_level_acceleration_structure const *const *src_top_level_acceleration_structures)
{
	uint32_t destination_descriptor_layout_index = -1;
	{
		uint32_t const descriptor_layout_count = static_cast<uint32_t>(this->m_descriptors.size());
		assert(0U < descriptor_layout_count);

		for (uint32_t descriptor_layout_index = 0U; descriptor_layout_index < descriptor_layout_count; ++descriptor_layout_index)
		{
			if (this->m_descriptors[descriptor_layout_index].root_parameter_shader_register == dst_binding)
			{
				destination_descriptor_layout_index = descriptor_layout_index;
				break;
			}
		}

		assert(static_cast<uint32_t>(-1) != destination_descriptor_layout_index);
	}

	brx_d3d12_descriptor &destination_descriptor = this->m_descriptors[destination_descriptor_layout_index];

	switch (wrapped_descriptor_type)
	{
	case BRX_DESCRIPTOR_TYPE_DYNAMIC_UNIFORM_BUFFER:
	{
		assert(0U == dst_array_element);
		assert(1U == src_descriptor_count);

		assert(NULL != src_dynamic_uniform_buffers);
		assert(NULL != src_dynamic_uniform_buffer_ranges);
		assert(NULL == src_storage_buffers);
		assert(NULL == src_sampled_images);
		assert(NULL == src_samplers);
		assert(NULL == src_storage_images);
		assert(NULL == src_top_level_acceleration_structures);

		assert(NULL != src_dynamic_uniform_buffers[0]);
		assert(NULL != src_dynamic_uniform_buffer_ranges[0]);
		destination_descriptor.root_constant_buffer_view.address_base = static_cast<brx_d3d12_uniform_upload_buffer const *>(src_dynamic_uniform_buffers[0])->get_resource()->GetGPUVirtualAddress();
	}
	break;
	case BRX_DESCRIPTOR_TYPE_READ_ONLY_STORAGE_BUFFER:
	{
		assert(NULL == src_dynamic_uniform_buffers);
		assert(NULL == src_dynamic_uniform_buffer_ranges);
		assert(NULL != src_storage_buffers);
		assert(NULL == src_sampled_images);
		assert(NULL == src_samplers);
		assert(NULL == src_storage_images);
		assert(NULL == src_top_level_acceleration_structures);

		for (uint32_t descriptor_index = 0U; descriptor_index < src_descriptor_count; ++descriptor_index)
		{
			assert(NULL != src_storage_buffers[descriptor_index]);
			assert((dst_array_element + descriptor_index) < destination_descriptor.root_descriptor_table.num_descriptors);

			ID3D12Resource *const resource = static_cast<brx_d3d12_storage_buffer const *>(src_storage_buffers[descriptor_index])->get_resource();
			D3D12_SHADER_RESOURCE_VIEW_DESC const *const shader_resource_view_desc = static_cast<brx_d3d12_storage_buffer const *>(src_storage_buffers[descriptor_index])->get_shader_resource_view_desc();

			D3D12_CPU_DESCRIPTOR_HANDLE const shader_visible_shader_resource_view_descriptor = descriptor_allocator->get_shader_visible_cbv_srv_uav_cpu_descriptor_handle(destination_descriptor.root_descriptor_table.base_descriptor_heap_index + dst_array_element + descriptor_index);
			D3D12_CPU_DESCRIPTOR_HANDLE const non_shader_visible_shader_resource_view_descriptor = descriptor_allocator->get_non_shader_visible_cbv_srv_uav_cpu_descriptor_handle(destination_descriptor.root_descriptor_table.base_descriptor_heap_index + dst_array_element + descriptor_index);

			device->CreateShaderResourceView(resource, shader_resource_view_desc, shader_visible_shader_resource_view_descriptor);
			device->CreateShaderResourceView(resource, shader_resource_view_desc, non_shader_visible_shader_resource_view_descriptor);
		}
	}
	break;
	case BRX_DESCRIPTOR_TYPE_STORAGE_BUFFER:
	{
		assert(NULL == src_dynamic_uniform_buffers);
		assert(NULL == src_dynamic_uniform_buffer_ranges);
		assert(NULL != src_storage_buffers);
		assert(NULL == src_sampled_images);
		assert(NULL == src_samplers);
		assert(NULL == src_storage_images);
		assert(NULL == src_top_level_acceleration_structures);

		for (uint32_t descriptor_index = 0U; descriptor_index < src_descriptor_count; ++descriptor_index)
		{
			assert(NULL != src_storage_buffers[descriptor_index]);
			assert((dst_array_element + descriptor_index) < destination_descriptor.root_descriptor_table.num_descriptors);

			ID3D12Resource *const resource = static_cast<brx_d3d12_storage_buffer const *>(src_storage_buffers[descriptor_index])->get_resource();
			D3D12_UNORDERED_ACCESS_VIEW_DESC unordered_access_view_desc = (*static_cast<brx_d3d12_storage_buffer const *>(src_storage_buffers[descriptor_index])->get_unordered_access_view_desc());

			D3D12_CPU_DESCRIPTOR_HANDLE const shader_visible_shader_resource_view_descriptor = descriptor_allocator->get_shader_visible_cbv_srv_uav_cpu_descriptor_handle(destination_descriptor.root_descriptor_table.base_descriptor_heap_index + dst_array_element + descriptor_index);
			D3D12_CPU_DESCRIPTOR_HANDLE const non_shader_visible_shader_resource_view_descriptor = descriptor_allocator->get_non_shader_visible_cbv_srv_uav_cpu_descriptor_handle(destination_descriptor.root_descriptor_table.base_descriptor_heap_index + dst_array_element + descriptor_index);

			device->CreateUnorderedAccessView(resource, NULL, &unordered_access_view_desc, shader_visible_shader_resource_view_descriptor);
			device->CreateUnorderedAccessView(resource, NULL, &unordered_access_view_desc, non_shader_visible_shader_resource_view_descriptor);
		}
	}
	break;
	case BRX_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
	{
		assert(NULL == src_dynamic_uniform_buffers);
		assert(NULL == src_dynamic_uniform_buffer_ranges);
		assert(NULL == src_storage_buffers);
		assert(NULL != src_sampled_images);
		assert(NULL == src_samplers);
		assert(NULL == src_storage_images);
		assert(NULL == src_top_level_acceleration_structures);

		for (uint32_t descriptor_index = 0U; descriptor_index < src_descriptor_count; ++descriptor_index)
		{
			assert(NULL != src_sampled_images[descriptor_index]);
			assert((dst_array_element + descriptor_index) < destination_descriptor.root_descriptor_table.num_descriptors);

			ID3D12Resource *const resource = static_cast<brx_d3d12_sampled_image const *>(src_sampled_images[descriptor_index])->get_resource();
			D3D12_SHADER_RESOURCE_VIEW_DESC const *const shader_resource_view_desc = static_cast<brx_d3d12_sampled_image const *>(src_sampled_images[descriptor_index])->get_shader_resource_view_desc();

			D3D12_CPU_DESCRIPTOR_HANDLE const shader_visible_shader_resource_view_descriptor = descriptor_allocator->get_shader_visible_cbv_srv_uav_cpu_descriptor_handle(destination_descriptor.root_descriptor_table.base_descriptor_heap_index + dst_array_element + descriptor_index);
			D3D12_CPU_DESCRIPTOR_HANDLE const non_shader_visible_shader_resource_view_descriptor = descriptor_allocator->get_non_shader_visible_cbv_srv_uav_cpu_descriptor_handle(destination_descriptor.root_descriptor_table.base_descriptor_heap_index + dst_array_element + descriptor_index);

			device->CreateShaderResourceView(resource, shader_resource_view_desc, shader_visible_shader_resource_view_descriptor);
			device->CreateShaderResourceView(resource, shader_resource_view_desc, non_shader_visible_shader_resource_view_descriptor);
		}
	}
	break;
	case BRX_DESCRIPTOR_TYPE_SAMPLER:
	{
		assert(NULL == src_dynamic_uniform_buffers);
		assert(NULL == src_dynamic_uniform_buffer_ranges);
		assert(NULL == src_storage_buffers);
		assert(NULL == src_sampled_images);
		assert(NULL != src_samplers);
		assert(NULL == src_storage_images);
		assert(NULL == src_top_level_acceleration_structures);

		for (uint32_t descriptor_index = 0U; descriptor_index < src_descriptor_count; ++descriptor_index)
		{
			assert(NULL != src_samplers[descriptor_index]);
			assert((dst_array_element + descriptor_index) < destination_descriptor.root_descriptor_table.num_descriptors);

			D3D12_SAMPLER_DESC const *const sampler_desc = static_cast<brx_d3d12_sampler const *>(src_samplers[descriptor_index])->get_sampler_desc();

			D3D12_CPU_DESCRIPTOR_HANDLE const shader_visible_sampler_descriptor = descriptor_allocator->get_shader_visible_sampler_cpu_descriptor_handle(destination_descriptor.root_descriptor_table.base_descriptor_heap_index + dst_array_element + descriptor_index);
			D3D12_CPU_DESCRIPTOR_HANDLE const non_shader_visible_sampler_descriptor = descriptor_allocator->get_non_shader_visible_sampler_cpu_descriptor_handle(destination_descriptor.root_descriptor_table.base_descriptor_heap_index + dst_array_element + descriptor_index);

			device->CreateSampler(sampler_desc, shader_visible_sampler_descriptor);
			device->CreateSampler(sampler_desc, non_shader_visible_sampler_descriptor);
		}
	}
	break;
	case BRX_DESCRIPTOR_TYPE_STORAGE_IMAGE:
	{
		assert(NULL == src_dynamic_uniform_buffers);
		assert(NULL == src_dynamic_uniform_buffer_ranges);
		assert(NULL == src_storage_buffers);
		assert(NULL == src_sampled_images);
		assert(NULL == src_samplers);
		assert(NULL != src_storage_images);
		assert(NULL == src_top_level_acceleration_structures);

		for (uint32_t descriptor_index = 0U; descriptor_index < src_descriptor_count; ++descriptor_index)
		{
			assert(NULL != src_storage_images[descriptor_index]);
			assert((dst_array_element + descriptor_index) < destination_descriptor.root_descriptor_table.num_descriptors);

			ID3D12Resource *const resource = static_cast<brx_d3d12_storage_image const *>(src_storage_images[descriptor_index])->get_resource();
			D3D12_UNORDERED_ACCESS_VIEW_DESC const *const unordered_access_view_desc = static_cast<brx_d3d12_storage_image const *>(src_storage_images[descriptor_index])->get_unordered_access_view_desc();

			D3D12_CPU_DESCRIPTOR_HANDLE const shader_visible_shader_resource_view_descriptor = descriptor_allocator->get_shader_visible_cbv_srv_uav_cpu_descriptor_handle(destination_descriptor.root_descriptor_table.base_descriptor_heap_index + dst_array_element + descriptor_index);
			D3D12_CPU_DESCRIPTOR_HANDLE const non_shader_visible_shader_resource_view_descriptor = descriptor_allocator->get_non_shader_visible_cbv_srv_uav_cpu_descriptor_handle(destination_descriptor.root_descriptor_table.base_descriptor_heap_index + dst_array_element + descriptor_index);

			device->CreateUnorderedAccessView(resource, NULL, unordered_access_view_desc, shader_visible_shader_resource_view_descriptor);
			device->CreateUnorderedAccessView(resource, NULL, unordered_access_view_desc, non_shader_visible_shader_resource_view_descriptor);
		}
	}
	break;
	case BRX_DESCRIPTOR_TYPE_TOP_LEVEL_ACCELERATION_STRUCTURE:
	{
		assert(NULL == src_dynamic_uniform_buffers);
		assert(NULL == src_dynamic_uniform_buffer_ranges);
		assert(NULL == src_storage_buffers);
		assert(NULL == src_sampled_images);
		assert(NULL == src_samplers);
		assert(NULL == src_storage_images);
		assert(NULL != src_top_level_acceleration_structures);

		for (uint32_t descriptor_index = 0U; descriptor_index < src_descriptor_count; ++descriptor_index)
		{
			assert(NULL != src_top_level_acceleration_structures[descriptor_index]);
			assert((dst_array_element + descriptor_index) < destination_descriptor.root_descriptor_table.num_descriptors);

			D3D12_SHADER_RESOURCE_VIEW_DESC const *const shader_resource_view_desc = static_cast<brx_d3d12_top_level_acceleration_structure const *>(src_top_level_acceleration_structures[descriptor_index])->get_shader_resource_view_desc();

			D3D12_CPU_DESCRIPTOR_HANDLE const shader_visible_shader_resource_view_descriptor = descriptor_allocator->get_shader_visible_cbv_srv_uav_cpu_descriptor_handle(destination_descriptor.root_descriptor_table.base_descriptor_heap_index + dst_array_element + descriptor_index);
			D3D12_CPU_DESCRIPTOR_HANDLE const non_shader_visible_shader_resource_view_descriptor = descriptor_allocator->get_non_shader_visible_cbv_srv_uav_cpu_descriptor_handle(destination_descriptor.root_descriptor_table.base_descriptor_heap_index + dst_array_element + descriptor_index);

			device->CreateShaderResourceView(NULL, shader_resource_view_desc, shader_visible_shader_resource_view_descriptor);
			device->CreateShaderResourceView(NULL, shader_resource_view_desc, non_shader_visible_shader_resource_view_descriptor);
		}
	}
	break;
	default:
	{
		assert(false);
	}
	}
}

uint32_t brx_d3d12_descriptor_set::get_descriptor_count() const
{
	return static_cast<uint32_t>(this->m_descriptors.size());
}

brx_d3d12_descriptor const *brx_d3d12_descriptor_set::get_descriptors() const
{
	return this->m_descriptors.data();
}
