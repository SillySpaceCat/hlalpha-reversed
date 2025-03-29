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

extern class CBaseGrenade;

//
// generic Monster
//
class CBaseMonster : public CBaseAnimating
{
public:
	int m_bloodColor;
	int m_iActivity;
	int m_iIdealActivity;
	float m_sightDistance;
	Vector goal_origin;
	float nextattack;
	float nextidle;
	float multi_damage;
	int multi_ent;
	byte byte1;
	byte byte2;
	byte byte3;
	Vector enemyposition;
	entvars_t* followentity;
	float m_followDistance;
	virtual int Classify() { return 0; };
	virtual void Pain(float flDamage) { return; };
	virtual void Die() { return; };
	virtual void Idle() { return; };
	virtual void Alert() { return; };
	virtual void SetActivity(int activity) { return; };
	virtual int CheckEnemy(entvars_t *a2, float a3) { return 0; };
	float GetDistance(entvars_t* enemy);
	BOOL function1(entvars_t* a2);
	BOOL CheckEnemyOnCrosshair(entvars_t* a2);
	virtual void TakeDamage(entvars_t* pevAttacker, float flDamage);
	void TraceAttack(float damage, int integer1, Vector dir);
	void FireBullets(int number, Vector dir, Vector spread, float distance);
	void ShootGrenade(CBaseGrenade *grenade, entvars_t* owner, Vector origin, Vector dir);
	void ApplyMultiDamage();
	void ClearMultiDamage();
	void AddMultiDamage(float a3);
	void Killed(int pevAttacker);
	void MonsterInit();
	void WalkMonsterStart();
	void CallMonsterThink();
};
#endif // BASEMONSTER_H