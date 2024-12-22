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

===== subs.cpp ========================================================

  frequently used global functions

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "doors.h"

// These are the new entry points to entities.


class CNullEntity : public CBaseEntity
{
public:
	void Spawn(void);
};


// Null Entity, remove on startup
void CNullEntity::Spawn(void)
{
	REMOVE_ENTITY(ENT(pev));
}


LINK_ENTITY_TO_CLASS(info_null, CNullEntity);
LINK_ENTITY_TO_CLASS(info_player_deathmatch,CBaseEntity);
LINK_ENTITY_TO_CLASS(info_player_start,CBaseEntity);
LINK_ENTITY_TO_CLASS(info_landmark, CPointEntity);

// Landmark class
void CPointEntity :: Spawn( void )
{
	pev->solid = SOLID_BBOX;
	UTIL_SetSize(pev, g_vecZero, g_vecZero);
}

void CBaseEntity::SUB_Remove( void )
{
	REMOVE_ENTITY(ENT(pev));
}

// Convenient way to explicitly do nothing (passed to functions that require a method)
void CBaseEntity :: SUB_DoNothing( void )
{
}

void CBaseDelay::SUB_UseTargets(entvars_t *pActivator)
{
	if (m_flDelay != 0)
	{
		CBaseDelay* pTemp = GetClassPtr((CBaseDelay*)NULL);
		pTemp->pev->classname = ALLOC_STRING("DelayedUse");
		pTemp->pev->nextthink = pev->pSystemGlobals->time + m_flDelay;
		pTemp->SetThink(&CBaseDelay::DelayThink);
		pTemp->m_iszKillTarget = m_iszKillTarget;
		pTemp->m_flDelay = 0; // prevent "recursion"
		pTemp->pev->target = pev->target;
	}
	else if (m_iszKillTarget)
	{
		edict_t* pentKillTarget = NULL;

		pentKillTarget = FIND_ENTITY_BY_STRING(ENT(pentKillTarget), "targetname", STRING(m_iszKillTarget));
		while (!FNullEnt(pentKillTarget))
		{
			REMOVE_ENTITY(ENT(pentKillTarget));
		}
	}
	else if (!FStringNull(pev->target))
	{
		globalvars_t *v9 = pgv;
		int v10 = 0;
		int v11 = v9->self;
		edict_t* pentTarget = NULL;
		edict_t* nextTarget = NULL;
		for (;;)
		{
			pentTarget = FIND_ENTITY_BY_STRING(nextTarget, "targetname", STRING(pev->target));
			nextTarget = pentTarget;
			if (FNullEnt(pentTarget))
				break;
			CBaseEntity* pTarget = CBaseEntity::Instance(pentTarget);
			if (pTarget)
				pTarget->Use(pActivator);
		}
	}
}

void CBaseDelay::DelayThink(void)
{
	//SUB_UseTargets(pActivator, 0);
	SUB_UseTargets(pev);
	REMOVE_ENTITY(ENT(pev));
}

void CBaseToggle::LinearMoveDone(void)
{
	Vector delta = m_vecFinalDest - pev->origin;
	float error = delta.Length();
	if (error > 0.03125)
	{
		LinearMove(m_vecFinalDest, 100);
		return;
	}

	UTIL_SetOrigin(pev, m_vecFinalDest);
	pev->velocity = g_vecZero;
	pev->nextthink = -1;
	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}

