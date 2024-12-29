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

// C functions for external declarations that call the appropriate C++ methods

extern void DispatchSpawn( entvars_t *pev );
extern void DispatchKeyValue( entvars_t *pevKeyvalue, KeyValueData *pkvd );
extern void DispatchTouch( entvars_t *pevTouched, entvars_t *pevOther );
extern void DispatchUse( entvars_t *pevUsed, entvars_t *pevOther );
extern void DispatchThink( entvars_t *pev );
extern void DispatchBlocked( entvars_t *pevBlocked, entvars_t *pevOther );
extern void DispatchSave( entvars_t *pev, void *pSaveData );
extern void DispatchRestore( entvars_t *pev, void *pSaveData );

typedef enum { USE_OFF = 0, USE_ON = 1, USE_SET = 2, USE_TOGGLE = 3 } USE_TYPE;

#ifdef _WIN32
#define EXPORT _declspec( dllexport )
#else
#define EXPORT __attribute__ ((visibility("default")))
#endif


//
// Base Entity. All entity types derive from this
//
class CBaseEntity
{
public:
	entvars_t *pev;		// Don't need to save/restore this pointer, the engine resets it
	globalvars_t *pgv;

	virtual void	Spawn( void ) { return; }
	virtual void	KeyValue( KeyValueData *pkvd ) { pkvd->fHandled = FALSE; }
	virtual void	Unknown() { return; }
	virtual int		Save( void *pSaveData );
	virtual void	Restore( void *pSaveData );
	virtual void	TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage) {return;}; // just so the game doesn't crash
	// fundamental callbacks
	void (CBaseEntity ::*m_pfnThink)( void );
	void (CBaseEntity ::*m_pfnTouch)( entvars_t *pevOther );
	void (CBaseEntity ::*m_pfnUse)( entvars_t *pevOther );
	void (CBaseEntity ::*m_pfnBlocked)( entvars_t *pevOther );
	void (CBaseEntity ::*m_pfnCallWhenMoveDone)(void);

	virtual void Think( void ) { if (m_pfnThink) (this->*m_pfnThink)(); };
	virtual void Touch( entvars_t *pevOther ) { if (m_pfnTouch) (this->*m_pfnTouch)( pevOther ); };
	virtual void Use( entvars_t *pevOther ) { if (m_pfnUse) (this->*m_pfnUse)( pevOther ); };
	virtual void Blocked( entvars_t *pevOther ) { if (m_pfnBlocked) (this->*m_pfnBlocked)( pevOther ); };

	// allow engine to allocate instance data
    void *operator new( size_t stAllocateBlock, entvars_t *pev )
	{
		return (void *)ALLOC_PRIVATE(ENT(pev), stAllocateBlock);
	};

	static CBaseEntity* Instance(edict_t* pent)
	{
		if (!pent)
			pent = ENT(0);
		CBaseEntity* pEnt = (CBaseEntity*)GET_PRIVATE(pent);
		return pEnt;
	}

	// common member functions
	void SUB_Remove(void);
	void SUB_DoNothing( void );
	void SUB_WrapperCallUseToggle(entvars_t* unused) { SUB_CallUseToggle(); }; //ugly hack aa
	void SUB_CallUseToggle(void) { this->Use(this->pev); };

};

//
// EHANDLE. Safe way to point to CBaseEntities who may die between frames
//
class EHANDLE
{
private:
	edict_t* m_pent;
	int		m_serialnumber;
public:
	edict_t* Get(void);
	edict_t* Set(edict_t* pent);

	operator int();

	operator CBaseEntity* ();

	CBaseEntity* operator = (CBaseEntity* pEntity);
	CBaseEntity* operator ->();
};



// Ugly technique to override base member functions
// Normally it's illegal to cast a pointer to a member function of a derived class to a pointer to a 
// member function of a base class.  static_cast is a sleezy way around that problem.

#define SetThink( a ) m_pfnThink = static_cast <void (CBaseEntity::*)(void)> (a);
#define SetTouch( a ) m_pfnTouch = static_cast <void (CBaseEntity::*)(entvars_t*)> (a);
#define SetUse( a ) m_pfnUse = static_cast <void (CBaseEntity::*)(entvars_t*)> (a);
#define SetBlocked( a ) m_pfnBlocked = static_cast <void (CBaseEntity::*)(entvars_t*)> (a);
#define SetMoveDone( a ) m_pfnCallWhenMoveDone = static_cast <void (CBaseEntity::*)(void)> (a)

class CPointEntity : public CBaseEntity
{
public:
	void	Spawn(void);
private:
};

