#pragma once
#include <Tera/Core.h>
#if TEXTURE_UTILS
#include <Tera/FString.h>
#include <Tera/FStream.h>

#include <array>
#include <filesystem>

#define DDS_MAGIC 0x20534444

namespace DDS
{
  enum FCC : uint32 {
    FCC_None = 0,
    FCC_DXT1 = 0x31545844,
    FCC_DXT3 = 0x33545844,
    FCC_DXT5 = 0x35545844,
    FCC_DX10 = 0x30315844
  };

  enum DXGI_FORMAT : uint32 {
    DXGI_FORMAT_UNKNOWN,
    DXGI_FORMAT_R32G32B32A32_TYPELESS,
    DXGI_FORMAT_R32G32B32A32_FLOAT,
    DXGI_FORMAT_R32G32B32A32_UINT,
    DXGI_FORMAT_R32G32B32A32_SINT,
    DXGI_FORMAT_R32G32B32_TYPELESS,
    DXGI_FORMAT_R32G32B32_FLOAT,
    DXGI_FORMAT_R32G32B32_UINT,
    DXGI_FORMAT_R32G32B32_SINT,
    DXGI_FORMAT_R16G16B16A16_TYPELESS,
    DXGI_FORMAT_R16G16B16A16_FLOAT,
    DXGI_FORMAT_R16G16B16A16_UNORM,
    DXGI_FORMAT_R16G16B16A16_UINT,
    DXGI_FORMAT_R16G16B16A16_SNORM,
    DXGI_FORMAT_R16G16B16A16_SINT,
    DXGI_FORMAT_R32G32_TYPELESS,
    DXGI_FORMAT_R32G32_FLOAT,
    DXGI_FORMAT_R32G32_UINT,
    DXGI_FORMAT_R32G32_SINT,
    DXGI_FORMAT_R32G8X24_TYPELESS,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
    DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS,
    DXGI_FORMAT_X32_TYPELESS_G8X24_UINT,
    DXGI_FORMAT_R10G10B10A2_TYPELESS,
    DXGI_FORMAT_R10G10B10A2_UNORM,
    DXGI_FORMAT_R10G10B10A2_UINT,
    DXGI_FORMAT_R11G11B10_FLOAT,
    DXGI_FORMAT_R8G8B8A8_TYPELESS,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
    DXGI_FORMAT_R8G8B8A8_UINT,
    DXGI_FORMAT_R8G8B8A8_SNORM,
    DXGI_FORMAT_R8G8B8A8_SINT,
    DXGI_FORMAT_R16G16_TYPELESS,
    DXGI_FORMAT_R16G16_FLOAT,
    DXGI_FORMAT_R16G16_UNORM,
    DXGI_FORMAT_R16G16_UINT,
    DXGI_FORMAT_R16G16_SNORM,
    DXGI_FORMAT_R16G16_SINT,
    DXGI_FORMAT_R32_TYPELESS,
    DXGI_FORMAT_D32_FLOAT,
    DXGI_FORMAT_R32_FLOAT,
    DXGI_FORMAT_R32_UINT,
    DXGI_FORMAT_R32_SINT,
    DXGI_FORMAT_R24G8_TYPELESS,
    DXGI_FORMAT_D24_UNORM_S8_UINT,
    DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
    DXGI_FORMAT_X24_TYPELESS_G8_UINT,
    DXGI_FORMAT_R8G8_TYPELESS,
    DXGI_FORMAT_R8G8_UNORM,
    DXGI_FORMAT_R8G8_UINT,
    DXGI_FORMAT_R8G8_SNORM,
    DXGI_FORMAT_R8G8_SINT,
    DXGI_FORMAT_R16_TYPELESS,
    DXGI_FORMAT_R16_FLOAT,
    DXGI_FORMAT_D16_UNORM,
    DXGI_FORMAT_R16_UNORM,
    DXGI_FORMAT_R16_UINT,
    DXGI_FORMAT_R16_SNORM,
    DXGI_FORMAT_R16_SINT,
    DXGI_FORMAT_R8_TYPELESS,
    DXGI_FORMAT_R8_UNORM,
    DXGI_FORMAT_R8_UINT,
    DXGI_FORMAT_R8_SNORM,
    DXGI_FORMAT_R8_SINT,
    DXGI_FORMAT_A8_UNORM,
    DXGI_FORMAT_R1_UNORM,
    DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
    DXGI_FORMAT_R8G8_B8G8_UNORM,
    DXGI_FORMAT_G8R8_G8B8_UNORM,
    DXGI_FORMAT_BC1_TYPELESS,
    DXGI_FORMAT_BC1_UNORM,
    DXGI_FORMAT_BC1_UNORM_SRGB,
    DXGI_FORMAT_BC2_TYPELESS,
    DXGI_FORMAT_BC2_UNORM,
    DXGI_FORMAT_BC2_UNORM_SRGB,
    DXGI_FORMAT_BC3_TYPELESS,
    DXGI_FORMAT_BC3_UNORM,
    DXGI_FORMAT_BC3_UNORM_SRGB,
    DXGI_FORMAT_BC4_TYPELESS,
    DXGI_FORMAT_BC4_UNORM,
    DXGI_FORMAT_BC4_SNORM,
    DXGI_FORMAT_BC5_TYPELESS,
    DXGI_FORMAT_BC5_UNORM,
    DXGI_FORMAT_BC5_SNORM,
    DXGI_FORMAT_B5G6R5_UNORM,
    DXGI_FORMAT_B5G5R5A1_UNORM,
    DXGI_FORMAT_B8G8R8A8_UNORM,
    DXGI_FORMAT_B8G8R8X8_UNORM,
    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM,
    DXGI_FORMAT_B8G8R8A8_TYPELESS,
    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
    DXGI_FORMAT_B8G8R8X8_TYPELESS,
    DXGI_FORMAT_B8G8R8X8_UNORM_SRGB,
    DXGI_FORMAT_BC6H_TYPELESS,
    DXGI_FORMAT_BC6H_UF16,
    DXGI_FORMAT_BC6H_SF16,
    DXGI_FORMAT_BC7_TYPELESS,
    DXGI_FORMAT_BC7_UNORM,
    DXGI_FORMAT_BC7_UNORM_SRGB,
    DXGI_FORMAT_AYUV,
    DXGI_FORMAT_Y410,
    DXGI_FORMAT_Y416,
    DXGI_FORMAT_NV12,
    DXGI_FORMAT_P010,
    DXGI_FORMAT_P016,
    DXGI_FORMAT_420_OPAQUE,
    DXGI_FORMAT_YUY2,
    DXGI_FORMAT_Y210,
    DXGI_FORMAT_Y216,
    DXGI_FORMAT_NV11,
    DXGI_FORMAT_AI44,
    DXGI_FORMAT_IA44,
    DXGI_FORMAT_P8,
    DXGI_FORMAT_A8P8,
    DXGI_FORMAT_B4G4R4A4_UNORM,
    DXGI_FORMAT_P208,
    DXGI_FORMAT_V208,
    DXGI_FORMAT_V408,
    DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE,
    DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE,
    DXGI_FORMAT_FORCE_UINT
  };

