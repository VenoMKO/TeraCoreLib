#pragma once

template<typename T>
T* Cast(const UObject* obj)
{
  return obj && obj->IsA(T::StaticClassName()) ? (T*)obj : nullptr;
}

template< class T >
inline T* ExactCast(UObject* obj)
{
  return obj && (obj->GetStaticClassName() == T::StaticClassName()) ? (T*)obj : nullptr;
}

template< class T, class U >
T* CastChecked(U* obj)
{
#if _DEBUG
  if (!obj || !obj->IsA(T::StaticClassName()))
  {
    LogE("Cast of %s to %s failed", obj ? obj->GetObjectNameString().UTF8().c_str() : "NULL", T::StaticClassName());
  }
#endif
  return (T*)obj;
}