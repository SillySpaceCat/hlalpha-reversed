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

===== hgrunt.cpp ========================================================

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

enum hgrunt_anims
{
	walk1 = 0,
	walk2,
	run,
	diesimple,
	dieforward,
	crouch,
	standup,
	crouching,
	crawl,
	smflinch,
	fire_grenade,
	notarget,
	idle1,
	idle2,
	reload,
	shootcycle,
	frontkick,
	swim,
	signal,
	_180L,
	_180R,
	strafeleft,
	straferight
};

class CHumanGrunt : public CBaseMonster
{
public:
	float painvariable;
	virtual void Spawn();
	virtual int Classify() { return 1; };
	virtual void SetActivity(int activity);
	virtual void Alert(void);
	virtual void Pain(float flDamage);
	virtual void Die();
};

LINK_ENTITY_TO_CLASS(monster_human_grunt, CHumanGrunt);

void CHumanGrunt::Spawn()
{
	PRECACHE_MODEL("models/hgrunt.mdl");
	PRECACHE_SOUND("player/hoot1.wav");
	PRECACHE_SOUND("hgrunt/gr_reload1.wav");
	PRECACHE_SOUND("hgrunt/gr_loadtalk.wav");
	PRECACHE_SOUND("hgrunt/gr_pain1.wav");
	PRECACHE_SOUND("hgrunt/gr_pain2.wav");
	PRECACHE_SOUND("hgrunt/gr_pain3.wav");
	PRECACHE_SOUND("hgrunt/gr_pain4.wav");
	PRECACHE_SOUND("hgrunt/gr_pain5.wav");
	PRECACHE_SOUND("hgrunt/gr_alert1.wav");
	PRECACHE_SOUND("hgrunt/gr_idle1.wav");
	PRECACHE_SOUND("hgrunt/gr_idle2.wav");
	PRECACHE_SOUND("hgrunt/gr_idle3.wav");
	PRECACHE_SOUND("hgrunt/gr_mgun.wav");
	PRECACHE_SOUND("hgrunt/gr_mgun2.wav");
	PRECACHE_SOUND("hgrunt/gr_mgun3.wav");
	PRECACHE_SOUND("hgrunt/gr_die1.wav");
	PRECACHE_SOUND("hgrunt/gr_die2.wav");
	PRECACHE_SOUND("hgrunt/gr_die3.wav");
	SET_MODEL(ENT(pev), "models/hgrunt.mdl");
	UTIL_SetSize(pev, Vector(-18, -18, 0), Vector(18, 18, 72));
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	pev->effects = 0;
	pev->health = UTIL_RandomFloat(0, 25) + 50;
	pev->yaw_speed = 14;
	pev->sequence = 23;
	//*(_DWORD*)(this + 312) = 0;
	//*(_DWORD*)(this + 276) = 0;
	//*(_DWORD*)(this + 304) = 1;
	//*(_DWORD*)(this + 272) = 0;
	//*(_DWORD*)(this + 252) = 45;
	//*(_DWORD*)(this + 256) = 512;
	m_bloodColor = 70;
	pev->nextthink = UTIL_RandomFloat(0, 0.5) + 0.5;
	SetThink(&CHumanGrunt::MonsterInit);
}

void CHumanGrunt::SetActivity(int activity)
{
	int activitynum = NULL;
	switch (activity)
	{
	case ACT_IDLE1:
		activitynum = notarget;
		break;
	case ACT_IDLE2:
		activitynum = notarget;
		break;
	case ACT_IDLE3:
		activitynum = notarget;
		break;
	case ACT_WALK:
		activitynum = walk1;
		break;
	case 5:
		activitynum = notarget;
		break;
	case 6:
		activitynum = notarget;
		break;
	case 7:
		activitynum = idle2;
		break;
	case 8:
		activitynum = run;
		break;
	case 9:
		activitynum = run;
		break;
	case 11:
		activitynum = strafeleft;
		break;
	case 12:
		activitynum = straferight;
		break;
	case 18:
		activitynum = smflinch;
		break;
	case 20:
	case 25:
	case 26:
	case 27:
	case 31:
		return;
	case 29:
		activitynum = frontkick;
		break;
	case 30:
		activitynum = shootcycle;
		break;
	case 32:
		activitynum = reload;
		break;
	case 33:
		activitynum = smflinch;
		break;
	case 35:
		activitynum = diesimple;
		break;
	case 37:
		activitynum = dieforward;
		break;
	default:
		ALERT(at_console, "HGrunt's monster state is bogus: %d", activity);
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
		case 2:
		case 3:
		case 4:
		case 9:
		case 14:
		case 15:
		case 16:
		case 21:
		case 22:
			return;
		case 11:
		case 12:
		case 13:
			pev->frame = UTIL_RandomFloat(0, 1) * 256;
		default:
			ALERT(at_console, "Bogus HGrunt anim: %d", activitynum);
			//this[66] = 0.0;
			//this[67] = 0.0;
			break;
		}
	}
}

void CHumanGrunt::Die()
{
	float sound = UTIL_RandomFloat(0, 1);
	if (pev->health > -30)
	{
		if (sound <= 0.33)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_die1.wav", 1, ATTN_NORM);
		else if (sound <= 0.66)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_die2.wav", 1, ATTN_NORM);
		else
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_die3.wav", 1, ATTN_NORM);
	}
	m_iActivity = 35;
	pev->ideal_yaw = pev->angles.y;
	pev->nextthink = pev->pSystemGlobals->time + 0.1;
	SetThink(&CBaseMonster::CallMonsterThink);
	SetActivity(m_iActivity);
}

void CHumanGrunt::Pain(float flDamage)
{
	float sound = UTIL_RandomFloat(0, 1);
	pev->pain_finished = pev->pSystemGlobals->time + 1;
	if (pev->pSystemGlobals->time <= painvariable)
	{
		if (sound <= 0.33)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_pain3.wav", 1, ATTN_NORM);
		else if (sound <= 0.66)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_pain4.wav", 1, ATTN_NORM);
		else
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_pain5.wav", 1, ATTN_NORM);
	}
	else
	{
		if (sound <= 0.2)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_pain1.wav", 1, ATTN_NORM);
		else if (sound <= 0.4)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_pain2.wav", 1, ATTN_NORM);
		else if (sound <= 0.6)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_pain3.wav", 1, ATTN_NORM);
		else if (sound <= 0.8)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_pain4.wav", 1, ATTN_NORM);
		else
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_pain5.wav", 1, ATTN_NORM);
		painvariable = pgv->time + 15;

	}
}

void CHumanGrunt::Alert()
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "hgrunt/gr_alert1.wav", 1, ATTN_NORM);
	//*(float*)(this + 272) = pev->pSystemGlobals->time + 1.0;
}