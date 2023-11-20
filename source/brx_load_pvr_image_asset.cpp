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
#include "brx_load_dds_image_asset.h"

extern bool brx_load_pvr_image_asset_header_from_input_stream(brx_load_asset_input_stream *input_stream, BRX_LOAD_IMAGE_ASSET_HEADER *image_asset_header, size_t *image_asset_data_offset);

extern bool brx_load_pvr_image_asset_data_from_input_stream(brx_load_asset_input_stream *input_stream, BRX_LOAD_IMAGE_ASSET_HEADER const *image_asset_header, size_t image_asset_data_offset, void *staging_upload_buffer_base, size_t subresource_count, BRX_LOAD_IMAGE_ASSET_SUBRESOURCE_MEMCPY_DEST const *subresource_memcpy_dests);

// https://github.com/powervr-graphics/Native_SDK/blob/master/framework/PVRCore/textureio/FileDefinesPVR.h
// https://github.com/powervr-graphics/Native_SDK/blob/master/framework/PVRCore/textureio/TextureReaderPVR.h
// https://github.com/powervr-graphics/Native_SDK/blob/master/framework/PVRCore/textureio/TextureReaderPVR.cpp
// https://github.com/powervr-graphics/Native_SDK/blob/master/framework/PVRCore/texture/TextureHeader.cpp

static inline constexpr uint32_t Pvr_MakeFourCC(char ch0, char ch1, char ch2, char ch3)
{
    return static_cast<uint32_t>(static_cast<uint8_t>(ch0)) | (static_cast<uint32_t>(static_cast<uint8_t>(ch1)) << 8U) | (static_cast<uint32_t>(static_cast<uint8_t>(ch2)) << 16U) | (static_cast<uint32_t>(static_cast<uint8_t>(ch3)) << 24U);
}

static inline constexpr uint64_t Pvr_MakePixelTypeChar(char C1Name, char C2Name, char C3Name, char C4Name, char C1Bits, char C2Bits, char C3Bits, char C4Bits)
{
    return static_cast<uint64_t>(static_cast<uint8_t>(C1Name)) | (static_cast<uint64_t>(static_cast<uint8_t>(C2Name)) << 8) | (static_cast<uint64_t>(static_cast<uint8_t>(C3Name)) << 16) | (static_cast<uint64_t>(static_cast<uint8_t>(C4Name)) << 24) | (static_cast<uint64_t>(static_cast<uint8_t>(C1Bits)) << 32) | (static_cast<uint64_t>(static_cast<uint8_t>(C2Bits)) << 40) | (static_cast<uint64_t>(static_cast<uint8_t>(C3Bits)) << 48) | (static_cast<uint64_t>(static_cast<uint8_t>(C4Bits)) << 56);
}

enum : uint32_t
{
    Pvr_HeaderVersionV3 = Pvr_MakeFourCC('P', 'V', 'R', 3)

    // TODO: support endianess
    // Pvr_HeaderVersionV3_Reversed = Pvr_MakeFourCC(3, 'R', 'V', 'P')
};

enum : uint64_t
{
    Pvr_PixelTypeID_PVRTCI_2bpp_RGB = 0,
    Pvr_PixelTypeID_PVRTCI_2bpp_RGBA = 1,
    Pvr_PixelTypeID_PVRTCI_4bpp_RGB = 2,
    Pvr_PixelTypeID_PVRTCI_4bpp_RGBA = 3,
    Pvr_PixelTypeID_PVRTCII_2bpp = 4,
    Pvr_PixelTypeID_PVRTCII_4bpp = 5,
    Pvr_PixelTypeID_ETC1 = 6,
    Pvr_PixelTypeID_DXT1 = 7,
    Pvr_PixelTypeID_DXT2 = 8,
    Pvr_PixelTypeID_DXT3 = 9,
    Pvr_PixelTypeID_DXT4 = 10,
    Pvr_PixelTypeID_DXT5 = 11,

    // These formats are identical to some DXT formats.
    Pvr_PixelTypeID_BC1 = Pvr_PixelTypeID_DXT1,
    Pvr_PixelTypeID_BC2 = Pvr_PixelTypeID_DXT3,
    Pvr_PixelTypeID_BC3 = Pvr_PixelTypeID_DXT5,

    // These are currently unsupported:
    Pvr_PixelTypeID_BC4 = 12,
    Pvr_PixelTypeID_BC5 = 13,
    Pvr_PixelTypeID_BC6 = 14,
    Pvr_PixelTypeID_BC7 = 15,

    // These are supported
    Pvr_PixelTypeID_UYVY = 16,
    Pvr_PixelTypeID_YUY2 = 17,
    Pvr_PixelTypeID_BW1bpp = 18,
    Pvr_PixelTypeID_SharedExponentR9G9B9E5 = 19,
    Pvr_PixelTypeID_RGBG8888 = 20,
    Pvr_PixelTypeID_GRGB8888 = 21,
    Pvr_PixelTypeID_ETC2_RGB = 22,
    Pvr_PixelTypeID_ETC2_RGBA = 23,
    Pvr_PixelTypeID_ETC2_RGB_A1 = 24,
    Pvr_PixelTypeID_EAC_R11 = 25,
    Pvr_PixelTypeID_EAC_RG11 = 26,

