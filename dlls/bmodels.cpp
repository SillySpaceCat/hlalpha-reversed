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

===== bmodels.cpp ========================================================

  spawn, think, and use functions for entities that use brush models

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "doors.h"

extern DLL_GLOBAL Vector		g_vecAttackDir;

#define		SF_BRUSH_ACCDCC	16// brush should accelerate and decelerate when toggled
#define		SF_BRUSH_HURT		32// rotating brush that inflicts pain based on rotation speed
#define		SF_ROTATING_NOT_SOLID	64	// some special rotating objects are not solid.

// covering cheesy noise1, noise2, & noise3 fields so they make more sense (for rotating fans)
#define		noiseStart		noise1
#define		noiseStop		noise2
#define		noiseRunning	noise3

#define		SF_PENDULUM_SWING		2	// spawnflag that makes a pendulum a rope swing.

// =================== FUNC_WALL ==============================================

//
// BModelOrigin - calculates origin of a bmodel from absmin/size because all bmodel origins are 0 0 0
//

/*QUAKED func_wall (0 .5 .8) ?
This is just a solid wall if not inhibited
*/
class CFuncWall : public CBaseEntity
{
public:
	void	Spawn(void);
	void	Use(entvars_t* pActivator);

	// Bmodels don't go across transitions
	//virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS(func_wall, CFuncWall);

void CFuncWall::Spawn(void)
{
	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_PUSH;  // so it doesn't get pushed by anything
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));
}


void CFuncWall::Use(entvars_t* pActivator)
{
	pev->frame = 1 - pev->frame;
}


// =================== FUNC_ILLUSIONARY ==============================================


/*QUAKED func_illusionary (0 .5 .8) ?
A simple entity that looks solid but lets you walk through it.
*/
class CFuncIllusionary : public CBaseEntity //CBaseToggle
{
public:
	void Spawn(void);
	//void EXPORT SloshTouch(CBaseEntity* pOther);
	void KeyValue(KeyValueData* pkvd);
	//virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS(func_illusionary, CFuncIllusionary);

void CFuncIllusionary::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "skin"))//skin is used for content type
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
}

void CFuncIllusionary::Spawn(void)
{
	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;// always solid_not 
	SET_MODEL(ENT(pev), STRING(pev->model));
	MAKE_STATIC(ENT(pev));
}


// =================== FUNC_GLASS ==============================================

class CFuncGlass : public CBaseEntity //CBaseToggle i think
{
public:
	void Spawn(void);
	int g_sprShard;
};

void CFuncGlass::Spawn(void)
{
	PRECACHE_SOUND("common/glass.wav");
	g_sprShard = PRECACHE_MODEL("sprites/shard.spr");
	if (!FBitSet(pev->spawnflags, 1))
		pev->takedamage = DAMAGE_YES;
	else
		pev->takedamage = DAMAGE_NO;
	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	SET_MODEL(ENT(pev), STRING(pev->model));
}