#pragma once
#include <Tera/Core.h>
#include <Tera/UActor.h>
#include <Tera/UMaterial.h>
#include <Tera/ULightMap.h>

class ULandscapeProxy : public UActor {
public:
  DECL_UOBJ(ULandscapeProxy, UActor);

  std::map<FString, UMaterialInterface*> MaterialInstanceConstantMap;

  void Serialize(FStream& s) override;
};

class ULandscapeLayerInfoObject : public UObject {
public:
  DECL_UOBJ(ULandscapeLayerInfoObject, UObject);

  UPROP_NOINIT(FName, LayerName);

  bool RegisterProperty(FPropertyTag* property) override;
};

struct LandscapeLayerStruct {
  FName LayerName;
  ULandscapeLayerInfoObject* LayerInfoObj = nullptr;
  void LoadFromPropertyValue(FPropertyValue* v);
};

class ULandscape : public ULandscapeProxy {
public:
  DECL_UOBJ(ULandscape, ULandscapeProxy);

  UPROP(int32, ComponentSizeQuads, 0);
  UPROP(int32, SubsectionSizeQuads, 0);
  UPROP(int32, NumSubsections, 0);
  UPROP_NOINIT(std::vector<class ULandscapeComponent*>, LandscapeComponents);
  UPROP_NOINIT(std::vector<LandscapeLayerStruct>, LayerInfoObjs);

  bool RegisterProperty(FPropertyTag* property) override;

  void GetHeightMapData(struct UTextureBitmapInfo& data) const;

  void GetWeighMapData(const FName& layer, struct UTextureBitmapInfo& data) const;
};

struct FWeightMapLayerAllocationInfo {
  FName LayerName;
  uint8 WeightmapTextureIndex = 0;
  uint8 WeightmapTextureChannel = 0;

  void LoadFromPropertyValue(FPropertyValue* v);
};

class ULandscapeComponent : public UPrimitiveComponent {
public:
  DECL_UOBJ(ULandscapeComponent, UPrimitiveComponent);

  UPROP(int32, SectionBaseX, 0);
  UPROP(int32, SectionBaseY, 0);
  UPROP(int32, ComponentSizeQuads, 0);
  UPROP(int32, SubsectionSizeQuads, 0);
  UPROP(int32, NumSubsections, 0);

  UPROP(UMaterialInstanceConstant*, MaterialInstance, nullptr);

  UPROP_NOINIT(std::vector<FWeightMapLayerAllocationInfo>, WeightmapLayerAllocations);

  UPROP_NOINIT(std::vector<UTexture2D*>, WeightmapTextures);

  UPROP_NOINIT(FVector4, WeightmapScaleBias);
  UPROP(float, WeightmapSubsectionOffset, 0.f);

  UPROP_NOINIT(FVector4, HeightmapScaleBias);
  UPROP(float, HeightmapSubsectionOffset, 0.f);
  UPROP_NOINIT(FVector2D, LayerUVPan);
  UPROP(UTexture2D*, HeightmapTexture, nullptr);

  FLightMap LightMap;

  bool RegisterProperty(FPropertyTag* property) override;

  void Serialize(FStream& s) override;

  void GetHeightMapData(struct UTextureBitmapInfo& data) const;

  void GetWeighMapData(const FName& layer, struct UTextureBitmapInfo& data) const;
};