/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Default behaviors.
//=========================================================
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "defaultai.h"

// Fail
Task_t tlFail[] = {
    { TASK_STOP_MOVING, 0 },
    { TASK_SET_ACTIVITY, (float)ACT_IDLE },
    { TASK_WAIT, 2 }
};
Schedule_t slFail[] = {
    { tlFail, ARRAYSIZE(tlFail), bits_COND_CAN_ATTACK, 0, "Fail" }
};

// Idle Stand
Task_t tlIdleStand[] = {
    { TASK_STOP_MOVING, 0 },
    { TASK_SET_ACTIVITY, (float)ACT_IDLE },
    { TASK_WAIT, 5 }
};
Schedule_t slIdleStand[] = {
    { tlIdleStand, ARRAYSIZE(tlIdleStand),
      bits_COND_NEW_ENEMY | bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE,
      0, "IdleStand" }
};

// Idle Walk
Task_t tlIdleWalk[] = {
    { TASK_WALK_PATH, 9999 },
    { TASK_WAIT_FOR_MOVEMENT, 0 }
};
Schedule_t slIdleWalk[] = {
    { tlIdleWalk, ARRAYSIZE(tlIdleWalk),
      bits_COND_NEW_ENEMY | bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE,
      0, "IdleWalk" }
};

// Alert Stand
Task_t tlAlertStand[] = {
    { TASK_STOP_MOVING, 0 },
    { TASK_SET_ACTIVITY, (float)ACT_IDLE },
    { TASK_WAIT, 20 }
};
Schedule_t slAlertStand[] = {
    { tlAlertStand, ARRAYSIZE(tlAlertStand),
      bits_COND_NEW_ENEMY | bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE,
      0, "AlertStand" }
};

// Combat Stand
Task_t tlCombatStand[] = {
    { TASK_STOP_MOVING, 0 },
    { TASK_SET_ACTIVITY, (float)ACT_IDLE },
    { TASK_WAIT_INDEFINITE, 0 }
};
Schedule_t slCombatStand[] = {
    { tlCombatStand, ARRAYSIZE(tlCombatStand),
      bits_COND_CAN_ATTACK, 0, "CombatStand" }
};

// Range Attack
Task_t tlRangeAttack1[] = {
    { TASK_STOP_MOVING, 0 },
    { TASK_FACE_ENEMY, 0 },
    { TASK_RANGE_ATTACK1, 0 }
};
Schedule_t slRangeAttack1[] = {
    { tlRangeAttack1, ARRAYSIZE(tlRangeAttack1),
      bits_COND_CAN_ATTACK, 0, "RangeAttack1" }
};

// Melee Attack
Task_t tlMeleeAttack[] = {
    { TASK_STOP_MOVING, 0 },
    { TASK_FACE_ENEMY, 0 },
    { TASK_MELEE_ATTACK1, 0 }
};
Schedule_t slMeleeAttack[] = {
    { tlMeleeAttack, ARRAYSIZE(tlMeleeAttack),
      bits_COND_CAN_ATTACK, 0, "MeleeAttack" }
};

// Take Cover
Task_t tlTakeCoverFromBestSound[] = {
    { TASK_STOP_MOVING, 0 },
    { TASK_FIND_COVER_FROM_BEST_SOUND, 0 },
    { TASK_RUN_PATH, 0 },
    { TASK_WAIT_FOR_MOVEMENT, 0 }
};
Schedule_t slTakeCoverFromBestSound[] = {
    { tlTakeCoverFromBestSound, ARRAYSIZE(tlTakeCoverFromBestSound),
      bits_COND_NEW_ENEMY, 0, "TakeCoverFromBestSound" }
};

// Chase Enemy
Task_t tlChaseEnemy[] = {
    { TASK_GET_PATH_TO_ENEMY, 0 },
    { TASK_RUN_PATH, 0 },
    { TASK_WAIT_FOR_MOVEMENT, 0 }
};
Schedule_t slChaseEnemy[] = {
    { tlChaseEnemy, ARRAYSIZE(tlChaseEnemy),
      bits_COND_NEW_ENEMY, 0, "ChaseEnemy" }
};

// Small Flinch
Task_t tlSmallFlinch[] = {
    { TASK_STOP_MOVING, 0 },
    { TASK_SMALL_FLINCH, 0 }
};
Schedule_t slSmallFlinch[] = {
    { tlSmallFlinch, ARRAYSIZE(tlSmallFlinch), 0, 0, "SmallFlinch" }
};

// Die
Task_t tlDie[] = {
    { TASK_STOP_MOVING, 0 },
    { TASK_DIE, 0 }
};
Schedule_t slDie[] = {
    { tlDie, ARRAYSIZE(tlDie), 0, 0, "Die" }
};