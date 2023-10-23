// Copyright BirdyB LLC


#include "AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelByXP(int32 XP) const
{
	int32 L = 0, R = LevelUpInfos.Num();
	int32 Cur = (L + R) / 2;
	
	while(L < R)
	{
		if(XP < LevelUpInfos[Cur].LevelUpRequirement)
		{
			R = Cur;
		}
		else
		{
			L = Cur;
		}
		Cur = (L + R) / 2 + L;
	}

	return Cur;
}
