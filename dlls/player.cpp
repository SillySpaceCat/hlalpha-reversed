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

===== player.cpp ========================================================

  functions dealing with the player

*/

#include <string>

using namespace std;

#include "extdll.h"
#include "util.h"

#include "cbase.h"
#include "player.h"
#include "weapons.h"

extern DLL_GLOBAL ULONG		g_ulModelIndexPlayer;

extern void DLLEXPORT SetChangeParms( globalvars_t *pgv );
extern edict_t *EntSelectSpawnPoint( globalvars_t *pgv );

float VectorNormalize(Vector v)
{
	float	length, ilength;

	length = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	length = sqrt(length);		// FIXME

	if (length)
	{
		ilength = 1 / length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}

	return length;
}

void CheckPowerUps(entvars_t* pev)
{
	if (pev->health <= 0)
		return;

	pev->modelindex = g_ulModelIndexPlayer;    // don't use eyes
}


// used by kill command and disconnect command
void set_suicide_frame(entvars_t *pev)
{

	if (!FStrEq(STRING(pev->model), "models/player.mdl"))
		return; // allready gibbed

//	pev->frame		= $deatha11;
	pev->solid		= SOLID_NOT;
	pev->movetype	= MOVETYPE_TOSS;
	pev->deadflag	= DEAD_DEAD;
	pev->nextthink	= -1;
}

void CheckWaterJump(entvars_t* pev)
{
	// check for a jump-out-of-water
	UTIL_MakeVectors(pev->angles);
	Vector start = pev->origin;
	start.z += 8;
	pev->pSystemGlobals->v_forward.z = 0;
	VectorNormalize(pev->pSystemGlobals->v_forward);
	Vector end = start + pev->pSystemGlobals->v_forward * 24;
	TraceResult *trace = new TraceResult;
	UTIL_TraceLine(start, end, ENT(pev), trace);
	if (trace->flFraction < 1)
	{
		// solid at waist
		start.z += pev->maxs.z - 8;
		end = start + pev->pSystemGlobals->v_forward * 24;
		pev->movedir = trace->vecPlaneNormal * -50;
		UTIL_TraceLine(start, end, ENT(pev), trace);
		if (trace->flFraction == 1)
		{   // open at eye level
			SetBits(pev->flags, FL_WATERJUMP);
			pev->velocity.z = 225;
			ClearBits(pev->flags, FL_JUMPRELEASED);
			pev->teleport_time = pev->pSystemGlobals->time + 2;  // safety net
			return;
		}
	}
	
}

void WaterMove(entvars_t* pev)
{
	if (pev->movetype == MOVETYPE_NOCLIP)
		return;

	if (pev->health < 0)
		return;

	// waterlevel 0 - not in water
	// waterlevel 1 - feet in water
	// waterlevel 2 - waist in water
	// waterlevel 3 - head in water

	if (pev->waterlevel != 3)
	{
		// not underwater

		// play 'up for air' sound
		if ((pev->air_finished < pev->pSystemGlobals->time) && (pev->pain_finished < pev->pSystemGlobals->time))
		{
			pev->dmg += 2.0;
			if (pev->dmg > 15)
				pev->dmg = 10;
			//takedamage
			pev->pain_finished = pev->pSystemGlobals->time + 1.0;
		}
	}
	else
	{
		if (pev->pSystemGlobals->time > pev->air_finished)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/gasp2.wav", 1, ATTN_NORM);
		else if (pev->pSystemGlobals->time + 9 > pev->air_finished)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/gasp1.wav", 1, ATTN_NORM);
		pev->air_finished = pev->pSystemGlobals->time + 12;
		pev->dmg = 2;
	}

	if (!pev->waterlevel)
	{
		if (FBitSet(pev->flags, FL_INWATER))
			EMIT_SOUND(ENT(pev), CHAN_BODY, "common/outwater.wav", 1, ATTN_NORM);
		    ClearBits(pev->flags, FL_INWATER);
			return;
	}

	if (pev->watertype == CONTENT_LAVA)
	{
		if (pev->pSystemGlobals->time > pev->dmgtime)
		{
			//takedamage
		}
	}
	else if (pev->watertype == CONTENT_SLIME)
	{
		pev->dmgtime = pev->pSystemGlobals->time + 1;
	}

	if (!(FBitSet(pev->flags, FL_INWATER)))
	{
		if (pev->watertype == CONTENT_LAVA)
			EMIT_SOUND(ENT(pev), CHAN_BODY, "player/inlava.wav", 1, ATTN_NORM);
		if (pev->watertype == CONTENT_WATER)
			EMIT_SOUND(ENT(pev), CHAN_BODY, "player/inh2o.wav", 1, ATTN_NORM);
		if (pev->watertype == CONTENT_SLIME)
			EMIT_SOUND(ENT(pev), CHAN_BODY, "player/slimbrn2.wav", 1, ATTN_NORM);

		SetBits(pev->flags, FL_INWATER);
		pev->dmgtime = 0;
	}

	/*
	if (((__int64)v21[95] & 2048) == 0)
	{
		v22 = *(float*)(*v3 + 128) * v21[102] * 0.8;
		v27 = v21[16] - v21[16] * v22;                no idea what this means but there's vectors involved
		v28 = v21[17] - v21[17] * v22;
		v29 = v21[18] - v22 * v21[18];
		v21[16] = v27;
		v21[17] = v28;
		v21[18] = v29;
	}
	*/
}

