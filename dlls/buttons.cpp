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

===== buttons.cpp ========================================================

  button-related code

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "doors.h"

#if !defined ( _WIN32 )
#include <string.h> // memset())))
#endif

#define SF_BUTTON_DONTMOVE		1
#define SF_ROTBUTTON_NOTSOLID	1
#define	SF_BUTTON_TOGGLE		32	// button stays pushed until reactivated
#define	SF_BUTTON_SPARK_IF_OFF	64	// button sparks in OFF state
#define SF_BUTTON_TOUCH_ONLY	256	// button only fires as a result of USE key.

#define SF_GLOBAL_SET			1	// Set global state to initial state on spawn

LINK_ENTITY_TO_CLASS(multisource, CMultiSource);
//
// Cache user-entity-field values until spawn is called.
//

void CMultiSource::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "style") ||
		FStrEq(pkvd->szKeyName, "height") ||
		FStrEq(pkvd->szKeyName, "killtarget") ||
		FStrEq(pkvd->szKeyName, "value1") ||
		FStrEq(pkvd->szKeyName, "value2") ||
		FStrEq(pkvd->szKeyName, "value3"))
		pkvd->fHandled = TRUE;
}

void CMultiSource::Spawn()
{
	// set up think for later registration

	pev->nextthink = m_iTotal + 1.0;
	SetThink(&CMultiSource::Register);
	SetUse(&CBaseEntity::SUB_WrapperCallUseToggle);
}

void CMultiSource::Use(entvars_t* pActivator)
{
	if (m_rgEntities <= 0)
		return;
	m_rgEntities -= 1;
	if (!m_rgEntities)
		return;
	if (!pev->target)
		return;
	edict_t* target = FIND_ENTITY_BY_STRING(NULL, "targetname", STRING(pev->target));
	pev->pSystemGlobals->self = OFFSET(target);
	pev->pSystemGlobals->other = pev->pSystemGlobals->self;
	CBaseEntity* pTarget = CBaseEntity::Instance(target);
	if (pTarget)
		pTarget->Use(0);
}

void CMultiSource::IsTriggered(entvars_t* pActivator)
{
	if (m_iTotal > m_rgEntities)
		m_rgEntities++;
}

void CMultiSource::Register()
{
	m_iTotal = 0;
	m_rgEntities = 0;
	SetThink(&CBaseEntity::SUB_DoNothing);
	edict_t *pentTarget = FIND_ENTITY_BY_STRING(NULL, "target", STRING(pev->targetname));
	//while (!FNullEnt(pentTarget) && (m_iTotal < MS_MAX_TARGETS))
	//{
	//	CBaseEntity* pTarget = CBaseEntity::Instance(pentTarget);
	//	if (pTarget)
	//		m_rgEntities[m_iTotal++] = pTarget;
	//
	//	pentTarget = FIND_ENTITY_BY_STRING(pentTarget, "target", STRING(pev->targetname));
	//}

}

void CBaseButton::KeyValue(KeyValueData* pkvd)
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
	else if (FStrEq(pkvd->szKeyName, "delay"))
	{
		m_flDelay = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "distance"))
	{
		m_flMoveDistance = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_sMaster = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
}

//void CBaseButton::Pain( ) ???is cbasebutton based on cbasemonster what do you mean
void CBaseButton::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage)
{
	pev->health = 9999;
	if (m_toggle_state == TS_GOING_UP && m_toggle_state == TS_GOING_DOWN)
		return;
	m_hActivator = pev->pSystemGlobals->other;
	if (m_toggle_state == TS_AT_TOP)
	{
		if (!m_fStayPushed)
		{
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);

			SUB_UseTargets( pev );
			ButtonReturn();
		}
	}
	else
		ButtonActivate();
}

LINK_ENTITY_TO_CLASS(func_button, CBaseButton);

