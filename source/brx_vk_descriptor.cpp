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

brx_vk_descriptor_set_layout::brx_vk_descriptor_set_layout()
    : m_descriptor_set_layout(VK_NULL_HANDLE),
      m_dynamic_uniform_buffer_descriptor_count(0U),
      m_storage_buffer_descriptor_count(0U),
      m_sampled_image_descriptor_count(0U),
      m_sampler_descriptor_count(0U),
      m_storage_image_descriptor_count(0U),
      m_top_level_acceleration_structure_descriptor_count(0U)
{
}

void brx_vk_descriptor_set_layout::init(uint32_t descriptor_set_binding_count, BRX_DESCRIPTOR_SET_LAYOUT_BINDING const *wrapped_descriptor_set_bindings, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
    PFN_vkCreateDescriptorSetLayout const pfn_create_descriptor_set_layout = reinterpret_cast<PFN_vkCreateDescriptorSetLayout>(pfn_get_device_proc_addr(device, "vkCreateDescriptorSetLayout"));
    assert(NULL != pfn_create_descriptor_set_layout);

    brx_vector<VkDescriptorSetLayoutBinding> descriptor_set_bindings(static_cast<size_t>(descriptor_set_binding_count));
    assert(0U == this->m_dynamic_uniform_buffer_descriptor_count);
    assert(0U == this->m_storage_buffer_descriptor_count);
    assert(0U == this->m_sampled_image_descriptor_count);
    assert(0U == this->m_sampler_descriptor_count);
    assert(0U == this->m_storage_image_descriptor_count);
    assert(0U == this->m_top_level_acceleration_structure_descriptor_count);
    for (uint32_t binding_index = 0U; binding_index < descriptor_set_binding_count; ++binding_index)
    {
        descriptor_set_bindings[binding_index].binding = wrapped_descriptor_set_bindings[binding_index].binding;
        switch (wrapped_descriptor_set_bindings[binding_index].descriptor_type)
        {
        case BRX_DESCRIPTOR_TYPE_DYNAMIC_UNIFORM_BUFFER:
        {
            descriptor_set_bindings[binding_index].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            assert(1U == wrapped_descriptor_set_bindings[binding_index].descriptor_count);
            ++this->m_dynamic_uniform_buffer_descriptor_count;
        }
        break;
        case BRX_DESCRIPTOR_TYPE_READ_ONLY_STORAGE_BUFFER:
        case BRX_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        {
            descriptor_set_bindings[binding_index].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            this->m_storage_buffer_descriptor_count += wrapped_descriptor_set_bindings[binding_index].descriptor_count;
        }
        break;
        case BRX_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        {
            descriptor_set_bindings[binding_index].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            this->m_sampled_image_descriptor_count += wrapped_descriptor_set_bindings[binding_index].descriptor_count;
        }
        break;
        case BRX_DESCRIPTOR_TYPE_SAMPLER:
        {
            descriptor_set_bindings[binding_index].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            this->m_sampler_descriptor_count += wrapped_descriptor_set_bindings[binding_index].descriptor_count;
        }
        break;
        case BRX_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        {
            descriptor_set_bindings[binding_index].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            this->m_storage_image_descriptor_count += wrapped_descriptor_set_bindings[binding_index].descriptor_count;
        }
        break;
        case BRX_DESCRIPTOR_TYPE_TOP_LEVEL_ACCELERATION_STRUCTURE:
        {
            descriptor_set_bindings[binding_index].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
            this->m_top_level_acceleration_structure_descriptor_count += wrapped_descriptor_set_bindings[binding_index].descriptor_count;
        }
        break;
        default:
        {
            assert(false);
            descriptor_set_bindings[binding_index].descriptorType = static_cast<VkDescriptorType>(-1);
        }
        }
        descriptor_set_bindings[binding_index].descriptorCount = wrapped_descriptor_set_bindings[binding_index].descriptor_count;
        // TODO:
        descriptor_set_bindings[binding_index].stageFlags = VK_SHADER_STAGE_ALL;
        descriptor_set_bindings[binding_index].pImmutableSamplers = NULL;
    }

    VkDescriptorSetLayoutCreateInfo const descriptor_set_layout_create_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        NULL,
        0U,
        descriptor_set_binding_count,
        &descriptor_set_bindings[0]};

    assert(VK_NULL_HANDLE == this->m_descriptor_set_layout);
    VkResult const res_create_global_descriptor_set_layout = pfn_create_descriptor_set_layout(device, &descriptor_set_layout_create_info, allocation_callbacks, &this->m_descriptor_set_layout);
    assert(VK_SUCCESS == res_create_global_descriptor_set_layout);
}

