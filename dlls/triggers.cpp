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

===== triggers.cpp ========================================================

  spawn and use functions for editor-placed triggers

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"

#define	SF_TRIGGER_PUSH_START_OFF	2//spawnflag that makes trigger_push spawn turned OFF
#define SF_TRIGGER_HURT_TARGETONCE	1// Only fire hurt target once
#define	SF_TRIGGER_HURT_START_OFF	2//spawnflag that makes trigger_push spawn turned OFF
#define	SF_TRIGGER_HURT_NO_CLIENTS	8//spawnflag that makes trigger_push spawn turned OFF
#define SF_TRIGGER_HURT_CLIENTONLYFIRE	16// trigger hurt will only fire its target if it is hurting a client
#define SF_TRIGGER_HURT_CLIENTONLYTOUCH 32// only clients may touch this trigger.

extern DLL_GLOBAL BOOL		g_fGameOver;

extern void SetMovedir(entvars_t* pev);

class CBaseTrigger : public CBaseToggle
{
public:
	void KeyValue(KeyValueData* pkvd);
	void MultiTouch(entvars_t* pOther);
	void HurtTouch(entvars_t* pOther);
	void CDAudioTouch(CBaseEntity* pOther);
	void ActivateMultiTrigger(entvars_t* pActivator);
	void MultiWaitOver(void);
	void CounterUse(entvars_t *pActivator);
	void ToggleUse(entvars_t* pActivator);
	void InitTrigger(void);
};

LINK_ENTITY_TO_CLASS(trigger, CBaseTrigger);

/*
================
InitTrigger
================
*/
void CBaseTrigger::InitTrigger()
{
	// trigger angles are used for one-way touches.  An angle of 0 is assumed
	// to mean no restrictions, so use a yaw of 360 instead.
	if (pev->angles != g_vecZero)
		SetMovedir(pev);
	pev->solid = SOLID_TRIGGER;
	SET_MODEL(ENT(pev), STRING(pev->model));    // set size and link into world
	pev->movetype = MOVETYPE_NONE;
	pev->modelindex = 0;
	pev->model = 0;
}


//
// Cache user-entity-field values until spawn is called.
//

void CBaseTrigger::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	if (FStrEq(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "count"))
	{
		m_cTriggersLeft = (int)atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "delay"))
	{
		m_flDelay = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
}

class CTriggerHurt : public CBaseTrigger
{
public:
	void Spawn(void);
};

LINK_ENTITY_TO_CLASS(trigger_hurt, CTriggerHurt);

//
// trigger_monsterjump
//
class CTriggerMonsterJump : public CBaseTrigger
{
public:
	void Spawn(void);
	void Touch(entvars_t* pOther);
	void Think(entvars_t * pOther);
};

LINK_ENTITY_TO_CLASS(trigger_monsterjump, CTriggerMonsterJump);

void CTriggerMonsterJump::Spawn(void)
{
	SetMovedir(pev);

	InitTrigger();

	SetUse(&CTriggerMonsterJump::Think);
	SetTouch(&CTriggerMonsterJump::Touch);
	pev->speed = 200;
	m_flHeight = 150;

	if (!FStringNull(pev->targetname))
	{// if targetted, spawn turned off
		pev->solid = SOLID_NOT;
	}
}


void CTriggerMonsterJump::Think(entvars_t* pOther)
{
	if (pev->solid != SOLID_NOT)
		pev->solid == SOLID_NOT;
	else
		pev->solid == SOLID_TRIGGER;
}

void CTriggerMonsterJump::Touch(entvars_t*	pOther)
{
	entvars_t* thing = VARS(pev->pSystemGlobals->other);

	if (FBitSet(thing->flags, FL_MONSTER))
	{
		pev->origin.z += 1.0;
		if (FBitSet(thing->flags, FL_ONGROUND))
			ClearBits(thing->flags, FL_ONGROUND);
		thing->velocity = thing->movedir * thing->speed;
		thing->velocity.z += m_flHeight;
		pev->solid = SOLID_NOT;
	}
		
}

//=====================================
//
// trigger_cdaudio - starts/stops cd audio tracks
//
class CTriggerCDAudio : public CBaseTrigger
{
public:
	void Spawn(void);
	void PlayCDTrack(entvars_t* pOther);
};

LINK_ENTITY_TO_CLASS(trigger_cdaudio, CTriggerCDAudio);

void CTriggerCDAudio::Spawn(void)
{
	InitTrigger();
	SetTouch(&CTriggerCDAudio::PlayCDTrack);
}

