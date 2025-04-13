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

===== subs.cpp ========================================================

  frequently used global functions

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "doors.h"
#include "studio.h"

// These are the new entry points to entities.


class CNullEntity : public CBaseEntity
{
public:
	void Spawn(void);
};


// Null Entity, remove on startup
void CNullEntity::Spawn(void)
{
	REMOVE_ENTITY(ENT(pev));
}


LINK_ENTITY_TO_CLASS(info_null, CNullEntity);
LINK_ENTITY_TO_CLASS(info_player_deathmatch,CBaseEntity);
LINK_ENTITY_TO_CLASS(info_player_start,CBaseEntity);
LINK_ENTITY_TO_CLASS(info_landmark, CPointEntity);

// Landmark class
void CPointEntity :: Spawn( void )
{
	pev->solid = SOLID_BBOX;
	UTIL_SetSize(pev, g_vecZero, g_vecZero);
}

void CBaseEntity::SUB_Remove( void )
{
	REMOVE_ENTITY(ENT(pev));
}

// Convenient way to explicitly do nothing (passed to functions that require a method)
void CBaseEntity :: SUB_DoNothing( void )
{
}

void CBaseDelay::SUB_UseTargets(entvars_t *pActivator)
{
	if (m_flDelay != 0)
	{
		CBaseDelay* pTemp = GetClassPtr((CBaseDelay*)NULL);
		pTemp->pev->classname = ALLOC_STRING("DelayedUse");
		pTemp->pev->nextthink = pev->pSystemGlobals->time + m_flDelay;
		pTemp->SetThink(&CBaseDelay::DelayThink);
		pTemp->m_iszKillTarget = m_iszKillTarget;
		pTemp->m_flDelay = 0; // prevent "recursion"
		pTemp->pev->target = pev->target;
	}
	else if (m_iszKillTarget)
	{
		edict_t* pentKillTarget = NULL;

		pentKillTarget = FIND_ENTITY_BY_STRING(ENT(pentKillTarget), "targetname", STRING(m_iszKillTarget));
		while (!FNullEnt(pentKillTarget))
		{
			REMOVE_ENTITY(ENT(pentKillTarget));
		}
	}
	else if (!FStringNull(pev->target))
	{
		globalvars_t *v9 = pgv;
		int v10 = 0;
		int v11 = v9->self;
		edict_t* pentTarget = NULL;
		edict_t* nextTarget = NULL;
		for (;;)
		{
			pentTarget = FIND_ENTITY_BY_STRING(nextTarget, "targetname", STRING(pev->target));
			nextTarget = pentTarget;
			if (FNullEnt(pentTarget))
				break;
			CBaseEntity* pTarget = CBaseEntity::Instance(pentTarget);
			if (pTarget)
				pTarget->Use(pActivator);
		}
	}
}

void CBaseDelay::DelayThink(void)
{
	//SUB_UseTargets(pActivator, 0);
	SUB_UseTargets(pev);
	REMOVE_ENTITY(ENT(pev));
}

void CBaseToggle::LinearMoveDone(void)
{
	Vector delta = m_vecFinalDest - pev->origin;
	float error = delta.Length();
	if (error > 0.03125)
	{
		LinearMove(m_vecFinalDest, 100);
		return;
	}

	UTIL_SetOrigin(pev, m_vecFinalDest);
	pev->velocity = g_vecZero;
	pev->nextthink = -1;
	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}

void CBaseToggle::LinearMove(Vector	vecDest, float flSpeed)
{
	m_vecFinalDest = vecDest;

	// Already there?
	if (vecDest == pev->origin)
	{
		LinearMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDest - pev->origin;

	// divide vector length by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to LinearMoveDone when dest is reached
	pev->nextthink = pev->ltime + flTravelTime;
	SetThink(&CBaseToggle::LinearMoveDone);

	// scale the destdelta vector by the time spent traveling to get velocity
	pev->velocity = vecDestDelta / flTravelTime;
}

/*
=============
AngularMove

calculate pev->velocity and pev->nextthink to reach vecDest from
pev->origin traveling at flSpeed
Just like LinearMove, but rotational.
===============
*/
void CBaseToggle::AngularMove(Vector vecDestAngle, float flSpeed)
{

	m_vecFinalAngle = vecDestAngle;

	// Already there?
	if (vecDestAngle == pev->angles)
	{
		AngularMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDestAngle - pev->angles;

	// divide by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to AngularMoveDone when dest is reached
	pev->nextthink = pev->ltime + flTravelTime;
	SetThink(&CBaseToggle::AngularMoveDone);

	// scale the destdelta vector by the time spent traveling to get velocity
	pev->avelocity = vecDestDelta / flTravelTime;
}


/*
============
After rotating, set angle to exact final angle, call "move done" function
============
*/
void CBaseToggle::AngularMoveDone(void)
{
	pev->angles = m_vecFinalAngle;
	pev->avelocity = g_vecZero;
	pev->nextthink = -1;
	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}

void CBaseToggle::AxisDir(entvars_t* pev)
{
	if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_Z))
		pev->movedir = Vector(0, 0, 1);	// around z-axis
	else if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_X))
		pev->movedir = Vector(1, 0, 0);	// around x-axis
	else
		pev->movedir = Vector(0, 1, 0);		// around y-axis
}


