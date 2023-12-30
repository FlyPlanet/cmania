#pragma once
#include "Game.h"
#include "Ruleset.h"
class IRulesetManager
{
public:
	virtual Ruleset& GetRuleset(std::string name) const = 0;
	/// <summary>
	/// ע�ᣬ�⽫�� Ruleset �Ŀ���Ȩת���� RulesetManager
	/// </summary>
	/// <param name="rul"></param>
	virtual void Register(Ruleset* rul) = 0;
};

GameComponent* MakeRulesetManager();