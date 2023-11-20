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
#include "brx_vector.h"
#include <assert.h>

brx_vk_graphics_pipeline::brx_vk_graphics_pipeline() : m_pipeline(VK_NULL_HANDLE)
{
}

void brx_vk_graphics_pipeline::init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, brx_render_pass const *wrapped_render_pass, brx_pipeline_layout const *wrapped_pipeline_layout, size_t vertex_shader_module_code_size, void const *vertex_shader_module_code, size_t fragment_shader_module_code_size, void const *fragment_shader_module_code, uint32_t vertex_binding_count, BRX_GRAPHICS_PIPELINE_VERTEX_BINDING const *vertex_bindings, uint32_t vertex_attribute_count, BRX_GRAPHICS_PIPELINE_VERTEX_ATTRIBUTE const *vertex_attributes, bool depth_enable, BRX_GRAPHICS_PIPELINE_COMPARE_OPERATION wrapped_depth_compare_operation)
{
	PFN_vkCreateShaderModule const pfn_create_shader_module = reinterpret_cast<PFN_vkCreateShaderModule>(pfn_get_device_proc_addr(device, "vkCreateShaderModule"));
	assert(pfn_create_shader_module);
	PFN_vkDestroyShaderModule const pfn_destroy_shader_module = reinterpret_cast<PFN_vkDestroyShaderModule>(pfn_get_device_proc_addr(device, "vkDestroyShaderModule"));
	assert(pfn_destroy_shader_module);
	PFN_vkCreateGraphicsPipelines const pfn_create_graphics_pipelines = reinterpret_cast<PFN_vkCreateGraphicsPipelines>(pfn_get_device_proc_addr(device, "vkCreateGraphicsPipelines"));
	assert(pfn_create_graphics_pipelines);

	// NOTE: single subpass is enough
	// input attachment is NOT necessary
	// use VK_ARM_rasterization_order_attachment_access (VK_EXT_rasterization_order_attachment_access) instead
	constexpr uint32_t const subpass_index = 0U;

	assert(NULL != wrapped_render_pass);
	VkRenderPass render_pass = static_cast<brx_vk_render_pass const *>(wrapped_render_pass)->get_render_pass();

	assert(NULL != wrapped_pipeline_layout);
	VkPipelineLayout pipeline_layout = static_cast<brx_vk_pipeline_layout const *>(wrapped_pipeline_layout)->get_pipeline_layout();

	VkShaderModule vertex_shader_module = VK_NULL_HANDLE;
	{
		VkShaderModuleCreateInfo const shader_module_create_info = {
			VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			NULL,
			0U,
			vertex_shader_module_code_size,
			static_cast<uint32_t const *>(vertex_shader_module_code)};

		VkResult const res_create_shader_module = pfn_create_shader_module(device, &shader_module_create_info, NULL, &vertex_shader_module);
		assert(VK_SUCCESS == res_create_shader_module);
	}

	VkShaderModule fragment_shader_module = VK_NULL_HANDLE;
	{
		VkShaderModuleCreateInfo const shader_module_create_info = {
			VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			NULL,
			0U,
			fragment_shader_module_code_size,
			static_cast<uint32_t const *>(fragment_shader_module_code)};

		VkResult const res_create_shader_module = pfn_create_shader_module(device, &shader_module_create_info, NULL, &fragment_shader_module);
		assert(VK_SUCCESS == res_create_shader_module);
	}

	VkPipelineShaderStageCreateInfo const stages[2] =
		{
			{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			 NULL,
			 0U,
			 VK_SHADER_STAGE_VERTEX_BIT,
			 vertex_shader_module,
			 "main",
			 NULL},
			{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			 NULL,
			 0U,
			 VK_SHADER_STAGE_FRAGMENT_BIT,
			 fragment_shader_module,
			 "main",
			 NULL}};

	brx_vector<VkVertexInputBindingDescription> vertex_binding_descriptions(static_cast<size_t>(vertex_binding_count));
	for (uint32_t vertex_binding_index = 0U; vertex_binding_index < vertex_binding_count; ++vertex_binding_index)
	{
		vertex_binding_descriptions[vertex_binding_index].binding = vertex_binding_index;
		vertex_binding_descriptions[vertex_binding_index].stride = vertex_bindings[vertex_binding_index].stride;
		vertex_binding_descriptions[vertex_binding_index].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}

	brx_vector<VkVertexInputAttributeDescription> vertex_attribute_descriptions(static_cast<size_t>(vertex_attribute_count));
	for (uint32_t vertex_attribute_index = 0U; vertex_attribute_index < vertex_attribute_count; ++vertex_attribute_index)
	{
		vertex_attribute_descriptions[vertex_attribute_index].location = vertex_attribute_index;
		vertex_attribute_descriptions[vertex_attribute_index].binding = vertex_attributes[vertex_attribute_index].binding;
		vertex_attribute_descriptions[vertex_attribute_index].offset = vertex_attributes[vertex_attribute_index].offset;
		switch (vertex_attributes[vertex_attribute_index].format)
		{
		case BRX_GRAPHICS_PIPELINE_VERTEX_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT:
			vertex_attribute_descriptions[vertex_attribute_index].format = VK_FORMAT_R32G32B32_SFLOAT;
			break;
		case BRX_GRAPHICS_PIPELINE_VERTEX_ATTRIBUTE_FORMAT_R16G16_UNORM:
			vertex_attribute_descriptions[vertex_attribute_index].format = VK_FORMAT_R16G16_UNORM;
			break;
		default:
			assert(false);
			vertex_attribute_descriptions[vertex_attribute_index].format = VK_FORMAT_UNDEFINED;
		}
	}

	VkPipelineVertexInputStateCreateInfo const vertex_input_state = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		NULL,
		0U,
		vertex_binding_count,
		(vertex_binding_count > 0U) ? &vertex_binding_descriptions[0] : NULL,
		vertex_attribute_count,
		(vertex_attribute_count > 0U) ? &vertex_attribute_descriptions[0] : NULL};

	VkPipelineInputAssemblyStateCreateInfo const input_assembly_state = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		NULL,
		0U,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_FALSE};

	VkPipelineViewportStateCreateInfo const viewport_state = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		NULL,
		0U,
		1U,
		NULL,
		1U,
		NULL};

	VkPipelineRasterizationStateCreateInfo const rasterization_state = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		NULL,
		0U,
		VK_FALSE,
		VK_FALSE,
		VK_POLYGON_MODE_FILL,
		VK_CULL_MODE_BACK_BIT,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		VK_FALSE,
		0.0F,
		0.0F,
		0.0F,
		1.0F};

	VkPipelineMultisampleStateCreateInfo const multisample_state = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		NULL,
		0U,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FALSE,
		0.0F,
		NULL,
		VK_FALSE,
		VK_FALSE};

	VkCompareOp depth_compare_op;
	switch (wrapped_depth_compare_operation)
	{
	case BRX_GRAPHICS_PIPELINE_COMPARE_OPERATION_GREATER:
		depth_compare_op = VK_COMPARE_OP_GREATER;
		break;
	case BRX_GRAPHICS_PIPELINE_COMPARE_OPERATION_LESS:
		depth_compare_op = VK_COMPARE_OP_LESS;
		break;
	case BRX_GRAPHICS_PIPELINE_COMPARE_OPERATION_ALWAYS:
		depth_compare_op = VK_COMPARE_OP_ALWAYS;
		break;
	default:
		assert(false);
		depth_compare_op = static_cast<VkCompareOp>(-1);
	}

	VkPipelineDepthStencilStateCreateInfo const depth_stencil_state = {
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		NULL,
		0U,
		depth_enable ? VK_TRUE : VK_FALSE,
		VK_TRUE,
		depth_compare_op,
		VK_FALSE,
		VK_FALSE,
		{VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 255, 255, 255},
		{VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 255, 255, 255},
		0.0F,
		1.0F};

	VkPipelineColorBlendAttachmentState const attachments[1] = {
		{VK_FALSE,
		 VK_BLEND_FACTOR_ZERO,
		 VK_BLEND_FACTOR_ZERO,
		 VK_BLEND_OP_ADD,
		 VK_BLEND_FACTOR_ZERO,
		 VK_BLEND_FACTOR_ZERO,
		 VK_BLEND_OP_ADD,
		 VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}};

	VkPipelineColorBlendStateCreateInfo const color_blend_state = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		NULL,
		0U,
		VK_FALSE,
		VK_LOGIC_OP_CLEAR,
		1U,
		attachments,
		{0.0F, 0.0F, 0.0F, 0.0F}};

	VkDynamicState const dynamic_states[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo const dynamic_state = {
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		NULL,
		0U,
		sizeof(dynamic_states) / sizeof(dynamic_states[0]),
		dynamic_states};

	VkGraphicsPipelineCreateInfo const graphics_pipeline_create_info = {
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		NULL,
		0U,
		sizeof(stages) / sizeof(stages[0]),
		stages,
		&vertex_input_state,
		&input_assembly_state,
		NULL,
		&viewport_state,
		&rasterization_state,
		&multisample_state,
		&depth_stencil_state,
		&color_blend_state,
		&dynamic_state,
		pipeline_layout,
		render_pass,
		subpass_index,
		VK_NULL_HANDLE,
		0U};
	assert(VK_NULL_HANDLE == this->m_pipeline);
	VkResult const res_create_graphics_pipelines = pfn_create_graphics_pipelines(device, VK_NULL_HANDLE, 1U, &graphics_pipeline_create_info, allocation_callbacks, &this->m_pipeline);
	assert(VK_SUCCESS == res_create_graphics_pipelines);

	pfn_destroy_shader_module(device, vertex_shader_module, allocation_callbacks);
	pfn_destroy_shader_module(device, fragment_shader_module, allocation_callbacks);
}

