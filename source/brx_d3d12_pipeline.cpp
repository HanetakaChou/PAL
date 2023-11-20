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

brx_d3d12_graphics_pipeline::brx_d3d12_graphics_pipeline() : m_pipeline_state(NULL)
{
}

void brx_d3d12_graphics_pipeline::init(ID3D12Device *device, brx_render_pass const *wrapped_render_pass, brx_pipeline_layout const *wrapped_pipeline_layout, size_t vertex_shader_module_code_size, void const *vertex_shader_module_code, size_t fragment_shader_module_code_size, void const *fragment_shader_module_code, uint32_t vertex_binding_count, BRX_GRAPHICS_PIPELINE_VERTEX_BINDING const *vertex_bindings, uint32_t vertex_attribute_count, BRX_GRAPHICS_PIPELINE_VERTEX_ATTRIBUTE const *vertex_attributes, bool depth_enable, BRX_GRAPHICS_PIPELINE_COMPARE_OPERATION wrapped_depth_compare_operation)
{
	assert(0U == this->m_vertex_buffer_strides.size());
	this->m_vertex_buffer_strides.resize(vertex_binding_count);
	for (uint32_t vertex_binding_index = 0U; vertex_binding_index < vertex_binding_count; ++vertex_binding_index)
	{
		this->m_vertex_buffer_strides[vertex_binding_index] = vertex_bindings[vertex_binding_index].stride;
	}

	D3D12_PRIMITIVE_TOPOLOGY_TYPE const new_primitive_topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	this->m_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	assert(NULL != wrapped_render_pass);
	assert(NULL != wrapped_pipeline_layout);
	uint32_t const color_attachment_count = static_cast<brx_d3d12_render_pass const *>(wrapped_render_pass)->get_color_attachment_count();
	BRX_COLOR_ATTACHMENT_IMAGE_FORMAT const *const color_attachment_formats = static_cast<brx_d3d12_render_pass const *>(wrapped_render_pass)->get_color_attachment_formats();
	BRX_DEPTH_STENCIL_ATTACHMENT_IMAGE_FORMAT const *const depth_stencil_attachment_format = static_cast<brx_d3d12_render_pass const *>(wrapped_render_pass)->get_depth_stencil_attachment_format();
	ID3D12RootSignature *const root_signature = static_cast<brx_d3d12_pipeline_layout const *>(wrapped_pipeline_layout)->get_root_signature();

	brx_vector<D3D12_INPUT_ELEMENT_DESC> input_element_descs(static_cast<size_t>(vertex_attribute_count));
	for (uint32_t vertex_attribute_index = 0U; vertex_attribute_index < vertex_attribute_count; ++vertex_attribute_index)
	{
		input_element_descs[vertex_attribute_index].SemanticName = "LOCATION";
		input_element_descs[vertex_attribute_index].SemanticIndex = vertex_attribute_index;
		input_element_descs[vertex_attribute_index].InputSlot = vertex_attributes[vertex_attribute_index].binding;
		input_element_descs[vertex_attribute_index].AlignedByteOffset = vertex_attributes[vertex_attribute_index].offset;
		input_element_descs[vertex_attribute_index].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		input_element_descs[vertex_attribute_index].InstanceDataStepRate = 0U;
		switch (vertex_attributes[vertex_attribute_index].format)
		{
		case BRX_GRAPHICS_PIPELINE_VERTEX_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT:
			input_element_descs[vertex_attribute_index].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case BRX_GRAPHICS_PIPELINE_VERTEX_ATTRIBUTE_FORMAT_R16G16_UNORM:
			input_element_descs[vertex_attribute_index].Format = DXGI_FORMAT_R16G16_UNORM;
			break;
		default:
			assert(false);
			input_element_descs[vertex_attribute_index].Format = DXGI_FORMAT_UNKNOWN;
		}
	}

	D3D12_COMPARISON_FUNC depth_func;
	switch (wrapped_depth_compare_operation)
	{
	case BRX_GRAPHICS_PIPELINE_COMPARE_OPERATION_GREATER:
		depth_func = D3D12_COMPARISON_FUNC_GREATER;
		break;
	case BRX_GRAPHICS_PIPELINE_COMPARE_OPERATION_LESS:
		depth_func = D3D12_COMPARISON_FUNC_LESS;
		break;
	case BRX_GRAPHICS_PIPELINE_COMPARE_OPERATION_ALWAYS:
		depth_func = D3D12_COMPARISON_FUNC_ALWAYS;
		break;
	default:
		assert(false);
		depth_func = static_cast<D3D12_COMPARISON_FUNC>(-1);
	}

	DXGI_FORMAT rtv_formats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
	for (uint32_t color_attachment_index = 0U; color_attachment_index < color_attachment_count; ++color_attachment_index)
	{
		switch (color_attachment_formats[color_attachment_index])
		{
		case BRX_COLOR_ATTACHMENT_FORMAT_B8G8R8A8_UNORM:
			rtv_formats[color_attachment_index] = DXGI_FORMAT_B8G8R8A8_UNORM;
			break;
		case BRX_COLOR_ATTACHMENT_FORMAT_R8G8B8A8_UNORM:
			rtv_formats[color_attachment_index] = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case BRX_COLOR_ATTACHMENT_FORMAT_A2B10G10R10_UNORM_PACK32:
			assert(false);
			rtv_formats[color_attachment_index] = static_cast<DXGI_FORMAT>(-1);
			break;
		case BRX_COLOR_ATTACHMENT_FORMAT_A2R10G10B10_UNORM_PACK32:
			rtv_formats[color_attachment_index] = DXGI_FORMAT_R10G10B10A2_UNORM;
			break;
		case BRX_COLOR_ATTACHMENT_FORMAT_R16G16_UNORM:
			rtv_formats[color_attachment_index] = DXGI_FORMAT_R16G16_UNORM;
			break;
		default:
			assert(false);
			rtv_formats[color_attachment_index] = static_cast<DXGI_FORMAT>(-1);
		}
	}
	for (uint32_t color_attachment_index = color_attachment_count; color_attachment_index < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++color_attachment_index)
	{
		rtv_formats[color_attachment_index] = DXGI_FORMAT_UNKNOWN;
	}

	DXGI_FORMAT dsv_format;
	if (NULL != depth_stencil_attachment_format)
	{
		switch (*depth_stencil_attachment_format)
		{
		case BRX_DEPTH_STENCIL_ATTACHMENT_FORMAT_D32_SFLOAT:
			dsv_format = DXGI_FORMAT_D32_FLOAT;
			break;
		case BRX_DEPTH_STENCIL_ATTACHMENT_FORMAT_D32_SFLOAT_S8_UINT:
			dsv_format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
			break;
		case BRX_DEPTH_STENCIL_ATTACHMENT_FORMAT_D24_UNORM_S8_UINT:
			dsv_format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		default:
			assert(false);
			dsv_format = static_cast<DXGI_FORMAT>(-1);
		}
	}
	else
	{
		dsv_format = DXGI_FORMAT_UNKNOWN;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC const desc = {
		root_signature,
		{vertex_shader_module_code, vertex_shader_module_code_size},
		{fragment_shader_module_code, fragment_shader_module_code_size},
		{NULL, 0U},
		{NULL, 0U},
		{NULL, 0U},
		{NULL, 0U, NULL, 0U, 0U},
		{FALSE, FALSE, {FALSE, FALSE, D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD, D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD, D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_RED | D3D12_COLOR_WRITE_ENABLE_GREEN | D3D12_COLOR_WRITE_ENABLE_BLUE | D3D12_COLOR_WRITE_ENABLE_ALPHA}},
		0XFFFFFFFFU,
		{D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_BACK, TRUE, 0, 0.0F, 0.0F, TRUE, FALSE, FALSE, 0U, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF},
		{depth_enable ? TRUE : FALSE, D3D12_DEPTH_WRITE_MASK_ALL, depth_func, FALSE, D3D12_DEFAULT_STENCIL_READ_MASK, D3D12_DEFAULT_STENCIL_WRITE_MASK, {D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS}, {D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS}},
		{(vertex_attribute_count > 0U) ? &input_element_descs[0] : NULL, vertex_attribute_count},
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
		new_primitive_topology_type,
		color_attachment_count,
		{rtv_formats[0], rtv_formats[1], rtv_formats[2], rtv_formats[3], rtv_formats[4], rtv_formats[5], rtv_formats[6], rtv_formats[7]},
		dsv_format,
		{1U, 0U},
		0U,
		{NULL, 0U},
		D3D12_PIPELINE_STATE_FLAG_NONE};

	assert(NULL == this->m_pipeline_state);
	HRESULT const hr_create_graphics_pipeline_state = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&this->m_pipeline_state));
	assert(SUCCEEDED(hr_create_graphics_pipeline_state));
}

