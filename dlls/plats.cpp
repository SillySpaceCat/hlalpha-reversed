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

===== plats.cpp ========================================================

  spawn, think, and touch functions for trains, etc

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

static void PlatSpawnInsideTrigger(entvars_t* pevPlatform);

class CFuncPlat : public CBaseToggle
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void Blocked(entvars_t* pOther);
	void PlatUse(entvars_t* pActivator);

	void GoUp(void);
	void GoDown(void);
	void HitTop(void);
	void HitBottom(void);
};
LINK_ENTITY_TO_CLASS(func_plat, CFuncPlat);

class CPlatTrigger : public CBaseEntity
{
public:
	void SpawnInsideTrigger(CFuncPlat* pPlatform);
	CFuncPlat* m_pPlatform;
};

void CFuncPlat::HitTop()
{
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, STRING(pev->noise1), 1, ATTN_NORM);
	m_toggle_state = TS_AT_TOP;
	SetThink(&CFuncPlat::GoDown);
	pev->nextthink = pev->ltime + 3;
}

void CFuncPlat::HitBottom()
{
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, STRING(pev->noise1), 1, ATTN_NORM);
	m_toggle_state = TS_AT_BOTTOM;
}

void CFuncPlat::GoUp()
{
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, STRING(pev->noise), 1, ATTN_NORM);
	m_toggle_state = TS_GOING_UP;
	SetMoveDone(&CFuncPlat::HitTop);
	LinearMove(m_vecPosition1, pev->speed);
}

void CFuncPlat::GoDown()
{
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, STRING(pev->noise), 1, ATTN_NORM);
	m_toggle_state = TS_GOING_DOWN;
	SetMoveDone(&CFuncPlat::HitBottom);
	LinearMove(m_vecPosition2, pev->speed);
}
void CFuncPlat::Blocked(entvars_t* pActivator)
{
	entvars_t* thing = VARS(pev->pSystemGlobals->other);
	CBaseMonster* otherthing = GetClassPtr((CBaseMonster*)thing);
	otherthing->TakeDamage(pev, 1);
	if (m_toggle_state == TS_GOING_UP)
		GoDown();
	else if (m_toggle_state == TS_GOING_DOWN)
		GoUp();
}

void CFuncPlat::PlatUse(entvars_t* pActivator)
{
	SetUse(NULL);
	if (m_toggle_state == TS_AT_TOP)
	{
		GoDown();
	}
}

void CFuncPlat::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "lip"))
	{
		m_flLip = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "height"))
	{
		m_flHeight = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
}

void CFuncPlat::Spawn(void)
{
	PRECACHE_SOUND("plats/platmove1.wav");
	PRECACHE_SOUND("plats/platstop1.wav");
	pev->noise1 = ALLOC_STRING("plats/platmove1.wav");
	pev->noise2 = ALLOC_STRING("plats/platstop1.wav");
	if (m_flTLength == 0)
		m_flTLength = 80;
	if (m_flTWidth == 0)
		m_flTWidth = 10;
	pev->angles = g_vecZero;
	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 150;

	m_vecPosition1 = pev->origin;
	m_vecPosition2 = pev->origin;

	if (m_flHeight != 0)
		m_vecPosition2.z = pev->origin.z - m_flHeight;
	else
		m_vecPosition2.z = pev->origin.z - pev->size.z + 8;
	PlatSpawnInsideTrigger(pev);
	if (pev->targetname)
	{
		UTIL_SetOrigin(pev, m_vecPosition1);
		m_toggle_state = TS_AT_TOP;
		SetUse(&CFuncPlat::PlatUse);
	}
	else
	{
		UTIL_SetOrigin(pev, m_vecPosition2);
		m_toggle_state = TS_AT_BOTTOM;
	}
}

static void PlatSpawnInsideTrigger(entvars_t* pevPlatform)
{
	GetClassPtr((CPlatTrigger*)NULL)->SpawnInsideTrigger(GetClassPtr((CFuncPlat*)pevPlatform));
}

void CPlatTrigger::SpawnInsideTrigger(CFuncPlat* pPlatform)
{
	m_pPlatform = pPlatform;
	// Create trigger entity, "point" it at the owning platform, give it a touch method
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	pev->origin = pPlatform->pev->origin;

	// Establish the trigger field's size
	Vector vecTMin = m_pPlatform->pev->mins + Vector(25, 25, 0);
	Vector vecTMax = m_pPlatform->pev->maxs + Vector(25, 25, 8);
	vecTMin.z = vecTMax.z - (m_pPlatform->m_vecPosition1.z - m_pPlatform->m_vecPosition2.z + 8);
	if (m_pPlatform->pev->size.x <= 50)
	{
		vecTMin.x = (m_pPlatform->pev->mins.x + m_pPlatform->pev->maxs.x) / 2;
		vecTMax.x = vecTMin.x + 1;
	}
	if (m_pPlatform->pev->size.y <= 50)
	{
		vecTMin.y = (m_pPlatform->pev->mins.y + m_pPlatform->pev->maxs.y) / 2;
		vecTMax.y = vecTMin.y + 1;
	}
	UTIL_SetSize(pev, vecTMin, vecTMax);
}