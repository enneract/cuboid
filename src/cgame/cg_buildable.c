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


#include "cg_local.h"

char *cg_buildableSoundNames[ MAX_BUILDABLE_ANIMATIONS ] =
{
  "construct1.wav",
  "construct2.wav",
  "idle1.wav",
  "idle2.wav",
  "idle3.wav",
  "attack1.wav",
  "attack2.wav",
  "spawn1.wav",
  "spawn2.wav",
  "pain1.wav",
  "pain2.wav",
  "destroy1.wav",
  "destroy2.wav",
  "destroyed.wav"
};

static sfxHandle_t defaultAlienSounds[ MAX_BUILDABLE_ANIMATIONS ];
static sfxHandle_t defaultHumanSounds[ MAX_BUILDABLE_ANIMATIONS ];


/*
======================
CG_DrawCuboid

cg_drawbbox for cuboids
======================
*/
static void CG_DrawCuboidFace( vec3_t a, vec3_t b, vec3_t c, vec3_t d, qhandle_t shader )
{
  polyVert_t  verts[ 4 ];
  vec4_t      color = { 255.0f, 255.0f, 255.0f, 255.0f };
  VectorCopy( d, verts[ 0 ].xyz );
  verts[ 0 ].st[ 0 ] = 1;
  verts[ 0 ].st[ 1 ] = 1;
  Vector4Copy( color, verts[ 0 ].modulate );
  VectorCopy( c, verts[ 1 ].xyz );
  verts[ 1 ].st[ 0 ] = 1;
  verts[ 1 ].st[ 1 ] = 0;
  Vector4Copy( color, verts[ 1 ].modulate );
  VectorCopy( b, verts[ 2 ].xyz );
  verts[ 2 ].st[ 0 ] = 0;
  verts[ 2 ].st[ 1 ] = 0;
  Vector4Copy( color, verts[ 2 ].modulate );
  VectorCopy( a, verts[ 3 ].xyz );
  verts[ 3 ].st[ 0 ] = 0;
  verts[ 3 ].st[ 1 ] = 1;
  Vector4Copy( color, verts[ 3 ].modulate );
  trap_R_AddPolyToScene( (shader ? shader : cgs.media.outlineShader ), 4, verts );
}

void CG_DrawCuboid( vec3_t origin, vec3_t dims, qhandle_t shader, int margin )
{
  vec3_t  mins, maxs;
  vec3_t  ppp, mpp, mmp, pmp;
  vec3_t  mmm, pmm, ppm, mpm;
  int i;
  BG_CuboidBBox(dims,mins,maxs);
  for(i=0;i<3&&margin;i++)
  {
   mins[i]-=margin;
   maxs[i]+=margin;
  }
  ppp[ 0 ] = origin[ 0 ] + maxs[ 0 ];
  ppp[ 1 ] = origin[ 1 ] + maxs[ 1 ];
  ppp[ 2 ] = origin[ 2 ] + maxs[ 2 ];
  mpp[ 0 ] = origin[ 0 ] + mins[ 0 ];
  mpp[ 1 ] = origin[ 1 ] + maxs[ 1 ];
  mpp[ 2 ] = origin[ 2 ] + maxs[ 2 ];
  mmp[ 0 ] = origin[ 0 ] + mins[ 0 ];
  mmp[ 1 ] = origin[ 1 ] + mins[ 1 ];
  mmp[ 2 ] = origin[ 2 ] + maxs[ 2 ];
  pmp[ 0 ] = origin[ 0 ] + maxs[ 0 ];
  pmp[ 1 ] = origin[ 1 ] + mins[ 1 ];
  pmp[ 2 ] = origin[ 2 ] + maxs[ 2 ];
  ppm[ 0 ] = origin[ 0 ] + maxs[ 0 ];
  ppm[ 1 ] = origin[ 1 ] + maxs[ 1 ];
  ppm[ 2 ] = origin[ 2 ] + mins[ 2 ];
  mpm[ 0 ] = origin[ 0 ] + mins[ 0 ];
  mpm[ 1 ] = origin[ 1 ] + maxs[ 1 ];
  mpm[ 2 ] = origin[ 2 ] + mins[ 2 ];
  mmm[ 0 ] = origin[ 0 ] + mins[ 0 ];
  mmm[ 1 ] = origin[ 1 ] + mins[ 1 ];
  mmm[ 2 ] = origin[ 2 ] + mins[ 2 ];
  pmm[ 0 ] = origin[ 0 ] + maxs[ 0 ];
  pmm[ 1 ] = origin[ 1 ] + mins[ 1 ];
  pmm[ 2 ] = origin[ 2 ] + mins[ 2 ];
  CG_DrawCuboidFace( ppp, mpp, mmp, pmp, shader );
  CG_DrawCuboidFace( ppp, pmp, pmm, ppm, shader );
  CG_DrawCuboidFace( mpp, ppp, ppm, mpm, shader );
  CG_DrawCuboidFace( mmp, mpp, mpm, mmm, shader );
  CG_DrawCuboidFace( pmp, mmp, mmm, pmm, shader );
  CG_DrawCuboidFace( mmm, mpm, ppm, pmm, shader );
}

void CG_DrawCuboidAxis(vec3_t cuboidOrigin, vec3_t size, int axis, qhandle_t shader)
{
 vec3_t origin, localZ, localX, localY, start, end, quad[2][4];
 float len, width;
 polyVert_t poly[4];
 
 VectorCopy(cuboidOrigin,origin);
 origin[2]+=size[2]/2.0f;
 VectorClear(localZ);
 VectorClear(localX);
 switch(axis)
 {
  case 0:
   localZ[0]=1.0f;
   localX[1]=1.0f;
   len=size[0];
   break;
  case 1:
   localZ[1]=1.0f;
   localX[2]=1.0f;
   len=size[1];
   break;
  default:
   localZ[2]=1.0f;
   localX[0]=1.0f;
   len=size[2];
   break;
 }
 CrossProduct(localZ,localX,localY);
 VectorMA(origin,-MAX(len/2.0f,32),localZ,start);
 VectorMA(origin,MAX(len/2.0f,32),localZ,end);
 VectorMA(start,4.0f,localX,quad[0][0]);
 VectorMA(start,-4.0f,localX,quad[0][1]);
 VectorMA(end,-4.0f,localX,quad[0][2]);
 VectorMA(end,4.0f,localX,quad[0][3]);
 VectorMA(start,4.0f,localY,quad[1][0]);
 VectorMA(start,-4.0f,localY,quad[1][1]);
 VectorMA(end,-4.0f,localY,quad[1][2]);
 VectorMA(end,4.0f,localY,quad[1][3]); 
 CG_DrawCuboidFace(quad[0][0],quad[0][1],quad[0][2],quad[0][3],shader);
 CG_DrawCuboidFace(quad[1][0],quad[1][1],quad[1][2],quad[1][3],shader);
}

/*
===================
CG_InitCuboidExplosions
CG_CuboidExplosion
CG_DrawCuboidParticles

Simple particle system based on cg_particles.c
===================
*/

#define MAX_CPARTICLES 4096
#define MAX_CPARTICLES_PER_EXPLOSION 128

typedef struct
{
 qboolean use;
 vec3_t origin;
 vec3_t velocity;
 float rot,rotspeed;
 float  size;
 qhandle_t shader;
 int lastphysics;
 int spawntime;
 qboolean solid;
} cuboidParticle_t;

cuboidParticle_t cuboidParticles[MAX_CPARTICLES];
int              cuboidParticlesCount=0;
qboolean         cuboidParticles_init=qfalse;

void CG_InitCuboidExplosions(void)
{
 memset(cuboidParticles,0,sizeof(cuboidParticles));
 cuboidParticles_init=qtrue;
 return;
}

void CG_CuboidExplosion(buildable_t buildable, vec3_t origin, vec3_t dims)
{
 int i,p,m,q;
 float v,s;
 cuboidParticle_t *pc;
 vec3_t sdims;
 const cuboidInfo_t *info;
 
 info=&cg_cuboids[buildable-CUBOID_FIRST];
 v=dims[0]*dims[1]*dims[2];
 for(i=0;i<info->destroySoundCount;i++)
  if(v*5.5306341e-5>=info->destroySoundThresholds[i])
  {
   trap_S_StartSound(origin,ENTITYNUM_WORLD,CHAN_AUTO,info->destroySounds[i]);
   break;
  }
  
  if(!info->fragmentCount)
   return;
  
 BG_CuboidSortSize(dims,sdims);
 
 q=cg_cuboidPSQuality.integer;
 
 if(q<=0)
  p=2;
 else if(q==1)
  p=ceil(sqrt(v/pow(sdims[2],3)))*2;
 else if(q==2)
  p=ceil(pow(v/pow(sdims[2],3),0.75f))*3;
 else if(q>=3)
  p=ceil(pow(v/pow(sdims[2],3),0.85f))*6;
 p=MIN(p,MAX_CPARTICLES_PER_EXPLOSION);
 
 s=ceil(pow(v/p,1.0f/3.0f))*1.2f;
 
 for(i=0;i<MAX_CPARTICLES&&p;i++)
 {
  pc=&cuboidParticles[i];
  if(pc->use)
   continue;
  pc->solid=qfalse;
  pc->origin[0]=origin[0]+crandom()*dims[0]*0.5f;
  pc->origin[1]=origin[1]+crandom()*dims[1]*0.5f;
  pc->origin[2]=origin[2]+dims[2]/2.0f+crandom()*dims[2]*0.5f;
  VectorSubtract(pc->origin,origin,pc->velocity);
  pc->size=s;
  pc->rot=crandom()*360.0f;
  pc->rotspeed=crandom()*450.0f;
  pc->shader=info->fragments[(int)(rand()%info->fragmentCount)];
  pc->lastphysics=pc->spawntime=cg.time;
  pc->use=qtrue;
  cuboidParticlesCount++;
  p--;
 }
}