#define MAX_MULTI_TARGETS	16 // maximum number of targets a single multi_manager entity may be assigned.
#define MS_MAX_TARGETS 32

class CMultiSource : public CPointEntity
{
public:
	void Spawn();
	void KeyValue(KeyValueData* pkvd);
	void Use(entvars_t * pActivator);
	void IsTriggered(entvars_t* pActivator);
	void Register(void);

	int		m_rgEntities;
	int			m_rgTriggered[MS_MAX_TARGETS];

	int			m_iTotal;
	string_t	m_globalstate;
};

//
// generic Delay entity.
//
class CBaseDelay : public CBaseEntity
{
public:
	float		m_flDelay;
	int			m_iszKillTarget;

	//virtual void	KeyValue(KeyValueData* pkvd); dunno if it exists in the original dll
	// common member functions
	void SUB_UseTargets(entvars_t * pActivator);
	void EXPORT DelayThink(void);
};


class CBaseAnimating : public CBaseDelay
{
public:
	float				m_flFrameRate;		// computed FPS for current sequence
	float				m_flGroundSpeed;	// computed linear movement rate for current sequence
	BOOL				m_fSequenceFinished;
	void StudioFrameAdvance(float flInterval);
	void ResetSequenceInfo(float flInterval);
	void DispatchAnimEvents(float flInterval);
};


class CBaseToggle : public CBaseAnimating
{
public:
	//void				KeyValue(KeyValueData* pkvd); also dunno if it exists in the original dll

	TOGGLE_STATE		m_toggle_state;
	float				m_flActivateFinished;//like attack_finished, but for doors
	float				m_flMoveDistance;// how far a door should slide or rotate
	float				m_flWait;
	float				m_flLip;
	float				m_flTWidth;// for plats
	float				m_flTLength;// for plats

	Vector				m_vecPosition1;
	Vector				m_vecPosition2;
	Vector				m_vecAngle1;
	Vector				m_vecAngle2;

	int					m_cTriggersLeft;		// trigger_counter only, # of activations remaining
	float				m_flHeight;
	//EHANDLE				m_hActivator;
	int                 m_hActivator;
	void (CBaseToggle::* m_pfnCallWhenMoveDone)(void);
	Vector				m_vecFinalDest;
	Vector				m_vecFinalAngle;

	int					m_bitsDamageInflict;	// DMG_ damage type that the door or tigger does

	virtual int		GetToggleState(void) { return m_toggle_state; }
	virtual float	GetDelay(void) { return m_flWait; }

	// common member functions
	void LinearMove(Vector	vecDest, float flSpeed);
	void EXPORT LinearMoveDone(void);
	void AngularMove(Vector vecDestAngle, float flSpeed);
	void EXPORT AngularMoveDone(void);

	virtual CBaseToggle* MyTogglePointer(void) { return this; }
	static void			AxisDir(entvars_t* pev);
	static float		AxisDelta(int flags, const Vector& angle1, const Vector& angle2);

	string_t m_sMaster;		// If this button has a master switch, this is the targetname.
	// A master switch must be of the multisource type. If all 
	// of the switches in the multisource have been triggered, then
	// the button will be allowed to operate. Otherwise, it will be
	// deactivated.
};

class CBaseMonster;
#include "basemonster.h"

char* ButtonSound(int sound);

//
// Generic Button
//
class CBaseButton : public CBaseToggle
{
public:
	void Spawn(void);
	virtual void KeyValue(KeyValueData* pkvd);
	void ButtonSpark(void);
	void ButtonUse(entvars_t* pActivator);
	void ButtonTouch(entvars_t* pOther);
	void ButtonActivate();
	void TriggerAndWait(void);
	void ButtonReturn(void);
	void ButtonBackHome(void);
	virtual void TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage);
	BOOL	m_fStayPushed;	// button stays pushed in until touched again?
	BOOL	m_fRotating;		// a rotating button?  default is a sliding button.
	int m_sounds;
};

//
// Converts a entvars_t * to a class pointer
// It will allocate the class and entity if necessary
//
template <class T> T * GetClassPtr( T *a )
{
	entvars_t *pev = (entvars_t *)a;

	// allocate entity if necessary
	if (pev == NULL)
		pev = VARS(CREATE_ENTITY());

	// get the private data
	a = (T *)GET_PRIVATE(ENT(pev));

	if (a == NULL) 
	{
		// allocate private data 
		a = new(pev) T;
		a->pev = pev;
		a->pgv = pev->pSystemGlobals;
	}
	return a;
}