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

===== barney.cpp ========================================================

  frEEze

*/

enum barney_anims
{
    idle1 = 0,
    idle2,
    idle3,
    walk,
    run,
    walkdraw,
    draw,
    drawslow,
    walkshoot,
    aim,
    shootgun,
    replacegun,
    diesimple,
    dieforward,
    diebackward,
    dieviolent,
    diecrump
};

#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CBarney : public CBaseMonster
{
public:
	virtual void Spawn();
	virtual int Classify() { return 5; };
	virtual void SetActivity(int activity);
	void Barney_Pissed();
    void Barney_Shoot();
	virtual void Pain(float flDamage);
	virtual void Die();
	virtual int CheckEnemy(entvars_t *a2, float a3);
};

LINK_ENTITY_TO_CLASS(monster_barney, CBarney);

void CBarney::Spawn()
{
	PRECACHE_SOUND("barney/ba_attack1.wav");
	PRECACHE_SOUND("barney/ba_attack2.wav");
	PRECACHE_SOUND("barney/ba_die1.wav");
	PRECACHE_SOUND("barney/ba_die2.wav");
	PRECACHE_SOUND("barney/ba_die3.wav");
	PRECACHE_SOUND("barney/ba_die4.wav");
	PRECACHE_SOUND("barney/ba_pain1.wav");
	PRECACHE_MODEL("models/barney.mdl");
	SET_MODEL(ENT(pev), "models/barney.mdl");
	UTIL_SetSize(pev, VEC_BARNEY_HULL_MIN, VEC_BARNEY_HULL_MAX);
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	pev->effects = 0;
	pev->health = 7;
	pev->yaw_speed = 8;
	pev->sequence = 17;
	m_bloodColor = 70;
	m_sightDistance = 384;
	m_followDistance = 128;
    pev->nextthink += UTIL_RandomFloat(0.0, 0.5) + 0.5;
	SetThink(&CBarney::MonsterInit);
}

void CBarney::SetActivity(int activity)
{
    int activitynum = NULL;
    Vector v4;
    float length;
    switch (activity)
    {
    case ACT_IDLE1:
        activitynum = idle1;
        break;
    case ACT_IDLE2:
        activitynum = idle2;
        break;
    case ACT_IDLE3:
        activitynum = idle3;
        break;
    case ACT_WALK:
        activitynum = walk;
        break;
    case ACT_AIM:
        activitynum = aim;
        break;
    case ACT_AIM2:
        activitynum = aim;
        break;
    case ACT_RUN:
        activitynum = run;
        break;
    case ACT_RUN2:
        activitynum = run;
        break;
    case ACT_FOLLOWPLAYER:                                    // player interacts with barney
        v4 = (pev->origin - followentity->origin);
        length = v4.Length();
        if (m_followDistance * 2.0 >= length)
        {
            if (m_followDistance < length)
                activitynum = walk;
            else
                activitynum = idle1;
        }
        else
        {
            activitynum = run;
        }
        break;
    case 29:
        activitynum = run;
        break;
    case 30:
        activitynum = shootgun;
        break;
    case 35:
    case 38:
        activitynum = diebackward;
        break;
    case 36:
        activitynum = dieviolent;
        break;
    case 37:
        activitynum = dieforward;
        break;
    default:
        ALERT(at_console, "Barney's monster state is bogus: %d", activity);
        break;
    }
    if (pev->sequence != activitynum)
    {
        pev->sequence = activitynum;
        if ((activitynum != 4 && activitynum != 3) || (pev->sequence != 4 && pev->sequence != 3))
            pev->frame = 0;
        ResetSequenceInfo(0.1);
        switch (activitynum)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            return;
        default:
            ALERT(at_console, "Bogus Barney anim: %d", activitynum);
            m_flFrameRate = 0.0;
            m_flGroundSpeed = 0.0;
            break;
        }
    }
}

void CBarney::Barney_Pissed()
{
    EMIT_SOUND(ENT(pev), CHAN_VOICE, "barney/ba_attack1.wav", 1, ATTN_NORM);
    m_iActivity = 6;
    nextattack = pev->pSystemGlobals->time + 1;
}

void CBarney::Barney_Shoot()
{
    pev->nextthink = pgv->time + 0.05;
    if (m_iActivity != 30)
    {
        m_iActivity = 30;
        SetActivity(30);
        nextattack = UTIL_RandomFloat(0.5, 1.5) + pgv->time;
    }
    byte dispatchanimevents = DispatchAnimEvents(0.1);
    StudioFrameAdvance(0.1);
    CHANGE_YAW(ENT(pev));
    if ((dispatchanimevents & 2) != 0)
    {
        EMIT_SOUND(ENT(pev), CHAN_VOICE, "barney/ba_attack2.wav", 1, ATTN_NORM);
        UTIL_MakeVectors(pev->angles);
        FireBullets(1, pgv->v_forward, Vector(0.05, 0.05, 0.05), 1024);
        pev->effects = static_cast<int>(pev->effects) | 2;
    }
    if (m_fSequenceFinished)
    {
        SetThink(&CBaseMonster::CallMonsterThink);
        m_iActivity = m_iIdealActivity;
        eventfired = 0;
    }
}

void CBarney::Pain(float flDamage)
{
    EMIT_SOUND(ENT(pev), CHAN_VOICE, "barney/ba_pain1.wav", 1, ATTN_NORM);
    Barney_Pissed();
}

void CBarney::Die()
{
    if (pev->health > -30)
    {
        switch (rand() % 4)
        {
        case 0:
            EMIT_SOUND(ENT(pev), CHAN_VOICE, "barney/ba_die1.wav", 1, ATTN_NORM);
            break;
        case 1:
            EMIT_SOUND(ENT(pev), CHAN_VOICE, "barney/ba_die2.wav", 1, ATTN_NORM);
            break;
        case 2:
            EMIT_SOUND(ENT(pev), CHAN_VOICE, "barney/ba_die3.wav", 1, ATTN_NORM);
            break;
        case 3:
            EMIT_SOUND(ENT(pev), CHAN_VOICE, "barney/ba_die4.wav", 1, ATTN_NORM);
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
    else
    {
        m_iActivity = 36;
        pev->ideal_yaw = pev->angles.y;
        pev->nextthink = pev->pSystemGlobals->time + 0.1;
        SetThink(&CBaseMonster::CallMonsterThink);
        SetActivity(m_iActivity);
    }
}

int CBarney::CheckEnemy(entvars_t *a2, float a3)
{
    if (a3 <= 64 && function1(a2))
    {
        m_iIdealActivity = 7;
        SetThink(&CBarney::Barney_Shoot);
        return 1;
    }
    else if (CheckEnemyOnCrosshair(a2) && a3 <= 1024)
    {
        m_iIdealActivity = 7;                               // m_iactivity
        SetThink(&CBarney::Barney_Shoot);                   // shoot player
        return 1;
    }
    else
    {
        return 0;                                   // player is not in barney's crosshair
    }
    return 0;
}