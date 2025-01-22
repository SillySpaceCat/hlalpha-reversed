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

===== h_export.cpp ========================================================

  Entity classes exported by Halflife.

*/

#include "extdll.h"
#include "util.h"
#include <windows.h>
#include <tlhelp32.h>
#include "model.h"

#include "cbase.h"


// Required DLL entry point
BOOL WINAPI DllMain(
   HINSTANCE hinstDLL,
   DWORD fdwReason,
   LPVOID lpvReserved)
{
	if      (fdwReason == DLL_PROCESS_ATTACH)
   {
   }
	else if (fdwReason == DLL_PROCESS_DETACH)
   {
   }
	return TRUE;
}


// Holds engine functionality callbacks
enginefuncs_t g_engfuncs;

BYTE* baseAddress = 0;

typedef trace_t(__cdecl* sv_move)(const float* start, const float* mins, const float* maxs, const float *end, int type, edict_t* passedict);
typedef int(__cdecl* sv_stepdirection)(edict_t* ent, float yaw, float dist);
typedef int(__cdecl* sv_newchasedir2)(edict_t* actor, const float destination, float dist);
typedef void(__cdecl* sv_linkedict)(edict_t* actor, int touchtrigger);
typedef void *(__cdecl* hunk_allocname)(int size, char* name);

sv_move SV_Move;
sv_stepdirection SV_StepDirection;
sv_newchasedir2 SV_NewChaseDir2;
hunk_allocname Hunk_AllocName;
sv_linkedict SV_LinkEdict;

const wchar_t* processname;

DWORD processID = 0;

int droptofloor(edict_t* e)
{
    vec3_t floor = e->v.origin;
    floor.z -= 256;
    trace_t trace;
    trace = SV_Move(e->v.origin, e->v.mins, e->v.maxs, floor, 0, e);
    if (trace.fraction == 1.0f)
        return 0.0;
    e->v.origin.x = trace.endpos.x;
    e->v.origin.y = trace.endpos.y;
    e->v.origin.z = trace.endpos.z;
    SV_LinkEdict(e, 0);
    SetBits(e->v.flags, FL_ONGROUND);
    edict_t* sv_edicts = (edict_t*)(baseAddress + 0x00861D04);
    e->v.groundentity = (byte *)trace.ent - (byte *)sv_edicts;
    return 1;
}

void movetoorigin(edict_t *ent, const float pflGoal, float dist, int iMoveType)
{
    vec3_t	vecDist;

    //vecDist.x = pflGoal[0];
    //vecDist.y = pflGoal[1];
    //vecDist.z = pflGoal[2];

    if (FBitSet(ent->v.flags, FL_ONGROUND))
    {
        if (iMoveType)
        {
            if ((rand() & 3) == 1 || !SV_StepDirection(ent, ent->v.ideal_yaw, dist))
            {
                SV_NewChaseDir2(ent, pflGoal, dist); // semms like there's 2 identical sv_newchasedirs for some reason
            }
        }
        /*
        else
        {
            vecDist[0] -= ent->v.origin[0];
            vecDist[1] -= ent->v.origin[1];

            if (ent->v.flags & (FL_FLY | FL_SWIM))
                vecDist[2] -= ent->v.origin[2];
            else vecDist[2] = 0.0f;

            VectorNormalize(vecDist);
            VectorScale(vecDist, dist, vecDist);
            SV_FlyDirection(ent, vecDist);
        }
        */
    }
}

int    LittleLong(int l)
{
    //byte    b1, b2, b3, b4;

    //b1 = l & 255;
    //b2 = (l >> 8) & 255;
    //b3 = (l >> 16) & 255;
    //b4 = (l >> 24) & 255;

    //return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
    return l;
}

float    LittleFloat(int l)
{
    //byte    b1, b2, b3, b4;

    //b1 = l & 255;
    //b2 = (l >> 8) & 255;
    //b3 = (l >> 16) & 255;
    //b4 = (l >> 24) & 255;

    //return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
    return l;
}

vec_t Length(vec3_t v)
{
    int		i;
    float	length;

    length = 0;
    for (i = 0; i < 3; i++)
        length += v[i] * v[i];
    length = sqrt(length);		// FIXME

    return length;
}