void CG_CuboidParticlePhysics(cuboidParticle_t *pc)
{
 float t;
 int c;
 vec3_t newOrigin;
 t=(cg.time-pc->lastphysics)/1e3;
 pc->lastphysics=cg.time;
 if(pc->spawntime+3000<cg.time)
 {
  pc->use=qfalse;
  cuboidParticlesCount--;
  return;
 }
 if(pc->solid)
  return;
 pc->rot+=pc->rotspeed*t;
 pc->velocity[2]-=t*DEFAULT_GRAVITY;
 VectorMA(pc->origin,t,pc->velocity,newOrigin);
 c=trap_CM_PointContents(newOrigin,0);
 if(c&CONTENTS_NODROP)
 {
  pc->use=qfalse;
  cuboidParticlesCount--;
  return;
 }
 if(c&(CONTENTS_SOLID|CONTENTS_BODY))
 {
  pc->solid=qtrue;
  return;
 }
 VectorCopy(newOrigin,pc->origin);
}

void CG_DrawCuboidParticles(void)
{
 int i,j,drawn=0,c;
 float t;
 vec3_t alight, dlight, lightdir, newOrigin;
 cuboidParticle_t *pc;
 refEntity_t re;
  
 if(!cuboidParticles_init)
  CG_InitCuboidExplosions();
  
 for(i=0;i<MAX_CPARTICLES;i++)
 {
  pc=&cuboidParticles[i];
  
  if(!pc->use)
   continue;
  CG_CuboidParticlePhysics(pc);
  if(!pc->use) //particle physics can delete a particle
   continue;
  memset(&re,0,sizeof(refEntity_t));
  re.reType=RT_SPRITE;
  VectorCopy(pc->origin,re.origin);
  //trap_R_LightForPoint(pc->origin,alight,dlight,lightdir);
  //for(j=0;j<3;j++)
  // re.shaderRGBA[j]=(byte)alight[j];
  for(j=0;j<3;j++)
   re.shaderRGBA[j]=255;
  re.customShader=pc->shader;
  re.radius=pc->size;
  re.rotation=pc->rot;
  trap_R_AddRefEntityToScene(&re);
  drawn++;
 }
}


/*
===================
CG_AlienBuildableExplosion

Generated a bunch of gibs launching out from a location
===================
*/
void CG_AlienBuildableExplosion( vec3_t origin, vec3_t dir )
{
  particleSystem_t  *ps;

  trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.alienBuildableExplosion );

  //particle system
  ps = CG_SpawnNewParticleSystem( cgs.media.alienBuildableDestroyedPS );

  if( CG_IsParticleSystemValid( &ps ) )
  {
    CG_SetAttachmentPoint( &ps->attachment, origin );
    CG_SetParticleSystemNormal( ps, dir );
    CG_AttachToPoint( &ps->attachment );
  }
}

/*
=================
CG_HumanBuildableExplosion

Called for human buildables as they are destroyed
=================
*/
void CG_HumanBuildableExplosion( vec3_t origin, vec3_t dir )
{
  particleSystem_t  *ps;

  trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.humanBuildableExplosion );

  //particle system
  ps = CG_SpawnNewParticleSystem( cgs.media.humanBuildableDestroyedPS );

  if( CG_IsParticleSystemValid( &ps ) )
  {
    CG_SetAttachmentPoint( &ps->attachment, origin );
    CG_SetParticleSystemNormal( ps, dir );
    CG_AttachToPoint( &ps->attachment );
  }
}


#define CREEP_SIZE            64.0f
#define CREEP_DISTANCE        64.0f

/*
==================
CG_Creep
==================
*/
static void CG_Creep( centity_t *cent )
{
  int           msec;
  float         size, frac;
  trace_t       tr;
  vec3_t        temp, origin;
  int           scaleUpTime = BG_Buildable( cent->currentState.modelindex, cent->currentState.angles )->buildTime;
  int           time;

  if(BG_Buildable(cent->currentState.modelindex,NULL)->cuboid)
   return;
  
  time = cent->currentState.time;

  //should the creep be growing or receding?
  if( time >= 0 )
  {
    msec = cg.time - time;
    if( msec >= 0 && msec < scaleUpTime )
      frac = (float)msec / scaleUpTime;
    else
      frac = 1.0f;
  }
  else if( time < 0 )
  {
    msec = cg.time + time;
    if( msec >= 0 && msec < CREEP_SCALEDOWN_TIME )
      frac = 1.0f - ( (float)msec / CREEP_SCALEDOWN_TIME );
    else
      frac = 0.0f;
  }

  VectorCopy( cent->currentState.origin2, temp );
  VectorScale( temp, -CREEP_DISTANCE, temp );
  VectorAdd( temp, cent->lerpOrigin, temp );

  CG_Trace( &tr, cent->lerpOrigin, NULL, NULL, temp, cent->currentState.number, MASK_PLAYERSOLID );

  if(tr.entityNum!=ENTITYNUM_WORLD)
   return;
  
  VectorCopy( tr.endpos, origin );

  size = CREEP_SIZE * frac;

  if( size > 0.0f && tr.fraction < 1.0f )
    CG_ImpactMark( cgs.media.creepShader, origin, cent->currentState.origin2,
                   0.0f, 1.0f, 1.0f, 1.0f, 1.0f, qfalse, size, qtrue );
}

/*
======================
CG_ParseBuildableAnimationFile

Read a configuration file containing animation counts and rates
models/buildables/hivemind/animation.cfg, etc
======================
*/
static qboolean CG_ParseBuildableAnimationFile( const char *filename, buildable_t buildable )
{
  char          *text_p;
  int           len;
  int           i;
  char          *token;
  float         fps;
  char          text[ 20000 ];
  fileHandle_t  f;
  animation_t   *animations;
  
  if(BG_Buildable(buildable,NULL)->cuboid)
   return qtrue;

  animations = cg_buildables[ buildable ].animations;

  // load the file
  len = trap_FS_FOpenFile( filename, &f, FS_READ );
  if( len < 0 )
    return qfalse;

  if( len == 0 || len >= sizeof( text ) - 1 )
  {
    trap_FS_FCloseFile( f );
    CG_Printf( "File %s is %s\n", filename, len == 0 ? "empty" : "too long" );
    return qfalse;
  }

  trap_FS_Read( text, len, f );
  text[ len ] = 0;
  trap_FS_FCloseFile( f );

  // parse the text
  text_p = text;

  // read information for each frame
  for( i = BANIM_NONE + 1; i < MAX_BUILDABLE_ANIMATIONS; i++ )
  {

    token = COM_Parse( &text_p );
    if( !*token )
      break;

    animations[ i ].firstFrame = atoi( token );

    token = COM_Parse( &text_p );
    if( !*token )
      break;

    animations[ i ].numFrames = atoi( token );
    animations[ i ].reversed = qfalse;
    animations[ i ].flipflop = qfalse;

    // if numFrames is negative the animation is reversed
    if( animations[ i ].numFrames < 0 )
    {
      animations[ i ].numFrames = -animations[ i ].numFrames;
      animations[ i ].reversed = qtrue;
    }

    token = COM_Parse( &text_p );
    if ( !*token )
      break;

    animations[i].loopFrames = atoi( token );

    token = COM_Parse( &text_p );
    if( !*token )
      break;

    fps = atof( token );
    if( fps == 0 )
      fps = 1;

    animations[ i ].frameLerp = 1000 / fps;
    animations[ i ].initialLerp = 1000 / fps;
  }

  if( i != MAX_BUILDABLE_ANIMATIONS )
  {
    CG_Printf( "Error parsing animation file: %s\n", filename );
    return qfalse;
  }

  return qtrue;
}

