// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "M1ProjectPlayerController.h"

#ifdef M1PROJECT_M1ProjectPlayerController_generated_h
#error "M1ProjectPlayerController.generated.h already included, missing '#pragma once' in M1ProjectPlayerController.h"
#endif
#define M1PROJECT_M1ProjectPlayerController_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS

// ********** Begin Class AM1ProjectPlayerController ***********************************************
struct Z_Construct_UClass_AM1ProjectPlayerController_Statics;
M1PROJECT_API UClass* Z_Construct_UClass_AM1ProjectPlayerController_NoRegister();

#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectPlayerController_h_19_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesAM1ProjectPlayerController(); \
	friend struct ::Z_Construct_UClass_AM1ProjectPlayerController_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend M1PROJECT_API UClass* ::Z_Construct_UClass_AM1ProjectPlayerController_NoRegister(); \
public: \
	DECLARE_CLASS2(AM1ProjectPlayerController, APlayerController, COMPILED_IN_FLAGS(CLASS_Abstract | CLASS_Config), CASTCLASS_None, TEXT("/Script/M1Project"), Z_Construct_UClass_AM1ProjectPlayerController_NoRegister) \
	DECLARE_SERIALIZER(AM1ProjectPlayerController)


#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectPlayerController_h_19_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API AM1ProjectPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	/** Deleted move- and copy-constructors, should never be used */ \
	AM1ProjectPlayerController(AM1ProjectPlayerController&&) = delete; \
	AM1ProjectPlayerController(const AM1ProjectPlayerController&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, AM1ProjectPlayerController); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(AM1ProjectPlayerController); \
	DEFINE_ABSTRACT_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(AM1ProjectPlayerController) \
	NO_API virtual ~AM1ProjectPlayerController();


#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectPlayerController_h_16_PROLOG
#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectPlayerController_h_19_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectPlayerController_h_19_INCLASS_NO_PURE_DECLS \
	FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectPlayerController_h_19_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class AM1ProjectPlayerController;

// ********** End Class AM1ProjectPlayerController *************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_M1ProjectPlayerController_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
