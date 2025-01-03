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

===== hgrunt.cpp ========================================================

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

enum polyrobo_anims
{
	stand = 0,
	dance,
	walk,
};

class CPolyRobo : public CBaseMonster
{
public:
	virtual void Spawn();
	virtual void Pain(float flDamage);
	virtual void Think();
};

LINK_ENTITY_TO_CLASS(monster_polyrobo, CPolyRobo);

void CPolyRobo::Spawn()
{
	PRECACHE_MODEL("models/polyrobo.mdl");
	SET_MODEL(ENT(pev), "models/polyrobo.mdl");
	UTIL_SetSize(pev, Vector(-48, -48, 0), Vector(48, 48, 212));
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	pev->effects = 0;
	pev->health = 99999;
	pev->yaw_speed = 10;
	pev->sequence = 3;
	pev->takedamage = DAMAGE_AIM;
	SetBits(pev->flags, FL_MONSTER);
	pev->nextthink += UTIL_RandomFloat(0, 0.5);
}

void CPolyRobo::Pain(float flDamage)
{
	if (pev->sequence == stand)
	{
		pev->sequence = dance;
		pev->frame = 0;
		ResetSequenceInfo(0.1);
	}
}

void CPolyRobo::Think()
{
	pev->nextthink = pgv->time + 0.1;
	if (m_fSequenceFinished)
	{
		if (pev->sequence >= 1 && pev->sequence <= 3)
		{
			pev->sequence = stand;
			pev->frame = 0;
		}
		ResetSequenceInfo(0.1);
	}
	DispatchAnimEvents(0.1);
	StudioFrameAdvance(0.1);
	if (pev->enemy)
	{
		edict_t *enemy = ENT(pev->enemy);
		Vector origin = enemy->v.origin - pev->origin;
		pev->angles.y = UTIL_VecToYaw(origin);
	}
}