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

===== alien_slave.cpp ========================================================

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CAlienSlave : public CBaseMonster
{
public:
	virtual void Spawn();
	virtual void SetActivity(int activity);
	virtual void Die();
};

LINK_ENTITY_TO_CLASS(monster_alien_slave, CAlienSlave);

void CAlienSlave::Spawn()
{
	PRECACHE_MODEL("models/islave.mdl");
	SET_MODEL(ENT(pev), "models/islave.mdl");
	UTIL_SetSize(pev, VEC_BARNEY_HULL_MIN, VEC_BARNEY_HULL_MAX);
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	pev->effects = 0;
	pev->health = 30;
	pev->yaw_speed = 8;
	pev->sequence = 13;
	//this[32] = 1;
	pev->nextthink += 1;
	WalkMonsterStart();
}

void CAlienSlave::SetActivity(int activity)
{
	int activitynum = NULL;
	switch (activity)
	{
	case 1:
		activitynum = 0;
		break;
	case 2:
		activitynum = 0;
		break;
	case 3:
		activitynum = 1;
		break;
	case 4:
		activitynum = 3;
		break;
	case 8:
		activitynum = 4;
		break;
	default:
		ALERT(at_console, "ISlave's monster state is bogus: %d", activity);
		break;
	}
	if (pev->sequence != activitynum)
	{
		pev->sequence = activitynum;
		switch (activitynum)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			return;
		default:
			ALERT(at_console, "Bogus ISlave anim: %d", activitynum);
			//this[66] = 0.0;
			//this[67] = 0.0;
			break;
		}
	}
}

void CAlienSlave::Die()
{
	SetThink(&CAlienSlave::SUB_Remove);
	pev->nextthink = pev->pSystemGlobals->time;
}