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
extern Vector VecBModelOrigin(entvars_t* pevBModel);

#define SF_MULTIMAN_CLONE		0x80000000
#define SF_MULTIMAN_THREAD		0x00000001

class CMultiManager : public CBaseToggle
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Spawn(void);
	void ManagerThink(void);
	void ManagerUse(entvars_t *pActivator);

	int	m_cTargets;	// the total number of targets in this manager's fire list.
	int m_index; // Current target
	int	m_iTargetName[MAX_MULTI_TARGETS];// list if indexes into global string array
	float m_flTargetDelay[MAX_MULTI_TARGETS];// delay (in seconds) from time of manager fire to target fire
	float m_startTime;
};
LINK_ENTITY_TO_CLASS(multi_manager, CMultiManager);

void CMultiManager::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else // add this field to the target list
	{
		// this assumes that additional fields are targetnames and their values are delay values.
		if (m_cTargets < MAX_MULTI_TARGETS)
		{
			char tmp[128];

			m_iTargetName[m_cTargets] = ALLOC_STRING(pkvd->szKeyName);
			m_flTargetDelay[m_cTargets] = atof(pkvd->szValue);
			m_cTargets++;
			pkvd->fHandled = TRUE;
		}
	}
}

void CMultiManager::Spawn()
{
	pev->solid = SOLID_NOT;
	SetUse(&CMultiManager::ManagerUse);
	SetThink(&CMultiManager::ManagerThink);
}

void CMultiManager::ManagerThink()
{
	float	time;

	time = pev->pSystemGlobals->time - m_startTime;
	while (m_index < m_cTargets && m_flTargetDelay[m_index] <= time)
	{
		edict_t* entity = FIND_ENTITY_BY_STRING(NULL, "targetname", STRING(m_iTargetName[m_index]));
		if (!FNullEnt(entity))
		{
			const char* str2 = STRING(entity->v.targetname);
			const char* str3 = STRING(entity->v.classname);
			CBaseEntity* pTarget = CBaseEntity::Instance(entity);
			if (pTarget)
				pTarget->Use(0);
		}
		m_index++;
	}
	if (m_index >= m_cTargets)// have we fired all targets?
	{
		SetThink(NULL);
		SetUse(&CMultiManager::ManagerUse);// allow manager re-use 
	}
	else
		pev->nextthink = m_startTime + m_flTargetDelay[m_index];
}

void CMultiManager::ManagerUse(entvars_t *pActivator)
{

	// Sort targets
// Quick and dirty bubble sort
	int swapped = 1;

	while (swapped)
	{
		swapped = 0;
		for (int i = 1; i < m_cTargets; i++)
		{
			if (m_flTargetDelay[i] < m_flTargetDelay[i - 1])
			{
				// Swap out of order elements
				int name = m_iTargetName[i];
				float delay = m_flTargetDelay[i];
				m_iTargetName[i] = m_iTargetName[i - 1];
				m_flTargetDelay[i] = m_flTargetDelay[i - 1];
				m_iTargetName[i - 1] = name;
				m_flTargetDelay[i - 1] = delay;
				swapped = 1;
			}
		}
	}

	m_index = 0;
	m_startTime = pev->pSystemGlobals->time;

	SetUse(NULL);// disable use until all targets have fired

	SetThink(&CMultiManager::ManagerThink);
	pev->nextthink = pev->pSystemGlobals->time;
}


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

	CBaseMonster* otherthing = GetClassPtr((CBaseMonster*)thing);
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
	entvars_t* thing2 = VARS(pgv->other);
	if (!FClassnameIs(thing2, "player"))
		return;
	SetTouch(NULL);
	pev->solid = SOLID_NOT;
	float* parms = (float*)malloc(120);
	memset(parms, 0, 120);
	pgv->spawn_parms = parms;
	parms[12] = 0;
	strcpy_s(st_szNextMap, m_szMapName);
	SUB_UseTargets(pev);
	Vector v2; 
	v2.z = pev->size.z * 0.5 + pev->absmin.z;
	v2.y = pev->size.y * 0.5 + pev->absmin.y;
	v2.x = pev->size.x * 0.5 + pev->absmin.x;
	edict_t* entity = UTIL_FindEntityInSphere(v2, 256);
	edict_t* entity2 = NULL;
	while (1)
	{
		if (!OFFSET(entity))
			break;
		if (!strcmp(STRING(entity->v.classname), "info_landmark"))
		{
			parms[12] = 1;
			char string[32];
			sprintf_s(string, sizeof(string), STRING(entity->v.target));
			memcpy(&parms[13], &string, sizeof(string));
			parms[18] = thing->origin.x - entity->v.origin.x;
			parms[19] = thing->origin.y - entity->v.origin.y;
			parms[20] = thing->origin.z - entity->v.origin.z;
			parms[24] = thing->angles.x;
			parms[25] = thing->angles.y;
			parms[26] = thing->angles.z;
			parms[36] = thing->weapon;
			float weapon = parms[36];
			parms[21] = thing->velocity.x;
			parms[22] = thing->velocity.y;
			parms[23] = thing->velocity.z;
			parms[27] = thing->v_angle.x;
			parms[28] = thing->v_angle.y;
			parms[29] = thing->v_angle.z;
			parms[29] = 0;
			parms[26] = 0;
			pgv->spawn_parms = parms;
			break;
		}
		entity = ENT(entity->v.chain);
	}
	strcpy_s(st_szNextSpot, STRING(entity->v.target));

	CHANGE_LEVEL(st_szNextMap, st_szNextSpot);

}

// ============================== LADDER =======================================

class CLadder : public CBaseTrigger
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Spawn(void);
	void Touch(void);
};
LINK_ENTITY_TO_CLASS(func_ladder, CLadder);


void CLadder::KeyValue(KeyValueData* pkvd)
{
	return;
}

void CLadder::Spawn()
{
	pev->solid = SOLID_TRIGGER;
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));
	pev->movetype = MOVETYPE_PUSH;
	pev->renderamt = 0;
	pev->rendermode = 1;
}

void CLadder::Touch()
{
	entvars_t* thing = VARS(pev->pSystemGlobals->other);
	if (FClassnameIs(thing, "player"))
	{
		CBasePlayer* pPlayer = (CBasePlayer*)GET_PRIVATE(ENT(thing));
		pPlayer->g_onladder = 1;
	}
}