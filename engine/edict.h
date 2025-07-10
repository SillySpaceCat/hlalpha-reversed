//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined EDICT_H
#define EDICT_H
#ifdef _WIN32
#pragma once
#endif
#define	MAX_ENT_LEAFS	16

#include "progdefs.h"

typedef struct
{
	vec3_t origin;
	vec3_t angles;
	int modelindex;
	int unknown1;
	int frame;
	int colormap;
	int skin;
	int effects;
	int rendermode;
	int renderamt;
	int renderfx;
} entity_state_t;

struct edict_s
{
	int	free;
	link_t		area;				// linked to a division node or leaf
	int serialnumber;

	int			num_leafs;
	short		leafnums[MAX_ENT_LEAFS];

	entity_state_t baseline;

	void* pvPrivateData;		// Alloced and freed by engine, used by DLLs	
	float freetime;
	entvars_t	v;					// C exported fields from progs

	// other fields from progs come immediately after
};

#endif