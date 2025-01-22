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
#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#pragma warning(disable : 4996)
extern DLL_GLOBAL Vector		g_vecAttackDir;
extern DLL_GLOBAL float		g_flDamage;

Vector VecBModelOrigin(entvars_t* pevBModel)
{
	return pevBModel->absmin + (pevBModel->size * 0.5);
}

void DispatchSpawn( entvars_t *pev )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(ENT(pev));

	if (pEntity)
	{
		pEntity->Spawn();
	}
}


void DispatchKeyValue( entvars_t *pevKeyvalue, KeyValueData *pkvd )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(ENT(pevKeyvalue));

	if ( pEntity )
		pEntity->KeyValue( pkvd );

	if ( !pkvd || pkvd->fHandled )
		return;

	if ( !stricmp( "transitionid", pkvd->szKeyName ) )
	{
	}
}


void DispatchTouch( entvars_t *pevTouched, entvars_t *pevOther )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(ENT(pevTouched));

	if ( pEntity )
		pEntity->Touch( pevOther );
}


void DispatchUse( entvars_t *pevUsed, entvars_t *pevOther )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(ENT(pevUsed));

	if ( pEntity )
		pEntity->Use( pevOther );
}


void DispatchThink( entvars_t *pev )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(ENT(pev));

	if ( pEntity )
		pEntity->Think();
}


void DispatchBlocked( entvars_t *pevBlocked, entvars_t *pevOther )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(ENT(pevBlocked));

	if ( pEntity )
		pEntity->Blocked( pevOther );
}


void DispatchSave( entvars_t *pev, void *pSaveData )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(ENT(pev));

	if ( pEntity )
		pEntity->Save( pSaveData );
}


void DispatchRestore( entvars_t *pev, void *pSaveData )
{
	CBaseEntity *pEntity = (CBaseEntity *)GET_PRIVATE(ENT(pev));

	if ( pEntity )
		pEntity->Restore( pSaveData );
}


int CBaseEntity::Save( void *pSaveData )
{
	ALERT( at_console, "Saving %s\n", STRING(pev->classname) );
	return 0;
}


void CBaseEntity::Restore( void *pSaveData )
{
}

void CBaseMonster::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage)
{
	Vector			vecTemp;

	if (!pev->takedamage)
		return;

	if (pevAttacker == pevInflictor)
	{
		vecTemp = pevInflictor->origin - (VecBModelOrigin(pev));
	}
	else
		// an actual missile was involved.
	{
		vecTemp = pevInflictor->origin - (VecBModelOrigin(pev));
	}

	// this global is still used for glass and other non-monster killables, along with decals.
	g_vecAttackDir = vecTemp.Normalize();

	// save damage based on the target's armor level
	g_flDamage = flDamage;

	if (FBitSet(pev->flags, FL_CLIENT))
	{
		pev->dmg_take += ceil(flDamage - ceil(pev->armortype * flDamage));
		pev->dmg_save += ceil(pev->armortype * flDamage);
		pev->dmg_inflictor = OFFSET(pevInflictor);
	}
	if (pevInflictor && OFFSET(pevInflictor))
	{
		if (pev->movetype == MOVETYPE_WALK && pev->solid != SOLID_TRIGGER)
		{
			Vector v15 = pev->origin;
			float v16 = v15[2] - pev->absmax.z + pev->absmin.z * 0.5;
			float v17 = v15[1] - pev->absmax.y + pev->absmin.y * 0.5;
			Vector v38 = v15 - pev->absmin + pev->absmax * 0.5;
			float v39 = v17;
			float v40 = v16;
			Vector v18 = v38.Normalize();
			Vector v41 = v18;
			Vector v35 = Vector(0, 0, 0);
			float v36 = 0;
			float v37 = 0;
			if (v18.x <= 64 && v18.y <= 64 && v18.z <= 64)
			{
				v35 = Vector(0, 0, 0);
			}
			else
			{
				float v21 = 1.0 / v41.x;
				v35 = v38 * v21;
				v36 = v39 * v21;
				v37 = v21 * v40;
			}
			v38 = v35;
			v39 = v36;
			v40 = v37;
			Vector v22 = pev->velocity;
			v35.x = v35.x * flDamage * 8.0 + v22.x;
			v36 = v36 * flDamage * 8.0 + v22[1];
			v37 = v37 * flDamage * 8.0 + v22[2];
			pev->velocity[0] = v35[0];
			pev->velocity[1] = v36;
			pev->velocity[2] = v37;
		}
	}

	if ((!FClassnameIs(pev, "player") || (!FBitSet(pev->flags, FL_GODMODE) && !pgv->coop || pev->team <= 0 || pev->team != pevAttacker->team)))
	{
		pev->health -= ceil(flDamage - ceil(pev->armortype * flDamage));
		if (pev->health <= 0)
		{
			Killed(OFFSET(ENT(pevAttacker)));
		}
		else if (FBitSet(pev->flags, FL_MONSTER) && OFFSET(ENT(pevAttacker)) && FBitSet(pevAttacker->flags, 40))
		{
			CBaseMonster* pMonster = (CBaseMonster*)GET_PRIVATE(ENT(pevAttacker));
			if (pMonster->Classify() != Classify())
			{
				pev->goalentity = OFFSET(pevAttacker);
				pev->enemy = pev->goalentity;
				//*((_DWORD*)this + 61) = pevAttacker;
				goal_origin = g_vecAttackDir * 64 + pev->origin;
				goal_origin.x -= pev->origin.x;
				goal_origin.y -= pev->origin.y;
				goal_origin.z -= pev->origin.z;
				pev->ideal_yaw = UTIL_VecToYaw(goal_origin);
			}
			if (pev->pSystemGlobals->time > pev->pain_finished)
				Pain(flDamage);
		}

	}
}

void CBaseMonster::Killed(int pevAttacker)
{
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM);
	pev->solid = SOLID_NOT;
	TraceResult tr;
	SetThink(NULL);
	SetTouch(NULL);
	Die();
}


edict_t* EHANDLE::Get(void)
{
	if (m_pent)
	{
		if (m_pent->serialnumber == m_serialnumber)
			return m_pent;
		else
			return NULL;
	}
	return NULL;
};

edict_t* EHANDLE::Set(edict_t* pent)
{
	m_pent = pent;
	if (pent)
		m_serialnumber = m_pent->serialnumber;
	return pent;
};


EHANDLE :: operator CBaseEntity* ()
{
	return (CBaseEntity*)GET_PRIVATE(Get());
};


CBaseEntity* EHANDLE :: operator = (CBaseEntity* pEntity)
{
	if (pEntity)
	{
		m_pent = ENT(pEntity->pev);
		if (m_pent)
			m_serialnumber = m_pent->serialnumber;
	}
	else
	{
		m_pent = NULL;
		m_serialnumber = 0;
	}
	return pEntity;
}

EHANDLE :: operator int()
{
	return Get() != NULL;
}

CBaseEntity* EHANDLE :: operator -> ()
{
	return (CBaseEntity*)GET_PRIVATE(Get());
}
