#include "Tera/Core.h"
#include "Tera/UTexture.h"
#include "Tera/Cast.h"
#include "Tera/FPackage.h"
#include "Tera/UClass.h"
#include "Tera/CoreMath.h"
#include "Tera/FObjectResource.h"

#include <Tera/Utils/ALog.h>
#include <Tera/Utils/TextureUtils.h>

#include <glew/glew.h>

struct PF_Info {
  uint32 BlockSizeX = 0;
  uint32 BlockSizeY = 0;
  uint32 BlockSizeZ = 0;
  uint32 BlockBytes = 0;
  uint32 NumComponents = 0;
  GLenum Format = 0;
};

PF_Info PixelFormatInfo[] = {
  //BsX     BsY     BsZ     Bb      Comps   GL_Format           //  Format
  { 0,			0,			0,			0,			0, 			GL_NONE},           //	PF_Unknown
  { 1,			1,			1,			16,			4, 			GL_RGBA},           //	PF_A32B32G32R32F
  { 1,			1,			1,			4,			4, 			GL_BGRA},           //	PF_A8R8G8B8
  { 1,			1,			1,			1,			1, 			GL_LUMINANCE},      //	PF_G8
  { 1,			1,			1,			2,			1, 			GL_NONE},           //	PF_G16
  { 4,			4,			1,			8,			3, 			GL_COMPRESSED_RGB_S3TC_DXT1_EXT},           //	PF_DXT1
  { 4,			4,			1,			16,			4, 			GL_COMPRESSED_RGBA_S3TC_DXT3_EXT},           //	PF_DXT3
  { 4,			4,			1,			16,			4, 			GL_COMPRESSED_RGBA_S3TC_DXT5_EXT},           //	PF_DXT5
  { 2,			1,			1,			4,			4, 			GL_NONE},           //	PF_UYVY
  { 1,			1,			1,			8,			3, 			GL_RGB},            //	PF_FloatRGB
  { 1,			1,			1,			8,			4, 			GL_RGBA},           //	PF_FloatRGBA
  { 1,			1,			1,			0,			1, 			GL_DEPTH_STENCIL},  //	PF_DepthStencil
  { 1,			1,			1,			4,			1, 			GL_DEPTH_COMPONENT},//	PF_ShadowDepth
  { 1,			1,			1,			4,			1, 			GL_DEPTH_COMPONENT},//	PF_FilteredShadowDepth
  { 1,			1,			1,			4,			1, 			GL_RED},            //	PF_R32F
  { 1,			1,			1,			4,			2, 			GL_RG},             //	PF_G16R16
  { 1,			1,			1,			4,			2, 			GL_RG},             //	PF_G16R16F
  { 1,			1,			1,			4,			2, 			GL_RG},             //	PF_G16R16F_FILTER
  { 1,			1,			1,			8,			2, 			GL_RG},             //	PF_G32R32F
  { 1,			1,			1,			4,			4, 			GL_RGBA},           //  PF_A2B10G10R10
  { 1,			1,			1,			8,			4, 			GL_RGBA},           //	PF_A16B16G16R16
  { 1,			1,			1,			4,			1, 			GL_NONE},           //	PF_D24
  { 1,			1,			1,			2,			1, 			GL_RED},            //	PF_R16F
  { 1,			1,			1,			2,			1, 			GL_RED},            //	PF_R16F_FILTER
  { 4,			4,			1,			16,			2, 			GL_NONE},           //	PF_BC5
  { 1,			1,			1,			2,			2, 			GL_RG},             //	PF_V8U8
  { 1,			1,			1,			1,			1, 			GL_NONE},           //	PF_A1			
  { 1,			1,			1,			0,			3, 			GL_NONE},           //	PF_FloatR11G11B10
};

