#pragma once
#include <Tera/UObject.h>
#include <array>

class TextureTravaller {
public:
  void SetFormat(EPixelFormat format);
  void SetAddressX(TextureAddress x);
  void SetAddressY(TextureAddress y);
  void SetCompression(TextureCompressionSettings compression);
  void SetSRGB(bool srgb);
  void SetIsNew(bool flag);
  void SetLODGroup(TextureGroup group);

  void SetRawData(void* data, int32 size, bool transferOwnership = false);
  void AddMipMap(int32 sizeX, int32 sizeY, int32 size, void* data);

  std::string GetError() const;

  bool Visit(class UTexture2D* texture);

private:
  struct TMipMap {
    int32 SizeX = 0;
    int32 SizeY = 0;
    int32 Size = 0;
    void* Data = nullptr;
  };

private:
  std::string Error;

  EPixelFormat Format = PF_Unknown;
  TextureAddress AddressX = TA_Wrap;
  TextureAddress AddressY = TA_Wrap;
  TextureGroup LODGroup = TEXTUREGROUP_World;
  TextureCompressionSettings Compression = TC_Default;
  bool SRGB = false;
  bool ConfigureAsNew = false;
  FString SourcePath;

  bool OwnsData = false;
  uint8* Data = nullptr;
  int32 Size = 0;
  std::vector<TMipMap> Mips;
};

struct TextureInfo {
  void SetRawData(void* data, int32 size, bool transferOwnership = false);
  void AddMipMap(int32 sizeX, int32 sizeY, int32 size, void* data);

  struct TMipMap {
    int32 SizeX = 0;
    int32 SizeY = 0;
    int32 Size = 0;
    void* Data = nullptr;
  };

  EPixelFormat Format = PF_Unknown;
  TextureAddress AddressX = TA_Wrap;
  TextureAddress AddressY = TA_Wrap;
  TextureCompressionSettings Compression = TC_Default;
  bool SRGB = false;

  // MipMaps
  bool OwnsData = false;
  uint8* Data = nullptr;
  int32 Size = 0;
  std::vector<TMipMap> Mips;
};

class UTexture : public UObject {
public:
  DECL_UOBJ(UTexture, UObject);

  UPROP_CREATABLE(bool, SRGB, true);
  UPROP_CREATABLE_ENUM(TextureCompressionSettings, CompressionSettings, TC_Default);
  UPROP_CREATABLE_STATIC_ARR(float, 4, UnpackMin, 0,0,0,0);
  UPROP_CREATABLE_ENUM(TextureGroup, LODGroup, TEXTUREGROUP_World);

  bool RegisterProperty(FPropertyTag* property) override;

  void Serialize(FStream& s) override;

protected:
  FByteBulkData SourceArt;
};

struct UTextureBitmapInfo {
  UTextureBitmapInfo() = default;

  UTextureBitmapInfo(int32 width)
    : Width(width)
  {}

  UTextureBitmapInfo(int32 width, int32 height)
    : Width(width)
    , Height(height)
  {}

  int32 Width = 0;
  int32 Height = 0;
  int32 Size = 0;
  void* Allocation = nullptr;
  uint32 Type = 0;
  uint32 Format = 0;
  uint32 InternalFormat = 0;

  bool IsValid() const
  {
    return Width > 0 && Height > 0 && Size > 0 && Allocation;
  }
};

class UTexture2D : public UTexture {
public:

  static EPixelFormat GetEffectivePixelFormat(const EPixelFormat format, bool bSRGB);

  DECL_UOBJ(UTexture2D, UTexture);

  ~UTexture2D() override;

  bool RegisterProperty(FPropertyTag* property) override;

  UPROP_CREATABLE_ENUM(EPixelFormat, Format, PF_Unknown);
  UPROP_CREATABLE(int32, SizeX, 0);
  UPROP_CREATABLE(int32, SizeY, 0);
  UPROP_CREATABLE_ENUM(TextureAddress, AddressX, TA_Wrap);
  UPROP_CREATABLE_ENUM(TextureAddress, AddressY, TA_Wrap);
  UPROP_CREATABLE(int32, MipTailBaseIdx, 0);
  UPROP_CREATABLE(int32, FirstResourceMemMip, 0);
  UPROP_CREATABLE(bool, bNoTiling, false);
  UPROP_CREATABLE(bool, NeverStream, false);
  UPROP_CREATABLE(bool, bIsStreamable, true);
  UPROP(FName*, TextureFileCacheName, nullptr);

  bool GetBitmapData(UTextureBitmapInfo& info);

  friend bool TextureTravaller::Visit(UTexture2D* texture);

  void Serialize(FStream& s) override;

  unsigned int Hash() const;

  // Disable texture caching, pull max mipmap, delete smaller mips
  // Needed for cross-region mods
  void DisableCaching();

  void DisableStreaming();

protected:
  void PostLoad() override;
  void DeleteStorage();

public:
  std::vector<FTexture2DMipMap*> Mips;
protected:
  FString SourceFilePath;
  std::vector<FTexture2DMipMap*> CachedMips;
  std::vector<FTexture2DMipMap*> CachedAtiMips;
  FByteBulkData CachedFlashMip;
  std::vector<FTexture2DMipMap*> CachedEtcMips;
  FGuid TextureFileCacheGuid;
  int32 MaxCachedResolution = 0;
};

class UTerrainWeightMapTexture : public UTexture2D {
public:
  DECL_UOBJ(UTerrainWeightMapTexture, UTexture2D);
};

class UTextureCube : public UTexture {
public:
  DECL_UOBJ(UTextureCube, UTexture);
  UPROP(UTexture2D*, FacePosX, nullptr);
  UPROP(UTexture2D*, FaceNegX, nullptr);
  UPROP(UTexture2D*, FacePosY, nullptr);
  UPROP(UTexture2D*, FaceNegY, nullptr);
  UPROP(UTexture2D*, FacePosZ, nullptr);
  UPROP(UTexture2D*, FaceNegZ, nullptr);

  bool RegisterProperty(FPropertyTag* property) override;

  std::array<UTexture2D*, 6> GetFaces()
  {
    return { FacePosX, FaceNegX, FacePosY, FaceNegY, FacePosZ, FaceNegZ };
  }

protected:
  void PostLoad() override;
};

class UTextureFlipBook : public UTexture2D {
public:
  DECL_UOBJ(UTextureFlipBook, UTexture2D);
};

class UShadowMapTexture2D : public UTexture2D {
public:
  DECL_UOBJ(UShadowMapTexture2D, UTexture2D);
};

class ULightMapTexture2D : public UTexture2D {
public:
  DECL_UOBJ(ULightMapTexture2D, UTexture2D);

  static void ConfigureClassObject(class UClass* obj);

  void Serialize(FStream& s) override;

protected:
  uint32 LightmapFlags = 0;
};