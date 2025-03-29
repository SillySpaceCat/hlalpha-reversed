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

===== bmodels.cpp ========================================================

  spawn, think, and use functions for entities that use brush models

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "doors.h"
#include <cstdlib>

extern DLL_GLOBAL Vector		g_vecAttackDir;

#define		SF_BRUSH_ACCDCC	16// brush should accelerate and decelerate when toggled
#define		SF_BRUSH_HURT		32// rotating brush that inflicts pain based on rotation speed
#define		SF_ROTATING_NOT_SOLID	64	// some special rotating objects are not solid.

// covering cheesy noise1, noise2, & noise3 fields so they make more sense (for rotating fans)
#define		noiseStart		noise1
#define		noiseStop		noise2
#define		noiseRunning	noise3

#define		SF_PENDULUM_SWING		2	// spawnflag that makes a pendulum a rope swing.

extern Vector VecBModelOrigin(entvars_t* pevBModel);

// =================== FUNC_WALL ==============================================

//
// BModelOrigin - calculates origin of a bmodel from absmin/size because all bmodel origins are 0 0 0
//

/*QUAKED func_wall (0 .5 .8) ?
This is just a solid wall if not inhibited
*/
class CFuncWall : public CBaseEntity
{
public:
	void	Spawn(void);
	void	Use(entvars_t* pActivator);

	// Bmodels don't go across transitions
	//virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS(func_wall, CFuncWall);

void CFuncWall::Spawn(void)
{
	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_PUSH;  // so it doesn't get pushed by anything
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));
}


void CFuncWall::Use(entvars_t* pActivator)
{
	pev->frame = 1 - pev->frame;
}


// =================== FUNC_ILLUSIONARY ==============================================


/*QUAKED func_illusionary (0 .5 .8) ?
A simple entity that looks solid but lets you walk through it.
*/
class CFuncIllusionary : public CBaseEntity //CBaseToggle
{
public:
	void Spawn(void);
	//void EXPORT SloshTouch(CBaseEntity* pOther);
	void KeyValue(KeyValueData* pkvd);
	//virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS(func_illusionary, CFuncIllusionary);

void CFuncIllusionary::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "skin"))//skin is used for content type
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
}

void CFuncIllusionary::Spawn(void)
{
	pev->angles = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;// always solid_not 
	SET_MODEL(ENT(pev), STRING(pev->model));
	MAKE_STATIC(ENT(pev));
}


// =================== FUNC_GLASS ==============================================

class CFuncGlass : public CBaseMonster
{
public:
	void Spawn(void);
	void Touch(entvars_t* pActivator);
	void Use(entvars_t* pActivator);
	void Killed(int pevAttacker);
	int g_sprShard;
};

void CFuncGlass::Spawn(void)
{
	PRECACHE_SOUND("common/glass.wav");
	g_sprShard = PRECACHE_MODEL("sprites/shard.spr");
	if (!FBitSet(pev->spawnflags, 1))
		pev->takedamage = DAMAGE_YES;
	else
		pev->takedamage = DAMAGE_NO;
	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	SET_MODEL(ENT(pev), STRING(pev->model));
}

void CFuncGlass::Touch(entvars_t* pActivator)
{
	entvars_t* thing = VARS(pev->pSystemGlobals->other);
	if (FBitSet(thing, FL_CLIENT))
	{
		if (pev->takedamage)
			TakeDamage(thing, pev->health);
	}
}
void CFuncGlass::Use(entvars_t* pActivator)
{

	Killed(NULL);
}
void CFuncGlass::Killed(int pevAttacker)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "common/glass.wav", 1, ATTN_NORM);
	UTIL_MakeVectors(pev->angles);
	WRITE_BYTE(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(MSG_BROADCAST, TE_BREAKMODEL);
	WRITE_COORD(MSG_BROADCAST, (pev->mins.x + pev->maxs.x) * 0.5);
	WRITE_COORD(MSG_BROADCAST, (pev->mins.y + pev->maxs.y) * 0.5);
	WRITE_COORD(MSG_BROADCAST, (pev->mins.z + pev->maxs.z) * 0.5);
	WRITE_COORD(MSG_BROADCAST, pev->size.x);
	WRITE_COORD(MSG_BROADCAST, pev->size.y);
	WRITE_COORD(MSG_BROADCAST, pev->size.z);
	WRITE_COORD(MSG_BROADCAST, 1);
	WRITE_COORD(MSG_BROADCAST, 1);
	WRITE_COORD(MSG_BROADCAST, 1);
	WRITE_SHORT(MSG_BROADCAST, g_sprShard);
	WRITE_BYTE(MSG_BROADCAST, 12);
	WRITE_BYTE(MSG_BROADCAST, 15);
	WRITE_BYTE(MSG_BROADCAST, 1);
	pev->solid = SOLID_NOT;
	pev->model = 0;
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = pev->ltime;
}