void brx_vk_descriptor_set_layout::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
    PFN_vkDestroyDescriptorSetLayout const pfn_destroy_descriptor_set_layout = reinterpret_cast<PFN_vkDestroyDescriptorSetLayout>(pfn_get_device_proc_addr(device, "vkDestroyDescriptorSetLayout"));
    assert(NULL != pfn_destroy_descriptor_set_layout);

    assert(VK_NULL_HANDLE != this->m_descriptor_set_layout);

    pfn_destroy_descriptor_set_layout(device, this->m_descriptor_set_layout, allocation_callbacks);

    this->m_descriptor_set_layout = VK_NULL_HANDLE;
}

brx_vk_descriptor_set_layout::~brx_vk_descriptor_set_layout()
{
    assert(VK_NULL_HANDLE == this->m_descriptor_set_layout);
}

VkDescriptorSetLayout brx_vk_descriptor_set_layout::get_descriptor_set_layout() const
{
    return this->m_descriptor_set_layout;
}

uint32_t brx_vk_descriptor_set_layout::get_dynamic_uniform_buffer_descriptor_count() const
{
    return this->m_dynamic_uniform_buffer_descriptor_count;
}

uint32_t brx_vk_descriptor_set_layout::get_storage_buffer_descriptor_count() const
{
    return this->m_storage_buffer_descriptor_count;
}

uint32_t brx_vk_descriptor_set_layout::get_sampled_image_descriptor_count() const
{
    return this->m_sampled_image_descriptor_count;
}

uint32_t brx_vk_descriptor_set_layout::get_sampler_descriptor_count() const
{
    return this->m_sampler_descriptor_count;
}

uint32_t brx_vk_descriptor_set_layout::get_storage_image_descriptor_count() const
{
    return this->m_storage_image_descriptor_count;
}

uint32_t brx_vk_descriptor_set_layout::get_top_level_acceleration_structure_descriptor_count() const
{
    return this->m_top_level_acceleration_structure_descriptor_count;
}

brx_vk_pipeline_layout::brx_vk_pipeline_layout(VkPipelineLayout pipeline_layout) : m_pipeline_layout(pipeline_layout)
{
}

VkPipelineLayout brx_vk_pipeline_layout::get_pipeline_layout() const
{
    return this->m_pipeline_layout;
}

void brx_vk_pipeline_layout::steal(VkPipelineLayout *out_pipeline_layout)
{
    assert(NULL != out_pipeline_layout);

    (*out_pipeline_layout) = this->m_pipeline_layout;

    this->m_pipeline_layout = VK_NULL_HANDLE;
}

brx_vk_pipeline_layout::~brx_vk_pipeline_layout()
{
    assert(VK_NULL_HANDLE == this->m_pipeline_layout);
}

brx_vk_descriptor_set::brx_vk_descriptor_set() : m_descriptor_pool(VK_NULL_HANDLE), m_descriptor_set(VK_NULL_HANDLE)
{
}

