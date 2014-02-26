/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2009 Darklegion Development

This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "g_local.h"

/*
================
G_SetBuildableAnim

Triggers an animation client side
================
*/
void G_SetBuildableAnim( gentity_t *ent, buildableAnimNumber_t anim, qboolean force )
{
  int localAnim = anim | ( ent->s.legsAnim & ANIM_TOGGLEBIT );

  if( force )
    localAnim |= ANIM_FORCEBIT;

  // don't flip the togglebit more than once per frame
  if( ent->animTime != level.time )
  {
    ent->animTime = level.time;
    localAnim ^= ANIM_TOGGLEBIT;
  }

  ent->s.legsAnim = localAnim;
}

/*
================
G_SetIdleBuildableAnim

Set the animation to use whilst no other animations are running
================
*/
void G_SetIdleBuildableAnim( gentity_t *ent, buildableAnimNumber_t anim )
{
  ent->s.torsoAnim = anim;
}

/*
===============
G_CheckSpawnPoint

Check if a spawn at a specified point is valid
===============
*/
gentity_t *G_CheckSpawnPoint( int spawnNum, const vec3_t origin,
    const vec3_t normal, buildable_t spawn, vec3_t spawnOrigin )
{
  float   displacement;
  vec3_t  mins, maxs;
  vec3_t  cmins, cmaxs;
  vec3_t  localOrigin;
  trace_t tr;

  BG_BuildableBoundingBox( spawn, mins, maxs );

  if( spawn == BA_A_SPAWN )
  {
    VectorSet( cmins, -MAX_ALIEN_BBOX, -MAX_ALIEN_BBOX, -MAX_ALIEN_BBOX );
    VectorSet( cmaxs,  MAX_ALIEN_BBOX,  MAX_ALIEN_BBOX,  MAX_ALIEN_BBOX );

    displacement = ( maxs[ 2 ] + MAX_ALIEN_BBOX ) * M_ROOT3;
    VectorMA( origin, displacement, normal, localOrigin );
  }
  else if( spawn == BA_H_SPAWN )
  {
    BG_ClassBoundingBox( PCL_HUMAN, cmins, cmaxs, NULL, NULL, NULL );

    VectorCopy( origin, localOrigin );
    localOrigin[ 2 ] += maxs[ 2 ] + fabs( cmins[ 2 ] ) + 1.0f;
  }
  else
    return NULL;

  trap_Trace( &tr, origin, NULL, NULL, localOrigin, spawnNum, MASK_SHOT );

  if( tr.entityNum != ENTITYNUM_NONE )
    return &g_entities[ tr.entityNum ];

  trap_Trace( &tr, localOrigin, cmins, cmaxs, localOrigin, -1, MASK_PLAYERSOLID );

  if( tr.entityNum != ENTITYNUM_NONE )
    return &g_entities[ tr.entityNum ];

  if( spawnOrigin != NULL )
    VectorCopy( localOrigin, spawnOrigin );

  return NULL;
}

/*
==================
G_GetBuildPoints

Get the number of build points from a position
==================
*/
int G_GetBuildPoints( const vec3_t pos, team_t team )
{
  if( G_TimeTilSuddenDeath( ) <= 0 )
  {
    return 0;
  }
  else if( team == TEAM_ALIENS )
  {
    return level.alienBuildPoints;
  }
  else if( team == TEAM_HUMANS )
  {
    return 0xDEADBEE; //humans use the material system
  }

  return 0;
}

/*
================
G_IsDCCBuilt

See if any powered DCC exists
================
*/
qboolean G_IsDCCBuilt( void )
{
  int       i;
  gentity_t *ent;
  static gentity_t *cache = NULL;
  
  if( cache && cache->inuse && cache->s.eType == ET_BUILDABLE &&
      cache->s.modelindex == BA_H_DCC && cache->spawned &&
      cache->powered && cache->health >= 0 )
    return qtrue;

  for( i = MAX_CLIENTS, ent = g_entities + i; i < level.num_entities; i++, ent++ )
  {
    if( ent->s.eType != ET_BUILDABLE )
      continue;

    if( ent->s.modelindex != BA_H_DCC )
      continue;

    if( !ent->spawned )
      continue;

    if( ent->health <= 0 )
      continue;
      
    if( !ent->powered )
      continue;

    cache = ent;

    return qtrue;
  }

  return qfalse;
}


/*
================
G_IsRTGBuilt

See if any RTG exists
================
*/
qboolean G_IsRTGBuilt( void )
{
  int       i;
  gentity_t *ent;
  static gentity_t *cache = NULL;
  
  if( cache && cache->inuse && cache->s.eType == ET_BUILDABLE &&
      cache->s.modelindex == BA_H_DCC && cache->spawned &&
      cache->health >= 0 )
    return qtrue;

  for( i = MAX_CLIENTS, ent = g_entities + i; i < level.num_entities; i++, ent++ )
  {
    if( !ent->r.linked )
      continue;
  
    if( ent->s.eType != ET_BUILDABLE )
      continue;

    if( ent->s.modelindex != BA_H_RTG )
      continue;

    if( !ent->spawned )
      continue;

    if( ent->health <= 0 )
      continue;

    cache = ent;

    return qtrue;
  }

  return qfalse;
}

/*
================
G_Overmind

Since there's only one overmind and we quite often want to find it, cache the
results, but check it for validity each time

The code here will break if more than one overmind is allowed, even
if one of them is dead/unspawned
================
*/
static gentity_t *G_FindBuildable( buildable_t buildable ); 

gentity_t *G_Overmind( void )
{
  static gentity_t *om;

  // If cache becomes invalid renew it
  if( !om || om->s.eType != ET_BUILDABLE || om->s.modelindex != BA_A_OVERMIND )
    om = G_FindBuildable( BA_A_OVERMIND );

  // If we found it and it's alive, return it
  if( om && om->spawned && om->health > 0 )
    return om;

  return NULL;
}

/*
================
G_FindCreep

attempt to find creep for self, return qtrue if successful
================
*/
qboolean G_FindCreep( gentity_t *self )
{
  int       i;
  gentity_t *ent;
  gentity_t *closestSpawn = NULL;
  int       distance = 0;
  int       minDistance = 10000;
  vec3_t    temp_v;

  //don't check for creep if flying through the air
  if( self->s.groundEntityNum == -1 )
    return qtrue;

  //if self does not have a parentNode or it's parentNode is invalid find a new one
  if( self->client || self->parentNode == NULL || !self->parentNode->inuse ||
      self->parentNode->health <= 0 )
  {
    for( i = MAX_CLIENTS, ent = g_entities + i; i < level.num_entities; i++, ent++ )
    {
      if( ent->s.eType != ET_BUILDABLE )
        continue;

      if( ( ent->s.modelindex == BA_A_SPAWN || 
            ent->s.modelindex == BA_A_OVERMIND ) &&
          ent->spawned && ent->health > 0 )
      {
        VectorSubtract( self->s.origin, ent->s.origin, temp_v );
        distance = VectorLength( temp_v );
        if( distance < minDistance )
        {
          closestSpawn = ent;
          minDistance = distance;
        }
      }
    }

    if( minDistance <= CREEP_BASESIZE )
    {
      if( !self->client )
        self->parentNode = closestSpawn;
      return qtrue;
    }
    else
      return qfalse;
  }

  if( self->client )
    return qfalse;

  //if we haven't returned by now then we must already have a valid parent
  return qtrue;
}

/*
================
G_IsCreepHere

simple wrapper to G_FindCreep to check if a location has creep
================
*/
static qboolean G_IsCreepHere( vec3_t origin )
{
  gentity_t dummy;

  memset( &dummy, 0, sizeof( gentity_t ) );

  dummy.parentNode = NULL;
  dummy.s.modelindex = BA_NONE;
  VectorCopy( origin, dummy.s.origin );

  return G_FindCreep( &dummy );
}

/*
================
G_CreepSlow

Set any nearby humans' SS_CREEPSLOWED flag
================
*/
static void G_CreepSlow( gentity_t *self )
{
  int         entityList[ MAX_GENTITIES ];
  vec3_t      range;
  vec3_t      mins, maxs;
  int         i, num;
  gentity_t   *enemy;
  buildable_t buildable = self->s.modelindex;
  float       creepSize = (float)BG_Buildable( buildable, self->cuboidSize )->creepSize;

  VectorSet( range, creepSize, creepSize, creepSize );

  VectorAdd( self->s.origin, range, maxs );
  VectorSubtract( self->s.origin, range, mins );

  //find humans
  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
  for( i = 0; i < num; i++ )
  {
    enemy = &g_entities[ entityList[ i ] ];

   if( enemy->flags & FL_NOTARGET )
     continue;

    if( enemy->client && enemy->client->ps.stats[ STAT_TEAM ] == TEAM_HUMANS &&
        enemy->client->ps.groundEntityNum != ENTITYNUM_NONE )
    {
      enemy->client->ps.stats[ STAT_STATE ] |= SS_CREEPSLOWED;
      enemy->client->lastCreepSlowTime = level.time;
    }
  }
}

/*
================
G_ScanPowerGrid
 
Recursively finds all power entities reachable from the specified entity
================
*/
static struct
{
  int networkID;
  
  gentity_t *load[ MAX_GENTITIES ];
  int loadCount;
  
  gentity_t *sources[ MAX_GENTITIES ];
  int sourceCount;
  
  qboolean visited[ MAX_GENTITIES ];
} grid;

void G_ScanPowerGrid( gentity_t *this )
{
  int i;
  int nextList[ MAX_GENTITIES ], nextCount;
  gentity_t *next;
  vec3_t mins, maxs;
  float range;

  switch( this->s.modelindex )
  {
    case BA_H_REACTOR:
    case BA_H_CAPBANK:
    case BA_H_RTG:
      range = REACTOR_BASESIZE;
      break;
    case BA_H_REPEATER:
      range = REPEATER_BASESIZE;
      break;
  }

  for( i = 0; i < 3; i++ )
  {
    mins[ i ] = this->s.origin[ i ] - range;
    maxs[ i ] = this->s.origin[ i ] + range;
  }
  
  nextCount = trap_EntitiesInBox( mins, maxs, nextList, MAX_GENTITIES );
  
  for( i = 0; i < nextCount; i++ )
  {
    if( grid.visited[ nextList[ i ] ] )
      continue;
    grid.visited[ nextList[ i ] ] = qtrue;
    
    next = g_entities + nextList[ i ];
    
    if( next->s.eType != ET_BUILDABLE )
    {
      //let ckits know in which network's range they're in
      if( next->client &&
          next->health >= 0 &&
          next->s.weapon == WP_HBUILD )
        next->powerNetwork = grid.networkID;
      continue;
    }

    if( next->health <= 0 )
      continue;
    if( next->buildableTeam != TEAM_HUMANS )
      continue;
      
    //repeater just extends the power grid
    //it does not provide or consume power
    if( next->spawned && next->s.modelindex == BA_H_REPEATER )
    {
      //switched off
      if( !next->active )
        continue;
    
      next->powerNetwork = grid.networkID;
      G_ScanPowerGrid( next );
      continue;
    }

    if( !next->isPowerSource && !next->requiresPower )
      continue;
      
    next->powerNetwork = grid.networkID;
    if( next->isPowerSource )
    {
      //switched off
      if( !next->active )
        continue;
      grid.sources[ grid.sourceCount++ ] = next;

      next->powerNetwork = grid.networkID;
      G_ScanPowerGrid( next );
    }
    else
      grid.load[ grid.loadCount++ ] = next;
  }
}


/*
================
G_CalculatePowerGrid

This function takes the listed power sources (with voltages V and 
internal resistances Rs) and load (with resistances R), constructs the
following electrical circuit and solves it:

  Legend:
 -(-+)--  voltage source
 -/\/\/-  resistor 
   |-     ground

        V1    Rs1   I1         R1
    +--(-+)--/\/\/-->-+     +--/\/\/--+
    |   V2    Rs2   I2|  I  |    R2   |
 |--+--(-+)--/\/\/-->-+-->--+--/\/\/--+--|
    |                 |     |         |
   ...               ...   ...       ...
    |   Vn    Rsn   In|     |    Rn   |
    +--(-+)--/\/\/-->-+     +--/\/\/--+

================
*/
void G_CalculatePowerGrid( void )
{
  int i, j;
  gentity_t *ent;

  float nGl, nGs, nG, nR;
  float URs, U2, Is;
  
  // the net load conductance (resistors in parallel)
  for( nGl = 0.0f, i = 0; i < grid.loadCount; i++ )
    nGl += 1.0f / grid.load[ i ]->resistance;
  
  // the net source conductance (resistors in parallel)
  for( nGs = 0.0f, i = 0; i < grid.sourceCount; i++ )
    nGs += 1.0f / grid.sources[ i ]->resistance;
  
  // solve the circuit using the superposition theorem
  for( i = 0; i < grid.sourceCount; i++ )
  {
    ent = grid.sources[ i ];
  
    // net load + other sources conductance
    nG = nGl + nGs - 1.0f / ent->resistance;
    
    // net resistance
    nR = 1.0f / nG + ent->resistance;
    
    // current flowing through the source
    Is = ent->voltage / nR;
    ent->current += ent->voltage / nR;

    // voltage drop on source's internal resistance
    URs = Is * ent->resistance;
    
    // voltage drop on other sources or the entire load
    U2 = ent->voltage - URs;
    
    // current flowing through other sources
    for( j = 0; j < grid.sourceCount; j++ )
      if( i != j )
        grid.sources[ j ]->current -= U2 / grid.sources[ j ]->resistance;

    // current flowing through parts of the load
    for( j = 0; j < grid.loadCount; j++ )
      grid.load[ j ]->current += U2 / grid.load[ j ]->resistance;
  }
}

