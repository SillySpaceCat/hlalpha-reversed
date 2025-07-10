/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

//part of the engine's reverse engineering

typedef enum { mod_brush, mod_sprite, mod_alias } modtype_t;

typedef enum { ST_SYNC = 0, ST_RAND } synctype_t;

typedef struct efrag_s
{
	struct mleaf_s* leaf;
	struct efrag_s* leafnext;
	struct entity_s* entity;
	struct efrag_s* entnext;
} efrag_t;

typedef struct mplane_s
{
	vec3_t	normal;
	float	dist;
	byte	type;			// for texture axis selection and fast side tests
	byte	signbits;		// signx + signy<<1 + signz<<1
	byte	pad[2];
} mplane_t;

//software
typedef struct texture_s
{
	char		name[16]; //16
	unsigned	width, height; //20
	int			anim_total;				// 24
	int			anim_min, anim_max;		// 28-32
	struct texture_s* anim_next;		// in the animation sequence
	struct texture_s* alternate_anims;	// bmodels in frmae 1 use these
	unsigned	offsets[4];		// four mip maps stored
	unsigned paloffset;

} texture_t;

//opengl

typedef struct opengl_texture_s
{
	char		name[16];
	unsigned	width, height;
	int			gl_texturenum;
	struct opengl_msurface_s* texturechain;	// for gl_texsort drawing
	int			anim_total;				// total tenths in sequence ( 0 = no)
	int			anim_min, anim_max;		// time for this frame min <=time< max
	struct opengl_texture_s* anim_next;		// in the animation sequence
	struct opengl_texture_s* alternate_anims;	// bmodels in frmae 1 use these
	unsigned	offsets[4];		// four mip maps stored
	unsigned int paloffset;
	char offset[2];

} opengl_texture_t;

typedef struct
{
	float		mins[3], maxs[3];
	float		origin[3];
	int			headnode[4];
	int			visleafs;		// not including the solid leaf 0
	int			firstface, numfaces;
} dmodel_t;

typedef struct
{
	float		vecs[2][4];
	float		mipadjust;
	texture_t* texture;
	int			flags;
} mtexinfo_t;

typedef struct
{
	float		vecs[2][4];
	float		mipadjust;
	opengl_texture_t* texture;
	int			flags;
} opengl_mtexinfo_t;

typedef struct msurface_s
{
	int			visframe;		// should be drawn when node is crossed

	int			dlightframe;
	int			dlightbits;

	mplane_t* plane;
	int			flags;

	int			firstedge;	// look up in model->surfedges[], negative numbers
	int			numedges;	// are backwards edges

	// surface generation data
	struct surfcache_s* cachespots[4];

	short		texturemins[2];
	short		extents[2];

	mtexinfo_t* texinfo;

	// lighting info
	byte		styles[4];
	byte* samples;		// [numstyles*surfsize]
} msurface_t;

typedef struct opengl_msurface_s
{
	int			visframe;		// should be drawn when node is crossed

	int			dlightframe;
	int			dlightbits;

	mplane_t* plane;
	int			flags;

	int			firstedge;	// look up in model->surfedges[], negative numbers
	int			numedges;	// are backwards edges

	// surface generation data
	struct surfcache_s* cachespots[4];

	short		texturemins[2];
	short		extents[2];

	opengl_mtexinfo_t* texinfo;

	// lighting info
	byte		styles[4];
	byte* samples;		// [numstyles*surfsize]
} opengl_msurface_t;

#define	MIPLEVELS	4
typedef struct miptex_s
{
	char		name[16];
	unsigned	width, height;
	unsigned	offsets[MIPLEVELS];		// four mip maps stored
} miptex_t;

typedef struct
{
	int			nummiptex;
	int			dataofs[4];		// [nummiptex]
} dmiptexlump_t;

typedef struct texinfo_s
{
	float		vecs[2][4];		// [s/t][xyz offset]
	int			miptex;
	int			flags;
} texinfo_t;

