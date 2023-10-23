// Copyright BirdyB LLC


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include <valarray>

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/AuraProjectile.h"
#include "AuraGameplayTags.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/GameplayStatics.h"

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
static TAutoConsoleVariable CVarDebugProjectileAbility(
	TEXT("DebugProjectileAbility"),
	0,
	TEXT("Show debug logs and shapes for projectile ability\n")
	TEXT("  0: Off\n")
	TEXT("  1: On\n"),
	ECVF_Cheat
);
#endif

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bCalculateArc)
{
	if(!GetAvatarActorFromActorInfo()->HasAuthority()) return;

	FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), SocketTag);
	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	//Rotation.Pitch = 0.f;
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SocketLocation);
	SpawnTransform.SetRotation(Rotation.Quaternion());
	
	AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
		ProjectileClass,
		SpawnTransform,
		GetOwningActorFromActorInfo(),
		Cast<APawn>(GetOwningActorFromActorInfo()),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if(bCalculateArc && FMath::IsNearlyEqual(Projectile->ProjectileMovement->ProjectileGravityScale, 1.f))
	{
		FVector LaunchVelocity;
		//UGameplayStatics::SuggestProjectileVelocity_CustomArc(GetAvatarActorFromActorInfo(), LaunchVelocity, SpawnTransform.GetLocation(), ProjectileTargetLocation);
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if(UGameplayStatics::SuggestProjectileVelocity(GetAvatarActorFromActorInfo(), LaunchVelocity, SpawnTransform.GetLocation(), ProjectileTargetLocation, Projectile->ProjectileMovement->InitialSpeed, false, 0.f, 0.f, ESuggestProjVelocityTraceOption::DoNotTrace, FCollisionResponseParams::DefaultResponseParam, TArray<AActor*>(), CVarDebugProjectileAbility.GetValueOnGameThread() > 0))
		{
			LaunchVelocity.Normalize();
			if(CVarDebugProjectileAbility.GetValueOnGameThread() > 0)
			{
				DrawDebugDirectionalArrow(GetWorld(), SocketLocation, SocketLocation + LaunchVelocity*100.f, 5.f, FColor::Blue, false, 5.f);
			}
#endif
#if (UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if(UGameplayStatics::SuggestProjectileVelocity(GetAvatarActorFromActorInfo(), LaunchVelocity, SpawnTransform.GetLocation(), ProjectileTargetLocation, Projectile->ProjectileMovement->InitialSpeed, false, 0.f, 0.f, ESuggestProjVelocityTraceOption::DoNotTrace))
		{
			LaunchVelocity.Normalize();
#endif
			SpawnTransform.SetRotation(LaunchVelocity.Rotation().Quaternion());
		}
	}

	const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
	FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		
	EffectContextHandle.SetAbility(this);
	EffectContextHandle.AddSourceObject(Projectile);

	EffectContextHandle.AddInstigator(SourceASC->GetAvatarActor(), Projectile);
		
	TArray<TWeakObjectPtr<AActor>> Actors;
	Actors.Add(Projectile);
	EffectContextHandle.AddActors(Actors);
		
	FHitResult HitResult;
	HitResult.Location = ProjectileTargetLocation;
	EffectContextHandle.AddHitResult(HitResult);
		
	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);
		
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

	for(TTuple<FGameplayTag, FScalableFloat>& Pair : DamageTypes)
	{
		const float ScaledDamage = Pair.Value.GetValueAtLevel(GetAbilityLevel());
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Pair.Key, ScaledDamage);
	}
		
	Projectile->DamageEffectSpecHandle = SpecHandle;
		
	Projectile->FinishSpawning(SpawnTransform);
}
