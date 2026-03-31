// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "M1ProjectGameMode.h"

#ifdef M1PROJECT_M1ProjectGameMode_generated_h
#error "M1ProjectGameMode.generated.h already included, missing '#pragma once' in M1ProjectGameMode.h"
#endif
#define M1PROJECT_M1ProjectGameMode_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class AM1ProjectGameMode *******************************************************
struct Z_Construct_UClass_AM1ProjectGameMode_Statics;
M1PROJECT_API UClass* Z_Construct_UClass_AM1ProjectGameMode_NoRegister();

#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectGameMode_h_15_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAM1ProjectGameMode(); \
	friend struct ::Z_Construct_UClass_AM1ProjectGameMode_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend M1PROJECT_API UClass* ::Z_Construct_UClass_AM1ProjectGameMode_NoRegister(); \
public: \
	DECLARE_CLASS2(AM1ProjectGameMode, AGameModeBase, COMPILED_IN_FLAGS(CLASS_Abstract | CLASS_Transient | CLASS_Config), CASTCLASS_None, TEXT("/Script/M1Project"), Z_Construct_UClass_AM1ProjectGameMode_NoRegister) \
	DECLARE_SERIALIZER(AM1ProjectGameMode)


#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectGameMode_h_15_ENHANCED_CONSTRUCTORS \
	/** Deleted move- and copy-constructors, should never be used */ \
	AM1ProjectGameMode(AM1ProjectGameMode&&) = delete; \
	AM1ProjectGameMode(const AM1ProjectGameMode&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AM1ProjectGameMode); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AM1ProjectGameMode); \
	DEFINE_ABSTRACT_DEFAULT_CONSTRUCTOR_CALL(AM1ProjectGameMode) \
	NO_API virtual ~AM1ProjectGameMode();


#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectGameMode_h_12_PROLOG
#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectGameMode_h_15_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectGameMode_h_15_INCLASS_NO_PURE_DECLS \
	FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectGameMode_h_15_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class AM1ProjectGameMode;

// ********** End Class AM1ProjectGameMode *********************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectGameMode_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
