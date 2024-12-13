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

===== doors.cpp ========================================================

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "doors.h"

class CBaseDoor : public CBaseEntity
{
public:
	void Spawn(void);
	void SmokeThing(void);
	void Precache(void);
	virtual void KeyValue(KeyValueData* pkvd);
	virtual void Use(CBaseEntity* pActivator);
	//virtual void Blocked(CBaseEntity* pOther);

	// used to selectivly override defaults
	void DoorTouch(entvars_t* pOther);

	// local functions
	void DoorActivate(void);
	void DoorGoUp(void);
	void DoorGoDown(void);
	void DoorHitTop(void);
	void DoorHitBottom(void);

	BYTE	m_bHealthValue;// some doors are medi-kit doors, they give players health

	BYTE	m_bMoveSnd;			// sound a door makes while moving
	BYTE	m_bStopSnd;			// sound a door makes when it stops

	//locksound_t m_ls;			// door lock sounds

	BYTE	m_bLockedSound;		// ordinals from entity selection
	BYTE	m_bLockedSentence;
	BYTE	m_bUnlockedSound;
	BYTE	m_bUnlockedSentence;
	float   m_flLip; // variables & functions from here to below need to be moved to cbasetoggle
	float   m_flWait;
	float   m_flMoveDistance;
	Vector				m_vecPosition1;
	Vector				m_vecPosition2;
	TOGGLE_STATE m_toggle_state;
	EHANDLE m_hActivator;
	void LinearMove(Vector	vecDest, float flSpeed);
	void LinearMoveDone(void);
	Vector				m_vecFinalDest;
	Vector				m_vecFinalAngle;
};

LINK_ENTITY_TO_CLASS(func_door, CBaseDoor);
//
// func_water - same as a door. 
//
LINK_ENTITY_TO_CLASS(func_water, CBaseDoor);

void CBaseDoor::LinearMoveDone(void)
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

void CBaseDoor::LinearMove(Vector	vecDest, float flSpeed)
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
	SetThink(&CBaseDoor::LinearMoveDone);

	// scale the destdelta vector by the time spent traveling to get velocity
	pev->velocity = vecDestDelta / flTravelTime;
}


void CBaseDoor::DoorHitBottom(void)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise2), 1, ATTN_NORM);

	m_toggle_state = TS_AT_BOTTOM;

	if (FBitSet(pev->spawnflags, SF_DOOR_USE_ONLY))
	{
		SetTouch(NULL);
	}
	else 
		SetTouch(&CBaseDoor::DoorTouch);
}

void CBaseDoor::DoorGoDown(void)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise1), 1, ATTN_NORM);
	m_toggle_state = TS_GOING_DOWN;

	SetMoveDone(&CBaseDoor::DoorHitBottom);

	LinearMove(m_vecPosition1, pev->speed);
}

void CBaseDoor::DoorHitTop(void)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise2), 1, ATTN_NORM);
	m_toggle_state = TS_AT_TOP;

	if (FBitSet(pev->spawnflags, SF_DOOR_NO_AUTO_RETURN))
	{
		// Re-instate touch method, movement is complete
		if (!FBitSet(pev->spawnflags, SF_DOOR_USE_ONLY))
			SetTouch(&CBaseDoor::DoorTouch);
	}
	else
	{
		pev->nextthink = pev->ltime + m_flWait;
		SetThink(&CBaseDoor::DoorGoDown);

		if (m_flWait == -1)
		{
			pev->nextthink = -1;
		}
	}
}

void CBaseDoor::DoorGoUp(void)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise1), 1, ATTN_NORM);
	m_toggle_state = TS_GOING_UP;
	SetMoveDone(&CBaseDoor::DoorHitTop);

	LinearMove(m_vecPosition2, pev->speed);
	//SUB_UseTargets(this, this + 1)

}

void CBaseDoor::DoorActivate(void)
{
	if (m_toggle_state != TS_GOING_DOWN && m_toggle_state != TS_GOING_UP)
	{
		if (!FBitSet(pev->spawnflags, SF_DOOR_NO_AUTO_RETURN) && m_toggle_state == TS_AT_BOTTOM)
		{// door should close
			if (m_hActivator != NULL && FClassnameIs(m_hActivator->pev, "player"))
			{
				VARS(m_hActivator)->health += m_bHealthValue;
			}
			DoorGoUp();
		}
		else
		{
			DoorGoDown();
		}
	}
}

void CBaseDoor::SmokeThing( void )
{
	//no idea what this is for lmao
	WRITE_BYTE(0, SVC_TEMPENTITY);
	WRITE_BYTE(0, TE_SMOKE);
	//WRITE_BYTE(0, (__int64)*(float*)(this[1] + 248));    not sure if this is pev->origin or not
	//WRITE_BYTE(0, (__int64)*(float*)(this[1] + 252));
	//WRITE_BYTE(0, (__int64)*(float*)(this[1] + 256));
	WRITE_BYTE(0, pev->origin.x);
	WRITE_BYTE(0, pev->origin.y);
	WRITE_BYTE(0, pev->origin.z);
	WRITE_BYTE(0, 1);
	//this[3] = 0; dunno
}

void CBaseDoor::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "lip"))
	{
		m_flLip = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "skin"))
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "distance"))
	{
		m_flMoveDistance = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "movesnd"))
	{
		m_bMoveSnd = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "stopsnd"))
	{
		m_bStopSnd = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "healthvalue"))
	{
		m_bHealthValue = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
}