/*
======================
CG_ParseBuildableSoundFile

Read a configuration file containing sound properties
sound/buildables/hivemind/sound.cfg, etc
======================
*/
static qboolean CG_ParseBuildableSoundFile( const char *filename, buildable_t buildable )
{
  char          *text_p;
  int           len;
  int           i;
  char          *token;
  char          text[ 20000 ];
  fileHandle_t  f;
  sound_t       *sounds;

  if(BG_Buildable(buildable,NULL)->cuboid)
   return qtrue;
  
  sounds = cg_buildables[ buildable ].sounds;

  // load the file
  len = trap_FS_FOpenFile( filename, &f, FS_READ );
  if ( len < 0 )
    return qfalse;

  if ( len == 0 || len >= sizeof( text ) - 1 )
  {
    trap_FS_FCloseFile( f );
    CG_Printf( "File %s is %s\n", filename, len == 0 ? "empty" : "too long" );
    return qfalse;
  }

  trap_FS_Read( text, len, f );
  text[len] = 0;
  trap_FS_FCloseFile( f );

  // parse the text
  text_p = text;

  // read information for each frame
  for( i = BANIM_NONE + 1; i < MAX_BUILDABLE_ANIMATIONS; i++ )
  {

    token = COM_Parse( &text_p );
    if ( !*token )
      break;

    sounds[ i ].enabled = atoi( token );

    token = COM_Parse( &text_p );
    if ( !*token )
      break;

    sounds[ i ].looped = atoi( token );

  }

  if( i != MAX_BUILDABLE_ANIMATIONS )
  {
    CG_Printf( "Error parsing sound file: %s\n", filename );
    return qfalse;
  }

  return qtrue;
}
/*
===============
CG_InitBuildables

Initialises the animation db
===============
*/
void CG_InitBuildables( void )
{
  char          filename[ MAX_QPATH ];
  char          soundfile[ MAX_QPATH ];
  char          *buildableName;
  const char    *cuboidName;
  char          *modelFile;
  int           i;
  int           j;
  fileHandle_t  f;
  pc_token_t    token;
  cuboidInfo_t  *cuboid;
  const char    *s;
  float         n;
 
  memset( cg_buildables, 0, sizeof( cg_buildables ) );

  //default sounds
  for( j = BANIM_NONE + 1; j < MAX_BUILDABLE_ANIMATIONS; j++ )
  {
    strcpy( soundfile, cg_buildableSoundNames[ j - 1 ] );

    Com_sprintf( filename, sizeof( filename ), "sound/buildables/alien/%s", soundfile );
    defaultAlienSounds[ j ] = trap_S_RegisterSound( filename, qfalse );

    Com_sprintf( filename, sizeof( filename ), "sound/buildables/human/%s", soundfile );
    defaultHumanSounds[ j ] = trap_S_RegisterSound( filename, qfalse );
  }

  cg.buildablesFraction = 0.0f;

  for( i = BA_NONE + 1; i < CUBOID_FIRST; i++ )
  {
    buildableName = BG_Buildable( i, NULL )->name;

    //animation.cfg
    Com_sprintf( filename, sizeof( filename ), "models/buildables/%s/animation.cfg", buildableName );
    if ( !CG_ParseBuildableAnimationFile( filename, i ) )
      Com_Printf( S_COLOR_YELLOW "WARNING: failed to load animation file %s\n", filename );

    //sound.cfg
    Com_sprintf( filename, sizeof( filename ), "sound/buildables/%s/sound.cfg", buildableName );
    if ( !CG_ParseBuildableSoundFile( filename, i ) )
      Com_Printf( S_COLOR_YELLOW "WARNING: failed to load sound file %s\n", filename );

    //models
    for( j = 0; j <= 3; j++ )
    {
      modelFile = BG_BuildableConfig( i )->models[ j ];
      if( strlen( modelFile ) > 0 )
        cg_buildables[ i ].models[ j ] = trap_R_RegisterModel( modelFile );
    }

    //sounds
    for( j = BANIM_NONE + 1; j < MAX_BUILDABLE_ANIMATIONS; j++ )
    {
      strcpy( soundfile, cg_buildableSoundNames[ j - 1 ] );
      Com_sprintf( filename, sizeof( filename ), "sound/buildables/%s/%s", buildableName, soundfile );

      if( cg_buildables[ i ].sounds[ j ].enabled )
      {
        if( trap_FS_FOpenFile( filename, &f, FS_READ ) > 0 )
        {
          //file exists so close it
          trap_FS_FCloseFile( f );

          cg_buildables[ i ].sounds[ j ].sound = trap_S_RegisterSound( filename, qfalse );
        }
        else
        {
          //file doesn't exist - use default
          if( BG_Buildable( i, NULL )->team == TEAM_ALIENS )
            cg_buildables[ i ].sounds[ j ].sound = defaultAlienSounds[ j ];
          else
            cg_buildables[ i ].sounds[ j ].sound = defaultHumanSounds[ j ];
        }
      }
    }

    cg.buildablesFraction = (float)i / (float)( BA_NUM_BUILDABLES - 1 );
    trap_UpdateScreen( );
  }

  cgs.media.teslaZapTS = CG_RegisterTrailSystem( "models/buildables/tesla/zap" );
  
  memset( cg_cuboids, 0, sizeof( cg_cuboids ) );
  
  for( i = CUBOID_FIRST; i <= CUBOID_LAST; i++ )
  {
    cuboid = &cg_cuboids[ i - CUBOID_FIRST ];
    cuboidName = BG_CuboidAttributes( i )->file;
    
    Com_sprintf( filename, sizeof( filename ), "configs/cuboids/%s.cfg", cuboidName ); 
    if( !( f = trap_Parse_LoadSource( filename ) ) )
    {
      Com_Printf( S_COLOR_YELLOW "WARNING: failed to load cuboid config %s\n", filename );
      continue;
    }
    while( 1 )
    {
      if( !trap_Parse_ReadToken( f, &token ) )
        break;
      else if( !Q_stricmp( token.string, "texture" ) )
      {
        if( cuboid->textureCount == MAX_CUBOID_TEXTURES )
        {
          Com_Printf( S_COLOR_RED "ERROR: too many cuboid textures in %s\n", filename );
          break;
        }
        if( trap_Parse_ReadToken( f, &token ) )
        {
          cuboid->textureThresholds[ cuboid->textureCount++ ] = atof( token.string );
          if( trap_Parse_ReadToken( f, &token ) )
            cuboid->textures[ cuboid->textureCount - 1 ] = trap_R_RegisterShader( token.string );
          else
          {
            Com_Printf( S_COLOR_YELLOW "WARNING: incomplete cuboid texture definition in %s\n", filename );
            cuboid->textureCount--;
          }
        }
      }
      else if( !Q_stricmp( token.string, "useCracks" ) )
        cuboid->useCracks=qtrue;
      else if( !Q_stricmp( token.string, "fragment")  )
      {
        if( cuboid->fragmentCount == MAX_CUBOID_FRAGMENTS )
        {
          Com_Printf( S_COLOR_RED "ERROR: too many cuboid fragments in %s\n", filename );
          break;
        }
        if( trap_Parse_ReadToken( f, &token ) )
          cuboid->fragments[ cuboid->fragmentCount++ ] = trap_R_RegisterShader( token.string );
      }
      else if( !Q_stricmp( token.string, "painSound" ) )
      {
        if( cuboid->painSoundCount == MAX_CUBOID_SOUNDS )
        {
          Com_Printf( S_COLOR_RED "ERROR: too many cuboid pain sounds in %s\n", filename );
          break;
        }
        if( trap_Parse_ReadToken( f, &token ) )
          cuboid->painSounds[ cuboid->painSoundCount++ ] = trap_S_RegisterSound( token.string, qfalse );
      }
      else if( !Q_stricmp( token.string, "destroySound" ) )
      {
        if( cuboid->destroySoundCount == MAX_CUBOID_SOUNDS )
        {
          Com_Printf( S_COLOR_RED "ERROR: too many cuboid destroy sounds in %s\n", filename );
          break;
        }
        if( trap_Parse_ReadToken( f, &token ) )
        {
          cuboid->destroySoundThresholds[ cuboid->destroySoundCount++ ] = atof( token.string );
          if( trap_Parse_ReadToken( f, &token ) )
            cuboid->destroySounds[ cuboid->destroySoundCount - 1 ] = trap_S_RegisterSound( token.string, qfalse );
          else
          {
            Com_Printf( S_COLOR_YELLOW "WARNING: incomplete cuboid destroy sound definition in %s\n", filename );
            cuboid->destroySoundCount--;
          }
        }
      }
      else
        Com_Printf( S_COLOR_YELLOW "WARNING: invalid token %s in %s\n", token.string, filename );
    }
    trap_Parse_FreeSource( f );
  }
}

/*
===============
CG_SetBuildableLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
static void CG_SetBuildableLerpFrameAnimation( buildable_t buildable, lerpFrame_t *lf, int newAnimation )
{
  animation_t *anim;

  lf->animationNumber = newAnimation;

  if( newAnimation < 0 || newAnimation >= MAX_BUILDABLE_ANIMATIONS )
    CG_Error( "Bad animation number: %i", newAnimation );

  anim = &cg_buildables[ buildable ].animations[ newAnimation ];

  //this item has just spawned so lf->frameTime will be zero
  if( !lf->animation )
    lf->frameTime = cg.time + 1000; //1 sec delay before starting the spawn anim

  lf->animation = anim;
  lf->animationTime = lf->frameTime + anim->initialLerp;

  if( cg_debugAnim.integer )
    CG_Printf( "Anim: %i\n", newAnimation );
}

/*
===============
CG_RunBuildableLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
static void CG_RunBuildableLerpFrame( centity_t *cent )
{
  buildable_t           buildable = cent->currentState.modelindex;
  lerpFrame_t           *lf = &cent->lerpFrame;
  buildableAnimNumber_t newAnimation = cent->buildableAnim & ~( ANIM_TOGGLEBIT|ANIM_FORCEBIT );

  // see if the animation sequence is switching
  if( newAnimation != lf->animationNumber || !lf->animation )
  {
    if( cg_debugRandom.integer )
      CG_Printf( "newAnimation: %d lf->animationNumber: %d lf->animation: %d\n",
                 newAnimation, lf->animationNumber, lf->animation );

    CG_SetBuildableLerpFrameAnimation( buildable, lf, newAnimation );

    if( !cg_buildables[ buildable ].sounds[ newAnimation ].looped &&
        cg_buildables[ buildable ].sounds[ newAnimation ].enabled )
    {
      if( cg_debugRandom.integer )
        CG_Printf( "Sound for animation %d for a %s\n",
            newAnimation, BG_Buildable( buildable, NULL )->humanName );

      trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_AUTO,
        cg_buildables[ buildable ].sounds[ newAnimation ].sound );
    }
  }

  if( cg_buildables[ buildable ].sounds[ lf->animationNumber ].looped &&
      cg_buildables[ buildable ].sounds[ lf->animationNumber ].enabled )
    trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin,
      cg_buildables[ buildable ].sounds[ lf->animationNumber ].sound );

  CG_RunLerpFrame( lf, 1.0f );

  // animation ended
  if( lf->frameTime == cg.time )
  {
    cent->buildableAnim = cent->currentState.torsoAnim;
    cent->buildableIdleAnim = qtrue;
  }
}

/*
===============
CG_BuildableAnimation
===============
*/
static void CG_BuildableAnimation( centity_t *cent, int *old, int *now, float *backLerp )
{
  entityState_t *es = &cent->currentState;

  //if no animation is set default to idle anim
  if( cent->buildableAnim == BANIM_NONE )
  {
    cent->buildableAnim = es->torsoAnim;
    cent->buildableIdleAnim = qtrue;
  }

  //display the first frame of the construction anim if not yet spawned
  if( !( es->eFlags & EF_B_SPAWNED ) )
  {
    animation_t *anim = &cg_buildables[ es->modelindex ].animations[ BANIM_CONSTRUCT1 ];

    //so that when animation starts for real it has sensible numbers
    cent->lerpFrame.oldFrameTime =
      cent->lerpFrame.frameTime =
      cent->lerpFrame.animationTime =
      cg.time;

    *old      = cent->lerpFrame.oldFrame = anim->firstFrame;
    *now      = cent->lerpFrame.frame    = anim->firstFrame;
    *backLerp = cent->lerpFrame.backlerp = 0.0f;

    //ensure that an animation is triggered once the buildable has spawned
    cent->oldBuildableAnim = BANIM_NONE;
  }
  else
  {
    if( ( cent->oldBuildableAnim ^ es->legsAnim ) & ANIM_TOGGLEBIT )
    {
      if( cg_debugAnim.integer )
        CG_Printf( "%d->%d l:%d t:%d %s(%d)\n",
                   cent->oldBuildableAnim, cent->buildableAnim,
                   es->legsAnim, es->torsoAnim,
                   BG_Buildable( es->modelindex, NULL )->humanName, es->number );

      if( cent->buildableAnim == es->torsoAnim || es->legsAnim & ANIM_FORCEBIT )
      {
        cent->buildableAnim = cent->oldBuildableAnim = es->legsAnim;
        cent->buildableIdleAnim = qfalse;
      }
      else
      {
        cent->buildableAnim = cent->oldBuildableAnim = es->torsoAnim;
        cent->buildableIdleAnim = qtrue;
      }
    }
    else if( cent->buildableIdleAnim == qtrue &&
             cent->buildableAnim != es->torsoAnim )
    {
      cent->buildableAnim = es->torsoAnim;
    }

    CG_RunBuildableLerpFrame( cent );

    *old      = cent->lerpFrame.oldFrame;
    *now      = cent->lerpFrame.frame;
    *backLerp = cent->lerpFrame.backlerp;
  }
}

