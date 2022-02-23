#pragma once
#include <Tera/UObject.h>

class UMetaData : public UObject {
public:
  DECL_UOBJ(UMetaData, UObject);

  const std::map<UObject*, std::map<FName, FString>>& GetObjectMetaDataMap();

  void Serialize(FStream& s) override;

protected:
  std::map<UObject*, std::map<FName, FString>> ObjectMetaDataMap;
};