void CBaseButton::Spawn()
{
	char* pszSound;

	//----------------------------------------------------
	//determine sounds for buttons
	//a sound of 0 should not make a sound
	//----------------------------------------------------
	pszSound = ButtonSound(m_sounds);
	PRECACHE_SOUND(pszSound);
	pev->noise = ALLOC_STRING(pszSound);

	if (FBitSet(pev->spawnflags, SF_BUTTON_SPARK_IF_OFF))// this button should spark in OFF state
	{
		PRECACHE_SOUND("buttons/spark1.wav");
		PRECACHE_SOUND("buttons/spark2.wav");
		PRECACHE_SOUND("buttons/spark3.wav");
		PRECACHE_SOUND("buttons/spark4.wav");
		PRECACHE_SOUND("buttons/spark5.wav");
		PRECACHE_SOUND("buttons/spark6.wav");
		SetThink(&CBaseButton::ButtonSpark);
		pev->nextthink = pev->pSystemGlobals->time + 0.5;// no hurry, make sure everything else spawns
	}
	SetMovedir(pev);

	pev->movetype = MOVETYPE_PUSH;
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 40;

	if (pev->health > 0)
	{
		pev->takedamage = DAMAGE_YES;
	}

	if (m_flWait == 0)
		m_flWait = 1;
	if (m_flLip == 0)
		m_flLip = 4;

	m_toggle_state = TS_AT_BOTTOM;
	m_vecPosition1 = pev->origin;

	m_vecPosition2 = m_vecPosition1 + (pev->movedir * (fabs(pev->movedir.x * pev->size.x) + fabs(pev->movedir.y * pev->size.y) + fabs(pev->movedir.z * pev->size.z) - m_flLip));


	// Is this a non-moving button?
	if (((m_vecPosition2 - m_vecPosition1).Length() < 1) || FBitSet(pev->spawnflags, SF_BUTTON_DONTMOVE))
		m_vecPosition2 = m_vecPosition1;

	m_fStayPushed = (m_flWait == -1 ? TRUE : FALSE);
	m_fRotating = FALSE;

	// if the button is flagged for USE button activation only, take away it's touch function and add a use function

	if (FBitSet(pev->spawnflags, SF_BUTTON_TOUCH_ONLY)) // touchable button
	{
		SetTouch(&CBaseButton::ButtonTouch);
	}
	else
	{
		SetTouch(NULL);
		SetUse(&CBaseButton::ButtonUse);
	}
}

char* ButtonSound(int sound)
{
	char* pszSound;

	switch (sound)
	{
	case 0: pszSound = "common/null.wav";        break;
	case 1: pszSound = "buttons/button1.wav";	break;
	case 2: pszSound = "buttons/button2.wav";	break;
	case 3: pszSound = "buttons/button3.wav";	break;
	case 4: pszSound = "buttons/button4.wav";	break;
	case 5: pszSound = "buttons/button5.wav";	break;
	case 6: pszSound = "buttons/button6.wav";	break;
	case 7: pszSound = "buttons/button7.wav";	break;
	case 8: pszSound = "buttons/button8.wav";	break;
	case 9: pszSound = "buttons/button9.wav";	break;
	case 10: pszSound = "buttons/button10.wav";	break;
	case 11: pszSound = "buttons/button11.wav";	break;
	default:pszSound = "buttons/button9.wav";	break;
	}

	return pszSound;
}

void CBaseButton::ButtonSpark()
{
	SetThink(&CBaseButton::ButtonSpark);
	pev->nextthink = pev->pSystemGlobals->time + (0.1 + UTIL_RandomFloat(0, 1.5));// spark again at random interval
	WRITE_BYTE(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(MSG_BROADCAST, TE_SPARKS);
	WRITE_COORD(MSG_BROADCAST, pev->size.x / 2 + pev->mins.x);
	WRITE_COORD(MSG_BROADCAST, pev->size.y / 2 + pev->mins.y);
	WRITE_COORD(MSG_BROADCAST, pev->size.z / 2 + pev->mins.z);
	float flVolume = UTIL_RandomFloat(0.25, 0.75);//random volume range
	switch ((int)(UTIL_RandomFloat(0, 1) * 6))
	{
		case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark1.wav", flVolume, ATTN_NORM);	break;
		case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark2.wav", flVolume, ATTN_NORM);	break;
		case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark3.wav", flVolume, ATTN_NORM);	break;
		case 3: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark4.wav", flVolume, ATTN_NORM);	break;
		case 4: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark5.wav", flVolume, ATTN_NORM);	break;
		case 5: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark6.wav", flVolume, ATTN_NORM);	break;
	}
}

void CBaseButton::ButtonUse(entvars_t* pActivator)
{
	ALERT(at_console, "%d\n", m_fStayPushed);
	if (m_toggle_state == TS_GOING_UP && m_toggle_state == TS_GOING_DOWN)
		return;
	m_hActivator = pev->pSystemGlobals->other;
	if (m_toggle_state == TS_AT_TOP)
	{
		if (!m_fStayPushed && FBitSet(pev->spawnflags, SF_BUTTON_TOGGLE))
		{
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);

			SUB_UseTargets(pev);
			ButtonReturn();
		}
	}
	else
		ButtonActivate();
}