  enum DDSD : uint32 {
    DDSD_NONE = 0,
    DDSD_CAPS = 0x00000001,
    DDSD_HEIGHT = 0x00000002,
    DDSD_WIDTH = 0x00000004,
    DDSD_PITCH = 0x00000008,
    DDSD_BACKBUFFERCOUNT = 0x00000020,
    DDSD_ZBUFFERBITDEPTH = 0x00000040,
    DDSD_ALPHABITDEPTH = 0x00000080,
    DDSD_LPSURFACE = 0x00000800,
    DDSD_PIXELFORMAT = 0x00001000,
    DDSD_CKDESTOVERLAY = 0x00002000,
    DDSD_CKDESTBLT = 0x00004000,
    DDSD_CKSRCOVERLAY = 0x00008000,
    DDSD_CKSRCBLT = 0x00010000,
    DDSD_MIPMAPCOUNT = 0x00020000,
    DDSD_REFRESHRATE = 0x00040000,
    DDSD_LINEARSIZE = 0x00080000,
    DDSD_TEXTURESTAGE = 0x00100000,
    DDSD_FVF = 0x00200000,
    DDSD_SRCVBHANDLE = 0x00400000,
    DDSD_DEPTH = 0x00800000,
    DDSD_ALL = 0x00fff9ee
  };