void brx_vk_descriptor_set::init(brx_descriptor_set_layout const *wrapped_descriptor_set_layout, PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
    // According to SRT(Shader Resource Table) in PS5, "descriptor set" is essentially a block of GPU-readable memory.

    // In Vulkan, there is no limit for the descriptor pool, and we can use one descriptor pool for each descriptor set.

    assert(NULL != wrapped_descriptor_set_layout);
    brx_vk_descriptor_set_layout const *unwrapped_descriptor_set_layout = static_cast<brx_vk_descriptor_set_layout const *>(wrapped_descriptor_set_layout);
    VkDescriptorSetLayout const descriptor_set_layout = unwrapped_descriptor_set_layout->get_descriptor_set_layout();
    uint32_t const dynamic_uniform_buffer_descriptor_count = unwrapped_descriptor_set_layout->get_dynamic_uniform_buffer_descriptor_count();
    uint32_t const storage_buffer_descriptor_count = unwrapped_descriptor_set_layout->get_storage_buffer_descriptor_count();
    uint32_t const sampled_image_descriptor_count = unwrapped_descriptor_set_layout->get_sampled_image_descriptor_count();
    uint32_t const sampler_descriptor_count = unwrapped_descriptor_set_layout->get_sampler_descriptor_count();
    uint32_t const storage_image_descriptor_count = unwrapped_descriptor_set_layout->get_storage_image_descriptor_count();
    uint32_t const top_level_acceleration_structure_descriptor_count = unwrapped_descriptor_set_layout->get_top_level_acceleration_structure_descriptor_count();

    PFN_vkCreateDescriptorPool const pfn_create_descriptor_pool = reinterpret_cast<PFN_vkCreateDescriptorPool>(pfn_get_device_proc_addr(device, "vkCreateDescriptorPool"));
    assert(NULL != pfn_create_descriptor_pool);

    brx_vector<VkDescriptorPoolSize> descriptor_pool_sizes;
    if (0U < dynamic_uniform_buffer_descriptor_count)
    {
        descriptor_pool_sizes.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, dynamic_uniform_buffer_descriptor_count});
    }
    if (0U < storage_buffer_descriptor_count)
    {
        descriptor_pool_sizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, storage_buffer_descriptor_count});
    }
    if (0U < sampled_image_descriptor_count)
    {
        descriptor_pool_sizes.push_back({VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, sampled_image_descriptor_count});
    }
    if (0U < sampler_descriptor_count)
    {
        descriptor_pool_sizes.push_back({VK_DESCRIPTOR_TYPE_SAMPLER, sampler_descriptor_count});
    }
    if (0U < storage_image_descriptor_count)
    {
        descriptor_pool_sizes.push_back({VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, storage_image_descriptor_count});
    }
    if (0U < top_level_acceleration_structure_descriptor_count)
    {
        descriptor_pool_sizes.push_back({VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, top_level_acceleration_structure_descriptor_count});
    }

    VkDescriptorPoolCreateInfo const descriptor_pool_create_info = {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        NULL,
        0U,
        1U,
        static_cast<uint32_t>(descriptor_pool_sizes.size()),
        &descriptor_pool_sizes[0]};

    assert(VK_NULL_HANDLE == this->m_descriptor_pool);
    VkResult const res_create_descriptor_pool = pfn_create_descriptor_pool(device, &descriptor_pool_create_info, allocation_callbacks, &this->m_descriptor_pool);
    assert(VK_SUCCESS == res_create_descriptor_pool);

    PFN_vkAllocateDescriptorSets const pfn_allocate_descriptor_sets = reinterpret_cast<PFN_vkAllocateDescriptorSets>(pfn_get_device_proc_addr(device, "vkAllocateDescriptorSets"));
    assert(NULL != pfn_create_descriptor_pool);

    VkDescriptorSetLayout const descriptor_set_layouts[1] = {descriptor_set_layout};

    VkDescriptorSetAllocateInfo const descriptor_set_allocate_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, NULL, this->m_descriptor_pool, sizeof(descriptor_set_layouts) / sizeof(descriptor_set_layouts[0]), descriptor_set_layouts};

    assert(VK_NULL_HANDLE == this->m_descriptor_set);
    VkResult const res_allocate_descriptor_sets = pfn_allocate_descriptor_sets(device, &descriptor_set_allocate_info, &this->m_descriptor_set);
    assert(VK_SUCCESS == res_allocate_descriptor_sets);
}

void brx_vk_descriptor_set::uninit(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, VkAllocationCallbacks const *allocation_callbacks)
{
    assert(VK_NULL_HANDLE != this->m_descriptor_set);
    this->m_descriptor_set = VK_NULL_HANDLE;

    assert(VK_NULL_HANDLE != this->m_descriptor_pool);

    PFN_vkDestroyDescriptorPool const pfn_destroy_descriptor_pool = reinterpret_cast<PFN_vkDestroyDescriptorPool>(pfn_get_device_proc_addr(device, "vkDestroyDescriptorPool"));
    assert(NULL != pfn_destroy_descriptor_pool);

    pfn_destroy_descriptor_pool(device, this->m_descriptor_pool, allocation_callbacks);

    this->m_descriptor_pool = VK_NULL_HANDLE;
}

brx_vk_descriptor_set::~brx_vk_descriptor_set()
{
    assert(VK_NULL_HANDLE == this->m_descriptor_set);
}