void CBaseButton::ButtonTouch(entvars_t* pActivator)
{
	entvars_t* thing = VARS(pev->pSystemGlobals->other);
	edict_t* v10 = NULL;
	if (!FClassnameIs(thing, "player"))
		return;
	if (m_toggle_state != TS_GOING_UP && m_toggle_state != TS_GOING_DOWN && (m_toggle_state == TS_AT_BOTTOM || m_fStayPushed || FBitSet(pev->spawnflags, SF_BUTTON_TOGGLE)))
	{
		edict_t* target = FIND_ENTITY_BY_STRING(NULL, "targetname", STRING(pev->target));
		CMultiSource* pTarget = GetClassPtr((CMultiSource*)target);
		if (!target)
			goto label_19;
		if (!OFFSET(target))
			goto label_19;
		if (!FClassnameIs(target, "multisource"))
			goto label_19;
		if (!pTarget->m_iTotal)
		{
			label_19:
			SetTouch(NULL);
			m_iszKillTarget = pev->pSystemGlobals->other;
			if (!m_fStayPushed && FBitSet(pev->spawnflags, SF_BUTTON_TOGGLE || m_toggle_state == TS_AT_BOTTOM))
				ButtonActivate();
			else
			{
				EMIT_SOUND(ENT(pev), CHAN_VOICE, STRING(pev->noise), 1, ATTN_NORM);
				SUB_UseTargets(pev);
				ButtonReturn();
			}
		}
	}

}

void CBaseButton::ButtonActivate()
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);
	m_toggle_state = TS_GOING_UP;
	SetMoveDone(&CBaseButton::TriggerAndWait);

	if (!m_fRotating)
		LinearMove(m_vecPosition2, pev->speed);
	else
		AngularMove(m_vecAngle2, pev->speed);
}

//
// Button has reached the "in/up" position.  Activate its "targets", and pause before "popping out".
//
void CBaseButton::TriggerAndWait(void)
{
	m_toggle_state = TS_AT_TOP;

	// If button automatically comes back out, start it moving out.
	// Else re-instate touch method
	if (m_fStayPushed || FBitSet(pev->spawnflags, SF_BUTTON_TOGGLE))
	{
		if (!FBitSet(pev->spawnflags, SF_BUTTON_TOUCH_ONLY)) // this button only works if USED, not touched!
		{
			// ALL buttons are now use only
			SetTouch(NULL);
		}
		else
			SetTouch(&CBaseButton::ButtonTouch);
	}
	else
	{
		pev->nextthink = pev->ltime + m_flWait;
		SetThink(&CBaseButton::ButtonReturn);
	}

	pev->frame = 1;			// use alternate textures


	SUB_UseTargets(pev);
}

void CBaseButton::ButtonReturn()
{
	m_toggle_state = TS_GOING_DOWN;

	SetMoveDone(&CBaseButton::ButtonBackHome);
	if (!m_fRotating)
		LinearMove(m_vecPosition1, pev->speed);
	else
		AngularMove(m_vecAngle1, pev->speed);

	pev->frame = 0;			// use normal textures
}

//
// Button has returned to start state.  Quiesce it.
//
void CBaseButton::ButtonBackHome(void)
{
	m_toggle_state = TS_AT_BOTTOM;

	if (!FStringNull(pev->target))
	{
		edict_t* pentTarget = NULL;
		edict_t* nextTarget = NULL;
		for (;;)
		{
			pentTarget = FIND_ENTITY_BY_STRING(nextTarget, "targetname", STRING(pev->target));
			nextTarget = pentTarget;
			if (FNullEnt(pentTarget))
				break;

			if (!FClassnameIs(pentTarget, "multisource"))
				continue;
			CMultiSource* pTarget = GetClassPtr((CMultiSource*)pentTarget);

			if (pTarget)
				pTarget->Use(0);
		}
	}
	// Re-instate touch method, movement cycle is complete.
	if (!FBitSet(pev->spawnflags, SF_BUTTON_TOUCH_ONLY)) // this button only works if USED, not touched!
	{
		// All buttons are now use only	
		SetTouch(NULL);
	}
	else
		SetTouch(&CBaseButton::ButtonTouch);

	// reset think for a sparking button
	if (FBitSet(pev->spawnflags, SF_BUTTON_SPARK_IF_OFF))
	{
		SetThink(&CBaseButton::ButtonSpark);
		pev->nextthink = pev->pSystemGlobals->time + 0.5;// no hurry.
	}
}