/*
================
G_UpdatePowerGrid

Recalculate the entire power grid
================
*/
void G_UpdatePowerGrid( float dt )
{
  int i;
  gentity_t *ent;

  // reset all ckits
  for( i = 0; i < MAX_CLIENTS; i++ )
    g_entities[ i ].powerNetwork = 0;

  // reset all power entities
  for( i = MAX_CLIENTS; i < level.num_entities; i++ )
  {
    ent = g_entities + i;
    
    if( ent->s.eType != ET_BUILDABLE )
      continue;
    if( ent->buildableTeam != TEAM_HUMANS )
      continue;

    ent->powerNetwork = 0;
    ent->current = 0.0f;
    
    if( !ent->spawned )
    {
      ent->resistance = PREBUILD_RESISTANCE;
      continue;
    }
    
    if( ent->s.modelindex == BA_H_REACTOR ||
        ent->s.modelindex == BA_H_RTG )
      ent->voltage = POWER_VOLTAGE * g_voltageModifier.value;
    
    if( !ent->requiresPower || ent->isPowerSource )
      continue;

    if( ent->surge )
      ent->resistance = BG_Buildable( ent->s.modelindex, NULL )->surgeResistance;
    else
      ent->resistance = BG_Buildable( ent->s.modelindex, NULL )->resistance;
  }

  // this table will be used throughout the following loop and its recursive calls
  memset( grid.visited, 0, sizeof( grid.visited ) );
  
  // find an unvisited power source and make a list of all power sources
  // and receivers reachable from it for G_CalculatePowerGrid
  for( i = MAX_CLIENTS; i < level.num_entities; i++ )
  {
    if( grid.visited[ i ] )
      continue;
  
    ent = g_entities + i;
  
    if( ent->s.eType != ET_BUILDABLE )
      continue;
    if( !ent->spawned )
      continue;
    if( ent->health <= 0 )
      continue;
    if( ent->buildableTeam != TEAM_HUMANS )
      continue;
    if( !ent->isPowerSource )
      continue;
    if( !ent->active )
      continue;

    // unique network id
    grid.networkID = ent->s.number;
    ent->powerNetwork = grid.networkID;

    // traverse the world and find all reachable power entities
    grid.loadCount = 0;
    grid.sourceCount = 0;

    // add this source to the list
    grid.visited[ i ] = qtrue;
    grid.sources[ grid.sourceCount++ ] = ent;
    
    // scan recursively
    G_ScanPowerGrid( ent );

    // calculate the power grid
    G_CalculatePowerGrid( );
  }
  
  // calculate voltages, power levels, etc.
  for( i = MAX_CLIENTS; i < level.num_entities; i++ )
  {
    ent = g_entities + i;

    if( ent->s.eType != ET_BUILDABLE )
      continue;
    if( ent->buildableTeam != TEAM_HUMANS )
      continue;

    if( ent->isPowerSource )
    {
      if( ent->active && ent->s.modelindex == BA_H_CAPBANK )
        ent->voltage -= ent->current * dt / CAPBANK_CAPACITY;

      //zapping effect
      #define MIN_ZAP_CURRENT 15.0f
      #define ZAP_CHANCE_FACTOR 0.001f
      if( ent->current > MIN_ZAP_CURRENT )
      {
        float chance;
        
        chance = ( ent->current - MIN_ZAP_CURRENT ) * ZAP_CHANCE_FACTOR;
        if( chance > 0.2f )
          chance = 0.2f;

        chance = 1.0f - chance;
        if( random() > chance )
          G_AddEvent( ent, EV_POWER_ZAP, 0 );
      }
    }
    else
      ent->voltage = ent->current * ent->resistance;
  }
}

/*
================
G_SetupPowerEntity

Called when a Human buildable finishes spawning and needs power grid
related variables to be set accordingly
================
*/
void G_SetupPowerEntity( gentity_t *built )
{
  built->requiresPower = BG_Buildable( built->s.modelindex, NULL )->requiresPower;
  built->isPowerSource = BG_Buildable( built->s.modelindex, NULL )->isPowerSource;
  built->resistance = BG_Buildable( built->s.modelindex, NULL )->resistance;

  if( built->isPowerSource )
  {
    switch( built->s.modelindex )
    {
      case BA_H_REACTOR:
        built->resistance = REACTOR_RESISTANCE;
        break;
      case BA_H_CAPBANK:
        built->voltage = 0.0f; //spawn discharged
        built->resistance = CAPBANK_RESISTANCE;
      case BA_H_RTG:
        built->resistance = RTG_RESISTANCE;
        break;
    }
  }
}

/*
================
G_PowerForPoint

Returns to which network ID this point belongs
================
*/

int G_PowerForPoint( vec3_t point )
{
  int i;
  int list[ MAX_GENTITIES ], count;
  gentity_t *ent;
  vec3_t mins, maxs;
  float range;
/*
  switch( this->s.modelindex )
  {

  }*/
  range = REACTOR_BASESIZE;

  for( i = 0; i < 3; i++ )
  {
    mins[ i ] = point[ i ] - range;
    maxs[ i ] = point[ i ] + range;
  }
  
  count = trap_EntitiesInBox( mins, maxs, list, MAX_GENTITIES );
  
  for( i = 0; i < count; i++ )
  {
    ent = g_entities + list[ i ];

    if( ent->s.eType != ET_BUILDABLE )
      continue;
    if( ent->buildableTeam != TEAM_HUMANS )
      continue;
    if( ent->health <= 0 )
      continue;
    if( !ent->spawned )
      continue;
    if( !ent->powerNetwork )
      continue;
    
    switch( ent->s.modelindex )
    {
      case BA_H_REACTOR:
      case BA_H_CAPBANK:
      case BA_H_RTG:
        range = REACTOR_BASESIZE;
        break;
      case BA_H_REPEATER:
        range = REPEATER_BASESIZE;
        break;
      default:
        continue;
    }
    
    if( Distance( ent->s.origin, point ) > range )
      continue;

    return ent->powerNetwork;
  }
  return 0;
}

/*
================
NOTES TO G_CheckPower AND G_Surge

Make sure that a buildable's resistance NEVER depends on its power
state. Failure to ensure that will result in feedback loops in the
power grid and general weirdness.
================
*/

/*
================
G_CheckPower

(Helper for Human buildable think functions)
Checks if there's enough power for the buildable to idle
================
*/
qboolean G_CheckPower( gentity_t *self, float min_current )
{
  self->surge = qfalse;
  self->surgePowered = qfalse;
  self->powered = qtrue;
  
  if( self->current < min_current )
  {
    self->powered = qfalse;
    return qfalse;
  }
  
  return qtrue;
}

/*
================
G_Surge

(Helper for Human buildable think functions)
Checks if there's enough power for the buildable to perform its action
================
*/
qboolean G_Surge( gentity_t *self, float surge_current )
{
  self->surge = qtrue;
  
  if( self->current < surge_current )
    return qfalse;
    
  self->surgePowered = qtrue;
  return qtrue;
}

/*
================
nullDieFunction

hack to prevent compilers complaining about function pointer -> NULL conversion
================
*/
static void nullDieFunction( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
}

//==================================================================================



/*
================
AGeneric_CreepRecede

Called when an alien buildable dies
================
*/
void AGeneric_CreepRecede( gentity_t *self )
{
  //if the creep just died begin the recession
  if( !( self->s.eFlags & EF_DEAD ) )
  {
    self->s.eFlags |= EF_DEAD;
    G_QueueBuildPoints( self );

    G_RewardAttackers( self );

    G_AddEvent( self, EV_BUILD_DESTROY, 0 );

    if( self->spawned )
      self->s.time = -level.time;
    else
      self->s.time = -( level.time -
          (int)( (float)CREEP_SCALEDOWN_TIME *
                 ( 1.0f - ( (float)( level.time - self->buildTime ) /
                            (float)BG_Buildable( self->s.modelindex, self->cuboidSize )->buildTime ) ) ) );
  }

  //creep is still receeding
  if( ( self->timestamp + 10000 ) > level.time )
    self->nextthink = level.time + 500;
  else //creep has died
    G_FreeEntity( self );
}

/*
================
AGeneric_Blast

Called when an Alien buildable explodes after dead state
================
*/
void AGeneric_Blast( gentity_t *self )
{
  vec3_t dir;

  VectorCopy( self->s.origin2, dir );

  //do a bit of radius damage
  G_SelectiveRadiusDamage( self->s.pos.trBase, g_entities + self->killedBy, self->splashDamage,
                           self->splashRadius, self, self->splashMethodOfDeath,
                           TEAM_ALIENS );

  //pretty events and item cleanup
  self->s.eFlags |= EF_NODRAW; //don't draw the model once it's destroyed
  G_AddEvent( self, EV_ALIEN_BUILDABLE_EXPLOSION, DirToByte( dir ) );
  self->timestamp = level.time;
  self->think = AGeneric_CreepRecede;
  self->nextthink = level.time + 500;

  self->r.contents = 0;    //stop collisions...
  trap_LinkEntity( self ); //...requires a relink
}

/*
================
AGeneric_Die

Called when an Alien buildable is killed and enters a brief dead state prior to
exploding.
================
*/
void AGeneric_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  G_SetBuildableAnim( self, BANIM_DESTROY1, qtrue );
  G_SetIdleBuildableAnim( self, BANIM_DESTROYED );

  self->die = nullDieFunction;
  self->killedBy = attacker - g_entities;
  self->think = AGeneric_Blast;
  self->s.eFlags &= ~EF_FIRING; //prevent any firing effects
  self->powered = qfalse;

  if( self->spawned )
    self->nextthink = level.time + 5000;
  else
    self->nextthink = level.time; //blast immediately

  G_LogDestruction( self, attacker, mod );
}

/*
================
AGeneric_CreepCheck

Tests for creep and kills the buildable if there is none
================
*/
void AGeneric_CreepCheck( gentity_t *self )
{
  gentity_t *spawn;

  spawn = self->parentNode;
  if( !G_FindCreep( self ) )
  {
    if( spawn )
      G_Damage( self, NULL, g_entities + spawn->killedBy, NULL, NULL,
                self->health, 0, MOD_NOCREEP );
    else
      G_Damage( self, NULL, NULL, NULL, NULL, self->health, 0, MOD_NOCREEP );
    return;
  }
  G_CreepSlow( self );
}

/*
================
AGeneric_Think

A generic think function for Alien buildables
================
*/
void AGeneric_Think( gentity_t *self )
{
  self->powered = G_Overmind( ) != NULL;
  self->nextthink = level.time + BG_Buildable( self->s.modelindex, NULL )->nextthink;
  AGeneric_CreepCheck( self );
}

/*
================
AGeneric_Pain

A generic pain function for Alien buildables
================
*/
void AGeneric_Pain( gentity_t *self, gentity_t *attacker, int damage )
{
  if( self->health <= 0 )
    return;
    
  // Alien buildables only have the first pain animation defined
  G_SetBuildableAnim( self, BANIM_PAIN1, qfalse );
}




//==================================================================================

/*
================
ASpawn_Think

think function for Alien Spawn
================
*/
void ASpawn_Think( gentity_t *self )
{
  gentity_t *ent;

  if( self->spawned )
  {
    //only suicide if at rest
    if( self->s.groundEntityNum )
    {
      if( ( ent = G_CheckSpawnPoint( self->s.number, self->s.origin,
              self->s.origin2, BA_A_SPAWN, NULL ) ) != NULL )
      {
        // If the thing blocking the spawn is a buildable, kill it. 
        // If it's part of the map, kill self. 
        if( ent->s.eType == ET_BUILDABLE )
        {
          if( ent->builtBy >= 0 ) // don't queue the bp from this
            G_Damage( ent, NULL, g_entities + ent->builtBy, NULL, NULL, 10000, 0, MOD_SUICIDE );
          else
            G_Damage( ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );

          G_SetBuildableAnim( self, BANIM_SPAWN1, qtrue );
        }
        else if( ent->s.number == ENTITYNUM_WORLD || ent->s.eType == ET_MOVER )
        {
          G_Damage( self, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
          return;
        }

        if( ent->s.eType == ET_CORPSE )
          G_FreeEntity( ent ); //quietly remove
      }
    }
  }

  G_CreepSlow( self );

  self->nextthink = level.time + BG_Buildable( self->s.modelindex, NULL )->nextthink;
}





//==================================================================================





#define OVERMIND_ATTACK_PERIOD 10000
#define OVERMIND_DYING_PERIOD  5000
#define OVERMIND_SPAWNS_PERIOD 30000

/*
================
AOvermind_Think

Think function for Alien Overmind
================
*/
void AOvermind_Think( gentity_t *self )
{
  vec3_t range = { OVERMIND_ATTACK_RANGE, OVERMIND_ATTACK_RANGE, OVERMIND_ATTACK_RANGE };
  int    i;

  if( self->spawned && ( self->health > 0 ) )
  {
    //do some damage
    if( G_SelectiveRadiusDamage( self->s.pos.trBase, self, self->splashDamage,
          self->splashRadius, self, MOD_OVERMIND, TEAM_ALIENS ) )
    {
      self->timestamp = level.time;
      G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );
    }

    // just in case an egg finishes building after we tell overmind to stfu
    if( level.numAlienSpawns > 0 )
      level.overmindMuted = qfalse;

    // shut up during intermission
    if( level.intermissiontime )
      level.overmindMuted = qtrue;

    //low on spawns
    if( !level.overmindMuted && level.numAlienSpawns <= 0 &&
        level.time > self->overmindSpawnsTimer )
    {
      qboolean haveBuilder = qfalse;
      gentity_t *builder;

      self->overmindSpawnsTimer = level.time + OVERMIND_SPAWNS_PERIOD;
      G_BroadcastEvent( EV_OVERMIND_SPAWNS, 0 );

      for( i = 0; i < level.numConnectedClients; i++ )
      {
        builder = &g_entities[ level.sortedClients[ i ] ];
        if( builder->health > 0 &&
          ( builder->client->pers.classSelection == PCL_ALIEN_BUILDER0 ||
            builder->client->pers.classSelection == PCL_ALIEN_BUILDER0_UPG ) )
        {
          haveBuilder = qtrue;
          break;
        }
      }
      // aliens now know they have no eggs, but they're screwed, so stfu
      if( !haveBuilder || G_TimeTilSuddenDeath( ) <= 0 )
        level.overmindMuted = qtrue;
    }

    //overmind dying
    if( self->health < ( OVERMIND_HEALTH / 10.0f ) && level.time > self->overmindDyingTimer )
    {
      self->overmindDyingTimer = level.time + OVERMIND_DYING_PERIOD;
      G_BroadcastEvent( EV_OVERMIND_DYING, 0 );
    }

    //overmind under attack
    if( self->health < self->lastHealth && level.time > self->overmindAttackTimer )
    {
      self->overmindAttackTimer = level.time + OVERMIND_ATTACK_PERIOD;
      G_BroadcastEvent( EV_OVERMIND_ATTACK, 0 );
    }

    self->lastHealth = self->health;
  }
  else
    self->overmindSpawnsTimer = level.time + OVERMIND_SPAWNS_PERIOD;

  G_CreepSlow( self );

  self->nextthink = level.time + BG_Buildable( self->s.modelindex, NULL )->nextthink;
}





//==================================================================================





/*
================
ABarricade_Pain

Barricade pain animation depends on shrunk state
================
*/
void ABarricade_Pain( gentity_t *self, gentity_t *attacker, int damage )
{
  if( self->health <= 0 )
    return;

  if( !self->shrunkTime )
    G_SetBuildableAnim( self, BANIM_PAIN1, qfalse );
  else
    G_SetBuildableAnim( self, BANIM_PAIN2, qfalse );
}

