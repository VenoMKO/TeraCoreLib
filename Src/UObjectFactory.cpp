#include "Tera/UObject.h"
#include "Tera/UObjectFactory.h"
#include "Tera/FObjectResource.h"
#include "Tera/FName.h"

#include "Tera/UClass.h"
#include "Tera/UProperty.h"
#include "Tera/UMetaData.h"

// Components
#include "Tera/UComponent.h"

// Objects
#include "Tera/UObjectRedirector.h"
#include "Tera/UMaterial.h"
#include "Tera/UMaterialFunction.h"
#include "Tera/UMaterialExpression.h"
#include "Tera/UPersistentCookerData.h"
#include "Tera/UTexture.h"
#include "Tera/USkeletalMesh.h"
#include "Tera/UStaticMesh.h"
#include "Tera/USoundNode.h"
#include "Tera/UAnimation.h"
#include "Tera/UPhysAsset.h"
#include "Tera/ULevelStreaming.h"
#include "Tera/USpeedTree.h"
#include "Tera/UPrefab.h"
#include "Tera/UObjectReferencer.h"
#include "Tera/UModel.h"
#include "Tera/UInstancedFoliageActor.h"

#include "Tera/UActor.h"
#include "Tera/ULevel.h"
#include "Tera/ULandscape.h"
#include "Tera/UTerrain.h"
#include "Tera/ULight.h"

#define TRANSLATION_UNIT_VALIDATION 1
std::map<std::size_t, UObjectFactory::TCtor> UObjectFactory::RegisteredClasses;

UObject* UObject::Object(FObjectExport* exp)
{
  const FString c = exp->GetClassNameString();
  if (c == UClass::StaticClassName())
  {
    return new UClass(exp, false);
  }

  if ((c.Find("Distribution") != std::string::npos) ||
      (c.StartsWith("UIComp_") || c == "RB_Handle"))
  {
    return new UComponent(exp);
  }

#if IS_ASTELLIA_BUILD
  if (c.StartsWith("PA") && (c.EndsWith("Agent") || c.EndsWith("Agent2")))
  {
    return new UComponent(exp);
  }
#endif

  if (UObject* result = UObjectFactory::Create(c, exp))
  {
    return result;
  }

  if (c.Find(UComponent::StaticClassName()) != std::string::npos)
  {
    return new UComponent(exp);
  }

  if (c.StartsWith("MaterialExpression"))
  {
    return new UMaterialExpression(exp);
  }

  if (c.Find(NAME_Actor) != std::string::npos && c != NAME_ActorFactory)
  {
    return new UActor(exp);
  }
  return new UObject(exp);
}

bool UObjectFactory::Register(const char* className, TCtor ctor)
{
  size_t hash = std::hash<std::string>{}(className);
  if (auto it = RegisteredClasses.find(hash); it == RegisteredClasses.end())
  {
    RegisteredClasses[hash] = ctor;
    return true;
  }
  return false;
}

UObject* UObjectFactory::Create(const FString& className, FObjectExport* exp)
{
  size_t hash = std::hash<std::string>{}(className.UTF8());
  if (auto it = RegisteredClasses.find(hash); it != RegisteredClasses.end())
  {
    return it->second(exp);
  }
  return nullptr;
}

// All impls must be in the same translation unit as the class map to prevent static initialization order fiasco

