// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "M1ProjectGameMode.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
static_assert(!UE_WITH_CONSTINIT_UOBJECT, "This generated code can only be compiled with !UE_WITH_CONSTINIT_OBJECT");
void EmptyLinkFunctionForGeneratedCodeM1ProjectGameMode() {}

// ********** Begin Cross Module References ********************************************************
ENGINE_API UClass* Z_Construct_UClass_AGameModeBase();
M1PROJECT_API UClass* Z_Construct_UClass_AM1ProjectGameMode();
M1PROJECT_API UClass* Z_Construct_UClass_AM1ProjectGameMode_NoRegister();
UPackage* Z_Construct_UPackage__Script_M1Project();
// ********** End Cross Module References **********************************************************

// ********** Begin Class AM1ProjectGameMode *******************************************************
FClassRegistrationInfo Z_Registration_Info_UClass_AM1ProjectGameMode;
UClass* AM1ProjectGameMode::GetPrivateStaticClass()
{
	using TClass = AM1ProjectGameMode;
	if (!Z_Registration_Info_UClass_AM1ProjectGameMode.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			TClass::StaticPackage(),
			TEXT("M1ProjectGameMode"),
			Z_Registration_Info_UClass_AM1ProjectGameMode.InnerSingleton,
			StaticRegisterNativesAM1ProjectGameMode,
			sizeof(TClass),
			alignof(TClass),
			TClass::StaticClassFlags,
			TClass::StaticClassCastFlags(),
			TClass::StaticConfigName(),
			(UClass::ClassConstructorType)InternalConstructor<TClass>,
			(UClass::ClassVTableHelperCtorCallerType)InternalVTableHelperCtorCaller<TClass>,
			UOBJECT_CPPCLASS_STATICFUNCTIONS_FORCLASS(TClass),
			&TClass::Super::StaticClass,
			&TClass::WithinClass::StaticClass
		);
	}
	return Z_Registration_Info_UClass_AM1ProjectGameMode.InnerSingleton;
}
UClass* Z_Construct_UClass_AM1ProjectGameMode_NoRegister()
{
	return AM1ProjectGameMode::GetPrivateStaticClass();
}
struct Z_Construct_UClass_AM1ProjectGameMode_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "/**\n *  Simple GameMode for a third person game\n */" },
#endif
		{ "HideCategories", "Info Rendering MovementReplication Replication Actor Input Movement Collision Rendering HLOD WorldPartition DataLayers Transformation" },
		{ "IncludePath", "M1ProjectGameMode.h" },
		{ "ModuleRelativePath", "M1ProjectGameMode.h" },
		{ "ShowCategories", "Input|MouseInput Input|TouchInput" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Simple GameMode for a third person game" },
#endif
	};
#endif // WITH_METADATA

// ********** Begin Class AM1ProjectGameMode constinit property declarations ***********************
// ********** End Class AM1ProjectGameMode constinit property declarations *************************
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AM1ProjectGameMode>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
}; // struct Z_Construct_UClass_AM1ProjectGameMode_Statics
UObject* (*const Z_Construct_UClass_AM1ProjectGameMode_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_AGameModeBase,
	(UObject* (*)())Z_Construct_UPackage__Script_M1Project,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AM1ProjectGameMode_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_AM1ProjectGameMode_Statics::ClassParams = {
	&AM1ProjectGameMode::StaticClass,
	"Game",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	nullptr,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	0,
	0,
	0x008002ADu,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AM1ProjectGameMode_Statics::Class_MetaDataParams), Z_Construct_UClass_AM1ProjectGameMode_Statics::Class_MetaDataParams)
};
void AM1ProjectGameMode::StaticRegisterNativesAM1ProjectGameMode()
{
}
UClass* Z_Construct_UClass_AM1ProjectGameMode()
{
	if (!Z_Registration_Info_UClass_AM1ProjectGameMode.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AM1ProjectGameMode.OuterSingleton, Z_Construct_UClass_AM1ProjectGameMode_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_AM1ProjectGameMode.OuterSingleton;
}
DEFINE_VTABLE_PTR_HELPER_CTOR_NS(, AM1ProjectGameMode);
AM1ProjectGameMode::~AM1ProjectGameMode() {}
// ********** End Class AM1ProjectGameMode *********************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectGameMode_h__Script_M1Project_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_AM1ProjectGameMode, AM1ProjectGameMode::StaticClass, TEXT("AM1ProjectGameMode"), &Z_Registration_Info_UClass_AM1ProjectGameMode, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AM1ProjectGameMode), 2552729534U) },
	};
}; // Z_CompiledInDeferFile_FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectGameMode_h__Script_M1Project_Statics 
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectGameMode_h__Script_M1Project_888136748{
	TEXT("/Script/M1Project"),
	Z_CompiledInDeferFile_FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectGameMode_h__Script_M1Project_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectGameMode_h__Script_M1Project_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0,
};
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
