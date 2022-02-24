#include "Tera/FObjectResource.h"
#include "Tera/FStream.h"
#include "Tera/FPackage.h"
#include "Tera/UObject.h"
#include "Tera/UClass.h"

FStream& operator<<(FStream& s, FObjectImport& i)
{
  if (s.IsReading() && s.GetPackage())
  {
    i.Package = s.GetPackage();
  }
  s << i.ClassPackage;
  s << i.ClassName;
  s << i.OuterIndex;
  s << i.ObjectName;
  return s;
}

FStream& operator<<(FStream& s, FObjectExport& e)
{
  if (s.IsReading() && s.GetPackage())
  {
    e.Package = s.GetPackage();
  }
  s << e.ClassIndex;
  s << e.SuperIndex;
  s << e.OuterIndex;
  s << e.ObjectName;
  s << e.ArchetypeIndex;
  s << e.ObjectFlags;

  s << e.SerialSize;
  s << e.SerialOffset;

  s << e.ExportFlags;

  s << e.GenerationNetObjectCount;

  s << e.PackageGuid;
  s << e.PackageFlags;
  return s;
}

FName FObjectExport::GetClassName() const
{
  if (ClassIndex)
  {
    return Package->GetResourceObject(ClassIndex)->GetObjectName();
  }
  return VName("Class");
}

FObjectImport* FObjectImport::CreateImport(FPackage* package, const FString& objectName, UClass* objectClass)
{
  if (!package || objectName.Empty())
  {
    return nullptr;
  }
  FObjectImport* result = new FObjectImport(package, objectName);
  package->GetNameIndex(objectName, true);
  package->GetNameIndex(objectClass ? objectClass->GetObjectNameString() : "Class", true);
  result->ClassName = FName(package, objectClass ? objectClass->GetObjectNameString() : "Class");
  result->ClassPackage = FName(package, objectClass ? objectClass->GetPackage()->GetPackageName() : "Core");
  return result;
}

FString FObjectImport::GetPackageName() const
{
  PACKAGE_INDEX outerIndex = OuterIndex;
  FObjectResource* outer = nullptr;
  while (outerIndex)
  {
    if (outerIndex < 0)
    {
      outer = Package->GetImportObject(outerIndex);
      outerIndex = outer->OuterIndex;
    }
    else
    {
      // Imports outer object is Export. Not sure what is the correct behavior
      outer = Package->GetExportObject(outerIndex);
      outerIndex = 0;
    }
  }
  return outer ? outer->ObjectName.String(true) : FString();
}

FString FObjectImport::GetObjectPath() const
{
  FObjectResource* outer = GetOuter();
  FString path;
  while (outer)
  {
    path = outer->GetObjectName().String() + "." + path;
    outer = outer->GetOuter();
  }
  path += GetObjectNameString();
  return path;
}

FObjectResource* FObjectResource::GetOuter() const
{
  if (OuterIndex)
  {
    return Package->GetResourceObject(OuterIndex);
  }
  return nullptr;
}

FString FObjectResource::GetObjectPath() const
{
  if (FObjectResource* outer = GetOuter())
  {
    return outer->GetObjectPath() + "." + GetObjectNameString();
  }
  return Package->GetPackageName() + "." + GetObjectNameString();
}

VObjectExport::VObjectExport(FPackage* pkg, const char* objName, const char* className)
  : FObjectExport(pkg)
{
  ObjectName = VName(objName);
  VObjectClassName = VName(className);
  ObjectIndex = VEXP_INDEX;
}

FString VObjectExport::GetObjectPath() const
{
  return Package->GetPackageName() + "." + GetObjectNameString();
}

VObjectExport::~VObjectExport()
{
  if (VObject)
  {
    delete VObject;
  }
}
