#include "Tera/UInstancedFoliageActor.h"
#include "Tera/UStaticMesh.h"
#include "Tera/FStream.h"

FStream& operator<<(FStream& s, FFoliageInstance& i)
{
  s << i.Base;
  s << i.Location;
  s << i.Rotation;
  s << i.DrawScale3D;
  s << i.ClusterIndex;
  s << i.PreAlignRotation;
  uint32 tmp = (uint32)i.Flags;
  s << tmp;
  i.Flags = (EFoliageInstanceFlags)tmp;
  s << i.ZOffset;
  return s;
}

FStream& operator<<(FStream& s, FFoliageInstanceCluster& c)
{
  return s << c.Bounds << c.ClusterComponent << c.InstanceIndices;
}

FStream& operator<<(FStream& s, FFoliageMeshInfo& i)
{
  s << i.Clusters;
  s << i.Instances;
  s << i.Settings;
  return s;
}

void UInstancedFoliageActor::Serialize(FStream& s)
{
  Super::Serialize(s);
  s << FoliageMeshes;
}
