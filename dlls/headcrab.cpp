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

===== headcrab.cpp ========================================================

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CHeadCrab : public CBaseMonster
{
public:
	virtual void Spawn();
	virtual void SetActivity(int activity);
	virtual int Classify() { return 8; };
	virtual void Alert();
	virtual void Pain(int a2);
	virtual void Die();
	virtual void Idle();
	virtual int CheckEnemy(int a2, float a3);
	//void leap();
	//void leap_touch;
};

LINK_ENTITY_TO_CLASS(monster_headcrab, CHeadCrab);

void CHeadCrab::Spawn()
{
	PRECACHE_SOUND("headcrab/hc_attack1.wav");
	PRECACHE_SOUND("headcrab/hc_alert1.wav");
	PRECACHE_SOUND("headcrab/hc_die1.wav");
	PRECACHE_SOUND("headcrab/hc_die2.wav");
	PRECACHE_SOUND("headcrab/hc_pain1.wav");
	PRECACHE_SOUND("headcrab/hc_pain2.wav");
	PRECACHE_SOUND("headcrab/hc_pain3.wav");
	PRECACHE_SOUND("headcrab/hc_idle1.wav");
	PRECACHE_SOUND("headcrab/hc_idle2.wav");
	PRECACHE_SOUND("headcrab/hc_idle3.wav");
	PRECACHE_SOUND("headcrab/hc_headbite.wav");
	PRECACHE_MODEL("models/headcrab.mdl");
	SET_MODEL(ENT(pev), "models/headcrab.mdl");
	UTIL_SetSize(pev, Vector(-12, -12, 0), Vector(12, 12, 24));
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	pev->effects = 0;
	pev->health = 12;
	pev->yaw_speed = 10;
	pev->sequence = 8;
	m_bloodColor = 54;
	pev->nextthink += UTIL_RandomFloat(0, 0.5) + 0.5;
	SetThink(&CHeadCrab::MonsterInit);
}

void CHeadCrab::SetActivity(int activity)
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
		activitynum = 0;
		break;
	case 4:
		activitynum = 1;
		break;
	case 6:
		activitynum = 0;
		break;
	case 7:
		activitynum = 0;
		break;
	case 8:
		activitynum = 2;
		break;
	case 9:
		activitynum = 2;
		break;
	case 18:
		activitynum = 5;
		break;
	case 30:
		activitynum = 4;
		break;
	case 31:
		return;
		break;
	case 33:
		activitynum = 5;
		break;
	case 35:
		activitynum = 6;
		break;
	case 41:
		activitynum = 7;
		break;
	default:
		ALERT(at_console, "Headcrab's monster state is bogus: %d", activity);
		break;
	}
	if (pev->sequence != activitynum)
	{
		pev->sequence = activitynum;
		pev->frame = 0;
		ResetSequenceInfo(0.1);
	}
}

void CHeadCrab::Pain(int a2)
{
	int sound = rand() % 3;
	if (sound == 1)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "headcrab/hc_pain3.wav", 1, ATTN_NORM);
	}
	else if (sound == 2)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "headcrab/hc_pain2.wav", 1, ATTN_NORM);
	}
	else
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "headcrab/hc_pain1.wav", 1, ATTN_NORM);
	}
	if (a2 < 5)
	{
		SetThink(&CHeadCrab::CallMonsterThink);
		//if (pev->enemy)
			//this[32] = 6;
	}
}

void CHeadCrab::Die()
{
	if (pev->health > -30)
	{
		switch (rand() % 2)
		{
		case 0:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "headcrab/hc_die2.wav", 1, ATTN_NORM);
			break;
		case 1:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "headcrab/hc_die1.wav", 1, ATTN_NORM);
			break;
		default:
			break;
		}
	}
	//get_death_type(0);
}

void CHeadCrab::Alert()
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "headcrab/hc_alert1.wav", 1, ATTN_NORM);
	//*(_DWORD*)(this + 128) = 31;
	//*(float*)(this + 272) = pev->pSystemGlobals->time + 1;
}

void CHeadCrab::Idle()
{
	float sound = UTIL_RandomFloat(0.0, 1.0);
	if (sound <= 0.33)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "headcrab/hc_idle1.wav", 1, ATTN_NORM);
	}
	else if (sound <= 0.66)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "headcrab/hc_idle2.wav", 1, ATTN_NORM);
	}
	else
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "headcrab/hc_idle3.wav", 1, ATTN_NORM);
	}
	//nextidlesound = pev->pSystemGlobals->time + UTIL_RandomFloat(0, 2) + 3
}

int CHeadCrab::CheckEnemy(int a2, float a3)
{
	/*
	if (!checkenemyoncrosshairs((int)this, a2) || a3 > (double)flt_1002ECA4)
		return 0;
	this[34] = 7;
	this[3] = Headcrab_leap;
	return 1;
	*/
	return 0;
}