#define TRACE_DEPTH 64.0f

/*
===============
CG_PositionAndOrientateBuildable
===============
*/
static void CG_PositionAndOrientateBuildable( const vec3_t angles, const vec3_t inOrigin,
                                              const vec3_t normal, const int skipNumber,
                                              const vec3_t mins, const vec3_t maxs,
                                              vec3_t outAxis[ 3 ], vec3_t outOrigin,
					      qboolean cuboid )
{
  vec3_t  forward, start, end;
  trace_t tr, box_tr;
  float mag, fraction;

  AngleVectors( angles, forward, NULL, NULL );
  VectorCopy( normal, outAxis[ 2 ] );
  ProjectPointOnPlane( outAxis[ 0 ], forward, outAxis[ 2 ] );

  if( !VectorNormalize( outAxis[ 0 ] ) )
  {
    if(cuboid)
      forward[0]=forward[1]=0.0f,forward[2]=1.0f;
    else
      AngleVectors( angles, NULL, NULL, forward );
    ProjectPointOnPlane( outAxis[ 0 ], forward, outAxis[ 2 ] );
    VectorNormalize( outAxis[ 0 ] );
  }

  CrossProduct( outAxis[ 0 ], outAxis[ 2 ], outAxis[ 1 ] );
  outAxis[ 1 ][ 0 ] = -outAxis[ 1 ][ 0 ];
  outAxis[ 1 ][ 1 ] = -outAxis[ 1 ][ 1 ];
  outAxis[ 1 ][ 2 ] = -outAxis[ 1 ][ 2 ];

  if(cuboid)
  {
    VectorCopy(inOrigin,outOrigin);
    return;
  }
  VectorMA( inOrigin, -TRACE_DEPTH, normal, end );
  VectorMA( inOrigin, 1.0f, normal, start );

  // Take both capsule and box traces. If the capsule trace does not differ
  //  significantly from the box trace use it. This may cause buildables to be
  //  positioned *inside* the surface on which it is placed. This is intentional

  CG_CapTrace( &tr, start, mins, maxs, end, skipNumber,
               MASK_PLAYERSOLID );

  if(tr.contents&(~MASK_SOLID)) //if we hit a player or another buildable then display buildable exactly at its origin
   VectorCopy(inOrigin,outOrigin);
  else
  {
   CG_Trace( &box_tr, start, mins, maxs, end, skipNumber,
             MASK_PLAYERSOLID );

   mag = Distance( tr.endpos, box_tr.endpos );

   fraction = tr.fraction;

   // this is either too far off of the bbox to be useful for gameplay purposes
   //  or the model is positioned in thin air anyways.
   if( mag > 15.0f || tr.fraction == 1.0f )
     fraction = box_tr.fraction; 

   VectorMA( inOrigin, fraction * -TRACE_DEPTH, normal, outOrigin );
  }
}

/*
==================
CG_GhostBuildable
==================
*/
void CG_Cuboid_Info(void);
void CG_GhostBuildable( buildable_t buildable, vec3_t dims )
{
  refEntity_t     ent;
  playerState_t   *ps;
  vec3_t          angles, entity_origin;
  vec3_t          mins, maxs;
  trace_t         tr;
  float           scale;
  vec3_t          viewangles;

  ps = &cg.predictedPlayerState;

  memset( &ent, 0, sizeof( ent ) );
  
  if(BG_Buildable(buildable,NULL)->cuboid)
  {
   BG_CuboidBBox(dims,mins,maxs);
   CG_Cuboid_Info();
  }
  else BG_BuildableBoundingBox( buildable, mins, maxs );

  if(!BG_PositionBuildableRelativeToPlayer( ps, BG_Buildable(buildable,NULL)->cuboid, mins, maxs, CG_Trace, entity_origin, angles, &tr ))
    return;

  if(BG_Buildable(buildable, NULL)->cuboid)
  { 
   qhandle_t shader, ashader;
  
   CG_Cuboid_Send(); //NOTE: CG_Cuboid_Send has its own timer so we don't spam server with commands every frame
   
   if(cg.forbidCuboids)
    shader=cgs.media.cuboidYellowBuildShader;
   else if(ps->stats[STAT_BUILDABLE]&SB_VALID_TOGGLEBIT)
    shader=cgs.media.cuboidGreenBuildShader;
   else
    shader=cgs.media.cuboidRedBuildShader;
   CG_DrawCuboid(entity_origin,dims,shader,0);
   CG_DrawCuboidAxis(entity_origin,dims,cg_cuboidResizeAxis.integer,cgs.media.cuboidAxis);
   return;
  }

  VectorCopy(ps->viewangles,viewangles);
  
  CG_PositionAndOrientateBuildable( viewangles, entity_origin, tr.plane.normal, ps->clientNum,
                                    mins, maxs, ent.axis, ent.origin, qfalse );

  //offset on the Z axis if required

  VectorMA( ent.origin, BG_BuildableConfig( buildable )->zOffset, tr.plane.normal, ent.origin );

  VectorCopy( ent.origin, ent.lightingOrigin );
  VectorCopy( ent.origin, ent.oldorigin ); // don't positionally lerp at all

  ent.hModel = cg_buildables[ buildable ].models[ 0 ];

  if( ps->stats[ STAT_BUILDABLE ] & SB_VALID_TOGGLEBIT )
    ent.customShader = cgs.media.greenBuildShader;
  else
    ent.customShader = cgs.media.redBuildShader;

  ent.nonNormalizedAxes = qfalse;

  //rescale the model
  scale = BG_BuildableConfig( buildable )->modelScale;

  if( scale != 1.0f )
  {
    VectorScale( ent.axis[ 0 ], scale, ent.axis[ 0 ] );
    VectorScale( ent.axis[ 1 ], scale, ent.axis[ 1 ] );
    VectorScale( ent.axis[ 2 ], scale, ent.axis[ 2 ] );

    ent.nonNormalizedAxes = qtrue;
  }

  // add to refresh list
  trap_R_AddRefEntityToScene( &ent );
}

/*
==================
CG_BuildableParticleEffects
==================
*/
static void CG_BuildableParticleEffects( centity_t *cent )
{
  entityState_t   *es = &cent->currentState;
  team_t          team = BG_Buildable( es->modelindex, NULL )->team;
  int             health = es->generic1;
  float           healthFrac = (float)health / BG_Buildable( es->modelindex, es->angles )->health;

  if( !( es->eFlags & EF_B_SPAWNED ) )
    return;

  if( team == TEAM_HUMANS )
  {
    if( healthFrac < 0.33f && !CG_IsParticleSystemValid( &cent->buildablePS ) )
    {
      cent->buildablePS = CG_SpawnNewParticleSystem( cgs.media.humanBuildableDamagedPS );

      if( CG_IsParticleSystemValid( &cent->buildablePS ) )
      {
        CG_SetAttachmentCent( &cent->buildablePS->attachment, cent );
        CG_AttachToCent( &cent->buildablePS->attachment );
      }
    }
    else if( healthFrac >= 0.33f && CG_IsParticleSystemValid( &cent->buildablePS ) )
      CG_DestroyParticleSystem( &cent->buildablePS );
  }
  else if( team == TEAM_ALIENS )
  {
    if( healthFrac < 0.33f && !CG_IsParticleSystemValid( &cent->buildablePS ) )
    {
      cent->buildablePS = CG_SpawnNewParticleSystem( cgs.media.alienBuildableDamagedPS );

      if( CG_IsParticleSystemValid( &cent->buildablePS ) )
      {
        CG_SetAttachmentCent( &cent->buildablePS->attachment, cent );
        CG_SetParticleSystemNormal( cent->buildablePS, es->origin2 );
        CG_AttachToCent( &cent->buildablePS->attachment );
      }
    }
    else if( healthFrac >= 0.33f && CG_IsParticleSystemValid( &cent->buildablePS ) )
      CG_DestroyParticleSystem( &cent->buildablePS );
  }
}

