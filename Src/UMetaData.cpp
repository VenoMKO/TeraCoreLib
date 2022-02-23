#include "Tera/UMetaData.h"
#include "Tera/FObjectResource.h"
#include "Tera/FPackage.h"

#include <filesystem>

const std::map<UObject*, std::map<FName, FString>>& UMetaData::GetObjectMetaDataMap()
{
  return ObjectMetaDataMap;
}

void UMetaData::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << ObjectMetaDataMap;
}
