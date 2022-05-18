#include "Tera/UMaterialExpression.h"
#include "Tera/FObjectResource.h"
#include "Tera/UClass.h"
#include "Tera/Cast.h"
#include "Tera/UTexture.h"

namespace {
  const char* VSEP = "\t";
}

#define REGISTER_INPUT(TName)\
if (PROP_IS(property, TName))\
{\
  __GLUE_PROP(TName,Property) = property;\
  TName = FExpressionInput(property);\
  TName.Title = #TName;\
  return true;\
}\
//

#define SUPER_ACCEPT()\
Super::AcceptVisitor(visitor)

#define SET_INPUT(...)\
visitor.SetInput({ __VA_ARGS__ });

FExpressionInput::FExpressionInput(const FPropertyTag * property)
{
  if (property && property->Value)
  {
    std::vector<FPropertyValue*> arr = property->Value->GetArray();
    for (FPropertyValue* item : arr)
    {
      FPropertyTag* tag = item->GetPropertyTagPtr();
      FString tagName = tag->Name.String();
      if (tagName == "Expression")
      {
        Expression = Cast<UMaterialExpression>(tag->GetObjectValuePtr());
      }
      else if (tagName == "Mask")
      {
        Mask = tag->Value ? tag->Value->GetBool() : tag->GetBool();
      }
      else if (tagName == "MaskR")
      {
        MaskR = tag->Value ? tag->Value->GetBool() : tag->GetBool();
      }
      else if (tagName == "MaskG")
      {
        MaskG = tag->Value ? tag->Value->GetBool() : tag->GetBool();
      }
      else if (tagName == "MaskB")
      {
        MaskB = tag->Value ? tag->Value->GetBool() : tag->GetBool();
      }
      else if (tagName == "MaskA")
      {
        MaskA = tag->Value ? tag->Value->GetBool() : tag->GetBool();
      }
      else if (tagName == "OutputIndex")
      {
        OutputIndex = tag->GetInt();
      }
      else if (tagName == "InputName" || tagName == "OutputName")
      {
        InputName = tag->GetString();
      }
    }
  }
}

FString FExpressionInput::GetDescription() const
{
  return CustomDescription.Length() ? CustomDescription : InputName.Length() ? InputName : Title.Empty() ? FString("In") : Title;
}

int32 UMaterialExpression::GetPosX() const
{
  if (MaterialExpressionEditorXProperty)
  {
    return MaterialExpressionEditorX;
  }
  return EditorX;
}

int32 UMaterialExpression::GetPosY() const
{
  if (MaterialExpressionEditorYProperty)
  {
    return MaterialExpressionEditorY;
  }
  return EditorY;
}

bool UMaterialExpression::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INT_PROP(MaterialExpressionEditorX);
  REGISTER_INT_PROP(MaterialExpressionEditorY);
  REGISTER_INT_PROP(EditorX);
  REGISTER_INT_PROP(EditorY);
  REGISTER_STR_PROP(Desc);
  return false;
}

FString UMaterialExpression::GetTitle() const
{
  FString title = GetClassNameString();
  if (title.StartsWith(UMaterialExpression::StaticClassName()))
  {
    title = title.Substr(strlen(UMaterialExpression::StaticClassName()));
    if (title.Empty())
    {
      title = GetClassNameString();
    }
  }
  return title;
}

void UMaterialExpression::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{  
  visitor.SetTitle(GetTitle());
  if (MaterialExpressionEditorXProperty || MaterialExpressionEditorYProperty)
  {
    visitor.SetEditorPosition(MaterialExpressionEditorX, MaterialExpressionEditorY);
  }
  else if (EditorXProperty || EditorYProperty)
  {
    visitor.SetEditorPosition(EditorX, EditorY);
  }
  visitor.SetDescription(Desc);
}

void UMaterialExpression::ExportExpression(AMaterialExpression& output)
{
  output.Parameters.clear();
  output.Class = GetStaticClassName();
  output.Index = Export->ObjectIndex;
  output.Position.X = (float)-GetPosX() - 250;
  output.Position.Y = (float)GetPosY();
  if (DescProperty)
  {
    output.Description = Desc;
  }
}

bool UMaterialExpressionAbs::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  return false;
}

void UMaterialExpressionAbs::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
}

void UMaterialExpressionAbs::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
}

bool UMaterialExpressionAdd::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionAdd::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

void UMaterialExpressionAdd::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(A);
  output.Parameters.emplace_back(B);
}

bool UMaterialExpressionAppendVector::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionAppendVector::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

void UMaterialExpressionAppendVector::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(A);
  output.Parameters.emplace_back(B);
}

bool UMaterialExpressionBumpOffset::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Coordinate);
  REGISTER_INPUT(Height);
  REGISTER_FLOAT_PROP(HeightRatio);
  REGISTER_FLOAT_PROP(ReferencePlane);
  return false;
}

void UMaterialExpressionBumpOffset::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Coordinate, Height);
  visitor.SetValue(FString::Sprintf("HeightRatio:%f\nReferencePlane:%f", HeightRatio, ReferencePlane));
}

void UMaterialExpressionBumpOffset::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Coordinate);
  output.Parameters.emplace_back(Height);
  if (HeightRatioProperty)
  {
    output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("HeightRatio", HeightRatio));
  }
  if (ReferencePlaneProperty)
  {
    output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("ReferencePlane", ReferencePlane));
  }
}

