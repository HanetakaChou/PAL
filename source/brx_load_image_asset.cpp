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
#include <stddef.h>
#include <assert.h>
#include <algorithm>
#include "../include/brx_load_image_asset.h"
#include "brx_format.h"
#include "brx_align_up.h"
#include "brx_load_dds_image_asset.h"
#include "brx_load_pvr_image_asset.h"

extern "C" uint32_t brx_load_image_asset_calculate_subresource_index(uint32_t mip_level, uint32_t array_layer, uint32_t aspect_index, uint32_t mip_levels, uint32_t array_layers)
{
    // [D3D12CalcSubresource](https://github.com/microsoft/DirectX-Headers/blob/48f23952bc08a6dce0727339c07cedbc4797356c/include/directx/d3dx12_core.h#L1166)
    return mip_level + array_layer * mip_levels + aspect_index * mip_levels * array_layers;
}

extern "C" size_t brx_load_image_asset_calculate_subresource_memcpy_dests(BRX_ASSET_IMAGE_FORMAT format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mip_levels, uint32_t array_layers, size_t staging_upload_buffer_base_offset, uint32_t staging_upload_buffer_offset_alignment, uint32_t staging_upload_buffer_row_pitch_alignment, uint32_t subresource_count, BRX_LOAD_IMAGE_ASSET_SUBRESOURCE_MEMCPY_DEST *subresource_memcpy_dests)
{
    // https://source.winehq.org/git/vkd3d.git/
    // libs/vkd3d/device.c
    // d3d12_device_GetCopyableFootprints
    // libs/vkd3d/utils.c
    // vkd3d_formats

    // https://github.com/ValveSoftware/dxvk/blob/master/src/dxvk/dxvk_context.cpp
    // DxvkContext::uploadImage

    // Context::texSubImage2D libANGLE/Context.cpp
    // Texture::setSubImage libANGLE/Texture.cpp
    // TextureVk::setSubImage libANGLE/renderer/vulkan/TextureVk.cpp
    // TextureVk::setSubImageImpl libANGLE/renderer/vulkan/TextureVk.cpp
    // ImageHelper::stageSubresourceUpdate libANGLE/renderer/vulkan/vk_helpers.cpp
    // ImageHelper::CalculateBufferInfo libANGLE/renderer/vulkan/vk_helpers.cpp
    // ImageHelper::stageSubresourceUpdateImpl libANGLE/renderer/vulkan/vk_helpers.cpp

    // [D3D12_PROPERTY_LAYOUT_FORMAT_TABLE::CalculateResourceSize](https://github.com/microsoft/DirectX-Headers/blob/v1.610.2/src/d3dx12_property_format_table.cpp#L1014)

    uint32_t const aspect_count = brx_get_format_aspect_count(format);
    uint32_t const block_size = brx_get_format_block_size(format);
    uint32_t const block_width = brx_get_format_block_width(format);
    uint32_t const block_height = brx_get_format_block_height(format);

    // vkspec: bufferOffset must be a multiple of 4
    staging_upload_buffer_offset_alignment = brx_align_up(brx_align_up(staging_upload_buffer_offset_alignment, 4U), block_size);

    size_t total_bytes = 0U;
    size_t staging_upload_buffer_offset = staging_upload_buffer_base_offset;

    // TODO: support more than one aspect
    assert(1U == aspect_count);
    for (uint32_t aspect_index = 0U; aspect_index < aspect_count; ++aspect_index)
    {
        for (uint32_t array_layer = 0U; array_layer < array_layers; ++array_layer)
        {
            uint32_t w = width;
            uint32_t h = height;
            uint32_t d = depth;

            for (uint32_t mipLevel = 0U; mipLevel < mip_levels; ++mipLevel)
            {

                uint32_t const output_row_size = ((w + (block_width - 1U)) / block_width) * block_size;
                uint32_t const output_row_count = (h + (block_height - 1U)) / block_height;
                uint32_t const output_slice_count = d;

                uint32_t const output_row_pitch = brx_align_up(output_row_size, staging_upload_buffer_row_pitch_alignment);
                uint32_t const output_slice_pitch = output_row_pitch * output_row_count;

                size_t const new_staging_upload_buffer_offset = brx_align_up(staging_upload_buffer_offset, static_cast<size_t>(staging_upload_buffer_offset_alignment));
                total_bytes += (new_staging_upload_buffer_offset - staging_upload_buffer_offset);
                staging_upload_buffer_offset = new_staging_upload_buffer_offset;

                uint32_t const subresource_index = brx_load_image_asset_calculate_subresource_index(mipLevel, array_layer, aspect_index, mip_levels, array_layers);
                assert(subresource_index < subresource_count);

                subresource_memcpy_dests[subresource_index].staging_upload_buffer_offset = staging_upload_buffer_offset;
                subresource_memcpy_dests[subresource_index].output_row_pitch = output_row_pitch;
                subresource_memcpy_dests[subresource_index].output_row_size = output_row_size;
                subresource_memcpy_dests[subresource_index].output_slice_pitch = output_slice_pitch;
                subresource_memcpy_dests[subresource_index].output_row_count = output_row_count;
                subresource_memcpy_dests[subresource_index].output_slice_count = output_slice_count;

                size_t const surface_size = (static_cast<size_t>(output_slice_pitch) * static_cast<size_t>(output_slice_count));
                total_bytes += surface_size;
                staging_upload_buffer_offset += surface_size;
                assert((staging_upload_buffer_base_offset + total_bytes) == staging_upload_buffer_offset);

                w = w >> 1U;
                h = h >> 1U;
                d = d >> 1U;
                if (0U == w)
                {
                    w = 1U;
                }
                if (0U == h)
                {
                    h = 1U;
                }
                if (0U == d)
                {
                    d = 1U;
                }
            }
        }
    }

    return total_bytes;
}

