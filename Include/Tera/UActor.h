#pragma once
#include <Tera/UObject.h>
#include <Tera/UActorComponent.h>

class UActor : public UObject {
public:
  DECL_UOBJ(UActor, UObject);

  UPROP(std::vector<UActorComponent*>, Components, {});
  UPROP_NOINIT(FVector, PrePivot);
  UPROP_NOINIT(FVector, Location);
  UPROP_NOINIT(FRotator, Rotation);
  UPROP(FVector, DrawScale3D, FVector(1, 1, 1));
  UPROP(float, DrawScale, 1.);
  UPROP(bool, bHidden, false);
  UPROP(bool, bCollideActors, true);
  UPROP_NOINIT(FName, Layer);

  // Location with unbaked prepivot transform
  FVector GetLocation(); 

  std::vector<FString> GetLayers() const;
  
  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
};

class UStaticMeshComponent;
class UStaticMeshActor : public UActor {
public:
  DECL_UOBJ(UStaticMeshActor, UActor);

  UPROP(UStaticMeshComponent*, StaticMeshComponent, nullptr);
  
  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
};

class USkeletalMeshComponent;
class USkeletalMeshActor : public UActor {
public:
  DECL_UOBJ(USkeletalMeshActor, UActor);
  USkeletalMeshActor(FObjectExport* exp);

  UPROP(USkeletalMeshComponent*, SkeletalMeshComponent, nullptr);
  
  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
};

class USpeedTreeComponent;
class USpeedTreeActor : public UActor {
public:
  DECL_UOBJ(USpeedTreeActor, UActor);

  UPROP(USpeedTreeComponent*, SpeedTreeComponent, nullptr);

  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
};

class UStaticMesh;
class UDynamicSMActor : public UActor {
public:
  DECL_UOBJ(UDynamicSMActor, UActor);
  UDynamicSMActor(FObjectExport* exp);

  UPROP(UStaticMeshComponent*, StaticMeshComponent, nullptr);
  UPROP(UStaticMesh*, ReplicatedMesh, nullptr);
  UPROP_NOINIT(FVector, ReplicatedMeshTranslation);
  UPROP_NOINIT(FRotator, ReplicatedMeshRotation);
  UPROP(FVector, ReplicatedMeshScale3D, FVector(1, 1, 1));

  bool RegisterProperty(FPropertyTag* property) override;
  void PostLoad() override;
};

class UInterpActor : public UDynamicSMActor {
public:
  DECL_UOBJ(UInterpActor, UDynamicSMActor);
};

class UEmitter : public UActor {
public:
  DECL_UOBJ(UEmitter, UActor);
};

class UHeightFog : public UActor {
public:
  DECL_UOBJ(UHeightFog, UActor);

  UPROP(UHeightFogComponent*, Component, nullptr);

  bool RegisterProperty(FPropertyTag* property) override;

  void PostLoad() override;
};

class USkeletalMeshCinematicActor : public USkeletalMeshActor {
public:
  DECL_UOBJ(USkeletalMeshCinematicActor, USkeletalMeshActor);
};

class USkeletalMeshActorMAT : public USkeletalMeshCinematicActor {
public:
  DECL_UOBJ(USkeletalMeshActorMAT, USkeletalMeshCinematicActor);
};

class UPASkeletalMeshActorMAT : public USkeletalMeshActorMAT {
public:
  DECL_UOBJ(UPASkeletalMeshActorMAT, USkeletalMeshActorMAT);
};