void mod_loadtextures(lump_t *l)
{
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processID);
    byte *mod_base;
    model_t* loadmodel;
    char loadname[32];
    //thanks for this chatgpt
    ReadProcessMemory(hProcess, LPCVOID(baseAddress + 0x00130F10), &mod_base, sizeof(mod_base), NULL);
    ReadProcessMemory(hProcess, LPCVOID(baseAddress + 0x00118700), &loadmodel, sizeof(loadmodel), NULL);
    ReadProcessMemory(hProcess, LPCVOID(baseAddress + 0x001186E0), &loadname, sizeof(loadname), NULL);

    Hunk_AllocName = (hunk_allocname)(baseAddress + 0x000194D0);

    int		i, j, pixels, num, max, altmax, palette;
    miptex_t* mt;
    texture_t* tx, * tx2;
    texture_t* anims[10];
    texture_t* altanims[10];
    dmiptexlump_t* m;
    pixels = 0;
    if (!l)
        return;
    if (!l->filelen)
    {
        loadmodel->textures = NULL;
        return;
    }
    m = (dmiptexlump_t*)(mod_base + l->fileofs);

    m->nummiptex = LittleLong(m->nummiptex);

    loadmodel->numtextures = m->nummiptex;
    //WriteProcessMemory(hProcess, LPVOID(baseAddress + 0x00190494), &r_notexture_mip, sizeof(r_notexture_mip), NULL);
    //WriteProcessMemory(hProcess, LPVOID(baseAddress + 0x00130F10), &mod_base, sizeof(mod_base), NULL);
    //WriteProcessMemory(hProcess, LPVOID(baseAddress + 0x00118700), &loadmodel, sizeof(loadmodel), NULL);
    //WriteProcessMemory(hProcess, LPVOID(baseAddress + 0x001186E0), &loadname, sizeof(loadname), NULL);
    loadmodel->textures = (texture_t **)Hunk_AllocName(4 * m->nummiptex * sizeof(*loadmodel->textures), loadname);

    for (i = 0; i < m->nummiptex; i++)
    {
        m->dataofs[i] = LittleLong(m->dataofs[i]);
        if (m->dataofs[i] == -1)
            continue;
        mt = (miptex_t*)((byte*)m + m->dataofs[i]);
        mt->width = LittleLong(mt->width);
        mt->height = LittleLong(mt->height);
        for (j = 0; j < MIPLEVELS; j++)
            mt->offsets[j] = LittleLong(mt->offsets[j]);

        if ((mt->width & 15) || (mt->height & 15))
            ALERT(at_console, "Texture %s is not 16 aligned", mt->name);
        pixels = 85 * mt->height * mt->width / 64;
        palette = 3 * *(unsigned __int16*)((char*)mt + sizeof(miptex_t) + pixels);
        tx = (texture_t*)Hunk_AllocName(2 + 8 * palette + pixels + sizeof(texture_t), loadname);
        loadmodel->textures[i] = tx;

        memcpy(tx->name, mt->name, sizeof(tx->name));
        tx->width = mt->width;
        tx->height = mt->height;
        for (j = 0; j < MIPLEVELS; j++)
            tx->offsets[j] = mt->offsets[j] + sizeof(texture_t) - sizeof(miptex_t);
        tx->paloffset = sizeof(texture_t) + pixels + 2;
        // the pixels immediately follow the structures
        memcpy(&tx[1], &mt[1], pixels + palette + 2);

        //if (!Q_strncmp(mt->name, "sky", 3))
            //R_InitSky(tx);
        unsigned __int8* mippal;
        unsigned __int16* texpal;
        static byte texgamma[256];
        ReadProcessMemory(hProcess, LPCVOID(baseAddress + 0x0013CFA0), &texgamma, sizeof(texgamma), NULL);
        mippal = (unsigned __int8*)&mt[1] + tx->paloffset;
        texpal = (unsigned __int16*)((char*)&tx[1] + tx->paloffset);
        for (int j = 0; j < palette; j++)
        {
            mippal = (unsigned __int8*)&mt[1] + tx->paloffset + j;
            texpal = (unsigned __int16*)((char*)&tx[1] + tx->paloffset + j);
            texpal[0] = texgamma[mippal[0]];
        }
    }

    //
    // sequence the animations
    //
    for (i = 0; i < m->nummiptex; i++)
    {
        tx = loadmodel->textures[i];
        if (!tx || tx->name[0] != '+')
            continue;
        if (tx->anim_next)
            continue;	// allready sequenced

        // find the number of frames in the animation
        memset(anims, 0, sizeof(anims));
        memset(altanims, 0, sizeof(altanims));

        max = tx->name[1];
        altmax = 0;
        if (max >= 'a' && max <= 'z')
            max -= 'a' - 'A';
        if (max >= '0' && max <= '9')
        {
            max -= '0';
            altmax = 0;
            anims[max] = tx;
            max++;
        }
        else if (max >= 'A' && max <= 'J')
        {
            altmax = max - 'A';
            max = 0;
            altanims[altmax] = tx;
            altmax++;
        }
        //else
            //Sys_Error("Bad animating texture %s", tx->name);

        for (j = i + 1; j < m->nummiptex; j++)
        {
            tx2 = loadmodel->textures[j];
            if (!tx2 || tx2->name[0] != '+')
                continue;
            if (strcmp(tx2->name + 2, tx->name + 2))
                continue;

            num = tx2->name[1];
            if (num >= 'a' && num <= 'z')
                num -= 'a' - 'A';
            if (num >= '0' && num <= '9')
            {
                num -= '0';
                anims[num] = tx2;
                if (num + 1 > max)
                    max = num + 1;
            }
            else if (num >= 'A' && num <= 'J')
            {
                num = num - 'A';
                altanims[num] = tx2;
                if (num + 1 > altmax)
                    altmax = num + 1;
            }
            //else
                //Sys_Error("Bad animating texture %s", tx->name);
        }

#define	ANIM_CYCLE	2
        // link them all together
        for (j = 0; j < max; j++)
        {
            tx2 = anims[j];
            //if (!tx2)
                //Sys_Error("Missing frame %i of %s", j, tx->name);
            tx2->anim_total = max * ANIM_CYCLE;
            tx2->anim_min = j * ANIM_CYCLE;
            tx2->anim_max = (j + 1) * ANIM_CYCLE;
            tx2->anim_next = anims[(j + 1) % max];
            if (altmax)
                tx2->alternate_anims = altanims[0];
        }
        for (j = 0; j < altmax; j++)
        {
            tx2 = altanims[j];
            //if (!tx2)
                //Sys_Error("Missing frame %i of %s", j, tx->name);
            tx2->anim_total = altmax * ANIM_CYCLE;
            tx2->anim_min = j * ANIM_CYCLE;
            tx2->anim_max = (j + 1) * ANIM_CYCLE;
            tx2->anim_next = altanims[(j + 1) % altmax];
            if (max)
                tx2->alternate_anims = anims[0];
        }
    }
    //WriteProcessMemory(hProcess, LPVOID(baseAddress + 0x00190494), &r_notexture_mip, sizeof(r_notexture_mip), NULL);
    //WriteProcessMemory(hProcess, LPVOID(baseAddress + 0x00130F10), &mod_base, sizeof(mod_base), NULL);
    WriteProcessMemory(hProcess, LPVOID(baseAddress + 0x00118700), &loadmodel, sizeof(loadmodel), NULL);
    //WriteProcessMemory(hProcess, LPVOID(baseAddress + 0x001186E0), &loadname, sizeof(loadname), NULL);
}