inline bool FindInternalFormatAndType(uint32 pixelFormat, GLenum& outFormat, GLenum& outType, bool bSRGB)
{
  switch (pixelFormat)
  {
  case PF_A32B32G32R32F:
    outFormat = bSRGB ? GL_NONE : GL_RGBA32F_ARB;
    outType = GL_UNSIGNED_NORMALIZED_ARB;
    return true;
  case PF_A8R8G8B8:
    outFormat = bSRGB ? GL_SRGB8_ALPHA8_EXT : GL_RGBA;
    outType = GL_UNSIGNED_BYTE;
    return true;
  case PF_DXT1:
    outFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    outType = GL_UNSIGNED_BYTE;
    return true;
  case PF_DXT3:
    outFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    outType = GL_UNSIGNED_BYTE;
    return true;
  case PF_DXT5:
    outFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    outType = GL_UNSIGNED_BYTE;
    return true;
  case PF_G8:
    outFormat = bSRGB ? GL_RGB8 : GL_LUMINANCE8;
    outType = GL_UNSIGNED_BYTE;
    return true;
  case PF_DepthStencil:
    outFormat = bSRGB ? GL_NONE : GL_DEPTH24_STENCIL8_EXT;
    outType = GL_UNSIGNED_INT_24_8_EXT;
    return true;
  case PF_D24:
  case PF_ShadowDepth:
  case PF_FilteredShadowDepth:
    outFormat = bSRGB ? GL_NONE : GL_DEPTH_COMPONENT24_ARB;
    outType = GL_UNSIGNED_INT;
    return true;
  case PF_R32F:
    outFormat = bSRGB ? GL_NONE : GL_R32F;
    outType = GL_FLOAT;
    return true;
  case PF_G16R16:
    outFormat = bSRGB ? GL_RGBA16 : GL_RG16;
    outType = GL_UNSIGNED_SHORT;
    return true;
  case PF_G16R16F:
  case PF_G16R16F_FILTER:
    outFormat = bSRGB ? GL_RGBA16F_ARB : GL_RG16F;
    outType = GL_HALF_FLOAT_ARB;
    return true;
  case PF_G32R32F:
    outFormat = bSRGB ? GL_RGBA32F_ARB : GL_RG32F;
    outType = GL_FLOAT;
    return true;
  case PF_A2B10G10R10:
    outFormat = bSRGB ? GL_NONE : GL_RGB10_A2;
    outType = GL_UNSIGNED_INT_2_10_10_10_REV;
    return true;
  case PF_A16B16G16R16:
    outFormat = bSRGB ? GL_NONE : GL_RGBA16F_ARB;
    outType = GL_HALF_FLOAT_ARB;
    return true;
  case PF_R16F:
  case PF_R16F_FILTER:
    outFormat = bSRGB ? GL_NONE : GL_R16F;
    outType = GL_HALF_FLOAT_ARB;
    return true;
  case PF_FloatRGB:
    outFormat = bSRGB ? GL_NONE : GL_RGB16F_ARB;
    outType = GL_HALF_FLOAT_ARB;
    return true;
  case PF_FloatRGBA:
    outFormat = bSRGB ? GL_NONE : GL_RGBA16F_ARB;
    outType = GL_HALF_FLOAT_ARB;
    return true;
  case PF_V8U8:
    outFormat = bSRGB ? GL_NONE : GL_RG8;
    outType = GL_BYTE;
    return true;
  default:
    return false;
  }
}

bool UTexture::RegisterProperty(FPropertyTag* property)
{
  if (PROP_IS(property, SRGB))
  {
    SRGB = property->BoolVal;
    SRGBProperty = property;
    return true;
  }
  if (PROP_IS(property, CompressionSettings))
  {
    CompressionSettings = (TextureCompressionSettings)property->Value->GetByte();
    CompressionSettingsProperty = property;
    return true;
  }
  if (PROP_IS(property, UnpackMin))
  {
    UnpackMin[property->ArrayIndex] = property->GetFloat();
    UnpackMinProperty[property->ArrayIndex] = property;
    return true;
  }
  else if (PROP_IS(property, LODGroup))
  {
    LODGroup = (TextureGroup)property->Value->GetByte();
    LODGroupProperty = property;
    return true;
  }
  return false;
}