  enum DDPF : uint32 {
    DDPF_NONE = 0,
    DDPF_ALPHAPIXELS = 0x00000001,
    DDPF_ALPHA = 0x00000002,
    DDPF_FOURCC = 0x00000004,
    DDPF_PALETTEINDEXED4 = 0x00000008,
    DDPF_PALETTEINDEXEDTO8 = 0x00000010,
    DDPF_PALETTEINDEXED8 = 0x00000020,
    DDPF_RGB = 0x00000040,
    DDPF_COMPRESSED = 0x00000080,
    DDPF_RGBTOYUV = 0x00000100,
    DDPF_YUV = 0x00000200,
    DDPF_ZBUFFER = 0x00000400,
    DDPF_PALETTEINDEXED1 = 0x00000800,
    DDPF_PALETTEINDEXED2 = 0x00001000,
    DDPF_ZPIXELS = 0x00002000,
    DDPF_STENCILBUFFER = 0x00004000,
    DDPF_ALPHAPREMULT = 0x00008000,
    DDPF_LUMINANCE = 0x00020000,
    DDPF_BUMPLUMINANCE = 0x00040000,
    DDPF_BUMPDUDV = 0x00080000
  };

  enum DDSCAPS : uint32
  {
    DDSCAPS_NONE = 0,
    DDSCAPS_RESERVED1 = 0x00000001,
    DDSCAPS_ALPHA = 0x00000002,
    DDSCAPS_BACKBUFFER = 0x00000004,
    DDSCAPS_COMPLEX = 0x00000008,
    DDSCAPS_FLIP = 0x00000010,
    DDSCAPS_FRONTBUFFER = 0x00000020,
    DDSCAPS_OFFSCREENPLAIN = 0x00000040,
    DDSCAPS_OVERLAY = 0x00000080,
    DDSCAPS_PALETTE = 0x00000100,
    DDSCAPS_PRIMARYSURFACE = 0x00000200,
    DDSCAPS_RESERVED3 = 0x00000400,
    DDSCAPS_SYSTEMMEMORY = 0x00000800,
    DDSCAPS_TEXTURE = 0x00001000,
    DDSCAPS_3DDEVICE = 0x00002000,
    DDSCAPS_VIDEOMEMORY = 0x00004000,
    DDSCAPS_VISIBLE = 0x00008000,
    DDSCAPS_WRITEONLY = 0x00010000,
    DDSCAPS_ZBUFFER = 0x00020000,
    DDSCAPS_OWNDC = 0x00040000,
    DDSCAPS_LIVEVIDEO = 0x00080000,
    DDSCAPS_HWCODEC = 0x00100000,
    DDSCAPS_MODEX = 0x00200000,
    DDSCAPS_MIPMAP = 0x00400000,
    DDSCAPS_RESERVED2 = 0x00800000,
    DDSCAPS_ALLOCONLOAD = 0x04000000,
    DDSCAPS_VIDEOPORT = 0x08000000,
    DDSCAPS_LOCALVIDMEM = 0x10000000,
    DDSCAPS_NONLOCALVIDMEM = 0x20000000,
    DDSCAPS_STANDARDVGAMODE = 0x40000000,
    DDSCAPS_OPTIMIZED = 0x80000000
  };

  enum DDSCAPS2 : uint32 {
    DDSCAPS2_NONE = 0,
    DDSCAPS2_RESERVED4 = 0x00000002,
    DDSCAPS2_HINTDYNAMIC = 0x00000004,
    DDSCAPS2_HINTSTATIC = 0x00000008,
    DDSCAPS2_TEXTUREMANAGE = 0x00000010,
    DDSCAPS2_RESERVED1 = 0x00000020,
    DDSCAPS2_RESERVED2 = 0x00000040,
    DDSCAPS2_OPAQUE = 0x00000080,
    DDSCAPS2_HINTANTIALIASING = 0x00000100,
    DDSCAPS2_CUBEMAP = 0x00000200,
    DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400,
    DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800,
    DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000,
    DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000,
    DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000,
    DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000,
    DDSCAPS2_CUBEMAP_ALLFACES = 0x0000fc00,
    DDSCAPS2_MIPMAPSUBLEVEL = 0x00010000,
    DDSCAPS2_D3DTEXTUREMANAGE = 0x00020000,
    DDSCAPS2_DONOTPERSIST = 0x00040000,
    DDSCAPS2_STEREOSURFACELEFT = 0x00080000,
    DDSCAPS2_VOLUME = 0x00200000,
    DDSCAPS2_NOTUSERLOCKABLE = 0x00400000,
    DDSCAPS2_POINTS = 0x00800000,
    DDSCAPS2_RTPATCHES = 0x01000000,
    DDSCAPS2_NPATCHES = 0x02000000,
    DDSCAPS2_RESERVED3 = 0x04000000,
    DDSCAPS2_DISCARDBACKBUFFER = 0x10000000,
    DDSCAPS2_ENABLEALPHACHANNEL = 0x20000000,
    DDSCAPS2_EXTENDEDFORMATPRIMARY = 0x40000000,
    DDSCAPS2_ADDITIONALPRIMARY = 0x80000000
  };