// =================== FUNC_ROTATING ==============================================
class CFuncRotating : public CBaseEntity
{
public:	// basic functions
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void HurtTouch(entvars_t* pActivator);
	void Touch(entvars_t* pActivator);
	void SpinUp(void);
	void SpinDown(void);
	void Use(entvars_t* pActivator);
	void Blocked(entvars_t* pOther);

	float m_flFriction;
	float m_flAttenuation;
	float m_flVolume;
	float m_pitch;
	int	  m_sounds;
};

LINK_ENTITY_TO_CLASS(func_rotating, CFuncRotating);

void CFuncRotating::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "friction"))
	{
		m_flFriction = atof(pkvd->szValue) / 100;
		pkvd->fHandled = TRUE;
	}
}

void CFuncRotating::Spawn()
{
	switch ((int)pev->sounds)
	{
	case 1:
		PRECACHE_SOUND("fans/fan1on.wav");
		PRECACHE_SOUND("fans/fan1off.wav");
		PRECACHE_SOUND("fans/fan1.wav");
		pev->noise1 = ALLOC_STRING("fans/fan1on.wav");
		pev->noise2 = ALLOC_STRING("fans/fan1off.wav");
		pev->noise3 = ALLOC_STRING("fans/fan1.wav");
		break;
	case 2:
		PRECACHE_SOUND("fans/fan2on.wav");
		PRECACHE_SOUND("fans/fan2off.wav");
		PRECACHE_SOUND("fans/fan2.wav");
		pev->noise1 = ALLOC_STRING("fans/fan2on.wav");
		pev->noise2 = ALLOC_STRING("fans/fan2off.wav");
		pev->noise3 = ALLOC_STRING("fans/fan2.wav");
		break;
	case 3:
		PRECACHE_SOUND("fans/fan3on.wav");
		PRECACHE_SOUND("fans/fan3off.wav");
		PRECACHE_SOUND("fans/fan3.wav");
		pev->noise1 = ALLOC_STRING("fans/fan3on.wav");
		pev->noise2 = ALLOC_STRING("fans/fan3off.wav");
		pev->noise3 = ALLOC_STRING("fans/fan3.wav");
		break;
	case 4:
		PRECACHE_SOUND("fans/fan4on.wav");
		PRECACHE_SOUND("fans/fan4off.wav");
		PRECACHE_SOUND("fans/fan4.wav");
		pev->noise1 = ALLOC_STRING("fans/fan4on.wav");
		pev->noise2 = ALLOC_STRING("fans/fan4off.wav");
		pev->noise3 = ALLOC_STRING("fans/fan4.wav");
		break;
	case 5:
		PRECACHE_SOUND("fans/fan5on.wav");
		PRECACHE_SOUND("fans/fan5off.wav");
		PRECACHE_SOUND("fans/fan5.wav");
		pev->noise1 = ALLOC_STRING("fans/fan5on.wav");
		pev->noise2 = ALLOC_STRING("fans/fan5off.wav");
		pev->noise3 = ALLOC_STRING("fans/fan5.wav");
		break;
	default:
		pev->noise1 = ALLOC_STRING("common/null.wav");
		pev->noise2 = ALLOC_STRING("common/null.wav");
		pev->noise3 = ALLOC_STRING("common/null.wav");
		break;
	}
	if (m_flVolume == 0.0)
		m_flVolume = 1.0;
	Vector somevector(0, 0, 0);
	if (FBitSet(pev->spawnflags, 4))
		pev->movedir.z = 1;
	else if (FBitSet(pev->spawnflags, 8))
		pev->movedir.x = 1;
	else 
		pev->movedir.y = 1;
	if (FBitSet(pev->spawnflags, 2))
		pev->movedir = -pev->movedir;

	if (FBitSet(pev->spawnflags, 64))
	{
		pev->solid = SOLID_NOT;
		pev->skin = -1;
	}
	else
		pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));
	if (pev->speed <= 0)
		pev->speed = 0;
	if (!pev->dmg)
		pev->dmg = 2;
	if (FBitSet(pev->spawnflags, 1))
	{
		SetThink(&CBaseEntity::SUB_CallUseToggle);
		pev->nextthink = pev->pSystemGlobals->time + 0.1;
	}
	if (FBitSet(pev->spawnflags, 32))
		SetTouch(&CFuncRotating::HurtTouch);

}

void CFuncRotating::HurtTouch(entvars_t* pActivator)
{
	Touch(pActivator);
}