void UTexture::Serialize(FStream& s)
{
  Super::Serialize(s);
  SourceArt.Serialize(s, this);
}

UTexture2D::~UTexture2D()
{
  DeleteStorage();
}

bool UTexture2D::RegisterProperty(FPropertyTag* property)
{
  if (Super::RegisterProperty(property))
  {
    return false;
  }
  if (PROP_IS(property, Format))
  {
    Format = (EPixelFormat)property->Value->GetByte();
    FormatProperty = property;
    return true;
  }
  else if (PROP_IS(property, SizeX))
  {
    SizeX = property->Value->GetInt();
    SizeXProperty = property;
    return true;
  }
  else if (PROP_IS(property, SizeY))
  {
    SizeY = property->Value->GetInt();
    SizeYProperty = property;
    return true;
  }
  else if (PROP_IS(property, AddressX))
  {
    AddressX = (TextureAddress)property->Value->GetByte();
    AddressXProperty = property;
    return true;
  }
  else if (PROP_IS(property, AddressY))
  {
    AddressY = (TextureAddress)property->Value->GetByte();
    AddressYProperty = property;
    return true;
  }
  else if (PROP_IS(property, MipTailBaseIdx))
  {
    MipTailBaseIdx = property->Value->GetInt();
    MipTailBaseIdxProperty = property;
    return true;
  }
  else if (PROP_IS(property, FirstResourceMemMip))
  {
    FirstResourceMemMip = property->Value->GetInt();
    FirstResourceMemMipProperty = property;
    return true;
  }
  else if (PROP_IS(property, bNoTiling))
  {
    bNoTiling = property->GetBool();
    bNoTilingProperty = property;
    return true;
  }
  else if (PROP_IS(property, TextureFileCacheName))
  {
    TextureFileCacheName = property->Value->GetNamePtr();
    TextureFileCacheNameProperty = property;
    //return true; Let subclasses get this value
  }
  else if (PROP_IS(property, NeverStream))
  {
    NeverStream = property->GetBool();
    NeverStreamProperty = property;
    return true;
  }
  else if (PROP_IS(property, bIsStreamable))
  {
    bIsStreamable = property->GetBool();
    bIsStreamableProperty = property;
    return true;
  }
  return false;
}

bool UTexture2D::GetBitmapData(UTextureBitmapInfo& info)
{
  info.Format = info.InternalFormat = PixelFormatInfo[Format].Format;
  if (!Mips.size() || !FindInternalFormatAndType(Format, info.InternalFormat, info.Type, SRGB) || info.InternalFormat == GL_NONE || info.Format == GL_NONE)
  {
    return false;
  }
  for (FTexture2DMipMap* mip : Mips)
  {
    if (mip->SizeX && mip->SizeY && mip->Data->GetAllocation())
    {
      if ((info.Height && mip->SizeY > info.Height) || (info.Width && mip->SizeX > info.Width))
      {
        continue;
      }
      info.Size = mip->Data->GetBulkDataSize();
      info.Allocation = mip->Data->GetAllocation();
      info.Width = mip->SizeX;
      info.Height = mip->SizeY;
      return true;
    }
  }
  return false;
}

void UTexture2D::Serialize(FStream& s)
{
  Super::Serialize(s);
  if (s.GetFV() == VER_TERA_CLASSIC || s.GetFV() == VER_TERA_MODERN)
  {
    s << SourceFilePath;
  }
  s.SerializeUntypeBulkDataArray(Mips, this);
  s << TextureFileCacheGuid;
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s.SerializeUntypeBulkDataArray(CachedMips, this);
    s << MaxCachedResolution;
    s.SerializeUntypeBulkDataArray(CachedAtiMips, this);
    CachedFlashMip.Serialize(s, this);
    s.SerializeUntypeBulkDataArray(CachedEtcMips, this);
  }
}

