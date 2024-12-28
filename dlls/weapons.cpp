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
/*

===== weapons.cpp ========================================================

  functions governing the selection/use of weapons for players

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

DLL_GLOBAL		short g_sModelIndexShell;
DLL_GLOBAL		short g_sModelIndexShrapnel;


void W_Precache( void )
{
	PRECACHE_MODEL ("models/grenade.mdl");
	PRECACHE_MODEL ("sprites/shard.spr");
	PRECACHE_MODEL ("models/v_crowbar.mdl");
	PRECACHE_MODEL ("models/v_glock.mdl");
	PRECACHE_MODEL ("models/v_mp5.mdl");
	
	g_sModelIndexShell = PRECACHE_MODEL ("models/shell.mdl");
	g_sModelIndexShrapnel = PRECACHE_MODEL ("models/shrapnel.mdl");

	PRECACHE_SOUND ("weapons/debris1.wav");
	PRECACHE_SOUND ("weapons/debris2.wav");
	PRECACHE_SOUND ("weapons/debris3.wav");
	PRECACHE_SOUND ("player/pl_shell1.wav");
	PRECACHE_SOUND ("player/pl_shell2.wav");
	PRECACHE_SOUND ("player/pl_shell3.wav");
	PRECACHE_SOUND ("weapons/hks1.wav");
	PRECACHE_SOUND ("weapons/hks2.wav");
	PRECACHE_SOUND ("weapons/hks3.wav");
	PRECACHE_SOUND ("weapons/pl_gun1.wav");
	PRECACHE_SOUND ("weapons/pl_gun2.wav");
	PRECACHE_SOUND ("weapons/glauncher.wav");
	PRECACHE_SOUND ("weapons/glauncher2.wav");
	PRECACHE_SOUND ("weapons/g_bounce1.wav");
	PRECACHE_SOUND ("weapons/g_bounce2.wav");
	PRECACHE_SOUND ("weapons/g_bounce3.wav");
}


/*
===============================================================================

PLAYER WEAPON USE

===============================================================================
*/

void CBasePlayer::W_ChangeWeapon(int weapon)
{
	if (weapon == 2)
		selectedweapon = WEAPON_CROWBAR;
	else if (weapon == 3)
		selectedweapon = WEAPON_GLOCK;
	else if (weapon == 4)
		selectedweapon = WEAPON_MP5;
}

void CBasePlayer::W_SetCurrentAmmo( void )
{
	if ( pev->weapon == WEAPON_CROWBAR )
	{
		pev->weaponmodel = ALLOC_STRING("models/v_crowbar.mdl");
		pev->currentammo = 99.0;
	}
	else if ( pev->weapon == WEAPON_GLOCK )
	{
		pev->weaponmodel = ALLOC_STRING("models/v_glock.mdl");
		pev->currentammo = 99.0;
	}
	else if ( pev->weapon == WEAPON_MP5 )
	{
		pev->weaponmodel = ALLOC_STRING("models/v_mp5.mdl");
		pev->currentammo = 99.0;
	}
	else
	{
		pev->weaponmodel = 0;
		pev->currentammo = 0;
	}
}

void CBasePlayer::ImpulseCommands()
{
	if ((pev->button & IN_USE) && (pev->pSystemGlobals->time > nextuse))
	{
		Use();
		nextuse = pev->pSystemGlobals->time + 0.5;
	}
	if (pev->impulse >= 1 && pev->impulse <= 8)
		W_ChangeWeapon(pev->impulse);
	if (pev->impulse == 10)
		W_ChangeWeapon(pev->weapon + 1);
	//char str[64];
	//char str2[64];
	//sprintf_s(str, "%d", pev->weapon);
	//sprintf_s(str2, "%s", STRING(pev->weaponmodel));
	//EMIT_SOUND(ENT(pev), CHAN_BODY, str, 1, ATTN_NORM);
	//EMIT_SOUND(ENT(pev), CHAN_BODY, str2, 1, ATTN_NORM);

	//if (pev->impulse == 11)
	//{
	//	if ((pev->weapon - 1) < 0)
	//		ServerflagsCommand(pev, 31);
	//	else
	//		ServerflagsCommand(pev, pev->weapon - 1);
	//}
	if (pev->impulse == 100)
	{
		if (pev->effects == 8)
		{
			//v44[0] = v8 & 0xFFFFFFF7;
			//*v7 = (float)(v8 & 0xFFFFFFF7);
		}
		else
		{
			//v44[0] = v8 | 8;
			//*v7 = (float)(int)(v8 | 8);
		}
	}
	if (pev->impulse == 200)
	{
		if (showlines)
		{
			showlines = 0;
			ALERT(at_console, "Lines Off");
		}
		else
		{
			showlines = 1;
			ALERT(at_console, "Lines On");
		}
	}
}

//   GUN ATTACK
//   other monsters use this but for now only the player has this function


