#include <Tera/Utils/DCKeyTool.h>
#if DC_KEY_TOOL
#include "../Private/DCKeyToolImp.h"
DCKeyTool::DCKeyTool()
  : Impl(new DCKeyToolImpl())
{
}

DCKeyTool::~DCKeyTool()
{
  delete Impl;
}

bool DCKeyTool::IsTeraRunning() const
{
  return Impl->IsTeraRunning();
}

bool DCKeyTool::FindKey()
{
  return Impl->FindKey();
}

std::vector<std::pair<std::string, std::string>> DCKeyTool::GetResults() const
{
  return Impl->GetResults();
}

bool DCKeyTool::IsX86()
{
  return Impl->IsX86();
}
#endif