void brx_d3d12_graphics_pipeline::uninit()
{
	assert(NULL != this->m_pipeline_state);

	this->m_pipeline_state->Release();

	this->m_pipeline_state = NULL;
}

brx_d3d12_graphics_pipeline::~brx_d3d12_graphics_pipeline()
{
	assert(NULL == this->m_pipeline_state);
}

uint32_t brx_d3d12_graphics_pipeline::get_vertex_buffer_count() const
{
	return static_cast<uint32_t>(this->m_vertex_buffer_strides.size());
}

uint32_t const *brx_d3d12_graphics_pipeline::get_vertex_buffer_strides() const
{
	uint32_t const *vertex_buffers_stride;

	if (this->m_vertex_buffer_strides.size() > 0U)
	{
		vertex_buffers_stride = &this->m_vertex_buffer_strides[0];
	}
	else
	{
		vertex_buffers_stride = NULL;
	}

	return vertex_buffers_stride;
}

D3D12_PRIMITIVE_TOPOLOGY brx_d3d12_graphics_pipeline::get_primitive_topology() const
{
	return this->m_primitive_topology;
}

ID3D12PipelineState *brx_d3d12_graphics_pipeline::get_pipeline() const
{
	return this->m_pipeline_state;
}

brx_d3d12_compute_pipeline::brx_d3d12_compute_pipeline() : m_pipeline_state(NULL)
{
}