void CBasePlayer::FireBullets(int number, Vector dir, Vector spread, float distance)
{
	UTIL_MakeVectors(pev->v_angle);

	Vector src = pev->origin + pgv->v_forward * 10;
	Vector direction;
	src.z = pev->absmin.z + pev->size.z * 0.7;

	//ClearMultiDamage();
	TraceResult tr;

	UTIL_TraceLine(src, src + dir * 2048, FALSE, &tr);
	//puff_org = trace_endpos - dir * 4;

	while (number > 0)
	{
		direction = dir + UTIL_RandomFloat(-1, 1) * spread.x * pgv->v_right + UTIL_RandomFloat(-1, 1) * spread.y * pgv->v_up;
		UTIL_TraceLine(src, src + direction * distance, ENT(pev), &tr);
		if (tr.flFraction != 1.0)
		{
			//TraceAttack(4, direction);
			WRITE_BYTE(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(MSG_BROADCAST, TE_GUNSHOT);
			WRITE_COORD(MSG_BROADCAST, tr.vecEndPos.x);
			WRITE_COORD(MSG_BROADCAST, tr.vecEndPos.y);
			WRITE_COORD(MSG_BROADCAST, tr.vecEndPos.z);
		}

		number -= 1;
	}

}

void CBasePlayer::Swing_Crowbar()
{
	pev->pSystemGlobals->msg_entity = OFFSET(pev); //i think

	WRITE_BYTE(1, SVC_WEAPONANIM);
	int anim = abs(rand());
	if ((anim) % 2 == 1)
		WRITE_BYTE(1, 1);
	else
		WRITE_BYTE(1, 2);
	pev->pSystemGlobals->msg_entity = 0;
	UTIL_MakeVectors(pev->v_angle);
	Vector aim = UTIL_GetAimVector(ENT(pev), 1000);
	FireBullets(1, aim, Vector(0.025, 0.025, 0.025), 64);
	nextattack = pgv->time + 1.0;
}

void CBasePlayer::Shoot_Pistol()
{
	pev->effects == static_cast<int>(pev->effects) | 2;
	UTIL_MakeVectors(pev->v_angle);
	pev->pSystemGlobals->msg_entity = OFFSET(pev); //i think

	WRITE_BYTE(1, SVC_WEAPONANIM);
	WRITE_BYTE(1, 0);
	pev->pSystemGlobals->msg_entity = 0;

	int sound = abs(rand());
	if ((sound) % 2 == 1)
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/pl_gun1.wav", 1, ATTN_NORM);
	else
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/pl_gun2.wav", 1, ATTN_NORM);
	Vector aimvector = UTIL_GetAimVector(ENT(pev), 2048);
	FireBullets(1, aimvector, Vector(0.025, 0.025, 0.025), 2048);

	nextattack = pgv->time + 0.3;

}
void CBasePlayer::Shoot_Mp5()
{
	pev->effects == static_cast<int>(pev->effects) | 2;

	pev->pSystemGlobals->msg_entity = OFFSET(pev); //i think

	WRITE_BYTE(1, SVC_WEAPONANIM);
	WRITE_BYTE(1, 0);
	pev->pSystemGlobals->msg_entity = 0;

	int sound = UTIL_RandomFloat(0.0, 1.0);
	//if (v8) damn you ida pro and your undefined values
	//	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks1.wav", 1, ATTN_NORM);
	if (sound < 0.66)
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks2.wav", 1, ATTN_NORM);
	else
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks3.wav", 1, ATTN_NORM);

	Vector aim = UTIL_GetAimVector(ENT(pev), 1000);
	FireBullets(1, aim, Vector(0.01, 0.01, 0.01), 2048);
	nextattack = pgv->time + 0.1;
	//variable = pgv->time + 1.0; not sure what this is

}

void CBasePlayer::W_Attack(void)
{
	if (pev->deadflag != DEAD_DEAD)
	{
		pev->weaponframe = 0;
		switch (pev->weapon)
		{
		case 1:
			Swing_Crowbar();
			//PlayerSetAnimation(30);
			break;
		case 2:
			Shoot_Pistol();
			//PlayerSetAnimation(30);
			break;
		case 4:
			Shoot_Mp5();
			//PlayerSetAnimation(30);
			break;
		}
	}
}
void CBasePlayer::W_WeaponFrame(void)
{
	if (pgv->time >= nextattack)
	{
		ImpulseCommands();
		if (pev->button & IN_CANCEL)
		{
			if (pev->weapons)
				pev->weapon = (pev->weapon & 0xFF0000u) >> 16; // not sure if this'll work
		}
		else if (pev->button & IN_ATTACK2)
		{
			if (pev->weapons && !firegrenade)
			{
				//shootgrenade()
				pev->button &= ~IN_ATTACK2;
				firegrenade = 1;
				return;
			}
			if (pev->weapon == 4 && pev->pSystemGlobals->time > nextgrenade)
			{
				firegrenade = 1;
				UTIL_MakeVectors(pev->v_angle);
				pev->pSystemGlobals->msg_entity = OFFSET(pev);
				WRITE_BYTE(1, SVC_WEAPONANIM);
				WRITE_BYTE(1, 2);
				pev->pSystemGlobals->msg_entity = 0;
				if (UTIL_RandomFloat(0.0, 1.0) < 0.5)
					EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/glauncher.wav", 0.75, ATTN_NORM);
				else
					EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/glauncher2.wav", 0.75, ATTN_NORM);
				
				//spawn grenade i think
				nextgrenade = pev->pSystemGlobals->time + 1.0;
			}
		}
		else
			firegrenade = 0;
		if ((pev->button & IN_ATTACK) != 0)
		{
			if (!weaponactivity)
			{
				if (pev->weapon != selectedweapon)
				{
					pev->weapon = selectedweapon; //???
					pev->button &= ~1u;
					W_SetCurrentAmmo();
					weaponactivity = 1;
				}
				else
					W_Attack();
			}
		}
		else
			weaponactivity = 0;
	}
}