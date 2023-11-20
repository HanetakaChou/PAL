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
#include "brx_d3d12_descriptor_allocator.h"
#include "brx_format.h"
#include <assert.h>
#include <cstring>
#ifndef NDEBUG
#include <pix.h>
#endif

brx_d3d12_graphics_command_buffer::brx_d3d12_graphics_command_buffer()
    : m_command_allocator(NULL),
      m_command_list(NULL),
      m_descriptor_allocator(NULL),
      m_current_render_pass(NULL),
      m_current_frame_buffer(NULL)
{
}

void brx_d3d12_graphics_command_buffer::init(ID3D12Device *device, bool uma, bool support_ray_tracing, brx_d3d12_descriptor_allocator *descriptor_allocator)
{
    assert(NULL == this->m_command_allocator);
    HRESULT const hr_create_command_allocator = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->m_command_allocator));
    assert(SUCCEEDED(hr_create_command_allocator));

    assert(NULL == this->m_command_list);
    HRESULT const hr_create_command_list = device->CreateCommandList(0U, D3D12_COMMAND_LIST_TYPE_DIRECT, this->m_command_allocator, NULL, IID_PPV_ARGS(&this->m_command_list));
    assert(SUCCEEDED(hr_create_command_list));

    HRESULT const hr_close = this->m_command_list->Close();
    assert(SUCCEEDED(hr_close));

    this->m_uma = uma;

    this->m_support_ray_tracing = support_ray_tracing;

    assert(NULL == this->m_descriptor_allocator);
    this->m_descriptor_allocator = descriptor_allocator;

    assert(NULL == this->m_current_render_pass);

    assert(NULL == this->m_current_frame_buffer);

    assert(0U == this->m_current_vertex_buffer_strides.size());
}

void brx_d3d12_graphics_command_buffer::uninit()
{
    assert(NULL != this->m_command_list);
    this->m_command_list->Release();
    this->m_command_list = NULL;

    assert(NULL != this->m_command_allocator);
    this->m_command_allocator->Release();
    this->m_command_allocator = NULL;
}

brx_d3d12_graphics_command_buffer::~brx_d3d12_graphics_command_buffer()
{
    assert(NULL == this->m_command_allocator);
    assert(NULL == this->m_command_list);
}

ID3D12CommandAllocator *brx_d3d12_graphics_command_buffer::get_command_allocator() const
{
    return this->m_command_allocator;
}

ID3D12GraphicsCommandList4 *brx_d3d12_graphics_command_buffer::get_command_list() const
{
    return this->m_command_list;
}

void brx_d3d12_graphics_command_buffer::begin()
{
    HRESULT hr_reset = this->m_command_list->Reset(this->m_command_allocator, NULL);
    assert(SUCCEEDED(hr_reset));

    this->m_descriptor_allocator->bind_command_list(this->m_command_list);

    this->m_current_vertex_buffer_strides.clear();
}

void brx_d3d12_graphics_command_buffer::acquire_asset_vertex_position_buffer(brx_asset_vertex_position_buffer *wrapped_asset_vertex_position_buffer)
{
    // The D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER is invalid for compute queue and we have to insert this barreir on graphics queue

    assert(NULL != wrapped_asset_vertex_position_buffer);
    ID3D12Resource *const asset_buffer_resource = static_cast<brx_d3d12_asset_vertex_position_buffer *>(wrapped_asset_vertex_position_buffer)->get_resource();

    D3D12_RESOURCE_BARRIER const acquire_barrier = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition = {
            asset_buffer_resource,
            0U,
            D3D12_RESOURCE_STATE_COMMON,
            (!this->m_support_ray_tracing) ? D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER : (D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)}};

    this->m_command_list->ResourceBarrier(1U, &acquire_barrier);
}

void brx_d3d12_graphics_command_buffer::acquire_asset_vertex_varying_buffer(brx_asset_vertex_varying_buffer *wrapped_asset_vertex_varying_buffer)
{
    // The D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER is invalid for compute queue and we have to insert this barreir on graphics queue

    assert(NULL != wrapped_asset_vertex_varying_buffer);
    ID3D12Resource *const asset_buffer_resource = static_cast<brx_d3d12_asset_vertex_varying_buffer *>(wrapped_asset_vertex_varying_buffer)->get_resource();

    D3D12_RESOURCE_BARRIER const acquire_barrier = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition = {
            asset_buffer_resource,
            0U,
            D3D12_RESOURCE_STATE_COMMON,
            (!this->m_support_ray_tracing) ? D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER : (D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)}};

    this->m_command_list->ResourceBarrier(1U, &acquire_barrier);
}

void brx_d3d12_graphics_command_buffer::acquire_asset_index_buffer(brx_asset_index_buffer *wrapped_asset_index_buffer)
{
    // The D3D12_RESOURCE_STATE_INDEX_BUFFER is invalid for compute queue and we have to insert this barreir on graphics queue

    assert(NULL != wrapped_asset_index_buffer);
    ID3D12Resource *const asset_buffer_resource = static_cast<brx_d3d12_asset_index_buffer *>(wrapped_asset_index_buffer)->get_resource();

    D3D12_RESOURCE_BARRIER const acquire_barrier = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition = {
            asset_buffer_resource,
            0U,
            D3D12_RESOURCE_STATE_COMMON,
            (!this->m_support_ray_tracing) ? D3D12_RESOURCE_STATE_INDEX_BUFFER : (D3D12_RESOURCE_STATE_INDEX_BUFFER | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE)}};
    this->m_command_list->ResourceBarrier(1U, &acquire_barrier);
}

void brx_d3d12_graphics_command_buffer::acquire_asset_sampled_image(brx_asset_sampled_image *wrapped_asset_sampled_image, uint32_t dst_mip_level)
{
    // The D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE is invalid for compute queue and we have to insert this barreir on graphics queue

    assert(NULL != wrapped_asset_sampled_image);
    ID3D12Resource *asset_sampled_image_resource = static_cast<brx_d3d12_asset_sampled_image *>(wrapped_asset_sampled_image)->get_resource();

    D3D12_RESOURCE_BARRIER const resource_acquire_barrier = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition = {
            asset_sampled_image_resource,
            dst_mip_level,
            D3D12_RESOURCE_STATE_COMMON,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}};
    this->m_command_list->ResourceBarrier(1U, &resource_acquire_barrier);
}

void brx_d3d12_graphics_command_buffer::acquire_asset_compacted_bottom_level_acceleration_structure(brx_asset_compacted_bottom_level_acceleration_structure *wrapped_asset_compacted_bottom_level_acceleration_structure)
{
    ID3D12Resource *asset_buffer_resource = static_cast<brx_d3d12_asset_compacted_bottom_level_acceleration_structure *>(wrapped_asset_compacted_bottom_level_acceleration_structure)->get_resource();

    // https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#synchronizing-acceleration-structure-memory-writesreads
    D3D12_RESOURCE_BARRIER const acquire_barrier = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .UAV = {
            asset_buffer_resource}};
    this->m_command_list->ResourceBarrier(1U, &acquire_barrier);
}

void brx_d3d12_graphics_command_buffer::begin_debug_utils_label(char const *label_name)
{
#ifndef NDEBUG
    PIXBeginEvent(this->m_command_list, 0, label_name);
#endif
}

void brx_d3d12_graphics_command_buffer::end_debug_utils_label()
{
#ifndef NDEBUG
    PIXEndEvent(this->m_command_list);
#endif
}