/*
================
ABarricade_Shrink

Set shrink state for a barricade. When unshrinking, checks to make sure there
is enough room.
================
*/
void ABarricade_Shrink( gentity_t *self, qboolean shrink )
{
  if ( !self->spawned || self->health <= 0 )
    shrink = qtrue;
  if ( shrink && self->shrunkTime )
  {
    int anim;

    // We need to make sure that the animation has been set to shrunk mode
    // because we start out shrunk but with the construct animation when built
    self->shrunkTime = level.time;
    anim = self->s.torsoAnim & ~( ANIM_FORCEBIT | ANIM_TOGGLEBIT );
    if ( self->spawned && self->health > 0 && anim != BANIM_DESTROYED )
    {
      G_SetIdleBuildableAnim( self, BANIM_DESTROYED );
      G_SetBuildableAnim( self, BANIM_ATTACK1, qtrue );
    }
    return;
  }

  if ( !shrink && ( !self->shrunkTime ||
       level.time < self->shrunkTime + BARRICADE_SHRINKTIMEOUT ) )
    return;

  BG_BuildableBoundingBox( BA_A_BARRICADE, self->r.mins, self->r.maxs );

  if ( shrink )
  {
    self->r.maxs[ 2 ] = (int)( self->r.maxs[ 2 ] * BARRICADE_SHRINKPROP );
    self->shrunkTime = level.time;

    // shrink animation, the destroy animation is used
    if ( self->spawned && self->health > 0 )
    {
      G_SetBuildableAnim( self, BANIM_ATTACK1, qtrue );
      G_SetIdleBuildableAnim( self, BANIM_DESTROYED );
    }
  }
  else
  {
    trace_t tr;
    int anim;

    trap_Trace( &tr, self->s.origin, self->r.mins, self->r.maxs,
                self->s.origin, self->s.number, MASK_PLAYERSOLID );
    if ( tr.startsolid || tr.fraction < 1.0f )
    {
      self->r.maxs[ 2 ] = (int)( self->r.maxs[ 2 ] * BARRICADE_SHRINKPROP );
      return;
    }
    self->shrunkTime = 0;

    // unshrink animation, IDLE2 has been hijacked for this
    anim = self->s.legsAnim & ~( ANIM_FORCEBIT | ANIM_TOGGLEBIT );
    if ( self->spawned && self->health > 0 &&
         anim != BANIM_CONSTRUCT1 && anim != BANIM_CONSTRUCT2 )
    {
      G_SetIdleBuildableAnim( self, BG_Buildable( BA_A_BARRICADE, NULL )->idleAnim );
      G_SetBuildableAnim( self, BANIM_ATTACK2, qtrue );
    }
  }

  // a change in size requires a relink
  if ( self->spawned )
    trap_LinkEntity( self );
}

/*
================
ABarricade_Die

Called when an alien barricade dies
================
*/
void ABarricade_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  AGeneric_Die( self, inflictor, attacker, damage, mod );
  ABarricade_Shrink( self, qtrue );
}

/*
================
ABarricade_Think

Think function for Alien Barricade
================
*/
void ABarricade_Think( gentity_t *self )
{
  AGeneric_Think( self );

  // Shrink if unpowered
  ABarricade_Shrink( self, !self->powered );
}

/*
================
ABarricade_Touch

Barricades shrink when they are come into contact with an Alien that can
pass through
================
*/

void ABarricade_Touch( gentity_t *self, gentity_t *other, trace_t *trace )
{
  gclient_t *client = other->client;
  int client_z, min_z;

  if( !client || client->pers.teamSelection != TEAM_ALIENS )
    return;

  // Client must be high enough to pass over. Note that STEPSIZE (18) is
  // hardcoded here because we don't include bg_local.h!
  client_z = other->s.origin[ 2 ] + other->r.mins[ 2 ];
  min_z = self->s.origin[ 2 ] - 18 +
          (int)( self->r.maxs[ 2 ] * BARRICADE_SHRINKPROP );
  if( client_z < min_z )
    return;
  ABarricade_Shrink( self, qtrue );
}

//==================================================================================




/*
================
AAcidTube_Think

Think function for Alien Acid Tube
================
*/
void AAcidTube_Think( gentity_t *self )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    range = { ACIDTUBE_RANGE, ACIDTUBE_RANGE, ACIDTUBE_RANGE };
  vec3_t    mins, maxs;
  int       i, num;
  gentity_t *enemy;

  AGeneric_Think( self );

  VectorAdd( self->s.origin, range, maxs );
  VectorSubtract( self->s.origin, range, mins );

  // attack nearby humans
  if( self->spawned && self->health > 0 && self->powered )
  {
    num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
    for( i = 0; i < num; i++ )
    {
      enemy = &g_entities[ entityList[ i ] ];

      if( enemy->flags & FL_NOTARGET )
        continue;

      if( !G_Visible( self, enemy, CONTENTS_SOLID ) )
        continue;

      if( enemy->client && enemy->client->ps.stats[ STAT_TEAM ] == TEAM_HUMANS )
      {
        // start the attack animation
        if( level.time >= self->timestamp + ACIDTUBE_REPEAT_ANIM )
        {
          self->timestamp = level.time;
          G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );
          G_AddEvent( self, EV_ALIEN_ACIDTUBE, DirToByte( self->s.origin2 ) );
        }
        
        G_SelectiveRadiusDamage( self->s.pos.trBase, self, ACIDTUBE_DAMAGE,
                                 ACIDTUBE_RANGE, self, MOD_ATUBE, TEAM_ALIENS );                           
        self->nextthink = level.time + ACIDTUBE_REPEAT;
        return;
      }
    }
  }
}




//==================================================================================

/*
================
AHive_CheckTarget

Returns true and fires the hive missile if the target is valid
================
*/
static qboolean AHive_CheckTarget( gentity_t *self, gentity_t *enemy )
{
  trace_t trace;
  vec3_t tip_origin, dirToTarget;

  // Check if this is a valid target
  if( enemy->health <= 0 || !enemy->client ||
      enemy->client->ps.stats[ STAT_TEAM ] != TEAM_HUMANS )
    return qfalse;

  if( enemy->flags & FL_NOTARGET )
    return qfalse;

  // Check if the tip of the hive can see the target
  VectorMA( self->s.pos.trBase, self->r.maxs[ 2 ], self->s.origin2,
            tip_origin );
  if( Distance( tip_origin, enemy->s.origin ) > HIVE_SENSE_RANGE )
    return qfalse;

  trap_Trace( &trace, tip_origin, NULL, NULL, enemy->s.pos.trBase,
              self->s.number, MASK_SHOT );
  if( trace.fraction < 1.0f && trace.entityNum != enemy->s.number )
    return qfalse;

  self->active = qtrue;
  self->target_ent = enemy;
  self->timestamp = level.time + HIVE_REPEAT;

  VectorSubtract( enemy->s.pos.trBase, self->s.pos.trBase, dirToTarget );
  VectorNormalize( dirToTarget );
  vectoangles( dirToTarget, self->turretAim );

  // Fire at target
  FireWeapon( self );
  G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );
  return qtrue;
}

/*
================
AHive_Think

Think function for Alien Hive
================
*/
void AHive_Think( gentity_t *self )
{
  int       start;

  AGeneric_Think( self );

  // Hive missile hasn't returned in HIVE_REPEAT seconds, forget about it
  if( self->timestamp < level.time )
    self->active = qfalse;

  // Find a target to attack
  if( self->spawned && !self->active && self->powered )
  {
    int i, num, entityList[ MAX_GENTITIES ];
    vec3_t mins, maxs,
           range = { HIVE_SENSE_RANGE, HIVE_SENSE_RANGE, HIVE_SENSE_RANGE };

    VectorAdd( self->s.origin, range, maxs );
    VectorSubtract( self->s.origin, range, mins );

    num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

    if( num == 0 )
      return;

    start = rand( ) / ( RAND_MAX / num + 1 );
    for( i = start; i < num + start; i++ )
    {
      if( AHive_CheckTarget( self, g_entities + entityList[ i % num ] ) )
        return;
    }
  }
}

/*
================
AHive_Pain

pain function for Alien Hive
================
*/
void AHive_Pain( gentity_t *self, gentity_t *attacker, int damage )
{
  if( self->spawned && self->powered && !self->active )
    AHive_CheckTarget( self, attacker );

  G_SetBuildableAnim( self, BANIM_PAIN1, qfalse );
}


//==================================================================================


/*
================
ABooster_Touch

Called when an alien touches a booster
================
*/
void ABooster_Touch( gentity_t *self, gentity_t *other, trace_t *trace )
{
  gclient_t *client = other->client;

  if( !self->spawned || !self->powered || self->health <= 0 )
    return;

  if( !client )
    return;

  if( client->ps.stats[ STAT_TEAM ] == TEAM_HUMANS )
    return;

  if( other->flags & FL_NOTARGET )
    return; // notarget cancels even beneficial effects?

  client->ps.stats[ STAT_STATE ] |= SS_BOOSTED;
  client->boostedTime = level.time;
  
  // give the rant bomb
  if( client->lastRantBombTime + LEVEL4_BOMB_REGEN <= level.time )
  {
    client->lastRantBombTime = level.time;
    client->ps.ammo = 1;
  }
}




//==================================================================================

#define TRAPPER_ACCURACY 10 // lower is better

/*
================
ATrapper_FireOnEnemy

Used by ATrapper_Think to fire at enemy
================
*/
void ATrapper_FireOnEnemy( gentity_t *self, int firespeed, float range )
{
  gentity_t *enemy = self->enemy;
  vec3_t    dirToTarget;
  vec3_t    halfAcceleration, thirdJerk;
  float     distanceToTarget = BG_Buildable( self->s.modelindex, NULL )->turretRange;
  int       lowMsec = 0;
  int       highMsec = (int)( (
    ( ( distanceToTarget * LOCKBLOB_SPEED ) +
      ( distanceToTarget * BG_Class( enemy->client->ps.stats[ STAT_CLASS ] )->speed ) ) /
    ( LOCKBLOB_SPEED * LOCKBLOB_SPEED ) ) * 1000.0f );

  VectorScale( enemy->acceleration, 1.0f / 2.0f, halfAcceleration );
  VectorScale( enemy->jerk, 1.0f / 3.0f, thirdJerk );

  // highMsec and lowMsec can only move toward
  // one another, so the loop must terminate
  while( highMsec - lowMsec > TRAPPER_ACCURACY )
  {
    int   partitionMsec = ( highMsec + lowMsec ) / 2;
    float time = (float)partitionMsec / 1000.0f;
    float projectileDistance = LOCKBLOB_SPEED * time;

    VectorMA( enemy->s.pos.trBase, time, enemy->s.pos.trDelta, dirToTarget );
    VectorMA( dirToTarget, time * time, halfAcceleration, dirToTarget );
    VectorMA( dirToTarget, time * time * time, thirdJerk, dirToTarget );
    VectorSubtract( dirToTarget, self->s.pos.trBase, dirToTarget );
    distanceToTarget = VectorLength( dirToTarget );

    if( projectileDistance < distanceToTarget )
      lowMsec = partitionMsec;
    else if( projectileDistance > distanceToTarget )
      highMsec = partitionMsec;
    else if( projectileDistance == distanceToTarget )
      break; // unlikely to happen
  }

  VectorNormalize( dirToTarget );
  vectoangles( dirToTarget, self->turretAim );

  //fire at target
  FireWeapon( self );
  G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );
  self->count = level.time + firespeed;
}

/*
================
ATrapper_CheckTarget

Used by ATrapper_Think to check enemies for validity
================
*/
qboolean ATrapper_CheckTarget( gentity_t *self, gentity_t *target, int range )
{
  vec3_t    distance;
  trace_t   trace;

  if( !target ) // Do we have a target?
    return qfalse;
  if( !target->inuse ) // Does the target still exist?
    return qfalse;
  if( target == self ) // is the target us?
    return qfalse;
  if( !target->client ) // is the target a bot or player?
    return qfalse;
  if( target->flags & FL_NOTARGET ) // is the target cheating?
    return qfalse;
  if( target->client->ps.stats[ STAT_TEAM ] == TEAM_ALIENS ) // one of us?
    return qfalse;
  if( target->client->sess.spectatorState != SPECTATOR_NOT ) // is the target alive?
    return qfalse;
  if( target->health <= 0 ) // is the target still alive?
    return qfalse;
  if( target->client->ps.stats[ STAT_STATE ] & SS_BLOBLOCKED ) // locked?
    return qfalse;

  VectorSubtract( target->r.currentOrigin, self->r.currentOrigin, distance );
  if( VectorLength( distance ) > range ) // is the target within range?
    return qfalse;

  //only allow a narrow field of "vision"
  VectorNormalize( distance ); //is now direction of target
  if( DotProduct( distance, self->s.origin2 ) < LOCKBLOB_DOT )
    return qfalse;

  trap_Trace( &trace, self->s.pos.trBase, NULL, NULL, target->s.pos.trBase, self->s.number, MASK_SHOT );
  if ( trace.contents & CONTENTS_SOLID ) // can we see the target?
    return qfalse;

  return qtrue;
}

/*
================
ATrapper_FindEnemy

Used by ATrapper_Think to locate enemy gentities
================
*/
void ATrapper_FindEnemy( gentity_t *ent, int range )
{
  gentity_t *target;
  int       i;
  int       start;

  // iterate through entities
  // note that if we exist then level.num_entities != 0
  start = rand( ) / ( RAND_MAX / level.num_entities + 1 );
  for( i = start; i < level.num_entities + start; i++ )
  {
    target = g_entities + ( i % level.num_entities );
    //if target is not valid keep searching
    if( !ATrapper_CheckTarget( ent, target, range ) )
      continue;

    //we found a target
    ent->enemy = target;
    return;
  }

  //couldn't find a target
  ent->enemy = NULL;
}

/*
================
ATrapper_Think

think function for Alien Defense
================
*/
void ATrapper_Think( gentity_t *self )
{
  int range =     BG_Buildable( self->s.modelindex, NULL )->turretRange;
  int firespeed = BG_Buildable( self->s.modelindex, NULL )->turretFireSpeed;

  AGeneric_Think( self );

  if( self->spawned && self->powered )
  {
    //if the current target is not valid find a new one
    if( !ATrapper_CheckTarget( self, self->enemy, range ) )
      ATrapper_FindEnemy( self, range );

    //if a new target cannot be found don't do anything
    if( !self->enemy )
      return;

    //if we are pointing at our target and we can fire shoot it
    if( self->count < level.time )
      ATrapper_FireOnEnemy( self, firespeed, range );
  }
}




//==================================================================================

/*
================
G_IdlePowerState

Set buildable idle animation to match power state
================
*/
static void G_IdlePowerState( gentity_t *self )
{
  if( self->powered )
  {
    if( self->s.torsoAnim == BANIM_IDLE3 )
      G_SetIdleBuildableAnim( self, BG_Buildable( self->s.modelindex, NULL )->idleAnim );
  }
  else
  {
    if( self->s.torsoAnim != BANIM_IDLE3 )
      G_SetIdleBuildableAnim( self, BANIM_IDLE3 );
  }
}


//==================================================================================


/*
================
HSpawn_Disappear

Called when a human spawn is destroyed before it is spawned
think function
================
*/
void HSpawn_Disappear( gentity_t *self )
{
  self->s.eFlags |= EF_NODRAW; //don't draw the model once its destroyed
  self->timestamp = level.time;
  G_QueueBuildPoints( self );
  G_RewardAttackers( self );

  G_FreeEntity( self );
}


/*
================
HSpawn_blast

Called when a human spawn explodes
think function
================
*/
void HSpawn_Blast( gentity_t *self )
{
  vec3_t  dir;

  // we don't have a valid direction, so just point straight up
  dir[ 0 ] = dir[ 1 ] = 0;
  dir[ 2 ] = 1;

  self->timestamp = level.time;

  //do some radius damage
  G_RadiusDamage( self->s.pos.trBase, g_entities + self->killedBy, self->splashDamage,
    self->splashRadius, self, self->splashMethodOfDeath );

  // begin freeing build points
  G_QueueBuildPoints( self );
  G_RewardAttackers( self );
  // turn into an explosion
  self->s.eType = ET_EVENTS + EV_HUMAN_BUILDABLE_EXPLOSION;
  self->freeAfterEvent = qtrue;
  G_AddEvent( self, EV_HUMAN_BUILDABLE_EXPLOSION, DirToByte( dir ) );
}


