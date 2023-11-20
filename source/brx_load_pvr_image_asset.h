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

#ifndef _BRX_LOAD_PVR_IMAGE_ASSET_H_
#define _BRX_LOAD_PVR_IMAGE_ASSET_H_ 1

#include "../include/brx_load_image_asset.h"

extern bool brx_load_pvr_image_asset_header_from_input_stream(brx_load_asset_input_stream *input_stream, BRX_LOAD_IMAGE_ASSET_HEADER *image_asset_header, size_t *image_asset_data_offset);

extern bool brx_load_pvr_image_asset_data_from_input_stream(brx_load_asset_input_stream *input_stream, BRX_LOAD_IMAGE_ASSET_HEADER const *image_asset_header, size_t image_asset_data_offset, void *staging_upload_buffer_base, size_t subresource_count, BRX_LOAD_IMAGE_ASSET_SUBRESOURCE_MEMCPY_DEST const *subresource_memcpy_dests);

#endif