typedef struct mleaf_s
{
	// common with node
	int			contents;		// wil be a negative contents number
	int			visframe;		// node needs to be traversed if current

	short		minmaxs[6];		// for bounding box culling

	struct mnode_s* parent;

	// leaf specific
	byte* compressed_vis;
	efrag_t* efrags;

	msurface_t** firstmarksurface;
	int			nummarksurfaces;
	int			key;			// BSP sequence number for leaf's contents
	byte		ambient_sound_level[4];
} mleaf_t;

typedef struct opengl_mleaf_s
{
	// common with node
	int			contents;		// wil be a negative contents number
	int			visframe;		// node needs to be traversed if current

	short		minmaxs[6];		// for bounding box culling

	struct mnode_s* parent;

	// leaf specific
	byte* compressed_vis;
	efrag_t* efrags;

	opengl_msurface_t** firstmarksurface;
	int			nummarksurfaces;
	int			key;			// BSP sequence number for leaf's contents
	byte		ambient_sound_level[4];
} opengl_mleaf_t;

typedef struct
{
	vec3_t		position;
} mvertex_t;

typedef struct
{
	unsigned short	v[2];
	unsigned int	cachededgeoffset;
} medge_t;

typedef struct mnode_s
{
	// common with leaf
	int			contents;		// 0, to differentiate from leafs
	int			visframe;		// node needs to be traversed if current

	short		minmaxs[6];		// for bounding box culling

	struct mnode_s* parent;

	// node specific
	mplane_t* plane;
	struct mnode_s* children[2];

	unsigned short		firstsurface;
	unsigned short		numsurfaces;
} mnode_t;

typedef struct
{
	int			planenum;
	short		children[2];	// negative numbers are contents
} dclipnode_t;

typedef struct
{
	dclipnode_t* clipnodes;
	mplane_t* planes;
	int			firstclipnode;
	int			lastclipnode;
	vec3_t		clip_mins;
	vec3_t		clip_maxs;
} hull_t;

typedef struct cache_user_s
{
	void* data;
} cache_user_t;

//software
typedef struct model_s
{
	char name[64];
	qboolean needload;
	modtype_t type;
	int numframes;
	synctype_t synctype;
	int flags;
	vec3_t mins;
	vec3_t maxs;
	float radius;
	int firstmodelsurface;
	int nummodelsurfaces;
	int numsubmodels;
	dmodel_t* submodels;
	int numplanes;
	mplane_t* planes;
	int numleafs;
	mleaf_t* leafs;
	int numvertexes;
	mvertex_t* vertexes;
	int numedges;
	medge_t* edges;
	int numnodes;
	mnode_t* nodes;
	int numtexinfo;
	mtexinfo_t* texinfo;
	int numsurfaces;
	msurface_t* surfaces;
	int numsurfedges;
	int* surfedges;
	int numclipnodes;
	dclipnode_t* clipnodes;
	int nummarksurfaces;
	msurface_t** marksurfaces;
	hull_t hulls[4];
	int numtextures;
	texture_t** textures;
	byte* visdata;
	byte* lightdata;
	char* entities;
	cache_user_t cache;
}model_t;

//opengl
typedef struct openglmodel_s
{
	char name[64];
	int offset[7];
	qboolean needload;
	modtype_t type;
	int numframes;
	synctype_t synctype;
	int flags;
	vec3_t mins;
	vec3_t maxs;
	float radius;
	int firstmodelsurface;
	int nummodelsurfaces;
	int numsubmodels;
	dmodel_t* submodels;
	int numplanes;
	mplane_t* planes;
	int numleafs;
	opengl_mleaf_t* leafs;
	int numvertexes;
	mvertex_t* vertexes;
	int numedges;
	medge_t* edges;
	int numnodes;
	mnode_t* nodes;
	int numtexinfo;
	opengl_mtexinfo_t* texinfo;
	int numsurfaces;
	opengl_msurface_t* surfaces;
	int numsurfedges;
	int* surfedges;
	int numclipnodes;
	dclipnode_t* clipnodes;
	int nummarksurfaces;
	opengl_msurface_t** marksurfaces;
	hull_t hulls[4];
	int numtextures;
	opengl_texture_t** textures;
	byte* visdata;
	byte* lightdata;
	char* entities;
	cache_user_t cache;
}opengl_model_t;

typedef struct lump_s
{
	int fileofs;
	int filelen;
} lump_t;