float CBaseToggle::AxisDelta(int flags, const Vector& angle1, const Vector& angle2)
{
	if (FBitSet(flags, SF_DOOR_ROTATE_Z))
		return angle1.z - angle2.z;

	if (FBitSet(flags, SF_DOOR_ROTATE_X))
		return angle1.x - angle2.x;

	return angle1.y - angle2.y;
}

//                 //
//    MONSTER AI   //
//				   // 

BOOL function3(entvars_t* pev, entvars_t* enemy, float a3)
{
	Vector v10;
	float v12, v13, v14, length, length2;

	UTIL_MakeVectors(pev->angles);

	if (!enemy) //avoid crashes, ai code isnt very complete
		return 0;

	v10.x = pev->origin.x - enemy->origin.x;
	v10.y = pev->origin.y - enemy->origin.y;
	v10.z = pev->origin.z - enemy->origin.z;
	length = v10.Length();
	if (length < 64)
	{
		v12 = 0;
		v13 = 0;
		v14 = 0;
	}
	else
	{
		v12 = (pev->origin.x - enemy->origin.x) / length;
		v13 = (pev->origin.y - enemy->origin.y) / length;
		v14 = (pev->origin.z - enemy->origin.z) / length;
	}

	length2 = pev->pSystemGlobals->v_forward[0] * v12 + pev->pSystemGlobals->v_forward[1] * v13 + pev->pSystemGlobals->v_forward[2] * v14;
	if (-length2 > a3)
		return 1;
	else
		return 0;
}

BOOL function2(entvars_t* pev, entvars_t* enemy, float a3)
{
	return function3(pev, enemy, a3);
}

BOOL CanSeePlayer(entvars_t* pev, entvars_t* enemy)
{
	Vector viewofs_pev, viewofs_enemy;
	TraceResult tr;
	viewofs_pev = pev->view_ofs + pev->origin;
	viewofs_enemy = enemy->view_ofs + enemy->origin;

	UTIL_TraceLine(viewofs_pev, viewofs_enemy, 0, ENT(pev), &tr);

	return (pev->pSystemGlobals->trace_fraction == 1.0);
}

BOOL CBaseMonster::function1(entvars_t* a2)
{
	if (a2 == NULL)
		return 0;
	if (pgv->time < nextattack)
		return 0;
	if (!function2(pev, a2, 0.1))
		return 0;
	if (CanSeePlayer(pev, a2))
		return (fabs(a2->origin.z - pev->origin.z) <= 48.0);
	return 0;
}

BOOL CBaseMonster::CheckEnemyOnCrosshair(entvars_t* a2)
{
	if (pgv->time < nextattack)
		return 0;
	if (function2(pev, a2, 0.9))
		return (CanSeePlayer(pev, a2));
}

float CBaseMonster::GetDistance(entvars_t* enemy)
{
	float length;
	Vector origin;
	origin = enemy->origin - pev->origin;

	byte1 = byte1 & 253;
	byte1 = (byte1 & 253) & 254;

	if (CanSeePlayer(pev, enemy))
		byte1 |= 2;
	if (function2(pev, enemy, 0.1))
		byte1 |= 1;

	length = sqrt(origin.x * origin.x + origin.y * origin.y + origin.z * origin.z);

	if ((byte1 & 2) != 0 && (byte1 & 1) != 0 || (byte1 & 2) != 0 && length < 256)
	{
		byte3 = 0;
		byte2 = 0;
		byte1 |= 4;
		enemyposition = enemy->origin;
	}
	pev->ideal_yaw = UTIL_VecToYaw(enemy->origin - pev->origin);
	return length;

}

void CBaseMonster::MonsterInit()
{
	WalkMonsterStart();
}