/*
==================
CG_BuildableStatusParse
==================
*/
void CG_BuildableStatusParse( const char *filename, buildStat_t *bs )
{
  pc_token_t token;
  int        handle;
  const char *s;
  int        i;
  float      f;
  vec4_t     c;

  handle = trap_Parse_LoadSource( filename );
  if( !handle )
    return;
  while( 1 )
  {
    if( !trap_Parse_ReadToken( handle, &token ) )
      break;
    if( !Q_stricmp( token.string, "frameShader" ) )
    {
      if( PC_String_Parse( handle, &s ) )
        bs->frameShader = trap_R_RegisterShader( s );
      continue;
    }
    else if( !Q_stricmp( token.string, "overlayShader" ) )
    {
      if( PC_String_Parse( handle, &s ) )
        bs->overlayShader = trap_R_RegisterShader( s );
      continue;
    }
    else if( !Q_stricmp( token.string, "noPowerShader" ) )
    {
      if( PC_String_Parse( handle, &s ) )
        bs->noPowerShader = trap_R_RegisterShader( s );
      continue;
    }
    else if( !Q_stricmp( token.string, "markedShader" ) )
    {
      if( PC_String_Parse( handle, &s ) )
        bs->markedShader = trap_R_RegisterShader( s );
      continue;
    }
    else if( !Q_stricmp( token.string, "healthSevereColor" ) )
    {
      if( PC_Color_Parse( handle, &c ) )
        Vector4Copy( c, bs->healthSevereColor );
      continue;
    }
    else if( !Q_stricmp( token.string, "healthHighColor" ) )
    {
      if( PC_Color_Parse( handle, &c ) )
        Vector4Copy( c, bs->healthHighColor );
      continue;
    }
    else if( !Q_stricmp( token.string, "healthElevatedColor" ) )
    {
      if( PC_Color_Parse( handle, &c ) )
        Vector4Copy( c, bs->healthElevatedColor );
      continue;
    }
    else if( !Q_stricmp( token.string, "healthGuardedColor" ) )
    {
      if( PC_Color_Parse( handle, &c ) )
        Vector4Copy( c, bs->healthGuardedColor );
      continue;
    }
    else if( !Q_stricmp( token.string, "healthLowColor" ) )
    {
      if( PC_Color_Parse( handle, &c ) )
        Vector4Copy( c, bs->healthLowColor );
      continue;
    }
    else if( !Q_stricmp( token.string, "foreColor" ) )
    {
      if( PC_Color_Parse( handle, &c ) )
        Vector4Copy( c, bs->foreColor );
      continue;
    }
    else if( !Q_stricmp( token.string, "backColor" ) )
    {
      if( PC_Color_Parse( handle, &c ) )
        Vector4Copy( c, bs->backColor );
      continue;
    }
    else if( !Q_stricmp( token.string, "frameHeight" ) )
    {
      if( PC_Int_Parse( handle, &i ) )
        bs->frameHeight = i;
      continue;
    }
    else if( !Q_stricmp( token.string, "frameWidth" ) )
    {
      if( PC_Int_Parse( handle, &i ) )
        bs->frameWidth = i;
      continue;
    }
    else if( !Q_stricmp( token.string, "healthPadding" ) )
    {
      if( PC_Int_Parse( handle, &i ) )
        bs->healthPadding = i;
      continue;
    }
    else if( !Q_stricmp( token.string, "overlayHeight" ) )
    {
      if( PC_Int_Parse( handle, &i ) )
        bs->overlayHeight = i;
      continue;
    }
    else if( !Q_stricmp( token.string, "overlayWidth" ) )
    {
      if( PC_Int_Parse( handle, &i ) )
        bs->overlayWidth = i;
      continue;
    }
    else if( !Q_stricmp( token.string, "verticalMargin" ) )
    {
      if( PC_Float_Parse( handle, &f ) )
        bs->verticalMargin = f;
      continue;
    }
    else if( !Q_stricmp( token.string, "horizontalMargin" ) )
    {
      if( PC_Float_Parse( handle, &f ) )
        bs->horizontalMargin = f;
      continue;
    }
    else
    {
      Com_Printf("CG_BuildableStatusParse: unknown token %s in %s\n",
        token.string, filename );
      bs->loaded = qfalse;
      trap_Parse_FreeSource( handle );
      return;
    }
  }
  bs->loaded = qtrue;
  trap_Parse_FreeSource( handle );
}

#define STATUS_FADE_TIME      200
#define STATUS_MAX_VIEW_DIST  900.0f
#define STATUS_PEEK_DIST      20
/*
==================
CG_BuildableStatusDisplay
==================
*/
static void CG_BuildableStatusDisplay( centity_t *cent, qboolean cuboid, vec3_t trac )
{
  entityState_t   *es = &cent->currentState;
  vec3_t          origin;
  float           healthScale;
  int             health;
  float           x, y;
  vec4_t          color;
  qboolean        powered, marked;
  trace_t         tr;
  float           d;
  buildStat_t     *bs;
  int             i, j;
  int             entNum;
  vec3_t          trOrigin;
  vec3_t          right;
  qboolean        visible = qfalse;
  vec3_t          mins, maxs;
  entityState_t   *hit;
  int             anim;
  char            buf[12];
  int             bufl;

  if( BG_Buildable( es->modelindex, NULL )->team == TEAM_ALIENS )
    bs = &cgs.alienBuildStat;
  else
    bs = &cgs.humanBuildStat;

  if( !bs->loaded )
    return;

  d = Distance( cent->lerpOrigin, cg.refdef.vieworg );
  if( d > STATUS_MAX_VIEW_DIST )
    return;

  Vector4Copy( bs->foreColor, color );

  // trace for center point
  if(BG_Buildable(es->modelindex, NULL)->cuboid)
    BG_CuboidBBox( es->angles, mins, maxs );
  else
    BG_BuildableBoundingBox( es->modelindex, mins, maxs );

  // hack for shrunken barricades
  anim = es->torsoAnim & ~( ANIM_FORCEBIT | ANIM_TOGGLEBIT );
  if( es->modelindex == BA_A_BARRICADE &&
      ( anim == BANIM_DESTROYED || !( es->eFlags & EF_B_SPAWNED ) ) )
    maxs[ 2 ] = (int)( maxs[ 2 ] * BARRICADE_SHRINKPROP );

  VectorCopy( cent->lerpOrigin, origin );

  // center point
  origin[ 2 ] += mins[ 2 ];
  origin[ 2 ] += ( abs( mins[ 2 ] ) + abs( maxs[ 2 ] ) ) / 2;

  entNum = cg.predictedPlayerState.clientNum;

  // if first try fails, step left, step right
  for( j = 0; j < 3 && !cuboid; j++ )
  {
    VectorCopy( cg.refdef.vieworg, trOrigin );
    switch( j )
    {
      case 1:
        // step right
        AngleVectors( cg.refdefViewAngles, NULL, right, NULL );
        VectorMA( trOrigin, STATUS_PEEK_DIST, right, trOrigin );
        break;
      case 2:
        // step left
        AngleVectors( cg.refdefViewAngles, NULL, right, NULL );
        VectorMA( trOrigin, -STATUS_PEEK_DIST, right, trOrigin );
        break;
      default:
        break;
    }
    // look through up to 3 players and/or transparent buildables
    for( i = 0; i < 3; i++ )
    {
      CG_Trace( &tr, trOrigin, NULL, NULL, origin, entNum, MASK_SHOT );
      if( tr.entityNum == cent->currentState.number )
      {
        visible = qtrue;
        break;
      }

      if( tr.entityNum == ENTITYNUM_WORLD )
        break;

      hit  = &cg_entities[ tr.entityNum ].currentState;

      if( tr.entityNum < MAX_CLIENTS || ( hit->eType == ET_BUILDABLE &&
          ( !( es->eFlags & EF_B_SPAWNED ) ||
            BG_Buildable( hit->modelindex, NULL )->transparentTest ) ) )
      {
        entNum = tr.entityNum;
        VectorCopy( tr.endpos, trOrigin );
      }
      else
        break;
    }
  }
  
  if(cuboid)
    visible=qtrue;
  
  // hack to make the kit obscure view
  if( cg_drawGun.integer && visible &&
      cg.predictedPlayerState.stats[ STAT_TEAM ] == TEAM_HUMANS &&
      CG_WorldToScreen( origin, &x, &y ) )
  {
    if( x > 450 && y > 290 )
      visible = qfalse;
  }

  if( !visible && cent->buildableStatus.visible )
  {
    cent->buildableStatus.visible   = qfalse;
    cent->buildableStatus.lastTime  = cg.time;
  }
  else if( visible && !cent->buildableStatus.visible )
  {
    cent->buildableStatus.visible   = qtrue;
    cent->buildableStatus.lastTime  = cg.time;
  }

  // Fade up
  if( cent->buildableStatus.visible )
  {
    if( cent->buildableStatus.lastTime + STATUS_FADE_TIME > cg.time )
      color[ 3 ] = (float)( cg.time - cent->buildableStatus.lastTime ) / STATUS_FADE_TIME;
  }

  // Fade down
  if( !cent->buildableStatus.visible )
  {
    if( cent->buildableStatus.lastTime + STATUS_FADE_TIME > cg.time )
      color[ 3 ] = 1.0f - (float)( cg.time - cent->buildableStatus.lastTime ) / STATUS_FADE_TIME;
    else
      return;
  }

  health = es->generic1;
  if(cuboid)
    health = BG_CuboidUnpackHealth(es);
  healthScale = (float)health / BG_Buildable( es->modelindex, es->angles )->health;

  if( health > 0 && healthScale < 0.01f )
    healthScale = 0.01f;
  else if( healthScale < 0.0f )
    healthScale = 0.0f;
  else if( healthScale > 1.0f )
    healthScale = 1.0f;

  if(cuboid)
  {
    x=320;
    y=240;
    d=Distance(cg.refdef.vieworg,trac);
    if(d<64.0f)
     d=64.0f;
  }
  else
    if( !CG_WorldToScreen( origin, &x, &y ) )
      return;
 
  {
    float  picH = bs->frameHeight;
    float  picW = bs->frameWidth;
    float  picX = x;
    float  picY = y;
    float  scale;
    float  subH, subY;
    float  clipX, clipY, clipW, clipH;
    vec4_t frameColor;

    // this is fudged to get the width/height in the cfg to be more realistic
    scale = ( picH / d ) * 3;

    powered = es->eFlags & EF_B_POWERED;
    marked = es->eFlags & EF_B_MARKED;

    picH *= scale;
    picW *= scale;
    picX -= ( picW * 0.5f );
    picY -= ( picH * 0.5f );

    // sub-elements such as icons and number
    subH = picH - ( picH * bs->verticalMargin );
    subY = picY + ( picH * 0.5f ) - ( subH * 0.5f );

    clipW = ( 640.0f * cg_viewsize.integer ) / 100.0f;
    clipH = ( 480.0f * cg_viewsize.integer ) / 100.0f;
    clipX = 320.0f - ( clipW * 0.5f );
    clipY = 240.0f - ( clipH * 0.5f );
    CG_SetClipRegion( clipX, clipY, clipW, clipH );

    if( bs->frameShader )
    {
      Vector4Copy( bs->backColor, frameColor );
      frameColor[ 3 ] = color[ 3 ];
      trap_R_SetColor( frameColor );
      CG_DrawPic( picX, picY, picW, picH, bs->frameShader );
      trap_R_SetColor( NULL );
    }

    if( health > 0 )
    {
      float hX, hY, hW, hH;
      vec4_t healthColor;

      hX = picX + ( bs->healthPadding * scale );
      hY = picY + ( bs->healthPadding * scale );
      hH = picH - ( bs->healthPadding * 2.0f * scale );
      hW = picW * healthScale - ( bs->healthPadding * 2.0f * scale );

      if( healthScale == 1.0f )
        Vector4Copy( bs->healthLowColor, healthColor );
      else if( healthScale >= 0.75f )
        Vector4Copy( bs->healthGuardedColor, healthColor );
      else if( healthScale >= 0.50f )
        Vector4Copy( bs->healthElevatedColor, healthColor );
      else if( healthScale >= 0.25f )
        Vector4Copy( bs->healthHighColor, healthColor );
      else
        Vector4Copy( bs->healthSevereColor, healthColor );

      healthColor[ 3 ] = color[ 3 ];
      trap_R_SetColor( healthColor );

      CG_DrawPic( hX, hY, hW, hH, cgs.media.whiteShader );
      trap_R_SetColor( NULL );
    }

    if( bs->overlayShader )
    {
      float oW = bs->overlayWidth;
      float oH = bs->overlayHeight;
      float oX = x;
      float oY = y;

      oH *= scale;
      oW *= scale;
      oX -= ( oW * 0.5f );
      oY -= ( oH * 0.5f );

      trap_R_SetColor( frameColor );
      CG_DrawPic( oX, oY, oW, oH, bs->overlayShader );
      trap_R_SetColor( NULL );
    }

    trap_R_SetColor( color );
    if( !powered )
    {
      float pX;

      pX = picX + ( subH * bs->horizontalMargin );
      CG_DrawPic( pX, subY, subH, subH, bs->noPowerShader );
    }

    if( marked )
    {
      float mX;

      mX = picX + picW - ( subH * bs->horizontalMargin ) - subH;
      CG_DrawPic( mX, subY, subH, subH, bs->markedShader );
    }

    //NOTE: dont use CG_DrawField, too few digits
    {
      float nX,cW,cH;
      int healthMax;
      int healthPoints;
      int frame;
      
      healthMax = BG_Buildable( es->modelindex, es->angles )->health;
      healthPoints = (int)( healthScale * healthMax );
      if( health > 0 && healthPoints < 1 )
        healthPoints = 1;
      
      Com_sprintf(buf,sizeof(buf),"%i",healthPoints);
      bufl=strlen(buf);
      cW=subH*cgDC.aspectScale;
      cH=subH;
      nX=picX+picW*0.5f-cW*bufl*0.5f;
      
      for(i=0;i<bufl;i++)
      {
       if(buf[i]=='-')
        frame=STAT_MINUS;
       else
        frame=buf[i]-'0';
       CG_DrawPic(nX+i*cW,y+bs->verticalMargin-subH*0.5f,cW,cH,cgs.media.numberShaders[frame]);
      }
    }

    trap_R_SetColor( NULL );
    CG_ClearClipRegion( );
  }
}