bool UMaterialExpressionCeil::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  return false;
}

void UMaterialExpressionCeil::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
}

void UMaterialExpressionCeil::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
}

bool UMaterialExpressionClamp::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_INPUT(Min);
  REGISTER_INPUT(Max);
  return false;
}

void UMaterialExpressionClamp::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input, Min, Max);
}

void UMaterialExpressionClamp::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
  output.Parameters.emplace_back(Min);
  output.Parameters.emplace_back(Max);
}

bool UMaterialExpressionComment::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INT_PROP(PosX);
  REGISTER_INT_PROP(PosY);
  REGISTER_INT_PROP(SizeX);
  REGISTER_INT_PROP(SizeY);
  REGISTER_STR_PROP(Text);
  return false;
}

void UMaterialExpressionComment::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetEditorSize(SizeX, SizeY);
  visitor.SetValue(Text);
}

bool UMaterialExpressionComponentMask::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_BOOL_PROP(R);
  REGISTER_BOOL_PROP(G);
  REGISTER_BOOL_PROP(B);
  REGISTER_BOOL_PROP(A);
  return false;
}

void UMaterialExpressionComponentMask::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  std::vector<char> channels;
  if (R)
  {
    channels.push_back('R');
  }
  if (G)
  {
    channels.push_back('G');
  }
  if (B)
  {
    channels.push_back('B');
  }
  if (A)
  {
    channels.push_back('A');
  }
  std::string mask;
  for (const auto& piece : channels)
  {
    mask.empty() ? mask += piece : mask += std::string(",") + piece;
  }
  visitor.SetValue(mask);
}

void UMaterialExpressionComponentMask::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
  if (RProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("R", R));
  if (GProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("G", G));
  if (BProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("B", B));
  if (AProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("A", A));
}

bool UMaterialExpressionCompound::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, MaterialExpressions))
  {
    MaterialExpressionsProperty = property;
    for (FPropertyValue* value : property->GetArray())
    {
      MaterialExpressions.push_back(Cast<UMaterialExpression>(value->GetObjectValuePtr()));
    }
    return true;
  }
  REGISTER_STR_PROP(Caption);
  REGISTER_BOOL_PROP(bExpanded);
  return false;
}

void UMaterialExpressionCompound::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  FString expressions;
  for (const auto exp : MaterialExpressions)
  {
    expressions.Empty() ? expressions += exp->GetObjectNameString() : expressions += FString(":") + exp->GetObjectNameString();
  }
  visitor.SetValue(expressions);
  visitor.SetTitle(GetTitle() + ": " + Caption);
}

bool UMaterialExpressionConstant::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(R);
  return false;
}

void UMaterialExpressionConstant::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(FString::Sprintf("%.2f", R));
}

void UMaterialExpressionConstant::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (RProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("R", R));
}

bool UMaterialExpressionConstant2Vector::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(R);
  REGISTER_FLOAT_PROP(G);
  return false;
}

void UMaterialExpressionConstant2Vector::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(FString::Sprintf("%.2f,%f", R, G));
}

void UMaterialExpressionConstant2Vector::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (RProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("R", R));
  if (GProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("G", G));
}

bool UMaterialExpressionConstant3Vector::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(R);
  REGISTER_FLOAT_PROP(G);
  REGISTER_FLOAT_PROP(B);
  return false;
}

void UMaterialExpressionConstant3Vector::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(FString::Sprintf("%.2f,%f,%f", R, G, B));
}

void UMaterialExpressionConstant3Vector::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (RProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("R", R));
  if (GProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("G", G));
  if (BProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("B", B));
}

bool UMaterialExpressionConstant4Vector::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(R);
  REGISTER_FLOAT_PROP(G);
  REGISTER_FLOAT_PROP(B);
  REGISTER_FLOAT_PROP(A);
  return false;
}

void UMaterialExpressionConstant4Vector::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(FString::Sprintf("%.2f,%f,%f,%f", R, G, B, A));
}

void UMaterialExpressionConstant4Vector::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (RProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("R", R));
  if (GProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("G", G));
  if (BProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("B", B));
  if (AProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("A", A));
}

bool UMaterialExpressionConstantBiasScale::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_FLOAT_PROP(Bias);
  REGISTER_FLOAT_PROP(Scale);
  return false;
}

void UMaterialExpressionConstantBiasScale::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  visitor.SetValue(FString::Sprintf("Bias:%f\nScale:%f", Bias, Scale));
}

void UMaterialExpressionConstantBiasScale::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
  if (Bias)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("Bias", Bias));
  if (Scale)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("Scale", Scale));
}

bool UMaterialExpressionConstantClamp::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_FLOAT_PROP(Min);
  REGISTER_FLOAT_PROP(Max);
  return false;
}

void UMaterialExpressionConstantClamp::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  visitor.SetValue(FString::Sprintf("Min:%f\nMax:%f", Min, Max));
}

void UMaterialExpressionConstantClamp::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
  if (MinProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("Min", Min));
  if (MaxProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("Max", Max));
}

bool UMaterialExpressionCosine::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_FLOAT_PROP(Period);
  return false;
}