unsigned int UTexture2D::Hash() const
{
  if (!Loaded)
  {
    return 0;
  }
  for (FTexture2DMipMap* mip : Mips)
  {
    if (mip->Data && mip->Data->ElementCount)
    {
      return CalculateDataCRC(mip->Data->GetAllocation(), mip->Data->GetBulkDataSize());
    }
  }
  return 0;
}

void UTexture2D::DisableCaching()
{
  Load();
  
  if (TextureFileCacheNameProperty)
  {
    RemoveProperty(TextureFileCacheNameProperty);
    TextureFileCacheName = nullptr;
    TextureFileCacheNameProperty = nullptr;
  }

  TextureFileCacheGuid = FGuid();

  if (FirstResourceMemMip != 0)
  {
    FirstResourceMemMip = 0;
    if (FirstResourceMemMipProperty)
    {
      FirstResourceMemMipProperty->Value->GetInt() = 0;
    }
  }

  if (MipTailBaseIdxProperty)
  {
    MipTailBaseIdx = 0;
    MipTailBaseIdxProperty->Value->GetInt() = 0;
  }

  for (int32 idx = 0; idx < Mips.size(); ++idx)
  {
    if (idx > 0)
    {
      delete Mips[idx];
      continue;
    }
    FTexture2DMipMap* mip = Mips[idx];
    if (mip->Data->BulkDataFlags & BULKDATA_Unused)
    {
      continue;
    }
#if TEXTURE2D_COMPRESSION == 0
    if (mip->Data->IsStoredCompressedOnDisk())
    {
      mip->Data->BulkDataFlags &= ~BULKDATA_SerializeCompressedLZO;
    }
#endif
    if (mip->Data->IsStoredInSeparateFile())
    {
      mip->Data->BulkDataFlags &= ~BULKDATA_StoreInSeparateFile;
    }
  }
  Mips.resize(1);
  MarkDirty();
}

void UTexture2D::DisableStreaming()
{
  if (NeverStreamProperty)
  {
    NeverStreamProperty->GetBool() = true;
    NeverStream = true;
  }
  else
  {
    CreatePropertyNeverStream(true);
  }

  if (bIsStreamableProperty)
  {
    bIsStreamableProperty->GetBool() = false;
    bIsStreamable = false;
  }
  else
  {
    CreatePropertybIsStreamable(false);
  }
}