void brx_d3d12_graphics_command_buffer::begin_render_pass(brx_render_pass const *brx_render_pass, brx_frame_buffer const *brx_frame_buffer, uint32_t width, uint32_t height, uint32_t color_clear_value_count, float const (*color_clear_values)[4], float const *depth_clear_value, uint8_t const *stencil_clear_value)
{
    assert(NULL != brx_render_pass);
    assert(NULL != brx_frame_buffer);
    assert(NULL == this->m_current_render_pass);
    assert(NULL == this->m_current_frame_buffer);
    this->m_current_render_pass = static_cast<brx_d3d12_render_pass const *>(brx_render_pass);
    this->m_current_frame_buffer = static_cast<brx_d3d12_frame_buffer const *>(brx_frame_buffer);

    uint32_t const num_render_targets = this->m_current_frame_buffer->get_num_render_targets();
    ID3D12Resource *const *render_target_resources = this->m_current_frame_buffer->get_render_target_view_resources();
    D3D12_CPU_DESCRIPTOR_HANDLE const *render_target_view_descriptors = this->m_current_frame_buffer->get_render_target_view_descriptors();
    ID3D12Resource *const depth_stencil_resource = this->m_current_frame_buffer->get_depth_stencil_resource();
    D3D12_CPU_DESCRIPTOR_HANDLE const *depth_stencil_view_descriptor = this->m_current_frame_buffer->get_depth_stencil_view_descriptor();
    assert(this->m_current_render_pass->get_color_attachment_count() == num_render_targets);

    uint32_t const color_attachments_clear_count = this->m_current_render_pass->get_color_attachments_clear_count();
    uint32_t const *color_attachment_clear_indices = this->m_current_render_pass->get_color_attachment_clear_indices();
    uint32_t const color_attachment_flush_for_sampled_image_count = this->m_current_render_pass->get_color_attachment_flush_for_sampled_image_count();
    uint32_t const *color_attachment_flush_for_sampled_image_indices = this->m_current_render_pass->get_color_attachment_flush_for_sampled_image_indices();
    uint32_t const color_attachment_flush_for_present_count = this->m_current_render_pass->get_color_attachment_flush_for_present_count();
    uint32_t const *color_attachment_flush_for_present_indices = this->m_current_render_pass->get_color_attachment_flush_for_present_indices();
    bool const depth_stencil_attachment_clear = this->m_current_render_pass->get_depth_stencil_attachment_clear();
    bool const depth_stencil_attachment_flush_for_sampled_image = this->m_current_render_pass->get_depth_stencil_attachment_flush_for_sampled_image();
    assert(color_attachments_clear_count <= num_render_targets);
    assert(color_attachment_flush_for_sampled_image_count <= num_render_targets);
    assert(color_attachment_flush_for_present_count <= num_render_targets);
    assert((!depth_stencil_attachment_clear) || (NULL != depth_stencil_resource));
    assert((!depth_stencil_attachment_clear) || (NULL != depth_stencil_view_descriptor));
    assert((!depth_stencil_attachment_flush_for_sampled_image) || (NULL != depth_stencil_resource));
    assert((!depth_stencil_attachment_flush_for_sampled_image) || (NULL != depth_stencil_view_descriptor));
    assert(color_attachments_clear_count == color_clear_value_count);
    assert((!depth_stencil_attachment_clear) || ((NULL != depth_clear_value) || (NULL != stencil_clear_value)));
    assert(((NULL == depth_clear_value) && (NULL == stencil_clear_value)) || depth_stencil_attachment_clear);

#if 1
    // According to the specification, the resource should be in the "D3D12_RESOURCE_STATE_RENDER_TARGET" or "D3D12_RESOURCE_STATE_DEPTH_WRITE" state before clear.
    // But the resource barrier is intrinsically the synchronization of the data between different types of the caches in GPU.
    // Since the the content will be cleared later, no synchronization is required to preserve the content.
    // And there is no such "VK_IMAGE_LAYOUT_UNDEFINED" state in D3D12.

    // TODO: Enhanced Barriers
    // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html
    // D3D12_BARRIER_LAYOUT_UNDEFINED

    for (uint32_t flush_for_sampled_image_index = 0U; flush_for_sampled_image_index < color_attachment_flush_for_sampled_image_count; ++flush_for_sampled_image_index)
    {
        uint32_t const color_attachment_flush_for_sampled_image_index = color_attachment_flush_for_sampled_image_indices[flush_for_sampled_image_index];
        ID3D12Resource *const render_target_resource = render_target_resources[color_attachment_flush_for_sampled_image_index];

        D3D12_RESOURCE_BARRIER const load_barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                render_target_resource,
                0U,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_RENDER_TARGET}};
        this->m_command_list->ResourceBarrier(1U, &load_barrier);
    }

    for (uint32_t flush_for_present_index = 0U; flush_for_present_index < color_attachment_flush_for_present_count; ++flush_for_present_index)
    {
        uint32_t const color_attachment_flush_for_present_index = color_attachment_flush_for_present_indices[flush_for_present_index];
        ID3D12Resource *const render_target_resource = render_target_resources[color_attachment_flush_for_present_index];

        D3D12_RESOURCE_BARRIER const load_barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                render_target_resource,
                0U,
                D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_RENDER_TARGET}};
        this->m_command_list->ResourceBarrier(1U, &load_barrier);
    }

    if (depth_stencil_attachment_flush_for_sampled_image)
    {
        D3D12_RESOURCE_BARRIER const load_barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                depth_stencil_resource,
                0U,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_DEPTH_WRITE}};
        this->m_command_list->ResourceBarrier(1U, &load_barrier);
    }
#endif

    for (uint32_t clear_index = 0U; clear_index < color_clear_value_count; ++clear_index)
    {
        uint32_t const color_attachment_clear_index = color_attachment_clear_indices[clear_index];

        FLOAT const(&color_rgba)[4] = color_clear_values[clear_index];

        this->m_command_list->ClearRenderTargetView(render_target_view_descriptors[color_attachment_clear_index], color_rgba, 0U, NULL);
    }

    if ((NULL != depth_clear_value) || (NULL != stencil_clear_value))
    {
        D3D12_CLEAR_FLAGS clear_flags;
        FLOAT depth;
        UINT8 stencil;
        if ((NULL != depth_clear_value) && (NULL != stencil_clear_value))
        {
            clear_flags = D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;
            depth = (*depth_clear_value);
            stencil = (*stencil_clear_value);
        }
        else if (NULL != depth_clear_value)
        {
            clear_flags = D3D12_CLEAR_FLAG_DEPTH;
            depth = (*depth_clear_value);
            stencil = 0U;
        }
        else if (NULL != stencil_clear_value)
        {
            clear_flags = D3D12_CLEAR_FLAG_STENCIL;
            depth = 0.0F;
            stencil = (*stencil_clear_value);
        }
        else
        {
            assert(false);
            clear_flags = static_cast<D3D12_CLEAR_FLAGS>(-1);
            depth = 0.0F;
            stencil = 0U;
        }

        this->m_command_list->ClearDepthStencilView(*depth_stencil_view_descriptor, clear_flags, depth, stencil, 0U, NULL);
    }

    this->m_command_list->OMSetRenderTargets(num_render_targets, render_target_view_descriptors, FALSE, depth_stencil_view_descriptor);
}

void brx_d3d12_graphics_command_buffer::bind_graphics_pipeline(brx_graphics_pipeline const *wrapped_graphics_pipeline)
{
    assert(NULL != wrapped_graphics_pipeline);
    brx_d3d12_graphics_pipeline const *const unwrapped_graphics_pipeline = static_cast<brx_d3d12_graphics_pipeline const *>(wrapped_graphics_pipeline);

    uint32_t vertex_buffer_count = unwrapped_graphics_pipeline->get_vertex_buffer_count();
    uint32_t const *vertex_buffer_strides = unwrapped_graphics_pipeline->get_vertex_buffer_strides();
    D3D12_PRIMITIVE_TOPOLOGY const primitive_topology = unwrapped_graphics_pipeline->get_primitive_topology();
    ID3D12PipelineState *graphics_pipeline = unwrapped_graphics_pipeline->get_pipeline();

    this->m_current_vertex_buffer_strides.resize(vertex_buffer_count);
    for (uint32_t vertex_buffer_index = 0U; vertex_buffer_index < vertex_buffer_count; ++vertex_buffer_index)
    {
        this->m_current_vertex_buffer_strides[vertex_buffer_index] = vertex_buffer_strides[vertex_buffer_index];
    }

    this->m_command_list->IASetPrimitiveTopology(primitive_topology);

    this->m_command_list->SetPipelineState(graphics_pipeline);
}

void brx_d3d12_graphics_command_buffer::set_view_port(uint32_t width, uint32_t height)
{
    D3D12_VIEWPORT view_port = {0.0F, 0.0F, static_cast<float>(width), static_cast<float>(height), 0.0F, 1.0F};
    this->m_command_list->RSSetViewports(1U, &view_port);
}

void brx_d3d12_graphics_command_buffer::set_scissor(uint32_t width, uint32_t height)
{
    D3D12_RECT rect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    this->m_command_list->RSSetScissorRects(1U, &rect);
}

