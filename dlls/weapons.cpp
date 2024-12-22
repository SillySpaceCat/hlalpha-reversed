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

void CBasePlayer::Gun_Attack(int number, Vector aim, Vector idk, Vector v_angles)
{/*
	UTIL_MakeVectors(pev->v_angle);
	Vector v11 = pgv->v_forward;
	float v69 = v11.x * 10.0;
	float v70 = v11.y * 10.0;
	float v12 = v11.z * 10.0;
	Vector v13 = pev->origin;
	float v71 = v12;
	float v42 = v12 + v13.z;
	float v41 = v13.y + v70;
	float v40 = v13.z + v69;
	Vector v43(v40, v41, v42);
	float v45 = pev->view_ofs.y - 4.0 + pev->origin.z;
	//variable1 = 0;
	//variable2 = 0;
	if (number)
	{
		Vector v15 = pgv->v_right;
		float v16 = UTIL_RandomFloat(-1.0, 1.0) * idk.x;
		v42 = v15[2] * v16;
		v41 = v15[1] * v16;
		v40 = v16 * v15.x;
		Vector v65(v40, v41, v42);
		Vector v17 = pgv->v_up;
		float v18 = UTIL_RandomFloat(-1.0, 1.0) * idk.y;
		v42 = v17[2] * v18;
		v41 = v17[1] * v18;
		v40 = v18 * *v17;
		Vector v66(v40, v41, v42);
		float v59 = aim.x + v65[0];
		float v60 = aim.y + v65[1];
		float v19 = aim.z + v65[2];
		float v61 = v19;
		v42 = v19 + v66[2];
		v41 = v66[1] + v60;
		v40 = v66[0] + v59;
		int v20 = 4;
		Vector v76(v40, v41, v42);
		v42 = v69;

		Vector v56;
		Vector v79;

		v79.x = v76.x * v_angles.x;
		v79.y = v76.y * v_angles.y;
		v79.z = v76.z * v_angles.z;
		Vector v22 = v79;

		v56.x = v22.x + v43.x;
		v56.y = v22.y + v43.y;
		v56.z = v22.z + v43.z;
		Vector vecend(0, 0, 0);
		TraceResult tr;
		UTIL_TraceLine(v43, vecend, ENT(pev), &tr);
		if (!strcmp(STRING(pev->classname), "player"))
			int v20 = 1;

	}
	*/

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
	//gunattack(1, aim, Vector(0.025, 0.025, 0.025), pev->v_angle[0], pev->v_angle[1], 64);
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
	//Gun_Attack(1, aimvector, Vector(0.25, 0.25, 0.25), pev->v_angle); unfinished

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
			//player_shot1();
			break;
		case 2:
			Shoot_Pistol();
			//player_shot1();
			break;
		case 4:
			Shoot_Mp5();
			//player_shot1();
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