void UTexture2D::PostLoad()
{
  Super::PostLoad();
  FString cacheName;
  FStream* rs = nullptr;
  for (int32 idx = 0; idx < Mips.size(); ++idx)
  {
    FTexture2DMipMap* mip = Mips[idx];
    if (mip->Data->IsStoredInSeparateFile())
    {
      bool loaded = false;
      if (TextureFileCacheName)
      {
        if (TextureFileCacheName->String() != cacheName)
        {
          if (rs)
          {
            delete rs;
            rs = nullptr;
          }
          FString path = FPackage::GetTextureFileCachePath(TextureFileCacheName->String());
          rs = new FReadStream(path);
          if (rs->IsGood())
          {
            cacheName = TextureFileCacheName->String();
          }
        }

        if (rs && rs->IsGood())
        {
          FStream& s = *rs;
          s.SetPosition(mip->Data->GetBulkDataOffsetInFile());

          try
          {
            mip->Data->SerializeSeparate(s, this, idx);
            loaded = true;
          }
          catch (...)
          {
            LogE("Failed to serialize a separate mip: %s.MipLevel_%d", GetObjectPath().C_str(), idx);
            delete rs;
            return;
          }
        }
      }

      if (loaded)
      {
        continue;
      }

      if (FPackage::GetCoreVersion() == VER_TERA_CLASSIC && (GetExportFlags() & EF_ForcedExport) && mip->Data->GetBulkDataOffsetInFile() >= 0)
      {
        FString objectPath = GetObjectPath();
        FString packageName;
        {
          std::vector<FString> comps = objectPath.Split('.');
          if (comps.size() > 2)
          {
            packageName = comps[1];
            objectPath = objectPath.Substr(comps[0].Size() + comps[1].Size() + 2);
          }
        }
        
        for (int32 number = 1; packageName.Size() && number < 6; ++number)
        {
          std::shared_ptr<FPackage> package = nullptr;
          try
          {
            FString pkgToOpen = packageName;
            if (number > 1)
            {
              pkgToOpen += "_" + std::to_string(number);
            }
            if (package = FPackage::GetPackageNamed(pkgToOpen))
            {
              if (package.get() == GetPackage())
              {
                FPackage::UnloadPackage(package);
                continue;
              }
              package->Load();
              if (UTexture2D* tex = Cast<UTexture2D>(package->GetObject(objectPath)))
              {
                FReadStream s = FReadStream(A2W(package->GetDataPath()));
                s.SetPackage(package.get());
                s.SetLoadSerializedObjects(package->GetStream().GetLoadSerializedObjects());
                s.SetPosition(mip->Data->GetBulkDataOffsetInFile());
                try
                {
                  mip->Data->SerializeSeparate(s, this, idx);
                  loaded = true;
                  GetPackage()->RetainPackage(package);
                }
                catch (...)
                {
                }
              }
              if (!loaded)
              {
                FPackage::UnloadPackage(package);
              }
            }
          }
          catch (...)
          {
          }
          if (loaded)
          {
            break;
          }
        }
      }

      if (loaded)
      {
        continue;
      }

      FString bulkDataName = GetObjectPath() + ".MipLevel_" + std::to_string(idx);
      bulkDataName = bulkDataName.ToUpper();
      FBulkDataInfo* info = FPackage::GetBulkDataInfo(bulkDataName);
      if (!info)
      {
        bulkDataName += "DXT";
        info = FPackage::GetBulkDataInfo(bulkDataName);
      }
      if (info)
      {
        if (cacheName != info->TextureFileCacheName)
        {
          if (rs)
          {
            delete rs;
            rs = nullptr;
          }
          FString path = FPackage::GetTextureFileCachePath(info->TextureFileCacheName);
          if (path.Size())
          {
            rs = new FReadStream(path);
            if (rs->IsGood())
            {
              cacheName = info->TextureFileCacheName;
            }
          }
        }

        if (rs)
        {
          FStream& s = *rs;
          s.SetPosition(info->SavedBulkDataOffsetInFile);
          try
          {
            mip->Data->SerializeSeparate(s, this, idx);
          }
          catch (...)
          {
            LogE("Failed to decompress bulkdata: %s", bulkDataName.C_str());
            delete rs;
            return;
          }
        }
      }
    }
  }
  if (rs)
  {
    delete rs;
  }
}

void UTexture2D::DeleteStorage()
{
  for (auto& e : Mips)
  {
    delete e;
  }
  Mips.clear();
  for (auto& e : CachedMips)
  {
    delete e;
  }
  CachedMips.clear();
  for (auto& e : CachedAtiMips)
  {
    delete e;
  }
  CachedAtiMips.clear();
  for (auto& e : CachedEtcMips)
  {
    delete e;
  }
  CachedEtcMips.clear();
}


EPixelFormat UTexture2D::GetEffectivePixelFormat(const EPixelFormat format, bool bSRGB)
{
  if ((format == PF_G8) && bSRGB)
  {
    return PF_A8R8G8B8;
  }
  return format;
}

bool UTextureCube::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(FacePosX, UTexture2D*);
  REGISTER_TOBJ_PROP(FaceNegX, UTexture2D*);
  REGISTER_TOBJ_PROP(FacePosY, UTexture2D*);
  REGISTER_TOBJ_PROP(FaceNegY, UTexture2D*);
  REGISTER_TOBJ_PROP(FacePosZ, UTexture2D*);
  REGISTER_TOBJ_PROP(FaceNegZ, UTexture2D*);
  return false;
}

void UTextureCube::PostLoad()
{
  Super::PostLoad();
  LoadObject(FacePosX);
  LoadObject(FaceNegX);
  LoadObject(FacePosY);
  LoadObject(FaceNegY);
  LoadObject(FacePosZ);
  LoadObject(FaceNegZ);
}

