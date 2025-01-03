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

typedef TraceResult(__cdecl* sv_move)(const float* start, const float* mins, const float* maxs, const float *end, int type, edict_t* passedict);
typedef int(__cdecl* sv_stepdirection)(edict_t* ent, float yaw, float dist);
typedef int(__cdecl* sv_newchasedir2)(edict_t* actor, const float destination, float dist);

int droptofloor(edict_t* e)
{
    sv_move SV_Move = (sv_move)(baseAddress + 0x00048a60);
    vec3_t floor = e->v.origin;
    floor.z -= 256;
    TraceResult trace;
    trace = SV_Move(e->v.origin, e->v.mins, e->v.maxs, floor, 0, e);
    if (trace.flFraction == 1.0f)
        return 0.0;
    e->v.origin.x = trace.vecEndPos.x;
    e->v.origin.y = trace.vecEndPos.y;
    e->v.origin.z = trace.vecEndPos.z;
    SetBits(e->v.flags, FL_ONGROUND);
    e->v.groundentity = OFFSET(trace.pHit);
    return 1;
}

void movetoorigin(edict_t *ent, const float pflGoal, float dist, int iMoveType)
{
    vec3_t	vecDist;

    sv_stepdirection SV_StepDirection = (sv_stepdirection)(baseAddress + 0x000184D0);
    sv_newchasedir2 SV_NewChaseDir2 = (sv_newchasedir2)(baseAddress + 0x00018AE0);

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

void hooktoengine()
{
    DWORD processID = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(snapshot, &processEntry)) {
        const wchar_t* processname = L"engine.exe";
        do {
            if (wcscmp(processEntry.szExeFile, processname) == 0) {
                processID = processEntry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &processEntry));
    }
    CloseHandle(snapshot);
    processID = processID;

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
}

void DLLEXPORT GiveFnptrsToDll( enginefuncs_t* pengfuncsFromEngine )
{
	memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));
	
    hooktoengine(); //i have to put some of the engine's
                    //functions here because i can't
                    //for the life of me figure out why
                    //move_to_origin and drop_to_floor make
                    //the engine freak out.
}