    Pvr_PixelTypeID_ASTC_4x4 = 27,
    Pvr_PixelTypeID_ASTC_5x4 = 28,
    Pvr_PixelTypeID_ASTC_5x5 = 29,
    Pvr_PixelTypeID_ASTC_6x5 = 30,
    Pvr_PixelTypeID_ASTC_6x6 = 31,
    Pvr_PixelTypeID_ASTC_8x5 = 32,
    Pvr_PixelTypeID_ASTC_8x6 = 33,
    Pvr_PixelTypeID_ASTC_8x8 = 34,
    Pvr_PixelTypeID_ASTC_10x5 = 35,
    Pvr_PixelTypeID_ASTC_10x6 = 36,
    Pvr_PixelTypeID_ASTC_10x8 = 37,
    Pvr_PixelTypeID_ASTC_10x10 = 38,
    Pvr_PixelTypeID_ASTC_12x10 = 39,
    Pvr_PixelTypeID_ASTC_12x12 = 40,

    Pvr_PixelTypeID_ASTC_3x3x3 = 41,
    Pvr_PixelTypeID_ASTC_4x3x3 = 42,
    Pvr_PixelTypeID_ASTC_4x4x3 = 43,
    Pvr_PixelTypeID_ASTC_4x4x4 = 44,
    Pvr_PixelTypeID_ASTC_5x4x4 = 45,
    Pvr_PixelTypeID_ASTC_5x5x4 = 46,
    Pvr_PixelTypeID_ASTC_5x5x5 = 47,
    Pvr_PixelTypeID_ASTC_6x5x5 = 48,
    Pvr_PixelTypeID_ASTC_6x6x5 = 49,
    Pvr_PixelTypeID_ASTC_6x6x6 = 50
};

enum : uint64_t
{
    Pvr_PixelTypeChar_I8 = Pvr_MakePixelTypeChar('i', '\0', '\0', '\0', 8, 0, 0, 0),
    Pvr_PixelTypeChar_R8 = Pvr_MakePixelTypeChar('r', '\0', '\0', '\0', 8, 0, 0, 0),
    Pvr_PixelTypeChar_R8G8 = Pvr_MakePixelTypeChar('r', 'g', '\0', '\0', 8, 8, 0, 0),
    Pvr_PixelTypeChar_R8G8B8 = Pvr_MakePixelTypeChar('r', 'g', 'b', '\0', 8, 8, 8, 0),
    Pvr_PixelTypeChar_B8G8R8 = Pvr_MakePixelTypeChar('b', 'g', 'r', '\0', 8, 8, 8, 0),
    Pvr_PixelTypeChar_R8G8B8A8 = Pvr_MakePixelTypeChar('r', 'g', 'b', 'a', 8, 8, 8, 8),
    Pvr_PixelTypeChar_B8G8R8A8 = Pvr_MakePixelTypeChar('b', 'g', 'r', 'a', 8, 8, 8, 8),
    Pvr_PixelTypeChar_A8B8G8R8 = Pvr_MakePixelTypeChar('a', 'b', 'g', 'r', 8, 8, 8, 8),
    Pvr_PixelTypeChar_R16 = Pvr_MakePixelTypeChar('r', '\0', '\0', '\0', 16, 0, 0, 0),
    Pvr_PixelTypeChar_R16G16 = Pvr_MakePixelTypeChar('r', 'g', '\0', '\0', 16, 16, 0, 0),
    Pvr_PixelTypeChar_L16A16 = Pvr_MakePixelTypeChar('l', 'a', '\0', '\0', 16, 16, 0, 0),
    Pvr_PixelTypeChar_R16G16B16 = Pvr_MakePixelTypeChar('r', 'g', 'b', '\0', 16, 16, 16, 0),
    Pvr_PixelTypeChar_R16G16B16A16 = Pvr_MakePixelTypeChar('r', 'g', 'b', 'a', 16, 16, 16, 16),
    Pvr_PixelTypeChar_R32 = Pvr_MakePixelTypeChar('r', '\0', '\0', '\0', 32, 0, 0, 0),
    Pvr_PixelTypeChar_L32 = Pvr_MakePixelTypeChar('l', '\0', '\0', '\0', 32, 0, 0, 0),
    Pvr_PixelTypeChar_R32G32 = Pvr_MakePixelTypeChar('r', 'g', '\0', '\0', 32, 32, 0, 0),
    Pvr_PixelTypeChar_L32A32 = Pvr_MakePixelTypeChar('l', 'a', '\0', '\0', 32, 32, 0, 0),
    Pvr_PixelTypeChar_R32G32B32 = Pvr_MakePixelTypeChar('r', 'g', 'b', '\0', 32, 32, 32, 0),
    Pvr_PixelTypeChar_R32G32B32A32 = Pvr_MakePixelTypeChar('r', 'g', 'b', 'a', 32, 32, 32, 32),
    Pvr_PixelTypeChar_R5G6B5 = Pvr_MakePixelTypeChar('r', 'g', 'b', '\0', 5, 6, 5, 0),
    Pvr_PixelTypeChar_R4G4B4A4 = Pvr_MakePixelTypeChar('r', 'g', 'b', 'a', 4, 4, 4, 4),
    Pvr_PixelTypeChar_R5G5B5A1 = Pvr_MakePixelTypeChar('r', 'g', 'b', 'a', 5, 5, 5, 1),
    Pvr_PixelTypeChar_R11G11B10 = Pvr_MakePixelTypeChar('b', 'g', 'r', '\0', 10, 11, 11, 0),
    Pvr_PixelTypeChar_D8 = Pvr_MakePixelTypeChar('d', '\0', '\0', '\0', 8, 0, 0, 0),
    Pvr_PixelTypeChar_S8 = Pvr_MakePixelTypeChar('s', '\0', '\0', '\0', 8, 0, 0, 0),
    Pvr_PixelTypeChar_D16 = Pvr_MakePixelTypeChar('d', '\0', '\0', '\0', 16, 0, 0, 0),
    Pvr_PixelTypeChar_D24 = Pvr_MakePixelTypeChar('d', '\0', '\0', '\0', 24, 0, 0, 0),
    Pvr_PixelTypeChar_D32 = Pvr_MakePixelTypeChar('d', '\0', '\0', '\0', 32, 0, 0, 0),
    Pvr_PixelTypeChar_D16S8 = Pvr_MakePixelTypeChar('d', 's', '\0', '\0', 16, 8, 0, 0),
    Pvr_PixelTypeChar_D24S8 = Pvr_MakePixelTypeChar('d', 's', '\0', '\0', 24, 8, 0, 0),
    Pvr_PixelTypeChar_D32S8 = Pvr_MakePixelTypeChar('d', 's', '\0', '\0', 32, 8, 0, 0)
};