  struct DDS_PIXELFORMAT {
    uint32 dwSize = sizeof(DDS_PIXELFORMAT);
    uint32 dwFlags = DDPF_FOURCC;
    uint32 dwFourCC = FCC_DX10;
    uint32 dwRGBBitCount = 0;
    uint32 dwRBitMask = 0;
    uint32 dwGBitMask = 0;
    uint32 dwBBitMask = 0;
    uint32 dwABitMask = 0;
  };

  struct DDS_HEADER_D3D9 {
    uint32 dwSize = sizeof(DDS_HEADER_D3D9);
    uint32 dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_MIPMAPCOUNT | DDSD_LINEARSIZE;
    uint32 dwHeight = 0;
    uint32 dwWidth = 0;
    uint32 dwPitchOrLinearSize = 0;
    uint32 dwDepth = 1;
    uint32 dwMipMapCount = 1;
    uint32 dwReserved1[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    DDS_PIXELFORMAT ddspf;
    uint32 dwCaps = DDSCAPS_COMPLEX | DDSCAPS_TEXTURE | DDSCAPS_OWNDC;
    uint32 dwCaps2 = 0;
    uint32 dwCaps3 = 0;
    uint32 dwCaps4 = 0;
    uint32 dwReserved2 = 0;
  };

  enum D3D10_RESOURCE_DIMENSION : uint32 {
    D3D10_RESOURCE_DIMENSION_UNKNOWN,
    D3D10_RESOURCE_DIMENSION_BUFFER,
    D3D10_RESOURCE_DIMENSION_TEXTURE1D,
    D3D10_RESOURCE_DIMENSION_TEXTURE2D,
    D3D10_RESOURCE_DIMENSION_TEXTURE3D
  };

  enum D3D10_MISC_FLAG : uint32 {
    DDS_MISC_TEXTURECUBE = 0x4,
  };

  struct DDS_HEADER_D3D10 {
    DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
    D3D10_RESOURCE_DIMENSION resourceDimension = D3D10_RESOURCE_DIMENSION_TEXTURE2D;
    uint32 miscFlag = 0;
    uint32 arraySize = 1;
    uint32 miscFlags2 = 1;
  };

  struct DDSHeader {
    uint32 dwMagic = DDS_MAGIC;

    DDS_HEADER_D3D9 D3D9;
    DDS_HEADER_D3D10 D3D10;

    FILE_OFFSET DataOffset = 0;

    friend FStream& operator<<(FStream& s, DDSHeader& h)
    {
      s << h.dwMagic;
      if (h.dwMagic != DDS_MAGIC)
      {
        UThrow("Not a DDS file!");
      }
      s.SerializeBytes(&h.D3D9, sizeof(DDS_HEADER_D3D9));
      if (h.D3D9.dwSize != sizeof(DDS_HEADER_D3D9))
      {
        UThrow("The DDS file is invalid!\nSize field does not match the spec!");
      }
      if (h.D3D9.dwFlags & DDPF_FOURCC && h.D3D9.ddspf.dwFourCC == FCC_DX10)
      {
        s.SerializeBytes(&h.D3D10, sizeof(DDS_HEADER_D3D10));
      }
      h.DataOffset = s.GetPosition();
      return s;
    }

    // Calculate size of the first mipmap
    FILE_OFFSET CalculateMipmapSize() const
    {
      if (D3D9.ddspf.dwFlags & DDPF_RGB || D3D9.ddspf.dwFlags & DDPF_LUMINANCE)
      {
        return (D3D9.dwWidth * D3D9.ddspf.dwRGBBitCount + 7) / 8 * D3D9.dwHeight;
      }
      const FILE_OFFSET blockSize = 4;
      const FILE_OFFSET blockBytes = GetPixelFormat() == PF_DXT1 ? 8 : 16;
      return (D3D9.dwWidth / blockSize) * (D3D9.dwHeight / blockSize) * blockBytes;
    }

    int32 GetWidth() const
    {
      return D3D9.dwWidth;
    }

