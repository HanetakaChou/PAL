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

#include "brx_format.h"

extern uint32_t brx_get_format_aspect_count(BRX_ASSET_IMAGE_FORMAT format)
{
    switch (format)
    {
    case BRX_ASSET_IMAGE_FORMAT_R8G8B8A8_UNORM:
        return 1U;
    case BRX_ASSET_IMAGE_FORMAT_BC7_UNORM_BLOCK:
        return 1U;
    case BRX_ASSET_IMAGE_FORMAT_ASTC_4x4_UNORM_BLOCK:
        return 1U;
    default:
        return -1;
    }
}

extern uint32_t brx_get_format_block_size(BRX_ASSET_IMAGE_FORMAT format)
{
    // TODO:
    switch (format)
    {
    case BRX_ASSET_IMAGE_FORMAT_R8G8B8A8_UNORM:
        return 4U;
    case BRX_ASSET_IMAGE_FORMAT_BC7_UNORM_BLOCK:
        return (128U / 8U);
    case BRX_ASSET_IMAGE_FORMAT_ASTC_4x4_UNORM_BLOCK:
        return (128U / 8U);
    default:
        return -1;
    }
}

extern uint32_t brx_get_format_block_width(BRX_ASSET_IMAGE_FORMAT format)
{
    // TODO:
    switch (format)
    {
    case BRX_ASSET_IMAGE_FORMAT_R8G8B8A8_UNORM:
        return 1U;
    case BRX_ASSET_IMAGE_FORMAT_BC7_UNORM_BLOCK:
        return 4U;
    case BRX_ASSET_IMAGE_FORMAT_ASTC_4x4_UNORM_BLOCK:
        return 4U;
    default:
        return -1;
    }
}

extern uint32_t brx_get_format_block_height(BRX_ASSET_IMAGE_FORMAT format)
{
    // TODO:
    switch (format)
    {
    case BRX_ASSET_IMAGE_FORMAT_R8G8B8A8_UNORM:
        return 1U;
    case BRX_ASSET_IMAGE_FORMAT_BC7_UNORM_BLOCK:
        return 4U;
    case BRX_ASSET_IMAGE_FORMAT_ASTC_4x4_UNORM_BLOCK:
        return 4U;
    default:
        return -1;
    }
}
