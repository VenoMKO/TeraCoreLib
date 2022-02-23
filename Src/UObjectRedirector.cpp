#include "Tera/UObjectRedirector.h"
#include "Tera/FPackage.h"
#include "Tera/FObjectResource.h"
#include "Tera/FName.h"

void UObjectRedirector::Serialize(FStream& s)
{
  Super::Serialize(s);
  SERIALIZE_UREF(s, Object);
  DBreakIf(ObjectRefIndex && !Object);
}