    int32 GetHeight() const
    {
      return D3D9.dwHeight;
    }

    EPixelFormat GetPixelFormat() const
    {
      if (D3D9.ddspf.dwFlags & DDPF_RGB)
      {
        if (D3D9.ddspf.dwRGBBitCount == 32 &&
          D3D9.ddspf.dwRBitMask == 0x00ff0000 &&
          D3D9.ddspf.dwGBitMask == 0x0000ff00 &&
          D3D9.ddspf.dwBBitMask == 0x000000ff &&
          D3D9.ddspf.dwABitMask == 0xff000000)
        {
          return PF_A8R8G8B8;
        }
      }
      if (D3D9.ddspf.dwFlags & DDPF_LUMINANCE)
      {
        if (D3D9.ddspf.dwRGBBitCount == 8)
        {
          return PF_G8;
        }
      }
      if (D3D9.dwFlags & DDPF_FOURCC)
      {
        if (D3D9.ddspf.dwFourCC == FCC_DX10)
        {
          switch (D3D10.dxgiFormat)
          {
          case DXGI_FORMAT_BC1_TYPELESS:
          case DXGI_FORMAT_BC1_UNORM:
          case DXGI_FORMAT_BC1_UNORM_SRGB:
            return PF_DXT1;
          case DXGI_FORMAT_BC2_TYPELESS:
          case DXGI_FORMAT_BC2_UNORM:
          case DXGI_FORMAT_BC2_UNORM_SRGB:
            return PF_DXT3;
          case DXGI_FORMAT_BC3_TYPELESS:
          case DXGI_FORMAT_BC3_UNORM:
          case DXGI_FORMAT_BC3_UNORM_SRGB:
            return PF_DXT5;
          case DXGI_FORMAT_B8G8R8A8_UNORM:
          case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
          case DXGI_FORMAT_B8G8R8A8_TYPELESS:
            return PF_A8R8G8B8;
          case DXGI_FORMAT_R8_TYPELESS:
          case DXGI_FORMAT_R8_UNORM:
          case DXGI_FORMAT_A8_UNORM:
            return PF_G8;
          }
        }
        else
        {
          switch (D3D9.ddspf.dwFourCC)
          {
          case FCC_DXT1:
            return PF_DXT1;
          case FCC_DXT3:
            return PF_DXT3;
          case FCC_DXT5:
            return PF_DXT5;
          }
        }
      }
      return PF_Unknown;
    }
  };
}

class TextureProcessor {
public:
  enum class TCFormat {
    None = 0,
    DXT,
    DXT1,
    DXT3,
    DXT5,
    ARGB8,
    G8,
    G16,
    PNG,
    TGA,
    DDS
  };

  static TCFormat GetTcFormatByExtension(const FString& ext)
  {
    if (ext == "dds" || ext == ".dds")
    {
      return TCFormat::DDS;
    }
    if (ext == "tga" || ext == ".tga")
    {
      return TCFormat::TGA;
    }
    if (ext == "png" || ext == ".png")
    {
      return TCFormat::PNG;
    }
    return HasAVX2() ? TCFormat::TGA : TCFormat::DDS;
  }

  TextureProcessor(TCFormat from, TCFormat to)
    : InputFormat(from)
    , OutputFormat(to)
  {}

  ~TextureProcessor()
  {
    free(InputData);
    free(OutputData);
    for (auto& cube : InputCube)
    {
      free(cube.Data);
    }
  }

  inline void SetInputCubeFace(int32 faceIdx, void* data, int32 size, int32 width, int32 height)
  {
    InIsCube = data;
    if (InputCube[faceIdx].Data)
    {
      free(InputCube[faceIdx].Data);
      InputCube[faceIdx].Data = nullptr;
    }
    if (size && data)
    {
      InputCube[faceIdx].Data = malloc(size);
      memcpy(InputCube[faceIdx].Data, data, size);
    }
    InputCube[faceIdx].Size = size;
    InputCube[faceIdx].X = width;
    InputCube[faceIdx].Y = height;
  }

  inline void SetInputData(void* data, int32 size)
  {
    if (InputData)
    {
      free(InputData);
      InputData = nullptr;
    }
    if (size && data)
    {
      InputData = malloc(size);
      memcpy(InputData, data, size);
    }
    InputDataSize = size;
  }

  inline void SetInputDataDimensions(int32 sizeX, int32 sizeY)
  {
    InputDataSizeX = sizeX;
    InputDataSizeY = sizeY;
  }

