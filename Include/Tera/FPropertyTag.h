#pragma once
#include <Tera/Core.h>
#include <Tera/FName.h>
#include <Tera/FStream.h>
#include <Tera/FString.h>
#include <Tera/FStructs.h>

struct FPropertyTag;
class UObject;
class UField;
class UProperty;
struct FPropertyValue {

  FPropertyValue()
  {}

  FPropertyValue(FPropertyTag* property)
    : Property(property)
  {}

  FPropertyValue(FPropertyTag* property, UField* field)
    : Property(property)
    , Field(field)
  {}

  enum class VID
  {
    Unk,
    None,
    Bool,
    Byte,
    Int,
    Float,
    Object,
    Delegate,
    Property,
    Name,
    String,
    Struct,
    Field,
    Array,
  };

  inline bool* GetBoolPtr()
  {
    return (bool*)Data;
  }

  inline uint8* GetBytePtr()
  {
    return (uint8*)Data;
  }

  inline uint32* GetIntPtr()
  {
    return (uint32*)Data;
  }

  inline float* GetFloatPtr()
  {
    return (float*)Data;
  }

  inline FName* GetNamePtr()
  {
    return (FName*)Data;
  }

  inline FString* GetStringPtr()
  {
    return (FString*)Data;
  }

  inline PACKAGE_INDEX* GetObjectIdxPtr()
  {
    return (PACKAGE_INDEX*)Data;
  }

  inline FScriptDelegate* GetScriptDelegatePtr()
  {
    return (FScriptDelegate*)Data;
  }

  inline FPropertyTag* GetPropertyTagPtr()
  {
    return (FPropertyTag*)Data;
  }

  inline std::vector<FPropertyValue*>* GetArrayPtr()
  {
    return (std::vector<FPropertyValue*>*)Data;
  }

  inline bool& GetBool()
  {
    return *(bool*)Data;
  }

  inline uint8& GetByte()
  {
    return *(uint8*)Data;
  }

  inline uint32& GetInt()
  {
    return *(uint32*)Data;
  }

  inline float& GetFloat()
  {
    return *(float*)Data;
  }

  inline FName& GetName()
  {
    return *(FName*)Data;
  }

  inline FString& GetString()
  {
    return *(FString*)Data;
  }

  inline PACKAGE_INDEX& GetObjectIndex()
  {
    return *(PACKAGE_INDEX*)Data;
  }

  inline FScriptDelegate& GetScriptDelegate()
  {
    return *(FScriptDelegate*)Data;
  }

  inline FPropertyTag& GetPropertyTag()
  {
    return *(FPropertyTag*)Data;
  }

  inline std::vector<FPropertyValue*>& GetArray()
  {
    return *(std::vector<FPropertyValue*>*)Data;
  }

  inline bool& GetBool() const
  {
    return *(bool*)Data;
  }

  inline const uint8& GetByte() const
  {
    return *(uint8*)Data;
  }

  inline const uint32& GetInt() const
  {
    return *(uint32*)Data;
  }

  inline const float& GetFloat() const
  {
    return *(float*)Data;
  }

  inline const FName& GetName() const
  {
    return *(FName*)Data;
  }

  inline const FString& GetString() const
  {
    return *(FString*)Data;
  }

  inline const PACKAGE_INDEX& GetObjectIndex() const
  {
    return *(PACKAGE_INDEX*)Data;
  }

  inline const FScriptDelegate& GetScriptDelegate() const
  {
    return *(FScriptDelegate*)Data;
  }

  inline const FPropertyTag& GetPropertyTag() const
  {
    return *(FPropertyTag*)Data;
  }

  inline const FPropertyTag* GetPropertyTagPtr() const
  {
    return (FPropertyTag*)Data;
  }

  inline const std::vector<FPropertyValue*>& GetArray() const
  {
    return *(std::vector<FPropertyValue*>*)Data;
  }

  template <typename T>
  T& GetTypedValue()
  {
    return *(T*)Data;
  }

  bool GetVector2D(FVector2D& output);

  bool GetVector(FVector& output);

  bool GetVector4(FVector4& output);

  UObject* GetObjectValuePtr(bool load = true);

  ~FPropertyValue();

  void RegisterEnumNames();

  FPropertyTag* FindSubProperty(const FString& name);