void brx_d3d12_graphics_command_buffer::bind_graphics_descriptor_sets(brx_pipeline_layout const *wrapped_pipeline_layout, uint32_t descriptor_set_count, brx_descriptor_set const *const *wrapped_descriptor_sets, uint32_t dynamic_offet_count, uint32_t const *dynamic_offsets)
{
    assert(NULL != wrapped_pipeline_layout);
    assert(NULL != wrapped_descriptor_sets);
    ID3D12RootSignature *root_signature = static_cast<brx_d3d12_pipeline_layout const *>(wrapped_pipeline_layout)->get_root_signature();

    this->m_command_list->SetGraphicsRootSignature(root_signature);

    uint32_t root_parameter_index = 0U;
    uint32_t dynamic_offet_index = 0U;
    for (uint32_t set_index = 0U; set_index < descriptor_set_count; ++set_index)
    {
        assert(NULL != wrapped_descriptor_sets[set_index]);
        uint32_t const binding_count = static_cast<brx_d3d12_descriptor_set const *>(wrapped_descriptor_sets[set_index])->get_descriptor_count();
        brx_d3d12_descriptor const *descriptors = static_cast<brx_d3d12_descriptor_set const *>(wrapped_descriptor_sets[set_index])->get_descriptors();
        for (uint32_t binding_index = 0U; binding_index < binding_count; ++binding_index)
        {
            switch (descriptors[binding_index].root_parameter_type)
            {
            case BRX_DESCRIPTOR_TYPE_DYNAMIC_UNIFORM_BUFFER:
            {
                D3D12_GPU_VIRTUAL_ADDRESS const buffer_location = {descriptors[binding_index].root_constant_buffer_view.address_base + dynamic_offsets[dynamic_offet_index]};
                this->m_command_list->SetGraphicsRootConstantBufferView(root_parameter_index, buffer_location);
                ++root_parameter_index;
                ++dynamic_offet_index;
            }
            break;
            case BRX_DESCRIPTOR_TYPE_READ_ONLY_STORAGE_BUFFER:
            {
                D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor = this->m_descriptor_allocator->get_cbv_srv_uav_gpu_descriptor_handle(descriptors[binding_index].root_descriptor_table.base_descriptor_heap_index);
                this->m_command_list->SetGraphicsRootDescriptorTable(root_parameter_index, base_descriptor);
                ++root_parameter_index;
            }
            break;
            case BRX_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            {
                D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor = this->m_descriptor_allocator->get_cbv_srv_uav_gpu_descriptor_handle(descriptors[binding_index].root_descriptor_table.base_descriptor_heap_index);
                this->m_command_list->SetGraphicsRootDescriptorTable(root_parameter_index, base_descriptor);
                ++root_parameter_index;
            }
            break;
            case BRX_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            {
                D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor = this->m_descriptor_allocator->get_cbv_srv_uav_gpu_descriptor_handle(descriptors[binding_index].root_descriptor_table.base_descriptor_heap_index);
                this->m_command_list->SetGraphicsRootDescriptorTable(root_parameter_index, base_descriptor);
                ++root_parameter_index;
            }
            break;
            case BRX_DESCRIPTOR_TYPE_SAMPLER:
            {
                D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor = this->m_descriptor_allocator->get_sampler_gpu_descriptor_handle(descriptors[binding_index].root_descriptor_table.base_descriptor_heap_index);
                this->m_command_list->SetGraphicsRootDescriptorTable(root_parameter_index, base_descriptor);
                ++root_parameter_index;
            }
            break;
            case BRX_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            {
                D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor = this->m_descriptor_allocator->get_cbv_srv_uav_gpu_descriptor_handle(descriptors[binding_index].root_descriptor_table.base_descriptor_heap_index);
                this->m_command_list->SetGraphicsRootDescriptorTable(root_parameter_index, base_descriptor);
                ++root_parameter_index;
            }
            break;
            case BRX_DESCRIPTOR_TYPE_TOP_LEVEL_ACCELERATION_STRUCTURE:
            {
                D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor = this->m_descriptor_allocator->get_cbv_srv_uav_gpu_descriptor_handle(descriptors[binding_index].root_descriptor_table.base_descriptor_heap_index);
                this->m_command_list->SetGraphicsRootDescriptorTable(root_parameter_index, base_descriptor);
                ++root_parameter_index;
            }
            break;
            default:
            {
                assert(false);
            }
            }
        }
    }
    assert(dynamic_offet_index == dynamic_offet_count);
}

void brx_d3d12_graphics_command_buffer::bind_vertex_buffers(uint32_t vertex_buffer_count, brx_vertex_buffer const *const *vertex_buffers)
{
    assert(this->m_current_vertex_buffer_strides.size() == vertex_buffer_count);

    brx_vector<D3D12_VERTEX_BUFFER_VIEW> vertex_buffer_views;
    {
        vertex_buffer_views.resize(vertex_buffer_count);
        for (uint32_t vertex_buffer_index = 0U; vertex_buffer_index < vertex_buffer_count; ++vertex_buffer_index)
        {
            assert(NULL != vertex_buffers[vertex_buffer_index]);
            ID3D12Resource *const resource = static_cast<brx_d3d12_vertex_buffer const *>(vertex_buffers[vertex_buffer_index])->get_resource();
            D3D12_RESOURCE_DESC const resource_desc = resource->GetDesc();

            vertex_buffer_views[vertex_buffer_index].BufferLocation = resource->GetGPUVirtualAddress();
            vertex_buffer_views[vertex_buffer_index].SizeInBytes = static_cast<UINT>(resource_desc.Width);
            vertex_buffer_views[vertex_buffer_index].StrideInBytes = this->m_current_vertex_buffer_strides[vertex_buffer_index];
        }
    }

    this->m_command_list->IASetVertexBuffers(0U, vertex_buffer_count, &vertex_buffer_views[0]);
}

void brx_d3d12_graphics_command_buffer::draw(uint32_t vertex_count, uint32_t instance_count)
{
    this->m_command_list->DrawInstanced(vertex_count, instance_count, 0U, 0U);
}

void brx_d3d12_graphics_command_buffer::draw_index(brx_index_buffer const *wrapped_index_buffer, BRX_GRAPHICS_PIPELINE_INDEX_TYPE wrapped_index_type, uint32_t index_count, uint32_t instance_count)
{
    assert(NULL != wrapped_index_buffer);
    ID3D12Resource *const resource = static_cast<brx_d3d12_index_buffer const *>(wrapped_index_buffer)->get_resource();
    D3D12_RESOURCE_DESC const resource_desc = resource->GetDesc();

    DXGI_FORMAT index_format;
    switch (wrapped_index_type)
    {
    case BRX_GRAPHICS_PIPELINE_INDEX_TYPE_UINT32:
        index_format = DXGI_FORMAT_R32_UINT;
        break;
    case BRX_GRAPHICS_PIPELINE_INDEX_TYPE_UINT16:
        index_format = DXGI_FORMAT_R16_UINT;
        break;
    default:
        assert(false);
        index_format = static_cast<DXGI_FORMAT>(-1);
    }

    D3D12_INDEX_BUFFER_VIEW const index_buffer_view = {
        resource->GetGPUVirtualAddress(),
        static_cast<UINT>(resource_desc.Width),
        index_format};
    this->m_command_list->IASetIndexBuffer(&index_buffer_view);

    this->m_command_list->DrawIndexedInstanced(index_count, instance_count, 0U, 0, 0U);
}

void brx_d3d12_graphics_command_buffer::end_render_pass()
{
    assert(NULL != this->m_current_render_pass);
    assert(NULL != this->m_current_frame_buffer);

    uint32_t const num_render_targets = this->m_current_frame_buffer->get_num_render_targets();
    ID3D12Resource *const *render_target_resources = this->m_current_frame_buffer->get_render_target_view_resources();
    D3D12_CPU_DESCRIPTOR_HANDLE const *render_target_view_descriptors = this->m_current_frame_buffer->get_render_target_view_descriptors();
    ID3D12Resource *const depth_stencil_resource = this->m_current_frame_buffer->get_depth_stencil_resource();
    D3D12_CPU_DESCRIPTOR_HANDLE const *depth_stencil_view_descriptor = this->m_current_frame_buffer->get_depth_stencil_view_descriptor();
    assert(this->m_current_render_pass->get_color_attachment_count() == num_render_targets);

    uint32_t const color_attachment_flush_for_sampled_image_count = this->m_current_render_pass->get_color_attachment_flush_for_sampled_image_count();
    uint32_t const *color_attachment_flush_for_sampled_image_indices = this->m_current_render_pass->get_color_attachment_flush_for_sampled_image_indices();
    uint32_t const color_attachment_flush_for_present_count = this->m_current_render_pass->get_color_attachment_flush_for_present_count();
    uint32_t const *color_attachment_flush_for_present_indices = this->m_current_render_pass->get_color_attachment_flush_for_present_indices();
    bool const depth_stencil_attachment_flush_for_sampled_image = this->m_current_render_pass->get_depth_stencil_attachment_flush_for_sampled_image();
    assert(color_attachment_flush_for_sampled_image_count <= num_render_targets);
    assert(color_attachment_flush_for_present_count <= num_render_targets);
    assert((!depth_stencil_attachment_flush_for_sampled_image) || (NULL != depth_stencil_resource));
    assert((!depth_stencil_attachment_flush_for_sampled_image) || (NULL != depth_stencil_view_descriptor));

    for (uint32_t flush_for_sampled_image_index = 0U; flush_for_sampled_image_index < color_attachment_flush_for_sampled_image_count; ++flush_for_sampled_image_index)
    {
        uint32_t const color_attachment_flush_for_sampled_image_index = color_attachment_flush_for_sampled_image_indices[flush_for_sampled_image_index];
        ID3D12Resource *const render_target_resource = render_target_resources[color_attachment_flush_for_sampled_image_index];

        D3D12_RESOURCE_BARRIER flush_for_sampled_image_resource_barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                render_target_resource,
                0U,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}};
        this->m_command_list->ResourceBarrier(1U, &flush_for_sampled_image_resource_barrier);
    }

    for (uint32_t flush_for_present_index = 0U; flush_for_present_index < color_attachment_flush_for_present_count; ++flush_for_present_index)
    {
        uint32_t const color_attachment_flush_for_present_index = color_attachment_flush_for_present_indices[flush_for_present_index];
        ID3D12Resource *const render_target_resource = render_target_resources[color_attachment_flush_for_present_index];

        D3D12_RESOURCE_BARRIER flush_for_present_resource_barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                render_target_resource,
                0U,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_PRESENT}};
        this->m_command_list->ResourceBarrier(1U, &flush_for_present_resource_barrier);
    }

    if (depth_stencil_attachment_flush_for_sampled_image)
    {
        D3D12_RESOURCE_BARRIER flush_for_sampled_image_resource_barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                depth_stencil_resource,
                0U,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}};
        this->m_command_list->ResourceBarrier(1U, &flush_for_sampled_image_resource_barrier);
    }

    this->m_current_render_pass = NULL;
    this->m_current_frame_buffer = NULL;
}