//--------------------------------------------------------------------------------------
static inline constexpr uint32_t MakeFourCC(char ch0, char ch1, char ch2, char ch3)
{
    return static_cast<uint32_t>(static_cast<uint8_t>(ch0)) | (static_cast<uint32_t>(static_cast<uint8_t>(ch1)) << 8U) | (static_cast<uint32_t>(static_cast<uint8_t>(ch2)) << 16U) | (static_cast<uint32_t>(static_cast<uint8_t>(ch3)) << 24U);
}

enum : uint32_t
{
    DDS_MAGIC = MakeFourCC('D', 'D', 'S', ' '),
    Pvr_HeaderVersionV3 = MakeFourCC('P', 'V', 'R', 3)

};

//--------------------------------------------------------------------------------------
extern "C" bool brx_load_image_asset_header_from_input_stream(brx_load_asset_input_stream *input_stream, BRX_LOAD_IMAGE_ASSET_HEADER *image_asset_header, size_t *image_asset_data_offset)
{
    if (-1 == input_stream->seek(0, LOAD_ASSET_INPUT_STREAM_SEEK_SET))
    {
        return false;
    }

    uint32_t fourcc;
    {
        ptrdiff_t const BytesRead = input_stream->read(&fourcc, sizeof(uint32_t));
        if (BytesRead == -1 || static_cast<size_t>(BytesRead) < sizeof(uint32_t))
        {
            return false;
        }
    }

    switch (fourcc)
    {
    case DDS_MAGIC:
        return brx_load_dds_image_asset_header_from_input_stream(input_stream, image_asset_header, image_asset_data_offset);
    case Pvr_HeaderVersionV3:
        return brx_load_pvr_image_asset_header_from_input_stream(input_stream, image_asset_header, image_asset_data_offset);
    default:
        return false;
    }
}

extern "C" bool brx_load_image_asset_data_from_input_stream(brx_load_asset_input_stream *input_stream, BRX_LOAD_IMAGE_ASSET_HEADER const *image_asset_header, size_t image_asset_data_offset, void *staging_upload_buffer_base, size_t subresource_count, BRX_LOAD_IMAGE_ASSET_SUBRESOURCE_MEMCPY_DEST const *subresource_memcpy_dests)
{
    if (-1 == input_stream->seek(0, LOAD_ASSET_INPUT_STREAM_SEEK_SET))
    {
        return false;
    }

    uint32_t fourcc;
    {
        ptrdiff_t const BytesRead = input_stream->read(&fourcc, sizeof(uint32_t));
        if (BytesRead == -1 || static_cast<size_t>(BytesRead) < sizeof(uint32_t))
        {
            return false;
        }
    }

    switch (fourcc)
    {
    case DDS_MAGIC:
        return brx_load_dds_image_asset_data_from_input_stream(input_stream, image_asset_header, image_asset_data_offset, staging_upload_buffer_base, subresource_count, subresource_memcpy_dests);
    case Pvr_HeaderVersionV3:
        return brx_load_pvr_image_asset_data_from_input_stream(input_stream, image_asset_header, image_asset_data_offset, staging_upload_buffer_base, subresource_count, subresource_memcpy_dests);
    default:
        return false;
    }
}