void CBaseMonster::WalkMonsterStart()
{
	if (pev->pSystemGlobals->deathmatch)
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}
	pev->origin.z += 1;

	DROP_TO_FLOOR(ENT(OFFSET(pev))); //for some reason some monsters dissappear when this                        
							//line executes ?????????
	if (!WALK_MOVE(ENT(pev), 0, 0))
	{
		ALERT(at_warning, "Monster %s stuck in wall--level design error", STRING(pev->classname));
		pev->effects = 1;
	}
	pev->view_ofs.x = 0;
	pev->view_ofs.y = 0;
	pev->view_ofs.z = 64;
	pev->takedamage = DAMAGE_AIM;
	pev->ideal_yaw = pev->angles.y;
	SetBits(pev->flags, FL_MONSTER);
	SetThink(&CBaseMonster::CallMonsterThink);
	m_iActivity = ACT_IDLE1;
	if (pev->target)
	{
		pev->goalentity = OFFSET(FIND_ENTITY_BY_STRING(NULL, "targetname", STRING(pev->target)));
		if (pev->goalentity)
		{
			entvars_t *goal = VARS(pev->goalentity);
			Vector goaltest;
			goaltest.x = goal->origin.x - pev->origin.x;
			goaltest.y = goal->origin.y - pev->origin.y;
			goaltest.z = goal->origin.z - pev->origin.z;
			pev->ideal_yaw = UTIL_VecToYaw(goaltest);
			if (!FClassnameIs(VARS(pev->goalentity), "path_corner"))
				ALERT(at_aiconsole, "WalkMonsterStart--monster's initial goal '%s' is not a path_corner", STRING(pev->target));
			m_iActivity = ACT_WALK;
		}
		else
		{
			ALERT(at_warning, "WalkMonsterStart--%s couldn't find target %s", STRING(pev->classname), STRING(pev->target));
		}
	}
		pev->nextthink += UTIL_RandomFloat(0.0, 0.5) + 0.5;
}

void CBaseMonster::CallMonsterThink()
{
	entvars_t *enemy = NULL;
	float distance = NULL;
	Vector length = Vector(0, 0, 0);
	edict_t* player = FIND_CLIENT_IN_PVS();
	pev->nextthink = pgv->time + 0.1;
	SetActivity(m_iActivity);
	StudioFrameAdvance(0.1);
	//ResetSequenceInfo(0.1); // test
	if (pev->enemy)
	{
		if (pev->health > 0)
		{
			enemy = VARS(pev->enemy);
			if (VARS(pev->enemy)->health <= 0)
			{
				pev->enemy = NULL;
				m_iActivity = 1;
				return;
			}
			distance = GetDistance(enemy);
		}
	}
    else
	{
		if (Classify() == 5)
		{
			if (CanSeePlayer(pev, VARS(player)))
			{
				if (function2(pev, VARS(player), 0.1))
					return;
			}
		}
	}

	switch (m_iActivity)
	{
	case 1:
	case 2:
	case 3:
		if (pgv->time > nextidle)
			Idle();
		if (m_fSequenceFinished)
		{
			int random = rand() % 10;
			if (random)
			{
				if (random == 1)
					m_iActivity = 3;
				else
					m_iActivity = 1;
			}
			else
				m_iActivity = 2;
		}
		break;
	case 4:
		UTIL_MoveToOrigin(ENT(pev), VARS(pev->goalentity)->origin, m_flGroundSpeed, 1);
		//  the line above causes the screen to blink and monsters dissappear
		//  and a bunch of other funky stuff i literally
		//  do not know why
		//ALERT(at_console, "%f, %f,%f\n", VARS(pev->goalentity)->origin.x, VARS(pev->goalentity)->origin.y, VARS(pev->goalentity)->origin.z);
		break;
	case 5:
		CHANGE_YAW(ENT(pev));
		CheckEnemy(enemy, distance);
		break;
	case 6:
		CHANGE_YAW(ENT(pev));
		if (!CheckEnemy(enemy, distance))
		{
			if ((byte1 & 2) != 0 && (byte1 & 1) != 0)
				m_iActivity = ACT_AIM2;
		}
	case 7:
		CHANGE_YAW(ENT(pev));
		if (!CheckEnemy(enemy, distance) 
			&& m_sightDistance < distance 
			&& (byte1 & 2) != 0 
			&& WALK_MOVE(ENT(pev), pev->ideal_yaw, 15))
		{
			m_iActivity = ACT_RUN;
		}
		break;
	case 10:
		GetDistance(followentity);
		length = pev->origin - followentity->origin;
		if (length.Length() <= m_followDistance)
			CHANGE_YAW(ENT(pev));
		else
		{
			UTIL_MoveToOrigin(ENT(pev), followentity->origin, m_flGroundSpeed, 1);
		}
		break;
	case 27:
		break;
	case 29:
	case 30:
		CHANGE_YAW(ENT(pev));
		break;
	case 31:
		if (!CheckEnemy(enemy, distance))
			m_iActivity = 6;
		break;
	case 35:
	case 36:
	case 37:
	case 38:
		CHANGE_YAW(ENT(pev));
		if (m_fSequenceFinished)
		{
			pev->framerate = 0;
			pev->solid = SOLID_NOT;
			m_iActivity = 42;
			SetThink(NULL);
		}
		break;
	case 42:
		SetThink(NULL);
		break;
	}
		if (Classify() != 5 && !pev->enemy)
		{
			if (FClassnameIs(player, "player"))
			{
				if (CanSeePlayer(pev, VARS(player)) && function2(pev, VARS(player), 0.1))
				{
					if (!FBitSet(pev->spawnflags, 1) || function2(pev, VARS(player), 0.7))
					{
						pev->enemy = OFFSET(player);
						pev->goalentity = pev->enemy;
						enemyposition = player->v.origin;
						Alert();
					}
				}
			}
		}
}