void UMaterialExpressionCosine::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  visitor.SetValue(FString::Sprintf("Period:%f", Period));
}

void UMaterialExpressionCosine::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
  if (PeriodProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("Period", Period));
}

bool UMaterialExpressionCrossProduct::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionCrossProduct::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

void UMaterialExpressionCrossProduct::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(A);
  output.Parameters.emplace_back(B);
}

bool UMaterialExpressionCustomTexture::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_OBJ_PROP(Texture);
  return false;
}

void UMaterialExpressionCustomTexture::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(Texture ? Texture->GetObjectNameString() : "NULL");
}

void UMaterialExpressionCustomTexture::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("Texture", Texture));
}

bool UMaterialExpressionDepthBiasedAlpha::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bNormalize);
  REGISTER_FLOAT_PROP(BiasScale);
  REGISTER_INPUT(Alpha);
  REGISTER_INPUT(Bias);
  return false;
}

void UMaterialExpressionDepthBiasedAlpha::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Alpha, Bias);
  visitor.SetValue(FString::Sprintf("bNormalize: %s\nBiasScale:%f", bNormalize ? "true" : "false", BiasScale));
}

void UMaterialExpressionDepthBiasedAlpha::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (bNormalizeProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("bNormalize", bNormalize));
  if (BiasScaleProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("BiasScale", BiasScale));
  output.Parameters.emplace_back(Alpha);
  output.Parameters.emplace_back(Bias);
}

bool UMaterialExpressionDepthBiasedBlend::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bNormalize);
  REGISTER_FLOAT_PROP(BiasScale);
  REGISTER_INPUT(RGB);
  REGISTER_INPUT(Alpha);
  REGISTER_INPUT(Bias);
  return false;
}

void UMaterialExpressionDepthBiasedBlend::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(RGB, Alpha, Bias);
  visitor.SetValue(FString::Sprintf("bNormalize: %s\nBiasScale:%f", bNormalize ? "true" : "false", BiasScale));
}

void UMaterialExpressionDepthBiasedBlend::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (bNormalizeProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("bNormalize", bNormalize));
  if (BiasScaleProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("BiasScale", BiasScale));
  output.Parameters.emplace_back(RGB);
  output.Parameters.emplace_back(Alpha);
  output.Parameters.emplace_back(Bias);
}

bool UMaterialExpressionDepthOfFieldFunction::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BYTE_PROP(FunctionValue);
  REGISTER_INPUT(Depth);
  return false;
}

void UMaterialExpressionDepthOfFieldFunction::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Depth);
  visitor.SetValue(FString::Sprintf("FunctionValue: %d", int(FunctionValue)));
}

bool UMaterialExpressionDeriveNormalZ::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(InXY);
  return false;
}

void UMaterialExpressionDeriveNormalZ::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(InXY);
}

void UMaterialExpressionDeriveNormalZ::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(InXY);
}

bool UMaterialExpressionDesaturation::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_INPUT(Percent);
  REGISTER_LCOL_PROP(LuminanceFactors);
  return false;
}

void UMaterialExpressionDesaturation::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input, Percent);
  visitor.SetValue(FString::Sprintf("LumFactors:\nR:%f G:%f\nB:%f A:%f", LuminanceFactors.R, LuminanceFactors.G, LuminanceFactors.B, LuminanceFactors.A));
}

void UMaterialExpressionDesaturation::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
  output.Parameters.emplace_back(Percent);
  if (LuminanceFactorsProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("LuminanceFactors", LuminanceFactors));
}

bool UMaterialExpressionDestDepth::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bNormalize);
  return false;
}

void UMaterialExpressionDestDepth::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(FString::Sprintf("bNormalize: %s", bNormalize ? "true" : "false"));
}

void UMaterialExpressionDestDepth::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (bNormalizeProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput("bNormalize", bNormalize));
}

bool UMaterialExpressionDistance::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionDistance::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

void UMaterialExpressionDistance::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(A);
  output.Parameters.emplace_back(B);
}

bool UMaterialExpressionDivide::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionDivide::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

void UMaterialExpressionDivide::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(A);
  output.Parameters.emplace_back(B);
}

bool UMaterialExpressionDotProduct::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionDotProduct::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

void UMaterialExpressionDotProduct::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(A);
  output.Parameters.emplace_back(B);
}

bool UMaterialExpressionDynamicParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, ParamNames))
  {
    ParamNamesProperty = property;
    for (const auto& param : property->GetArray())
    {
      ParamNames.push_back(param->GetString());
    }
    return true;
  }
  return false;
}

void UMaterialExpressionDynamicParameter::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  FString v;
  for (const auto& name : ParamNames)
  {
    v.Empty() ? v += name : v += FString(",") + name;
  }
  visitor.SetValue(v);
}

bool UMaterialExpressionFloor::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  return false;
}

void UMaterialExpressionFloor::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
}

void UMaterialExpressionFloor::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
}

bool UMaterialExpressionFluidNormal::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Coordinates);
  return false;
}

void UMaterialExpressionFluidNormal::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Coordinates);
}

void UMaterialExpressionFluidNormal::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Coordinates);
}

bool UMaterialExpressionFmod::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionFmod::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

void UMaterialExpressionFmod::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(A);
  output.Parameters.emplace_back(B);
}