/*
==================
CG_SortDistance
==================
*/
static int CG_SortDistance( const void *a, const void *b )
{
  centity_t    *aent, *bent;
  float        adist, bdist;

  aent = &cg_entities[ *(int *)a ];
  bent = &cg_entities[ *(int *)b ];
  adist = Distance( cg.refdef.vieworg, aent->lerpOrigin );
  bdist = Distance( cg.refdef.vieworg, bent->lerpOrigin );
  if( adist > bdist )
    return -1;
  else if( adist < bdist )
    return 1;
  else
    return 0;
}

/*
==================
CG_PlayerIsBuilder
==================
*/
static qboolean CG_PlayerIsBuilder( buildable_t buildable )
{
  switch( cg.predictedPlayerState.weapon )
  {
    case WP_ABUILD:
    case WP_ABUILD2:
    case WP_HBUILD:
      return BG_Buildable( buildable, NULL )->team ==
             BG_Weapon( cg.predictedPlayerState.weapon )->team;

    default:
      return qfalse;
  }
}

/*
==================
CG_BuildableRemovalPending
==================
*/
static qboolean CG_BuildableRemovalPending( int entityNum )
{
  int           i;
  playerState_t *ps = &cg.snap->ps;

  if( !( ps->stats[ STAT_BUILDABLE ] & SB_VALID_TOGGLEBIT ) )
    return qfalse;

  for( i = 0; i < MAX_MISC; i++ )
  {
    if( ps->misc[ i ] == entityNum )
      return qtrue;
  }

  return qfalse;
}

/*
==================
CG_DrawBuildableStatus
==================
*/
void CG_DrawBuildableStatus( void )
{
  int             i;
  centity_t       *cent;
  entityState_t   *es;
  int             buildableList[ MAX_ENTITIES_IN_SNAPSHOT ];
  int             buildables = 0;
  vec3_t          end;
  trace_t         tr;
  qboolean        cuboid;
  
  if((cg.predictedPlayerState.stats[STAT_BUILDABLE]&~SB_VALID_TOGGLEBIT)>BA_NONE)
   return; //hide buildstats if we're placing a buildable
  
  for( i = 0; i < cg.snap->numEntities; i++ )
  {
    cent  = &cg_entities[ cg.snap->entities[ i ].number ];
    es    = &cent->currentState;

    if( es->eType == ET_BUILDABLE && CG_PlayerIsBuilder( es->modelindex ) )
      buildableList[ buildables++ ] = cg.snap->entities[ i ].number;
  }

  VectorMA(cg.refdef.vieworg,STATUS_MAX_VIEW_DIST,cg.refdef.viewaxis[0],end);
  CG_Trace(&tr,cg.refdef.vieworg,NULL,NULL,end,cg.predictedPlayerState.clientNum,MASK_SHOT);
  
  qsort( buildableList, buildables, sizeof( int ), CG_SortDistance );
  for( i = 0; i < buildables; i++ )
  {
    cuboid = BG_Buildable(cg_entities[buildableList[i]].currentState.modelindex,NULL)->cuboid;
    if(cuboid && tr.entityNum!=buildableList[i] )
      continue;
    CG_BuildableStatusDisplay( &cg_entities[ buildableList[ i ] ], cuboid, tr.endpos );
  }
}

#define BUILDABLE_SOUND_PERIOD  500