enum : uint32_t
{
    Pvr_ColorSpace_lRGB = 0,
    Pvr_ColorSpace_sRGB = 1
};

enum : uint32_t
{
    Pvr_ChannelType_UnsignedByteNorm = 0,
    Pvr_ChannelType_SignedByteNorm = 1,
    Pvr_ChannelType_UnsignedByte = 2,
    Pvr_ChannelType_SignedByte = 3,
    Pvr_ChannelType_UnsignedShortNorm = 4,
    Pvr_ChannelType_SignedShortNorm = 5,
    Pvr_ChannelType_UnsignedShort = 6,
    Pvr_ChannelType_SignedShort = 7,
    Pvr_ChannelType_UnsignedIntegerNorm = 8,
    Pvr_ChannelType_SignedIntegerNorm = 9,
    Pvr_ChannelType_UnsignedInteger = 10,
    Pvr_ChannelType_SignedInteger = 11,
    Pvr_ChannelType_SignedFloat = 12,
    Pvr_ChannelType_UnsignedFloat = 13,
    Pvr_ChannelType_NumCTs = 14
};

// V3 Header Identifiers.
/// <summary>This header stores everything that you would ever need to load (but not necessarily use) a texture's
/// data accurately, but no more. Data that is provided but is not needed to read the data is stored in the
/// Metadata section (See TextureHeaderWithMetadata). Correct use of the texture may rely on meta data, but
/// accurate data loading can be done through the standard header alone.</summary>
struct Pvr_HeaderV3
{
    uint32_t version;      //!< PVR format v3 identifier
    uint32_t flags;        //!< Various format flags.
    uint64_t pixelFormat;  //!< The pixel format, 8cc value storing the 4 channel identifiers and their respective sizes.
    uint32_t colorSpace;   //!< The Color Space of the texture, currently either linear RGB or sRGB.
    uint32_t channelType;  //!< Variable type that the channel is stored in. Supports signed/unsigned int/short/char/float.
    uint32_t height;       //!< Height of the texture.
    uint32_t width;        //!< Width of the texture.
    uint32_t depth;        //!< Depth of the texture. (Z-slices)
    uint32_t numSurfaces;  //!< Number of members in a Texture Array.
    uint32_t numFaces;     //!< Number of faces in a Cube Map. Maybe be a value other than 6.
    uint32_t numMipMaps;   //!< Number of MIP Maps in the texture - NB: Includes top level.
    uint32_t metaDataSize; //!< Size of the accompanying meta data.
};

struct Pvr_MetaData
{
    uint32_t _fourCC;   // A 4cc descriptor of the data type's creator. // Values equating to values between 'P' 'V' 'R' 0 and 'P' 'V' 'R' 255 will be used by our headers.
    uint32_t _key;      // Enumeration key identifying the data type.
    uint32_t _dataSize; // Size of attached data.
    char _data[1];      // Data array, can be absolutely anything, the loader needs to know how to handle it based on fourCC and key.
};

//--------------------------------------------------------------------------------------
static inline uint32_t Pvr_GetPixelFormatPartHigh(uint64_t pixelFormat);

static inline bool Pvr_GetMinDimensionsForFormat(uint64_t pixelFormat, uint32_t *minX, uint32_t *minY, uint32_t *minZ);

static inline uint32_t Pvr_GetBitsPerPixel(uint64_t pixelFormat);

static inline uint32_t pvr_get_format_plane_count(uint64_t pixelFormat);

static inline bool pvr_is_depth_stencil(uint64_t pixelFormat);

