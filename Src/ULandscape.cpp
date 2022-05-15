#include "Tera/ULandscape.h"
#include "Tera/FPackage.h"
#include "Tera/UTexture.h"
#include "Tera/Cast.h"

#include <glew/glew.h>

void ULandscapeProxy::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << MaterialInstanceConstantMap;
}

bool ULandscapeComponent::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INT_PROP(SectionBaseX);
  REGISTER_INT_PROP(SectionBaseY);
  REGISTER_INT_PROP(ComponentSizeQuads);
  REGISTER_INT_PROP(SubsectionSizeQuads);
  REGISTER_INT_PROP(NumSubsections);
  REGISTER_TOBJ_PROP(MaterialInstance, UMaterialInstanceConstant*);
  if (PROP_IS(property, WeightmapLayerAllocations))
  {
    WeightmapLayerAllocationsProperty = property;
    for (FPropertyValue* v : property->GetArray())
    {
      FWeightMapLayerAllocationInfo& i = WeightmapLayerAllocations.emplace_back();
      i.LoadFromPropertyValue(v);
    }
    return true;
  }
  if (PROP_IS(property, WeightmapTextures))
  {
    WeightmapTexturesProperty = property;
    for (FPropertyValue* v : property->GetArray())
    {
      WeightmapTextures.emplace_back(Cast<UTexture2D>(GetPackage()->GetObject(v->GetObjectIndex())));
    }
    return true;
  }
  REGISTER_VEC4_PROP(WeightmapScaleBias);
  REGISTER_FLOAT_PROP(WeightmapSubsectionOffset);
  REGISTER_VEC4_PROP(HeightmapScaleBias);
  REGISTER_FLOAT_PROP(HeightmapSubsectionOffset);
  REGISTER_VEC2D_PROP(LayerUVPan);
  REGISTER_TOBJ_PROP(HeightmapTexture, UTexture2D*);
  return false;
}

void ULandscapeComponent::Serialize(FStream& s)
{
  Super::Serialize(s);
  LightMap.Serialize(s);
}

void ULandscapeComponent::GetHeightMapData(UTextureBitmapInfo& out) const
{
  UTextureBitmapInfo bitmap;
  if (!HeightmapTexture || !HeightmapTexture->GetBitmapData(bitmap))
  {
    return;
  }
  out = bitmap;
  out.Format = GL_RED;
  out.InternalFormat = GL_R16;
  out.Type = GL_UNSIGNED_SHORT;
  uint32* colorData = (uint32*)bitmap.Allocation;
  int32 stride = HeightmapTexture->SizeX;
  int32 compOffsetX = round((float)HeightmapTexture->SizeX * HeightmapScaleBias.Z);
  int32 compOffsetY = round((float)HeightmapTexture->SizeY * HeightmapScaleBias.W);
  int32 size = (SubsectionSizeQuads + 1) * NumSubsections;
  out.Size = size * sizeof(uint16);
  out.Allocation = malloc(out.Size * out.Size);
  memset(out.Allocation, 0, out.Size * out.Size);
  uint16* data = (uint16*)out.Allocation;
  for (int32 subY = 0; subY < size; ++subY)
  {
    for (int32 subX = 0; subX < size; ++subX)
    {
      const FColor* texel = (FColor*)&colorData[compOffsetX + (subY + compOffsetY) * stride + subX];
      uint32 tmp1 = (texel->R << 8) + texel->G;
      DBreakIf(tmp1 >= 0xffff);
      uint16 tmp = (texel->R << 8) + texel->G;
      memcpy((uint16*)out.Allocation + (subY * size) + subX, &tmp, sizeof(tmp));
    }
  }
}