void CBaseToggle::LinearMove(Vector	vecDest, float flSpeed)
{
	m_vecFinalDest = vecDest;

	// Already there?
	if (vecDest == pev->origin)
	{
		LinearMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDest - pev->origin;

	// divide vector length by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to LinearMoveDone when dest is reached
	pev->nextthink = pev->ltime + flTravelTime;
	SetThink(&CBaseToggle::LinearMoveDone);

	// scale the destdelta vector by the time spent traveling to get velocity
	pev->velocity = vecDestDelta / flTravelTime;
}

/*
=============
AngularMove

calculate pev->velocity and pev->nextthink to reach vecDest from
pev->origin traveling at flSpeed
Just like LinearMove, but rotational.
===============
*/
void CBaseToggle::AngularMove(Vector vecDestAngle, float flSpeed)
{

	m_vecFinalAngle = vecDestAngle;

	// Already there?
	if (vecDestAngle == pev->angles)
	{
		AngularMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDestAngle - pev->angles;

	// divide by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to AngularMoveDone when dest is reached
	pev->nextthink = pev->ltime + flTravelTime;
	SetThink(&CBaseToggle::AngularMoveDone);

	// scale the destdelta vector by the time spent traveling to get velocity
	pev->avelocity = vecDestDelta / flTravelTime;
}


/*
============
After rotating, set angle to exact final angle, call "move done" function
============
*/
void CBaseToggle::AngularMoveDone(void)
{
	pev->angles = m_vecFinalAngle;
	pev->avelocity = g_vecZero;
	pev->nextthink = -1;
	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}

void CBaseToggle::AxisDir(entvars_t* pev)
{
	if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_Z))
		pev->movedir = Vector(0, 0, 1);	// around z-axis
	else if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_X))
		pev->movedir = Vector(1, 0, 0);	// around x-axis
	else
		pev->movedir = Vector(0, 1, 0);		// around y-axis
}


float CBaseToggle::AxisDelta(int flags, const Vector& angle1, const Vector& angle2)
{
	if (FBitSet(flags, SF_DOOR_ROTATE_Z))
		return angle1.z - angle2.z;

	if (FBitSet(flags, SF_DOOR_ROTATE_X))
		return angle1.x - angle2.x;

	return angle1.y - angle2.y;
}

void CBaseMonster::MonsterInit()
{
	WalkMonsterStart();
}

void CBaseMonster::WalkMonsterStart()
{
	if (pev->pSystemGlobals->deathmatch)
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}
	pev->origin.x = pev->origin.x + 1;
	DROP_TO_FLOOR(ENT(pev));
	if (!WALK_MOVE(ENT(pev), 0, 0))
	{
		ALERT(at_warning, "Monster %s stuck in wall--level design error", STRING(pev->classname));
		pev->effects = 1;
	}
	pev->takedamage = DAMAGE_AIM;
	pev->ideal_yaw = pev->angles.y;
	SetThink(&CBaseMonster::CallMonsterThink);
	if (pev->target)
	{
		pev->goalentity = OFFSET(FIND_ENTITY_BY_STRING(NULL, "targetname", STRING(pev->target)));
		if (pev->goalentity)
		{
			edict_t *goal = ENT(VARS(pev->goalentity));
			goal->v.origin.x -= pev->origin.x;
			goal->v.origin.y -= pev->origin.y;
			goal->v.origin.z -= pev->origin.z;
			pev->ideal_yaw = UTIL_VecToYaw(goal->v.origin);
			if (!FClassnameIs(ENT(VARS(pev->goalentity)), "path_corner"))
				ALERT(at_aiconsole, "WalkMonsterStart--monster's initial goal '%s' is not a path_corner", STRING(pev->target));
			//*(_DWORD *)(this + 128) = 4;
		}
		else
		{
			ALERT(at_warning, "WalkMonsterStart--%s couldn't find target %s", STRING(pev->classname), STRING(pev->target));
		}
	}
	pev->nextthink = UTIL_RandomFloat(0, 0.5) + pev->nextthink;
}

void CBaseMonster::CallMonsterThink()
{
	//SetActivity(activity);
	//StudioFrameAdvance(0.1);
	if (pev->enemy)
	{
		if (pev->health > 0)
		{
			if (ENT(VARS(pev->enemy))->v.health <= 0)
			{
				pev->enemy = NULL;
				//activity = 1;
				return;
			}
		}
	}
}