//--------------------------------------------------------------------------------------
extern bool brx_load_pvr_image_asset_header_from_input_stream(brx_load_asset_input_stream *input_stream, BRX_LOAD_IMAGE_ASSET_HEADER *image_asset_header, size_t *image_asset_data_offset)
{
    assert(image_asset_header != NULL);
    assert(image_asset_data_offset != NULL);

    if (-1 == input_stream->seek(0, LOAD_ASSET_INPUT_STREAM_SEEK_SET))
    {
        return false;
    }

    Pvr_HeaderV3 header;
    {
        ptrdiff_t const BytesRead = input_stream->read(&header, sizeof(header.version) + sizeof(header.flags) + sizeof(header.pixelFormat) + sizeof(header.colorSpace) + sizeof(header.channelType) + sizeof(header.height) + sizeof(header.width) + sizeof(header.depth) + sizeof(header.numSurfaces) + sizeof(header.numFaces) + sizeof(header.numMipMaps) + sizeof(header.metaDataSize));
        if (BytesRead == -1 || static_cast<size_t>(BytesRead) < (sizeof(header.version) + sizeof(header.flags) + sizeof(header.pixelFormat) + sizeof(header.colorSpace) + sizeof(header.channelType) + sizeof(header.height) + sizeof(header.width) + sizeof(header.depth) + sizeof(header.numSurfaces) + sizeof(header.numFaces) + sizeof(header.numMipMaps) + sizeof(header.metaDataSize)))
        {
            return false;
        }
    }

    if (Pvr_HeaderVersionV3 != header.version)
    {
        return false;
    }

#ifndef NDEBUG
    // TODO: handle metadata
    {
        uint32_t metaDataRead = 0U;

        while (metaDataRead < header.metaDataSize)
        {
            Pvr_MetaData metadata;
            {
                ptrdiff_t BytesRead = input_stream->read(&metadata, sizeof(metadata._fourCC) + sizeof(metadata._key) + sizeof(metadata._dataSize));
                if (BytesRead == -1 || static_cast<size_t>(BytesRead) < (sizeof(metadata._fourCC) + sizeof(metadata._key) + sizeof(metadata._dataSize)))
                {
                    return false;
                }
            }
            metaDataRead += (sizeof(metadata._fourCC) + sizeof(metadata._key) + sizeof(metadata._dataSize));

            if (Pvr_HeaderVersionV3 != metadata._fourCC)
            {
                return false;
            }

            if (-1 == input_stream->seek(metadata._dataSize, LOAD_ASSET_INPUT_STREAM_SEEK_CUR))
            {
                return false;
            }
            metaDataRead += metadata._dataSize;
        }

        assert(metaDataRead == header.metaDataSize);
    }
#endif

    (*image_asset_data_offset) = (sizeof(header.version) + sizeof(header.flags) + sizeof(header.pixelFormat) + sizeof(header.colorSpace) + sizeof(header.channelType) + sizeof(header.height) + sizeof(header.width) + sizeof(header.depth) + sizeof(header.numSurfaces) + sizeof(header.numFaces) + sizeof(header.numMipMaps) + sizeof(header.metaDataSize) + header.metaDataSize);

    if (1U == header.numFaces)
    {
        image_asset_header->is_cube_map = false;
    }
    else if (header.numFaces > 1)
    {
        image_asset_header->is_cube_map = true;
    }
    else
    {
        return false;
    }

    if (1U == header.depth)
    {
        if (header.height > 1U)
        {
            image_asset_header->type = BRX_ASSET_IMAGE_TYPE_2D;
        }
        else if (1U == header.height)
        {
            image_asset_header->type = BRX_ASSET_IMAGE_TYPE_1D;
        }
        else
        {
            return false;
        }
    }
    else if (header.depth > 1)
    {
        image_asset_header->type = BRX_ASSET_IMAGE_TYPE_3D;
    }
    else
    {
        return false;
    }

    if (0U == Pvr_GetPixelFormatPartHigh(header.pixelFormat))
    {
        switch (header.pixelFormat)
        {
        // TODO:
        // case Pvr_PixelTypeID_BC6:
        // {
        //     image_asset_header->format = (Pvr_ChannelType_UnsignedFloat == header.channelType) ? BRX_ASSET_IMAGE_FORMAT_BC6H_UFLOAT_BLOCK : ((Pvr_ChannelType_SignedFloat == header.colorSpace) ? BRX_ASSET_IMAGE_FORMAT_BC6H_SFLOAT_BLOCK : static_cast<BRX_ASSET_IMAGE_FORMAT>(-1));
        // }
        // break;
        case Pvr_PixelTypeID_BC7:
        {
            image_asset_header->format = (Pvr_ColorSpace_lRGB == header.colorSpace) ? BRX_ASSET_IMAGE_FORMAT_BC7_UNORM_BLOCK : ((Pvr_ColorSpace_sRGB == header.colorSpace) ? static_cast<BRX_ASSET_IMAGE_FORMAT>(-1) : static_cast<BRX_ASSET_IMAGE_FORMAT>(-1));
        }
        break;
        case Pvr_PixelTypeID_ASTC_4x4:
        {
            image_asset_header->format = (Pvr_ColorSpace_lRGB == header.colorSpace) ? BRX_ASSET_IMAGE_FORMAT_ASTC_4x4_UNORM_BLOCK : ((Pvr_ColorSpace_sRGB == header.colorSpace) ? static_cast<BRX_ASSET_IMAGE_FORMAT>(-1) : static_cast<BRX_ASSET_IMAGE_FORMAT>(-1));
        }
        break;
        default:
            // TODO: support more format
            assert(false);
            image_asset_header->format = static_cast<BRX_ASSET_IMAGE_FORMAT>(-1);
            return false;
        }
    }
    else
    {
        switch (header.pixelFormat)
        {
        case Pvr_PixelTypeChar_R8G8B8A8:
        {
            image_asset_header->format = (Pvr_ChannelType_UnsignedByteNorm == header.channelType) ? ((Pvr_ColorSpace_lRGB == header.colorSpace) ? BRX_ASSET_IMAGE_FORMAT_R8G8B8A8_UNORM : static_cast<BRX_ASSET_IMAGE_FORMAT>(-1)) : static_cast<BRX_ASSET_IMAGE_FORMAT>(-1);
        }
        break;
        default:
            // TODO: support more format
            assert(false);
            image_asset_header->format = static_cast<BRX_ASSET_IMAGE_FORMAT>(-1);
            return false;
        }
    }

    image_asset_header->width = header.width;
    image_asset_header->height = header.height;
    image_asset_header->depth = header.depth;
    image_asset_header->mip_levels = header.numMipMaps;
    image_asset_header->array_layers = header.numFaces * header.numSurfaces;

    return true;
}