bool UMaterialExpressionFontSample::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_OBJ_PROP(Font);
  REGISTER_INT_PROP(FontTexturePage);
  return false;
}

void UMaterialExpressionFontSample::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(FString::Sprintf("Font: %s\nFontTexturePage: %d", (Font ? Font->GetObjectNameString().UTF8().c_str() : "NULL"), FontTexturePage));
}

void UMaterialExpressionFontSample::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (FontProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_Font, Font));
  if (FontTexturePageProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_FontTexturePage, FontTexturePage));
}

bool UMaterialExpressionFontSampleParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_NAME_PROP(ParameterName);
  return false;
}

void UMaterialExpressionFontSampleParameter::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(ParameterName.String());
}

void UMaterialExpressionFontSampleParameter::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (ParameterNameProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_ParameterName, ParameterName.String()));
}

bool UMaterialExpressionFrac::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  return false;
}

void UMaterialExpressionFrac::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
}

void UMaterialExpressionFrac::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
}

bool UMaterialExpressionFresnel::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(Exponent);
  REGISTER_INPUT(Normal);
  return false;
}

void UMaterialExpressionFresnel::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Normal);
  visitor.SetValue(FString::Sprintf("Exp:%f", Exponent));
}

void UMaterialExpressionFresnel::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Normal);
  if (ExponentProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_Exponent, Exponent));
}

bool UMaterialExpressionIf::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  REGISTER_INPUT(AGreaterThanB);
  REGISTER_INPUT(AEqualsB);
  REGISTER_INPUT(ALessThanB);
  return false;
}

void UMaterialExpressionIf::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B, AGreaterThanB, AEqualsB, ALessThanB);
}

void UMaterialExpressionIf::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(A);
  output.Parameters.emplace_back(B);
  output.Parameters.emplace_back(AGreaterThanB);
  output.Parameters.emplace_back(AEqualsB);
  output.Parameters.emplace_back(ALessThanB);
}

bool UMaterialExpressionLightmassReplace::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Realtime);
  REGISTER_INPUT(Lightmass);
  return false;
}

void UMaterialExpressionLightmassReplace::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Realtime, Lightmass);
}

void UMaterialExpressionLightmassReplace::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Realtime);
  output.Parameters.emplace_back(Lightmass);
}

bool UMaterialExpressionLinearInterpolate::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  REGISTER_INPUT(Alpha);
  return false;
}

void UMaterialExpressionLinearInterpolate::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B, Alpha);
}

void UMaterialExpressionLinearInterpolate::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(A);
  output.Parameters.emplace_back(B);
  output.Parameters.emplace_back(Alpha);
}

bool UMaterialExpressionMultiply::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionMultiply::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

void UMaterialExpressionMultiply::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(A);
  output.Parameters.emplace_back(B);
}

bool UMaterialExpressionNormalize::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(VectorInput);
  return false;
}

void UMaterialExpressionNormalize::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(VectorInput);
}

void UMaterialExpressionNormalize::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(VectorInput);
}

bool UMaterialExpressionOneMinus::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  return false;
}

void UMaterialExpressionOneMinus::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
}

void UMaterialExpressionOneMinus::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
}

bool UMaterialExpressionPanner::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Coordinate);
  REGISTER_INPUT(Time);
  REGISTER_FLOAT_PROP(SpeedX);
  REGISTER_FLOAT_PROP(SpeedY);
  return false;
}

void UMaterialExpressionPanner::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Coordinate, Time);
  visitor.SetValue(FString::Sprintf("SpdX:%f\nSpdY:%f", SpeedX, SpeedY));
}

void UMaterialExpressionPanner::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Coordinate);
  output.Parameters.emplace_back(Time);
  if (SpeedXProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_SpeedX, SpeedX));
  if (SpeedYProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_SpeedY, SpeedY));
}

bool UMaterialExpressionParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_NAME_PROP(ParameterName);
  return false;
}

void UMaterialExpressionParameter::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(ParameterName.String());
}

void UMaterialExpressionParameter::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (ParameterNameProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_ParameterName, ParameterName.String()));
}

bool UMaterialExpressionScalarParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(DefaultValue);
  return false;
}

void UMaterialExpressionScalarParameter::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(ParameterName.String() + FString(":") + FString::Sprintf("%.2f", DefaultValue));
}

void UMaterialExpressionScalarParameter::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (DefaultValueProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_DefaultValue, DefaultValue));
}

bool UMaterialExpressionStaticComponentMaskParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_BOOL_PROP(DefaultR);
  REGISTER_BOOL_PROP(DefaultG);
  REGISTER_BOOL_PROP(DefaultB);
  REGISTER_BOOL_PROP(DefaultA);
  return false;
}

void UMaterialExpressionStaticComponentMaskParameter::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  FString mask;
  if (DefaultR)
  {
    mask += "R";
  }
  if (DefaultG)
  {
    mask += "G";
  }
  if (DefaultB)
  {
    mask += "B";
  }
  if (DefaultA)
  {
    mask += "A";
  }
  visitor.SetValue(mask);
}

void UMaterialExpressionStaticComponentMaskParameter::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
  if (DefaultRProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_DefaultR, DefaultR));
  if (DefaultGProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_DefaultG, DefaultG));
  if (DefaultBProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_DefaultB, DefaultB));
  if (DefaultAProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_DefaultA, DefaultA));
}

bool UMaterialExpressionStaticSwitchParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(DefaultValue);
  REGISTER_BOOL_PROP(ExtendedCaptionDisplay);
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionStaticSwitchParameter::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
  visitor.SetValue(ParameterName.String() + FString(":") + (DefaultValue ? "true" : "false"));
}

void UMaterialExpressionStaticSwitchParameter::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(A);
  output.Parameters.emplace_back(B);
  if (DefaultValueProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_DefaultValue, DefaultValue));
  if (ExtendedCaptionDisplayProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_ExtendedCaptionDisplay, ExtendedCaptionDisplay));
}

bool UMaterialExpressionVectorParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_LCOL_PROP(DefaultValue);
  return false;
}

void UMaterialExpressionVectorParameter::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(ParameterName.String() + FString::Sprintf("\n%.2f,%f,%f,%f", DefaultValue.R, DefaultValue.G, DefaultValue.B, DefaultValue.A));
}

void UMaterialExpressionVectorParameter::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_DefaultValue, DefaultValue));
}

bool UMaterialExpressionParticleMacroUV::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bUseViewSpace);
  return false;
}

void UMaterialExpressionParticleMacroUV::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(FString::Sprintf("bUseViewSpace: %s", bUseViewSpace ? "true" : "false"));
}

void UMaterialExpressionParticleMacroUV::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (bUseViewSpaceProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_bUseViewSpace, bUseViewSpace));
}

bool UMaterialExpressionPixelDepth::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bNormalize);
  return false;
}

void UMaterialExpressionPixelDepth::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(FString::Sprintf("bNormalize: %s", bNormalize ? "true" : "false"));
}

void UMaterialExpressionPixelDepth::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (bNormalizeProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_bNormalize, bNormalize));
}

bool UMaterialExpressionPower::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Base);
  REGISTER_INPUT(Exponent);
  return false;
}

void UMaterialExpressionPower::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Base, Exponent);
}

void UMaterialExpressionPower::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Base);
  output.Parameters.emplace_back(Exponent);
}

bool UMaterialExpressionRotateAboutAxis::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(NormalizedRotationAxisAndAngle);
  REGISTER_INPUT(PositionOnAxis);
  REGISTER_INPUT(Position);
  return false;
}

void UMaterialExpressionRotateAboutAxis::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(NormalizedRotationAxisAndAngle, PositionOnAxis, Position);
}

void UMaterialExpressionRotateAboutAxis::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(NormalizedRotationAxisAndAngle);
  output.Parameters.emplace_back(PositionOnAxis);
  output.Parameters.emplace_back(Position);
}

bool UMaterialExpressionRotator::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Coordinate);
  REGISTER_INPUT(Time);
  REGISTER_FLOAT_PROP(CenterX);
  REGISTER_FLOAT_PROP(CenterY);
  REGISTER_FLOAT_PROP(Speed);
  return false;
}

void UMaterialExpressionRotator::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Coordinate, Time);
  visitor.SetValue(FString::Sprintf("Center:%f,%f\nSpeed:%f", CenterX, CenterY, Speed));
}

void UMaterialExpressionRotator::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Coordinate);
  output.Parameters.emplace_back(Time);
  if (CenterXProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_CenterX, CenterX));
  if (CenterYProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_CenterY, CenterY));
  if (SpeedProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_Speed, Speed));
}

bool UMaterialExpressionSceneDepth::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Coordinates);
  REGISTER_BOOL_PROP(bNormalize);
  return false;
}

void UMaterialExpressionSceneDepth::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(FString::Sprintf("bNormalize: %s", bNormalize ? "true" : "false"));
}

void UMaterialExpressionSceneDepth::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Coordinates);
  if (bNormalizeProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_bNormalize, bNormalize));
}

bool UMaterialExpressionSceneTexture::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Coordinates);
  REGISTER_BYTE_PROP(SceneTextureType);
  REGISTER_BOOL_PROP(ScreenAlign);
  return false;
}

void UMaterialExpressionSceneTexture::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Coordinates);
  visitor.SetValue(FString::Sprintf("SceneTextureType: %d\nScreenAlign: %s", int(SceneTextureType), ScreenAlign ? "true" : "false"));
}

void UMaterialExpressionSceneTexture::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Coordinates);
}

bool UMaterialExpressionScreenPosition::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(ScreenAlign);
  return false;
}

void UMaterialExpressionScreenPosition::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(FString::Sprintf("ScreenAlign: %s", ScreenAlign ? "true" : "false"));
}

bool UMaterialExpressionSine::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_FLOAT_PROP(Period);
  return false;
}

void UMaterialExpressionSine::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  visitor.SetValue(FString::Sprintf("Period:%f", Period));
}

void UMaterialExpressionSine::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
  if (PeriodProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_Period, Period));
}

bool UMaterialExpressionSphereMask::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  REGISTER_INPUT(Radius);
  REGISTER_FLOAT_PROP(AttenuationRadius);
  REGISTER_FLOAT_PROP(HardnessPercent);
  return false;
}

void UMaterialExpressionSphereMask::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B, Radius);
  visitor.SetValue(FString::Sprintf("AttRadius:%f\nHrdsPercent:%f", AttenuationRadius, HardnessPercent));
}

