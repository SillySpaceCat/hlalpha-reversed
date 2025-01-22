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
// GoldSrc Model Viewer (MDL v6)
//
// Purpose: STUDIO MODELS
//
//=============================================================================

#ifndef _STUDIO_H_
#define _STUDIO_H_


#define MAXSTUDIOTRIANGLES	20000	// TODO: tune this
#define MAXSTUDIOVERTS		4096	// TODO: tune this (was 2048)
// TOMAS: polyrobo.mdl has 3576 verts
#define MAXSTUDIOSEQUENCES	256		// total animation sequences
#define MAXSTUDIOSKINS		100		// total textures
#define MAXSTUDIOSRCBONES	512		// bones allowed at source movement
#define MAXSTUDIOBONES		128		// total bones actually used
#define MAXSTUDIOMODELS		32		// sub-models per model
#define MAXSTUDIOBODYPARTS	32
#define MAXSTUDIOGROUPS		4
#define MAXSTUDIOANIMATIONS	512		// per sequence
#define MAXSTUDIOMESHES		256
#define MAXSTUDIOEVENTS		1024
#define MAXSTUDIOPIVOTS		256
#define MAXSTUDIOCONTROLLERS 8


#define STUDIO_VERSION		6

// little-endian "IDST"
#define IDSTUDIOHEADER	(('T'<<24)+('S'<<16)+('D'<<8)+'I')


//----------------------------------------------------------------------------
// File header
//----------------------------------------------------------------------------

// Model header (188B)
// TOMAS: OK
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

//----------------------------------------------------------------------------
// Bones
//----------------------------------------------------------------------------

// bones (60B)
// TOMAS: OK
typedef struct
{
	char				name[32];		// bone name for symbolic links
	int		 			parent;			// parent bone
	int		 			unused[6];
} mstudiobone_t;


// bone controllers (16B)
// TOMAS: OK
typedef struct
{
	int					bone;			// -1 == 0 (TOMAS??)
	int					type;			// X, Y, Z, XR, YR, ZR, M
	float				start;
	float				end;
} mstudiobonecontroller_t;

//----------------------------------------------------------------------------
// Sequence
//----------------------------------------------------------------------------

// sequence descriptions (104B)
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

// events (4B)
// TOMAS: OK
typedef struct
{
	short	 			frame;
	short				type;
} mstudioevent_t;


// pivots
typedef struct
{
	vec3_t				org;			// pivot point
	int					start;
	int					end;
} mstudiopivot_t;

//----------------------------------------------------------------------------
// Animations
//----------------------------------------------------------------------------

// animation frames (16B)
// TOMAS: "New" in MDL v6
typedef struct
{
	int					numpos;			// count of mstudiobnonepos_t
	int					posindex;		// (->mstudiobnonepos_t)
	int					numrot;			// count of mstudiobonerot_t
	int					rotindex;		// (->mstudiobonerot_t)
} mstudioanim_t;

// animation frames data (16B)
// TOMAS: "New" in MDL v6
typedef struct
{
	int					frame;			// frame id (frame <= numframes)
	vec3_t				pos;
} mstudiobonepos_t;

// animation frames data (8B)
// TOMAS: "New" in MDL v6
typedef struct
{
	short 				frame;			// frame id (frame <= numframes)
	short 				angle[3];		// (values: +/-18000; 18000 = 180deg)
} mstudiobonerot_t;


//----------------------------------------------------------------------------
// Body parts
//----------------------------------------------------------------------------

// body part index (76B)
// TOMAS: OK (unchanged)
typedef struct
{
	char				name[64];
	int					nummodels;
	int					base;
	int					modelindex; // index into models array (->mstudiomodel_t)
} mstudiobodyparts_t;


//----------------------------------------------------------------------------
// Textures
//----------------------------------------------------------------------------

// skin info (80B)
// TOMAS: OK (unchanged)
typedef struct
{
	char				name[64];
	int					flags;
	int					width;
	int					height;
	int					index;
} mstudiotexture_t;

//----------------------------------------------------------------------------
// Model parts
//----------------------------------------------------------------------------

// studio models (108B)
typedef struct
{
	char				name[64];

	int					type;

	int					unk01;			// TOMAS: (==1)
	int					unused01;		// TOMAS: UNUSED (checked)

	int					nummesh;
	int					meshindex;

	// vertex bone info
	int					numverts;		// number of unique vertices
	int					vertinfoindex;	// vertex bone info

	// normal bone info
	int					numnorms;		// number of unique surface normals
	int					norminfoindex;	// normal bone info

	// TOMAS: NEW IN MDL v6
	int					unused02;		// TOMAS: UNUSED (checked)
	int					modeldataindex;	// (->mstudiomodeldata_t)
} mstudiomodel_t;

// TOMAS: NEW IN MDL v6
// studio models data (28B)
typedef struct
{
	// TOMAS: UNDONE:
	int					unk01;
	int					unk02;
	int					unk03;

	int					numverts;		// number of unique vertices
	int					vertindex;		// vertex vec3_t (data)

	int					numnorms;		// number of unique surface normals
	int					normindex;		// normal vec3_t (data)

} mstudiomodeldata_t;

// meshes (20B)
// TOMAS: OK (unchanged)
typedef struct
{
	int					numtris;
	int					triindex;		// separate triangles (->mstudiotrivert_t)
	int					skinref;
	int					numnorms;		// per mesh normals
	int					normindex;		// normal vec3_t
	// TOMAS: "0"
} mstudiomesh_t;

// triangles
// TOMAS: OK (unchanged)
typedef struct
{
	short				vertindex;		// index into vertex array (relative)
	short				normindex;		// index into normal array (relative)
	short				s, t;			// s,t position on skin
} mstudiotrivert_t;

//----------------------------------------------------------------------------
// Flags
//----------------------------------------------------------------------------

// lighting options
#define STUDIO_NF_FLATSHADE		0x0001
#define STUDIO_NF_CHROME		0x0002
#define STUDIO_NF_FULLBRIGHT	0x0004

// motion flags
#define STUDIO_X		0x0001
#define STUDIO_Y		0x0002
#define STUDIO_Z		0x0004
#define STUDIO_XR		0x0008
#define STUDIO_YR		0x0010
#define STUDIO_ZR		0x0020
#define STUDIO_LX		0x0040
#define STUDIO_LY		0x0080
#define STUDIO_LZ		0x0100
#define STUDIO_AX		0x0200
#define STUDIO_AY		0x0400
#define STUDIO_AZ		0x0800
#define STUDIO_AXR		0x1000
#define STUDIO_AYR		0x2000
#define STUDIO_AZR		0x4000
#define STUDIO_TYPES	0x7FFF
#define STUDIO_RLOOP	0x8000	// controller that wraps shortest distance

// sequence flags
#define STUDIO_LOOPING	0x0001

// bone flags
#define STUDIO_HAS_NORMALS	0x0001
#define STUDIO_HAS_VERTICES 0x0002
#define STUDIO_HAS_BBOX		0x0004
#define STUDIO_HAS_CHROME	0x0008	// if any of the textures have chrome on them

#define	Q_PI_F				(float)Q_PI

#define RAD_TO_STUDIO		(18000.0f/Q_PI_F)
#define STUDIO_TO_RAD		(Q_PI_F/18000.0f)

#endif