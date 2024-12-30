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

===== m44.cpp ========================================================

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CM44 : public CBaseEntity
{
public:
	void Spawn();
};

LINK_ENTITY_TO_CLASS(monster_m44, CM44);

void CM44::Spawn()
{
	PRECACHE_MODEL("models/m44.mdl");
	SET_MODEL(ENT(pev), "models/m44.mdl");
	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_STEP; // were they planning on having moving vehicles at some point?
	pev->takedamage = 0;
	pev->effects = 0;
	pev->health = 80;
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 32));
	pev->sequence = 0;
	pev->frame = 0;
	pev->nextthink += UTIL_RandomFloat(0, 0.5);
}