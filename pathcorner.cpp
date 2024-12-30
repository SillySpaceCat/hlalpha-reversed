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
//
// ========================== PATH_CORNER ===========================
//

#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CPathCorner : public CBaseMonster //i think
{
public:
	void Spawn();
	void KeyValue(KeyValueData* pkvd);
	void Touch( entvars_t *pOther );
private:
	float m_flWait;
};

LINK_ENTITY_TO_CLASS(path_corner, CPathCorner);

void CPathCorner::Spawn()
{
	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));
}

void CPathCorner::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
}

void CPathCorner::Touch(entvars_t* pOther)
{
	entvars_t *entity = VARS(pev->pSystemGlobals->other);
	if (pev->pSystemGlobals->self != entity->goalentity && entity->enemy)
		return;
	if (m_flWait != 0)
		ALERT(at_warning, "Non-zero path-cornder waits NYI");

	if (FStringNull(pev->target))
		ALERT(at_warning, "PathCornerTouch: no next stop specified");

	entity->goalentity = OFFSET(FIND_ENTITY_BY_STRING(NULL, "targetname", STRING(pev->target)));
	if (!entity->goalentity)
	{
		ALERT(at_console, "PathCornerTouch--%s couldn't find next stop in path: %s", STRING(pev->classname), STRING(pev->target));
		return;
	}
		entity->ideal_yaw = UTIL_VecToYaw(FIND_ENTITY_BY_STRING(NULL, "targetname", STRING(pev->target))->v.origin - entity->origin);

}