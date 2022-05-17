#pragma once
#include <Tera/UActor.h>
#include <Tera/FStructs.h>

class UInstancedFoliageActor : public UActor {
public:
  DECL_UOBJ(UInstancedFoliageActor, UActor);
  
  std::map<UStaticMesh*, FFoliageMeshInfo> FoliageMeshes;

  void Serialize(FStream& s) override;
};
