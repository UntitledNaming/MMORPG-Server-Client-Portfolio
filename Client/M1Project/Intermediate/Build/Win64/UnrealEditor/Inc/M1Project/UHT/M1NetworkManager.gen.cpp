// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "Network/M1NetworkManager.h"
#include "Engine/GameInstance.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
static_assert(!UE_WITH_CONSTINIT_UOBJECT, "This generated code can only be compiled with !UE_WITH_CONSTINIT_OBJECT");
void EmptyLinkFunctionForGeneratedCodeM1NetworkManager() {}

// ********** Begin Cross Module References ********************************************************
ENGINE_API UClass* Z_Construct_UClass_UGameInstanceSubsystem();
M1PROJECT_API UClass* Z_Construct_UClass_UM1NetworkManager();
M1PROJECT_API UClass* Z_Construct_UClass_UM1NetworkManager_NoRegister();
UPackage* Z_Construct_UPackage__Script_M1Project();
// ********** End Cross Module References **********************************************************

// ********** Begin Class UM1NetworkManager ********************************************************
FClassRegistrationInfo Z_Registration_Info_UClass_UM1NetworkManager;
UClass* UM1NetworkManager::GetPrivateStaticClass()
{
	using TClass = UM1NetworkManager;
	if (!Z_Registration_Info_UClass_UM1NetworkManager.InnerSingleton)
	{
		GetPrivateStaticClassBody(
			TClass::StaticPackage(),
			TEXT("M1NetworkManager"),
			Z_Registration_Info_UClass_UM1NetworkManager.InnerSingleton,
			StaticRegisterNativesUM1NetworkManager,
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
	return Z_Registration_Info_UClass_UM1NetworkManager.InnerSingleton;
}
UClass* Z_Construct_UClass_UM1NetworkManager_NoRegister()
{
	return UM1NetworkManager::GetPrivateStaticClass();
}
struct Z_Construct_UClass_UM1NetworkManager_Statics
{
#if WITH_METADATA
	static constexpr UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[] = {
		{ "IncludePath", "Network/M1NetworkManager.h" },
		{ "ModuleRelativePath", "Network/M1NetworkManager.h" },
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ServerIP_MetaData[] = {
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Config \xed\x82\xa4\xec\x9b\x8c\xeb\x93\x9c\xeb\xa5\xbc \xeb\xb6\x99\xec\x9d\xb4\xeb\xa9\xb4 .ini \xed\x8c\x8c\xec\x9d\xbc\xec\x9d\x98 \xec\x84\xb9\xec\x85\x98\xec\x97\x90\xec\x84\x9c \xea\xb0\x92\xec\x9d\x84 \xec\x9e\x90\xeb\x8f\x99\xec\x9c\xbc\xeb\xa1\x9c \xea\xb0\x80\xec\xa0\xb8\xec\x98\xb5\xeb\x8b\x88\xeb\x8b\xa4.\n" },
#endif
		{ "ModuleRelativePath", "Network/M1NetworkManager.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Config \xed\x82\xa4\xec\x9b\x8c\xeb\x93\x9c\xeb\xa5\xbc \xeb\xb6\x99\xec\x9d\xb4\xeb\xa9\xb4 .ini \xed\x8c\x8c\xec\x9d\xbc\xec\x9d\x98 \xec\x84\xb9\xec\x85\x98\xec\x97\x90\xec\x84\x9c \xea\xb0\x92\xec\x9d\x84 \xec\x9e\x90\xeb\x8f\x99\xec\x9c\xbc\xeb\xa1\x9c \xea\xb0\x80\xec\xa0\xb8\xec\x98\xb5\xeb\x8b\x88\xeb\x8b\xa4." },
#endif
	};
	static constexpr UECodeGen_Private::FMetaDataPairParam NewProp_ServerPort_MetaData[] = {
		{ "ModuleRelativePath", "Network/M1NetworkManager.h" },
	};
#endif // WITH_METADATA

// ********** Begin Class UM1NetworkManager constinit property declarations ************************
	static const UECodeGen_Private::FStrPropertyParams NewProp_ServerIP;
	static const UECodeGen_Private::FIntPropertyParams NewProp_ServerPort;
	static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
// ********** End Class UM1NetworkManager constinit property declarations **************************
	static UObject* (*const DependentSingletons[])();
	static constexpr FCppClassTypeInfoStatic StaticCppClassTypeInfo = {
		TCppClassTypeTraits<UM1NetworkManager>::IsAbstract,
	};
	static const UECodeGen_Private::FClassParams ClassParams;
}; // struct Z_Construct_UClass_UM1NetworkManager_Statics

// ********** Begin Class UM1NetworkManager Property Definitions ***********************************
const UECodeGen_Private::FStrPropertyParams Z_Construct_UClass_UM1NetworkManager_Statics::NewProp_ServerIP = { "ServerIP", nullptr, (EPropertyFlags)0x0020080000004000, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UM1NetworkManager, ServerIP), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ServerIP_MetaData), NewProp_ServerIP_MetaData) };
const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_UM1NetworkManager_Statics::NewProp_ServerPort = { "ServerPort", nullptr, (EPropertyFlags)0x0020080000004000, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(UM1NetworkManager, ServerPort), METADATA_PARAMS(UE_ARRAY_COUNT(NewProp_ServerPort_MetaData), NewProp_ServerPort_MetaData) };
const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_UM1NetworkManager_Statics::PropPointers[] = {
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UM1NetworkManager_Statics::NewProp_ServerIP,
	(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_UM1NetworkManager_Statics::NewProp_ServerPort,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UM1NetworkManager_Statics::PropPointers) < 2048);