void UMaterialExpressionSphereMask::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(A);
  output.Parameters.emplace_back(B);
  output.Parameters.emplace_back(Radius);
  if (AttenuationRadiusProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_AttenuationRadius, AttenuationRadius));
  if (HardnessPercentProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_HardnessPercent, HardnessPercent));
}

bool UMaterialExpressionSquareRoot::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  return false;
}

void UMaterialExpressionSquareRoot::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
}

void UMaterialExpressionSquareRoot::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
}

bool UMaterialExpressionSubtract::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(A);
  REGISTER_INPUT(B);
  return false;
}

void UMaterialExpressionSubtract::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A, B);
}

void UMaterialExpressionSubtract::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(A);
  output.Parameters.emplace_back(B);
}

bool UMaterialExpressionTerrainLayerCoords::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_ENUM_STR_PROP(MappingType);
  REGISTER_FLOAT_PROP(MappingScale);
  REGISTER_FLOAT_PROP(MappingRotation);
  REGISTER_FLOAT_PROP(MappingPanU);
  REGISTER_FLOAT_PROP(MappingPanV);
  return false;
}

void UMaterialExpressionTerrainLayerCoords::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(FString::Sprintf("Type: %s\nScale:%f\nRotation:%f\nPan:%f,%f", MappingType.UTF8().c_str(), MappingScale, MappingRotation, MappingPanU, MappingPanV));
}

void UMaterialExpressionTerrainLayerCoords::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (MappingTypeProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_MappingType, MappingType.String()));
  if (MappingScaleProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_MappingScale, MappingScale));
  if (MappingRotationProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_MappingRotation, MappingRotation));
  if (MappingPanUProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_MappingPanU, MappingPanU));
  if (MappingPanVProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_MappingPanV, MappingPanV));
}

bool UMaterialExpressionTerrainLayerWeight::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Base);
  REGISTER_INPUT(Layer);
  REGISTER_NAME_PROP(ParameterName);
  return false;
}

void UMaterialExpressionTerrainLayerWeight::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Base, Layer);
  visitor.SetValue(ParameterName.String());
}

void UMaterialExpressionTerrainLayerWeight::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Base);
  output.Parameters.emplace_back(Layer);
  if (ParameterNameProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_ParameterName, ParameterName.String()));
}

bool UMaterialExpressionLandscapeLayerBlend::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  if (PROP_IS(property, Layers))
  {
    for (FPropertyValue* v : property->GetArray())
    {
      Layers.emplace_back().LoadFromPropertyValue(v);
    }
    return true;
  }
  return false;
}

void UMaterialExpressionLandscapeLayerBlend::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  std::vector<FExpressionInput> inputs;
  for (const FLandscapeLayerBlendInput& layer : Layers)
  {
    inputs.emplace_back(layer.LayerInput).CustomDescription = layer.LayerName.String() + "_In";
    if (layer.HeightInput.Expression)
    {
      inputs.emplace_back(layer.HeightInput).CustomDescription = layer.LayerName.String() + "_HeightIn";
    }
  }
  visitor.SetInput(inputs);
}

void UMaterialExpressionLandscapeLayerBlend::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
}

bool UMaterialExpressionTextureCoordinate::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INT_PROP(CoordinateIndex);
  REGISTER_FLOAT_PROP(UTiling);
  REGISTER_FLOAT_PROP(VTiling);
  REGISTER_BOOL_PROP(UnMirrorU);
  REGISTER_BOOL_PROP(UnMirrorV);
  return false;
}

void UMaterialExpressionTextureCoordinate::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(FString::Sprintf("CoordIdx: %d\nTilingUV:%f,%f\nUnMirrorUV: %s, %s", CoordinateIndex, UTiling, VTiling, UnMirrorU ? "true" : "false", UnMirrorV ? "true" : "false"));
}

void UMaterialExpressionTextureCoordinate::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (CoordinateIndexProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_CoordinateIndex, CoordinateIndex));
  if (UTilingProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_UTiling, UTiling));
  if (VTilingProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_VTiling, VTiling));
  if (UnMirrorUProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_UnMirrorU, UnMirrorU));
  if (UnMirrorVProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_UnMirrorV, UnMirrorV));
}

bool UMaterialExpressionTextureSample::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Coordinates);
  REGISTER_INPUT(TextureObject);
  REGISTER_OBJ_PROP(Texture);
  return false;
}

void UMaterialExpressionTextureSample::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  bool needsTexture = true;
  if (TextureObject.Expression)
  {
    SET_INPUT(Coordinates, TextureObject);
    needsTexture = false;
  }
  else if (Coordinates.Expression)
  {
    SET_INPUT(Coordinates);
  }
  if (needsTexture)
  {
    visitor.SetValue(Texture ? Texture->GetObjectNameString() : "NULL");
  }
}

void UMaterialExpressionTextureSample::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Coordinates);
  if (TextureProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_Texture, Texture));
}

bool UMaterialExpressionDepthBiasBlend::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Bias);
  REGISTER_BOOL_PROP(bNormalize);
  REGISTER_FLOAT_PROP(BiasScale);
  return false;
}

void UMaterialExpressionDepthBiasBlend::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Bias);
  visitor.SetValue(FString::Sprintf("bNormalize: %s\nBiasScale:%f", bNormalize ? "true" : "false", BiasScale));
}

