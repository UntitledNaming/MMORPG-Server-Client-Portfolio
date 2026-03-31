// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeM1Project_init() {}
static_assert(!UE_WITH_CONSTINIT_UOBJECT, "This generated code can only be compiled with !UE_WITH_CONSTINIT_OBJECT");	M1PROJECT_API UFunction* Z_Construct_UDelegateFunction_M1Project_OnEnemyDied__DelegateSignature();
	static FPackageRegistrationInfo Z_Registration_Info_UPackage__Script_M1Project;
	FORCENOINLINE UPackage* Z_Construct_UPackage__Script_M1Project()
	{
		if (!Z_Registration_Info_UPackage__Script_M1Project.OuterSingleton)
		{
		static UObject* (*const SingletonFuncArray[])() = {
			(UObject* (*)())Z_Construct_UDelegateFunction_M1Project_OnEnemyDied__DelegateSignature,
		};
		static const UECodeGen_Private::FPackageParams PackageParams = {
			"/Script/M1Project",
			SingletonFuncArray,
			UE_ARRAY_COUNT(SingletonFuncArray),
			PKG_CompiledIn | 0x00000000,
			0xFA853967,
			0xD8F53B29,
			METADATA_PARAMS(0, nullptr)
		};
		UECodeGen_Private::ConstructUPackage(Z_Registration_Info_UPackage__Script_M1Project.OuterSingleton, PackageParams);
	}
	return Z_Registration_Info_UPackage__Script_M1Project.OuterSingleton;
}
static FRegisterCompiledInInfo Z_CompiledInDeferPackage_UPackage__Script_M1Project(Z_Construct_UPackage__Script_M1Project, TEXT("/Script/M1Project"), Z_Registration_Info_UPackage__Script_M1Project, CONSTRUCT_RELOAD_VERSION_INFO(FPackageReloadVersionInfo, 0xFA853967, 0xD8F53B29));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