IMPL_UOBJ(UActor); // UActor
IMPL_UOBJ(UAeroInnerVolume); // UAeroInnerVolume
IMPL_UOBJ(UAeroVolume); // UAeroVolume
IMPL_UOBJ(UAmbientSound); // UAmbientSound
IMPL_UOBJ(UAmbientSoundMovable); // UAmbientSoundMovable
IMPL_UOBJ(UAmbientSoundNonLoop); // UAmbientSoundNonLoop
IMPL_UOBJ(UAmbientSoundNonLoopingToggleable); // UAmbientSoundNonLoopingToggleable
IMPL_UOBJ(UAmbientSoundSimple); // UAmbientSoundSimple
IMPL_UOBJ(UAmbientSoundSimpleToggleable); // UAmbientSoundSimpleToggleable
IMPL_UOBJ(UAnimSequence); // UAnimSequence
IMPL_UOBJ(UAnimSet); // UAnimSet
IMPL_UOBJ(UArrayProperty); // UArrayProperty
IMPL_UOBJ(UAudioComponent); // UAudioComponent
IMPL_UOBJ(UBlockingVolume); // UBlockingVolume
IMPL_UOBJ(UBoolProperty); // UBoolProperty
IMPL_UOBJ(UBrush); // UBrush
IMPL_UOBJ(UBrushComponent); // UBrushComponent
IMPL_UOBJ(UByteProperty); // UByteProperty
IMPL_UOBJ(UClassProperty); // UClassProperty
IMPL_UOBJ(UComponent); // UComponent
IMPL_UOBJ(UComponentProperty); // UComponentProperty
IMPL_UOBJ(UConst); // UConst
IMPL_UOBJ(UDelegateProperty); // UDelegateProperty
IMPL_UOBJ(UDirectionalLight); // UDirectionalLight
IMPL_UOBJ(UDirectionalLightComponent); // UDirectionalLightComponent
IMPL_UOBJ(UDirectionalLightToggleable); // UDirectionalLightToggleable
IMPL_UOBJ(UDistributionFloatConstant); // UDistributionFloatConstant
IMPL_UOBJ(UDistributionFloatUniform); // UDistributionFloatUniform
IMPL_UOBJ(UDominantDirectionalLightComponent); // UDominantDirectionalLightComponent
IMPL_UOBJ(UDominantSpotLightComponent); // UDominantSpotLightComponent
IMPL_UOBJ(UDynamicSMActor); // UDynamicSMActor
IMPL_UOBJ(UEmitter); // UEmitter
IMPL_UOBJ(UEnum); // UEnum
IMPL_UOBJ(UField); // UField
IMPL_UOBJ(UFloatProperty); // UFloatProperty
IMPL_UOBJ(UFunction); // UFunction
IMPL_UOBJ(UHeightFog); // UHeightFog
IMPL_UOBJ(UHeightFogComponent); // UHeightFogComponent
IMPL_UOBJ(UInstancedFoliageActor); // UInstancedFoliageActor
IMPL_UOBJ(UInterfaceProperty); // UInterfaceProperty
IMPL_UOBJ(UInterpActor); // UInterpActor
IMPL_UOBJ(UIntProperty); // UIntProperty
IMPL_UOBJ(ULandscape); // ULandscape
IMPL_UOBJ(ULandscapeComponent); // ULandscapeComponent
IMPL_UOBJ(ULandscapeLayerInfoObject); // ULandscapeLayerInfoObject
IMPL_UOBJ(ULevel); // ULevel
IMPL_UOBJ(ULevelStreamingAlwaysLoaded); // ULevelStreamingAlwaysLoaded
IMPL_UOBJ(ULevelStreamingDistance); // ULevelStreamingDistance
IMPL_UOBJ(ULevelStreamingKismet); // ULevelStreamingKismet
IMPL_UOBJ(ULevelStreamingPersistent); // ULevelStreamingPersistent
IMPL_UOBJ(ULevelStreamingVolume); // ULevelStreamingVolume
IMPL_UOBJ(ULight); // ULight
IMPL_UOBJ(ULightComponent); // ULightComponent
IMPL_UOBJ(ULightMapTexture2D); // ULightMapTexture2D
IMPL_UOBJ(ULightmassImportanceVolume); // ULightmassImportanceVolume
IMPL_UOBJ(UMapProperty); // UMapProperty
IMPL_UOBJ(UMaterial); // UMaterial
IMPL_UOBJ(UMaterialFunction); // UMaterialFunction
IMPL_UOBJ(UMaterialInstance); // UMaterialInstance
IMPL_UOBJ(UMaterialInstanceConstant); // UMaterialInstanceConstant
IMPL_UOBJ(UMetaData); // UMetaData
IMPL_UOBJ(UModel); // UModel
IMPL_UOBJ(UNameProperty); // UNameProperty
IMPL_UOBJ(UObjectProperty); // UObjectProperty
IMPL_UOBJ(UObjectRedirector); // UObjectRedirector
IMPL_UOBJ(UObjectReferencer); // UObjectReferencer
IMPL_UOBJ(UPackage); // UPackage
IMPL_UOBJ(UPersistentCookerData); // UPersistentCookerData
IMPL_UOBJ(UPhysicsAssetInstance); // UPhysicsAssetInstance
IMPL_UOBJ(UPointLight); // UPointLight
IMPL_UOBJ(UPointLightComponent); // UPointLightComponent
IMPL_UOBJ(UPointLightMovable); // UPointLightMovable
IMPL_UOBJ(UPointLightToggleable); // UPointLightToggleable
IMPL_UOBJ(UPolys); // UPolys
IMPL_UOBJ(UPostProcessVolume); // UPostProcessVolume
IMPL_UOBJ(UPrefab); // UPrefab
IMPL_UOBJ(UPrefabInstance); // UPrefabInstance
IMPL_UOBJ(URB_BodySetup); // URB_BodySetup
IMPL_UOBJ(UReverbVolume); // UReverbVolume
IMPL_UOBJ(UScriptStruct); // UScriptStruct
IMPL_UOBJ(UShadowMapTexture2D); // UShadowMapTexture2D
IMPL_UOBJ(USkeletalMesh); // USkeletalMesh
IMPL_UOBJ(USkeletalMeshActor); // USkeletalMeshActor
IMPL_UOBJ(USkeletalMeshActorMAT); // USkeletalMeshActorMAT
IMPL_UOBJ(USkeletalMeshCinematicActor); // USkeletalMeshCinematicActor
IMPL_UOBJ(USkeletalMeshComponent); // USkeletalMeshComponent
IMPL_UOBJ(USkyLight); // USkyLight
IMPL_UOBJ(USkyLightComponent); // USkyLightComponent
IMPL_UOBJ(USkyLightToggleable); // USkyLightToggleable
IMPL_UOBJ(USoundCue); // USoundCue
IMPL_UOBJ(USoundNodeAmbient); // USoundNodeAmbient
IMPL_UOBJ(USoundNodeAmbientNonLoop); // USoundNodeAmbientNonLoop
IMPL_UOBJ(USoundNodeAmbientNonLoopToggle); // USoundNodeAmbientNonLoopToggle
IMPL_UOBJ(USoundNodeAttenuation); // USoundNodeAttenuation
IMPL_UOBJ(USoundNodeConcatenator); // USoundNodeConcatenator
IMPL_UOBJ(USoundNodeLooping); // USoundNodeLooping
IMPL_UOBJ(USoundNodeMixer); // USoundNodeMixer
IMPL_UOBJ(USoundNodeModulator); // USoundNodeModulator
IMPL_UOBJ(USoundNodeRandom); // USoundNodeRandom
IMPL_UOBJ(USoundNodeWave); // USoundNodeWave
IMPL_UOBJ(USpeedTree); // USpeedTree
IMPL_UOBJ(USpeedTreeActor); // USpeedTreeActor
IMPL_UOBJ(USpeedTreeComponent); // USpeedTreeComponent
IMPL_UOBJ(USpotLight); // USpotLight
IMPL_UOBJ(USpotLightComponent); // USpotLightComponent
IMPL_UOBJ(USpotLightMovable); // USpotLightMovable
IMPL_UOBJ(USpotLightToggleable); // USpotLightToggleable
IMPL_UOBJ(UState); // UState
IMPL_UOBJ(UStaticMesh); // UStaticMesh
IMPL_UOBJ(UStaticMeshActor); // UStaticMeshActor
IMPL_UOBJ(UStaticMeshComponent); // UStaticMeshComponent
IMPL_UOBJ(UStrProperty); // UStrProperty
IMPL_UOBJ(UStruct); // UStruct
IMPL_UOBJ(UStructProperty); // UStructProperty
IMPL_UOBJ(UTerrain); // UTerrain
IMPL_UOBJ(UTerrainLayerSetup); // UTerrainLayerSetup
IMPL_UOBJ(UTerrainMaterial); // UTerrainMaterial
IMPL_UOBJ(UTerrainWeightMapTexture); // UTerrainWeightMapTexture
IMPL_UOBJ(UTextBuffer); // UTextBuffer
IMPL_UOBJ(UTexture2D); // UTexture2D
IMPL_UOBJ(UTextureCube); // UTextureCube
IMPL_UOBJ(UTextureFlipBook); // UTextureFlipBook