extern bool brx_load_pvr_image_asset_data_from_input_stream(brx_load_asset_input_stream *input_stream, BRX_LOAD_IMAGE_ASSET_HEADER const *image_asset_header, size_t image_asset_data_offset, void *staging_upload_buffer_base, size_t subresource_count, BRX_LOAD_IMAGE_ASSET_SUBRESOURCE_MEMCPY_DEST const *subresource_memcpy_dests)
{
#ifndef NDEBUG
    BRX_LOAD_IMAGE_ASSET_HEADER image_asset_header_for_validate;
    size_t image_asset_data_offset_for_validate;
    if (!brx_load_pvr_image_asset_header_from_input_stream(input_stream, &image_asset_header_for_validate, &image_asset_data_offset_for_validate))
    {
        return false;
    }

    assert(image_asset_header->is_cube_map == image_asset_header_for_validate.is_cube_map);
    assert(image_asset_header->type == image_asset_header_for_validate.type);
    assert(image_asset_header->format == image_asset_header_for_validate.format);
    assert(image_asset_header->width == image_asset_header_for_validate.width);
    assert(image_asset_header->height == image_asset_header_for_validate.height);
    assert(image_asset_header->depth == image_asset_header_for_validate.depth);
    assert(image_asset_header->mip_levels == image_asset_header_for_validate.mip_levels);
    assert(image_asset_header->array_layers == image_asset_header_for_validate.array_layers);

    assert(image_asset_data_offset == image_asset_data_offset_for_validate);
#endif

    uint64_t pixel_format;
    switch (image_asset_header->format)
    {
    case BRX_ASSET_IMAGE_FORMAT_R8G8B8A8_UNORM:
        pixel_format = Pvr_PixelTypeChar_R8G8B8A8;
        break;
    case BRX_ASSET_IMAGE_FORMAT_BC7_UNORM_BLOCK:
        pixel_format = Pvr_PixelTypeID_BC7;
        break;
    case BRX_ASSET_IMAGE_FORMAT_ASTC_4x4_UNORM_BLOCK:
        pixel_format = Pvr_PixelTypeID_ASTC_4x4;
        break;
    default:
        assert(false);
        pixel_format = -1;
    }

    uint32_t numberOfPlanes = pvr_get_format_plane_count(pixel_format);
    if (0U == numberOfPlanes)
    {
        return false;
    }

    if ((numberOfPlanes > 1U) && pvr_is_depth_stencil(pixel_format))
    {
        // DirectX 12 uses planes for stencil, DirectX 11 does not
        return false;
    }

    size_t const numberOfResources = ((BRX_ASSET_IMAGE_TYPE_3D == image_asset_header->type) ? static_cast<size_t>(1U) : static_cast<size_t>(image_asset_header->array_layers)) * static_cast<size_t>(image_asset_header->mip_levels) * static_cast<size_t>(numberOfPlanes);

    if (numberOfResources != subresource_count)
    {
        return false;
    }

    uint32_t uiSmallestWidth;
    uint32_t uiSmallestHeight;
    uint32_t uiSmallestDepth;
    if (!Pvr_GetMinDimensionsForFormat(pixel_format, &uiSmallestWidth, &uiSmallestHeight, &uiSmallestDepth))
    {
        return false;
    }

    size_t inputSkipBytes = image_asset_data_offset;

    // Write the texture data
    for (uint32_t mipMap = 0; mipMap < image_asset_header->mip_levels; ++mipMap)
    {
        uint32_t uiWidth;
        uint32_t uiHeight;
        uint32_t uiDepth;
        {
            // Get the dimensions of the current MIP Map level.
            uiWidth = std::max<uint32_t>(image_asset_header->width >> mipMap, 1);
            uiHeight = std::max<uint32_t>(image_asset_header->height >> mipMap, 1);
            uiDepth = std::max<uint32_t>(image_asset_header->depth >> mipMap, 1);

            // If pixel format is compressed, the dimensions need to be padded.
            if (0U == Pvr_GetPixelFormatPartHigh(pixel_format))
            {
                uiWidth = uiWidth + ((-1 * uiWidth) % uiSmallestWidth);
                uiHeight = uiHeight + ((-1 * uiHeight) % uiSmallestHeight);
                uiDepth = uiDepth + ((-1 * uiDepth) % uiSmallestDepth);
            }
        }

        uint32_t uiRowBytes;
        uint32_t uiNumRows;
        uint32_t uiNumSlices;
        if (pixel_format >= Pvr_PixelTypeID_ASTC_4x4 && pixel_format <= Pvr_PixelTypeID_ASTC_6x6x6)
        {
            uiRowBytes = (128U / 8U) * (uiWidth / uiSmallestWidth);
            uiNumRows = (uiHeight / uiSmallestHeight);
            uiNumSlices = (uiDepth / uiSmallestDepth);
        }
        else
        {
            uint32_t const bpp = Pvr_GetBitsPerPixel(pixel_format);

            uiRowBytes = ((bpp * uiWidth) / 8U) * uiSmallestHeight * uiSmallestDepth;
            uiNumRows = (uiHeight / uiSmallestHeight);
            uiNumSlices = (uiDepth / uiSmallestDepth);
        }

        size_t inputRowSize = uiRowBytes;
        size_t inputNumRows = uiNumRows;
        size_t inputSliceSize = uiRowBytes * uiNumRows;
        size_t inputNumSlices = uiNumSlices;

        for (uint32_t arrayIndex = 0U; arrayIndex < image_asset_header->array_layers; ++arrayIndex)
        {

            // TODO: support more than one plane
            assert(1 == numberOfPlanes);
            for (uint32_t planeIndex = 0; planeIndex < 1U; ++planeIndex)
            {
                uint32_t dstSubresource = brx_load_image_asset_calculate_subresource_index(mipMap, arrayIndex, planeIndex, image_asset_header->mip_levels, image_asset_header->array_layers);
                assert(dstSubresource < subresource_count);

                assert(inputNumSlices == subresource_memcpy_dests[dstSubresource].output_slice_count);
                assert(inputNumRows == subresource_memcpy_dests[dstSubresource].output_row_count);
                assert(inputRowSize == subresource_memcpy_dests[dstSubresource].output_row_size);

                if (inputSliceSize == subresource_memcpy_dests[dstSubresource].output_slice_pitch && inputRowSize == subresource_memcpy_dests[dstSubresource].output_row_pitch)
                {
                    assert(input_stream->seek(0, LOAD_ASSET_INPUT_STREAM_SEEK_CUR) == inputSkipBytes);

                    ptrdiff_t BytesRead = input_stream->read(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(staging_upload_buffer_base) + subresource_memcpy_dests[dstSubresource].staging_upload_buffer_offset), inputSliceSize * inputNumSlices);
                    if (BytesRead == -1 || static_cast<size_t>(BytesRead) < (inputSliceSize * inputNumSlices))
                    {
                        return false;
                    }
                }
                else if (inputRowSize == subresource_memcpy_dests[dstSubresource].output_row_pitch)
                {
                    assert(inputSliceSize <= subresource_memcpy_dests[dstSubresource].output_slice_pitch);

                    for (size_t z = 0; z < inputNumSlices; ++z)
                    {
                        assert(input_stream->seek(0, LOAD_ASSET_INPUT_STREAM_SEEK_CUR) == (inputSkipBytes + inputSliceSize * z));

                        ptrdiff_t BytesRead = input_stream->read(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(staging_upload_buffer_base) + (subresource_memcpy_dests[dstSubresource].staging_upload_buffer_offset + subresource_memcpy_dests[dstSubresource].output_slice_pitch * z)), inputSliceSize);
                        if (BytesRead == -1 || static_cast<size_t>(BytesRead) < inputSliceSize)
                        {
                            return false;
                        }
                    }
                }
                else
                {
                    assert(inputSliceSize <= subresource_memcpy_dests[dstSubresource].output_slice_pitch);
                    assert(inputRowSize <= subresource_memcpy_dests[dstSubresource].output_row_pitch);

                    for (size_t z = 0; z < inputNumSlices; ++z)
                    {
                        for (size_t y = 0; y < inputNumRows; ++y)
                        {
                            assert(input_stream->seek(0, LOAD_ASSET_INPUT_STREAM_SEEK_CUR) == (inputSkipBytes + inputSliceSize * z + inputRowSize * y));

                            ptrdiff_t BytesRead = input_stream->read(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(staging_upload_buffer_base) + (subresource_memcpy_dests[dstSubresource].staging_upload_buffer_offset + subresource_memcpy_dests[dstSubresource].output_slice_pitch * z + subresource_memcpy_dests[dstSubresource].output_row_pitch * y)), inputRowSize);
                            if (BytesRead == -1 || static_cast<size_t>(BytesRead) < inputRowSize)
                            {
                                return false;
                            }
                        }
                    }
                }
            }

            inputSkipBytes += inputSliceSize * inputNumSlices;
        }
    }

    uint8_t u_assert_only[1];
    assert(input_stream->read(u_assert_only, sizeof(uint8_t)) == 0);
    return true;
}

