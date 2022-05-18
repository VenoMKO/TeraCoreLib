#pragma once
#include <map>
#include <functional>

class UObjectFactory {
public:
  using TCtor = std::function<class UObject* (class FObjectExport*)>;

  UObjectFactory() = delete;

  static bool Register(const char* className, TCtor ctor);

  static UObject* Create(const class FString& className, class FObjectExport* exp);

private:
  static std::map<std::size_t, TCtor> RegisteredClasses;
};