void CBaseDoor::Precache( void )
{
	char* pszSound;

	// set the door's "in-motion" sound
	switch (m_bMoveSnd)
	{
	case	0:
		pev->noise1 = ALLOC_STRING("common/null.wav");
		break;
	case	1:
		PRECACHE_SOUND("doors/doormove1.wav");
		pev->noise1 = ALLOC_STRING("doors/doormove1.wav");
		break;
	case	2:
		PRECACHE_SOUND("doors/doormove2.wav");
		pev->noise1 = ALLOC_STRING("doors/doormove2.wav");
		break;
	case	3:
		PRECACHE_SOUND("doors/doormove3.wav");
		pev->noise1 = ALLOC_STRING("doors/doormove3.wav");
		break;
	case	4:
		PRECACHE_SOUND("doors/doormove4.wav");
		pev->noise1 = ALLOC_STRING("doors/doormove4.wav");
		break;
	case	5:
		PRECACHE_SOUND("doors/doormove5.wav");
		pev->noise1 = ALLOC_STRING("doors/doormove5.wav");
		break;
	case	6:
		PRECACHE_SOUND("doors/doormove6.wav");
		pev->noise1 = ALLOC_STRING("doors/doormove6.wav");
		break;
	case	7:
		PRECACHE_SOUND("doors/doormove7.wav");
		pev->noise1 = ALLOC_STRING("doors/doormove7.wav");
		break;
	case	8:
		PRECACHE_SOUND("doors/doormove8.wav");
		pev->noise1 = ALLOC_STRING("doors/doormove8.wav");
		break;
	case	9:
		PRECACHE_SOUND("doors/doormove9.wav");
		pev->noise1 = ALLOC_STRING("doors/doormove9.wav");
		break;
	case	10:
		PRECACHE_SOUND("doors/doormove10.wav");
		pev->noise1 = ALLOC_STRING("doors/doormove10.wav");
		break;
	default:
		pev->noise1 = ALLOC_STRING("common/null.wav");
		break;
	}

	switch (m_bStopSnd)
	{
	case	0:
		pev->noise2 = ALLOC_STRING("common/null.wav");
		break;
	case	1:
		PRECACHE_SOUND("doors/doorstop1.wav");
		pev->noise2 = ALLOC_STRING("doors/doorstop1.wav");
		break;
	case	2:
		PRECACHE_SOUND("doors/doorstop2.wav");
		pev->noise2 = ALLOC_STRING("doors/doorstop2.wav");
		break;
	case	3:
		PRECACHE_SOUND("doors/doorstop3.wav");
		pev->noise2 = ALLOC_STRING("doors/doorstop3.wav");
		break;
	case	4:
		PRECACHE_SOUND("doors/doorstop4.wav");
		pev->noise2 = ALLOC_STRING("doors/doorstop4.wav");
		break;
	case	5:
		PRECACHE_SOUND("doors/doorstop5.wav");
		pev->noise2 = ALLOC_STRING("doors/doorstop5.wav");
		break;
	case	6:
		PRECACHE_SOUND("doors/doorstop6.wav");
		pev->noise2 = ALLOC_STRING("doors/doorstop6.wav");
		break;
	case	7:
		PRECACHE_SOUND("doors/doorstop7.wav");
		pev->noise2 = ALLOC_STRING("doors/doorstop7.wav");
		break;
	case	8:
		PRECACHE_SOUND("doors/doorstop8.wav");
		pev->noise2 = ALLOC_STRING("doors/doorstop8.wav");
		break;
	default:
		pev->noise2 = ALLOC_STRING("common/null.wav");
		break;
	}

}

void CBaseDoor::DoorTouch(entvars_t* pOther)
{
	//CBaseEntity* pEntity = (CBaseEntity*)GET_PRIVATE(ENT(pOther));
	//if (FClassnameIs(pEntity->pev, "player")) why is pev nullptr
	//{
		//if (!pev->targetname)
		//{
			//m_hActivator = pOther->pev->pSystemGlobals->self;  doesnt work idk why i dont understand
			//m_hActivator = pEntity;
			DoorActivate();
		//}
	//}
}

void CBaseDoor::Spawn(void)
{
	Precache();
	SetMovedir(pev);
	if (pev->skin)
	{
		pev->solid = SOLID_NOT;
		SetThink(&CBaseDoor::SmokeThing);
		pev->nextthink = pev->pSystemGlobals->time + 1.0;
	}
	else
	{
		if (FBitSet(pev->spawnflags, SF_DOOR_PASSABLE))
			pev->solid = SOLID_NOT;
		else
			pev->solid = SOLID_BSP;
		pev->movetype = MOVETYPE_PUSH;
	}
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));
	if (pev->speed == 0)
		pev->speed = 100;
	if (pev->dmg == 0)
		pev->dmg = 2;
	m_vecPosition1 = pev->origin;
	m_vecPosition2 = m_vecPosition1 + (pev->movedir * (fabs(pev->movedir.x * (pev->size.x - 2)) + fabs(pev->movedir.y * (pev->size.y - 2)) + fabs(pev->movedir.z * (pev->size.z - 2)) - m_flLip));

	if (FBitSet(pev->spawnflags, SF_DOOR_START_OPEN))
	{
		UTIL_SetOrigin(pev, m_vecPosition2);
		m_vecPosition2 = m_vecPosition1;
		m_vecPosition1 = pev->origin;
	}
	m_toggle_state = TS_AT_BOTTOM;

	if (FBitSet(pev->spawnflags, SF_DOOR_USE_ONLY))
	{
		SetTouch(NULL);
	}
	else // touchable button
		SetTouch(&CBaseDoor::DoorTouch);
}

void CBaseDoor::Use(CBaseEntity* pActivator)
{
	m_hActivator = pActivator;
	if (m_toggle_state == TS_AT_BOTTOM || FBitSet(pev->spawnflags, SF_DOOR_NO_AUTO_RETURN) && m_toggle_state == TS_AT_TOP)
		DoorActivate();
}