#include "Tera/ULevel.h"
#include "Tera/UClass.h"
#include "Tera/UProperty.h"

#include "Tera/FPackage.h"
#include "Tera/Cast.h"

FStream& operator<<(FStream& s, FGuidPair& p)
{
  return s << p.Guid << p.RefId;;
}

FStream& operator<<(FStream& s, FCoverIndexPair& p)
{
  return s << p.ActorRefItem << p.SlotIdx;
}

void ULevelBase::Serialize(FStream& s)
{
  Super::Serialize(s);
  SERIALIZE_UREF(s, ActorsArrayOwner);
  s << Actors;
  s << URL;
}

std::vector<UActor*> ULevelBase::GetActors() const
{
  std::vector<UActor*> result;
  for (PACKAGE_INDEX idx : Actors)
  {
    result.push_back(Cast<UActor>(GetPackage()->GetObject(idx)));
  }
  return result;
}

void ULevel::ConfigureClassObject(UClass* object)
{
  CreateClassProperty("LightmapTotalSize", UFloatProperty::StaticClassName(), object);
  CreateClassProperty("ShadowmapTotalSize", UFloatProperty::StaticClassName(), object);

  object->Link();
}

void ULevel::Serialize(FStream& s)
{
  Super::Serialize(s);

  SERIALIZE_UREF(s, Model);
  s << ModelComponents;
  
  s << LevelSequences;
  
  s << TextureToInstancesMap;
  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << DynamicTextureInstances;
    s << ApexDestructionDataSize;
    if (s.IsReading() && ApexDestructionDataSize)
    {
      ApexDestructionData = malloc(ApexDestructionDataSize);
    }
    s.SerializeBytes(ApexDestructionData, ApexDestructionDataSize);
  }
  
  s << CachedPhysBSPDataElementSize;
  s << CachedPhysBSPData;

  s << CachedPhysSMDataMap;
  s << CachedPhysSMDataStore;

  s << CachedPhysPerTriSMDataMap;
  s << CachedPhysPerTriSMDataStore;

  s << CachedPhysBSPDataVersion;
  s << CachedPhysSMDataVersion;

  s << ForceStreamTextures;

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << CachedPhysConvexBSPData;
    s << CachedPhysConvexBSPVersion;
  }
  
  SERIALIZE_UREF(s, NavListStart);
  SERIALIZE_UREF(s, NavListEnd);

  SERIALIZE_UREF(s, CoverListStart);
  SERIALIZE_UREF(s, CoverListEnd);

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    SERIALIZE_UREF(s, PylonListStart);
    SERIALIZE_UREF(s, PylonListEnd);

    s << CrossLevelCoverGuidRefs;

    s << CoverLinkRefs;
    s << CoverIndexPairs;
  }

  s << CrossLevelActors;

  if (s.GetFV() > VER_TERA_CLASSIC)
  {
    s << PrecomputedLightVolume;
    s << PrecomputedVisibilityHandler;
    s << PrecomputedVolumeDistanceField;
  }

  s << Unk1;
}