void CBasePlayer::Killed(void)
{
	pev->modelindex = g_ulModelIndexPlayer;
	pev->weaponmodel = 0;
	pev->deadflag = DEAD_DYING;
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_TOSS;
	ClearBits(pev->flags, FL_ONGROUND);

	if (pev->velocity.z < 10)
		pev->velocity.z += UTIL_RandomFloat(0.0, 300);

	if (pev->health < -40)
	{
		//Pain();
		pev->angles.x = 0;
		pev->angles.z = 0;
		//DeathSound();
		//SetThink(&CBasePlayer::PlayerDeathThink)
	}
}

void CBasePlayer::Spawn( void )
{
	pev->classname		= ALLOC_STRING("player");
	pev->health			= 100;
	pev->takedamage		= DAMAGE_AIM;
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_WALK;
	pev->max_health		= 100;
	pev->flags			= FL_CLIENT;
	pev->air_finished	= pgv->time + 12;
	pev->dmg			= 2;				// initial water damage
	pev->effects		= 0;
	pev->sequence		= 9;
	pev->deadflag		= DEAD_NO;
	SetChangeParms( pgv );

	// TODO: SaveRestore
	{
		// default to normal spawn
		edict_t* pentSpawnSpot = EntSelectSpawnPoint( pgv );
		pev->weapon = 2;
		selectedweapon = 2;
		pev->origin = VARS(pentSpawnSpot)->origin;
		pev->angles = VARS(pentSpawnSpot)->angles;
	}

	pev->origin;
	pev->angles;
	pev->fixangle = TRUE;           // turn this way immediately

	SAVE_SPAWN_PARMS(ENT(pev));

    SET_MODEL(ENT(pev), "models/doctor.mdl");
    g_ulModelIndexPlayer = pev->modelindex;

	UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);

	pev->weapons = 0x17;
	W_SetCurrentAmmo();

    pev->view_ofs = VEC_VIEW;
}


void CBasePlayer::Jump( void )
{
	if (FBitSet(pev->flags, FL_WATERJUMP))
		return;

	if (pev->waterlevel >= 2)
	{
		if (pev->watertype == CONTENT_WATER)
			pev->velocity.z = 100;
		else if (pev->watertype == CONTENT_SLIME)
			pev->velocity.z = 80;
		else
			pev->velocity.z = 50;

		// play swimming sound
				// TODO
	}

	if (!FBitSet(pev->flags, FL_ONGROUND))
		return;

	if (!FBitSet(pev->flags, FL_JUMPRELEASED))
		return;		// don't pogo stick

	ClearBits(pev->flags, FL_JUMPRELEASED);

	ClearBits(pev->flags, FL_ONGROUND);	// don't stairwalk

	//SetAnimation( PLAYER_JUMP );

	pev->button &= ~IN_JUMP;
	// player jumping sound
	if (UTIL_RandomFloat(0.0, 1.0) < 0.5)
		EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_jump1.wav", 1, ATTN_NORM);
	else
		EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_jump2.wav", 1, ATTN_NORM);
	pev->velocity.z += 270;
}