void ULightMapTexture2D::ConfigureClassObject(UClass* obj)
{
  obj->SetSuperStruct(obj->GetPackage()->GetClass(UTexture2D::StaticClassName()));
  obj->SetPropertyLink(obj->GetSuperStruct()->GetPropertyLink());
}

void ULightMapTexture2D::Serialize(FStream& s)
{
  if (s.IsReading())
  {
    LODGroup = TEXTUREGROUP_Lightmap;
  }
  Super::Serialize(s);
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << LightmapFlags;
  }
}

void TextureTravaller::SetFormat(EPixelFormat format)
{
  Format = format;
}

void TextureTravaller::SetAddressX(TextureAddress x)
{
  AddressX = x;
}

void TextureTravaller::SetAddressY(TextureAddress y)
{
  AddressY = y;
}

void TextureTravaller::SetCompression(TextureCompressionSettings compression)
{
  Compression = compression;
}

void TextureTravaller::SetSRGB(bool srgb)
{
  SRGB = srgb;
}

void TextureTravaller::SetIsNew(bool flag)
{
  ConfigureAsNew = flag;
}

void TextureTravaller::SetLODGroup(TextureGroup group)
{
  LODGroup = group;
}

void TextureTravaller::SetRawData(void* data, int32 size, bool transferOwnership)
{
  Data = (uint8*)data;
  Size = size;
  OwnsData = transferOwnership;
}

void TextureTravaller::AddMipMap(int32 sizeX, int32 sizeY, int32 size, void* data)
{
  Mips.push_back({ sizeX, sizeY, size, data });
}

std::string TextureTravaller::GetError() const
{
  return Error;
}

