// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "Variant_Combat/Interfaces/CombatDamageable.h"

#ifdef M1PROJECT_CombatDamageable_generated_h
#error "CombatDamageable.generated.h already included, missing '#pragma once' in CombatDamageable.h"
#endif
#define M1PROJECT_CombatDamageable_generated_h

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
class AActor;

// ********** Begin Interface UCombatDamageable ****************************************************
#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Variant_Combat_Interfaces_CombatDamageable_h_17_RPC_WRAPPERS_NO_PURE_DECLS \
	DECLARE_FUNCTION(execNotifyDanger); \
	DECLARE_FUNCTION(execApplyHealing); \
	DECLARE_FUNCTION(execHandleDeath); \
	DECLARE_FUNCTION(execApplyDamage);


struct Z_Construct_UClass_UCombatDamageable_Statics;
M1PROJECT_API UClass* Z_Construct_UClass_UCombatDamageable_NoRegister();

#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Variant_Combat_Interfaces_CombatDamageable_h_17_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	M1PROJECT_API UCombatDamageable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	/** Deleted move- and copy-constructors, should never be used */ \
	UCombatDamageable(UCombatDamageable&&) = delete; \
	UCombatDamageable(const UCombatDamageable&) = delete; \
	DECLARE_VTABLE_PTR_HELPER_CTOR(M1PROJECT_API, UCombatDamageable); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UCombatDamageable); \
	DEFINE_ABSTRACT_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UCombatDamageable) \
	virtual ~UCombatDamageable() = default;


#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Variant_Combat_Interfaces_CombatDamageable_h_17_GENERATED_UINTERFACE_BODY() \
private: \
	static void StaticRegisterNativesUCombatDamageable(); \
	friend struct ::Z_Construct_UClass_UCombatDamageable_Statics; \
	static UClass* GetPrivateStaticClass(); \
	friend M1PROJECT_API UClass* ::Z_Construct_UClass_UCombatDamageable_NoRegister(); \
public: \
	DECLARE_CLASS2(UCombatDamageable, UInterface, COMPILED_IN_FLAGS(CLASS_Abstract | CLASS_Interface), CASTCLASS_None, TEXT("/Script/M1Project"), Z_Construct_UClass_UCombatDamageable_NoRegister) \
	DECLARE_SERIALIZER(UCombatDamageable)


#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Variant_Combat_Interfaces_CombatDamageable_h_17_GENERATED_BODY \
	PRAGMA_DISABLE_DEPRECATION_WARNINGS \
	FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Variant_Combat_Interfaces_CombatDamageable_h_17_GENERATED_UINTERFACE_BODY() \
	FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Variant_Combat_Interfaces_CombatDamageable_h_17_ENHANCED_CONSTRUCTORS \
private: \
	PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Variant_Combat_Interfaces_CombatDamageable_h_17_INCLASS_IINTERFACE_NO_PURE_DECLS \
protected: \
	virtual ~ICombatDamageable() {} \
public: \
	typedef UCombatDamageable UClassType; \
	typedef ICombatDamageable ThisClass; \
	virtual UObject* _getUObject() const { return nullptr; }


#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Variant_Combat_Interfaces_CombatDamageable_h_14_PROLOG
#define FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Variant_Combat_Interfaces_CombatDamageable_h_22_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Variant_Combat_Interfaces_CombatDamageable_h_17_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Variant_Combat_Interfaces_CombatDamageable_h_17_INCLASS_IINTERFACE_NO_PURE_DECLS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


class UCombatDamageable;

// ********** End Interface UCombatDamageable ******************************************************

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Users_sdj53_Documents_GitHub_MMO_Server_Portfolio_Client_M1Project_Source_M1Project_Variant_Combat_Interfaces_CombatDamageable_h

PRAGMA_ENABLE_DEPRECATION_WARNINGS