// ********** End Class UM1NetworkManager Property Definitions *************************************
UObject* (*const Z_Construct_UClass_UM1NetworkManager_Statics::DependentSingletons[])() = {
	(UObject* (*)())Z_Construct_UClass_UGameInstanceSubsystem,
	(UObject* (*)())Z_Construct_UPackage__Script_M1Project,
};
static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_UM1NetworkManager_Statics::DependentSingletons) < 16);
const UECodeGen_Private::FClassParams Z_Construct_UClass_UM1NetworkManager_Statics::ClassParams = {
	&UM1NetworkManager::StaticClass,
	"Game",
	&StaticCppClassTypeInfo,
	DependentSingletons,
	nullptr,
	Z_Construct_UClass_UM1NetworkManager_Statics::PropPointers,
	nullptr,
	UE_ARRAY_COUNT(DependentSingletons),
	0,
	UE_ARRAY_COUNT(Z_Construct_UClass_UM1NetworkManager_Statics::PropPointers),
	0,
	0x001000A4u,
	METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_UM1NetworkManager_Statics::Class_MetaDataParams), Z_Construct_UClass_UM1NetworkManager_Statics::Class_MetaDataParams)
};
void UM1NetworkManager::StaticRegisterNativesUM1NetworkManager()
{
}
UClass* Z_Construct_UClass_UM1NetworkManager()
{
	if (!Z_Registration_Info_UClass_UM1NetworkManager.OuterSingleton)
	{
		UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_UM1NetworkManager.OuterSingleton, Z_Construct_UClass_UM1NetworkManager_Statics::ClassParams);
	}
	return Z_Registration_Info_UClass_UM1NetworkManager.OuterSingleton;
}
UM1NetworkManager::UM1NetworkManager() {}
DEFINE_VTABLE_PTR_HELPER_CTOR_NS(, UM1NetworkManager);
UM1NetworkManager::~UM1NetworkManager() {}
// ********** End Class UM1NetworkManager **********************************************************

// ********** Begin Registration *******************************************************************
struct Z_CompiledInDeferFile_FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Network_M1NetworkManager_h__Script_M1Project_Statics
{
	static constexpr FClassRegisterCompiledInInfo ClassInfo[] = {
		{ Z_Construct_UClass_UM1NetworkManager, UM1NetworkManager::StaticClass, TEXT("UM1NetworkManager"), &Z_Registration_Info_UClass_UM1NetworkManager, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(UM1NetworkManager), 3485517547U) },
	};
}; // Z_CompiledInDeferFile_FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Network_M1NetworkManager_h__Script_M1Project_Statics 
static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Network_M1NetworkManager_h__Script_M1Project_1769545900{
	TEXT("/Script/M1Project"),
	Z_CompiledInDeferFile_FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Network_M1NetworkManager_h__Script_M1Project_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Network_M1NetworkManager_h__Script_M1Project_Statics::ClassInfo),
	nullptr, 0,
	nullptr, 0,
};
// ********** End Registration *********************************************************************

PRAGMA_ENABLE_DEPRECATION_WARNINGS