void brx_d3d12_graphics_command_buffer::compute_pass_load_storage_image(brx_storage_image const *wrapped_storage_image, BRX_STORAGE_IMAGE_LOAD_OPERATION load_operation)
{
    ID3D12Resource *const load_resource = static_cast<brx_d3d12_storage_image const *>(wrapped_storage_image)->get_resource();

    // TODO: Enhanced Barriers
    // https://microsoft.github.io/DirectX-Specs/d3d/D3D12EnhancedBarriers.html
    // D3D12_BARRIER_LAYOUT_UNDEFINED

    D3D12_RESOURCE_BARRIER const load_barrier = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition = {
            load_resource,
            0U,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS}};
    this->m_command_list->ResourceBarrier(1U, &load_barrier);
}

void brx_d3d12_graphics_command_buffer::bind_compute_pipeline(brx_compute_pipeline const *wrapped_compute_pipeline)
{
    assert(NULL != wrapped_compute_pipeline);
    brx_d3d12_compute_pipeline const *const unwrapped_compute_pipeline = static_cast<brx_d3d12_compute_pipeline const *>(wrapped_compute_pipeline);

    ID3D12PipelineState *compute_pipeline = unwrapped_compute_pipeline->get_pipeline();

    this->m_command_list->SetPipelineState(compute_pipeline);
}

void brx_d3d12_graphics_command_buffer::bind_compute_descriptor_sets(brx_pipeline_layout const *wrapped_pipeline_layout, uint32_t descriptor_set_count, brx_descriptor_set const *const *wrapped_descriptor_sets, uint32_t dynamic_offet_count, uint32_t const *dynamic_offsets)
{
    assert(NULL != wrapped_pipeline_layout);
    assert(NULL != wrapped_descriptor_sets);
    ID3D12RootSignature *root_signature = static_cast<brx_d3d12_pipeline_layout const *>(wrapped_pipeline_layout)->get_root_signature();

    this->m_command_list->SetComputeRootSignature(root_signature);

    uint32_t root_parameter_index = 0U;
    uint32_t dynamic_offet_index = 0U;
    for (uint32_t set_index = 0U; set_index < descriptor_set_count; ++set_index)
    {
        assert(NULL != wrapped_descriptor_sets[set_index]);
        uint32_t const binding_count = static_cast<brx_d3d12_descriptor_set const *>(wrapped_descriptor_sets[set_index])->get_descriptor_count();
        brx_d3d12_descriptor const *descriptors = static_cast<brx_d3d12_descriptor_set const *>(wrapped_descriptor_sets[set_index])->get_descriptors();
        for (uint32_t binding_index = 0U; binding_index < binding_count; ++binding_index)
        {
            switch (descriptors[binding_index].root_parameter_type)
            {
            case BRX_DESCRIPTOR_TYPE_DYNAMIC_UNIFORM_BUFFER:
            {
                D3D12_GPU_VIRTUAL_ADDRESS const buffer_location = {descriptors[binding_index].root_constant_buffer_view.address_base + dynamic_offsets[dynamic_offet_index]};
                this->m_command_list->SetComputeRootConstantBufferView(root_parameter_index, buffer_location);
                ++root_parameter_index;
                ++dynamic_offet_index;
            }
            break;
            case BRX_DESCRIPTOR_TYPE_READ_ONLY_STORAGE_BUFFER:
            {
                D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor = this->m_descriptor_allocator->get_cbv_srv_uav_gpu_descriptor_handle(descriptors[binding_index].root_descriptor_table.base_descriptor_heap_index);
                this->m_command_list->SetComputeRootDescriptorTable(root_parameter_index, base_descriptor);
                ++root_parameter_index;
            }
            break;
            case BRX_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            {
                D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor = this->m_descriptor_allocator->get_cbv_srv_uav_gpu_descriptor_handle(descriptors[binding_index].root_descriptor_table.base_descriptor_heap_index);
                this->m_command_list->SetComputeRootDescriptorTable(root_parameter_index, base_descriptor);
                ++root_parameter_index;
            }
            break;
            case BRX_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            {
                D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor = this->m_descriptor_allocator->get_cbv_srv_uav_gpu_descriptor_handle(descriptors[binding_index].root_descriptor_table.base_descriptor_heap_index);
                this->m_command_list->SetComputeRootDescriptorTable(root_parameter_index, base_descriptor);
                ++root_parameter_index;
            }
            break;
            case BRX_DESCRIPTOR_TYPE_SAMPLER:
            {
                D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor = this->m_descriptor_allocator->get_sampler_gpu_descriptor_handle(descriptors[binding_index].root_descriptor_table.base_descriptor_heap_index);
                this->m_command_list->SetComputeRootDescriptorTable(root_parameter_index, base_descriptor);
                ++root_parameter_index;
            }
            break;
            case BRX_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            {
                D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor = this->m_descriptor_allocator->get_cbv_srv_uav_gpu_descriptor_handle(descriptors[binding_index].root_descriptor_table.base_descriptor_heap_index);
                this->m_command_list->SetComputeRootDescriptorTable(root_parameter_index, base_descriptor);
                ++root_parameter_index;
            }
            break;
            case BRX_DESCRIPTOR_TYPE_TOP_LEVEL_ACCELERATION_STRUCTURE:
            {
                D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor = this->m_descriptor_allocator->get_cbv_srv_uav_gpu_descriptor_handle(descriptors[binding_index].root_descriptor_table.base_descriptor_heap_index);
                this->m_command_list->SetComputeRootDescriptorTable(root_parameter_index, base_descriptor);
                ++root_parameter_index;
            }
            break;
            default:
            {
                assert(false);
            }
            }
        }
    }
    assert(dynamic_offet_index == dynamic_offet_count);
}

void brx_d3d12_graphics_command_buffer::dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z)
{
    this->m_command_list->Dispatch(group_count_x, group_count_y, group_count_z);
}

void brx_d3d12_graphics_command_buffer::compute_pass_store_storage_image(brx_storage_image const *wrapped_storage_image, BRX_STORAGE_IMAGE_STORE_OPERATION store_operation)
{
    ID3D12Resource *const store_resource = static_cast<brx_d3d12_storage_image const *>(wrapped_storage_image)->get_resource();

    D3D12_RESOURCE_BARRIER const store_barrier = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition = {
            store_resource,
            0U,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}};
    this->m_command_list->ResourceBarrier(1U, &store_barrier);
}