/*
================
HSpawn_die

Called when a human spawn dies
================
*/
void HSpawn_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  G_SetBuildableAnim( self, BANIM_DESTROY1, qtrue );
  G_SetIdleBuildableAnim( self, BANIM_DESTROYED );

  self->die = nullDieFunction;
  self->killedBy = attacker - g_entities;
  self->s.eFlags &= ~EF_FIRING; //prevent any firing effects

  if( self->spawned )
  {
    self->think = HSpawn_Blast;
    self->nextthink = level.time + HUMAN_DETONATION_DELAY;
  }
  else
  {
    self->think = HSpawn_Disappear;
    self->nextthink = level.time; //blast immediately
  }

  G_LogDestruction( self, attacker, mod );
}

/*
================
HSpawn_Think

Think for human spawn
================
*/
void HSpawn_Think( gentity_t *self )
{
  gentity_t *ent;

  if( !self->spawned || self->health <= 0 )
    return;
    
  //only suicide if at rest
  if( self->s.groundEntityNum )
  {
    if( ( ent = G_CheckSpawnPoint( self->s.number, self->s.origin,
            self->s.origin2, BA_H_SPAWN, NULL ) ) != NULL )
    {
      // If the thing blocking the spawn is a buildable, kill it. 
      // If it's part of the map, kill self. 
      if( ent->s.eType == ET_BUILDABLE )
      {
        G_Damage( ent, NULL, NULL, NULL, NULL, self->health, 0, MOD_SUICIDE );
        G_SetBuildableAnim( self, BANIM_SPAWN1, qtrue );
      }
      else if( ent->s.number == ENTITYNUM_WORLD || ent->s.eType == ET_MOVER )
      {
        G_Damage( self, NULL, NULL, NULL, NULL, self->health, 0, MOD_SUICIDE );
        return;
      }

      if( ent->s.eType == ET_CORPSE )
        G_FreeEntity( ent ); //quietly remove
    }
  }

  self->nextthink = level.time + BG_Buildable( self->s.modelindex, NULL )->nextthink;
}




//==================================================================================

/*
================
HRepeater_Think

Think function for Human Repeater
================
*/

void HRepeater_Think( gentity_t *self )
{
  self->nextthink = level.time + BG_Buildable( self->s.modelindex, NULL )->nextthink;

  if( !self->spawned || self->health <= 0 )
    return;

  self->powered = self->active && ( self->powerNetwork != 0 );

  G_IdlePowerState( self );
}


/*
================
HRepeater_Die

Called when a repeater dies
================
*/
static void HRepeater_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  G_SetBuildableAnim( self, BANIM_DESTROY1, qtrue );
  G_SetIdleBuildableAnim( self, BANIM_DESTROYED );

  self->die = nullDieFunction;
  self->killedBy = attacker - g_entities;
  self->s.eFlags &= ~EF_FIRING; //prevent any firing effects

  if( self->spawned )
  {
    self->think = HSpawn_Blast;
    self->nextthink = level.time + HUMAN_DETONATION_DELAY;
  }
  else
  {
    self->think = HSpawn_Disappear;
    self->nextthink = level.time; //blast immediately
  }

  G_LogDestruction( self, attacker, mod );
}

/*
================
HSwitchable_Use

Use for switchable buildings
================
*/
void HSwitchable_Use( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  if( !self->spawned || self->health <= 0 )
    return;

  if( !other || !other->client )
    return;
  
  // ckits and blasters switch the building
  if( other->s.weapon == WP_HBUILD ||
      other->s.weapon == WP_BLASTER )
  {
    self->active ^= 1;
    G_AddEvent( self, EV_POWER_SWITCH, 0 );
    return;
  }

  // not powered
  if( !self->powerNetwork )
    return;

  if( other && other->client )
    G_GiveClientMaxAmmo( other, qtrue );
}

/*
================
HReactor_Think

Think function for Human Reactor
================
*/
void HReactor_Think( gentity_t *self )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    range = { REACTOR_ATTACK_RANGE,
                      REACTOR_ATTACK_RANGE,
                      REACTOR_ATTACK_RANGE };
  vec3_t    dccrange = { REACTOR_ATTACK_DCC_RANGE,
                         REACTOR_ATTACK_DCC_RANGE,
                         REACTOR_ATTACK_DCC_RANGE };
  vec3_t    mins, maxs;
  int       i, num;
  gentity_t *enemy, *tent;
  qboolean fired = qfalse;

  if( !self->spawned || self->health <= 0 )
    return;

  self->powered = self->active;
  
  G_IdlePowerState( self );

  if( G_IsDCCBuilt( ) )
  {
    VectorAdd( self->s.origin, dccrange, maxs );
    VectorSubtract( self->s.origin, dccrange, mins );
  }
  else
  {
    VectorAdd( self->s.origin, range, maxs );
    VectorSubtract( self->s.origin, range, mins );
  }

  // Creates a tesla trail for every target
  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
  for( i = 0; i < num; i++ )
  {
    enemy = &g_entities[ entityList[ i ] ];
    if( !enemy->client ||
        enemy->client->ps.stats[ STAT_TEAM ] != TEAM_ALIENS )
      continue;
    if( enemy->flags & FL_NOTARGET )
      continue;

    tent = G_TempEntity( enemy->s.pos.trBase, EV_TESLATRAIL );
    tent->s.generic1 = self->s.number; //src
    tent->s.clientNum = enemy->s.number; //dest
    VectorCopy( self->s.pos.trBase, tent->s.origin2 );
    fired = qtrue;
  }

  // Actual damage is done by radius
  if( fired )
  {
    self->timestamp = level.time;
    if( G_IsDCCBuilt( ) )
      G_SelectiveRadiusDamage( self->s.pos.trBase, self,
                               REACTOR_ATTACK_DCC_DAMAGE,
                               REACTOR_ATTACK_DCC_RANGE, self,
                               MOD_REACTOR, TEAM_HUMANS );
    else
      G_SelectiveRadiusDamage( self->s.pos.trBase, self,
                               REACTOR_ATTACK_DAMAGE,
                               REACTOR_ATTACK_RANGE, self,
                               MOD_REACTOR, TEAM_HUMANS );
  }

  if( G_IsDCCBuilt( ) )
    self->nextthink -= REACTOR_ATTACK_DCC_REPEAT - REACTOR_ATTACK_REPEAT;
}

//==================================================================================

/*
================
HArmoury_Think
* 
Think function for Human Armoury
================
*/
void HArmoury_Think( gentity_t *self )
{
  self->nextthink = level.time + BG_Buildable( self->s.modelindex, NULL )->nextthink;

  if( !self->spawned )
    return;

  G_CheckPower( self, ARMOURY_CURRENT );
}

/*
================
HArmoury_Activate

Called when a human activates an Armoury
================
*/
void HArmoury_Activate( gentity_t *self, gentity_t *other, gentity_t *activator )
{
  if( self->spawned )
  {
    //only humans can activate this
    if( activator->client->ps.stats[ STAT_TEAM ] != TEAM_HUMANS )
      return;

    //if this is powered then call the armoury menu
    if( self->powered )
      G_TriggerMenu( activator->client->ps.clientNum, MN_H_ARMOURY );
    else
      G_TriggerMenu( activator->client->ps.clientNum, MN_H_NOTPOWERED );
  }
}

//==================================================================================


/*
================
HMedistat_Die

Die function for Human Medistation
================
*/
void HMedistat_Die( gentity_t *self, gentity_t *inflictor,
                    gentity_t *attacker, int damage, int mod )
{
  //clear target's healing flag
  if( self->enemy && self->enemy->client )
    self->enemy->client->ps.stats[ STAT_STATE ] &= ~SS_HEALING_ACTIVE;

  HSpawn_Die( self, inflictor, attacker, damage, mod );
}

/*
================
HMedistat_Think

think function for Human Medistation
================
*/
void HMedistat_Think( gentity_t *self )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    mins, maxs;
  int       i, num;
  gentity_t *player;
  qboolean  occupied = qfalse;

  self->nextthink = level.time + BG_Buildable( self->s.modelindex, NULL )->nextthink;
  
  if( !self->spawned || self->health <= 0 )
    return;

  G_CheckPower( self, MEDISTAT_I_IDLE );

  G_IdlePowerState( self );

  //clear target's healing flag
  if( self->enemy && self->enemy->client )
    self->enemy->client->ps.stats[ STAT_STATE ] &= ~SS_HEALING_ACTIVE;

  //make sure we have power
  if( !self->powered )
  {
    if( self->active )
    {
      self->active = qfalse;
      self->enemy = NULL;
    }
  }

  VectorAdd( self->s.origin, self->r.maxs, maxs );
  VectorAdd( self->s.origin, self->r.mins, mins );

  mins[ 2 ] += fabs( self->r.mins[ 2 ] ) + self->r.maxs[ 2 ];
  maxs[ 2 ] += 60; //player height

  //if active use the healing idle
  if( self->active )
    G_SetIdleBuildableAnim( self, BANIM_IDLE2 );
    
  //check if a previous occupier is still here
  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
  for( i = 0; i < num; i++ )
  {
    player = &g_entities[ entityList[ i ] ];

    if( player->flags & FL_NOTARGET )
      continue; // notarget cancels even beneficial effects?
    
    //remove poison from everyone, not just the healed player
    if( player->client && player->client->ps.stats[ STAT_STATE ] & SS_POISONED )
      player->client->ps.stats[ STAT_STATE ] &= ~SS_POISONED;

    if( self->enemy == player && player->client &&
        ( ( player->client->ps.stats[ STAT_TEAM ] == TEAM_HUMANS ) && 
        player->health < player->client->ps.stats[ STAT_MAX_HEALTH ] &&
        PM_Live( player->client->ps.pm_type ) ) )
    {
      if( !G_Surge( self, MEDISTAT_I_ACTIVE ) )
        return;

      occupied = qtrue;
      player->client->ps.stats[ STAT_STATE ] |= SS_HEALING_ACTIVE;
    }
  }

  if( !occupied )
  {
    self->enemy = NULL;

    //look for something to heal
    for( i = 0; i < num; i++ )
    {
      player = &g_entities[ entityList[ i ] ];

      if( player->flags & FL_NOTARGET )
        continue; // notarget cancels even beneficial effects?

      if( player->client && player->client->ps.stats[ STAT_TEAM ] == TEAM_HUMANS )
      {
        if( ( player->health < player->client->ps.stats[ STAT_MAX_HEALTH ] ||
              player->client->ps.stats[ STAT_STAMINA ] < STAMINA_MAX ) &&
            PM_Live( player->client->ps.pm_type ) )
        {
          if( !G_Surge( self, MEDISTAT_I_ACTIVE ) )
            return;
    
          self->enemy = player;

          //start the heal anim
          if( !self->active && self->surge && self->surgePowered )
          {
            G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );
            self->active = qtrue;
            player->client->ps.stats[ STAT_STATE ] |= SS_HEALING_ACTIVE;
          }
        }
        else if( !BG_InventoryContainsUpgrade( UP_MEDKIT, player->client->ps.stats ) )
          BG_AddUpgradeToInventory( UP_MEDKIT, player->client->ps.stats );
      }
    }
  }

  //nothing left to heal so go back to idling
  if( !self->enemy && self->active )
  {
    G_SetBuildableAnim( self, BANIM_CONSTRUCT2, qtrue );
    G_SetIdleBuildableAnim( self, BANIM_IDLE1 );

    self->active = qfalse;
  }
  else if( self->enemy && self->enemy->client ) //heal!
  {
    if( self->enemy->client->ps.stats[ STAT_STAMINA ] <  STAMINA_MAX )
      self->enemy->client->ps.stats[ STAT_STAMINA ] += STAMINA_MEDISTAT_RESTORE;

    if( self->enemy->client->ps.stats[ STAT_STAMINA ] > STAMINA_MAX )
      self->enemy->client->ps.stats[ STAT_STAMINA ] = STAMINA_MAX;

    self->enemy->health++;

    //if they're completely healed, give them a medkit
    if( self->enemy->health >= self->enemy->client->ps.stats[ STAT_MAX_HEALTH ] )
    {
      self->enemy->health =  self->enemy->client->ps.stats[ STAT_MAX_HEALTH ];
      if( !BG_InventoryContainsUpgrade( UP_MEDKIT, self->enemy->client->ps.stats ) )
        BG_AddUpgradeToInventory( UP_MEDKIT, self->enemy->client->ps.stats );
    }
  }
}




//==================================================================================




/*
================
HMGTurret_CheckTarget

Used by HMGTurret_Think to check enemies for validity
================
*/
qboolean HMGTurret_CheckTarget( gentity_t *self, gentity_t *target,
                                qboolean los_check )
{
  trace_t   tr;
  vec3_t    dir, end;
  
  if( !target || target->health <= 0 || !target->client ||
      target->client->pers.teamSelection != TEAM_ALIENS )
    return qfalse;

  if( target->flags & FL_NOTARGET )
    return qfalse;
    
  if( !los_check )
    return qtrue;

  // Accept target if we can line-trace to it
  VectorSubtract( target->s.pos.trBase, self->s.pos.trBase, dir );
  VectorNormalize( dir );
  VectorMA( self->s.pos.trBase, MGTURRET_RANGE, dir, end );
  trap_Trace( &tr, self->s.pos.trBase, NULL, NULL, end,
              self->s.number, MASK_SHOT );
  return tr.entityNum == target - g_entities;
}


/*
================
HMGTurret_TrackEnemy

Used by HMGTurret_Think to track enemy location
================
*/
qboolean HMGTurret_TrackEnemy( gentity_t *self )
{
  vec3_t  dirToTarget, dttAdjusted, angleToTarget, angularDiff, xNormal;
  vec3_t  refNormal = { 0.0f, 0.0f, 1.0f };
  float   temp, rotAngle;

  VectorSubtract( self->enemy->s.pos.trBase, self->s.pos.trBase, dirToTarget );
  VectorNormalize( dirToTarget );

  CrossProduct( self->s.origin2, refNormal, xNormal );
  VectorNormalize( xNormal );
  rotAngle = RAD2DEG( acos( DotProduct( self->s.origin2, refNormal ) ) );
  RotatePointAroundVector( dttAdjusted, xNormal, dirToTarget, rotAngle );

  vectoangles( dttAdjusted, angleToTarget );

  angularDiff[ PITCH ] = AngleSubtract( self->s.angles2[ PITCH ], angleToTarget[ PITCH ] );
  angularDiff[ YAW ] = AngleSubtract( self->s.angles2[ YAW ], angleToTarget[ YAW ] );

  //if not pointing at our target then move accordingly
  if( angularDiff[ PITCH ] < 0 && angularDiff[ PITCH ] < (-MGTURRET_ANGULARSPEED) )
    self->s.angles2[ PITCH ] += MGTURRET_ANGULARSPEED;
  else if( angularDiff[ PITCH ] > 0 && angularDiff[ PITCH ] > MGTURRET_ANGULARSPEED )
    self->s.angles2[ PITCH ] -= MGTURRET_ANGULARSPEED;
  else
    self->s.angles2[ PITCH ] = angleToTarget[ PITCH ];

  //disallow vertical movement past a certain limit
  temp = fabs( self->s.angles2[ PITCH ] );
  if( temp > 180 )
    temp -= 360;

  if( temp < -MGTURRET_VERTICALCAP )
    self->s.angles2[ PITCH ] = (-360) + MGTURRET_VERTICALCAP;

  //if not pointing at our target then move accordingly
  if( angularDiff[ YAW ] < 0 && angularDiff[ YAW ] < ( -MGTURRET_ANGULARSPEED ) )
    self->s.angles2[ YAW ] += MGTURRET_ANGULARSPEED;
  else if( angularDiff[ YAW ] > 0 && angularDiff[ YAW ] > MGTURRET_ANGULARSPEED )
    self->s.angles2[ YAW ] -= MGTURRET_ANGULARSPEED;
  else
    self->s.angles2[ YAW ] = angleToTarget[ YAW ];

  AngleVectors( self->s.angles2, dttAdjusted, NULL, NULL );
  RotatePointAroundVector( dirToTarget, xNormal, dttAdjusted, -rotAngle );
  vectoangles( dirToTarget, self->turretAim );

  //fire if target is within accuracy
  return ( abs( angularDiff[ YAW ] ) - MGTURRET_ANGULARSPEED <=
           MGTURRET_ACCURACY_TO_FIRE ) &&
         ( abs( angularDiff[ PITCH ] ) - MGTURRET_ANGULARSPEED <=
           MGTURRET_ACCURACY_TO_FIRE );
}