bool TextureTravaller::Visit(UTexture2D* texture)
{
  if (!texture)
  {
    Error = "Texture is NULL.";
    return false;
  }
  if (!Mips.size())
  {
    Error = "Texture has no mipmaps.";
    return false;
  }

  if (ConfigureAsNew)
  {
    for (FPropertyTag* p : texture->Properties)
    {
      delete p;
    }
    texture->Properties.clear();
    texture->SourceFilePath = SourcePath;
  }

  // TODO: patch Resource allocation
  texture->GetPackage()->GetTextureAllocations().TextureTypes.clear();
  // Invalidate texture cache
  texture->TextureFileCacheGuid = FGuid();

  if (Compression != TC_Default)
  {
    if (!texture->CompressionSettingsProperty)
    {
      texture->CreatePropertyCompressionSettings(Compression);
    }
    else
    {
      texture->CompressionSettings = Compression;
      texture->CompressionSettingsProperty->GetByte() = (uint8)Compression;
      FString enumName = texture->CompressionSettingsProperty->Value->Enum->GetEnum(Compression).String();
      texture->GetPackage()->GetNameIndex(enumName, true);
    }
    if (Compression == TC_NormalmapAlpha || Compression == TC_Normalmap)
    {
      for (int32 idx = 0; idx < 3; ++idx)
      {
        if (!texture->UnpackMinProperty[idx])
        {
          texture->CreatePropertyUnpackMin(-1., idx);
        }
        else
        {
          texture->UnpackMinProperty[idx]->GetFloat() = -1.;
          texture->UnpackMin[idx] = -1.;
        }
      }
    }
  }

  if (LODGroup != texture->LODGroup)
  {
    if (!texture->LODGroupProperty)
    {
      texture->CreatePropertyLODGroup(LODGroup);
    }
    else
    {
      texture->LODGroup = LODGroup;
      texture->LODGroupProperty->GetByte() = (uint8)LODGroup;
      FString enumName = texture->LODGroupProperty->Value->Enum->GetEnum(LODGroup).String();
      texture->GetPackage()->GetNameIndex(enumName, true);
    }
  }

  if (AddressX != TA_Wrap || AddressY != TA_Wrap)
  {
    if (!texture->AddressXProperty)
    {
      texture->CreatePropertyAddressX(AddressX);
    }
    else
    {
      texture->AddressX = AddressX;
      FString enumName = texture->AddressXProperty->Value->Enum->GetEnum(AddressX).String();
      texture->GetPackage()->GetNameIndex(enumName, true);
    }
    if (!texture->AddressYProperty)
    {
      texture->CreatePropertyAddressY(AddressY);
    }
    else
    {
      texture->AddressY = AddressY;
      FString enumName = texture->AddressYProperty->Value->Enum->GetEnum(Compression).String();
      texture->GetPackage()->GetNameIndex(enumName, true);
    }
  }

  if (!texture->FormatProperty)
  {
    texture->CreatePropertyFormat(Format);
    texture->Format = Format;
  }
  if (Format != texture->Format)
  {
    texture->Format = Format;
    texture->FormatProperty->Value->GetByte() = Format;
    FString enumName = texture->FormatProperty->Value->Enum->GetEnum(Format).String();
    texture->GetPackage()->GetNameIndex(enumName, true);
  }

  if (Mips[0].SizeX != texture->SizeX)
  {
    texture->SizeX = Mips[0].SizeX;
    if (!texture->SizeXProperty)
    {
      texture->CreatePropertySizeX(Mips[0].SizeX);
    }
    else
    {
      texture->SizeXProperty->Value->GetInt() = Mips[0].SizeX;
    }
  }

  if (Mips[0].SizeY != texture->SizeY)
  {
    texture->SizeY = Mips[0].SizeY;
    if (!texture->SizeYProperty)
    {
      texture->CreatePropertySizeY(Mips[0].SizeY);
    }
    else
    {
      texture->SizeYProperty->Value->GetInt() = Mips[0].SizeY;
    }
  }

  if (!SRGB && !texture->SRGBProperty)
  {
    texture->CreatePropertySRGB(SRGB);
  }
  if (SRGB != texture->SRGB)
  {
    texture->SRGB = SRGB;
    texture->SRGBProperty->Value->GetBool() = SRGB;
  }

  if (!texture->MipTailBaseIdxProperty)
  {
    texture->CreatePropertyMipTailBaseIdx((int32)Mips.size() - 1);
  }
  if (Mips.size() - 1 != texture->MipTailBaseIdx)
  {
    texture->MipTailBaseIdx = (int32)Mips.size() - 1;
    texture->MipTailBaseIdxProperty->Value->GetInt() = (uint32)Mips.size() - 1;
  }

  if (texture->TextureFileCacheNameProperty)
  {
    texture->RemoveProperty(texture->TextureFileCacheNameProperty);
    texture->TextureFileCacheName = nullptr;
    texture->TextureFileCacheNameProperty = nullptr;
  }

  if (texture->GetPackage()->GetFileVersion() > VER_TERA_CLASSIC)
  {
    if (!texture->FirstResourceMemMipProperty)
    {
      texture->CreatePropertyFirstResourceMemMip(0);
    }
    if (texture->FirstResourceMemMip != 0)
    {
      texture->FirstResourceMemMip = 0;
      texture->FirstResourceMemMipProperty->Value->GetInt() = 0;
    }
  }

  // We pulled all the data, so it should be safe to turn off fexp flag
  texture->Export->ExportFlags &= ~EF_ForcedExport;

  texture->DeleteStorage();

  for (TMipMap& tmip : Mips)
  {
    FTexture2DMipMap* mip = new FTexture2DMipMap();
    mip->SizeX = tmip.SizeX;
    mip->SizeY = tmip.SizeY;

    void* data = malloc(tmip.Size);
    memcpy(data, tmip.Data, tmip.Size);
    mip->Data = new FByteBulkData(texture->GetPackage(), TEXTURE2D_COMPRESSION ? BULKDATA_SerializeCompressedLZO : BULKDATA_None, tmip.Size, data, true);
    texture->Mips.push_back(mip);
  }

  texture->MarkDirty();
  return true;
}