void CBasePlayer::Duck(void)
{
	//shitty duck code but it'll do for now
	if (pev->button & IN_DUCK)
	{
		if (!FBitSet(pev->flags, FL_DUCKING))
		{
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
		}
	}
	else
	{
		UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);
	}
}


void DLLEXPORT PlayerPreThink(globalvars_t* pgv)
{
	entvars_t* pev = VARS(pgv->self);
	CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(ENT(pev));
	if (pPlayer)
		pPlayer->PreThink();
}


void CBasePlayer::PreThink(void)
{
	if (pev->view_ofs == g_vecZero)
		return;	// intermission or finale

	UTIL_MakeVectors(pev->v_angle);		// is this still used?

	//Something1();
	//Something2();
	WaterMove(pev);

	if (pev->waterlevel == 2)
		CheckWaterJump(pev);

	if (pev->deadflag >= DEAD_DYING)
	{
		//PlayerDeadThink();
		return;
	}

	if (pev->button & IN_JUMP)
	{
		// If on a latter, jump off the ladder
		// else Jump
		Jump();

	}
	else
		SetBits(pev->flags, FL_JUMPRELEASED);

	// If trying to duck, already ducked, or in the process of ducking
	if ((pev->button & IN_DUCK) || FBitSet(pev->flags, FL_DUCKING))
		Duck();
	else
		UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX); // broken hack, player's gonna get stuck on the ground

	//if (pev->pSystemGlobals->time < someunknownvariable)   not sure yet
	//	pev->velocity = g_vecZero;
	if (pev->pSystemGlobals->time > nextstep && FBitSet(pev->flags, FL_ONGROUND) && pev->velocity != g_vecZero)
	{
		if (pev->velocity.Length() > 200)
		{
			//unknownfunction( 4 )
			float step = UTIL_RandomFloat(0, 1);
			if (step <= 0.25)
				EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step1.wav", 1, ATTN_NORM);
			else if (step <= 0.5)
				EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step2.wav", 1, ATTN_NORM);
			else if (step <= 0.75)
				EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step3.wav", 1, ATTN_NORM);
			else
				EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_step4.wav", 1, ATTN_NORM);
			nextstep = pev->pSystemGlobals->time + 0.3;
		}
	}

}


void DLLEXPORT PlayerPostThink( globalvars_t *pgv )
{
	entvars_t *pev = VARS(pgv->self);
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(ENT(pev));
	if ( pPlayer )
		pPlayer->PostThink();
}


void CBasePlayer::PostThink( void )
{
	char* v13;
	float v12;
	if (pev->view_ofs == g_vecZero || pev->deadflag)
		return;
	W_WeaponFrame();
	//char str[64];
	//sprintf(str, "%.2f", pev->velocity[2]);
	//EMIT_SOUND(ENT(pev), CHAN_BODY, str, 1, ATTN_NORM);
	
	if ((jumpflag < -300) && (FBitSet(pev->flags, FL_ONGROUND)))
	{
		if (pev->watertype == CONTENT_WATER)
			EMIT_SOUND(ENT(pev), CHAN_BODY, "player/h2ojump.wav", 1, ATTN_NORM);
		else if (jumpflag < -650)
		{
			//T_Damage(self, world, world, 5);
			pev->health -= 5; // TakeDamage(pev, pOwner->pev, 5, DMG_GENERIC) im too lazy to implement that
			if (UTIL_RandomFloat(0.0, 1.0) <= 0.66)
			    EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_fallpain2.wav", 1, ATTN_NORM);
			else
				EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_fallpain3.wav", 1, ATTN_NORM);
		}
		else
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_jumpland2.wav", 1, ATTN_NORM);
	}
	// bunch of punch angle code i think its related to punch angle
	jumpflag = pev->velocity[2];
	CheckPowerUps(pev);

}