void GetSequenceInfo(void* pmodel, entvars_t* pev, float* pflFrameRate, float* pflGroundSpeed)
{
	studiohdr_t* pstudiohdr;

	pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr)
		return;

	mstudioseqdesc_t* pseqdesc;

	if (pev->sequence >= pstudiohdr->numseq)
	{
		*pflFrameRate = 0.0;
		*pflGroundSpeed = 0.0;
		return;
	}

	pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	if (pseqdesc->numframes > 1)
	{
		*pflFrameRate = 256 * pseqdesc->fps / (pseqdesc->numframes - 1);
		*pflGroundSpeed = sqrt(pseqdesc->linearmovement[0] * pseqdesc->linearmovement[0] + pseqdesc->linearmovement[1] * pseqdesc->linearmovement[1] + pseqdesc->linearmovement[2] * pseqdesc->linearmovement[2]);
		*pflGroundSpeed = *pflGroundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);
	}
	else
	{
		*pflFrameRate = 256.0;
		*pflGroundSpeed = 0.0;
	}
}

void CBaseAnimating::StudioFrameAdvance(float flInterval)
{
	if (pev->animtime)
		pev->frame = (pgv->time - pev->animtime) * pev->framerate * m_flFrameRate + pev->frame;
	pev->animtime = pgv->time;
	if (pev->frame < 0.0 || pev->frame >= 256.0)
		pev->frame += (pev->frame / 256.0) * -256.0;
	float flEnd = pev->frame + flInterval * m_flFrameRate * pev->framerate;

	m_fSequenceFinished = FALSE;
	if (flEnd >= 256 || flEnd <= 0.0)
		m_fSequenceFinished = TRUE;
}

void CBaseAnimating::ResetSequenceInfo(float flInterval)
{
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	GetSequenceInfo(pmodel, pev, &m_flFrameRate, &m_flGroundSpeed);
	pev->animtime = pgv->time;
	pev->framerate = 1;
	m_fSequenceFinished = FALSE;
	m_flGroundSpeed *= flInterval;
}

byte CBaseAnimating::HandleAnimEvent(void *phdr, float flInterval)
{
	studiohdr_t *pstudiohdr = (studiohdr_t*)phdr;
	float flStart;

	mstudioseqdesc_t *seqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;
	mstudioevent_t* event = (mstudioevent_t*)((byte*)pstudiohdr + seqdesc->eventindex);
	byte eventnum = 0;
	if ((!phdr) || (pstudiohdr->numseq <= pev->sequence) || (!seqdesc->numevents))
		return 0;
	flStart = (float)seqdesc->numframes / 256.0 * pev->frame;
	for (int i = 0; i < seqdesc->numevents; i++)
	{
		if (event->frame >= flStart)
		{
			float flCurrentFrame = seqdesc->fps * flInterval + flStart;
			if ((flCurrentFrame > event->frame && !eventfired))
			{
				eventnum |= 1 << event->type;
				eventfired = 1;
			}
		}
		event++;
	}
	return eventnum;
}

byte CBaseAnimating::DispatchAnimEvents(float flInterval)
{
	return HandleAnimEvent(GET_MODEL_PTR(ENT(pev)), flInterval);
}

float BoneControllerInternal(void* phdr, entvars_t *pev, int nCtrlId, float flValue)
{
	float var1;
	studiohdr_t* pstudiohdr = (studiohdr_t *)phdr;
	if ((!pstudiohdr) || (nCtrlId >= pstudiohdr->numbonecontrollers))
		return 0;
	mstudiobonecontroller_t* bonecontroller = (mstudiobonecontroller_t*)((byte*)pstudiohdr + pstudiohdr->bonecontrollerindex) + nCtrlId;

	if ((bonecontroller->type & 56 != 0) &&
		(bonecontroller->start + 359.0) >= bonecontroller->end &&
		(bonecontroller->start + bonecontroller->end) / 2 + 180.0 < flValue)
		flValue -= 360.0;

	var1 = (flValue - bonecontroller->start) / (bonecontroller->end - bonecontroller->start) * 255.0;
	if (var1 < 0)
		var1 = 0;
	if (var1 > 255)
		var1 = 255;

	pev->controller = var1;
	return var1;
}

float CBaseAnimating::BoneController(int nCtrlId, float flInterval)
{
	return BoneControllerInternal(GET_MODEL_PTR(ENT(pev)), pev, nCtrlId, flInterval);
}