  VID Type = VID::None;
  void* Data = nullptr;
  UEnum* Enum = nullptr;
  UField* Field = nullptr;
  UStruct* Struct = nullptr;
  FPropertyTag* Property = nullptr;
};

struct FPropertyTag {
  FName Type;
  uint8 BoolVal = 0;
  FName Name;
  FName StructName;
  FName EnumName;
  int32 Size = 0;
  int32 ArrayIndex = 0;
  int32 ArrayDim = 1;
  int32 SizeOffset = 0;
  
  UObject* Owner = nullptr;
  FPropertyValue* Value = nullptr;
  FPropertyTag* StaticArrayNext = nullptr;
  UProperty* ClassProperty = nullptr;

  FPropertyTag()
  {
    NewValue();
  }

  FPropertyTag(UObject* owner)
  {
    Owner = owner;
    NewValue();
  }

  FPropertyTag(UObject* owner, const FString& name, const FString& type);

  ~FPropertyTag()
  {
    if (Value)
    {
      delete Value;
    }
  }

  void NewValue()
  {
    if (Value)
    {
      delete Value;
    }
    Value = new FPropertyValue(this);
  }

  inline uint8& GetBool()
  {
    return BoolVal;
  }

  inline uint8& GetByte()
  {
    return Value->GetByte();
  }

  inline uint32& GetInt()
  {
    return Value->GetInt();
  }

  inline float& GetFloat()
  {
    return Value->GetFloat();
  }

  inline FName& GetName()
  {
    return Value->GetName();
  }

  inline FString& GetString()
  {
    return Value->GetString();
  }

  inline PACKAGE_INDEX& GetObjectIndex()
  {
    return Value->GetObjectIndex();
  }

  inline FScriptDelegate& GetScriptDelegate()
  {
    return Value->GetScriptDelegate();
  }

  inline FPropertyTag& GetPropertyTag()
  {
    return Value->GetPropertyTag();
  }

  inline std::vector<FPropertyValue*>& GetArray()
  {
    return Value->GetArray();
  }

  inline FPropertyTag* GetPropertyTagPtr()
  {
    return Value->GetPropertyTagPtr();
  }

  inline const uint8& GetBool() const
  {
    return BoolVal;
  }

  inline const uint8& GetByte() const
  {
    return Value->GetByte();
  }

  inline const uint32& GetInt() const
  {
    return Value->GetInt();
  }

  inline const float& GetFloat() const
  {
    return Value->GetFloat();
  }

  inline const FName& GetName() const
  {
    return Value->GetName();
  }

  inline const FString& GetString() const
  {
    return Value->GetString();
  }

  inline const PACKAGE_INDEX& GetObjectIndex() const
  {
    return Value->GetObjectIndex();
  }

  inline const FScriptDelegate& GetScriptDelegate() const
  {
    return Value->GetScriptDelegate();
  }

  inline const FPropertyTag& GetPropertyTag() const
  {
    return Value->GetPropertyTag();
  }

  inline const std::vector<FPropertyValue*>& GetArray() const
  {
    return Value->GetArray();
  }

  inline const FPropertyTag* GetPropertyTagPtr() const
  {
    return Value->GetPropertyTagPtr();
  }

  inline UObject* GetObjectValuePtr()
  {
    return Value->GetObjectValuePtr();
  }

  FGuid GetGuid() const;

  void* GetValueData()
  {
    return Value ? Value->Data : nullptr;
  }

  template <typename T>
  T& GetTypedValue()
  {
    if (Type.String() == NAME_BoolProperty)
    {
      return *(T*)&BoolVal;
    }
    return *(T*)(Value->Data);
  }

  template <typename T>
  const T& GetTypedValue() const
  {
    if (Type.String() == NAME_BoolProperty)
    {
      return *(T*)&BoolVal;
    }
    return *(T*)(Value->Data);
  }

  bool GetVector2D(FVector2D& output) const;
  bool GetVector(FVector& output) const;
  bool GetVector4(FVector4& output) const;
  bool GetRotator(FRotator& output) const;
  bool GetLinearColor(FLinearColor& output) const;
  bool GetColor(FColor& output) const;

  friend FStream& operator<<(FStream& s, FPropertyTag& tag);
};