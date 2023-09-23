// Copyright BirdyB LLC

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "AuraEffectActor.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EActorDestroyPolicy
{
	DestroyAfterOverlap,
	DestroyAfterEndOverlap,
	DoNotDestroy
};

UENUM(BlueprintType)
enum class EEffectApplicationPolicy
{
	ApplyOnOverlap,
	ApplyOnEndOverlap,
	DoNotApply
};

UENUM(BlueprintType)
enum class EEffectRemovalPolicy
{
	RemoveOnEndOverlap,
	DoNotRemove
};

UENUM(BlueprintType)
enum class ETargetTypePolicy
{
	AllTargets,
	PlayersOnly,
	EnemiesOnly
};

USTRUCT(BlueprintType)
struct FInstantGameplayEffectWrapper
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectApplicationPolicy EffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;
};

USTRUCT(BlueprintType)
struct FDurationGameplayEffectWrapper
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectApplicationPolicy EffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;
};

USTRUCT(BlueprintType)
struct FInfiniteGameplayEffectWrapper
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> GameplayEffectClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectApplicationPolicy EffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EEffectRemovalPolicy EffectRemovalPolicy = EEffectRemovalPolicy::RemoveOnEndOverlap;
};

UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAuraEffectActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass, bool InfiniteAndStore = false);

	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor* TargetActor);
	bool TargetMatchesPolicy(const AActor* TargetActor) const;

	UFUNCTION(BlueprintCallable)
	void OnEndOverlap(AActor* TargetActor);
	
	//~ BEGIN Instant Gameplay Effect Attributes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	TArray<FInstantGameplayEffectWrapper> InstantGameplayEffects;
	//~ END Instant Gameplay Effect Attributes

	//~ BEGIN Duration Gameplay Effect Attributes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	TArray<FDurationGameplayEffectWrapper> DurationGameplayEffects;
	//~ END Duration Gameplay Effect Attributes

	//~ BEGIN Infinite Gameplay Effect Attributes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	TArray<FInfiniteGameplayEffectWrapper> InfiniteGameplayEffects;
	//~ END Infinite Gameplay Effect Attributes

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	EActorDestroyPolicy ActorDestroyPolicy = EActorDestroyPolicy::DoNotDestroy;

	TMap<UAbilitySystemComponent*, TArray<FActiveGameplayEffectHandle>> ActiveEffectHandles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	float ActorLevel = 1.f;


	/*
	 * If eventually more tags than "Player" and "Enemy" ->
	 * There could be a bool for bALlTargets ->
	 * if bAllTargets is false -> allow user to add
	 * acceptable FName tags to an array
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Applied Effects")
	ETargetTypePolicy TargetTypePolicy = ETargetTypePolicy::AllTargets;
};
