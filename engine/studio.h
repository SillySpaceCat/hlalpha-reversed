//=============================================================================
//
// Copyright (c) 1998, Valve LLC. All rights reserved.
//
// Copyright (c) 1999-2013 Tomas Slavotinek (aka baso88, GeckoN[CZ])
// E-mail: slavotinek@gmail.com
//
// This product contains software technology licensed from Id
// Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
// All Rights Reserved.
//
// contains code from GoldSrc Model Viewer (MDL v6) made by Tomas Slavotinek (aka baso88, GeckoN[CZ]) 
//
// Purpose: STUDIO MODELS
//
//=============================================================================




#ifndef _STUDIO_H_
#define _STUDIO_H_

/*
==============================================================================

STUDIO MODELS

Studio models are position independent, so the cache manager can move them.
==============================================================================
*/
 

#define MAXSTUDIOTRIANGLES	20000	// TODO: tune this
#define MAXSTUDIOVERTS		2048	// TODO: tune this
#define MAXSTUDIOSEQUENCES	2048	// total animation sequences -- KSH incremented
#define MAXSTUDIOSKINS		100		// total textures
#define MAXSTUDIOSRCBONES	512		// bones allowed at source movement
#define MAXSTUDIOBONES		128		// total bones actually used
#define MAXSTUDIOMODELS		32		// sub-models per model
#define MAXSTUDIOBODYPARTS	32
#define MAXSTUDIOGROUPS		16
#define MAXSTUDIOANIMATIONS	2048		
#define MAXSTUDIOMESHES		256
#define MAXSTUDIOEVENTS		1024
#define MAXSTUDIOPIVOTS		256
#define MAXSTUDIOCONTROLLERS 8

typedef struct
{
	int					id;
	int					version;

	char				name[64];
	// 48h
	int					length;

	// 4Ch
	int					numbones;				// bones
	int					boneindex;				// (->BCh)

	// 54h
	int					numbonecontrollers;		// bone controllers
	// TOMAS: turret.mdl has 2
	int					bonecontrollerindex;	// if num == 0 then this points to bones! not controlers!

	// 5Ch
	int					numseq;					// animation sequences
	int					seqindex;

	// 64h
	int					numtextures;			// raw textures
	int					textureindex;
	int					texturedataindex;

	// 70h
	int					numskinref;				// replaceable textures
	int					numskinfamilies;
	int					skinindex;

	// 7Ch
	int					numbodyparts;
	int					bodypartindex;			// (->mstudiobodyparts_t)

	int					unused[14];				// TOMAS: UNUSED (checked)

} studiohdr_t;

typedef struct
{
	char				label[32];	// textual name
	char				name[64];	// file name
    //int32				unused1;    // was "cache"  - index pointer
	//int					unused2;    // was "data" -  hack for group 0
} mstudioseqgroup_t;

typedef struct
{
	char				label[32];		// sequence label

	float				fps;			// frames per second
	int					flags;			// looping/non-looping flags

	int					numevents;		// TOMAS: USED (not always 0)
	int					eventindex;

	int					numframes;		// number of frames per sequence

	int					unused01;		// TOMAS: UNUSED (checked)

	int					numpivots;		// number of foot pivots
	// TOMAS: polyrobo.mdl use this (4)
	int					pivotindex;

	int					motiontype;		// TOMAS: USED (not always 0)
	int					motionbone;		// motion bone id (0)

	int					unused02;		// TOMAS: UNUSED (checked)
	vec3_t				linearmovement;	// TOMAS: USED (not always 0)

	int					numblends;		// TOMAS: UNUSED (checked)
	int					animindex;		// (->mstudioanim_t)

	int					unused03[2];	// TOMAS: UNUSED (checked)

} mstudioseqdesc_t;

#endif