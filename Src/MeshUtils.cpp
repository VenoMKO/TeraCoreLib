#include <Tera/Core.h>
#if MESH_UTILS
#include "../Private/FbxMeshUtils.h"
#include "../Private/PskMeshUtils.h"

MeshExporterType MeshUtils::GetExporterTypeFromExtension(const std::string& ext)
{
  if (ext == "psk")
  {
    return MET_Psk;
  }
  return MET_Fbx;
}

std::unique_ptr<MeshUtils> MeshUtils::CreateUtils(MeshExporterType type)
{
  switch (type)
  {
  case MET_Fbx:
    return std::unique_ptr<MeshUtils>(new FbxUtils);
  case MET_Psk:
    return std::unique_ptr<MeshUtils>(new PskUtils);
  }
  DBreak();
  return nullptr;
}
#endif