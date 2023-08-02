// Copyright BirdyB LLC


#include "Character/AuraEnemy.h"

#include "Aura/Aura.h"

AAuraEnemy::AAuraEnemy()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AAuraEnemy::HighlightActor()
{
	if(GetMesh()) GetMesh()->SetRenderCustomDepth(true);
	if(GetMesh()) GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	if(Weapon) Weapon->SetRenderCustomDepth(true);
	if(Weapon) Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
}

void AAuraEnemy::UnHighlightActor()
{
	if(GetMesh()) GetMesh()->SetRenderCustomDepth(false);
	if(Weapon) Weapon->SetRenderCustomDepth(false);
}