/*
================
HMGTurret_FindEnemy

Used by HMGTurret_Think to locate enemy gentities
================
*/
void HMGTurret_FindEnemy( gentity_t *self )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    range;
  vec3_t    mins, maxs;
  int       i, num;
  gentity_t *target;
  int       start;

  if( self->enemy )
    self->enemy->targeted = NULL;

  self->enemy = NULL;
    
  // Look for targets in a box around the turret
  VectorSet( range, MGTURRET_RANGE, MGTURRET_RANGE, MGTURRET_RANGE );
  VectorAdd( self->s.origin, range, maxs );
  VectorSubtract( self->s.origin, range, mins );
  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

  if( num == 0 )
    return;

  start = rand( ) / ( RAND_MAX / num + 1 );
  for( i = start; i < num + start ; i++ )
  {
    target = &g_entities[ entityList[ i % num ] ];
    if( !HMGTurret_CheckTarget( self, target, qtrue ) )
      continue;

    self->enemy = target;
    self->enemy->targeted = self;
    return;
  }
}

/*
================
HMGTurret_State

Raise or lower MG turret towards desired state
================
*/
enum {
  MGT_STATE_INACTIVE,
  MGT_STATE_DROP,
  MGT_STATE_RISE,
  MGT_STATE_ACTIVE
};

static qboolean HMGTurret_State( gentity_t *self, int state )
{
  float angle;

  if( self->waterlevel == state )
    return qfalse;

  angle = AngleNormalize180( self->s.angles2[ PITCH ] );

  if( state == MGT_STATE_INACTIVE )
  {
    if( angle < MGTURRET_VERTICALCAP )
    {
      if( self->waterlevel != MGT_STATE_DROP )
      {
        self->speed = 0.25f;
        self->waterlevel = MGT_STATE_DROP;
      }
      else
        self->speed *= 1.25f;

      self->s.angles2[ PITCH ] = 
        MIN( MGTURRET_VERTICALCAP, angle + self->speed );
      return qtrue;
    }
    else
      self->waterlevel = MGT_STATE_INACTIVE;
  }
  else if( state == MGT_STATE_ACTIVE )
  {
    if( !self->enemy && angle > 0.0f )
    {
      self->waterlevel = MGT_STATE_RISE;
      self->s.angles2[ PITCH ] =
        MAX( 0.0f, angle - MGTURRET_ANGULARSPEED * 0.5f );
    }
    else
      self->waterlevel = MGT_STATE_ACTIVE;
  }

  return qfalse;
}

/*
================
HMGTurret_Think

Think function for MG turret
================
*/

void HMGTurret_Think( gentity_t *self )
{
  self->nextthink = level.time + 
                    BG_Buildable( self->s.modelindex, NULL )->nextthink;

  // Turn off client side muzzle flashes
  self->s.eFlags &= ~EF_FIRING;
  
  if( !self->spawned || self->health <= 0 )
    return;
  
  G_CheckPower( self, MGTURRET_I_IDLE );

  G_IdlePowerState( self );

  // If not powered or spawned don't do anything
  if( !self->powered )
  {
    // if power loss drop turret
    if( self->spawned &&
        HMGTurret_State( self, MGT_STATE_INACTIVE ) );
      return;

    return;
  }
  if( !self->spawned )
    return;
    
  // If the current target is not valid find a new enemy
  if( !HMGTurret_CheckTarget( self, self->enemy, qtrue ) )
  {
    self->active = qfalse;
    self->turretSpinupTime = -1;
    HMGTurret_FindEnemy( self );
  }
  // if newly powered raise turret
  HMGTurret_State( self, MGT_STATE_ACTIVE );
  if( !self->enemy )
    return;

  // Track until we can hit the target
  if( !HMGTurret_TrackEnemy( self ) )
  {
    self->active = qfalse;
    self->turretSpinupTime = -1;
    return;
  }

  // Update spin state
  if( !self->active && self->timestamp < level.time )
  {
    self->active = qtrue;

    self->turretSpinupTime = level.time + MGTURRET_SPINUP_TIME;
    G_AddEvent( self, EV_MGTURRET_SPINUP, 0 );
  }

  // Not firing or haven't spun up yet
  if( !self->active || self->turretSpinupTime > level.time )
    return;
    
  if( !G_Surge( self, MGTURRET_I_ACTIVE ) )
    return;

  // Fire repeat delay
  if( self->timestamp > level.time )
    return;

  FireWeapon( self );
  self->s.eFlags |= EF_FIRING;
  self->timestamp = level.time + BG_Buildable( self->s.modelindex, NULL )->turretFireSpeed;
  G_AddEvent( self, EV_FIRE_WEAPON, 0 );
  G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );
}




//==================================================================================




/*
================
HTeslaGen_Think

Think function for Tesla Generator
================
*/
void HTeslaGen_Think( gentity_t *self )
{
  vec3_t origin, range, mins, maxs;
  int entityList[ MAX_GENTITIES ], i, num;
  
  self->nextthink = level.time + BG_Buildable( self->s.modelindex, NULL )->nextthink;

  if( !self->spawned || self->health <= 0 )
    return;

  G_CheckPower( self, TESLAGEN_I_IDLE );

  G_IdlePowerState( self );

  //if not powered don't do anything and check again for power next think
  if( !self->powered )
  {
    self->s.eFlags &= ~EF_FIRING;
    return;
  }

  if( !self->spawned )
    return;

  // Communicates firing state to client
  self->s.eFlags &= ~EF_FIRING;

  // Move the muzzle from the entity origin up a bit to fire over turrets
  VectorMA( self->s.origin, self->r.maxs[ 2 ], self->s.origin2, origin );

  VectorSet( range, TESLAGEN_RANGE, TESLAGEN_RANGE, TESLAGEN_RANGE );
  VectorAdd( origin, range, maxs );
  VectorSubtract( origin, range, mins );

  // Attack nearby Aliens
  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
  for( i = 0; i < num; i++ )
  {
    self->enemy = &g_entities[ entityList[ i ] ];

    if( self->enemy->flags & FL_NOTARGET )
      continue;

    if( self->enemy->client && self->enemy->health > 0 &&
        self->enemy->client->ps.stats[ STAT_TEAM ] == TEAM_ALIENS &&
        Distance( origin, self->enemy->s.pos.trBase ) <= TESLAGEN_RANGE )
    {
      if( !G_Surge( self, TESLAGEN_I_ACTIVE ) )
        break;

      FireWeapon( self );
    }
  }
  self->enemy = NULL;

  if( self->s.eFlags & EF_FIRING )
  {
    G_AddEvent( self, EV_FIRE_WEAPON, 0 );

    //doesn't really need an anim
    //G_SetBuildableAnim( self, BANIM_ATTACK1, qfalse );

    self->timestamp = level.time + TESLAGEN_REPEAT;
  }
}

//==================================================================================

void HDCC_Think( gentity_t *self )
{
  int i, count, list[ MAX_GENTITIES ];
  gentity_t *ent;
  vec3_t mins, maxs;
  
  self->nextthink = level.time + DC_HEALRATE;

  if( !self->spawned || self->health <= 0 )
    return;
    
  G_CheckPower( self, DC_I_IDLE );
  G_IdlePowerState( self );
  if( !self->powered )
    return;

  for( i = 0; i < 3; i++ )
    mins[ i ] = self->s.origin[ i ] - DC_RANGE,
    maxs[ i ] = self->s.origin[ i ] + DC_RANGE;
  
  count = trap_EntitiesInBox( mins, maxs, list, MAX_GENTITIES );
  
  for( i = 0; i < count; i++ )
  {
    ent = g_entities + list[ i ];
    
    if( ent->s.eType != ET_BUILDABLE )
      continue;
    if( ent->buildableTeam != TEAM_HUMANS )
      continue;
    if( ent->health >= BG_Buildable( ent->s.modelindex, ent->cuboidSize )->health )
      continue;      
    if( ent->lastDamageTime + HUMAN_REGEN_DAMAGE_TIME >= level.time )
      continue;
    if( BG_Buildable( ent->s.modelindex, NULL )->cuboid &&
        !BG_CuboidAttributes( ent->s.modelindex )->repairable )
      continue;
      
    if( !G_Surge( self, DC_I_ACTIVE ) )
      return;
    
    ent->health++;
  }
}

//==================================================================================

void HCapbank_Think( gentity_t *self)
{
  //nothing to do here
}

//==================================================================================

void HRTG_Think( gentity_t *self)
{
  self->nextthink = level.time + BG_Buildable( self->s.modelindex, NULL )->nextthink;

  if( !self->spawned || self->health <= 0 )
    return;

  if( G_TimeTilSuddenDeath( ) <= 0 )
  {
    self->storedBP = 0;
    return;
  }

  self->storedBP += RTG_YIELD * g_massYieldModifier.value;
  
  if( self->storedBP > RTG_STORAGE )
    self->storedBP = RTG_STORAGE;
}

//==================================================================================

void HRefinery_Think( gentity_t *self)
{
  self->nextthink = level.time + BG_Buildable( self->s.modelindex, NULL )->nextthink;

  if( !self->spawned || self->health <= 0 )
    return;

  G_CheckPower( self, REFINERY_I_IDLE );
  G_IdlePowerState( self );

  if( G_TimeTilSuddenDeath( ) <= 0 )
  {
    self->storedBP = 0;
    return;
  }

  //nothing to do
  if( self->storedBP >= REFINERY_STORAGE - 0.001f )
    return;

  if( !G_Surge( self, REFINERY_I_ACTIVE ) )
    return;
  
  self->storedBP += REFINERY_YIELD * g_massYieldModifier.value;
  
  if( self->storedBP > REFINERY_STORAGE )
    self->storedBP = REFINERY_STORAGE;
}

//==================================================================================
//         CUBOID FUNCTIONS
// generic functions for all types of cuboids
//==================================================================================

void Cuboid_Think(gentity_t *self)
{
}

//Cuboids need a new die function because of the cuboid explosion effects.
void Cuboid_Die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
  vec3_t dir;
  qboolean event = qfalse;
  const cuboidAttributes_t *cuboid;
  
  cuboid = BG_CuboidAttributes( self->s.modelindex );
  G_SetBuildableAnim( self, BANIM_DESTROY1, qtrue ); // just for sound
  self->die = nullDieFunction;
  self->killedBy = attacker - g_entities;
  self->s.eFlags &= ~EF_FIRING;
  G_LogDestruction( self, attacker, mod );
  dir[ 0 ] =
  dir[ 1 ] = 0;
  dir[ 2 ] = 1;
  self->timestamp = level.time;
  G_QueueBuildPoints( self );
  
  if( mod != MOD_DECONSTRUCT && self->spawned )
  {
    G_RewardAttackers( self );
    G_RadiusDamage( self->s.pos.trBase,
                    g_entities + self->killedBy,
                    self->splashDamage,
                    self->splashRadius,
                    self,
                    self->splashMethodOfDeath );
    //NOTE: all cuboid info is already packed
    self->s.eType = ET_EVENTS + EV_CUBOID_EXPLOSION;
    self->freeAfterEvent = qtrue;
    G_AddEvent( self, EV_HUMAN_BUILDABLE_EXPLOSION, DirToByte( dir ) );
    event = qtrue;
    self->r.contents = 0;
    trap_LinkEntity( self );
  }
  else
  {
    self->s.eType = 0;
    G_FreeEntity( self );
  }
}


//==================================================================================




/*
============
G_QueueValue
============
*/

static int G_QueueValue( gentity_t *self )
{
  int       i;
  int       damageTotal = 0;
  int       queuePoints;
  double    queueFraction = 0;

  for( i = 0; i < level.maxclients; i++ )
  {
    gentity_t *player = g_entities + i;

    damageTotal += self->credits[ i ];

    if( self->buildableTeam != player->client->pers.teamSelection )
      queueFraction += (double) self->credits[ i ];
  }

  if( damageTotal > 0 )
    queueFraction = queueFraction / (double) damageTotal;
  else // all damage was done by nonclients, so queue everything
    queueFraction = 1.0;

  queuePoints = (int) ( queueFraction * (double) BG_Buildable( self->s.modelindex, self->cuboidSize )->buildPoints );
  return queuePoints;
}

/*
============
G_QueueBuildPoints
============
*/
void G_QueueBuildPoints( gentity_t *self )
{
  gentity_t *powerEntity;
  int       queuePoints;

  queuePoints = G_QueueValue( self );

  if( !queuePoints )
    return;
      
  switch( self->buildableTeam )
  {
    default:
    case TEAM_NONE:
      return;

    case TEAM_ALIENS:
      if( !level.alienBuildPointQueue )
        level.alienNextQueueTime = level.time + g_alienBuildQueueTime.integer;

      level.alienBuildPointQueue += queuePoints;
      break;
  }
}

/*
============
G_NextQueueTime
============
*/
int G_NextQueueTime( int queuedBP, int totalBP, int queueBaseRate )
{
  float fractionQueued;

  if( totalBP == 0 )
    return 0;

  fractionQueued = queuedBP / (float)totalBP;
  return ( 1.0f - fractionQueued ) * queueBaseRate;
}

/*
============
G_BuildableTouchTriggers

Find all trigger entities that a buildable touches.
============
*/
void G_BuildableTouchTriggers( gentity_t *ent )
{
  int       i, num;
  int       touch[ MAX_GENTITIES ];
  gentity_t *hit;
  trace_t   trace;
  vec3_t    mins, maxs;
  vec3_t    bmins, bmaxs;
  static    vec3_t range = { 10, 10, 10 };

  // dead buildables don't activate triggers!
  if( ent->health <= 0 )
    return;

  BG_BuildableBoundingBox( ent->s.modelindex, bmins, bmaxs );

  VectorAdd( ent->s.origin, bmins, mins );
  VectorAdd( ent->s.origin, bmaxs, maxs );

  VectorSubtract( mins, range, mins );
  VectorAdd( maxs, range, maxs );

  num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

  VectorAdd( ent->s.origin, bmins, mins );
  VectorAdd( ent->s.origin, bmaxs, maxs );

  for( i = 0; i < num; i++ )
  {
    hit = &g_entities[ touch[ i ] ];

    if( !hit->touch )
      continue;

    if( !( hit->r.contents & CONTENTS_TRIGGER ) )
      continue;

    //ignore buildables not yet spawned
    if( !ent->spawned )
      continue;

    if( !trap_EntityContact( mins, maxs, hit ) )
      continue;

    memset( &trace, 0, sizeof( trace ) );

    if( hit->touch )
      hit->touch( hit, ent, &trace );
  }
}


