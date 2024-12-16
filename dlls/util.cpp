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

===== util.cpp ========================================================

  Utility code.  Really not optional after all.

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"

void UTIL_EmitAmbientSound( Vector vecOrigin, const char *samp, float vol, float attenuation )
{
	float rgfl[3];
	vecOrigin.CopyToArray(rgfl);

	EMIT_AMBIENT_SOUND(rgfl, samp, vol, attenuation);
}


void UTIL_SetSize( entvars_t *pev, Vector vecMin, Vector vecMax )
{
	SET_SIZE( ENT(pev), vecMin, vecMax );
}


void UTIL_SetOrigin( entvars_t *pev, Vector vecOrigin )
{
	SET_ORIGIN(ENT(pev), vecOrigin );
}

void UTIL_MakeVectors( const Vector& vecAngles )
{
	MAKE_VECTORS( vecAngles );
}

void UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, edict_t* pentIgnore, TraceResult* ptr)
{
	TRACE_LINE(vecStart, vecEnd, 0, pentIgnore, ptr);
}
Vector UTIL_GetAimVector(edict_t* pent, float flSpeed)
{
	Vector tmp;
	GET_AIM_VECTOR(pent, flSpeed, tmp);
	return tmp;
}
int UTIL_FindEntityInSphere(Vector vec1, float radius)
{
	return FIND_ENTITY_IN_SPHERE(vec1, radius);
}

void SetMovedir(entvars_t* pev)
{
	if (pev->angles == Vector(0, -1, 0))
	{
		pev->movedir = Vector(0, 0, 1);
	}
	else if (pev->angles == Vector(0, -2, 0))
	{
		pev->movedir = Vector(0, 0, -1);
	}
	else
	{
		UTIL_MakeVectors(pev->angles);
		pev->movedir = pev->pSystemGlobals->v_forward;
	}

	pev->angles = g_vecZero;
}