void CTriggerCDAudio::PlayCDTrack(entvars_t* pOther)
{
	entvars_t* thing = VARS(pev->pSystemGlobals->other);
	if (FClassnameIs(thing, "player"))
	{
		switch ((int)pev->health)
		{
		case -1:
			CLIENT_COMMAND(ENT(thing), "cd stop");
			break;
		case 1:
			CLIENT_COMMAND(ENT(thing), "cd 1");
			break;
		case 2:
			CLIENT_COMMAND(ENT(thing), "cd 2");
			break;
		case 3:
			CLIENT_COMMAND(ENT(thing), "cd 3");
			break;
		case 4:
			CLIENT_COMMAND(ENT(thing), "cd 4");
			break;
		case 5:
			CLIENT_COMMAND(ENT(thing), "cd 5");
			break;
		case 6:
			CLIENT_COMMAND(ENT(thing), "cd 6");
			break;
		case 7:
			CLIENT_COMMAND(ENT(thing), "cd 7");
			break;
		case 8:
			CLIENT_COMMAND(ENT(thing), "cd 8");
			break;
		case 9:
			CLIENT_COMMAND(ENT(thing), "cd 9");
			break;
		case 10:
			CLIENT_COMMAND(ENT(thing), "cd 10");
			break;
		case 11:
			CLIENT_COMMAND(ENT(thing), "cd 11");
			break;
		case 12:
			CLIENT_COMMAND(ENT(thing), "cd 12");
			break;
		default:
			ALERT(at_console, "Unknown Track");
			break;
		}
		SetTouch(NULL);
		SetThink(&CBaseEntity::SUB_Remove);
		pev->nextthink = pev->pSystemGlobals->time + 0.1;
	}
}

//=====================================

//
// trigger_hurt - hurts anything that touches it. if the trigger has a targetname, firing it will toggle state
//
//int gfToggleState = 0; // used to determine when all radiation trigger hurts have called 'RadiationThink'

void CTriggerHurt::Spawn(void)
{
	InitTrigger();
	SetTouch(&CTriggerHurt::HurtTouch);

	if (!FStringNull(pev->targetname))
	{
		SetUse(&CTriggerHurt::ToggleUse);
	}
	else
	{
		SetUse(NULL);
	}
}

// When touched, a hurt trigger does DMG points of damage each half-second
void CBaseTrigger::HurtTouch(entvars_t* pOther)
{
	entvars_t* thing = VARS(pev->pSystemGlobals->other);

	if (!thing->takedamage)
		return;
	if (pev->pSystemGlobals->time < m_flDelay)
		return;

	CBaseEntity* otherthing = GetClassPtr((CBaseEntity*)thing);
	otherthing->TakeDamage(pev, pev, pev->dmg);

	m_flDelay += 0.5;
}

// When touched, a hurt trigger does DMG points of damage each half-second
void CBaseTrigger::ToggleUse(entvars_t* pOther)
{
	if (!pev->solid)
		pev->solid = SOLID_NOT;
	else
		pev->solid = SOLID_TRIGGER;
}

class CTriggerMultiple : public CBaseTrigger
{
public:
	void Spawn(void);
};

LINK_ENTITY_TO_CLASS(trigger_multiple, CTriggerMultiple);


void CTriggerMultiple::Spawn(void)
{
	PRECACHE_SOUND("common/null.wav");
	pev->noise = ALLOC_STRING("common/null.wav");
	if (m_flWait == 0)
		m_flWait = 0.2;
	InitTrigger();
	if (!FBitSet(pev->spawnflags, 1))
		SetTouch(&CTriggerMultiple::MultiTouch);
}


class CTriggerOnce : public CTriggerMultiple
{
public:
	void Spawn(void);
};

LINK_ENTITY_TO_CLASS(trigger_once, CTriggerOnce);
void CTriggerOnce::Spawn(void)
{
	m_flWait = -1;

	CTriggerMultiple::Spawn();
}


void CBaseTrigger::MultiTouch(entvars_t* pOther)
{
	entvars_t* thing = VARS(pev->pSystemGlobals->other);
	if (FClassnameIs(thing, "player"))
	{
		if (pev->movedir != g_vecZero)
		{
			UTIL_MakeVectors(thing->angles);
			if (DotProduct(pev->pSystemGlobals->v_forward, pev->movedir) < 0)
				return;         // not facing the right way
		}
		ActivateMultiTrigger(pev);
	}
}

