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

#ifndef _BRX_LOAD_IMAGE_ASSET_H_
#define _BRX_LOAD_IMAGE_ASSET_H_ 1

#include "brx_device.h"
#include "brx_load_asset_input_stream.h"

struct BRX_LOAD_IMAGE_ASSET_HEADER
{
    bool is_cube_map;
    BRX_ASSET_IMAGE_TYPE type;
    BRX_ASSET_IMAGE_FORMAT format;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mip_levels;
    uint32_t array_layers;
};

struct BRX_LOAD_IMAGE_ASSET_SUBRESOURCE_MEMCPY_DEST
{
    size_t staging_upload_buffer_offset;
    uint32_t output_row_pitch;
    uint32_t output_row_size;
    uint32_t output_row_count;
    uint32_t output_slice_pitch;
    uint32_t output_slice_count;
};

extern "C" uint32_t brx_load_image_asset_calculate_subresource_index(uint32_t mip_level, uint32_t array_layer, uint32_t aspect_index, uint32_t mip_levels, uint32_t array_layers);

extern "C" size_t brx_load_image_asset_calculate_subresource_memcpy_dests(BRX_ASSET_IMAGE_FORMAT format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mip_levels, uint32_t array_layers, size_t staging_upload_buffer_base_offset, uint32_t staging_upload_buffer_offset_alignment, uint32_t staging_upload_buffer_row_pitch_alignment, uint32_t subresource_count, BRX_LOAD_IMAGE_ASSET_SUBRESOURCE_MEMCPY_DEST *subresource_memcpy_dests);

extern "C" bool brx_load_image_asset_header_from_input_stream(brx_load_asset_input_stream *input_stream, BRX_LOAD_IMAGE_ASSET_HEADER *image_asset_header, size_t *image_asset_data_offset);

extern "C" bool brx_load_image_asset_data_from_input_stream(brx_load_asset_input_stream *input_stream, BRX_LOAD_IMAGE_ASSET_HEADER const *image_asset_header, size_t image_asset_data_offset, void *staging_upload_buffer_base, size_t subresource_count, BRX_LOAD_IMAGE_ASSET_SUBRESOURCE_MEMCPY_DEST const *subresource_memcpy_dests);

#endif
