#pragma once
#include <Tera/Core.h>
#include <Tera/UObject.h>

class UMaterialFunction : public UObject {
public:
  DECL_UOBJ(UMaterialFunction, UObject);

  UPROP_NOINIT(std::vector<class UMaterialExpression*>, FunctionExpressions);
  UPROP(UMaterialFunction*, ParentFunction, nullptr);

  bool RegisterProperty(FPropertyTag* property) override;

  std::vector<class UMaterialExpressionFunctionOutput*> GetOutputs() const;
  std::vector<class UMaterialExpression*> GetExpressions() const;
};
