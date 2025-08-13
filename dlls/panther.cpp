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

===== panther.cpp ========================================================

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

enum panther_anims
{
    pan_idle = 0,
    pan_walk2,
    pan_attack,
    pan_flinch
};

// Animation event IDs
enum
{
    AE_PANTHER_CLAW = 1,
    AE_PANTHER_BITE,
    AE_PANTHER_LEAP
};

class CPanther : public CBaseMonster
{
public:
    virtual void Spawn();
    virtual void SetActivity(int activity);
    virtual int Classify() { return 8; } // mirror alpha style (see headcrab)
    virtual void Alert();
    virtual void Pain(float flDamage);
    virtual void Die();
    virtual void Idle();
    virtual int CheckEnemy(entvars_t *a2, float a3);

    void Leap();
    void LeapTouch(CBaseEntity *pOther);
    virtual void HandleAnimEvent(MonsterEvent_t *pEvent);

private:
    float m_flNextLeap;
};

LINK_ENTITY_TO_CLASS(monster_panther, CPanther);

// ---------------------------------------------------------------------
// Spawn/Precache
// ---------------------------------------------------------------------
void CPanther::Spawn()
{
    PRECACHE_SOUND("panthereye/pa_idle1.wav");
    PRECACHE_SOUND("panthereye/pa_idle2.wav");
    PRECACHE_SOUND("panthereye/pa_idle3.wav");
    PRECACHE_SOUND("panthereye/pa_idle4.wav");

    PRECACHE_MODEL("models/panthereye.mdl");
    SET_MODEL(ENT(pev), "models/panthereye.mdl");

    // Bigger than headcrab; use medium hull like other quadrupeds
    UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 64));

    pev->solid     = SOLID_SLIDEBOX;
    pev->movetype  = MOVETYPE_STEP;
    pev->effects   = 0;
    pev->health    = 80;
    pev->yaw_speed = 10;

    // Start idle sequence
    pev->sequence  = pan_idle;

    m_bloodColor   = 54;

    m_sightDistance = 768; // a bit farther than headcrab
    pev->nextthink += UTIL_RandomFloat(0.0, 0.5) + 0.5;
    SetThink(&CPanther::MonsterInit);

    // Leap cooldown baseline
    m_flNextLeap = 0;
}

// ---------------------------------------------------------------------
// Activity mapping
// ---------------------------------------------------------------------
void CPanther::SetActivity(int activity)
{
    int activitynum = pan_idle;

    switch (activity)
    {
        case 1:  activitynum = pan_idle;   break; // ACT_IDLE
        case 2:  activitynum = pan_idle;   break;
        case 3:  activitynum = pan_idle;   break;

        case 4:  activitynum = pan_walk2;  break; // ACT_WALK
        case 8:  activitynum = pan_walk2;  break; // ACT_RUN
        case 9:  activitynum = pan_walk2;  break;

        case 18: activitynum = pan_flinch; break; // ACT_SMALL_FLINCH
        case 33: activitynum = pan_flinch; break; // ACT_BIG_FLINCH

        // Use same attack anim for melee and range (leap) — physics sells the leap.
        case 30: activitynum = pan_attack; break; // ACT_RANGE_ATTACK1 (mapped like headcrab jump)
        case 43: activitynum = pan_attack; break; // ACT_MELEE_ATTACK1 (if used)
        case 44: activitynum = pan_attack; break; // ACT_MELEE_ATTACK2 (if used)

        case 31: return;

        default:
            ALERT(at_console, "Panther's monster state is bogus: %d", activity);
            break;
    }

    if (pev->sequence != activitynum)
    {
        pev->sequence = activitynum;
        pev->frame = 0;
        // Alpha DLLs often used a small rate scalar in ResetSequenceInfo
        ResetSequenceInfo(0.1);
    }
}

// ---------------------------------------------------------------------
// Basic reactions
// ---------------------------------------------------------------------
void CPanther::Alert()
{
    m_iActivity = 31;
    nextattack = pev->pSystemGlobals->time + 0.8f;
}

void CPanther::Pain(float flDamage)
{
    if (flDamage < 10)
    {
        SetThink(&CPanther::CallMonsterThink);
        if (pev->enemy) m_iActivity = 18; // flinch
    }
}

void CPanther::Die()
{
    SUB_Remove();
}

