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

===== weapons.cpp ========================================================

  functions governing the selection/use of weapons for players

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

DLL_GLOBAL		short g_sModelIndexShell;
DLL_GLOBAL		short g_sModelIndexShrapnel;


void W_Precache( void )
{
	PRECACHE_MODEL ("models/grenade.mdl");
	PRECACHE_MODEL ("sprites/shard.spr");
	PRECACHE_MODEL ("models/v_crowbar.mdl");
	PRECACHE_MODEL ("models/v_glock.mdl");
	PRECACHE_MODEL ("models/v_mp5.mdl");
	
	g_sModelIndexShell = PRECACHE_MODEL ("models/shell.mdl");
	g_sModelIndexShrapnel = PRECACHE_MODEL ("models/shrapnel.mdl");

	PRECACHE_SOUND ("weapons/debris1.wav");
	PRECACHE_SOUND ("weapons/debris2.wav");
	PRECACHE_SOUND ("weapons/debris3.wav");
	PRECACHE_SOUND ("player/pl_shell1.wav");
	PRECACHE_SOUND ("player/pl_shell2.wav");
	PRECACHE_SOUND ("player/pl_shell3.wav");
	PRECACHE_SOUND ("weapons/hks1.wav");
	PRECACHE_SOUND ("weapons/hks2.wav");
	PRECACHE_SOUND ("weapons/hks3.wav");
	PRECACHE_SOUND ("weapons/pl_gun1.wav");
	PRECACHE_SOUND ("weapons/pl_gun2.wav");
	PRECACHE_SOUND ("weapons/glauncher.wav");
	PRECACHE_SOUND ("weapons/glauncher2.wav");
	PRECACHE_SOUND ("weapons/g_bounce1.wav");
	PRECACHE_SOUND ("weapons/g_bounce2.wav");
	PRECACHE_SOUND ("weapons/g_bounce3.wav");
}


/*
===============================================================================

PLAYER WEAPON USE

===============================================================================
*/

void CBasePlayer::W_ChangeWeapon(int weapon)
{
	if (weapon == 2)
		selectedweapon = WEAPON_CROWBAR;
	else if (weapon == 3)
		selectedweapon = WEAPON_GLOCK;
	else if (weapon == 4)
		selectedweapon = WEAPON_MP5;
}

void CBasePlayer::W_SetCurrentAmmo( void )
{
	if ( pev->weapon == WEAPON_CROWBAR )
	{
		pev->weaponmodel = ALLOC_STRING("models/v_crowbar.mdl");
		pev->currentammo = 99.0;
	}
	else if ( pev->weapon == WEAPON_GLOCK )
	{
		pev->weaponmodel = ALLOC_STRING("models/v_glock.mdl");
		pev->currentammo = 99.0;
	}
	else if ( pev->weapon == WEAPON_MP5 )
	{
		pev->weaponmodel = ALLOC_STRING("models/v_mp5.mdl");
		pev->currentammo = 99.0;
	}
	else
	{
		pev->weaponmodel = 0;
		pev->currentammo = 0;
	}
}

void CBasePlayer::ImpulseCommands()
{
	if ((pev->button & IN_USE) && (pev->pSystemGlobals->time > nextuse))
	{
		Use();
		nextuse = pev->pSystemGlobals->time + 0.5;
	}
	if (pev->impulse >= 1 && pev->impulse <= 8)
		W_ChangeWeapon(pev->impulse);
	if (pev->impulse == 10)
		W_ChangeWeapon(pev->weapon + 1);
	//char str[64];
	//char str2[64];
	//sprintf_s(str, "%d", pev->weapon);
	//sprintf_s(str2, "%s", STRING(pev->weaponmodel));
	//EMIT_SOUND(ENT(pev), CHAN_BODY, str, 1, ATTN_NORM);
	//EMIT_SOUND(ENT(pev), CHAN_BODY, str2, 1, ATTN_NORM);

	//if (pev->impulse == 11)
	//{
	//	if ((pev->weapon - 1) < 0)
	//		ServerflagsCommand(pev, 31);
	//	else
	//		ServerflagsCommand(pev, pev->weapon - 1);
	//}
	if (pev->impulse == 100)
	{
		if (FBitSet(pev->effects, 8))
		{
			ClearBits(pev->effects, 8);
		}
		else
		{
			SetBits(pev->effects, 8);
		}
	}
	if (pev->impulse == 200)
	{
		if (showlines)
		{
			showlines = 0;
			ALERT(at_console, "Lines Off");
		}
		else
		{
			showlines = 1;
			ALERT(at_console, "Lines On");
		}
	}

	pev->impulse = 0;
}