void UMaterialExpressionDepthBiasBlend::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Bias);
  if (BiasScaleProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_BiasScale, BiasScale));
  if (bNormalizeProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_bNormalize, bNormalize));
}

bool UMaterialExpressionAntialiasedTextureMask::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_FLOAT_PROP(Threshold);
  REGISTER_ENUM_STR_PROP(Channel);
  return false;
}

void UMaterialExpressionAntialiasedTextureMask::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(FString::Sprintf("Threshold:%f\nChannel: %s", Threshold, Channel.UTF8().c_str()));
}

void UMaterialExpressionAntialiasedTextureMask::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (ThresholdProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_Threshold, Threshold));
  if (ChannelProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_Channel, Channel));
}

bool UMaterialExpressionTime::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_BOOL_PROP(bIgnorePause);
  return false;
}

void UMaterialExpressionTime::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(FString::Sprintf("bIgnorePause: %s", bIgnorePause ? "true" : "false"));
}

void UMaterialExpressionTime::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (bIgnorePauseProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_bIgnorePause, bIgnorePause));
}

bool UMaterialExpressionTransform::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_ENUM_STR_PROP(TransformSourceType);
  REGISTER_ENUM_STR_PROP(TransformType);
  return false;
}

void UMaterialExpressionTransform::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  visitor.SetValue(FString::Sprintf("SourceType: %s\nType: %s", TransformSourceType.UTF8().c_str(), TransformType.UTF8().c_str()));
}

void UMaterialExpressionTransform::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
  if (TransformSourceTypeProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_TransformSourceType, TransformSourceType));
  if (TransformTypeProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_TransformType, TransformType));
}

bool UMaterialExpressionTransformPosition::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_INPUT(Input);
  REGISTER_ENUM_STR_PROP(TransformType);
  return false;
}

void UMaterialExpressionTransformPosition::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(Input);
  visitor.SetValue(FString::Sprintf("Type: %s", TransformType.UTF8().c_str()));
}

void UMaterialExpressionTransformPosition::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  output.Parameters.emplace_back(Input);
  if (TransformTypeProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_TransformType, TransformType));
}

bool UMaterialExpressionTextureSampleParameter::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_NAME_PROP(ParameterName);
  return false;
}

void UMaterialExpressionTextureSampleParameter::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(ParameterName.String());
}

void UMaterialExpressionTextureSampleParameter::ExportExpression(AMaterialExpression& output)
{
  Super::ExportExpression(output);
  if (ParameterNameProperty)
  output.Parameters.emplace_back(AExpressionInput::MakeExpressionInput(P_ParameterName, ParameterName.String()));
}

AExpressionInput::AExpressionInput(const FExpressionInput& input)
  : Name(input.Title)
  , Type("expression")
  , Expression(input.Expression ? input.Expression->GetExportObject()->ObjectIndex : INDEX_NONE)
  , Mask(input.Mask)
  , MaskR(input.MaskR)
  , MaskG(input.MaskG)
  , MaskB(input.MaskB)
  , MaskA(input.MaskA)
{}

void FLandscapeLayerBlendInput::LoadFromPropertyValue(const FPropertyValue* value)
{
  for (FPropertyValue* v : value->GetArray())
  {
    if (v->GetPropertyTagPtr()->Name == "LayerName")
    {
      LayerName = v->GetPropertyTagPtr()->GetName();
    }
    else if (v->GetPropertyTagPtr()->Name == "BlendType")
    {
      BlendType = (LayerBlendType)(v->GetPropertyTagPtr()->GetByte());
    }
    else if (v->GetPropertyTagPtr()->Name == "PreviewWeight")
    {
      PreviewWeight = v->GetPropertyTagPtr()->GetFloat();
    }
    else if (v->GetPropertyTagPtr()->Name == "LayerInput")
    {
      LayerInput = FExpressionInput(v->GetPropertyTagPtr());
      LayerInput.CustomDescription = "LayerIn";
    }
    else if (v->GetPropertyTagPtr()->Name == "HeightInput")
    {
      HeightInput = FExpressionInput(v->GetPropertyTagPtr());
      HeightInput.CustomDescription = "HeightIn";
    }
  }
}

bool UMaterialExpressionFunctionInput::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_STR_PROP(InputName);
  REGISTER_ENUM_PROP(InputType, EFunctionInputType);
  REGISTER_INPUT(Preview);
  REGISTER_VEC4_PROP(PreviewValue);
  REGISTER_INT_PROP(SortPriority);
  REGISTER_TOBJ_PROP(Function, UMaterialFunction*);
  REGISTER_BOOL_PROP(bUsePreviewValueAsDefault);
  return false;
}

