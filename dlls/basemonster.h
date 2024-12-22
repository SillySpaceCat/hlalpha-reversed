/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#ifndef BASEMONSTER_H
#define BASEMONSTER_H

//
// generic Monster
//
class CBaseMonster : public CBaseToggle
{
public:
	virtual int Classify() { return 0; };
	virtual void Pain(int a2) { return; };
	virtual void Die() { return; };
	virtual void Idle() { return; };
	virtual void SetActivity(int activity) { return; };
	virtual int CheckEnemy(int a2, float a3) { return 0; };
	void MonsterInit();
	void WalkMonsterStart();
	void CallMonsterThink();
};
#endif // BASEMONSTER_H