/*
===============
G_BuildableThink

General think function for buildables
===============
*/
void G_BuildableThink( gentity_t *ent, int msec )
{
  int maxHealth = BG_Buildable( ent->s.modelindex, ent->cuboidSize )->health;
  int regenRate = BG_Buildable( ent->s.modelindex, ent->cuboidSize )->regenRate;
  int buildTime = BG_Buildable( ent->s.modelindex, ent->cuboidSize )->buildTime;
  int buildRate;
  
  //toggle spawned flag for buildables
  if( !ent->spawned && ( ent->healthLeft<=0 || ent->health==maxHealth ) && !level.pausedTime )
  {
    ent->spawned = qtrue;
    if( ent->s.modelindex == BA_A_OVERMIND )
    {
      G_TeamCommand( TEAM_ALIENS, "cp \"The Overmind has awakened!\"" );
    }
    G_SetupPowerEntity( ent );
  }

  // Timer actions
  ent->time1000 += msec;
  if( ent->time1000 >= 1000 )
  {
    ent->time1000 -= 1000;

    if( !ent->spawned )
    {
      buildRate = (int)( ceil( (float)maxHealth / (float)buildTime * 1.0e3f ) );
      
      if( ent->buildableTeam == TEAM_HUMANS &&
          ent->s.modelindex != BA_H_RTG )
      {
        buildRate *= ent->current / PREBUILD_CURRENT;
        if( buildRate < 0 )
          buildRate = 0;
        ent->powered = ( buildRate > 0 );
      }
      ent->health = MIN( ent->health + buildRate, maxHealth );
      ent->healthLeft = MAX( ent->healthLeft - buildRate, 0 );
    }
    else if( ent->health > 0 && ent->health < maxHealth )
    {
      if( ent->buildableTeam == TEAM_ALIENS && regenRate &&
        ( ent->lastDamageTime + ALIEN_REGEN_DAMAGE_TIME ) < level.time )
      {
        ent->health += regenRate;
      }
    }

    if( ent->health >= maxHealth )
    {
      int i;
      ent->health = maxHealth;
      for( i = 0; i < MAX_CLIENTS; i++ )
        ent->credits[ i ] = 0;
    }
  }

  if( ent->clientSpawnTime > 0 )
    ent->clientSpawnTime -= msec;

  if( ent->clientSpawnTime < 0 )
    ent->clientSpawnTime = 0;

  // Set health
  ent->s.generic1 = MIN( MAX( ent->health, 0 ), 999 );
  
  // Set health for cuboids (BG_CuboidPackHealthSafe does nothing if not a cuboid)
  BG_CuboidPackHealthSafe( ent->s.modelindex, &ent->s, ent->health );

  // Set flags
  ent->s.eFlags &= ~( EF_B_POWERED | EF_B_SPAWNED | EF_B_SURGE );
  
  if( ent->buildableTeam == TEAM_HUMANS )
  {
    if( ent->s.modelindex == BA_H_REPEATER )
    {
      if( ent->powered )
        ent->s.eFlags |= EF_B_POWERED;
      if( ent->active )
        ent->s.eFlags |= EF_B_SURGE;
    }
    else if( ent->isPowerSource )
    {
      ent->s.eFlags |= EF_B_POWERED;
      if( ent->active )
        ent->s.eFlags |= EF_B_SURGE;
    }
    else
    {
      if( ent->powered )
      {
        if( ent->surge )
        {
          ent->s.eFlags |= EF_B_SURGE;
          if( ent->surgePowered )
            ent->s.eFlags |= EF_B_POWERED;
        }
        else
          ent->s.eFlags |= EF_B_POWERED;
      }
    }
  }
  else
  {
    if( ent->powered )
      ent->s.eFlags |= EF_B_POWERED;
  }

  if( ent->spawned )
    ent->s.eFlags |= EF_B_SPAWNED;

  // Check if this buildable is touching any triggers
  G_BuildableTouchTriggers( ent );

  // Fall back on normal physics routines
  G_Physics( ent, msec );
}


/*
===============
G_BuildableRange

Check whether a point is within some range of a type of buildable
===============
*/
qboolean G_BuildableRange( vec3_t origin, float r, buildable_t buildable )
{
  int       entityList[ MAX_GENTITIES ];
  vec3_t    range;
  vec3_t    mins, maxs;
  int       i, num;
  gentity_t *ent;

  VectorSet( range, r, r, r );
  VectorAdd( origin, range, maxs );
  VectorSubtract( origin, range, mins );

  num = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );
  for( i = 0; i < num; i++ )
  {
    ent = &g_entities[ entityList[ i ] ];

    if( ent->s.eType != ET_BUILDABLE )
      continue;

    if( ent->buildableTeam == TEAM_HUMANS && !ent->powered )
      continue;

    if( ent->s.modelindex == buildable && ent->spawned )
      return qtrue;
  }

  return qfalse;
}

/*
================
G_FindBuildable

Finds a buildable of the specified type
================
*/
static gentity_t *G_FindBuildable( buildable_t buildable )
{
  int       i;
  gentity_t *ent;

  for( i = MAX_CLIENTS, ent = g_entities + i;
       i < level.num_entities; i++, ent++ )
  {
    if( ent->s.eType != ET_BUILDABLE )
      continue;

    if( ent->s.modelindex == buildable && !( ent->s.eFlags & EF_DEAD ) )
      return ent;
  }

  return NULL;
}

/*
===============
G_BuildablesIntersect

Test if two buildables intersect each other
===============
*/
static qboolean G_BuildablesIntersect( buildable_t a, vec3_t originA, vec3_t cuboidA,
                                       buildable_t b, vec3_t originB, vec3_t cuboidB )
{
  vec3_t minsA, maxsA;
  vec3_t minsB, maxsB;

  if(BG_Buildable(a,NULL)->cuboid)
    BG_CuboidBBox(cuboidA,minsA,maxsA);
  else
    BG_BuildableBoundingBox( a, minsA, maxsA );
  VectorAdd( minsA, originA, minsA );
  VectorAdd( maxsA, originA, maxsA );

  if(BG_Buildable(b,NULL)->cuboid)
    BG_CuboidBBox(cuboidB,minsB,maxsB);
  else
    BG_BuildableBoundingBox( b, minsB, maxsB );
  VectorAdd( minsB, originB, minsB );
  VectorAdd( maxsB, originB, maxsB );

  return BoundsIntersect( minsA, maxsA, minsB, maxsB );
}

/*
===============
G_SufficientBPAvailable

Determine if enough build points can be released for the buildable
and list the buildables that must be destroyed if this is the case
===============
*/
static itemBuildError_t G_SufficientBPAvailable( gclient_t      *builder,
                                                 buildable_t     buildable,
                                                 vec3_t          origin,
                                                 vec3_t          cuboidSize )
{
  int required;
  int available;
  int error;
  
  required = BG_Buildable( buildable, cuboidSize )->buildPoints;
  
  switch( BG_Buildable( buildable, NULL )->team )
  {
    case TEAM_ALIENS:
      error = IBE_NOALIENBP;
      available = G_GetBuildPoints( origin, TEAM_ALIENS );
      break;
      
    case TEAM_HUMANS:
      error = IBE_NOHUMANBP;
      if( !builder )
        available = required; //always enough
      else
        //first RTG is free
        if( buildable == BA_H_RTG && !G_IsRTGBuilt() )
          available = INFINITE;
        else
          available = builder->ps.persistant[ PERS_BUILDPOINTS ];
      break;
  }
  
  if( required > available )
    return error;
    
  return IBE_NONE;


#if 0
  // Check for buildable<->buildable collisions
  // FIXME: is this check even needed?
  for( i = MAX_CLIENTS, ent = g_entities + i; i < level.num_entities; i++, ent++ )
  {
    if( ent->s.eType != ET_BUILDABLE )
      continue;

    if( G_BuildablesIntersect( buildable, origin, cuboidSize, ent->s.modelindex, ent->s.origin, ent->cuboidSize ) )
      return IBE_NOROOM;
  }
#endif
}

/*
================
G_SetBuildableLinkState

Links or unlinks all the buildable entities
================
*/
static void G_SetBuildableLinkState( qboolean link )
{
  int       i;
  gentity_t *ent;

  for ( i = 1, ent = g_entities + i; i < level.num_entities; i++, ent++ )
  {
    if( ent->s.eType != ET_BUILDABLE )
      continue;

    if( link )
      trap_LinkEntity( ent );
    else
      trap_UnlinkEntity( ent );
  }
}

/*
================
G_CanBuild

Checks to see if a buildable can be built
================
*/
itemBuildError_t G_CanBuild( gentity_t *ent, buildable_t buildable, int distance, vec3_t origin, vec3_t normal, vec3_t cuboidSize )
{
  vec3_t            angles;
  vec3_t            entity_origin;
  vec3_t            mins, maxs;
  trace_t           tr1, tr2, tr3;
  itemBuildError_t  reason = IBE_NONE, tempReason;
  gentity_t         *tempent;
  float             minNormal;
  qboolean          invert;
  int               contents;
  playerState_t     *ps = &ent->client->ps;

  if( BG_Buildable( buildable, NULL )->cuboid )
  {
    if( !G_CheckCuboidSize( cuboidSize, buildable ) )
      return IBE_INVALIDSIZE;
    BG_CuboidBBox( cuboidSize, mins, maxs );
  }
  else
    BG_BuildableBoundingBox( buildable, mins, maxs );
  
  if( !BG_PositionBuildableRelativeToPlayer(
        ps, BG_Buildable( buildable, NULL )->cuboid,
        mins, maxs, trap_Trace, entity_origin, angles, &tr1 ) )
    return IBE_NOSURF;
  trap_Trace( &tr2, entity_origin, mins, maxs, entity_origin, -1, MASK_PLAYERSOLID );
  trap_Trace( &tr3, ps->origin, NULL, NULL, entity_origin, ent->s.number, MASK_PLAYERSOLID );

  VectorCopy( entity_origin, origin );

  VectorCopy( tr1.plane.normal, normal );
  minNormal = BG_Buildable( buildable, NULL )->minNormal;
  invert = BG_Buildable( buildable, NULL )->invertNormal;
  
  //can we build at this angle?
  if(!BG_Buildable(buildable,NULL)->cuboid)
    if( !( normal[ 2 ] >= minNormal || ( invert && normal[ 2 ] <= -minNormal ) ) )
      reason = IBE_NORMAL;

  //the only buildable one can build on is a cuboid
  /*
  if( tr1.entityNum != ENTITYNUM_WORLD )
    if( g_entities[ tr1.entityNum ].s.eType == ET_BUILDABLE &&
        !BG_Buildable( g_entities[ tr1.entityNum ].s.modelindex, NULL )->cuboid )
      reason = IBE_NORMAL;
  */

  contents = trap_PointContents( entity_origin, -1 );

  if( ( tempReason = G_SufficientBPAvailable( ent->client, buildable, origin, cuboidSize ) ) != IBE_NONE )
    reason = tempReason;

  if( ent->client->ps.stats[ STAT_TEAM ] == TEAM_ALIENS )
  {
    //alien criteria

    // Check there is an Overmind
    if( buildable != BA_A_OVERMIND )
    {
      if( !G_Overmind( ) )
        reason = IBE_NOOVERMIND;
    }

    //check there is creep near by for building on
    if( BG_Buildable( buildable, NULL )->creepTest )
    {
      if( !G_IsCreepHere( entity_origin ) )
        reason = IBE_NOCREEP;
    }

    // Check permission to build here
    if( tr1.surfaceFlags & SURF_NOALIENBUILD || contents & CONTENTS_NOALIENBUILD )
      reason = IBE_PERMISSION;
  }
  else if( ent->client->ps.stats[ STAT_TEAM ] == TEAM_HUMANS )
  {
    //human criteria
    if( !G_PowerForPoint( origin ) && 
        buildable != BA_H_RTG )
      reason = IBE_NOPOWERHERE;

    //this buildable requires a DCC
    if( BG_Buildable( buildable, NULL )->dccTest && !G_IsDCCBuilt( ) )
      reason = IBE_NODCC;

    // Check permission to build here
    if( tr1.surfaceFlags & SURF_NOHUMANBUILD || contents & CONTENTS_NOHUMANBUILD )
      reason = IBE_PERMISSION;
  }

  // Check permission to build here
  if( tr1.surfaceFlags & SURF_NOBUILD || contents & CONTENTS_NOBUILD )
    reason = IBE_PERMISSION;

  // Can we only have one of these?
  if( BG_Buildable( buildable, NULL )->uniqueTest )
  {
    tempent = G_FindBuildable( buildable );
    if( tempent )
    {
      switch( buildable )
      {
        case BA_A_OVERMIND:
          reason = IBE_ONEOVERMIND;
          break;

        case BA_H_REACTOR:
          reason = IBE_ONEREACTOR;
          break;

        default:
          Com_Error( ERR_FATAL, "No reason for denying build of %d\n", buildable );
          break;
      }
    }
  }

  //check there is enough room to spawn from (presuming this is a spawn)
  if( reason == IBE_NONE )
    if( G_CheckSpawnPoint( ENTITYNUM_NONE, origin, normal, buildable, NULL ) != NULL )
      reason = IBE_NORMAL;

  //this item does not fit here
  if( reason == IBE_NONE && ( tr2.startsolid || !COMPARE_FLOAT_EPSILON(tr3.fraction,1.0f) ) )
    reason = IBE_NOROOM;
  
  if( g_buildableDensityLimit.integer > 0 )
  {
    int i, count, numents, ents[ MAX_GENTITIES ];
    vec3_t rmins, rmaxs;
    gentity_t *other;
    
    for( i = 0; i < 3; i++ )
      rmins[ i ] = origin[ i ] - g_buildableDensityLimitRange.value,
      rmaxs[ i ] = origin[ i ] + g_buildableDensityLimitRange.value;

    numents = trap_EntitiesInBox( rmins, rmaxs, ents, MAX_GENTITIES );
 
    for( count = 0, i = 0; i < numents ; i++ )
    {
      other = &g_entities[ ents[ i ] ];
      
      if( other->s.eType != ET_BUILDABLE )
        continue;
      
      if( other->buildableTeam != ent->client->ps.stats[ STAT_TEAM ] )
        continue;
      
      if( Distance( origin, other->s.origin ) > g_buildableDensityLimitRange.value )
        continue;
      
      if( !trap_InPVSIgnorePortals( origin, other->s.origin ) )
        continue;

      if( ++count > g_buildableDensityLimit.integer )
      {
        reason = IBE_TOODENSE;
        break;
      }
    }
  }

  return reason;
}