/*
==================
CG_Buildable
==================
*/
void CG_Buildable( centity_t *cent )
{
  refEntity_t     ent;
  entityState_t   *es = &cent->currentState;
  vec3_t          angles;
  vec3_t          surfNormal, xNormal, mins, maxs;
  vec3_t          refNormal = { 0.0f, 0.0f, 1.0f };
  float           rotAngle;
  team_t          team = BG_Buildable( es->modelindex, NULL )->team;
  float           scale;
  int             health;

  //must be before EF_NODRAW check
  if( team == TEAM_ALIENS )
    CG_Creep( cent );

  // if set to invisible, skip
  if( es->eFlags & EF_NODRAW )
  {
    if( CG_IsParticleSystemValid( &cent->buildablePS ) )
      CG_DestroyParticleSystem( &cent->buildablePS );

    return;
  }
  
  // cuboids use a bit different rendering code !@#CUBOID
  if( BG_IsCuboid( es->modelindex ) )
  {
    qhandle_t texture=0,cracks=0;
    vec3_t    dims;
    const cuboidAttributes_t *cuboidAttr;
    const cuboidInfo_t *cuboidInfo;
    int       i, health, sound;
    float     healthPct;
   
    cuboidAttr = BG_CuboidAttributes( es->modelindex );
    cuboidInfo = &cg_cuboids[ es->modelindex - CUBOID_FIRST ];
    VectorCopy( es->angles, dims );
    healthPct = (float)( health = BG_CuboidUnpackHealth(es) ) / (float)BG_Buildable(es->modelindex,dims)->health;

    if( healthPct > 1.0f )
      healthPct = 1.0f;
    else if ( healthPct < 0.0f )
      healthPct = 0.0f;
    
    if( cuboidInfo->useCracks )
    {
      if( cuboidInfo->textureCount )
        texture = cuboidInfo->textures[ 0 ];
      if( healthPct < 0.75f )
        cracks = cgs.media.cuboidCracks[ (int)( CUBOID_CRACK_TEXTURES - 1 - floor( CUBOID_CRACK_TEXTURES * healthPct ) ) - 1 ];
    }
    else
      for( i = 0; i < cuboidInfo->textureCount; i++ )
        if( healthPct >= cuboidInfo->textureThresholds[ i ] )
        {
          texture = cuboidInfo->textures[ i ];
          break;
        }
        
    if( !( es->eFlags & EF_B_SPAWNED ) )
    {
      sfxHandle_t prebuildSound=cgs.media.humanBuildablePrebuild;
      if( team == TEAM_HUMANS )
      {
        texture = cgs.media.humanSpawningShader;
        prebuildSound = cgs.media.humanBuildablePrebuild;
      }
      else if(team==TEAM_ALIENS)
      {
        texture = cgs.media.cuboidAlienPrebuild;
        prebuildSound = cgs.media.alienBuildablePrebuild;
      }
      cracks = 0;
      trap_S_AddLoopingSound( es->number, cent->lerpOrigin, vec3_origin, prebuildSound );
    }
    
    memset( &ent, 0, sizeof( ent ) );
    ent.reType = RT_MODEL;
    VectorCopy( cent->lerpOrigin, ent.origin );
    VectorCopy( cent->lerpOrigin, ent.oldorigin );
    VectorCopy( cent->lerpOrigin, ent.lightingOrigin );
    
    //NOTE: don't use CG_PositionAndOrientateBuildable, it screws up everything
    ent.axis[0][0]=-dims[0]/2.0f;ent.axis[0][1]=0.0f;         ent.axis[0][2]=0.0f;
    ent.axis[1][0]=0.0f;         ent.axis[1][1]=-dims[1]/2.0f;ent.axis[1][2]=0.0f;
    ent.axis[2][0]=0.0f;         ent.axis[2][1]=0.0f;         ent.axis[2][2]=dims[2]/2.0f;
    ent.nonNormalizedAxes = qtrue;
  
    ent.customShader = texture;
    ent.hModel = cgs.media.cuboidModel;
    trap_R_AddRefEntityToScene( &ent );
    if( cracks )
      CG_DrawCuboid( ent.origin, dims, cracks, 1 );
   
    if( health < cent->lastBuildableHealth && ( es->eFlags & EF_B_SPAWNED ) )
    {
      if( cent->lastBuildableDamageSoundTime + BUILDABLE_SOUND_PERIOD < cg.time )
      {
        sound = (int)( cuboidInfo->painSoundCount * random() );
        trap_S_StartSound( NULL, es->number, CHAN_BODY, cuboidInfo->painSounds[sound] );
        cent->lastBuildableDamageSoundTime = cg.time;
      }
      cent->lastBuildableHealth = health;
    }
  }
  else
  {

  memset ( &ent, 0, sizeof( ent ) );

  VectorCopy( cent->lerpOrigin, ent.origin );
  VectorCopy( cent->lerpOrigin, ent.oldorigin );
  VectorCopy( cent->lerpOrigin, ent.lightingOrigin );

  VectorCopy( es->origin2, surfNormal );

  VectorCopy( es->angles, angles );
  BG_BuildableBoundingBox( es->modelindex, mins, maxs );

  if( es->pos.trType == TR_STATIONARY )
  {
    // Positioning a buildable involves potentially up to two traces, and
    // seeing as buildables rarely move, we cache the results and recalculate
    // only if the buildable moves or changes orientation
    if( VectorCompare( cent->buildableCache.cachedOrigin, cent->lerpOrigin ) &&
        VectorCompare( cent->buildableCache.cachedNormal, surfNormal ) )
    {
      VectorCopy( cent->buildableCache.axis[ 0 ], ent.axis[ 0 ] );
      VectorCopy( cent->buildableCache.axis[ 1 ], ent.axis[ 1 ] );
      VectorCopy( cent->buildableCache.axis[ 2 ], ent.axis[ 2 ] );
      VectorCopy( cent->buildableCache.origin, ent.origin );
    }
    else
    {
      CG_PositionAndOrientateBuildable( angles, ent.origin, surfNormal,
                                        es->number, mins, maxs, ent.axis,
                                        ent.origin, qfalse );
      VectorCopy( ent.axis[ 0 ], cent->buildableCache.axis[ 0 ] );
      VectorCopy( ent.axis[ 1 ], cent->buildableCache.axis[ 1 ] );
      VectorCopy( ent.axis[ 2 ], cent->buildableCache.axis[ 2 ] );
      VectorCopy( ent.origin, cent->buildableCache.origin );
      VectorCopy( cent->lerpOrigin, cent->buildableCache.cachedOrigin );
      VectorCopy( surfNormal, cent->buildableCache.cachedNormal );
    }
  }

  VectorMA( ent.origin, BG_BuildableConfig( es->modelindex )->zOffset, surfNormal, ent.origin );
  

  VectorCopy( ent.origin, ent.oldorigin ); // don't positionally lerp at all
  VectorCopy( ent.origin, ent.lightingOrigin );

  ent.hModel = cg_buildables[ es->modelindex ].models[ 0 ];

  if( !( es->eFlags & EF_B_SPAWNED ) )
  {
    sfxHandle_t prebuildSound = cgs.media.humanBuildablePrebuild;

    if( team == TEAM_HUMANS )
    {
      ent.customShader = cgs.media.humanSpawningShader;
      prebuildSound = cgs.media.humanBuildablePrebuild;
    }
    else if( team == TEAM_ALIENS )
      prebuildSound = cgs.media.alienBuildablePrebuild;

    trap_S_AddLoopingSound( es->number, cent->lerpOrigin, vec3_origin, prebuildSound );
  }

  CG_BuildableAnimation( cent, &ent.oldframe, &ent.frame, &ent.backlerp );

  //rescale the model
  scale = BG_BuildableConfig( es->modelindex )->modelScale;

  if( scale != 1.0f )
  {
    VectorScale( ent.axis[ 0 ], scale, ent.axis[ 0 ] );
    VectorScale( ent.axis[ 1 ], scale, ent.axis[ 1 ] );
    VectorScale( ent.axis[ 2 ], scale, ent.axis[ 2 ] );

    ent.nonNormalizedAxes = qtrue;
  }
  else
   ent.nonNormalizedAxes = qfalse;
    

  if( CG_PlayerIsBuilder( es->modelindex ) && CG_BuildableRemovalPending( es->number ) )
    ent.customShader = cgs.media.redBuildShader;

  //add to refresh list
  trap_R_AddRefEntityToScene( &ent );

  CrossProduct( surfNormal, refNormal, xNormal );
  VectorNormalize( xNormal );
  rotAngle = RAD2DEG( acos( DotProduct( surfNormal, refNormal ) ) );

  //turret barrel bit
  if( cg_buildables[ es->modelindex ].models[ 1 ] )
  {
    refEntity_t turretBarrel;
    vec3_t      flatAxis[ 3 ];

    memset( &turretBarrel, 0, sizeof( turretBarrel ) );

    turretBarrel.hModel = cg_buildables[ es->modelindex ].models[ 1 ];

    CG_PositionEntityOnTag( &turretBarrel, &ent, ent.hModel, "tag_turret" );
    VectorCopy( cent->lerpOrigin, turretBarrel.lightingOrigin );
    AnglesToAxis( es->angles2, flatAxis );

    RotatePointAroundVector( turretBarrel.axis[ 0 ], xNormal, flatAxis[ 0 ], -rotAngle );
    RotatePointAroundVector( turretBarrel.axis[ 1 ], xNormal, flatAxis[ 1 ], -rotAngle );
    RotatePointAroundVector( turretBarrel.axis[ 2 ], xNormal, flatAxis[ 2 ], -rotAngle );

    turretBarrel.oldframe = ent.oldframe;
    turretBarrel.frame    = ent.frame;
    turretBarrel.backlerp = ent.backlerp;

    turretBarrel.customShader = ent.customShader;

    if( scale != 1.0f )
    {
      VectorScale( turretBarrel.axis[ 0 ], scale, turretBarrel.axis[ 0 ] );
      VectorScale( turretBarrel.axis[ 1 ], scale, turretBarrel.axis[ 1 ] );
      VectorScale( turretBarrel.axis[ 2 ], scale, turretBarrel.axis[ 2 ] );

      turretBarrel.nonNormalizedAxes = qtrue;
    }
    else
      turretBarrel.nonNormalizedAxes = qfalse;

    if( CG_PlayerIsBuilder( es->modelindex ) && CG_BuildableRemovalPending( es->number ) )
      turretBarrel.customShader = cgs.media.redBuildShader;

    trap_R_AddRefEntityToScene( &turretBarrel );
  }

  //turret barrel bit
  if( cg_buildables[ es->modelindex ].models[ 2 ] )
  {
    refEntity_t turretTop;
    vec3_t      flatAxis[ 3 ];
    vec3_t      swivelAngles;

    memset( &turretTop, 0, sizeof( turretTop ) );

    VectorCopy( es->angles2, swivelAngles );
    swivelAngles[ PITCH ] = 0.0f;

    turretTop.hModel = cg_buildables[ es->modelindex ].models[ 2 ];

    CG_PositionRotatedEntityOnTag( &turretTop, &ent, ent.hModel, "tag_turret" );
    VectorCopy( cent->lerpOrigin, turretTop.lightingOrigin );
    AnglesToAxis( swivelAngles, flatAxis );

    RotatePointAroundVector( turretTop.axis[ 0 ], xNormal, flatAxis[ 0 ], -rotAngle );
    RotatePointAroundVector( turretTop.axis[ 1 ], xNormal, flatAxis[ 1 ], -rotAngle );
    RotatePointAroundVector( turretTop.axis[ 2 ], xNormal, flatAxis[ 2 ], -rotAngle );

    turretTop.oldframe = ent.oldframe;
    turretTop.frame    = ent.frame;
    turretTop.backlerp = ent.backlerp;

    turretTop.customShader = ent.customShader;

    if( scale != 1.0f )
    {
      VectorScale( turretTop.axis[ 0 ], scale, turretTop.axis[ 0 ] );
      VectorScale( turretTop.axis[ 1 ], scale, turretTop.axis[ 1 ] );
      VectorScale( turretTop.axis[ 2 ], scale, turretTop.axis[ 2 ] );

      turretTop.nonNormalizedAxes = qtrue;
    }
    else
      turretTop.nonNormalizedAxes = qfalse;

    if( CG_PlayerIsBuilder( es->modelindex ) && CG_BuildableRemovalPending( es->number ) )
      turretTop.customShader = cgs.media.redBuildShader;

    trap_R_AddRefEntityToScene( &turretTop );
  }

  //weapon effects for turrets
  if( es->eFlags & EF_FIRING )
  {
    weaponInfo_t  *weapon = &cg_weapons[ es->weapon ];

    if( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME ||
        BG_Buildable( es->modelindex, NULL )->turretProjType == WP_TESLAGEN )
    {
      if( weapon->wim[ WPM_PRIMARY ].flashDlightColor[ 0 ] ||
          weapon->wim[ WPM_PRIMARY ].flashDlightColor[ 1 ] ||
          weapon->wim[ WPM_PRIMARY ].flashDlightColor[ 2 ] )
      {
        trap_R_AddLightToScene( cent->lerpOrigin, 300 + ( rand( ) & 31 ),
            weapon->wim[ WPM_PRIMARY ].flashDlightColor[ 0 ],
            weapon->wim[ WPM_PRIMARY ].flashDlightColor[ 1 ],
            weapon->wim[ WPM_PRIMARY ].flashDlightColor[ 2 ] );
      }
    }

    if( weapon->wim[ WPM_PRIMARY ].firingSound )
    {
      trap_S_AddLoopingSound( es->number, cent->lerpOrigin, vec3_origin,
          weapon->wim[ WPM_PRIMARY ].firingSound );
    }
    else if( weapon->readySound )
      trap_S_AddLoopingSound( es->number, cent->lerpOrigin, vec3_origin, weapon->readySound );
  }
  
  //smoke etc for damaged buildables
  CG_BuildableParticleEffects( cent );
  
  

  
  health = es->generic1;

  if( health < cent->lastBuildableHealth &&
      ( es->eFlags & EF_B_SPAWNED ) )
  {
    if( cent->lastBuildableDamageSoundTime + BUILDABLE_SOUND_PERIOD < cg.time )
    {
      if( team == TEAM_HUMANS )
      {
        int i = rand( ) % 4;
        trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.humanBuildableDamage[ i ] );
      }
      else if( team == TEAM_ALIENS )
        trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.alienBuildableDamage );

      cent->lastBuildableDamageSoundTime = cg.time;
    }
  }

  cent->lastBuildableHealth = health;

  } //if (is a cuboid)
}

