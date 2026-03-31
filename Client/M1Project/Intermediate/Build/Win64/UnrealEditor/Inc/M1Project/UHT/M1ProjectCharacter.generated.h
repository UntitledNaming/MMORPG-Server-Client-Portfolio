// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "M1ProjectCharacter.h"

#ifdef M1PROJECT_M1ProjectCharacter_generated_h
#error "M1ProjectCharacter.generated.h already included, missing '#pragma once' in M1ProjectCharacter.h"
#endif
#define M1PROJECT_M1ProjectCharacter_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class AM1ProjectCharacter ******************************************************
#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectCharacter_h_24_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execDoJumpEnd); \
	DECLARE_FUNCTION(execDoJumpStart); \
	DECLARE_FUNCTION(execDoLook); \
	DECLARE_FUNCTION(execDoMove);


struct Z_Construct_UClass_AM1ProjectCharacter_Statics;
M1PROJECT_API UClass* Z_Construct_UClass_AM1ProjectCharacter_NoRegister();

#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectCharacter_h_24_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAM1ProjectCharacter(); \
	friend struct ::Z_Construct_UClass_AM1ProjectCharacter_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend M1PROJECT_API UClass* ::Z_Construct_UClass_AM1ProjectCharacter_NoRegister(); \
public: \
	DECLARE_CLASS2(AM1ProjectCharacter, ACharacter, COMPILED_IN_FLAGS(CLASS_Abstract | CLASS_Config), CASTCLASS_None, TEXT("/Script/M1Project"), Z_Construct_UClass_AM1ProjectCharacter_NoRegister) \
	DECLARE_SERIALIZER(AM1ProjectCharacter)


#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectCharacter_h_24_ENHANCED_CONSTRUCTORS \
	/** Deleted move- and copy-constructors, should never be used */ \
	AM1ProjectCharacter(AM1ProjectCharacter&&) = delete; \
	AM1ProjectCharacter(const AM1ProjectCharacter&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AM1ProjectCharacter); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AM1ProjectCharacter); \
	DEFINE_ABSTRACT_DEFAULT_CONSTRUCTOR_CALL(AM1ProjectCharacter) \
	NO_API virtual ~AM1ProjectCharacter();


#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectCharacter_h_21_PROLOG
#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectCharacter_h_24_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectCharacter_h_24_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectCharacter_h_24_INCLASS_NO_PURE_DECLS \
	FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectCharacter_h_24_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class AM1ProjectCharacter;

// ********** End Class AM1ProjectCharacter ********************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectCharacter_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
