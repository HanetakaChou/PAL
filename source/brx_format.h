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

#ifndef _BRX_FORMAT_H_
#define _BRX_FORMAT_H_ 1

#include "../include/brx_device.h"

extern uint32_t brx_get_format_aspect_count(BRX_ASSET_IMAGE_FORMAT format);

extern uint32_t brx_get_format_block_size(BRX_ASSET_IMAGE_FORMAT format);

extern uint32_t brx_get_format_block_width(BRX_ASSET_IMAGE_FORMAT format);

extern uint32_t brx_get_format_block_height(BRX_ASSET_IMAGE_FORMAT format);

#endif
