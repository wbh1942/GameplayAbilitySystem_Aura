// Copyright BirdyB LLC


#include "Actor/AuraEffectActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

AAuraEffectActor::AAuraEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SetRootComponent(CreateDefaultSubobject<USceneComponent>("SceneRoot"));
}

void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AAuraEffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass, bool InfiniteAndStore)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if(TargetASC == nullptr) return;

	check(GameplayEffectClass);
	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	if(InfiniteAndStore)
	{
		if(!ActiveEffectHandles.Contains(TargetASC))
		{
			ActiveEffectHandles.Add(TargetASC);
		}
		ActiveEffectHandles[TargetASC].Emplace(ActiveEffectHandle);
	}
}

void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
	if (!TargetMatchesPolicy(TargetActor)) return;

	if(!InstantGameplayEffects.IsEmpty())
	for(const auto [GameplayEffectClass, EffectApplicationPolicy] : InstantGameplayEffects)
	{
		if(EffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
		{
			ApplyEffectToTarget(TargetActor, GameplayEffectClass);
		}
	}

	if(!DurationGameplayEffects.IsEmpty())
	for(const auto [GameplayEffectClass, EffectApplicationPolicy] : DurationGameplayEffects)
	{
		if(EffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
		{
			ApplyEffectToTarget(TargetActor, GameplayEffectClass);
		}
	}

	if(!InfiniteGameplayEffects.IsEmpty())
	for(const auto [GameplayEffectClass, EffectApplicationPolicy, EffectRemovalPolicy] : InfiniteGameplayEffects)
	{
		if(EffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
		{
			ApplyEffectToTarget(TargetActor, GameplayEffectClass, EffectRemovalPolicy != EEffectRemovalPolicy::DoNotRemove);
		}
	}

	if(ActorDestroyPolicy == EActorDestroyPolicy::DestroyAfterOverlap) Destroy();
}

void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (!TargetMatchesPolicy(TargetActor)) return;
	
	if(!InstantGameplayEffects.IsEmpty())
	for(const auto [GameplayEffectClass, EffectApplicationPolicy] : InstantGameplayEffects)
	{
		if(EffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
		{
			ApplyEffectToTarget(TargetActor, GameplayEffectClass);
		}
	}

	if(!DurationGameplayEffects.IsEmpty())
	for(const auto [GameplayEffectClass, EffectApplicationPolicy] : DurationGameplayEffects)
	{
		if(EffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
		{
			ApplyEffectToTarget(TargetActor, GameplayEffectClass);
		}
	}

	if(!InfiniteGameplayEffects.IsEmpty())
	for(const auto [GameplayEffectClass, EffectApplicationPolicy, EffectRemovalPolicy] : InfiniteGameplayEffects)
	{
		if(EffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
		{
			ApplyEffectToTarget(TargetActor, GameplayEffectClass, EffectRemovalPolicy != EEffectRemovalPolicy::DoNotRemove);
		}
	}

	if(UAbilitySystemComponent* TargetASC
		= UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor))
	{
		if(TArray<FActiveGameplayEffectHandle>* Handles = ActiveEffectHandles.Find(TargetASC))
		{
			for(const FActiveGameplayEffectHandle Handle : *Handles)
			{
				TargetASC->RemoveActiveGameplayEffect(Handle, 1);
			}
			Handles->Empty();
			ActiveEffectHandles.Remove(TargetASC); // TODO: Delete Mapping or not
		}
	}

	if(ActorDestroyPolicy == EActorDestroyPolicy::DestroyAfterEndOverlap) Destroy();
}

bool AAuraEffectActor::TargetMatchesPolicy(const AActor* TargetActor) const
{
	// This would technically be wrong if an actor had both the "Player" and "Enemy" tags
	// It will assumed that this will not happen
	if(TargetTypePolicy == ETargetTypePolicy::EnemiesOnly && !TargetActor->ActorHasTag(FName("Enemy"))) return false;
	if(TargetTypePolicy == ETargetTypePolicy::PlayersOnly && !TargetActor->ActorHasTag(FName("Player"))) return false;

	return true;
}