  inline void SetInputPath(const std::string& inputPath)
  {
    InputPath = inputPath;
  }

  inline void SetOutputPath(const std::string& outputPath)
  {
    static const std::vector<wchar> forbiddenChars = { L'\\', L'/', L':', L'*', L'?', L'\"', L'<', L'>', L'|' };
    std::filesystem::path tmp(A2W(outputPath));
    std::wstring name = tmp.filename();
    for (size_t pos = 0; pos < name.size(); ++pos)
    {
      if (std::find(forbiddenChars.begin(), forbiddenChars.end(), name[pos]) != forbiddenChars.end())
      {
        name[pos] = L'_';
      }
    }
    tmp.replace_filename(name);
    OutputPath = W2A(tmp.wstring());
  }

  inline std::string GetError() const
  {
    return Error;
  }

  inline void SetSrgb(bool srgb)
  {
    SRGB = srgb;
  }

  inline void SetNormal(bool normal)
  {
    Normal = normal;
  }

  inline void SetAddressX(TextureAddress x)
  {
    AddressX = x;
  }

  inline void SetAddressY(TextureAddress y)
  {
    AddressY = y;
  }

  inline void SetGenerateMips(bool generate)
  {
    GenerateMips = generate;
  }

  inline void SetMipFilter(MipFilterType filter)
  {
    MipFilter = filter;
  }

  inline bool GetAlpha() const
  {
    return Alpha;
  }

  inline void SetOutputFormat(TCFormat to)
  {
    OutputFormat = to;
  }

  inline void SetSplitAlpha(bool flag)
  {
    SplitAlpha = flag;
  }

  inline void ClearOutput()
  {
    if (OutputData)
    {
      free(OutputData);
    }
    OutputMips.clear();
  }

  bool Process();

  struct OutputMip {
    int32 SizeX = 0;
    int32 SizeY = 0;
    int32 Size = 0;
    void* Data = nullptr;
  };

  struct Cube {
    void* Data = nullptr;
    int32 Size = 0;
    int32 X = 0;
    int32 Y = 0;
  };

  inline const std::vector<OutputMip>& GetOutputMips() const
  {
    return OutputMips;
  }

  TCFormat GetOutputFormat() const
  {
    return OutputFormat;
  }

  bool UnfoldCube();

private:
  bool BytesToFile();
  bool BytesToDDS();
  bool BytesToBytes();
  bool FileToBytes();
  bool DDSToBytes();

  inline bool InputCubeIsValid() const
  {
    int32 x = InputCube[0].X;
    int32 y = InputCube[0].Y;
    for (const Cube& c : InputCube)
    {
      if (!c.Data || !c.Size || x != c.X || y != c.Y || !x || !y)
      {
        return false;
      }
    }
    return true;
  }

private:
  TCFormat InputFormat = TCFormat::None;
  TCFormat OutputFormat = TCFormat::None;

  void* InputData = nullptr;
  int32 InputDataSize = 0;
  int32 InputDataSizeX = 0;
  int32 InputDataSizeY = 0;
  std::string InputPath;
  std::array<Cube, 6> InputCube;

  void* OutputData = nullptr;
  int32 OutputDataSize = 0;
  int32 OutputMipCount = 0;

  std::vector<OutputMip> OutputMips;

  std::string OutputPath;

  bool SRGB = false;
  bool Alpha = false;
  bool Normal = false;
  bool GenerateMips = false;
  bool InIsCube = false;
  bool SplitAlpha = false;

  MipFilterType MipFilter = MipFilterType::Mitchell;

  TextureAddress AddressX = TA_Wrap;
  TextureAddress AddressY = TA_Wrap;

  std::string Error;
};

class TfcBuilder {
public:
  TfcBuilder(const FString& name)
    : Name(name)
  {}

  ~TfcBuilder()
  {
    free(TfcData);
  }

  inline void* GetAllocation(FILE_OFFSET& size) const
  {
    size = TfcDataSize;
    return TfcData;
  }

  inline FString GetError() const
  {
    return Error;
  }

  inline size_t GetCount() const
  {
    return Textures.size();
  }

  bool AddTexture(class UTexture2D* texture);
  bool Compile();

private:
  FString Name;
  void* TfcData = nullptr;
  FILE_OFFSET TfcDataSize = 0;
  std::vector<class UTexture2D*> Textures;
  FString Error;
};
#endif