/*
================
G_CuboidName

For standard buildables the function returns humanName.
In case of cuboids, a "dynamic" one is generated.
================
*/

const char *G_CuboidName(buildable_t buildable, const vec3_t cuboidSize, qboolean verbose)
{
 static char buffer[100];
 const buildableAttributes_t *battr;
 
 if((battr=BG_Buildable(buildable,cuboidSize))->cuboid)
 {
  if(verbose)
   Com_sprintf(buffer,sizeof(buffer),"%s %fx%fx%f %ibp",battr->humanName,cuboidSize[0],cuboidSize[1],cuboidSize[2],battr->buildPoints);
  else
   Com_sprintf(buffer,sizeof(buffer),"%s %s (%i bp)",BG_CuboidAttributes(buildable)->icon,battr->humanName,battr->buildPoints);
  return buffer;
 }
 else
  return battr->humanName;
}

/*
================
G_Build

Spawns a buildable
================
*/
static gentity_t *G_Build( gentity_t *builder, buildable_t buildable,
    const vec3_t origin, const vec3_t normal, const vec3_t angles,
    const vec3_t cuboidSize )
{
  gentity_t *built;
  vec3_t    localOrigin;
  char      buildnums[ MAX_STRING_CHARS ];
  buildLog_t *log;
  qboolean  cuboid;
  
  cuboid = BG_Buildable(buildable,NULL)->cuboid;

  VectorCopy( origin, localOrigin );

  if( builder->client )
    log = G_BuildLogNew( builder, BF_CONSTRUCT );
  else
    log = NULL;

  // Spawn the buildable
  built = G_Spawn();
  built->s.eType = ET_BUILDABLE;
  built->killedBy = ENTITYNUM_NONE;
  built->classname = BG_Buildable( buildable, NULL )->entityName;
  built->s.modelindex = buildable;
  built->buildableTeam = built->s.modelindex2 = BG_Buildable( buildable, NULL )->team;

  // when building the initial layout, spawn the entity slightly off its
  // target surface so that it can be "dropped" onto it
  if( !builder->client )
    VectorMA( localOrigin, 1.0f, normal, localOrigin );

  if( cuboid )
  {
    BG_CuboidBBox( cuboidSize, built->r.mins, built->r.maxs );
    VectorCopy( cuboidSize, &built->cuboidSize );
  }
  else
    BG_BuildableBoundingBox( buildable, built->r.mins, built->r.maxs );
  
  built->health = 1;
  built->healthLeft = BG_Buildable( buildable, cuboidSize )->health-1;
  built->s.generic1 = MIN(MAX(built->health,0),999);
  built->splashDamage = BG_Buildable( buildable, cuboidSize )->splashDamage;
  built->splashRadius = BG_Buildable( buildable, cuboidSize )->splashRadius;
  built->splashMethodOfDeath = BG_Buildable( buildable, cuboidSize )->meansOfDeath;
  
  built->nextthink = BG_Buildable( buildable, NULL )->nextthink;

  built->takedamage = qtrue;
  built->spawned = qfalse;
  built->buildTime = built->s.time = level.time;

  if( builder->client && g_instantBuild.integer )
  {
    built->health = BG_Buildable( buildable, cuboidSize )->health;
    built->buildTime = built->s.time =
      level.time -BG_Buildable( buildable, cuboidSize )->buildTime;
  }

  //things that vary for each buildable that aren't in the dbase
  switch( buildable )
  {
    case BA_A_SPAWN:
      built->die = AGeneric_Die;
      built->think = ASpawn_Think;
      built->pain = AGeneric_Pain;
      break;

    case BA_A_BARRICADE:
      built->die = ABarricade_Die;
      built->think = ABarricade_Think;
      built->pain = ABarricade_Pain;
      built->touch = ABarricade_Touch;
      built->shrunkTime = 0;
      ABarricade_Shrink( built, qtrue );
      break;

    case BA_A_BOOSTER:
      built->die = AGeneric_Die;
      built->think = AGeneric_Think;
      built->pain = AGeneric_Pain;
      built->touch = ABooster_Touch;
      break;

    case BA_A_ACIDTUBE:
      built->die = AGeneric_Die;
      built->think = AAcidTube_Think;
      built->pain = AGeneric_Pain;
      break;

    case BA_A_HIVE:
      built->die = AGeneric_Die;
      built->think = AHive_Think;
      built->pain = AHive_Pain;
      break;

    case BA_A_TRAPPER:
      built->die = AGeneric_Die;
      built->think = ATrapper_Think;
      built->pain = AGeneric_Pain;
      break;

    case BA_A_OVERMIND:
      built->die = AGeneric_Die;
      built->think = AOvermind_Think;
      built->pain = AGeneric_Pain;
      break;

    case BA_H_SPAWN:
      built->die = HSpawn_Die;
      built->think = HSpawn_Think;
      break;

    case BA_H_MGTURRET:
      built->die = HSpawn_Die;
      built->think = HMGTurret_Think;
      break;

    case BA_H_TESLAGEN:
      built->die = HSpawn_Die;
      built->think = HTeslaGen_Think;
      break;

    case BA_H_ARMOURY:
      built->think = HArmoury_Think;
      built->die = HSpawn_Die;
      built->use = HArmoury_Activate;
      break;

    case BA_H_DCC:
      built->think = HDCC_Think;
      built->die = HSpawn_Die;
      break;

    case BA_H_MEDISTAT:
      built->think = HMedistat_Think;
      built->die = HMedistat_Die;
      break;

    case BA_H_REACTOR:
      built->think = HReactor_Think;
      built->die = HSpawn_Die;
      built->use = HSwitchable_Use;
      built->powered = built->active = qtrue;
      break;

    case BA_H_REPEATER:
      built->think = HRepeater_Think;
      built->die = HRepeater_Die;
      built->use = HSwitchable_Use;
      built->count = -1;
      break;
    
    case BA_H_CAPBANK:
      built->think = HCapbank_Think;
      built->die = HSpawn_Die;
      built->use = HSwitchable_Use;
      break;

    case BA_H_RTG:
      built->think = HRTG_Think;
      built->die = HSpawn_Die;
      built->use = HSwitchable_Use;
      break;
      
    case BA_H_REFINERY:
      built->think = HRefinery_Think;
      built->die = HSpawn_Die;
      //built->use = HSwitchable_Use;
      break;

    default:
      //erk
      break;
  }

  if( buildable >= CUBOID_FIRST && buildable <= CUBOID_LAST )
  {
    built->think = Cuboid_Think;
    built->die = Cuboid_Die;
  }
  
  built->clipmask = MASK_PLAYERSOLID;
  built->r.contents = CONTENTS_BODY;
  built->s.number = built - g_entities;
  built->enemy = NULL;
  built->s.weapon = BG_Buildable( buildable, NULL )->turretProjType;

  if( builder->client )
    built->builtBy = builder->client->ps.clientNum;
  else
    built->builtBy = -1;

  G_SetOrigin( built, localOrigin );

  VectorScale( normal, -512.0f, built->s.pos.trDelta );

  if( BG_Buildable( buildable, NULL )->cuboid )
    VectorCopy( cuboidSize,built->s.angles );
  else
  {
    VectorCopy( angles, built->s.angles );
    built->s.angles[ PITCH ] = 0.0f;
  }
  
  built->s.pos.trType = BG_Buildable( buildable, NULL )->traj;
  built->s.pos.trTime = level.time;
  built->physicsBounce = BG_Buildable( buildable, NULL )->bounce;
  built->s.groundEntityNum = -1;
  //turret angles
  VectorCopy( builder->s.angles2, built->s.angles2 );
  built->s.angles2[ YAW ] = angles[ YAW ];
  built->s.angles2[ PITCH ] = MGTURRET_VERTICALCAP;

  built->powered = qtrue;
  built->s.eFlags |= EF_B_POWERED;

  built->s.eFlags &= ~EF_B_SPAWNED;

  VectorCopy( normal, built->s.origin2 );

  G_AddEvent( built, EV_BUILD_CONSTRUCT, 0 );

  G_SetIdleBuildableAnim( built, BG_Buildable( buildable, NULL )->idleAnim );

  if( built->builtBy >= 0 )
    G_SetBuildableAnim( built, BANIM_CONSTRUCT1, qtrue );

  if( BG_Buildable( buildable, NULL )->team == TEAM_HUMANS )
  {
    //special case for the RTG unit that can be built w/o power
    if( buildable == BA_H_RTG )
      built->requiresPower = qfalse;
    else
      built->requiresPower = qtrue;
      
    built->isPowerSource = qfalse;
    built->resistance = PREBUILD_RESISTANCE;
    if( BG_Buildable( buildable, NULL )->isPowerSource ||
        buildable == BA_H_REPEATER )
      built->active = qtrue; //spawn enabled
  }
  
  // subtract build points
  if( buildable != BA_H_RTG || G_IsRTGBuilt( ) ) //first RTG is free
    if( builder && builder->client )
      builder->client->ps.persistant[ PERS_BUILDPOINTS ] -= BG_Buildable( buildable, cuboidSize )->buildPoints;

  BG_CuboidPackHealthSafe( built->s.modelindex, &built->s, built->health );

  trap_LinkEntity( built );

  if( builder && builder->client )
  {
    G_TeamCommand( builder->client->ps.stats[ STAT_TEAM ],
      va( "print \"%s ^2built^7 by %s\n\"",
        G_CuboidName( built->s.modelindex, cuboidSize, qfalse ),
        builder->client->pers.netname ) );
    G_LogPrintf( "Construct: %d %d %s%s: %s" S_COLOR_WHITE " is building %s\n",
      builder - g_entities,
      built - g_entities,
      BG_Buildable( built->s.modelindex, NULL )->name,
      buildnums,
      builder->client->pers.netname,
      G_CuboidName( built->s.modelindex, built->cuboidSize, qtrue ) );
  }

  if( log )
    G_BuildLogSet( log, built );

  return built;
}

/*
=================
G_BuildIfValid
=================
*/
qboolean G_BuildIfValid( gentity_t *ent, buildable_t buildable, vec3_t cuboidSize )
{
  float         dist;
  vec3_t        origin, normal;
  
  dist = BG_Class( ent->client->ps.stats[ STAT_CLASS ] )->buildDist;

  switch( G_CanBuild( ent, buildable, dist, origin, normal, cuboidSize ) )
  {
    case IBE_NONE:
      G_Build( ent, buildable, origin, normal, ent->s.apos.trBase, cuboidSize );
      return qtrue;

    case IBE_NOALIENBP:
      G_TriggerMenu( ent->client->ps.clientNum, MN_A_NOBP );
      return qfalse;

    case IBE_NOOVERMIND:
      G_TriggerMenu( ent->client->ps.clientNum, MN_A_NOOVMND );
      return qfalse;

    case IBE_NOCREEP:
      G_TriggerMenu( ent->client->ps.clientNum, MN_A_NOCREEP );
      return qfalse;

    case IBE_ONEOVERMIND:
      G_TriggerMenu( ent->client->ps.clientNum, MN_A_ONEOVERMIND );
      return qfalse;

    case IBE_NORMAL:
      G_TriggerMenu( ent->client->ps.clientNum, MN_B_NORMAL );
      return qfalse;

    case IBE_PERMISSION:
      G_TriggerMenu( ent->client->ps.clientNum, MN_B_NORMAL );
      return qfalse;

    case IBE_ONEREACTOR:
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_ONEREACTOR );
      return qfalse;

    case IBE_NOPOWERHERE:
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOPOWERHERE );
      return qfalse;

    case IBE_NOROOM:
      G_TriggerMenu( ent->client->ps.clientNum, MN_B_NOROOM );
      return qfalse;

    case IBE_NOHUMANBP:
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_NOBP);
      return qfalse;

    case IBE_NODCC:
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_NODCC );
      return qfalse;

    case IBE_RPTPOWERHERE:
      G_TriggerMenu( ent->client->ps.clientNum, MN_H_RPTPOWERHERE );
      return qfalse;

    case IBE_LASTSPAWN:
      G_TriggerMenu( ent->client->ps.clientNum, MN_B_LASTSPAWN );
      return qfalse;

    case IBE_NOSURF:
      G_TriggerMenu( ent->client->ps.clientNum, MN_B_NOSURF );
      return qfalse;

    case IBE_TOODENSE:
      G_TriggerMenu( ent->client->ps.clientNum, MN_B_TOODENSE );
      return qfalse;

    case IBE_INVALIDSIZE:
      G_TriggerMenu( ent->client->ps.clientNum, MN_B_INVALIDSIZE );
      return qfalse;

    default:
      break;
  }

  return qfalse;
}

/*
================
G_FinishSpawningBuildable

Traces down to find where an item should rest, instead of letting them
free fall from their spawn points
================
*/
static gentity_t *G_FinishSpawningBuildable( gentity_t *ent, qboolean force )
{
  trace_t     tr;
  vec3_t      normal, dest;
  gentity_t   *built;
  buildable_t buildable = ent->s.modelindex;

  if( ent->s.origin2[ 0 ] || ent->s.origin2[ 1 ] || ent->s.origin2[ 2 ] )
    VectorCopy( ent->s.origin2, normal );
  else if( BG_Buildable( buildable, NULL )->traj == TR_BUOYANCY )
    VectorSet( normal, 0.0f, 0.0f, -1.0f );
  else
    VectorSet( normal, 0.0f, 0.0f, 1.0f );

  built = G_Build( ent, buildable, ent->s.pos.trBase, normal, ent->s.angles, built->cuboidSize );

  built->takedamage = qtrue;
  built->spawned = qtrue; //map entities are already spawned
  built->health = BG_Buildable( buildable, built->cuboidSize )->health;
  built->s.eFlags |= EF_B_SPAWNED;

  G_SetupPowerEntity( built );

  // drop towards normal surface
  VectorScale( built->s.origin2, -4096.0f, dest );
  VectorAdd( dest, built->s.origin, dest );

  trap_Trace( &tr, built->s.origin, built->r.mins, built->r.maxs, dest, built->s.number, built->clipmask );

  if( tr.startsolid && !force )
  {
    G_Printf( S_COLOR_YELLOW "G_FinishSpawningBuildable: %s startsolid at %s\n",
              built->classname, vtos( built->s.origin ) );
    G_FreeEntity( built );
    return NULL;
  }

  //point items in the correct direction
  VectorCopy( tr.plane.normal, built->s.origin2 );

  // allow to ride movers
  built->s.groundEntityNum = tr.entityNum;

  G_SetOrigin( built, tr.endpos );

  trap_LinkEntity( built );
  return built;
}

/*
============
G_SpawnBuildableThink

Complete spawning a buildable using it's placeholder
============
*/
static void G_SpawnBuildableThink( gentity_t *ent )
{
  G_FinishSpawningBuildable( ent, qfalse );
  G_FreeEntity( ent );
}

