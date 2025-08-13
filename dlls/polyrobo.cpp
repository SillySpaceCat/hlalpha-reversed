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

    // Default: no head turn
    float headYawDelta = 0.0f;

    if (pev->enemy)
    {
        Vector toEnemy = ENT(pev->enemy)->v.origin - pev->origin;
        float targetYaw = UTIL_VecToYaw(toEnemy);

        // Work out shortest signed angular difference to target
        float delta = targetYaw - pev->angles.y;
        while (delta > 180) delta -= 360;
        while (delta < -180) delta += 360;

        headYawDelta = delta;
    }
    else
    {
        // idle scan when no enemy, e.g. slow sinusoidal sweep
        headYawDelta = 30.0f * sinf(pgv->time);
    }

    // Map −180..180 to 0..255 for controller 0
    BoneController(0, (headYawDelta / 360.0f) * 255.0f);
}