void brx_vk_descriptor_set::write_descriptor(PFN_vkGetDeviceProcAddr pfn_get_device_proc_addr, VkDevice device, BRX_DESCRIPTOR_TYPE wrapped_descriptor_type, uint32_t dst_binding, uint32_t dst_array_element, uint32_t src_descriptor_count, brx_uniform_upload_buffer const *const *src_dynamic_uniform_buffers, uint32_t const *src_dynamic_uniform_buffer_ranges, brx_storage_buffer const *const *src_storage_buffers, brx_sampled_image const *const *src_sampled_images, brx_sampler const *const *src_samplers, brx_storage_image const *const *src_storage_images, brx_top_level_acceleration_structure const *const *src_top_level_acceleration_structures)
{
    PFN_vkUpdateDescriptorSets const pfn_update_descriptor_sets = reinterpret_cast<PFN_vkUpdateDescriptorSets>(pfn_get_device_proc_addr(device, "vkUpdateDescriptorSets"));
    assert(NULL != pfn_update_descriptor_sets);

    VkWriteDescriptorSet descriptor_write;
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = this->m_descriptor_set;
    descriptor_write.dstBinding = dst_binding;
    descriptor_write.dstArrayElement = dst_array_element;
    descriptor_write.descriptorCount = src_descriptor_count;

    brx_vector<VkDescriptorBufferInfo> buffer_info(static_cast<size_t>(src_descriptor_count));
    brx_vector<VkDescriptorImageInfo> image_info(static_cast<size_t>(src_descriptor_count));
    brx_vector<VkAccelerationStructureKHR> acceleration_structure_info(static_cast<size_t>(src_descriptor_count));
    switch (wrapped_descriptor_type)
    {
    case BRX_DESCRIPTOR_TYPE_DYNAMIC_UNIFORM_BUFFER:
    {
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

        assert(NULL != src_dynamic_uniform_buffers);
        assert(NULL != src_dynamic_uniform_buffer_ranges);
        assert(NULL == src_storage_buffers);
        assert(NULL == src_sampled_images);
        assert(NULL == src_samplers);
        assert(NULL == src_storage_images);
        assert(NULL == src_top_level_acceleration_structures);
        descriptor_write.pNext = NULL;
        descriptor_write.pImageInfo = NULL;
        descriptor_write.pBufferInfo = &buffer_info[0];
        descriptor_write.pTexelBufferView = NULL;

        for (uint32_t descriptor_index = 0U; descriptor_index < src_descriptor_count; ++descriptor_index)
        {
            assert(NULL != src_dynamic_uniform_buffers[descriptor_index]);
            buffer_info[descriptor_index].buffer = static_cast<brx_vk_uniform_upload_buffer const *>(src_dynamic_uniform_buffers[descriptor_index])->get_buffer();
            buffer_info[descriptor_index].offset = 0U;
            buffer_info[descriptor_index].range = src_dynamic_uniform_buffer_ranges[descriptor_index];
        }
    }
    break;
    case BRX_DESCRIPTOR_TYPE_READ_ONLY_STORAGE_BUFFER:
    case BRX_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    {
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

        assert(NULL == src_dynamic_uniform_buffers);
        assert(NULL == src_dynamic_uniform_buffer_ranges);
        assert(NULL != src_storage_buffers);
        assert(NULL == src_sampled_images);
        assert(NULL == src_samplers);
        assert(NULL == src_storage_images);
        assert(NULL == src_top_level_acceleration_structures);
        descriptor_write.pNext = NULL;
        descriptor_write.pImageInfo = NULL;
        descriptor_write.pBufferInfo = &buffer_info[0];
        descriptor_write.pTexelBufferView = NULL;

        for (uint32_t descriptor_index = 0U; descriptor_index < src_descriptor_count; ++descriptor_index)
        {
            assert(NULL != src_storage_buffers[descriptor_index]);
            buffer_info[descriptor_index].buffer = static_cast<brx_vk_storage_buffer const *>(src_storage_buffers[descriptor_index])->get_buffer();
            buffer_info[descriptor_index].offset = 0U;
            buffer_info[descriptor_index].range = static_cast<brx_vk_storage_buffer const *>(src_storage_buffers[descriptor_index])->get_size();
        }
    }
    break;
    case BRX_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    {
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;

        assert(NULL == src_dynamic_uniform_buffers);
        assert(NULL == src_dynamic_uniform_buffer_ranges);
        assert(NULL == src_storage_buffers);
        assert(NULL != src_sampled_images);
        assert(NULL == src_samplers);
        assert(NULL == src_storage_images);
        assert(NULL == src_top_level_acceleration_structures);
        descriptor_write.pNext = NULL;
        descriptor_write.pImageInfo = &image_info[0];
        descriptor_write.pBufferInfo = NULL;
        descriptor_write.pTexelBufferView = NULL;

        for (uint32_t descriptor_index = 0U; descriptor_index < src_descriptor_count; ++descriptor_index)
        {
            assert(NULL != src_sampled_images[descriptor_index]);
            image_info[descriptor_index].sampler = VK_NULL_HANDLE;
            image_info[descriptor_index].imageView = static_cast<brx_vk_sampled_image const *>(src_sampled_images[descriptor_index])->get_image_view();
            image_info[descriptor_index].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
    }
    break;
    case BRX_DESCRIPTOR_TYPE_SAMPLER:
    {
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;

        assert(NULL == src_dynamic_uniform_buffers);
        assert(NULL == src_dynamic_uniform_buffer_ranges);
        assert(NULL == src_storage_buffers);
        assert(NULL == src_sampled_images);
        assert(NULL != src_samplers);
        assert(NULL == src_storage_images);
        assert(NULL == src_top_level_acceleration_structures);
        descriptor_write.pNext = NULL;
        descriptor_write.pImageInfo = &image_info[0];
        descriptor_write.pBufferInfo = NULL;
        descriptor_write.pTexelBufferView = NULL;

        for (uint32_t descriptor_index = 0U; descriptor_index < src_descriptor_count; ++descriptor_index)
        {
            assert(NULL != src_samplers[descriptor_index]);
            image_info[descriptor_index].sampler = static_cast<brx_vk_sampler const *>(src_samplers[descriptor_index])->get_sampler();
            image_info[descriptor_index].imageView = VK_NULL_HANDLE;
            image_info[descriptor_index].imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
    }
    break;
    case BRX_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    {
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

        assert(NULL == src_dynamic_uniform_buffers);
        assert(NULL == src_dynamic_uniform_buffer_ranges);
        assert(NULL == src_storage_buffers);
        assert(NULL == src_sampled_images);
        assert(NULL == src_samplers);
        assert(NULL != src_storage_images);
        assert(NULL == src_top_level_acceleration_structures);
        descriptor_write.pNext = NULL;
        descriptor_write.pImageInfo = &image_info[0];
        descriptor_write.pBufferInfo = NULL;
        descriptor_write.pTexelBufferView = NULL;

        for (uint32_t descriptor_index = 0U; descriptor_index < src_descriptor_count; ++descriptor_index)
        {
            assert(NULL != src_storage_images[descriptor_index]);
            image_info[descriptor_index].sampler = VK_NULL_HANDLE;
            image_info[descriptor_index].imageView = static_cast<brx_vk_storage_image const *>(src_storage_images[descriptor_index])->get_image_view();
            image_info[descriptor_index].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        }
    }
    break;
    case BRX_DESCRIPTOR_TYPE_TOP_LEVEL_ACCELERATION_STRUCTURE:
    {
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

        VkWriteDescriptorSetAccelerationStructureKHR const descriptor_write_acceleration_structure = {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
            NULL,
            src_descriptor_count,
            &acceleration_structure_info[0]};

        assert(NULL == src_dynamic_uniform_buffers);
        assert(NULL == src_dynamic_uniform_buffer_ranges);
        assert(NULL == src_storage_buffers);
        assert(NULL == src_sampled_images);
        assert(NULL == src_samplers);
        assert(NULL != src_top_level_acceleration_structures);
        descriptor_write.pNext = &descriptor_write_acceleration_structure;
        descriptor_write.pImageInfo = NULL;
        descriptor_write.pBufferInfo = NULL;
        descriptor_write.pTexelBufferView = NULL;

        for (uint32_t descriptor_index = 0U; descriptor_index < src_descriptor_count; ++descriptor_index)
        {
            assert(NULL != src_top_level_acceleration_structures[descriptor_index]);
            acceleration_structure_info[descriptor_index] = static_cast<brx_vk_top_level_acceleration_structure const *>(src_top_level_acceleration_structures[descriptor_index])->get_acceleration_structure();
        }
    }
    break;
    default:
    {
        assert(false);
        descriptor_write.descriptorType = static_cast<VkDescriptorType>(-1);
        descriptor_write.pImageInfo = NULL;
        descriptor_write.pBufferInfo = NULL;
        descriptor_write.pTexelBufferView = NULL;
    }
    }

    pfn_update_descriptor_sets(device, 1U, &descriptor_write, 0U, NULL);
}

VkDescriptorSet brx_vk_descriptor_set::get_descriptor_set() const
{
    return this->m_descriptor_set;
}