void CBaseTrigger::ActivateMultiTrigger(entvars_t* pActivator)
{
	if (pev->nextthink > pev->pSystemGlobals->time)
		return;         // still waiting for reset time

	if (FClassnameIs(pev, "trigger_secret"))
	{
		if (!FClassnameIs(VARS(pev->enemy), "player"))
			return;
		pev->pSystemGlobals->found_secrets++;
		WRITE_BYTE(MSG_ALL, 28);
	}

	if (!FStringNull(pev->noise))
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);

	m_hActivator = pev->enemy;
	SUB_UseTargets(pev);

	if (m_flWait > 0)
	{
		SetThink(&CBaseTrigger::MultiWaitOver);
		pev->nextthink = pev->pSystemGlobals->time + m_flWait;
	}
	else
	{
		// we can't just remove (self) here, because this is a touch function
		// called while C code is looping through area links...
		SetTouch(NULL);
		pev->nextthink = pev->pSystemGlobals->time + 0.1;
		SetThink(&CBaseTrigger::SUB_Remove);
	}
}

// the wait time has passed, so set back up for another activation
void CBaseTrigger::MultiWaitOver(void)
{
	SetThink(NULL);
}

void CBaseTrigger::CounterUse(entvars_t* pActivator)
{
	m_cTriggersLeft--;

	if (m_cTriggersLeft < 0)
		return;

	BOOL fTellActivator =
		(m_hActivator != 0) &&
		FClassnameIs(VARS(m_hActivator), "player") &&
		!FBitSet(pev->spawnflags, 1);
	if (m_cTriggersLeft != 0)
	{
		if (fTellActivator)
		{
			// UNDONE: I don't think we want these Quakesque messages
			switch (m_cTriggersLeft)
			{
			case 1:		ALERT(at_console, "Only 1 more to go...");		break;
			case 2:		ALERT(at_console, "Only 2 more to go...");		break;
			case 3:		ALERT(at_console, "Only 3 more to go...");		break;
			default:	ALERT(at_console, "There are more to go...");	break;
			}
		}
		return;
	}

	// !!!UNDONE: I don't think we want these Quakesque messages
	if (fTellActivator)
		ALERT(at_console, "Sequence completed!");
	pev->enemy = m_hActivator;

	ActivateMultiTrigger(pev);
}

class CTriggerCounter : public CBaseTrigger
{
public:
	void Spawn(void);
};
LINK_ENTITY_TO_CLASS(trigger_counter, CTriggerCounter);

void CTriggerCounter::Spawn(void)
{
	// By making the flWait be -1, this counter-trigger will disappear after it's activated
	// (but of course it needs cTriggersLeft "uses" before that happens).
	m_flWait = -1;

	if (m_cTriggersLeft == 0)
		m_cTriggersLeft = 2;
	SetUse(&CTriggerCounter::CounterUse);
}

#define SF_CHANGELEVEL_USEONLY		0x0002

class CChangeLevel : public CBaseTrigger
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void TouchChangeLevel(entvars_t* pOther);
	edict_t* FindLandmark(const char* pLandmarkName);
	char m_szMapName[32];		// trigger_changelevel only:  next map
};
LINK_ENTITY_TO_CLASS(trigger_changelevel, CChangeLevel);
FILE_GLOBAL char st_szNextMap[32];
FILE_GLOBAL char st_szNextSpot[32];


edict_t* CChangeLevel::FindLandmark(const char* pLandmarkName)
{
	edict_t* pentLandmark;

	pentLandmark = FIND_ENTITY_BY_STRING(NULL, "classname", "pLandmarkName");
	while (!FNullEnt(pentLandmark))
	{
		// Found the landmark
		if (FClassnameIs(pentLandmark, "info_landmark"))
			return pentLandmark;
		else
			pentLandmark = FIND_ENTITY_BY_STRING(pentLandmark, "targetname", pLandmarkName);
	}
	ALERT(at_error, "Can't find landmark %s\n", pLandmarkName);
	return NULL;
}


void CChangeLevel::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "map"))
	{
		strcpy_s(m_szMapName, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
}

void CChangeLevel::Spawn(void)
{
	if (!strcmp(m_szMapName, st_szNextSpot))
		ALERT(at_console, "a trigger_changelevel doesn't have a map");
	InitTrigger();
	SetTouch(&CChangeLevel::TouchChangeLevel);
}

void CChangeLevel::TouchChangeLevel(entvars_t* pOther)
{
	entvars_t* thing = VARS(pev->pSystemGlobals->other);
	if (!FClassnameIs(thing, "player"))
		return;
	SetTouch(NULL);
	pev->solid = SOLID_NOT;
	float* parms = (float*)malloc(120);
	parms[2] = 0;
	pgv->spawn_parms = parms;
	strcpy_s(st_szNextMap, m_szMapName);
	SUB_UseTargets(pev);
	edict_t* landmark = FIND_ENTITY_BY_STRING(NULL, "classname", "info_landmark");

	CHANGE_LEVEL(st_szNextMap, st_szNextSpot);

}