/*
============
G_SpawnBuildable

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void G_SpawnBuildable( gentity_t *ent, buildable_t buildable )
{
  ent->s.modelindex = buildable;

  // some movers spawn on the second frame, so delay item
  // spawns until the third frame so they can ride trains
  ent->nextthink = level.time + FRAMETIME * 2;
  ent->think = G_SpawnBuildableThink;
}

/*
============
G_LayoutSave
============
*/
void G_LayoutSave( char *name )
{
  char map[ MAX_QPATH ];
  char fileName[ MAX_OSPATH ];
  fileHandle_t f;
  int len;
  int i;
  gentity_t *ent;
  char *s;

  trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );
  if( !map[ 0 ] )
  {
    G_Printf( "LayoutSave( ): no map is loaded\n" );
    return;
  }
  Com_sprintf( fileName, sizeof( fileName ), "layouts/%s/%s.dat", map, name );

  len = trap_FS_FOpenFile( fileName, &f, FS_WRITE );
  if( len < 0 )
  {
    G_Printf( "layoutsave: could not open %s\n", fileName );
    return;
  }

  G_Printf( "layoutsave: saving layout to %s\n", fileName );

  for( i = MAX_CLIENTS; i < level.num_entities; i++ )
  {
    ent = &level.gentities[ i ];
    if( ent->s.eType != ET_BUILDABLE )
      continue;

    s = va( "%s %f %f %f %f %f %f %f %f %f %f %f %f\n",
      BG_Buildable( ent->s.modelindex, NULL )->name,
      ent->s.pos.trBase[ 0 ],
      ent->s.pos.trBase[ 1 ],
      ent->s.pos.trBase[ 2 ],
      ent->s.angles[ 0 ],
      ent->s.angles[ 1 ],
      ent->s.angles[ 2 ],
      ent->s.origin2[ 0 ],
      ent->s.origin2[ 1 ],
      ent->s.origin2[ 2 ],
      ent->s.angles2[ 0 ],
      ent->s.angles2[ 1 ],
      ent->s.angles2[ 2 ] );
    trap_FS_Write( s, strlen( s ), f );
  }
  trap_FS_FCloseFile( f );
}

/*
============
G_LayoutList
============
*/
int G_LayoutList( const char *map, char *list, int len )
{
  // up to 128 single character layout names could fit in layouts
  char fileList[ ( MAX_CVAR_VALUE_STRING / 2 ) * 5 ] = {""};
  char layouts[ MAX_CVAR_VALUE_STRING ] = {""};
  int numFiles, i, fileLen = 0, listLen;
  int  count = 0;
  char *filePtr;

  Q_strcat( layouts, sizeof( layouts ), "*BUILTIN* " );
  numFiles = trap_FS_GetFileList( va( "layouts/%s", map ), ".dat",
    fileList, sizeof( fileList ) );
  filePtr = fileList;
  for( i = 0; i < numFiles; i++, filePtr += fileLen + 1 )
  {
    fileLen = strlen( filePtr );
    listLen = strlen( layouts );
    if( fileLen < 5 )
      continue;

    // list is full, stop trying to add to it
    if( ( listLen + fileLen ) >= sizeof( layouts ) )
      break;

    Q_strcat( layouts,  sizeof( layouts ), filePtr );
    listLen = strlen( layouts );

    // strip extension and add space delimiter
    layouts[ listLen - 4 ] = ' ';
    layouts[ listLen - 3 ] = '\0';
    count++;
  }
  if( count != numFiles )
  {
    G_Printf( S_COLOR_YELLOW "WARNING: layout list was truncated to %d "
      "layouts, but %d layout files exist in layouts/%s/.\n",
      count, numFiles, map );
  }
  Q_strncpyz( list, layouts, len );
  return count + 1;
}

/*
============
G_LayoutSelect

set level.layout based on g_layouts or g_layoutAuto
============
*/
void G_LayoutSelect( void )
{
  char fileName[ MAX_OSPATH ];
  char layouts[ MAX_CVAR_VALUE_STRING ];
  char layouts2[ MAX_CVAR_VALUE_STRING ];
  char *l;
  char map[ MAX_QPATH ];
  char *s;
  int cnt = 0;
  int layoutNum;

  Q_strncpyz( layouts, g_layouts.string, sizeof( layouts ) );
  trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );

  // one time use cvar
  trap_Cvar_Set( "g_layouts", "" );

  // pick an included layout at random if no list has been provided
  if( !layouts[ 0 ] && g_layoutAuto.integer )
  {
    G_LayoutList( map, layouts, sizeof( layouts ) );
  }

  if( !layouts[ 0 ] )
    return;

  Q_strncpyz( layouts2, layouts, sizeof( layouts2 ) );
  l = &layouts2[ 0 ];
  layouts[ 0 ] = '\0';
  while( 1 )
  {
    s = COM_ParseExt( &l, qfalse );
    if( !*s )
      break;

    if( !Q_stricmp( s, "*BUILTIN*" ) )
    {
      Q_strcat( layouts, sizeof( layouts ), s );
      Q_strcat( layouts, sizeof( layouts ), " " );
      cnt++;
      continue;
    }

    Com_sprintf( fileName, sizeof( fileName ), "layouts/%s/%s.dat", map, s );
    if( trap_FS_FOpenFile( fileName, NULL, FS_READ ) > 0 )
    {
      Q_strcat( layouts, sizeof( layouts ), s );
      Q_strcat( layouts, sizeof( layouts ), " " );
      cnt++;
    }
    else
      G_Printf( S_COLOR_YELLOW "WARNING: layout \"%s\" does not exist\n", s );
  }
  if( !cnt )
  {
      G_Printf( S_COLOR_RED "ERROR: none of the specified layouts could be "
        "found, using map default\n" );
      return;
  }
  layoutNum = rand( ) / ( RAND_MAX / cnt + 1 ) + 1;
  cnt = 0;

  Q_strncpyz( layouts2, layouts, sizeof( layouts2 ) );
  l = &layouts2[ 0 ];
  while( 1 )
  {
    s = COM_ParseExt( &l, qfalse );
    if( !*s )
      break;

    Q_strncpyz( level.layout, s, sizeof( level.layout ) );
    cnt++;
    if( cnt >= layoutNum )
      break;
  }
  G_Printf( "using layout \"%s\" from list (%s)\n", level.layout, layouts );
}

/*
============
G_LayoutBuildItem
============
*/
void G_LayoutBuildItem( buildable_t buildable, vec3_t origin,
  vec3_t angles, vec3_t origin2, vec3_t angles2 )
{
  gentity_t *builder;

  builder = G_Spawn( );
  builder->client = 0;
  VectorCopy( origin, builder->s.pos.trBase );
  VectorCopy( angles, builder->s.angles );
  VectorCopy( angles, builder->cuboidSize );
  VectorCopy( origin2, builder->s.origin2 );
  VectorCopy( angles2, builder->s.angles2 );
  G_SpawnBuildable( builder, buildable );
}

/*
============
G_LayoutLoad

load the layout .dat file indicated by level.layout and spawn buildables
as if a builder was creating them
============
*/
void G_LayoutLoad( void )
{
  fileHandle_t f;
  int len;
  char *layout, *layoutHead;
  char map[ MAX_QPATH ];
  char buildName[ MAX_TOKEN_CHARS ];
  int buildable;
  vec3_t origin = { 0.0f, 0.0f, 0.0f };
  vec3_t angles = { 0.0f, 0.0f, 0.0f };
  vec3_t origin2 = { 0.0f, 0.0f, 0.0f };
  vec3_t angles2 = { 0.0f, 0.0f, 0.0f };
  char line[ MAX_STRING_CHARS ];
  int i = 0;

  if( !level.layout[ 0 ] || !Q_stricmp( level.layout, "*BUILTIN*" ) )
    return;

  trap_Cvar_VariableStringBuffer( "mapname", map, sizeof( map ) );
  len = trap_FS_FOpenFile( va( "layouts/%s/%s.dat", map, level.layout ),
    &f, FS_READ );
  if( len < 0 )
  {
    G_Printf( "ERROR: layout %s could not be opened\n", level.layout );
    return;
  }
  layoutHead = layout = BG_Alloc( len + 1 );
  trap_FS_Read( layout, len, f );
  layout[ len ] = '\0';
  trap_FS_FCloseFile( f );
  while( *layout )
  {
    if( i >= sizeof( line ) - 1 )
    {
      G_Printf( S_COLOR_RED "ERROR: line overflow in %s before \"%s\"\n",
       va( "layouts/%s/%s.dat", map, level.layout ), line );
      break;
    }
    line[ i++ ] = *layout;
    line[ i ] = '\0';
    if( *layout == '\n' )
    {
      i = 0;
      sscanf( line, "%s %f %f %f %f %f %f %f %f %f %f %f %f\n",
        buildName,
        &origin[ 0 ], &origin[ 1 ], &origin[ 2 ],
        &angles[ 0 ], &angles[ 1 ], &angles[ 2 ],
        &origin2[ 0 ], &origin2[ 1 ], &origin2[ 2 ],
        &angles2[ 0 ], &angles2[ 1 ], &angles2[ 2 ] );

      buildable = BG_BuildableByName( buildName )->number;
      if( buildable <= BA_NONE || buildable >= BA_NUM_BUILDABLES )
        G_Printf( S_COLOR_YELLOW "WARNING: bad buildable name (%s) in layout."
          " skipping\n", buildName );
      else
        G_LayoutBuildItem( buildable, origin, angles, origin2, angles2 );
    }
    layout++;
  }
  BG_Free( layoutHead );
}

/*
============
G_BaseSelfDestruct
============
*/
void G_BaseSelfDestruct( team_t team )
{
  int       i;
  gentity_t *ent;

  for( i = MAX_CLIENTS; i < level.num_entities; i++ )
  {
    ent = &level.gentities[ i ];
    if( ent->health <= 0 )
      continue;
    if( ent->s.eType != ET_BUILDABLE )
      continue;
    if( ent->buildableTeam != team )
      continue;
    G_Damage( ent, NULL, NULL, NULL, NULL, 10000, 0, MOD_SUICIDE );
  }
}

/*
============
build log
============
*/
buildLog_t *G_BuildLogNew( gentity_t *actor, buildFate_t fate )
{
  buildLog_t *log = &level.buildLog[ level.buildId++ % MAX_BUILDLOG ];

  if( level.numBuildLogs < MAX_BUILDLOG )
    level.numBuildLogs++;
  log->time = level.time;
  log->fate = fate;
  log->actor = actor && actor->client ? actor->client->pers.namelog : NULL;
  return log;
}

void G_BuildLogSet( buildLog_t *log, gentity_t *ent )
{
  log->modelindex = ent->s.modelindex;
  VectorCopy( ent->s.pos.trBase, log->origin );
  VectorCopy( ent->s.angles, log->angles );
  VectorCopy( ent->s.origin2, log->origin2 );
  VectorCopy( ent->s.angles2, log->angles2 );
  log->powerSource = ent->parentNode ? ent->parentNode->s.modelindex : BA_NONE;
  log->powerValue = G_QueueValue( ent );
}

void G_BuildLogAuto( gentity_t *actor, gentity_t *buildable, buildFate_t fate )
{
  G_BuildLogSet( G_BuildLogNew( actor, fate ), buildable );
}

void G_BuildLogRevertThink( gentity_t *ent )
{
  gentity_t *built;
  vec3_t    mins, maxs;
  int       blockers[ MAX_GENTITIES ];
  int       num;
  int       victims = 0;
  int       i;

  if( ent->suicideTime > 0 )
  {
    BG_BuildableBoundingBox( ent->s.modelindex, mins, maxs );
    VectorAdd( ent->s.pos.trBase, mins, mins );
    VectorAdd( ent->s.pos.trBase, maxs, maxs );
    num = trap_EntitiesInBox( mins, maxs, blockers, MAX_GENTITIES );
    for( i = 0; i < num; i++ )
    {
      gentity_t *targ;
      vec3_t    push;

      targ = g_entities + blockers[ i ];
      if( targ->client )
      {
        float val = ( targ->client->ps.eFlags & EF_WALLCLIMB) ? 300.0 : 150.0;

        VectorSet( push, crandom() * val, crandom() * val, random() * val );
        VectorAdd( targ->client->ps.velocity, push, targ->client->ps.velocity );
        victims++;
      }
    }

    ent->suicideTime--;

    if( victims )
    {
      // still a blocker
      ent->nextthink = level.time + FRAMETIME;
      return;
    }
  }

  built = G_FinishSpawningBuildable( ent, qtrue );
  built->buildTime = built->s.time = 0;
  G_KillBox( built );

  G_LogPrintf( "revert: restore %d %s\n",
    built - g_entities, BG_Buildable( built->s.modelindex, NULL )->name );

  G_FreeEntity( ent );
}

void G_BuildLogRevert( int id )
{
  buildLog_t *log;
  gentity_t  *ent;
  int        i;
  vec3_t     dist;

  level.numBuildLogs -= level.buildId - id;
  while( level.buildId > id )
  {
    log = &level.buildLog[ --level.buildId % MAX_BUILDLOG ];
    if( log->fate == BF_CONSTRUCT )
    {
      for( i = MAX_CLIENTS; i < level.num_entities; i++ )
      {
        ent = &g_entities[ i ];
        if( ( ( ent->s.eType == ET_BUILDABLE &&
                ent->health > 0 ) ||
              ( ent->s.eType == ET_GENERAL &&
                ent->think == G_BuildLogRevertThink ) ) &&
            ent->s.modelindex == log->modelindex )
        {
          VectorSubtract( ent->s.pos.trBase, log->origin, dist );
          if( VectorLengthSquared( dist ) <= 2.0f )
          {
            if( ent->s.eType == ET_BUILDABLE )
              G_LogPrintf( "revert: remove %d %s\n",
                ent - g_entities, BG_Buildable( ent->s.modelindex, NULL )->name );
            G_FreeEntity( ent );
            break;
          }
        }
      }
    }
    else
    {
      gentity_t  *builder = G_Spawn();

      builder->client = NULL;
      VectorCopy( log->origin, builder->s.pos.trBase );
      VectorCopy( log->angles, builder->s.angles );
      VectorCopy( log->origin2, builder->s.origin2 );
      VectorCopy( log->angles2, builder->s.angles2 );
      builder->s.modelindex = log->modelindex;

      builder->think = G_BuildLogRevertThink;
      builder->nextthink = level.time + FRAMETIME;

      // Number of thinks before giving up and killing players in the way
      builder->suicideTime = 30; 

      if( log->fate == BF_DESTROY || log->fate == BF_TEAMKILL )
      {
        int value = log->powerValue;

        if( BG_Buildable( log->modelindex, NULL )->team == TEAM_ALIENS )
        {
          level.alienBuildPointQueue =
            MAX( 0, level.alienBuildPointQueue - value );
        }
      }
    }
  }
}

/*
============
G_RemoveUnbuiltBuildables

Kill all player's buildables if they havent spawned yet
============
*/
void G_RemoveUnbuiltBuildables( gentity_t *self )
{
  int i;
  vec3_t dir;
  gentity_t *ent;
  
  dir[0] = dir[1] = 0.0f;
  dir[2] = 1.0f;
   
  for( i = MAX_CLIENTS, ent = g_entities + i; i < level.num_entities; i++, ent++ )
  {
    if( ent->s.eType != ET_BUILDABLE )
      continue;

    if( ent == self )
      continue;

    if( ent->spawned )
      continue;

    if( ent->builtBy != self->client->ps.clientNum )
      continue;

    if( ent->s.modelindex < CUBOID_FIRST )
      continue;

    G_Damage( ent, self, NULL, dir, dir, ent->health, 0, MOD_DECONSTRUCT );
  }
}
