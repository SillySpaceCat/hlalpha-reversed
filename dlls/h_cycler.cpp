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

===== h_cycler.cpp ========================================================

  The Halflife Cycler Monsters

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"


#define TEMP_FOR_SCREEN_SHOTS
#ifdef TEMP_FOR_SCREEN_SHOTS //===================================================

class CCycler : public CBaseEntity //CBaseMonster
{
public:
	void CyclerSpawn(char* szModel, Vector vecMin, Vector vecMax);
	//virtual int	ObjectCaps(void) { return (CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE); }
	void Pain(float flDamage);
	//void Pain( float flDamage );
	void Use(entvars_t* pActivator);

	int			m_animate;
};


//
// all cyclers down below
//

class CCyclerScientist : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerHeadCrab : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerPanther : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerHoundEye : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerSecure : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerBullChicken : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerDoctor : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerRedDoctor : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerGreenDoctor : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerBlueDoctor : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerTurret : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerHumanAssault : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerHumanGrunt : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerDesert : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerOlive : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerAlienGrunt : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerAlienSlave : public CCycler
{
public:
	void Spawn(void);
};
class CCyclerProbe : public CCycler
{
public:
	void Spawn(void);
};
void CCyclerScientist::Spawn(void)
{
	CyclerSpawn("models/scientist.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerHeadCrab::Spawn(void)
{
	CyclerSpawn("models/headcrab.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerPanther::Spawn(void)
{
	CyclerSpawn("models/panther.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerHoundEye::Spawn(void)
{
	CyclerSpawn("models/houndeye.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerSecure::Spawn(void)
{
	CyclerSpawn("models/barney.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerBullChicken::Spawn(void)
{
	CyclerSpawn("models/bullchik.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerDoctor::Spawn(void)
{
	CyclerSpawn("models/doctor.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerRedDoctor::Spawn(void)
{
	CyclerSpawn("models/reddoc.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerGreenDoctor::Spawn(void)
{
	CyclerSpawn("models/greendoc.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerBlueDoctor::Spawn(void)
{
	CyclerSpawn("models/bluedoc.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerTurret::Spawn(void)
{
	CyclerSpawn("models/turret.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerHumanAssault::Spawn(void)
{
	CyclerSpawn("models/hassault.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerHumanGrunt::Spawn(void)
{
	CyclerSpawn("models/hgrunt.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerDesert::Spawn(void)
{
	CyclerSpawn("models/desert.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerOlive::Spawn(void)
{
	CyclerSpawn("models/olive.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerAlienGrunt::Spawn(void)
{
	CyclerSpawn("models/agrunt.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerAlienSlave::Spawn(void)
{
	CyclerSpawn("models/islave.mdl", Vector(-16, -16, 0), Vector(16, 16, 64));
}
void CCyclerProbe::Spawn(void)
{
	pev->origin = pev->origin + Vector(0, 0, 16);
	CyclerSpawn("models/prdroid.mdl", Vector(-16, -16, -16), Vector(16, 16, 16));
}

LINK_ENTITY_TO_CLASS(cycler_scientist, CCyclerScientist);
LINK_ENTITY_TO_CLASS(cycler_headcrab, CCyclerHeadCrab);
LINK_ENTITY_TO_CLASS(cycler_panther, CCyclerPanther);
LINK_ENTITY_TO_CLASS(cycler_houndeye, CCyclerHoundEye);
LINK_ENTITY_TO_CLASS(cycler_secure, CCyclerSecure);
LINK_ENTITY_TO_CLASS(cycler_bullchicken, CCyclerBullChicken);
LINK_ENTITY_TO_CLASS(cycler_doctor, CCyclerDoctor);
LINK_ENTITY_TO_CLASS(cycler_reddoc, CCyclerRedDoctor);
LINK_ENTITY_TO_CLASS(cycler_greendoc, CCyclerGreenDoctor);
LINK_ENTITY_TO_CLASS(cycler_bluedoc, CCyclerBlueDoctor);
LINK_ENTITY_TO_CLASS(cycler_human_assault, CCyclerHumanAssault);
LINK_ENTITY_TO_CLASS(cycler_human_grunt, CCyclerHumanGrunt);
LINK_ENTITY_TO_CLASS(cycler_desert, CCyclerDesert);
LINK_ENTITY_TO_CLASS(cycler_olive, CCyclerOlive);
LINK_ENTITY_TO_CLASS(cycler_alien_grunt, CCyclerAlienGrunt);
LINK_ENTITY_TO_CLASS(cycler_alien_slave, CCyclerAlienSlave);
LINK_ENTITY_TO_CLASS(cycler_prdroid, CCyclerProbe);
LINK_ENTITY_TO_CLASS(cycler_turret, CCyclerTurret);




// Cycler member functions

void CCycler::CyclerSpawn(char* szModel, Vector vecMin, Vector vecMax)
{
	PRECACHE_MODEL(szModel);
	SET_MODEL(ENT(pev), szModel);
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_NONE;
	pev->takedamage = DAMAGE_AIM;
	pev->effects = 0;
	pev->health = 80000;// no cycler should die
	pev->yaw_speed = 5;
	UTIL_SetSize(pev, vecMin, vecMax);
	//m_flFrameRate = 75;
	//m_flGroundSpeed = 0;
	pev->classname = ALLOC_STRING("cycler");
	pev->sequence = 0;
	pev->frame = 0;
	//ResetSequenceInfo(0.1);
	//SetUse(SUB_CallUseToggle);

	pev->nextthink += 1.0;

}
//
// CyclerUse - starts a rotation trend
//
void CCycler::Use(entvars_t* pActivator)
{
	pev->nextthink = pev->pSystemGlobals->time + 0.1;
	//if (pev->pSystemGlobals->time < somevalue)
	//	pev->angles.x += 4;
	//StudioFrameAdvance(0.1);
}

//
// CyclerPain , changes sequences when shot
//
void CCycler::Pain(float flDamage)
{
	pev->health += flDamage;
	++pev->sequence;
	//ResetSequenceInfo(0.1);
	//if (((_DWORD)this[66] & 0x7FFFFFFF) == 0)
	//{
	//	pev->sequence = 0;
	//	ResetSequenceInfo(0.1);
	//}
	pev->frame = 0;
}

#endif