void mod_loadtexinfo(lump_t* l)
{
    texinfo_t* in;
    mtexinfo_t* out;
    int 	i, j, count;
    int		miptex;
    float	len1, len2;
    texture_s *r_notexture_mip;
    model_t* loadmodel;
    byte* mod_base;
    char loadname[32];
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processID);

    ReadProcessMemory(hProcess, LPCVOID(baseAddress + 0x00190494), &r_notexture_mip, sizeof(r_notexture_mip), NULL);
    ReadProcessMemory(hProcess, LPCVOID(baseAddress + 0x00130F10), &mod_base, sizeof(mod_base), NULL);
    ReadProcessMemory(hProcess, LPCVOID(baseAddress + 0x00118700), &loadmodel, sizeof(loadmodel), NULL);
    ReadProcessMemory(hProcess, LPCVOID(baseAddress + 0x001186E0), &loadname, sizeof(loadname), NULL);

    in = (texinfo_t*)(mod_base + l->fileofs);
    if (l->filelen % sizeof(*in))
        ALERT(at_console, "mod_loadtexinfo: funny lump size in %s",loadmodel->name);
    count = l->filelen / sizeof(*in);
    out = (mtexinfo_t*)Hunk_AllocName(count * sizeof(*out), loadname);

    loadmodel->texinfo = out;
    loadmodel->numtexinfo = count;

    for (i = 0; i < count; i++, in++, out++)
    {
        for (j = 0; j < 8; j++)
        {
            out->vecs[0][j] = in->vecs[0][j];
        }

        len1 = Length(out->vecs[0]);
        len2 = Length(out->vecs[1]);
        len1 = (len1 + len2) / 2;
        if (len1 < 0.32)
            out->mipadjust = 4;
        else if (len1 < 0.49)
            out->mipadjust = 3;
        else if (len1 < 0.99)
            out->mipadjust = 2;
        else
            out->mipadjust = 1;

        miptex = LittleLong(in->miptex);
        out->flags = LittleLong(in->flags);

        if (!loadmodel->textures)
        {
            out->texture = r_notexture_mip;	// checkerboard texture
            out->flags = 0;
        }
        else
        {
            if (miptex >= loadmodel->numtextures)
                ALERT(at_console, "miptex >= loadmodel->numtextures");
            out->texture = loadmodel->textures[miptex];
            if (!out->texture)
            {
                out->texture = r_notexture_mip; // texture not found
                out->flags = 0;
            }
        }
    }
    WriteProcessMemory(hProcess, LPVOID(baseAddress + 0x00118700), &loadmodel, sizeof(loadmodel), NULL);
}