void CBaseMonster::ClearMultiDamage()
{
	multi_damage = 0;
	multi_ent = 0;
}

void CBaseMonster::AddMultiDamage(float a3)
{
	if (pgv->trace_ent)
	{
		if (pgv->trace_ent == multi_ent)
		{
			multi_damage += a3;
		}
		else
		{
			ApplyMultiDamage();
			multi_ent = pgv->trace_ent;
			multi_damage = a3;
		}
	}
}

void CBaseMonster::ApplyMultiDamage()
{
	if (!multi_ent)
		return;
	CBaseMonster* pMonster = (CBaseMonster*)GET_PRIVATE(ENT(multi_ent));
	if (pMonster)
		pMonster->TakeDamage(pev, multi_damage);
	if (!FClassnameIs(VARS(multi_ent), "cycler"))
	{
		if (UTIL_RandomFloat(0.0, 1.0) < 0.3)
		{
			//UTIL_MakeVectors(pev->origin - VARS(multi_ent)->origin);

		}
	}
}

void CBaseMonster::TraceAttack(float damage, int integer1, Vector dir)
{
	Vector  vel, org;

	org = pgv->trace_endpos - dir * 4;

	if (VARS(pgv->trace_ent)->takedamage)
	{
		AddMultiDamage(damage);
		if (FClassnameIs(VARS(pgv->trace_ent), "func_glass"))
			return;
		if (FClassnameIs(VARS(pgv->trace_ent), "func_breakable"))
			return;
		if (VARS(pgv->trace_ent)->health > 1000) //probably a cycler or sumthing
			return;

		//BloodSpray(org, bloodcolor, damage);
		WRITE_BYTE(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(MSG_BROADCAST, TE_BLOOD);
		WRITE_COORD(MSG_BROADCAST, pgv->trace_endpos.x);
		WRITE_COORD(MSG_BROADCAST, pgv->trace_endpos.y);
		WRITE_COORD(MSG_BROADCAST, pgv->trace_endpos.z);
		WRITE_COORD(MSG_BROADCAST, 0);
		WRITE_COORD(MSG_BROADCAST, 0);
		WRITE_COORD(MSG_BROADCAST, 0);
		CBaseMonster* pMonster = (CBaseMonster*)GET_PRIVATE(ENT(pgv->trace_ent));
		WRITE_BYTE(MSG_BROADCAST, pMonster->m_bloodColor);
		WRITE_BYTE(MSG_BROADCAST, damage);


		if (VARS(pgv->trace_ent)->health <= multi_damage)
		{
			WRITE_BYTE(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(MSG_BROADCAST, TE_BLOODSTREAM);
			WRITE_COORD(MSG_BROADCAST, pgv->trace_endpos.x);
			WRITE_COORD(MSG_BROADCAST, pgv->trace_endpos.y);
			WRITE_COORD(MSG_BROADCAST, pgv->trace_endpos.z);
			WRITE_COORD(MSG_BROADCAST, UTIL_RandomFloat(-1, 1));
			WRITE_COORD(MSG_BROADCAST, UTIL_RandomFloat(-1, 1));
			WRITE_COORD(MSG_BROADCAST, UTIL_RandomFloat(0, 1));
			CBaseMonster* pMonster = (CBaseMonster*)GET_PRIVATE(ENT(pgv->trace_ent));
			WRITE_BYTE(MSG_BROADCAST, pMonster->m_bloodColor);
			WRITE_BYTE(MSG_BROADCAST, UTIL_RandomLong(80, 150));
		}
	}
	if (!integer1)
	{
		entvars_t *entity = VARS(pgv->trace_ent);
		if (entity->solid == SOLID_BSP)
		{
			WRITE_BYTE(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(MSG_BROADCAST, TE_GUNSHOT);
			WRITE_COORD(MSG_BROADCAST, pgv->trace_endpos.x);
			WRITE_COORD(MSG_BROADCAST, pgv->trace_endpos.y);
			WRITE_COORD(MSG_BROADCAST, pgv->trace_endpos.z);

			WRITE_BYTE(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(MSG_BROADCAST, TE_DECAL);
			WRITE_COORD(MSG_BROADCAST, pgv->trace_endpos.x);
			WRITE_COORD(MSG_BROADCAST, pgv->trace_endpos.y);
			WRITE_COORD(MSG_BROADCAST, pgv->trace_endpos.z);
			WRITE_SHORT(MSG_BROADCAST, ENTINDEX(pgv->trace_ent));
			WRITE_BYTE(MSG_BROADCAST, rand() % 5);
		}
	}
}

// 
// GUN ATTACK
//


void CBaseMonster::FireBullets(int number, Vector dir, Vector spread, float distance)
{
	UTIL_MakeVectors(pev->v_angle);

	Vector src = pev->origin + (pgv->v_forward * 10);
	Vector direction;
	src.z = pev->view_ofs.z - 4 + pev->origin.z;

	ClearMultiDamage();
	TraceResult tr;

	UTIL_TraceLine(src, src + dir * 2048, 1, FALSE, &tr);
	//puff_org = trace_endpos - dir * 4;

	while (number > 0)
	{
		direction = dir + UTIL_RandomFloat(-1, 1) * spread.x * pgv->v_right + UTIL_RandomFloat(-1, 1) * spread.y * pgv->v_up;
		UTIL_TraceLine(src, src + direction * distance, 1, ENT(pev), &tr);
		if (FClassnameIs(pev, "player"))
		{
			if (pev->weapon == 4)
			{
				Vector v_forward;
				Vector v_right;
				Vector total;
				if FBitSet(pev->flags, FL_DUCKING)
				{
					v_forward = pev->pSystemGlobals->v_forward * 16;
					v_right = pev->pSystemGlobals->v_right * 2.0 + pev->origin;
					v_right.z += 6.0;
					total = v_forward + v_right;
				}
				else
				{
					v_forward = pev->pSystemGlobals->v_forward * 16;
					v_right = pev->pSystemGlobals->v_right * 2.0 + pev->origin;
					v_right.z += 24.0;
					total = v_forward + v_right;
				}
				WRITE_BYTE(MSG_BROADCAST, SVC_TEMPENTITY);
				WRITE_BYTE(MSG_BROADCAST, TE_TRACER);
				WRITE_COORD(MSG_BROADCAST, total.x);
				WRITE_COORD(MSG_BROADCAST, total.y);
				WRITE_COORD(MSG_BROADCAST, total.z);
				WRITE_COORD(MSG_BROADCAST, tr.vecEndPos.x);
				WRITE_COORD(MSG_BROADCAST, tr.vecEndPos.y);
				WRITE_COORD(MSG_BROADCAST, tr.vecEndPos.z);
			}
		}
		if (tr.flFraction != 1.0)
		{
			//TraceAttack(2, integer1, direction);
			if (pgv->trace_ent)
				edict_t *phitentity = ENT(pgv->trace_ent);
			TraceAttack(2, 0, direction);
		}

		number -= 1;
	}
	ApplyMultiDamage();
}

void CBasePlayer::Swing_Crowbar()
{
	pev->pSystemGlobals->msg_entity = OFFSET(pev); //i think

	WRITE_BYTE(1, SVC_WEAPONANIM);
	int anim = abs(rand());
	if ((anim) % 2 == 1)
		WRITE_BYTE(1, 1);
	else
		WRITE_BYTE(1, 2);
	pev->pSystemGlobals->msg_entity = 0;
	UTIL_MakeVectors(pev->v_angle);
	Vector aim = UTIL_GetAimVector(ENT(pev), 1000);
	FireBullets(1, aim, Vector(0.025, 0.025, 0.025), 64);
	nextattack = pgv->time + 1.0;
}

void CBasePlayer::Shoot_Pistol()
{
	pev->effects = static_cast<int>(pev->effects) | 2;
	UTIL_MakeVectors(pev->v_angle);
	pev->pSystemGlobals->msg_entity = OFFSET(pev); //i think

	WRITE_BYTE(1, SVC_WEAPONANIM);
	WRITE_BYTE(1, 0);
	pev->pSystemGlobals->msg_entity = 0;

	int sound = abs(rand());
	if ((sound) % 2 == 1)
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/pl_gun1.wav", 1, ATTN_NORM);
	else
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/pl_gun2.wav", 1, ATTN_NORM);
	Vector aimvector = UTIL_GetAimVector(ENT(pev), 2048);
	FireBullets(1, aimvector, Vector(0.025, 0.025, 0.025), 2048);

	nextattack = pgv->time + 0.3;

}
void CBasePlayer::Shoot_Mp5()
{
	pev->effects = static_cast<int>(pev->effects) | 2;

	pev->pSystemGlobals->msg_entity = OFFSET(pev); //i think

	WRITE_BYTE(1, SVC_WEAPONANIM);
	WRITE_BYTE(1, 0);
	pev->pSystemGlobals->msg_entity = 0;
	UTIL_MakeVectors(pev->v_angle);

	float sound = UTIL_RandomFloat(0.0, 1.0);
	if (sound < 0.33)
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks1.wav", 1, ATTN_NORM);
	else if (sound < 0.66)
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks2.wav", 1, ATTN_NORM);
	else
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks3.wav", 1, ATTN_NORM);

	Vector aim = UTIL_GetAimVector(ENT(pev), 1000);
	FireBullets(1, aim, Vector(0.01, 0.01, 0.01), 2048);
	nextattack = pgv->time + 0.1;
	//variable = pgv->time + 1.0; not sure what this is

}

void CBasePlayer::W_Attack(void)
{
	if (pev->deadflag != DEAD_DEAD)
	{
		pev->weaponframe = 0;
		switch (pev->weapon)
		{
		case 1:
			Swing_Crowbar();
			//PlayerSetAnimation(30);
			break;
		case 2:
			Shoot_Pistol();
			//PlayerSetAnimation(30);
			break;
		case 4:
			Shoot_Mp5();
			//PlayerSetAnimation(30);
			break;
		}
	}
}

void CBaseItem::Touch(entvars_t* pActivator)
{
	edict_t* entity = ENT(pgv->other);
	if (!strcmp(STRING(entity->v.classname), "player"))
	{
		SetThink(&CBaseItem::SUB_Remove);
		pev->nextthink = pev->pSystemGlobals->time + 0.1;
	}
}

void CBaseMonster::ShootGrenade(CBaseGrenade *grenade, entvars_t *owner, Vector origin, Vector dir)
{
	Vector angles;

	grenade->pev->movetype = MOVETYPE_BOUNCE;
	grenade->pev->classname = ALLOC_STRING("grenade");
	grenade->pev->renderamt = 0;
	grenade->pev->rendermode = 0;
	grenade->pev->renderfx = 0;
	if (!strcmp(STRING(owner->classname), "player"))
		grenade->pev->gravity = 0.4;
	grenade->pev->solid = SOLID_BBOX;
	grenade->pev->owner = OFFSET(owner);
	SET_MODEL(ENT(grenade->pev), "models/grenade.mdl");
	UTIL_SetSize(grenade->pev, Vector(0, 0, 0), Vector(0, 0, 0));
	grenade->pev->origin = origin;
	grenade->pev->velocity = dir;
	angles = UTIL_VecToAngles(grenade->pev->velocity);
	grenade->pev->angles = angles;
	grenade->pev->dmg = 100;

	if (!strcmp(STRING(owner->classname), "player"))
	{
		grenade->m_pfnThink = (&CBaseMonster::SUB_DoNothing);
		grenade->pev->avelocity.x = UTIL_RandomFloat(-100.0, -500.0);
	}
	else //timed explosion
	{
		grenade->m_pfnThink = static_cast <void (CBaseEntity::*)(void)> (&CBaseGrenade::Explosion);
		grenade->pev->nextthink = grenade->pev->pSystemGlobals->time + 2.0;
		grenade->pev->avelocity.x = -400;
	}
}


void CBasePlayer::W_WeaponFrame(void)
{
	if (pgv->time >= nextattack)
	{
		ImpulseCommands();
		if (pev->button & IN_CANCEL)
		{
			if (pev->weapons)
				pev->weapon = (pev->weapon & 0xFF0000u) >> 16; // not sure if this'll work
		}
		else if (pev->button & IN_ATTACK2)
		{
			if (pev->weapons && !firegrenade)
			{
				//shootgrenade()
				pev->button &= ~IN_ATTACK2;
				firegrenade = 1;
				return;
			}
			if (pev->weapon == 4 && pev->pSystemGlobals->time > nextgrenade)
			{
				firegrenade = 1;
				UTIL_MakeVectors(pev->v_angle);
				pev->pSystemGlobals->msg_entity = OFFSET(pev);
				WRITE_BYTE(1, SVC_WEAPONANIM);
				WRITE_BYTE(1, 2);
				pev->pSystemGlobals->msg_entity = 0;
				if (UTIL_RandomFloat(0.0, 1.0) < 0.5)
					EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/glauncher.wav", 0.75, ATTN_NORM);
				else
					EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/glauncher2.wav", 0.75, ATTN_NORM);
				
				CBaseGrenade* grenade = GetClassPtr((CBaseGrenade*)NULL);
				ShootGrenade(grenade, pev, pev->origin + pev->view_ofs, (pev->pSystemGlobals->v_forward * 800));
				pev->button &= ~IN_ATTACK2;
				nextgrenade = pev->pSystemGlobals->time + 1.0;
			}
		}
		else
			firegrenade = 0;
		if ((pev->button & IN_ATTACK) != 0)
		{
			if (!weaponactivity)
			{
				if (pev->weapon != selectedweapon)
				{
					pev->weapon = selectedweapon; //???
					pev->button &= ~1u;
					W_SetCurrentAmmo();
					weaponactivity = 1;
				}
				else
					W_Attack();
			}
		}
		else
			weaponactivity = 0;
	}
}

void ExplodeModel(const Vector& vecOrigin, float speed, int model, int count)
{
	WRITE_BYTE(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(MSG_BROADCAST, TE_EXPLODEMODEL);
	WRITE_COORD(MSG_BROADCAST, vecOrigin.x);
	WRITE_COORD(MSG_BROADCAST, vecOrigin.y);
	WRITE_COORD(MSG_BROADCAST, vecOrigin.z);
	WRITE_COORD(MSG_BROADCAST, speed);
	WRITE_SHORT(MSG_BROADCAST, model);
	WRITE_SHORT(MSG_BROADCAST, count);
	WRITE_BYTE(MSG_BROADCAST, 15);// 1.5 seconds
}

void RadiusDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int iClassIgnore)
{
	TraceResult	tr;
	trace_t trace;
	float		flAdjustedDamage, falloff;
	Vector		vecSpot;

	pevInflictor->origin.z = pevInflictor->origin.z + 1.0;

	// iterate on all entities in the vicinity.
	edict_t* entity = UTIL_FindEntityInSphere(pevInflictor->origin, flDamage * 2.0);
	edict_t* entity2 = NULL;
	while (1)
	{
		if (!OFFSET(entity))
			break;
		if (entity->v.takedamage != DAMAGE_NO)
		{
			CBaseMonster* pTarget = (CBaseMonster*)(CBaseEntity::Instance(entity));
			if (!pTarget)
				return;
			if (pTarget->Classify() == iClassIgnore)
			{
				break;
			}

			UTIL_TraceLine(pevInflictor->origin, Vector(entity->v.origin.x, entity->v.origin.y, entity->v.origin.z + (entity->v.size.y * 0.5)), 0, ENT(pevInflictor), &tr);
			
			Vector distance = Vector(pevInflictor->origin.x - entity->v.origin.x,
									pevInflictor->origin.y - entity->v.origin.y,
									pevInflictor->origin.z - entity->v.origin.z);
			flAdjustedDamage = flDamage - distance.Length() * 0.4;

			if (tr.flFraction == 1.0)
			{
				pTarget->TakeDamage(pevInflictor, flAdjustedDamage);
			}
		}

		entity = ENT(entity->v.chain);
		if (entity == NULL)
			return;
	}
}

void CBaseGrenade::Explosion()
{
	TraceResult tr;
	pev->model = 0;
	pev->solid = SOLID_NOT;
	WRITE_BYTE(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(MSG_BROADCAST, TE_EXPLOSION);
	WRITE_COORD(MSG_BROADCAST, pev->origin.x);
	WRITE_COORD(MSG_BROADCAST, pev->origin.y);
	WRITE_COORD(MSG_BROADCAST, pev->origin.z);
	ExplodeModel(pev->origin, 400, g_sModelIndexShrapnel, 30);
	RadiusDamage(pev, VARS(pev->owner), pev->dmg, 0);

	Vector vecSpot = pev->origin - pev->velocity.Normalize() * 32;

	if (pev->velocity != g_vecZero)
		UTIL_TraceLine(vecSpot, vecSpot + pev->velocity.Normalize() * 64, 0, ENT(pev), &tr);
	else //grenade is probably on the ground
		UTIL_TraceLine(vecSpot, Vector(pev->origin.x, pev->origin.y + 8.0, pev->origin.z - 24), 0, ENT(pev), &tr);


	WRITE_BYTE(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(MSG_BROADCAST, TE_DECAL);
	WRITE_COORD(MSG_BROADCAST, pev->origin.x);
	WRITE_COORD(MSG_BROADCAST, pev->origin.y);
	WRITE_COORD(MSG_BROADCAST, pev->origin.z);
	WRITE_SHORT(MSG_BROADCAST, ENTINDEX(pgv->trace_ent));
	if (UTIL_RandomFloat(0.0, 1.0) >= 0.5)
		WRITE_BYTE(MSG_BROADCAST, 13);
	else
		WRITE_BYTE(MSG_BROADCAST, 12);

	SetThink(&CBaseGrenade::SUB_Remove);
	pev->nextthink = pev->pSystemGlobals->time + 2.0;
}

void CBaseGrenade::Touch(entvars_t* pActivator)
{
	edict_t* owner = ENT(pev->owner);
	edict_t* toucher = ENT(pgv->other);
	const char *classname = STRING(owner->v.classname);
	if (!strcmp(classname, "player"))
	{
		pev->enemy = OFFSET(ENT(pgv->other)->v.pContainingEntity);
		SetThink(&CBaseGrenade::Explosion);
		pev->nextthink = pev->pSystemGlobals->time;
		if (!strcmp(STRING(toucher->v.classname), "func_breakable") || !strcmp(STRING(toucher->v.classname), "func_glass"))
		{
			CBaseEntity* breakable = (CBaseEntity*)GET_PRIVATE(toucher->v.pContainingEntity);
			breakable->TakeDamage(pev, pev->dmg);
		}
	}
	else
	{
		pev->movetype = MOVETYPE_BOUNCE;
		pev->avelocity = Vector(300, 300, 300);
		pev->gravity = 1;
		if (pev->pSystemGlobals->other != pev->owner)
		{
			//play bounce sound
			//
			//


			pev->velocity = pev->velocity * 0.8;
		}
	}
}