void ULandscapeComponent::GetWeighMapData(const FName& layer, UTextureBitmapInfo& out) const
{
  int32 layerIndex = -1;
  int32 channelIndex = -1;
  for (int32 idx = 0; idx < (int32)WeightmapLayerAllocations.size(); ++idx)
  {
    if (WeightmapLayerAllocations[idx].LayerName == layer)
    {
      layerIndex = WeightmapLayerAllocations[idx].WeightmapTextureIndex;
      channelIndex = WeightmapLayerAllocations[idx].WeightmapTextureChannel;
      break;
    }
  }

  int32 size = (SubsectionSizeQuads + 1) * NumSubsections;
  UTextureBitmapInfo bitmap;
  out.Format = GL_RED;
  out.InternalFormat = GL_R8;
  out.Type = GL_UNSIGNED_BYTE;
  out.Size = size * sizeof(uint8);
  out.Allocation = malloc(out.Size * out.Size);
  memset(out.Allocation, 0, out.Size * out.Size);
  if (WeightmapTextures.size() <= layerIndex || !WeightmapTextures[layerIndex]->GetBitmapData(bitmap))
  {
    return;
  }
  out.Width = bitmap.Width;
  out.Height = bitmap.Height;
  uint32* colorData = (uint32*)bitmap.Allocation;
  int64 chOffsets[4] = { (int64)&((FColor*)NULL)->R, (int64)&((FColor*)NULL)->G, (int64)&((FColor*)NULL)->B, (int64)&((FColor*)NULL)->A };
  uint8* texPtr = (uint8*)colorData + chOffsets[channelIndex];
  for (int32 idx = 0; idx < (size * size); ++idx)
  {
    memcpy((uint8*)out.Allocation + idx, texPtr + (idx * 4), sizeof(uint8));
  }
}

void FWeightMapLayerAllocationInfo::LoadFromPropertyValue(FPropertyValue* value)
{
  for (FPropertyValue* v : value->GetArray())
  {
    if (v->GetPropertyTagPtr()->Name == "LayerName")
    {
      LayerName = v->GetPropertyTagPtr()->Value->GetName();
    }
    else if (v->GetPropertyTagPtr()->Name == "WeightmapTextureIndex")
    {
      WeightmapTextureIndex = v->GetPropertyTagPtr()->GetByte();
    }
    else if (v->GetPropertyTagPtr()->Name == "WeightmapTextureChannel")
    {
      WeightmapTextureChannel = v->GetPropertyTagPtr()->GetByte();
    }
  }
}

bool ULandscape::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INT_PROP(ComponentSizeQuads);
  REGISTER_INT_PROP(SubsectionSizeQuads);
  REGISTER_INT_PROP(NumSubsections);
  if (PROP_IS(property, LandscapeComponents))
  {
    LandscapeComponentsProperty = property;
    for (FPropertyValue* v : property->GetArray())
    {
      if (ULandscapeComponent* comp = Cast<ULandscapeComponent>(v->GetObjectValuePtr(false)))
      {
        LandscapeComponents.emplace_back(comp);
      }
    }
    return true;
  }
  if (PROP_IS(property, LayerInfoObjs))
  {
    LayerInfoObjsProperty = property;
    for (FPropertyValue* v : property->GetArray())
    {
      LayerInfoObjs.emplace_back().LoadFromPropertyValue(v);
    }
    return true;
  }
  return false;
}