void brx_d3d12_graphics_command_buffer::build_top_level_acceleration_structure(brx_top_level_acceleration_structure *wrapped_top_level_acceleration_structure, uint32_t top_level_acceleration_structure_instance_count, brx_top_level_acceleration_structure_instance_upload_buffer *wrapped_top_level_acceleration_structure_instance_upload_buffer, brx_scratch_buffer *wrapped_scratch_buffer)
{
    assert(NULL != wrapped_top_level_acceleration_structure);
    ID3D12Resource *const destination_acceleration_structure_buffer_resource = static_cast<brx_d3d12_top_level_acceleration_structure *>(wrapped_top_level_acceleration_structure)->get_resource();
    D3D12_GPU_VIRTUAL_ADDRESS const destination_acceleration_structure_device_memory_range_base = destination_acceleration_structure_buffer_resource->GetGPUVirtualAddress();
    assert(0U == (destination_acceleration_structure_device_memory_range_base % D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT));

    assert(NULL != wrapped_top_level_acceleration_structure_instance_upload_buffer);
    D3D12_GPU_VIRTUAL_ADDRESS const top_level_acceleration_structure_instance_upload_buffer_device_memory_range_base = static_cast<brx_d3d12_top_level_acceleration_structure_instance_upload_buffer *>(wrapped_top_level_acceleration_structure_instance_upload_buffer)->get_resource()->GetGPUVirtualAddress();
    assert(0U == (top_level_acceleration_structure_instance_upload_buffer_device_memory_range_base % D3D12_RAYTRACING_INSTANCE_DESCS_BYTE_ALIGNMENT));

    assert(NULL != wrapped_scratch_buffer);
    D3D12_GPU_VIRTUAL_ADDRESS const scratch_buffer_device_memory_range_base = static_cast<brx_d3d12_scratch_buffer *>(wrapped_scratch_buffer)->get_resource()->GetGPUVirtualAddress();
    assert(0U == (scratch_buffer_device_memory_range_base % D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT));

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC const ray_tracing_acceleration_structure_desc = {
        destination_acceleration_structure_device_memory_range_base,
        {D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
         D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD,
         static_cast<UINT>(top_level_acceleration_structure_instance_count),
         D3D12_ELEMENTS_LAYOUT_ARRAY,
         {.InstanceDescs = top_level_acceleration_structure_instance_upload_buffer_device_memory_range_base}},
        0U,
        scratch_buffer_device_memory_range_base};

    this->m_command_list->BuildRaytracingAccelerationStructure(&ray_tracing_acceleration_structure_desc, 0U, NULL);

    static_cast<brx_d3d12_top_level_acceleration_structure *>(wrapped_top_level_acceleration_structure)->set_instance_count(top_level_acceleration_structure_instance_count);

    // https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#synchronizing-acceleration-structure-memory-writesreads
    D3D12_RESOURCE_BARRIER const release_barrier = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .UAV = {
            destination_acceleration_structure_buffer_resource}};
    this->m_command_list->ResourceBarrier(1U, &release_barrier);
}

void brx_d3d12_graphics_command_buffer::update_top_level_acceleration_structure(brx_top_level_acceleration_structure *wrapped_top_level_acceleration_structure, brx_top_level_acceleration_structure_instance_upload_buffer *wrapped_top_level_acceleration_structure_instance_upload_buffer, brx_scratch_buffer *wrapped_scratch_buffer)
{

    assert(NULL != wrapped_top_level_acceleration_structure);
    ID3D12Resource *const destination_acceleration_structure_buffer_resource = static_cast<brx_d3d12_top_level_acceleration_structure *>(wrapped_top_level_acceleration_structure)->get_resource();
    D3D12_GPU_VIRTUAL_ADDRESS const destination_acceleration_structure_device_memory_range_base = destination_acceleration_structure_buffer_resource->GetGPUVirtualAddress();
    assert(0U == (destination_acceleration_structure_device_memory_range_base % D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT));
    // [Acceleration structure update constraints](https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#acceleration-structure-update-constraints)
    uint32_t const top_level_acceleration_structure_instance_count = static_cast<brx_d3d12_top_level_acceleration_structure *>(wrapped_top_level_acceleration_structure)->get_instance_count();

    assert(NULL != wrapped_top_level_acceleration_structure_instance_upload_buffer);
    D3D12_GPU_VIRTUAL_ADDRESS const top_level_acceleration_structure_instance_upload_buffer_device_memory_range_base = static_cast<brx_d3d12_top_level_acceleration_structure_instance_upload_buffer *>(wrapped_top_level_acceleration_structure_instance_upload_buffer)->get_resource()->GetGPUVirtualAddress();
    assert(0U == (top_level_acceleration_structure_instance_upload_buffer_device_memory_range_base % D3D12_RAYTRACING_INSTANCE_DESCS_BYTE_ALIGNMENT));

    assert(NULL != wrapped_scratch_buffer);
    D3D12_GPU_VIRTUAL_ADDRESS const scratch_buffer_device_memory_range_base = static_cast<brx_d3d12_scratch_buffer *>(wrapped_scratch_buffer)->get_resource()->GetGPUVirtualAddress();
    assert(0U == (scratch_buffer_device_memory_range_base % D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT));

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC const ray_tracing_acceleration_structure_desc = {
        destination_acceleration_structure_device_memory_range_base,
        {D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL,
         D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD,
         static_cast<UINT>(top_level_acceleration_structure_instance_count),
         D3D12_ELEMENTS_LAYOUT_ARRAY,
         {.InstanceDescs = top_level_acceleration_structure_instance_upload_buffer_device_memory_range_base}},
        destination_acceleration_structure_device_memory_range_base,
        scratch_buffer_device_memory_range_base};

    this->m_command_list->BuildRaytracingAccelerationStructure(&ray_tracing_acceleration_structure_desc, 0U, NULL);
}

void brx_d3d12_graphics_command_buffer::acceleration_structure_pass_store_top_level(brx_top_level_acceleration_structure *wrapped_top_level_acceleration_structure)
{
    assert(NULL != wrapped_top_level_acceleration_structure);
    ID3D12Resource *const destination_acceleration_structure_buffer_resource = static_cast<brx_d3d12_top_level_acceleration_structure *>(wrapped_top_level_acceleration_structure)->get_resource();

    // https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#synchronizing-acceleration-structure-memory-writesreads
    D3D12_RESOURCE_BARRIER const release_barrier = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .UAV = {
            destination_acceleration_structure_buffer_resource}};
    this->m_command_list->ResourceBarrier(1U, &release_barrier);
}

void brx_d3d12_graphics_command_buffer::end()
{
    HRESULT const hr_close = this->m_command_list->Close();
    assert(SUCCEEDED(hr_close));
}

brx_d3d12_upload_command_buffer::brx_d3d12_upload_command_buffer() : m_command_allocator(NULL), m_command_list(NULL), m_upload_queue_submit_fence(NULL)
{
}

void brx_d3d12_upload_command_buffer::init(ID3D12Device *device, bool uma, bool support_ray_tracing)
{
    this->m_uma = uma;

    this->m_support_ray_tracing = support_ray_tracing;

    if ((!this->m_uma) || this->m_support_ray_tracing)
    {
        assert(NULL == this->m_command_allocator);
        HRESULT const hr_create_command_allocator = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&this->m_command_allocator));
        assert(SUCCEEDED(hr_create_command_allocator));

        assert(NULL == this->m_command_list);
        HRESULT const hr_create_command_list = device->CreateCommandList(0U, D3D12_COMMAND_LIST_TYPE_COMPUTE, this->m_command_allocator, NULL, IID_PPV_ARGS(&this->m_command_list));
        assert(SUCCEEDED(hr_create_command_list));

        HRESULT const hr_close = this->m_command_list->Close();
        assert(SUCCEEDED(hr_close));

        assert(NULL == this->m_upload_queue_submit_fence);
        HRESULT const hr_create_fence = device->CreateFence(0U, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->m_upload_queue_submit_fence));
        assert(SUCCEEDED(hr_create_fence));
    }
    else
    {
        assert(NULL == this->m_command_allocator);
        assert(NULL == this->m_command_list);
        assert(NULL == this->m_upload_queue_submit_fence);
    }
}

void brx_d3d12_upload_command_buffer::uninit()
{
    if ((!this->m_uma) || this->m_support_ray_tracing)
    {
        assert(NULL != this->m_upload_queue_submit_fence);
        this->m_upload_queue_submit_fence->Release();
        this->m_upload_queue_submit_fence = NULL;

        assert(NULL != this->m_command_list);
        this->m_command_list->Release();
        this->m_command_list = NULL;

        assert(NULL != this->m_command_allocator);
        this->m_command_allocator->Release();
        this->m_command_allocator = NULL;
    }
    else
    {
        assert(NULL == this->m_command_allocator);
        assert(NULL == this->m_command_list);
        assert(NULL == this->m_upload_queue_submit_fence);
    }
}

brx_d3d12_upload_command_buffer::~brx_d3d12_upload_command_buffer()
{
    assert(NULL == this->m_command_allocator);
    assert(NULL == this->m_command_list);
    assert(NULL == this->m_upload_queue_submit_fence);
}

ID3D12CommandAllocator *brx_d3d12_upload_command_buffer::get_command_allocator() const
{
    return this->m_command_allocator;
}

ID3D12GraphicsCommandList4 *brx_d3d12_upload_command_buffer::get_command_list() const
{
    return this->m_command_list;
}