void brx_vk_graphics_pipeline::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkDestroyPipeline const pfn_destroy_pipeline = reinterpret_cast<PFN_vkDestroyPipeline>(pfn_get_device_proc_addr(device, "vkDestroyPipeline"));
	assert(NULL != pfn_destroy_pipeline);

	assert(VK_NULL_HANDLE != this->m_pipeline);

	pfn_destroy_pipeline(device, this->m_pipeline, allocation_callbacks);

	this->m_pipeline = VK_NULL_HANDLE;
}

brx_vk_graphics_pipeline::~brx_vk_graphics_pipeline()
{
	assert(VK_NULL_HANDLE == this->m_pipeline);
}

VkPipeline brx_vk_graphics_pipeline::get_pipeline() const
{
	return this->m_pipeline;
}

brx_vk_compute_pipeline::brx_vk_compute_pipeline() : m_pipeline(VK_NULL_HANDLE)
{
}

void brx_vk_compute_pipeline::init(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks, brx_pipeline_layout const *wrapped_pipeline_layout, size_t compute_shader_module_code_size, void const *compute_shader_module_code)
{
	PFN_vkCreateShaderModule const pfn_create_shader_module = reinterpret_cast<PFN_vkCreateShaderModule>(pfn_get_device_proc_addr(device, "vkCreateShaderModule"));
	assert(pfn_create_shader_module);
	PFN_vkDestroyShaderModule const pfn_destroy_shader_module = reinterpret_cast<PFN_vkDestroyShaderModule>(pfn_get_device_proc_addr(device, "vkDestroyShaderModule"));
	assert(pfn_destroy_shader_module);
	PFN_vkCreateComputePipelines const pfn_create_compute_pipelines = reinterpret_cast<PFN_vkCreateComputePipelines>(pfn_get_device_proc_addr(device, "vkCreateComputePipelines"));
	assert(pfn_create_compute_pipelines);

	assert(NULL != wrapped_pipeline_layout);
	VkPipelineLayout pipeline_layout = static_cast<brx_vk_pipeline_layout const *>(wrapped_pipeline_layout)->get_pipeline_layout();

	VkShaderModule compute_shader_module = VK_NULL_HANDLE;
	{
		VkShaderModuleCreateInfo const shader_module_create_info = {
			VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			NULL,
			0U,
			compute_shader_module_code_size,
			static_cast<uint32_t const *>(compute_shader_module_code)};

		VkResult const res_create_shader_module = pfn_create_shader_module(device, &shader_module_create_info, NULL, &compute_shader_module);
		assert(VK_SUCCESS == res_create_shader_module);
	}

	VkComputePipelineCreateInfo const compute_pipeline_create_info = {
		VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		NULL,
		0U,
		{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, NULL, 0U, VK_SHADER_STAGE_COMPUTE_BIT, compute_shader_module, "main", NULL},
		pipeline_layout,
		VK_NULL_HANDLE,
		0U};
	assert(VK_NULL_HANDLE == this->m_pipeline);
	VkResult const res_create_compute_pipelines = pfn_create_compute_pipelines(device, VK_NULL_HANDLE, 1U, &compute_pipeline_create_info, allocation_callbacks, &this->m_pipeline);
	assert(VK_SUCCESS == res_create_compute_pipelines);

	pfn_destroy_shader_module(device, compute_shader_module, allocation_callbacks);
}

void brx_vk_compute_pipeline::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
	PFN_vkDestroyPipeline const pfn_destroy_pipeline = reinterpret_cast<PFN_vkDestroyPipeline>(pfn_get_device_proc_addr(device, "vkDestroyPipeline"));
	assert(NULL != pfn_destroy_pipeline);

	assert(VK_NULL_HANDLE != this->m_pipeline);

	pfn_destroy_pipeline(device, this->m_pipeline, allocation_callbacks);

	this->m_pipeline = VK_NULL_HANDLE;
}

brx_vk_compute_pipeline::~brx_vk_compute_pipeline()
{
	assert(VK_NULL_HANDLE == this->m_pipeline);
}

VkPipeline brx_vk_compute_pipeline::get_pipeline() const
{
	return this->m_pipeline;
}