//--------------------------------------------------------------------------------------
static inline uint32_t Pvr_GetPixelFormatPartHigh(uint64_t pixelFormat)
{
    return (pixelFormat >> 32);
}

static inline bool Pvr_GetMinDimensionsForFormat(uint64_t pixelFormat, uint32_t *minX, uint32_t *minY, uint32_t *minZ)
{
    assert(NULL != minX);
    assert(NULL != minY);
    assert(NULL != minZ);

    if (0U == Pvr_GetPixelFormatPartHigh(pixelFormat))
    {
        switch (pixelFormat)
        {
        case Pvr_PixelTypeID_DXT1:
        case Pvr_PixelTypeID_DXT2:
        case Pvr_PixelTypeID_DXT3:
        case Pvr_PixelTypeID_DXT4:
        case Pvr_PixelTypeID_DXT5:
        case Pvr_PixelTypeID_BC4:
        case Pvr_PixelTypeID_BC5:
        case Pvr_PixelTypeID_ETC1:
        case Pvr_PixelTypeID_ETC2_RGB:
        case Pvr_PixelTypeID_ETC2_RGBA:
        case Pvr_PixelTypeID_ETC2_RGB_A1:
        case Pvr_PixelTypeID_EAC_R11:
        case Pvr_PixelTypeID_EAC_RG11:
            (*minX) = 4U;
            (*minY) = 4U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_PVRTCI_4bpp_RGB:
        case Pvr_PixelTypeID_PVRTCI_4bpp_RGBA:
            (*minX) = 8U;
            (*minY) = 8U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_PVRTCI_2bpp_RGB:
        case Pvr_PixelTypeID_PVRTCI_2bpp_RGBA:
            (*minX) = 16U;
            (*minY) = 8U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_PVRTCII_4bpp:
            (*minX) = 4U;
            (*minY) = 4U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_PVRTCII_2bpp:
            (*minX) = 8U;
            (*minY) = 4U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_UYVY:
        case Pvr_PixelTypeID_YUY2:
        case Pvr_PixelTypeID_RGBG8888:
        case Pvr_PixelTypeID_GRGB8888:
            (*minX) = 2U;
            (*minY) = 1U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_BW1bpp:
            (*minX) = 8U;
            (*minY) = 1U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_4x4:
            (*minX) = 4U;
            (*minY) = 4U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_5x4:
            (*minX) = 5U;
            (*minY) = 4U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_5x5:
            (*minX) = 5U;
            (*minY) = 5U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_6x5:
            (*minX) = 6U;
            (*minY) = 5U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_6x6:
            (*minX) = 6U;
            (*minY) = 6U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_8x5:
            (*minX) = 8U;
            (*minY) = 5U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_8x6:
            (*minX) = 8U;
            (*minY) = 6U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_8x8:
            (*minX) = 8U;
            (*minY) = 8U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_10x5:
            (*minX) = 10U;
            (*minY) = 5U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_10x6:
            (*minX) = 10U;
            (*minY) = 6U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_10x8:
            (*minX) = 10U;
            (*minY) = 8U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_10x10:
            (*minX) = 10U;
            (*minY) = 10U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_12x10:
            (*minX) = 12U;
            (*minY) = 10U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_12x12:
            (*minX) = 12U;
            (*minY) = 12U;
            (*minZ) = 1U;
            return true;
        case Pvr_PixelTypeID_ASTC_3x3x3:
            (*minX) = 3U;
            (*minY) = 3U;
            (*minZ) = 3U;
            return true;
        case Pvr_PixelTypeID_ASTC_4x3x3:
            (*minX) = 4U;
            (*minY) = 3U;
            (*minZ) = 3U;
            return true;
        case Pvr_PixelTypeID_ASTC_4x4x3:
            (*minX) = 4U;
            (*minY) = 4U;
            (*minZ) = 3U;
            return true;
        case Pvr_PixelTypeID_ASTC_4x4x4:
            (*minX) = 4U;
            (*minY) = 4U;
            (*minZ) = 4U;
            return true;
        case Pvr_PixelTypeID_ASTC_5x4x4:
            (*minX) = 5U;
            (*minY) = 4U;
            (*minZ) = 4U;
            return true;
        case Pvr_PixelTypeID_ASTC_5x5x4:
            (*minX) = 5U;
            (*minY) = 5U;
            (*minZ) = 4U;
            return true;
        case Pvr_PixelTypeID_ASTC_5x5x5:
            (*minX) = 5U;
            (*minY) = 5U;
            (*minZ) = 5U;
            return true;
        case Pvr_PixelTypeID_ASTC_6x5x5:
            (*minX) = 6U;
            (*minY) = 5U;
            (*minZ) = 5U;
            return true;
        case Pvr_PixelTypeID_ASTC_6x6x5:
            (*minX) = 6U;
            (*minY) = 6U;
            (*minZ) = 5U;
            return true;
        case Pvr_PixelTypeID_ASTC_6x6x6:
            (*minX) = 6U;
            (*minY) = 6U;
            (*minZ) = 6U;
            return true;
        default:
            return false;
        }
    }
    else
    {
        (*minX) = 1U;
        (*minY) = 1U;
        (*minZ) = 1U;
        return true;
    }
}