void ULandscape::GetHeightMapData(UTextureBitmapInfo& out) const
{
  if (LandscapeComponents.empty())
  {
    return;
  }
  FIntPoint min = { LandscapeComponents.front()->SectionBaseX, LandscapeComponents.front()->SectionBaseY };
  FIntPoint max = min;
  int32 componentSize = LandscapeComponents.front()->HeightmapTexture->SizeX;
  for (ULandscapeComponent* comp : LandscapeComponents)
  {
    if (comp->SectionBaseX > max.X)
    {
      max.X = comp->SectionBaseX;
    }
    if (comp->SectionBaseY > max.Y)
    {
      max.Y = comp->SectionBaseY;
    }
    if (comp->SectionBaseX < min.X)
    {
      min.X = comp->SectionBaseX;
    }
    if (comp->SectionBaseY < min.Y)
    {
      min.Y = comp->SectionBaseY;
    }
  }
  max.X -= min.X - componentSize;
  max.Y -= min.Y - componentSize;
  out.Size = max.X * max.Y * sizeof(uint16);
  out.Allocation = malloc(out.Size);
  out.Width = max.X;
  out.Height = max.Y;
  out.Format = GL_RED;
  out.InternalFormat = GL_R16;
  out.Type = GL_UNSIGNED_SHORT;

  for (ULandscapeComponent* comp : LandscapeComponents)
  {
    int32 posX = comp->SectionBaseX - min.X;
    int32 posY = comp->SectionBaseY - min.Y;
    UTextureBitmapInfo tmpInfo;
    comp->GetHeightMapData(tmpInfo);
    for (int32 y = 0; y < componentSize; ++y)
    {
      const uint16* src = (uint16*)tmpInfo.Allocation + (y * componentSize);
      uint16* dst = (uint16*)out.Allocation + ((posY + y) * out.Width) + posX;
      memcpy(dst, src, componentSize * sizeof(uint16));
    }
    free(tmpInfo.Allocation);
  }
}

void ULandscape::GetWeighMapData(const FName& layer, struct UTextureBitmapInfo& out) const
{
  if (LandscapeComponents.empty())
  {
    return;
  }
  FIntPoint min = { LandscapeComponents.front()->SectionBaseX, LandscapeComponents.front()->SectionBaseY };
  FIntPoint max = min;
  int32 componentSize = LandscapeComponents.front()->WeightmapTextures.front()->SizeX;
  for (ULandscapeComponent* comp : LandscapeComponents)
  {
    if (comp->SectionBaseX > max.X)
    {
      max.X = comp->SectionBaseX;
    }
    if (comp->SectionBaseY > max.Y)
    {
      max.Y = comp->SectionBaseY;
    }
    if (comp->SectionBaseX < min.X)
    {
      min.X = comp->SectionBaseX;
    }
    if (comp->SectionBaseY < min.Y)
    {
      min.Y = comp->SectionBaseY;
    }
  }
  max.X -= min.X - componentSize;
  max.Y -= min.Y - componentSize;
  out.Size = max.X * max.Y * sizeof(uint16);
  out.Allocation = malloc(out.Size);
  out.Width = max.X;
  out.Height = max.Y;
  out.Format = GL_RED;
  out.InternalFormat = GL_R8;
  out.Type = GL_UNSIGNED_BYTE;

  for (ULandscapeComponent* comp : LandscapeComponents)
  {
    int32 posX = comp->SectionBaseX - min.X;
    int32 posY = comp->SectionBaseY - min.Y;
    UTextureBitmapInfo tmpInfo;
    comp->GetWeighMapData(layer, tmpInfo);
    for (int32 y = 0; y < componentSize; ++y)
    {
      const uint8* src = (uint8*)tmpInfo.Allocation + (y * componentSize);
      uint8* dst = (uint8*)out.Allocation + ((posY + y) * out.Width) + posX;
      memcpy(dst, src, componentSize * sizeof(uint8));
    }
    free(tmpInfo.Allocation);
  }
}

bool ULandscapeLayerInfoObject::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_NAME_PROP(LayerName);
  return false;
}

void LandscapeLayerStruct::LoadFromPropertyValue(FPropertyValue* value)
{
  for (FPropertyValue* v : value->GetArray())
  {
    if (v->GetPropertyTagPtr()->Name == "LayerName")
    {
      LayerName = v->GetPropertyTagPtr()->Value->GetName();
    }
    else if (v->GetPropertyTagPtr()->Name == "LayerInfoObj")
    {
      LayerInfoObj = Cast<ULandscapeLayerInfoObject>(v->GetPropertyTagPtr()->GetObjectValuePtr());
    }
  }
}