void opengl_mod_loadtextures(lump_t* l)
{
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processID);
    byte* mod_base;
    model_t* loadmodel;
    char loadname[32];
    //thanks for this chatgpt
    ReadProcessMemory(hProcess, LPCVOID(baseAddress + 0x00130F10), &mod_base, sizeof(mod_base), NULL);
    ReadProcessMemory(hProcess, LPCVOID(baseAddress + 0x00118700), &loadmodel, sizeof(loadmodel), NULL);
    ReadProcessMemory(hProcess, LPCVOID(baseAddress + 0x001186E0), &loadname, sizeof(loadname), NULL);

    Hunk_AllocName = (hunk_allocname)(baseAddress + 0x000194D0);

    int		i, j, pixels, num, max, altmax, palette;
    miptex_t* mt;
    texture_t* tx, * tx2;
    texture_t* anims[10];
    texture_t* altanims[10];
    dmiptexlump_t* m;
    pixels = 0;
    if (!l)
        return;
    if (!l->filelen)
    {
        loadmodel->textures = NULL;
        return;
    }
    m = (dmiptexlump_t*)(mod_base + l->fileofs);

    m->nummiptex = LittleLong(m->nummiptex);

    loadmodel->numtextures = m->nummiptex;
    loadmodel->textures = (texture_t**)Hunk_AllocName(4 * m->nummiptex * sizeof(*loadmodel->textures), loadname);

    for (i = 0; i < m->nummiptex; i++)
    {
        m->dataofs[i] = LittleLong(m->dataofs[i]);
        if (m->dataofs[i] == -1)
            continue;
        mt = (miptex_t*)((byte*)m + m->dataofs[i]);
        mt->width = LittleLong(mt->width);
        mt->height = LittleLong(mt->height);
        for (j = 0; j < MIPLEVELS; j++)
            mt->offsets[j] = LittleLong(mt->offsets[j]);

        if ((mt->width & 15) || (mt->height & 15))
            ALERT(at_console, "Texture %s is not 16 aligned", mt->name);
        pixels = 85 * mt->height * mt->width / 64;
        palette = 3 * *(unsigned __int16*)((char*)mt + sizeof(miptex_t) + pixels);
        tx = (texture_t*)Hunk_AllocName(2 + 8 * palette + pixels + sizeof(texture_t), loadname);
        loadmodel->textures[i] = tx;

        memcpy(tx->name, mt->name, sizeof(tx->name));
        tx->width = mt->width;
        tx->height = mt->height;
        for (j = 0; j < MIPLEVELS; j++)
            tx->offsets[j] = mt->offsets[j] + sizeof(texture_t) - sizeof(miptex_t);
        tx->paloffset = sizeof(texture_t) + pixels + 2;
        // the pixels immediately follow the structures
        memcpy(&tx[1], &mt[1], pixels + palette + 2);

        //if (!Q_strncmp(mt->name, "sky", 3))
            //R_InitSky(tx);
        unsigned __int8* mippal;
        unsigned __int16* texpal;
        static byte texgamma[256];
        ReadProcessMemory(hProcess, LPCVOID(baseAddress + 0x0013CFA0), &texgamma, sizeof(texgamma), NULL);
        mippal = (unsigned __int8*)&mt[1] + tx->paloffset;
        texpal = (unsigned __int16*)((char*)&tx[1] + tx->paloffset);
        for (int j = 0; j < palette; j++)
        {
            mippal = (unsigned __int8*)&mt[1] + tx->paloffset + j;
            texpal = (unsigned __int16*)((char*)&tx[1] + tx->paloffset + j);
            texpal[0] = texgamma[mippal[0]];
        }
    }

    //
    // sequence the animations
    //
    for (i = 0; i < m->nummiptex; i++)
    {
        tx = loadmodel->textures[i];
        if (!tx || tx->name[0] != '+')
            continue;
        if (tx->anim_next)
            continue;	// allready sequenced

        // find the number of frames in the animation
        memset(anims, 0, sizeof(anims));
        memset(altanims, 0, sizeof(altanims));

        max = tx->name[1];
        altmax = 0;
        if (max >= 'a' && max <= 'z')
            max -= 'a' - 'A';
        if (max >= '0' && max <= '9')
        {
            max -= '0';
            altmax = 0;
            anims[max] = tx;
            max++;
        }
        else if (max >= 'A' && max <= 'J')
        {
            altmax = max - 'A';
            max = 0;
            altanims[altmax] = tx;
            altmax++;
        }
        //else
            //Sys_Error("Bad animating texture %s", tx->name);

        for (j = i + 1; j < m->nummiptex; j++)
        {
            tx2 = loadmodel->textures[j];
            if (!tx2 || tx2->name[0] != '+')
                continue;
            if (strcmp(tx2->name + 2, tx->name + 2))
                continue;

            num = tx2->name[1];
            if (num >= 'a' && num <= 'z')
                num -= 'a' - 'A';
            if (num >= '0' && num <= '9')
            {
                num -= '0';
                anims[num] = tx2;
                if (num + 1 > max)
                    max = num + 1;
            }
            else if (num >= 'A' && num <= 'J')
            {
                num = num - 'A';
                altanims[num] = tx2;
                if (num + 1 > altmax)
                    altmax = num + 1;
            }
            //else
                //Sys_Error("Bad animating texture %s", tx->name);
        }

#define	ANIM_CYCLE	2
        // link them all together
        for (j = 0; j < max; j++)
        {
            tx2 = anims[j];
            //if (!tx2)
                //Sys_Error("Missing frame %i of %s", j, tx->name);
            tx2->anim_total = max * ANIM_CYCLE;
            tx2->anim_min = j * ANIM_CYCLE;
            tx2->anim_max = (j + 1) * ANIM_CYCLE;
            tx2->anim_next = anims[(j + 1) % max];
            if (altmax)
                tx2->alternate_anims = altanims[0];
        }
        for (j = 0; j < altmax; j++)
        {
            tx2 = altanims[j];
            //if (!tx2)
                //Sys_Error("Missing frame %i of %s", j, tx->name);
            tx2->anim_total = altmax * ANIM_CYCLE;
            tx2->anim_min = j * ANIM_CYCLE;
            tx2->anim_max = (j + 1) * ANIM_CYCLE;
            tx2->anim_next = altanims[(j + 1) % altmax];
            if (max)
                tx2->alternate_anims = anims[0];
        }
    }
    //WriteProcessMemory(hProcess, LPVOID(baseAddress + 0x00190494), &r_notexture_mip, sizeof(r_notexture_mip), NULL);
    //WriteProcessMemory(hProcess, LPVOID(baseAddress + 0x00130F10), &mod_base, sizeof(mod_base), NULL);
    WriteProcessMemory(hProcess, LPVOID(baseAddress + 0x00118700), &loadmodel, sizeof(loadmodel), NULL);
    //WriteProcessMemory(hProcess, LPVOID(baseAddress + 0x001186E0), &loadname, sizeof(loadname), NULL);
}

