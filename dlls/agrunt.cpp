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

===== agrunt.cpp ========================================================

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

enum agrunt_anims
{
	idle1 = 0,
	idle2,
	idle3,
	walk,
	run,
	jump,	
	turn,
	crouch,
	smallflinch,
	bigflinch,
	attack,
	attack2,
	diesimple,
	dieforward,
	attack3
};

class CAlienGrunt : public CBaseMonster
{
public:
	virtual void Spawn();
	virtual int Classify() { return 2; };
	virtual void SetActivity(int activity);
	virtual void Alert(void);
	virtual void Pain(float flDamage);
	virtual void Die();
};

LINK_ENTITY_TO_CLASS(monster_alien_grunt, CAlienGrunt);

void CAlienGrunt::Spawn()
{
	PRECACHE_MODEL("models/agrunt.mdl");
	PRECACHE_MODEL("models/hornet.mdl");
	PRECACHE_SOUND("hornet/ag_buzz1.wav");
	PRECACHE_SOUND("hornet/ag_buzz2.wav");
	PRECACHE_SOUND("hornet/ag_buzz3.wav");
	PRECACHE_SOUND("agrunt/ag_alert1.wav");
	PRECACHE_SOUND("agrunt/ag_alert2.wav");
	PRECACHE_SOUND("agrunt/ag_alert3.wav");
	PRECACHE_SOUND("agrunt/ag_die1.wav");
	PRECACHE_SOUND("agrunt/ag_die2.wav");
	PRECACHE_SOUND("agrunt/ag_die3.wav");
	PRECACHE_SOUND("agrunt/ag_idle1.wav");
	PRECACHE_SOUND("agrunt/ag_idle2.wav");
	PRECACHE_SOUND("agrunt/ag_idle3.wav");
	PRECACHE_SOUND("agrunt/ag_idle4.wav");
	PRECACHE_SOUND("agrunt/ag_idle5.wav");
	PRECACHE_SOUND("agrunt/ag_idle6.wav");
	PRECACHE_SOUND("agrunt/ag_idle7.wav");
	PRECACHE_SOUND("agrunt/ag_pain1.wav");
	PRECACHE_SOUND("agrunt/ag_pain2.wav");
	PRECACHE_SOUND("agrunt/ag_pain3.wav");
	PRECACHE_SOUND("agrunt/ag_pain4.wav");
	PRECACHE_SOUND("agrunt/ag_pain5.wav");
	SET_MODEL(ENT(pev), "models/agrunt.mdl");
	UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 64));
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	pev->effects = 0;
	pev->health = UTIL_RandomFloat(0, 30) + 70;
	pev->yaw_speed = 8;
	pev->sequence = 15;
	//*(_DWORD*)(this + 256) = 512;
	m_bloodColor = -61;
	pev->nextthink = UTIL_RandomFloat(0, 0.5) + 0.5;
	SetThink(&CAlienGrunt::MonsterInit);
}

void CAlienGrunt::SetActivity(int activity)
{
	int activitynum = NULL;
	switch (activity)
	{
	case ACT_IDLE1:
		activitynum = idle1;
		break;
	case ACT_IDLE2:
		activitynum = idle1;
		break;
	case ACT_IDLE3:
		activitynum = idle2;
		break;
	case ACT_WALK:
		activitynum = walk;
		break;
	case 7:
		activitynum = idle1;
		break;
	case 8:
		activitynum = run;
		break;
	case 29:
		activitynum = attack;
		break;
	case 30:
		activitynum = attack3;
		break;
	case 35:
		activitynum = diesimple;
		break;
	case 37:
		activitynum = dieforward;
		break;
	}
	if (pev->sequence != activitynum)
	{
		pev->sequence = activitynum;
		if ((activitynum != 4 && activitynum != 3) || (pev->sequence != 4 && pev->sequence != 4))
			pev->frame = 0;
		ResetSequenceInfo(0.1);
		switch (activitynum)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 7:
		case 10:
		case 12:
		case 13:
		case 14:
			return;
		default:
			ALERT(at_console, "Bogus AGrunt anim: %d", activitynum);
			//this[66] = 0.0;
			//this[67] = 0.0;
			break;
		}
	}
}

void CAlienGrunt::Die()
{
	switch (rand() % 3)
	{
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "agrunt/ag_die1.wav", 1, ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "agrunt/ag_die2.wav", 1, ATTN_NORM);
		break;
	case 2:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "agrunt/ag_die3.wav", 1, ATTN_NORM);
		break;
	default:
		break;
	}
	m_iActivity = 35;
	pev->ideal_yaw = pev->angles.y;
	pev->nextthink = pev->pSystemGlobals->time + 0.1;
	SetThink(&CBaseMonster::CallMonsterThink);
	SetActivity(m_iActivity);
}

void CAlienGrunt::Pain(float flDamage)
{
	switch (rand() % 5)
	{
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "agrunt/ag_pain1.wav", 1, ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "agrunt/ag_pain2.wav", 1, ATTN_NORM);
		break;
	case 2:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "agrunt/ag_pain3.wav", 1, ATTN_NORM);
		break;
	case 3:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "agrunt/ag_pain4.wav", 1, ATTN_NORM);
		break;
	case 4:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "agrunt/ag_pain5.wav", 1, ATTN_NORM);
		break;
	default:
		break;
	}
	pev->pain_finished = pev->pSystemGlobals->time + 1;
	//if (this[32] == 1 || this[32] == 4)
	//	Alert();
}

void CAlienGrunt::Alert()
{
	switch (rand() % 3)
	{
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "agrunt/ag_alert1.wav", 1, ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "agrunt/ag_alert2.wav", 1, ATTN_NORM);
		break;
	case 2:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "agrunt/ag_alert3.wav", 1, ATTN_NORM);
		break;
	default:
		break;
	}
	//*(float*)(this + 272) = pev->pSystemGlobals->time + 1.0;
}