void CPanther::Idle()
{
    float sound = UTIL_RandomFloat(0.0, 1.0);
    if (sound <= 0.25)
        EMIT_SOUND(ENT(pev), CHAN_VOICE, "panthereye/pa_idle1.wav", 1, ATTN_IDLE);
    else if (sound <= 0.5)
        EMIT_SOUND(ENT(pev), CHAN_VOICE, "panthereye/pa_idle2.wav", 1, ATTN_IDLE);
    else if (sound <= 0.75)
        EMIT_SOUND(ENT(pev), CHAN_VOICE, "panthereye/pa_idle3.wav", 1, ATTN_IDLE);
    else
        EMIT_SOUND(ENT(pev), CHAN_VOICE, "panthereye/pa_idle4.wav", 1, ATTN_IDLE);

    nextidle = pev->pSystemGlobals->time + UTIL_RandomFloat(0, 2) + 3;
}

// ---------------------------------------------------------------------
// Enemy checks (alpha NPCs push actions via SetThink on simple gates)
// ---------------------------------------------------------------------
int CPanther::CheckEnemy(entvars_t *a2, float a3)
{
    // If enemy on crosshair and within mid-range, commit to a leap.
    if (CheckEnemyOnCrosshair(a2) && a3 <= 768)
    {
        m_iIdealActivity = 7; // keep movement pressure (run/advance)
        SetThink(&CPanther::Leap);
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------
// Leap (Alpha style: set anim, push velocity, optional Touch damage)
// ---------------------------------------------------------------------
void CPanther::Leap()
{
    pev->nextthink = pgv->time + 0.1;

    if (m_iActivity != 30) // use same code used in SetActivity switch
    {
        entvars_t* enemy = VARS(pev->enemy);
        m_iActivity = 30; // mapped to pan_attack
        SetActivity(30);

        // No dedicated attack/roar sound in 0.52 — stay silent.

        UTIL_MakeVectors(pev->angles);

        Vector origin = pev->origin;
        origin.z += 1.0;
        UTIL_SetOrigin(pev, origin);

        // Direction toward enemy eye position; normalize and add forward bias
        Vector toEnemy = pev->origin - (enemy->origin + enemy->view_ofs);
        float len = toEnemy.Length();

        Vector dir;
        if (len < 64)
        {
            dir = g_vecZero;
        }
        else
        {
            dir.x = toEnemy.x * (1.0 / len);
            dir.y = toEnemy.y * (1.0 / len);
            dir.z = toEnemy.z * (1.0 / len);
        }

        Vector v_forward = (pgv->v_forward + dir) * 350.0; // faster/heavier than headcrab
        v_forward.z += 140.0; // small upward bias for arc

        pev->velocity = v_forward;

        // Apply damage on contact for a short window
        SetTouch(&CPanther::LeapTouch);
        m_flNextLeap = pgv->time + 2.0; // cooldown mirrors alpha-style gating
    }
}

void CPanther::LeapTouch(CBaseEntity *pOther)
{
    if (!pOther || !pOther->pev || !pOther->pev->takedamage)
        return;

    pOther->TakeDamage(pev, pev, 20, DMG_CLUB);
    SetTouch(NULL); // one-hit window
}

// ---------------------------------------------------------------------
// Anim events
// ---------------------------------------------------------------------
void CPanther::HandleAnimEvent(MonsterEvent_t *pEvent)
{
    switch (pEvent->event)
    {
        case AE_PANTHER_CLAW:
        {
            CBaseEntity *pHurt = m_hEnemy;
            if (pHurt && (pev->origin - pHurt->pev->origin).Length() <= 64)
                pHurt->TakeDamage(pev, pev, 15, DMG_SLASH);
            break;
        }
        case AE_PANTHER_BITE:
        {
            CBaseEntity *pHurt = m_hEnemy;
            if (pHurt && (pev->origin - pHurt->pev->origin).Length() <= 48)
                pHurt->TakeDamage(pev, pev, 25, DMG_BULLET);
            break;
        }
        case AE_PANTHER_LEAP:
        {
            // Some QCs raise a leap event; mirror our Leap() push if present
            if (!m_hEnemy) break;
            UTIL_MakeVectors(pev->angles);
            Vector toEnemy = pev->origin - (m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs);
            float len = toEnemy.Length();
            Vector dir = (len > 1) ? toEnemy * (1.0 / len) : g_vecZero;
            Vector v_forward = (pgv->v_forward + dir) * 350.0; v_forward.z += 140.0;
            pev->velocity = v_forward;
            SetTouch(&CPanther::LeapTouch);
            m_flNextLeap = pgv->time + 2.0;
            break;
        }
        default:
            CBaseMonster::HandleAnimEvent(pEvent);
            break;
    }
}