#if IS_TERA_BUILD
IMPL_UOBJ(US1LevelStreamingBaseLevel); // US1LevelStreamingBaseLevel
IMPL_UOBJ(US1LevelStreamingDistance); // US1LevelStreamingDistance
IMPL_UOBJ(US1LevelStreamingSound); // US1LevelStreamingSound
IMPL_UOBJ(US1LevelStreamingSuperLow); // US1LevelStreamingSuperLow
IMPL_UOBJ(US1LevelStreamingVOID); // US1LevelStreamingVOID
IMPL_UOBJ(US1MusicVolume); // US1MusicVolume
IMPL_UOBJ(US1WaterVolume); // US1WaterVolume
#endif

#if IS_ASTELLIA_BUILD
IMPL_UOBJ(UPAPostProcessVolume); // UPAPostProcessVolume
IMPL_UOBJ(UPASkeletalMeshActorMAT); // UPAPostProcessVolume
#endif

IMPL_UOBJ(UMaterialExpressionAbs); // UMaterialExpressionAbs
IMPL_UOBJ(UMaterialExpressionAdd); // UMaterialExpressionAdd
IMPL_UOBJ(UMaterialExpressionAntialiasedTextureMask); // UMaterialExpressionAntialiasedTextureMask
IMPL_UOBJ(UMaterialExpressionAppendVector); // UMaterialExpressionAppendVector
IMPL_UOBJ(UMaterialExpressionBumpOffset); // UMaterialExpressionBumpOffset
IMPL_UOBJ(UMaterialExpressionCameraVector); // UMaterialExpressionCameraVector
IMPL_UOBJ(UMaterialExpressionCameraWorldPosition); // UMaterialExpressionCameraWorldPosition
IMPL_UOBJ(UMaterialExpressionCeil); // UMaterialExpressionCeil
IMPL_UOBJ(UMaterialExpressionClamp); // UMaterialExpressionClamp
IMPL_UOBJ(UMaterialExpressionComment); // UMaterialExpressionComment
IMPL_UOBJ(UMaterialExpressionComponentMask); // UMaterialExpressionComponentMask
IMPL_UOBJ(UMaterialExpressionCompound); // UMaterialExpressionCompound
IMPL_UOBJ(UMaterialExpressionConstant); // UMaterialExpressionConstant
IMPL_UOBJ(UMaterialExpressionConstant2Vector); // UMaterialExpressionConstant2Vector
IMPL_UOBJ(UMaterialExpressionConstant3Vector); // UMaterialExpressionConstant3Vector
IMPL_UOBJ(UMaterialExpressionConstant4Vector); // UMaterialExpressionConstant4Vector
IMPL_UOBJ(UMaterialExpressionConstantBiasScale); // UMaterialExpressionConstantBiasScale
IMPL_UOBJ(UMaterialExpressionConstantClamp); // UMaterialExpressionConstantClamp
IMPL_UOBJ(UMaterialExpressionCosine); // UMaterialExpressionCosine
IMPL_UOBJ(UMaterialExpressionCrossProduct); // UMaterialExpressionCrossProduct
IMPL_UOBJ(UMaterialExpressionCustomTexture); // UMaterialExpressionCustomTexture
IMPL_UOBJ(UMaterialExpressionDepthBiasBlend); // UMaterialExpressionDepthBiasBlend
IMPL_UOBJ(UMaterialExpressionDepthBiasedAlpha); // UMaterialExpressionDepthBiasedAlpha
IMPL_UOBJ(UMaterialExpressionDepthBiasedBlend); // UMaterialExpressionDepthBiasedBlend
IMPL_UOBJ(UMaterialExpressionDepthOfFieldFunction); // UMaterialExpressionDepthOfFieldFunction
IMPL_UOBJ(UMaterialExpressionDeriveNormalZ); // UMaterialExpressionDeriveNormalZ
IMPL_UOBJ(UMaterialExpressionDesaturation); // UMaterialExpressionDesaturation
IMPL_UOBJ(UMaterialExpressionDestColor); // UMaterialExpressionDestColor
IMPL_UOBJ(UMaterialExpressionDestDepth); // UMaterialExpressionDestDepth
IMPL_UOBJ(UMaterialExpressionDistance); // UMaterialExpressionDistance
IMPL_UOBJ(UMaterialExpressionDivide); // UMaterialExpressionDivide
IMPL_UOBJ(UMaterialExpressionDotProduct); // UMaterialExpressionDotProduct
IMPL_UOBJ(UMaterialExpressionDynamicParameter); // UMaterialExpressionDynamicParameter
IMPL_UOBJ(UMaterialExpressionFlipBookSample); // UMaterialExpressionFlipBookSample
IMPL_UOBJ(UMaterialExpressionFloor); // UMaterialExpressionFloor
IMPL_UOBJ(UMaterialExpressionFluidNormal); // UMaterialExpressionFluidNormal
IMPL_UOBJ(UMaterialExpressionFmod); // UMaterialExpressionFmod
IMPL_UOBJ(UMaterialExpressionFoliageImpulseDirection); // UMaterialExpressionFoliageImpulseDirection
IMPL_UOBJ(UMaterialExpressionFoliageNormalizedRotationAxisAndAngle); // UMaterialExpressionFoliageNormalizedRotationAxisAndAngle
IMPL_UOBJ(UMaterialExpressionFontSample); // UMaterialExpressionFontSample
IMPL_UOBJ(UMaterialExpressionFontSampleParameter); // UMaterialExpressionFontSampleParameter
IMPL_UOBJ(UMaterialExpressionFrac); // UMaterialExpressionFrac
IMPL_UOBJ(UMaterialExpressionFresnel); // UMaterialExpressionFresnel
IMPL_UOBJ(UMaterialExpressionFunctionInput); // UMaterialExpressionFunctionInput
IMPL_UOBJ(UMaterialExpressionFunctionOutput); // UMaterialExpressionFunctionOutput
IMPL_UOBJ(UMaterialExpressionIf); // UMaterialExpressionIf
IMPL_UOBJ(UMaterialExpressionLandscapeLayerBlend); // UMaterialExpressionLandscapeLayerBlend
IMPL_UOBJ(UMaterialExpressionLensFlareIntensity); // UMaterialExpressionLensFlareIntensity
IMPL_UOBJ(UMaterialExpressionLensFlareOcclusion); // UMaterialExpressionLensFlareOcclusion
IMPL_UOBJ(UMaterialExpressionLensFlareRadialDistance); // UMaterialExpressionLensFlareRadialDistance
IMPL_UOBJ(UMaterialExpressionLensFlareRayDistance); // UMaterialExpressionLensFlareRayDistance
IMPL_UOBJ(UMaterialExpressionLensFlareSourceDistance); // UMaterialExpressionLensFlareSourceDistance
IMPL_UOBJ(UMaterialExpressionLightmapUVs); // UMaterialExpressionLightmapUVs
IMPL_UOBJ(UMaterialExpressionLightmassReplace); // UMaterialExpressionLightmassReplace
IMPL_UOBJ(UMaterialExpressionLightVector); // UMaterialExpressionLightVector
IMPL_UOBJ(UMaterialExpressionLinearInterpolate); // UMaterialExpressionLinearInterpolate
IMPL_UOBJ(UMaterialExpressionMaterialFunctionCall); // UMaterialExpressionMaterialFunctionCall
IMPL_UOBJ(UMaterialExpressionMeshEmitterDynamicParameter); // UMaterialExpressionMeshEmitterDynamicParameter
IMPL_UOBJ(UMaterialExpressionMeshEmitterVertexColor); // UMaterialExpressionMeshEmitterVertexColor
IMPL_UOBJ(UMaterialExpressionMeshSubUV); // UMaterialExpressionMeshSubUV
IMPL_UOBJ(UMaterialExpressionMeshSubUVBlend); // UMaterialExpressionMeshSubUVBlend
IMPL_UOBJ(UMaterialExpressionMultiply); // UMaterialExpressionMultiply
IMPL_UOBJ(UMaterialExpressionNormalize); // UMaterialExpressionNormalize
IMPL_UOBJ(UMaterialExpressionObjectOrientation); // UMaterialExpressionObjectOrientation
IMPL_UOBJ(UMaterialExpressionObjectRadius); // UMaterialExpressionObjectRadius
IMPL_UOBJ(UMaterialExpressionObjectWorldPosition); // UMaterialExpressionObjectWorldPosition
IMPL_UOBJ(UMaterialExpressionOcclusionPercentage); // UMaterialExpressionOcclusionPercentage
IMPL_UOBJ(UMaterialExpressionOneMinus); // UMaterialExpressionOneMinus
IMPL_UOBJ(UMaterialExpressionPanner); // UMaterialExpressionPanner
IMPL_UOBJ(UMaterialExpressionParameter); // UMaterialExpressionParameter
IMPL_UOBJ(UMaterialExpressionParticleMacroUV); // UMaterialExpressionParticleMacroUV
IMPL_UOBJ(UMaterialExpressionParticleSubUV); // UMaterialExpressionParticleSubUV
IMPL_UOBJ(UMaterialExpressionPerInstanceRandom); // UMaterialExpressionPerInstanceRandom
IMPL_UOBJ(UMaterialExpressionPixelDepth); // UMaterialExpressionPixelDepth
IMPL_UOBJ(UMaterialExpressionPower); // UMaterialExpressionPower
IMPL_UOBJ(UMaterialExpressionReflectionVector); // UMaterialExpressionReflectionVector
IMPL_UOBJ(UMaterialExpressionRotateAboutAxis); // UMaterialExpressionRotateAboutAxis
IMPL_UOBJ(UMaterialExpressionRotator); // UMaterialExpressionRotator
IMPL_UOBJ(UMaterialExpressionScalarParameter); // UMaterialExpressionScalarParameter
IMPL_UOBJ(UMaterialExpressionSceneDepth); // UMaterialExpressionSceneDepth
IMPL_UOBJ(UMaterialExpressionSceneTexture); // UMaterialExpressionSceneTexture
IMPL_UOBJ(UMaterialExpressionScreenPosition); // UMaterialExpressionScreenPosition
IMPL_UOBJ(UMaterialExpressionSine); // UMaterialExpressionSine
IMPL_UOBJ(UMaterialExpressionSphereMask); // UMaterialExpressionSphereMask
IMPL_UOBJ(UMaterialExpressionSquareRoot); // UMaterialExpressionSquareRoot
IMPL_UOBJ(UMaterialExpressionStaticComponentMaskParameter); // UMaterialExpressionStaticComponentMaskParameter
IMPL_UOBJ(UMaterialExpressionStaticSwitchParameter); // UMaterialExpressionStaticSwitchParameter
IMPL_UOBJ(UMaterialExpressionSubtract); // UMaterialExpressionSubtract
IMPL_UOBJ(UMaterialExpressionTerrainLayerCoords); // UMaterialExpressionTerrainLayerCoords
IMPL_UOBJ(UMaterialExpressionTerrainLayerWeight); // UMaterialExpressionTerrainLayerWeight
IMPL_UOBJ(UMaterialExpressionTextureCoordinate); // UMaterialExpressionTextureCoordinate
IMPL_UOBJ(UMaterialExpressionTextureObject); // UMaterialExpressionTextureObject
IMPL_UOBJ(UMaterialExpressionTextureSample); // UMaterialExpressionTextureSample
IMPL_UOBJ(UMaterialExpressionTextureSampleParameter); // UMaterialExpressionTextureSampleParameter
IMPL_UOBJ(UMaterialExpressionTextureSampleParameter2D); // UMaterialExpressionTextureSampleParameter2D
IMPL_UOBJ(UMaterialExpressionTextureSampleParameterCube); // UMaterialExpressionTextureSampleParameterCube
IMPL_UOBJ(UMaterialExpressionTextureSampleParameterMeshSubUV); // UMaterialExpressionTextureSampleParameterMeshSubUV
IMPL_UOBJ(UMaterialExpressionTextureSampleParameterMeshSubUVBlend); // UMaterialExpressionTextureSampleParameterMeshSubUVBlend
IMPL_UOBJ(UMaterialExpressionTextureSampleParameterMovie); // UMaterialExpressionTextureSampleParameterMovie
IMPL_UOBJ(UMaterialExpressionTextureSampleParameterNormal); // UMaterialExpressionTextureSampleParameterNormal
IMPL_UOBJ(UMaterialExpressionTextureSampleParameterSubUV); // UMaterialExpressionTextureSampleParameterSubUV
IMPL_UOBJ(UMaterialExpressionTime); // UMaterialExpressionTime
IMPL_UOBJ(UMaterialExpressionTransform); // UMaterialExpressionTransform
IMPL_UOBJ(UMaterialExpressionTransformPosition); // UMaterialExpressionTransformPosition
IMPL_UOBJ(UMaterialExpressionTwoSidedSign); // UMaterialExpressionTwoSidedSign
IMPL_UOBJ(UMaterialExpressionVectorParameter); // UMaterialExpressionVectorParameter
IMPL_UOBJ(UMaterialExpressionVertexColor); // UMaterialExpressionVertexColor
IMPL_UOBJ(UMaterialExpressionWindDirectionAndSpeed); // UMaterialExpressionWindDirectionAndSpeed
IMPL_UOBJ(UMaterialExpressionWorldNormal); // UMaterialExpressionWorldNormal
IMPL_UOBJ(UMaterialExpressionWorldPosition); // UMaterialExpressionWorldPosition