ID3D12Fence *brx_d3d12_upload_command_buffer::get_upload_queue_submit_fence() const
{
    return this->m_upload_queue_submit_fence;
}

void brx_d3d12_upload_command_buffer::begin()
{
    if ((!this->m_uma) || this->m_support_ray_tracing)
    {
        HRESULT hr_reset = this->m_command_list->Reset(this->m_command_allocator, NULL);
        assert(SUCCEEDED(hr_reset));
    }
    else
    {
        assert(NULL == this->m_command_allocator);
        assert(NULL == this->m_command_list);
    }
}

void brx_d3d12_upload_command_buffer::upload_from_staging_upload_buffer_to_asset_vertex_position_buffer(brx_asset_vertex_position_buffer *wrapped_asset_vertex_position_buffer, uint64_t dst_offset, brx_staging_upload_buffer *wrapped_staging_upload_buffer, uint64_t src_offset, uint32_t src_size)
{
    ID3D12Resource *asset_buffer_resource = static_cast<brx_d3d12_asset_vertex_position_buffer *>(wrapped_asset_vertex_position_buffer)->get_resource();
    ID3D12Resource *staging_upload_buffer_resource = static_cast<brx_d3d12_staging_upload_buffer *>(wrapped_staging_upload_buffer)->get_resource();

    if (!this->m_uma)
    {
        this->m_command_list->CopyBufferRegion(asset_buffer_resource, dst_offset, staging_upload_buffer_resource, src_offset, src_size);

        D3D12_RESOURCE_BARRIER const store_barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                asset_buffer_resource,
                0U,
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_COMMON}};
        this->m_command_list->ResourceBarrier(1U, &store_barrier);
    }
    else
    {
        assert(this->m_support_ray_tracing || NULL == this->m_command_allocator);
        assert(this->m_support_ray_tracing || NULL == this->m_command_list);

        void *asset_buffer_memory_range_base = NULL;
        {
            D3D12_RANGE const read_range = {0U, 0U};
            HRESULT const hr_map = asset_buffer_resource->Map(0U, &read_range, &asset_buffer_memory_range_base);
            assert(SUCCEEDED(hr_map));
        }

        void *staging_upload_buffer_memory_range_base = static_cast<brx_d3d12_staging_upload_buffer *>(wrapped_staging_upload_buffer)->get_host_memory_range_base();

        std::memcpy(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(asset_buffer_memory_range_base) + dst_offset), reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(staging_upload_buffer_memory_range_base) + src_offset), src_size);

        {
            D3D12_RANGE const written_range = {static_cast<SIZE_T>(dst_offset), static_cast<SIZE_T>(src_size)};
            asset_buffer_resource->Unmap(0U, &written_range);
        }
    }
}

void brx_d3d12_upload_command_buffer::upload_from_staging_upload_buffer_to_asset_vertex_varying_buffer(brx_asset_vertex_varying_buffer *wrapped_asset_vertex_varying_buffer, uint64_t dst_offset, brx_staging_upload_buffer *wrapped_staging_upload_buffer, uint64_t src_offset, uint32_t src_size)
{
    ID3D12Resource *asset_buffer_resource = static_cast<brx_d3d12_asset_vertex_varying_buffer *>(wrapped_asset_vertex_varying_buffer)->get_resource();
    ID3D12Resource *staging_upload_buffer_resource = static_cast<brx_d3d12_staging_upload_buffer *>(wrapped_staging_upload_buffer)->get_resource();

    if (!this->m_uma)
    {
        this->m_command_list->CopyBufferRegion(asset_buffer_resource, dst_offset, staging_upload_buffer_resource, src_offset, src_size);

        assert(NULL != wrapped_asset_vertex_varying_buffer);
        ID3D12Resource *const asset_buffer_resource = static_cast<brx_d3d12_asset_vertex_varying_buffer *>(wrapped_asset_vertex_varying_buffer)->get_resource();

        D3D12_RESOURCE_BARRIER const store_barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                asset_buffer_resource,
                0U,
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_COMMON}};
        this->m_command_list->ResourceBarrier(1U, &store_barrier);
    }
    else
    {
        assert(this->m_support_ray_tracing || NULL == this->m_command_allocator);
        assert(this->m_support_ray_tracing || NULL == this->m_command_list);

        void *asset_buffer_memory_range_base = NULL;
        {
            D3D12_RANGE const read_range = {0U, 0U};
            HRESULT const hr_map = asset_buffer_resource->Map(0U, &read_range, &asset_buffer_memory_range_base);
            assert(SUCCEEDED(hr_map));
        }

        void *staging_upload_buffer_memory_range_base = static_cast<brx_d3d12_staging_upload_buffer *>(wrapped_staging_upload_buffer)->get_host_memory_range_base();

        std::memcpy(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(asset_buffer_memory_range_base) + dst_offset), reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(staging_upload_buffer_memory_range_base) + src_offset), src_size);

        {
            D3D12_RANGE const written_range = {static_cast<SIZE_T>(dst_offset), static_cast<SIZE_T>(src_size)};
            asset_buffer_resource->Unmap(0U, &written_range);
        }
    }
}

void brx_d3d12_upload_command_buffer::upload_from_staging_upload_buffer_to_asset_index_buffer(brx_asset_index_buffer *wrapped_asset_index_buffer, uint64_t dst_offset, brx_staging_upload_buffer *wrapped_staging_upload_buffer, uint64_t src_offset, uint32_t src_size)
{
    ID3D12Resource *asset_buffer_resource = static_cast<brx_d3d12_asset_index_buffer *>(wrapped_asset_index_buffer)->get_resource();
    ID3D12Resource *staging_upload_buffer_resource = static_cast<brx_d3d12_staging_upload_buffer *>(wrapped_staging_upload_buffer)->get_resource();

    if (!this->m_uma)
    {
        this->m_command_list->CopyBufferRegion(asset_buffer_resource, dst_offset, staging_upload_buffer_resource, src_offset, src_size);

        D3D12_RESOURCE_BARRIER const store_barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                asset_buffer_resource,
                0U,
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_COMMON}};
        this->m_command_list->ResourceBarrier(1U, &store_barrier);
    }
    else
    {
        assert(this->m_support_ray_tracing || NULL == this->m_command_allocator);
        assert(this->m_support_ray_tracing || NULL == this->m_command_list);

        void *asset_buffer_memory_range_base = NULL;
        {
            D3D12_RANGE const read_range = {0U, 0U};
            HRESULT const hr_map = asset_buffer_resource->Map(0U, &read_range, &asset_buffer_memory_range_base);
            assert(SUCCEEDED(hr_map));
        }

        void *staging_upload_buffer_memory_range_base = static_cast<brx_d3d12_staging_upload_buffer *>(wrapped_staging_upload_buffer)->get_host_memory_range_base();

        std::memcpy(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(asset_buffer_memory_range_base) + dst_offset), reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(staging_upload_buffer_memory_range_base) + src_offset), src_size);

        {
            D3D12_RANGE const written_range = {static_cast<SIZE_T>(dst_offset), static_cast<SIZE_T>(src_size)};
            asset_buffer_resource->Unmap(0U, &written_range);
        }
    }
}

