// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "Network/M1NetworkManager.h"

#ifdef M1PROJECT_M1NetworkManager_generated_h
#error "M1NetworkManager.generated.h already included, missing '#pragma once' in M1NetworkManager.h"
#endif
#define M1PROJECT_M1NetworkManager_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class UM1NetworkManager ********************************************************
struct Z_Construct_UClass_UM1NetworkManager_Statics;
M1PROJECT_API UClass* Z_Construct_UClass_UM1NetworkManager_NoRegister();

#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Network_M1NetworkManager_h_13_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUM1NetworkManager(); \
	friend struct ::Z_Construct_UClass_UM1NetworkManager_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend M1PROJECT_API UClass* ::Z_Construct_UClass_UM1NetworkManager_NoRegister(); \
public: \
	DECLARE_CLASS2(UM1NetworkManager, UGameInstanceSubsystem, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/M1Project"), Z_Construct_UClass_UM1NetworkManager_NoRegister) \
	DECLARE_SERIALIZER(UM1NetworkManager) \
	static constexpr const TCHAR* StaticConfigName() {return TEXT("Game");} \



#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Network_M1NetworkManager_h_13_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UM1NetworkManager(); \
	/** Deleted move- and copy-constructors, should never be used */ \
	UM1NetworkManager(UM1NetworkManager&&) = delete; \
	UM1NetworkManager(const UM1NetworkManager&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UM1NetworkManager); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UM1NetworkManager); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(UM1NetworkManager) \
	NO_API virtual ~UM1NetworkManager();


#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Network_M1NetworkManager_h_10_PROLOG
#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Network_M1NetworkManager_h_13_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Network_M1NetworkManager_h_13_INCLASS_NO_PURE_DECLS \
	FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Network_M1NetworkManager_h_13_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class UM1NetworkManager;

// ********** End Class UM1NetworkManager **********************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Network_M1NetworkManager_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