static inline uint32_t Pvr_GetBitsPerPixel(uint64_t pixelFormat)
{
    if (0U == Pvr_GetPixelFormatPartHigh(pixelFormat))
    {
        switch (pixelFormat)
        {
        case Pvr_PixelTypeID_BW1bpp:
            return 1U;
        case Pvr_PixelTypeID_PVRTCI_2bpp_RGB:
        case Pvr_PixelTypeID_PVRTCI_2bpp_RGBA:
        case Pvr_PixelTypeID_PVRTCII_2bpp:
            return 2U;
        case Pvr_PixelTypeID_PVRTCI_4bpp_RGB:
        case Pvr_PixelTypeID_PVRTCI_4bpp_RGBA:
        case Pvr_PixelTypeID_PVRTCII_4bpp:
        case Pvr_PixelTypeID_ETC1:
        case Pvr_PixelTypeID_EAC_R11:
        case Pvr_PixelTypeID_ETC2_RGB:
        case Pvr_PixelTypeID_ETC2_RGB_A1:
        case Pvr_PixelTypeID_DXT1:
        case Pvr_PixelTypeID_BC4:
            return 4U;
        case Pvr_PixelTypeID_DXT2:
        case Pvr_PixelTypeID_DXT3:
        case Pvr_PixelTypeID_DXT4:
        case Pvr_PixelTypeID_DXT5:
        case Pvr_PixelTypeID_BC5:
        case Pvr_PixelTypeID_EAC_RG11:
        case Pvr_PixelTypeID_ETC2_RGBA:
            return 8U;
        case Pvr_PixelTypeID_YUY2:
        case Pvr_PixelTypeID_UYVY:
        case Pvr_PixelTypeID_RGBG8888:
        case Pvr_PixelTypeID_GRGB8888:
            return 16U;
        case Pvr_PixelTypeID_SharedExponentR9G9B9E5:
            return 32U;
        default:
            return -1;
        }
    }
    else
    {
        return reinterpret_cast<char *>(&pixelFormat)[4] + reinterpret_cast<char *>(&pixelFormat)[5] + reinterpret_cast<char *>(&pixelFormat)[6] + reinterpret_cast<char *>(&pixelFormat)[7];
    }
}