void brx_d3d12_compute_pipeline::init(ID3D12Device *device, brx_pipeline_layout const *wrapped_pipeline_layout, size_t compute_shader_module_code_size, void const *compute_shader_module_code)
{
	assert(NULL != wrapped_pipeline_layout);
	ID3D12RootSignature *const root_signature = static_cast<brx_d3d12_pipeline_layout const *>(wrapped_pipeline_layout)->get_root_signature();

	D3D12_COMPUTE_PIPELINE_STATE_DESC const desc = {
		root_signature,
		{compute_shader_module_code, compute_shader_module_code_size},
		0U,
		{NULL, 0U},
		D3D12_PIPELINE_STATE_FLAG_NONE};

	assert(NULL == this->m_pipeline_state);
	HRESULT const hr_create_compute_pipeline_state = device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&this->m_pipeline_state));
	assert(SUCCEEDED(hr_create_compute_pipeline_state));
}

void brx_d3d12_compute_pipeline::uninit()
{
	assert(NULL != this->m_pipeline_state);

	this->m_pipeline_state->Release();

	this->m_pipeline_state = NULL;
}

brx_d3d12_compute_pipeline::~brx_d3d12_compute_pipeline()
{
	assert(NULL == this->m_pipeline_state);
}

ID3D12PipelineState *brx_d3d12_compute_pipeline::get_pipeline() const
{
	return this->m_pipeline_state;
}