void opengl_mod_loadtexinfo(lump_t* l)
{

}

void REtest()
{
    DWORD oldProtect;
    BYTE jmp[5] = { 0xE9 };

    if (processname == L"engine.exe")
    {
        //
        //MOD_LOADTEXTURES
        //
        VirtualProtect(baseAddress + 0x0004AAF0, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

        *(DWORD*)(jmp + 1) = (DWORD)mod_loadtextures - ((DWORD)baseAddress + 0x0004AAF0) - 5;
        memcpy(baseAddress + 0x0004AAF0, jmp, 5);

        VirtualProtect(baseAddress + 0x0004AAF0, 5, oldProtect, &oldProtect);

        //
        //MOD_LOADTEXINFO
        //

        VirtualProtect(baseAddress + 0x0004B320, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

        *(DWORD*)(jmp + 1) = (DWORD)mod_loadtexinfo - ((DWORD)baseAddress + 0x0004B320) - 5;
        memcpy(baseAddress + 0x0004B320, jmp, 5);

        VirtualProtect(baseAddress + 0x0004B320, 5, oldProtect, &oldProtect);
    }
    else
    {
        //
        //MOD_LOADTEXTURES
        //
        VirtualProtect(baseAddress + 0x0001141A, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

        *(DWORD*)(jmp + 1) = (DWORD)opengl_mod_loadtextures - ((DWORD)baseAddress + 0x0001141A) - 5;
        memcpy(baseAddress + 0x0001141A, jmp, 5);

        VirtualProtect(baseAddress + 0x0001141A, 5, oldProtect, &oldProtect);

        //
        //MOD_LOADTEXINFO
        //

        VirtualProtect(baseAddress + 0x00011BCF, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

        *(DWORD*)(jmp + 1) = (DWORD)opengl_mod_loadtexinfo - ((DWORD)baseAddress + 0x00011BCF) - 5;
        memcpy(baseAddress + 0x00011BCF, jmp, 5);

        VirtualProtect(baseAddress + 0x00011BCF, 5, oldProtect, &oldProtect);
    }
}

void hooktoengine()
{
    processname = L"engine.exe";
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(snapshot, &processEntry)) 
    {
        do {
            if (wcscmp(processEntry.szExeFile, processname) == 0) {
                processID = processEntry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &processEntry));

        if (!processID) //opengl
        {
            processname = L"enginegl.exe";
            do {
                if (wcscmp(processEntry.szExeFile, processname) == 0) {
                    processID = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &processEntry));
        }
    }
    CloseHandle(snapshot);

    HANDLE snapshot2 = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
    if (snapshot2 != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 moduleEntry;
        moduleEntry.dwSize = sizeof(MODULEENTRY32);

        if (Module32First(snapshot2, &moduleEntry)) {

            baseAddress = moduleEntry.modBaseAddr;
        }
        CloseHandle(snapshot2);  // Always close the snapshot handle when done
    }

    DWORD oldProtect1;
    DWORD oldProtect2;


    VirtualProtect(baseAddress + 0x00040330, 5, PAGE_EXECUTE_READWRITE, &oldProtect1);// droptofloor
    VirtualProtect(baseAddress + 0x00018DF0, 5, PAGE_EXECUTE_READWRITE, &oldProtect2);// movetoorigin

    BYTE jmp1[5] = { 0xE9 };
    *(DWORD*)(jmp1 + 1) = (DWORD)droptofloor - ((DWORD)baseAddress + 0x00040330) - 5;// droptofloor
    memcpy(baseAddress + 0x00040330, jmp1, 5);

    BYTE jmp2[5] = { 0xE9 };
    *(DWORD*)(jmp2 + 1) = (DWORD)movetoorigin - ((DWORD)baseAddress + 0x00018DF0) - 5;// movetoorigin
    memcpy(baseAddress + 0x00018DF0, jmp2, 5);

    VirtualProtect(baseAddress + 0x00040330, 5, oldProtect1, &oldProtect1); // droptofloor
    VirtualProtect(baseAddress + 0x00018DF0, 5, oldProtect2, &oldProtect2); // movetoorigin

    if (processname == L"engine.exe") //shitty way to see if its software or opengl
    {
        VirtualProtect(baseAddress + 0x00040330, 5, PAGE_EXECUTE_READWRITE, &oldProtect1);// droptofloor
        VirtualProtect(baseAddress + 0x00018DF0, 5, PAGE_EXECUTE_READWRITE, &oldProtect2);// movetoorigin

        BYTE jmp1[5] = { 0xE9 };
        *(DWORD*)(jmp1 + 1) = (DWORD)droptofloor - ((DWORD)baseAddress + 0x00040330) - 5;// droptofloor
        memcpy(baseAddress + 0x00040330, jmp1, 5);

        BYTE jmp2[5] = { 0xE9 };
        *(DWORD*)(jmp2 + 1) = (DWORD)movetoorigin - ((DWORD)baseAddress + 0x00018DF0) - 5;// movetoorigin
        memcpy(baseAddress + 0x00018DF0, jmp2, 5);

        VirtualProtect(baseAddress + 0x00040330, 5, oldProtect1, &oldProtect1); // droptofloor
        VirtualProtect(baseAddress + 0x00018DF0, 5, oldProtect2, &oldProtect2); // movetoorigin


        SV_Move = (sv_move)(baseAddress + 0x00048A60);
        SV_StepDirection = (sv_stepdirection)(baseAddress + 0x000184D0);
        SV_NewChaseDir2 = (sv_newchasedir2)(baseAddress + 0x00018AE0);
        SV_LinkEdict = (sv_linkedict)(baseAddress + 0x00047A50);
    }
    else
    {
        VirtualProtect(baseAddress + 0x0000331C, 5, PAGE_EXECUTE_READWRITE, &oldProtect1);// droptofloor
        VirtualProtect(baseAddress + 0x00005BD9, 5, PAGE_EXECUTE_READWRITE, &oldProtect2);// movetoorigin

        BYTE jmp1[5] = { 0xE9 };
        *(DWORD*)(jmp1 + 1) = (DWORD)droptofloor - ((DWORD)baseAddress + 0x0000331C) - 5;// droptofloor
        memcpy(baseAddress + 0x0000331C, jmp1, 5);

        BYTE jmp2[5] = { 0xE9 };
        *(DWORD*)(jmp2 + 1) = (DWORD)movetoorigin - ((DWORD)baseAddress + 0x00005BD9) - 5;// movetoorigin
        memcpy(baseAddress + 0x00005BD9, jmp2, 5);

        VirtualProtect(baseAddress + 0x0000331C, 5, oldProtect1, &oldProtect1); // droptofloor
        VirtualProtect(baseAddress + 0x00005BD9, 5, oldProtect2, &oldProtect2); // movetoorigin


        SV_Move = (sv_move)(baseAddress + 0x0002B17C);
        SV_StepDirection = (sv_stepdirection)(baseAddress + 0x000052B4);
        SV_NewChaseDir2 = (sv_newchasedir2)(baseAddress + 0x000058D4);
        SV_LinkEdict = (sv_linkedict)(baseAddress + 0x0002a218);
    }
}

void DLLEXPORT GiveFnptrsToDll( enginefuncs_t* pengfuncsFromEngine )
{
	memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));
	
    hooktoengine(); //i have to put some of the engine's
                    //functions here because i can't
                    //for the life of me figure out why
                    //move_to_origin and drop_to_floor make
                    //the engine freak out.

    //REtest(); // just to help me reverse engineer the engine
}