void CFuncRotating::Touch(entvars_t* pActivator)
{
	entvars_t* thing = VARS(pev->pSystemGlobals->other);
	if (pev->takedamage)
	{
		pev->dmg = pev->avelocity.Length() / 10;
		CBaseMonster* otherthing = GetClassPtr((CBaseMonster*)thing);
		otherthing->TakeDamage(pev, pev->dmg);

		thing->velocity = (thing->origin - VecBModelOrigin(pev)).Normalize() * pev->dmg;
	}
}

//
// SpinUp - accelerates a non-moving func_rotating up to it's speed
//
void CFuncRotating::SpinUp(void)
{
	Vector	vecAVel;//rotational velocity

	pev->nextthink = pev->ltime + 0.1;
	pev->avelocity = pev->avelocity + (pev->movedir * (pev->speed * m_flFriction));

	vecAVel = pev->avelocity;// cache entity's rotational velocity

	// if we've met or exceeded target speed, set target speed and stop thinking
	if (abs(vecAVel.x) >= abs(pev->movedir.x * pev->speed) &&
		abs(vecAVel.y) >= abs(pev->movedir.y * pev->speed) &&
		abs(vecAVel.z) >= abs(pev->movedir.z * pev->speed))
	{
		pev->avelocity = pev->movedir * pev->speed;// set speed in case we overshot
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, (char*)STRING(pev->noise3), 1, ATTN_NORM);

		pev->nextthink = pev->ltime + 99999;
	}
}

void CFuncRotating::SpinDown(void)
{
	Vector	vecAVel;//rotational velocity
	vec_t vecdir;

	pev->nextthink = pev->ltime + 0.1;

	pev->avelocity = pev->avelocity - (pev->movedir * (pev->speed * m_flFriction));//spin down slower than spinup

	vecAVel = pev->avelocity;// cache entity's rotational velocity
	if (pev->avelocity <= g_vecZero)
	{
		pev->avelocity = 0;
	}
}

void CFuncRotating::Use(entvars_t* pActivator)
{
	if (FBitSet(pev->spawnflags, SF_BRUSH_ACCDCC))
	{
		if (pev->avelocity != g_vecZero)
		{
			SetThink(&CFuncRotating::SpinDown);
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, STRING(pev->noise2), 1, ATTN_NORM);
		}
		else
		{
			SetThink(&CFuncRotating::SpinUp);
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, STRING(pev->noise1), 1, ATTN_NORM);
		}
		pev->nextthink = pev->ltime + 0.1;
	}
	else
	{
		if (pev->avelocity != g_vecZero)
		{
			pev->avelocity = g_vecZero;
		}
		else
		{
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, STRING(pev->noise3), 1, ATTN_NORM);
			pev->avelocity = pev->movedir * pev->speed;
			pev->nextthink = pev->pSystemGlobals->time + 99999;
		}
	}
}

void CFuncRotating::Blocked(entvars_t* pOther)
{
	entvars_t* thing = VARS(pev->pSystemGlobals->other);
	CBaseMonster* otherthing = GetClassPtr((CBaseMonster*)thing);
	otherthing->TakeDamage(pev, pev->dmg);
}

class CFuncPushable : public CBaseEntity
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void Touch(entvars_t* pActivator);
	float friction;
};

LINK_ENTITY_TO_CLASS(func_pushable, CFuncPushable);

void CFuncPushable::Spawn(void)
{
	pev->movetype = 13;
	pev->solid = SOLID_BBOX;
	SET_MODEL(ENT(pev), STRING(pev->model));
	if (pev->friction > 399)
		pev->friction = 399;
	friction = 400 - pev->friction;
	pev->friction = 0;
}
void CFuncPushable::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "size"))
	{
		int bbox = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

		switch (bbox)
		{
		case 0:	// Point
			UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));
			break;

		case 2: // Big Hull!?!?	!!!BUGBUG Figure out what this hull really is
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN * 2, VEC_DUCK_HULL_MAX * 2);
			break;

		case 3: // Player duck
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
			break;

		default:
		case 1: // Player
			UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);
			break;
		}

	}
}

void CFuncPushable::Touch(entvars_t* pActivator)
{
	entvars_t* thing = VARS(pev->pSystemGlobals->other);
	if (FClassnameIs(thing, "player"))
	{
		if ((!FBitSet(thing, FL_ONGROUND)) && VARS(thing->groundentity) != pev)
		{
			pev->velocity.x = thing->velocity.x * 0.5 + pev->velocity.x;
			pev->velocity.y = thing->velocity.y * 0.5 + pev->velocity.y;
		}
	}
}