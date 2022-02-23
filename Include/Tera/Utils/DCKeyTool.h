#pragma once
#include <Tera/Core.h>
#if DC_KEY_TOOL
#include <string>
#include <vector>

// Helper to find the decryption key from a running instance of Tera
struct DCKeyTool {
  DCKeyTool();
  ~DCKeyTool();

  bool IsTeraRunning() const;

  bool FindKey();

  std::vector<std::pair<std::string, std::string>> GetResults() const;

  bool IsX86();

protected:
  class DCKeyToolImpl* Impl = nullptr;
};
#endif