void brx_d3d12_upload_command_buffer::upload_from_staging_upload_buffer_to_asset_sampled_image(brx_asset_sampled_image *wrapped_asset_sampled_image, BRX_ASSET_IMAGE_FORMAT wrapped_asset_sampled_image_format, uint32_t asset_sampled_image_width, uint32_t asset_sampled_image_height, uint32_t dst_mip_level, brx_staging_upload_buffer *wrapped_staging_upload_buffer, uint64_t src_offset, uint32_t src_row_pitch, uint32_t src_row_count)
{
    assert(NULL != wrapped_asset_sampled_image);
    ID3D12Resource *const asset_sampled_image = static_cast<brx_d3d12_asset_sampled_image *>(wrapped_asset_sampled_image)->get_resource();

    assert(NULL != wrapped_staging_upload_buffer);
    ID3D12Resource *const staging_upload_buffer = static_cast<brx_d3d12_staging_upload_buffer *>(wrapped_staging_upload_buffer)->get_resource();

    DXGI_FORMAT asset_sampled_image_format;
    switch (wrapped_asset_sampled_image_format)
    {
    case BRX_ASSET_IMAGE_FORMAT_R8G8B8A8_UNORM:
        asset_sampled_image_format = DXGI_FORMAT_R8G8B8A8_UNORM;
        break;
    case BRX_ASSET_IMAGE_FORMAT_BC7_UNORM_BLOCK:
        asset_sampled_image_format = DXGI_FORMAT_BC7_UNORM;
        break;
    default:
        assert(false);
        asset_sampled_image_format = DXGI_FORMAT_UNKNOWN;
    }

    uint32_t const block_width = brx_get_format_block_width(wrapped_asset_sampled_image_format);
    uint32_t const block_height = brx_get_format_block_height(wrapped_asset_sampled_image_format);

    uint32_t const width = (((asset_sampled_image_width >> dst_mip_level) + (block_width - 1U)) / block_width) * block_width;
    uint32_t const height = (((asset_sampled_image_height >> dst_mip_level) + (block_height - 1U)) / block_height) * block_height;

#ifndef NDEBUG
    {
        ID3D12Device *device = NULL;
        HRESULT const hr_get_device = asset_sampled_image->GetDevice(IID_PPV_ARGS(&device));
        assert(SUCCEEDED(hr_get_device));

        D3D12_RESOURCE_DESC const asset_sampled_image_resource_desc = asset_sampled_image->GetDesc();

        // The resulting structures are GPU adapter-agnostic, meaning that the values will not vary from one GPU adapter to the next.
        // This means that we can implement this function by ourselves according to the specification.
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts[1];
        UINT num_rows[1];
        device->GetCopyableFootprints(&asset_sampled_image_resource_desc, dst_mip_level, 1U, src_offset, layouts, num_rows, NULL, NULL);

        device->Release();

        assert(layouts[0].Offset == src_offset);
        assert(layouts[0].Footprint.Format == asset_sampled_image_format);
        assert(layouts[0].Footprint.Width == width);
        assert(layouts[0].Footprint.Height == height);
        assert(layouts[0].Footprint.Depth == 1U);
        assert(layouts[0].Footprint.RowPitch == src_row_pitch);
        assert(num_rows[0] == src_row_count);
    }
#endif

    if (!this->m_uma)
    {
        {
            D3D12_TEXTURE_COPY_LOCATION const destination = {
                .pResource = asset_sampled_image,
                .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
                .SubresourceIndex = dst_mip_level};

            D3D12_TEXTURE_COPY_LOCATION const source = {
                .pResource = staging_upload_buffer,
                .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
                .PlacedFootprint = {
                    src_offset,
                    {asset_sampled_image_format,
                     static_cast<UINT>(width),
                     height,
                     1U,
                     src_row_pitch}}};

            this->m_command_list->CopyTextureRegion(&destination, 0U, 0U, 0U, &source, NULL);
        }

        D3D12_RESOURCE_BARRIER const store_barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                asset_sampled_image,
                dst_mip_level,
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_COMMON}};
        this->m_command_list->ResourceBarrier(1U, &store_barrier);
    }
    else
    {
        assert(this->m_support_ray_tracing || NULL == this->m_command_allocator);
        assert(this->m_support_ray_tracing || NULL == this->m_command_list);

        {
            D3D12_RANGE const read_range = {0U, 0U};
            HRESULT const hr_map = asset_sampled_image->Map(0U, &read_range, NULL);
            assert(SUCCEEDED(hr_map));
        }

        void *staging_upload_buffer_memory_range_base = static_cast<brx_d3d12_staging_upload_buffer *>(wrapped_staging_upload_buffer)->get_host_memory_range_base();

        // the tiling mode is vendor specific
        // for example,  the AMD addrlib [ac_surface_addr_from_coord](https://gitlab.freedesktop.org/mesa/mesa/-/blob/22.3/src/amd/vulkan/radv_meta_bufimage.c#L1372)
        asset_sampled_image->WriteToSubresource(dst_mip_level, NULL, reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(staging_upload_buffer_memory_range_base) + src_offset), src_row_pitch, src_row_pitch * src_row_count);

        asset_sampled_image->Unmap(0U, NULL);
    }
}

