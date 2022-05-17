#include "Tera/UMaterialFunction.h"
#include "Tera/UMaterialExpression.h"
#include "Tera/Cast.h"

bool UMaterialFunction::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, FunctionExpressions))
  {
    FunctionExpressionsProperty = property;
    for (FPropertyValue* v : property->GetArray())
    {
      FunctionExpressions.emplace_back(Cast<UMaterialExpression>(v->GetObjectValuePtr()));
    }
    return true;
  }
  REGISTER_TOBJ_PROP(ParentFunction, UMaterialFunction*);
  return false;
}

std::vector<UMaterialExpressionFunctionOutput*> UMaterialFunction::GetOutputs() const
{
  std::vector<UMaterialExpressionFunctionOutput*> result;
  for (const UMaterialExpression* exp : FunctionExpressions)
  {
    if (UMaterialExpressionFunctionOutput* o = Cast<UMaterialExpressionFunctionOutput>(exp))
    {
      result.emplace_back(o);
    }
  }
  return result;
}

std::vector<UMaterialExpression*> UMaterialFunction::GetExpressions() const
{
  return FunctionExpressions;
}
