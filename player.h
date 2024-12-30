/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef PLAYER_H
#define PLAYER_H

class CBasePlayer : public CBaseMonster
{
public:
	float jumpflag;
	int firegrenade;
	float nextattack;
	int weaponactivity;
	int showlines;
	int selectedweapon;
	float nextgrenade;
	float nextstep;
	float nextuse;
	int g_onladder;
	float multi_damage;
	int multi_ent;
	virtual void Spawn( void );
	virtual void Use(void);
	virtual void Jump( void );
	virtual void Duck( void );
	virtual void PreThink( void );
	virtual void PostThink( void );
	virtual void Killed(void);
	void ImpulseCommands();
	void FireBullets(int number, Vector dir, Vector spread, float distance);
	void TraceAttack(float damage, int integer1, Vector dir);
	void ClearMultiDamage();
	void ApplyMultiDamage();
	void AddMultiDamage(float a3);
	void Swing_Crowbar();
	void Shoot_Pistol();
	void Shoot_Mp5();
	void W_ChangeWeapon( int weapon );
	void W_SetCurrentAmmo( void );
	void W_Attack(void);
	void W_WeaponFrame(void);
};

#endif // PLAYER_H