char cuboidInfo[128];

/*
======================
CG_Cuboid_DrawInfo

Draw the cuboid info string generated by CG_Cuboid_Info.
======================
*/
void CG_Cuboid_DrawInfo(void)
{
 float x,y,w,h,s=0.5f;
 
 if(!BG_Buildable(cg.predictedPlayerState.stats[STAT_BUILDABLE]&~SB_VALID_TOGGLEBIT,NULL)->cuboid)
  return;
 
 w=UI_Text_Width(cuboidInfo,s);
 h=UI_Text_Height(cuboidInfo,s);
 x=320.0f-w/2.0f+cg_cuboidInfoX.value;
 y=240.0f-h/2.0f+cg_cuboidInfoY.value;
 
 UI_Text_Paint(x,y,s,colorWhite,cuboidInfo,0,0,ITEM_TEXTSTYLE_SHADOWEDMORE);
}

/*
======================
CG_Cuboid_Info

Update the cuboid info string with current cuboid's dimensions,
name, health, build time and build points (pretty much everything
one needs to know about his cuboid).
======================
*/
void CG_Cuboid_Info(void)
{
 const buildableAttributes_t *attr;
 int axis=cg_cuboidResizeAxis.integer;
 
 attr=BG_Buildable(cg.predictedPlayerState.stats[STAT_BUILDABLE]&~SB_VALID_TOGGLEBIT,cg.cuboidSelection);
 Com_sprintf(cuboidInfo,sizeof(cuboidInfo),
             "^7[^3%c^7] | ^%c%.1f^7x^%c%.1f^7x^%c%.1f ^7| ^3%i^7HP | ^3%i^7ms | ^3%i^7BP",
             (axis==0?'X':(axis==1?'Y':'Z')),
             (axis==0?'3':'5'),
             cg.cuboidSelection[0],
             (axis==1?'3':'5'),
             cg.cuboidSelection[1],
             (axis==2?'3':'5'),
             cg.cuboidSelection[2],
             attr->health,
             attr->buildTime,
             attr->buildPoints);
}

/*
======================
CG_Cuboid_Send

Send the cuboid selection via commands.
======================
*/
void CG_Cuboid_Send(void)
{
 static qboolean init=qfalse;
 static int lastupdate;
 static vec3_t lastcuboid;
 
 if(!BG_Buildable(cg.predictedPlayerState.stats[STAT_BUILDABLE]&~SB_VALID_TOGGLEBIT,NULL)->cuboid)
  return;
 
 if(!init)
 {
  lastupdate=cg.time;
  VectorCopy(cg.cuboidSelection,lastcuboid);
  init=qtrue;
 }

 if(lastupdate+100>cg.time)
  return;
 
 if(!VectorCompareEpsilon(lastcuboid,cg.cuboidSelection,1e-3))
 {
  cg.latestCBNumber++;
  cg.latestCBNumber%=100;
  trap_SendClientCommand(va("cb %i %f %f %f\n",cg.latestCBNumber,cg.cuboidSelection[0],cg.cuboidSelection[1],cg.cuboidSelection[2]));
  cg.forbidCuboids=qtrue; //wait for response
 }
 lastupdate=cg.time;
 VectorCopy(cg.cuboidSelection,lastcuboid);
}

/*
======================
CG_Cuboid_Response

Server responded to our cb with either cb2 or cb3.
======================
*/
void CG_Cuboid_Response(void)
{
 // cb2 <a> <b> <c>         : server sets client-side cuboid
 // cb3 <echo>              : server agrees on player's cuboid
 // cb3 <echo> <a> <b> <c>  : server doesnt agree on player's cuboid and corrects it
 static qboolean init=qfalse;
 if(!BG_Buildable(cg.predictedPlayerState.stats[STAT_BUILDABLE]&~SB_VALID_TOGGLEBIT,NULL)->cuboid)
  return;
 if(!Q_stricmp(CG_Argv(0),"cb2")&&trap_Argc()==4)
 {
  cg.cuboidSelection[0]=atof(CG_Argv(1));
  cg.cuboidSelection[1]=atof(CG_Argv(2));
  cg.cuboidSelection[2]=atof(CG_Argv(3));
  return;
 }
 else if(!Q_stricmp(CG_Argv(0),"cb3"))
 {
  if(trap_Argc()==2)
  {
   if(atoi(CG_Argv(1))==cg.latestCBNumber)
    cg.forbidCuboids=qfalse;
   return;
  }
  else if(trap_Argc()==5)
  {
   cg.cuboidSelection[0]=atof(CG_Argv(2));
   cg.cuboidSelection[1]=atof(CG_Argv(3));
   cg.cuboidSelection[2]=atof(CG_Argv(4));
   if(atoi(CG_Argv(1))==cg.latestCBNumber)
   { 
    cg.forbidCuboids=qfalse;
    if(cg.lastCuboidError+250<cg.time)
    {
     trap_S_StartLocalSound(cgs.media.cuboidErrorSound,CHAN_LOCAL_SOUND);
     cg.lastCuboidError=cg.time;
    }     
   }
   return;
  }
 }
 Com_Printf("^3warning: wrong cb2/cb3 from server\n");
}

/*
======================
CG_CuboidRotate_f

Rotate the cuboid selection 90 degrees around the current axis.
Syntax:
 cuboidRotate
======================
*/
void CG_CuboidResize(qboolean enlarge)
{
 vec3_t dims;
 int rate=(enlarge?1:-1)*5; //FIXME: cvar for rate
 if(!BG_Buildable(cg.predictedPlayerState.stats[STAT_BUILDABLE]&~SB_VALID_TOGGLEBIT,NULL)->cuboid)
  return;
 VectorCopy(cg.cuboidSelection,dims);
 switch(cg_cuboidResizeAxis.integer)
 {
  case 0:
   dims[0]+=rate;
   break;
  case 1:
   dims[1]+=rate;
   break;
  default:
   dims[2]+=rate;
   break;
 }
 if(dims[0]<1||dims[1]<1||dims[2]<1|| 
    dims[0]*dims[1]*dims[2]<CUBOID_MINVOLUME )
  return;
 if(enlarge)
  trap_S_StartLocalSound(cgs.media.cuboidResizeSoundA,CHAN_LOCAL_SOUND);
 else
  trap_S_StartLocalSound(cgs.media.cuboidResizeSoundB,CHAN_LOCAL_SOUND);
 VectorCopy(dims,cg.cuboidSelection);
}

/*
======================
CG_CuboidRotate_f

Rotate the cuboid selection 90 degrees around the current axis.
Syntax:
 cuboidRotate
======================
*/
#define SWAPFLOATS(a,b) {float __t;__t=a,a=b,b=__t;}
void CG_CuboidRotate_f(void)
{
 int axis;

 if(!BG_Buildable(cg.predictedPlayerState.stats[STAT_BUILDABLE]&~SB_VALID_TOGGLEBIT,NULL)->cuboid)
  return;
 axis=cg_cuboidResizeAxis.integer;
 switch(axis)
 {
  case 0:
   SWAPFLOATS(cg.cuboidSelection[1],cg.cuboidSelection[2]);
   break;
  case 1:
   SWAPFLOATS(cg.cuboidSelection[2],cg.cuboidSelection[0]);
   break;
  case 2:
   SWAPFLOATS(cg.cuboidSelection[0],cg.cuboidSelection[1]);
   break;
 }
 trap_S_StartLocalSound(cgs.media.cuboidRotateSound,CHAN_LOCAL_SOUND);
 CG_Cuboid_Send();
}

/*
======================
CG_CuboidAxis_f

Control the axis along which cuboids are resized and around which they are rotated.
Syntax:
 cuboidAxis next - next axis (order is X->Y->Z)
 cuboidAxis 0|1|2 - switch to a specific axis (respectively: X, Y and Z)
======================
*/
void CG_CuboidAxis_f(void)
{
 int axis;
 
 axis=cg_cuboidResizeAxis.integer;
 if(!BG_Buildable(cg.predictedPlayerState.stats[STAT_BUILDABLE]&~SB_VALID_TOGGLEBIT,NULL)->cuboid)
  return;
 if(!CG_Argv(1))
 {
  Com_Printf("cuboidAxis next|0|1|2 : set axis on which you want to resize your cuboid selection (0 - X, 1 - Y, 2 - Z)\n");
  return;
 }
 if(!Q_stricmp(CG_Argv(1),"next"))
  axis++;
 else
  axis=atoi(CG_Argv(1));
 trap_Cvar_Set("cg_cuboidResizeAxis",va("%i",(axis+3)%3));
 trap_S_StartLocalSound(cgs.media.cuboidAxisChangeSound,CHAN_LOCAL_SOUND);
}

/*
======================
CG_CuboidAttack_f

Replaces +attack/-attack.
If building a cuboid and the selection is somehow invalid, play an error sound.
Otherwise send the normal +attack / -attack;
======================
*/
void CG_CuboidAttack_f(void)
{
 if(BG_Buildable(cg.predictedPlayerState.stats[STAT_BUILDABLE]&~SB_VALID_TOGGLEBIT,NULL)->cuboid&&cg.forbidCuboids)
 {
  trap_S_StartLocalSound(cgs.media.cuboidErrorSound,CHAN_LOCAL_SOUND);
  return;
 }
 trap_SendClientCommand(va("%s",CG_Argv(0)));
}