void UMaterialExpressionFunctionInput::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  if (Preview.Expression)
  {
    SET_INPUT(Preview);
  }
  visitor.SetTitle("FuncIn: " + InputName);
  FString inType = "Type: ";
  FString prevVal;
  switch (InputType)
  {
  case FunctionInput_Scalar:
    inType += "Scalar";
    if (PreviewValueProperty)
    {
      prevVal = FString::Sprintf("Preview: %.3f", PreviewValue.X);
    }
    break;
  case FunctionInput_Vector2:
    inType += "Vector2D";
    if (PreviewValueProperty)
    {
      prevVal = FString::Sprintf("Preview: (%.2f, %.2f)", PreviewValue.X, PreviewValue.Y);
    }
    break;
  case FunctionInput_Vector3:
    inType += "Vector";
    if (PreviewValueProperty)
    {
      prevVal = FString::Sprintf("Preview: (%.2f, %.2f, %.2f)", PreviewValue.X, PreviewValue.Y, PreviewValue.Z);
    }
    break;
  case FunctionInput_Vector4:
    inType += "Vector4";
    if (PreviewValueProperty)
    {
      prevVal = FString::Sprintf("Preview: (%.2f, %.2f, %.2f, %.2f)", PreviewValue.X, PreviewValue.Y, PreviewValue.Z, PreviewValue.W);
    }
    break;
  case FunctionInput_Texture2D:
    inType += "Texture2D";
    break;
  case FunctionInput_TextureCube:
    inType += "TextureCube";
    break;
  case FunctionInput_StaticBool:
    inType += "Bool";
    if (PreviewValueProperty)
    {
      prevVal = FString::Sprintf("Preview: %s", PreviewValue.X ? "true" : "false");
    }
    break;
  default:
    inType += "Unk";
    break;
  }
  FString val = inType + FString::Sprintf("\nUsePreviewAsDef = %s", bUsePreviewValueAsDefault ? "true" : "false");
  if (prevVal.Length())
  {
    val += "\n";
    val += prevVal;
  }
  visitor.SetValue(val);
}

bool UMaterialExpressionFunctionOutput::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_STR_PROP(OutputName);
  REGISTER_INPUT(A);
  REGISTER_INT_PROP(SortPriority);
  REGISTER_TOBJ_PROP(Function, UMaterialFunction*);
  return false;
}

void UMaterialExpressionFunctionOutput::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  SET_INPUT(A);
  visitor.SetTitle("FuncOut: " + OutputName);
  visitor.SetIsFinalNode();
}

void FFunctionExpressionInput::LoadFromPropertyValue(const FPropertyValue* value)
{
  for (const FPropertyValue* v : value->GetArray())
  {
    if (v->GetPropertyTagPtr()->Name == "ExpressionInput")
    {
      ExpressionInput = Cast<UMaterialExpression>(v->GetPropertyTagPtr()->Value->GetObjectValuePtr());
    }
    else if (v->GetPropertyTagPtr()->Name == "ExpressionInputId")
    {
      ExpressionInputId = v->GetPropertyTagPtr()->GetGuid();
    }
    else if (v->GetPropertyTagPtr()->Name == "Input")
    {
      Input = FExpressionInput(v->GetPropertyTagPtr());
    }
  }
}

void FFunctionExpressionOutput::LoadFromPropertyValue(const FPropertyValue* value)
{
  for (const FPropertyValue* v : value->GetArray())
  {
    if (v->GetPropertyTagPtr()->Name == "ExpressionOutput")
    {
      ExpressionOutput = Cast<UMaterialExpression>(v->GetPropertyTagPtr()->Value->GetObjectValuePtr());
    }
    else if (v->GetPropertyTagPtr()->Name == "ExpressionOutputId")
    {
      ExpressionOutputId = v->GetPropertyTagPtr()->GetGuid();
    }
    else if (v->GetPropertyTagPtr()->Name == "Output")
    {
      Output = FExpressionInput(v->GetPropertyTagPtr());
    }
  }
}

bool UMaterialExpressionMaterialFunctionCall::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(MaterialFunction, UMaterialFunction*);
  if (PROP_IS(property, FunctionInputs))
  {
    FunctionInputsProperty = property;
    for (FPropertyValue* v : FunctionInputsProperty->GetArray())
    {
      FunctionInputs.emplace_back().LoadFromPropertyValue(v);
    }
    return true;
  }
  if (PROP_IS(property, FunctionOutputs))
  {
    FunctionOutputsProperty = property;
    for (FPropertyValue* v : FunctionOutputsProperty->GetArray())
    {
      FunctionOutputs.emplace_back().LoadFromPropertyValue(v);
    }
    return true;
  }
  REGISTER_TOBJ_PROP(Function, UMaterialFunction*);
  REGISTER_TOBJ_PROP(Material, UMaterial*);
  return false;
}

void UMaterialExpressionMaterialFunctionCall::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  std::vector<FExpressionInput> inputs;
  for (const FFunctionExpressionInput& i : FunctionInputs)
  {
    inputs.emplace_back(i.Input);
  }
  visitor.SetInput(inputs);
  inputs.clear();
  for (const FFunctionExpressionOutput& i : FunctionOutputs)
  {
    inputs.emplace_back(i.Output);
  }
  visitor.SetOutput(inputs);
  visitor.SetTitle("MFCall: " + MaterialFunction->GetObjectNameString());
}

bool UMaterialExpressionTextureObject::RegisterProperty(FPropertyTag* property)
{
  SUPER_REGISTER_PROP();
  REGISTER_TOBJ_PROP(Texture, UTexture2D*);
  return false;
}

void UMaterialExpressionTextureObject::AcceptVisitor(UMaterialExpressionViewVisitor& visitor)
{
  SUPER_ACCEPT();
  visitor.SetValue(Texture ? Texture->GetObjectNameString() : "NULL");
}