static inline uint32_t pvr_get_format_plane_count(uint64_t pixelFormat)
{
    if (0U == Pvr_GetPixelFormatPartHigh(pixelFormat))
    {
        static uint32_t const pvr_compressed_format_plane_count_table[] = {
            1, // Pvr_PixelTypeID_PVRTCI_2bpp_RGB
            1, // Pvr_PixelTypeID_PVRTCI_2bpp_RGBA
            1, // Pvr_PixelTypeID_PVRTCI_4bpp_RGB
            1, // Pvr_PixelTypeID_PVRTCI_4bpp_RGBA
            1, // Pvr_PixelTypeID_PVRTCII_2bpp
            1, // Pvr_PixelTypeID_PVRTCII_4bpp
            1, // Pvr_PixelTypeID_ETC1
            1, // Pvr_PixelTypeID_DXT1
            1, // Pvr_PixelTypeID_DXT2
            1, // Pvr_PixelTypeID_DXT3
            1, // Pvr_PixelTypeID_DXT4
            1, // Pvr_PixelTypeID_DXT5
            1, // Pvr_PixelTypeID_BC4
            1, // Pvr_PixelTypeID_BC5
            1, // Pvr_PixelTypeID_BC6
            1, // Pvr_PixelTypeID_BC7
            1, // Pvr_PixelTypeID_UYVY
            1, // Pvr_PixelTypeID_YUY2
            1, // Pvr_PixelTypeID_BW1bpp
            1, // Pvr_PixelTypeID_SharedExponentR9G9B9E5,
            1, // Pvr_PixelTypeID_RGBG8888
            1, // Pvr_PixelTypeID_GRGB8888
            1, // Pvr_PixelTypeID_ETC2_RGB
            1, // Pvr_PixelTypeID_ETC2_RGBA
            1, // Pvr_PixelTypeID_ETC2_RGB_A1
            1, // Pvr_PixelTypeID_EAC_R11
            1, // Pvr_PixelTypeID_EAC_RG11
            1, // Pvr_PixelTypeID_ASTC_4x4
            1, // Pvr_PixelTypeID_ASTC_5x4
            1, // Pvr_PixelTypeID_ASTC_5x5
            1, // Pvr_PixelTypeID_ASTC_6x5
            1, // Pvr_PixelTypeID_ASTC_6x6
            1, // Pvr_PixelTypeID_ASTC_8x5
            1, // Pvr_PixelTypeID_ASTC_8x6
            1, // Pvr_PixelTypeID_ASTC_8x8
            1, // Pvr_PixelTypeID_ASTC_10x5
            1, // Pvr_PixelTypeID_ASTC_10x6
            1, // Pvr_PixelTypeID_ASTC_10x8
            1, // Pvr_PixelTypeID_ASTC_10x10
            1, // Pvr_PixelTypeID_ASTC_12x10
            1, // Pvr_PixelTypeID_ASTC_12x12
            1, // Pvr_PixelTypeID_ASTC_3x3x3
            1, // Pvr_PixelTypeID_ASTC_4x3x3
            1, // Pvr_PixelTypeID_ASTC_4x4x3
            1, // Pvr_PixelTypeID_ASTC_4x4x4
            1, // Pvr_PixelTypeID_ASTC_5x4x4
            1, // Pvr_PixelTypeID_ASTC_5x5x4
            1, // Pvr_PixelTypeID_ASTC_5x5x5
            1, // Pvr_PixelTypeID_ASTC_6x5x5
            1, // Pvr_PixelTypeID_ASTC_6x6x5
            1  // Pvr_PixelTypeID_ASTC_6x6x6
        };

        return pvr_compressed_format_plane_count_table[pixelFormat];
    }
    else
    {
        switch (pixelFormat)
        {
        case Pvr_PixelTypeChar_I8:
        case Pvr_PixelTypeChar_R8:
        case Pvr_PixelTypeChar_R8G8:
        case Pvr_PixelTypeChar_R8G8B8:
        case Pvr_PixelTypeChar_B8G8R8:
        case Pvr_PixelTypeChar_R8G8B8A8:
        case Pvr_PixelTypeChar_B8G8R8A8:
        case Pvr_PixelTypeChar_A8B8G8R8:
        case Pvr_PixelTypeChar_R16:
        case Pvr_PixelTypeChar_R16G16:
        case Pvr_PixelTypeChar_L16A16:
        case Pvr_PixelTypeChar_R16G16B16:
        case Pvr_PixelTypeChar_R16G16B16A16:
        case Pvr_PixelTypeChar_R32:
        case Pvr_PixelTypeChar_L32:
        case Pvr_PixelTypeChar_R32G32:
        case Pvr_PixelTypeChar_L32A32:
        case Pvr_PixelTypeChar_R32G32B32:
        case Pvr_PixelTypeChar_R32G32B32A32:
        case Pvr_PixelTypeChar_R5G6B5:
        case Pvr_PixelTypeChar_R4G4B4A4:
        case Pvr_PixelTypeChar_R5G5B5A1:
        case Pvr_PixelTypeChar_R11G11B10:
        case Pvr_PixelTypeChar_D8:
        case Pvr_PixelTypeChar_S8:
        case Pvr_PixelTypeChar_D16:
        case Pvr_PixelTypeChar_D24:
        case Pvr_PixelTypeChar_D32:
            return 1U;
        case Pvr_PixelTypeChar_D16S8:
        case Pvr_PixelTypeChar_D24S8:
        case Pvr_PixelTypeChar_D32S8:
            return 2U;
        default:
            return 0U;
        }
    }
}

static inline bool pvr_is_depth_stencil(uint64_t pixelFormat)
{
    switch (pixelFormat)
    {
    case Pvr_PixelTypeChar_D32S8:
    case Pvr_PixelTypeChar_D32:
    case Pvr_PixelTypeChar_D24S8:
    case Pvr_PixelTypeChar_D24:
    case Pvr_PixelTypeChar_D16S8:
    case Pvr_PixelTypeChar_D16:
    case Pvr_PixelTypeChar_S8:
    case Pvr_PixelTypeChar_D8:
        return true;
    default:
        return false;
    }
}