void brx_d3d12_upload_command_buffer::build_staging_non_compacted_bottom_level_acceleration_structure(brx_staging_non_compacted_bottom_level_acceleration_structure *wrapped_staging_non_compacted_bottom_level_acceleration_structure, uint32_t bottom_level_acceleration_structure_geometry_count, BRX_BOTTOM_LEVEL_ACCELERATION_STRUCTURE_GEOMETRY const *wrapped_bottom_level_acceleration_structure_geometries, brx_scratch_buffer *wrapped_scratch_buffer, brx_compacted_bottom_level_acceleration_structure_size_query_pool *wrapped_compacted_bottom_level_acceleration_structure_size_query_pool, uint32_t query_index)
{
    assert(NULL != wrapped_staging_non_compacted_bottom_level_acceleration_structure);
    D3D12_GPU_VIRTUAL_ADDRESS const destination_acceleration_structure_device_memory_range_base = static_cast<brx_d3d12_staging_non_compacted_bottom_level_acceleration_structure *>(wrapped_staging_non_compacted_bottom_level_acceleration_structure)->get_resource()->GetGPUVirtualAddress();
    assert(0U == (destination_acceleration_structure_device_memory_range_base % D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT));

    assert(NULL != wrapped_bottom_level_acceleration_structure_geometries);
    brx_vector<D3D12_RESOURCE_BARRIER> vertex_position_buffer_load_barriers;
    brx_vector<D3D12_RESOURCE_BARRIER> vertex_position_buffer_store_barriers;
    brx_vector<D3D12_RESOURCE_BARRIER> index_buffer_load_barriers;
    brx_vector<D3D12_RESOURCE_BARRIER> index_buffer_store_barriers;
    brx_vector<D3D12_RAYTRACING_GEOMETRY_DESC> ray_tracing_geometry_descs;
    vertex_position_buffer_load_barriers.reserve(bottom_level_acceleration_structure_geometry_count);
    vertex_position_buffer_store_barriers.reserve(bottom_level_acceleration_structure_geometry_count);
    index_buffer_load_barriers.reserve(bottom_level_acceleration_structure_geometry_count);
    index_buffer_store_barriers.reserve(bottom_level_acceleration_structure_geometry_count);
    ray_tracing_geometry_descs.reserve(bottom_level_acceleration_structure_geometry_count);
    for (uint32_t bottom_level_acceleration_structure_geometry_index = 0U; bottom_level_acceleration_structure_geometry_index < bottom_level_acceleration_structure_geometry_count; ++bottom_level_acceleration_structure_geometry_index)
    {
        BRX_BOTTOM_LEVEL_ACCELERATION_STRUCTURE_GEOMETRY const &wrapped_bottom_level_acceleration_structure_geometry = wrapped_bottom_level_acceleration_structure_geometries[bottom_level_acceleration_structure_geometry_index];

        ID3D12Resource *const unwrapped_vertex_position_buffer_resource = static_cast<brx_d3d12_vertex_position_buffer const *>(wrapped_bottom_level_acceleration_structure_geometry.vertex_position_buffer)->get_resource();

        D3D12_RESOURCE_BARRIER const vertex_position_buffer_load_barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                unwrapped_vertex_position_buffer_resource,
                0U,
                D3D12_RESOURCE_STATE_COMMON,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE}};
        vertex_position_buffer_load_barriers.push_back(vertex_position_buffer_load_barrier);

        D3D12_RESOURCE_BARRIER const vertex_position_buffer_store_barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                unwrapped_vertex_position_buffer_resource,
                0U,
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                D3D12_RESOURCE_STATE_COMMON}};
        vertex_position_buffer_store_barriers.push_back(vertex_position_buffer_store_barrier);

        ID3D12Resource *const unwrapped_index_buffer_resource = (BRX_GRAPHICS_PIPELINE_INDEX_TYPE_NONE != wrapped_bottom_level_acceleration_structure_geometry.index_type) ? static_cast<brx_d3d12_index_buffer const *>(wrapped_bottom_level_acceleration_structure_geometry.index_buffer)->get_resource() : NULL;

        if (BRX_GRAPHICS_PIPELINE_INDEX_TYPE_NONE != wrapped_bottom_level_acceleration_structure_geometry.index_type)
        {
            D3D12_RESOURCE_BARRIER const index_buffer_load_barrier = {
                .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
                .Transition = {
                    unwrapped_index_buffer_resource,
                    0U,
                    D3D12_RESOURCE_STATE_COMMON,
                    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE}};
            index_buffer_load_barriers.push_back(index_buffer_load_barrier);

            D3D12_RESOURCE_BARRIER const index_buffer_store_barrier = {
                .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
                .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
                .Transition = {
                    unwrapped_index_buffer_resource,
                    0U,
                    D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                    D3D12_RESOURCE_STATE_COMMON}};
            index_buffer_store_barriers.push_back(index_buffer_store_barrier);
        }

        DXGI_FORMAT vertex_position_attribute_format;
        switch (wrapped_bottom_level_acceleration_structure_geometry.vertex_position_attribute_format)
        {
        case BRX_GRAPHICS_PIPELINE_VERTEX_ATTRIBUTE_FORMAT_R32G32B32_SFLOAT:
            vertex_position_attribute_format = DXGI_FORMAT_R32G32B32_FLOAT;
            break;
        default:
            // VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR
            assert(false);
            vertex_position_attribute_format = static_cast<DXGI_FORMAT>(-1);
            break;
        }

        D3D12_GPU_VIRTUAL_ADDRESS const vertex_position_buffer_device_memory_range_base = unwrapped_vertex_position_buffer_resource->GetGPUVirtualAddress();

        DXGI_FORMAT index_format;
        switch (wrapped_bottom_level_acceleration_structure_geometry.index_type)
        {
        case BRX_GRAPHICS_PIPELINE_INDEX_TYPE_UINT32:
            index_format = DXGI_FORMAT_R32_UINT;
            break;
        case BRX_GRAPHICS_PIPELINE_INDEX_TYPE_UINT16:
            index_format = DXGI_FORMAT_R16_UINT;
            break;
        case BRX_GRAPHICS_PIPELINE_INDEX_TYPE_NONE:
            index_format = DXGI_FORMAT_UNKNOWN;
            break;
        default:
            assert(false);
            index_format = static_cast<DXGI_FORMAT>(-1);
        }

        D3D12_GPU_VIRTUAL_ADDRESS const index_buffer_device_memory_range_base = (BRX_GRAPHICS_PIPELINE_INDEX_TYPE_NONE != wrapped_bottom_level_acceleration_structure_geometry.index_type) ? unwrapped_index_buffer_resource->GetGPUVirtualAddress() : NULL;

        D3D12_RAYTRACING_GEOMETRY_DESC const ray_tracing_geometry_geometry_desc = {
            D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES,
            wrapped_bottom_level_acceleration_structure_geometry.force_closest_hit ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE,
            {.Triangles = {
                 0U,
                 index_format,
                 vertex_position_attribute_format,
                 (BRX_GRAPHICS_PIPELINE_INDEX_TYPE_NONE != wrapped_bottom_level_acceleration_structure_geometry.index_type) ? wrapped_bottom_level_acceleration_structure_geometry.index_count : 0U,
                 wrapped_bottom_level_acceleration_structure_geometry.vertex_count,
                 index_buffer_device_memory_range_base,
                 {vertex_position_buffer_device_memory_range_base, wrapped_bottom_level_acceleration_structure_geometry.vertex_position_binding_stride}

             }}};

        ray_tracing_geometry_descs.push_back(ray_tracing_geometry_geometry_desc);
    }
    assert(bottom_level_acceleration_structure_geometry_count == ray_tracing_geometry_descs.size());

    assert(NULL != wrapped_scratch_buffer);
    D3D12_GPU_VIRTUAL_ADDRESS const scratch_buffer_device_memory_range_base = static_cast<brx_d3d12_scratch_buffer *>(wrapped_scratch_buffer)->get_resource()->GetGPUVirtualAddress();
    assert(0U == (scratch_buffer_device_memory_range_base % D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT));

    assert(NULL != wrapped_compacted_bottom_level_acceleration_structure_size_query_pool);
    D3D12_GPU_VIRTUAL_ADDRESS const query_pool_device_memory_range_base = static_cast<brx_d3d12_compacted_bottom_level_acceleration_structure_size_query_pool *>(wrapped_compacted_bottom_level_acceleration_structure_size_query_pool)->get_resource()->GetGPUVirtualAddress();

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC const ray_tracing_acceleration_structure_desc = {
        destination_acceleration_structure_device_memory_range_base,
        {D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL,
         D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_COMPACTION | D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE,
         static_cast<UINT>(ray_tracing_geometry_descs.size()),
         D3D12_ELEMENTS_LAYOUT_ARRAY,
         {.pGeometryDescs = &ray_tracing_geometry_descs[0]}},
        0U,
        scratch_buffer_device_memory_range_base};

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC const ray_tracing_acceleration_structure_postbuild_info_desc = {
        query_pool_device_memory_range_base + sizeof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC) * query_index,
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE,
    };

    assert(bottom_level_acceleration_structure_geometry_count == vertex_position_buffer_load_barriers.size());
    this->m_command_list->ResourceBarrier(bottom_level_acceleration_structure_geometry_count, &vertex_position_buffer_load_barriers[0]);

    assert(bottom_level_acceleration_structure_geometry_count >= index_buffer_load_barriers.size());
    this->m_command_list->ResourceBarrier(static_cast<UINT>(index_buffer_load_barriers.size()), &index_buffer_load_barriers[0]);

    this->m_command_list->BuildRaytracingAccelerationStructure(&ray_tracing_acceleration_structure_desc, 1U, &ray_tracing_acceleration_structure_postbuild_info_desc);

    assert(bottom_level_acceleration_structure_geometry_count == vertex_position_buffer_store_barriers.size());
    this->m_command_list->ResourceBarrier(bottom_level_acceleration_structure_geometry_count, &vertex_position_buffer_store_barriers[0]);

    assert(bottom_level_acceleration_structure_geometry_count >= index_buffer_store_barriers.size());
    this->m_command_list->ResourceBarrier(static_cast<UINT>(index_buffer_store_barriers.size()), &index_buffer_store_barriers[0]);
}

void brx_d3d12_upload_command_buffer::compact_bottom_level_acceleration_structure(brx_asset_compacted_bottom_level_acceleration_structure *wrapped_destination_asset_compacted_bottom_level_acceleration_structure, brx_staging_non_compacted_bottom_level_acceleration_structure *wrapped_source_staging_non_compacted_bottom_level_acceleration_structure)
{
    // NOTE: we don't need the barrier to wait for the building of the source acceleration structure, since we already use the fence to wait before we get the size of the compacted acceleration structure

    assert(NULL != wrapped_destination_asset_compacted_bottom_level_acceleration_structure);
    assert(NULL != wrapped_source_staging_non_compacted_bottom_level_acceleration_structure);
    ID3D12Resource *const destination_acceleration_structure_buffer_resource = static_cast<brx_d3d12_asset_compacted_bottom_level_acceleration_structure *>(wrapped_destination_asset_compacted_bottom_level_acceleration_structure)->get_resource();
    D3D12_GPU_VIRTUAL_ADDRESS const destination_acceleration_structure_device_memory_range_base = destination_acceleration_structure_buffer_resource->GetGPUVirtualAddress();
    assert(0U == (destination_acceleration_structure_device_memory_range_base % D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT));
    D3D12_GPU_VIRTUAL_ADDRESS const source_acceleration_structure_device_memory_range_base = static_cast<brx_d3d12_staging_non_compacted_bottom_level_acceleration_structure *>(wrapped_source_staging_non_compacted_bottom_level_acceleration_structure)->get_resource()->GetGPUVirtualAddress();
    assert(0U == (source_acceleration_structure_device_memory_range_base % D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT));

    this->m_command_list->CopyRaytracingAccelerationStructure(destination_acceleration_structure_device_memory_range_base, source_acceleration_structure_device_memory_range_base, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_COPY_MODE_COMPACT);

    // https://microsoft.github.io/DirectX-Specs/d3d/Raytracing.html#synchronizing-acceleration-structure-memory-writesreads
    D3D12_RESOURCE_BARRIER const store_barrier = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_UAV,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .UAV = {
            destination_acceleration_structure_buffer_resource}};
    this->m_command_list->ResourceBarrier(1U, &store_barrier);
}

void brx_d3d12_upload_command_buffer::release_asset_vertex_position_buffer(brx_asset_vertex_position_buffer *asset_vertex_position_buffer)
{
    // do nothing
}

void brx_d3d12_upload_command_buffer::release_asset_vertex_varying_buffer(brx_asset_vertex_varying_buffer *asset_vertex_varying_buffer)
{
    // do nothing
}

void brx_d3d12_upload_command_buffer::release_asset_index_buffer(brx_asset_index_buffer *asset_index_buffer)
{
    // do nothing
}

void brx_d3d12_upload_command_buffer::release_asset_sampled_image(brx_asset_sampled_image *asset_sampled_image, uint32_t dst_mip_level)
{
    // do nothing
}

void brx_d3d12_upload_command_buffer::release_asset_compacted_bottom_level_acceleration_structure(brx_asset_compacted_bottom_level_acceleration_structure *asset_compacted_bottom_level_acceleration_structure)
{
    // do nothing
}

void brx_d3d12_upload_command_buffer::end()
{
    if ((!this->m_uma) || this->m_support_ray_tracing)
    {
        HRESULT hr_close = this->m_command_list->Close();
        assert(SUCCEEDED(hr_close));
    }
    else
    {
        assert(NULL == this->m_command_allocator);
        assert(NULL == this->m_command_list);
    }
}
