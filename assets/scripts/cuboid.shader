/* This is a shader file for Cuboid Tremulous 1.2 mod.
 *
 * Textures:  cuboids/tex/<name>
 *            cuboids/tex/<name>_<frame>  : for cuboids with their own animation
 *
 * G. anim.:  cuboids/fx/ga_<frame>
 *
 * Particles: cuboids/fx/ps_<name>
 */

/*  CUBOID PARTICLE SHADER
 * These shaders are used for particle systems
 * that are displayed when a cuboid dies.
 */
models/cuboid/concrete/fragment
{
 {
  map models/cuboid/concrete/fragment.tga
  alphaFunc GE128
 }
}

models/cuboid/glass/fragment
{
 {
  map models/cuboid/glass/fragment.jpg
  blendFunc filter
 }
}

models/cuboid/ladder/fragment
{
 {
  map models/cuboid/ladder/fragment.tga
  alphaFunc GE128
 }
}

models/cuboid/organic/fragment
{
 {
  map models/cuboid/organic/fragment.tga
  alphaFunc GE128
 }
}

models/cuboid/slime/fragment
{
 {
  map models/cuboid/slime/fragment.tga
  alphaFunc GE128
 }
}

/*  CUBOID BUILDING SHADERS
 * These shaders appear on ghost model while
 * placing a cuboid. Red and green colours work
 * the same as regular building shaders. Yellow
 * colour is used after resizing to indicate, that
 * server hasn't responded to our cuboid resizing
 * command yet. 
 *
 * cuboids/build_axis shader shows rotation/scaling
 * axis on ghost model.
 */
gfx/cuboid/build_red
{
 nopicmip
 nomipmaps
 cull none
 {
  map gfx/cuboid/build_red.tga
  alphaFunc GE128
 }
 {
  map gfx/cuboid/build_red_fx.jpg
  blendFunc add
 }
}

gfx/cuboid/build_yellow
{
 nopicmip
 nomipmaps
 cull none
 {
  map gfx/cuboid/build_yellow.tga
  alphaFunc GE128
 }
 {
  map gfx/cuboid/build_yellow_fx.jpg
  blendFunc add
 }
}

gfx/cuboid/build_green
{
 nopicmip
 nomipmaps
 cull none
 {
  map gfx/cuboid/build_green.tga
  alphaFunc GE128
 }
 {
  map gfx/cuboid/build_green_fx.jpg
  blendFunc add
 }
}

gfx/cuboid/build_axis
{
 nopicmip
 nomipmaps
 cull none
 {
  map gfx/cuboid/build_axis.tga
  alphaFunc GE128
 }
}

/*  CRACKS
 * This is an animated shader, starting at 0s and ending at 1s.
 * Cgame uses a shaderTime hack to select the proper frame.
 */

models/cuboid/cracks
{
 sort 3
 {
  map models/cuboid/cracks.tga
  alphaFunc GT0
  alphaGen wave sin 0.5 0.5 0.0 0.2
 }
}
 
/*  CUBOID-SPECIFIC TEXTURES
 * These shaders are applied to cuboid's model.
 */

models/cuboid/glass/glass
{
 cull none
 sort 4
 {
  map models/cuboid/glass/glass
  blendFunc filter
  rgbGen identity
 }
}


models/cuboid/concrete/concrete
{
 sort 3
 {
  map models/cuboid/concrete/concrete.jpg
  rgbGen vertex
 }
}

models/cuboid/ladder/ladder
{
 sort 3
 {
  map models/cuboid/ladder/ladder.jpg
  rgbGen vertex
 }
}

models/cuboid/organic/organic
{
 sort 3
 {
  map models/cuboid/organic/organic.jpg
  rgbGen vertex
 }
}
models/cuboid/organic/organic_1
{
 sort 3
 {
  map models/cuboid/organic/organic.jpg
  rgbGen vertex
 }
 {
  map models/cuboid/organic/organic_fx.jpg
  blendFunc add
  rgbGen wave sin 0.2 0.1 0 0.1
 }
}

models/cuboid/organic/organic_2
{
 sort 3
 {
  map models/cuboid/organic/organic.jpg
  rgbGen vertex
 }
 {
  map models/cuboid/organic/organic_fx.jpg
  blendFunc add
  rgbGen wave sin 0.4 0.15 0 0.14
 }
}

models/cuboid/organic/organic_3
{
 sort 3
 {
  map models/cuboid/organic/organic.jpg
  rgbGen vertex
 }
 {
  map models/cuboid/organic/organic_fx.jpg
  blendFunc add
  rgbGen wave sin 0.8 0.2 0 0.19
 }
}

models/cuboid/slime/slime
{
 sort 3
 {
  map models/cuboid/slime/slime.jpg
  rgbGen vertex
  tcMod turb 0.75 0.25 0 0.07
 }
 {
  map models/cuboid/slime/slime.jpg
  blendFunc add
  rgbGen vertex
  tcMod turb 0.63 0.19 0.5 0.11
 }
}

/*  CAPACITOR BANK
 */
models/buildables/capbank/capbank
{
 {
  map models/buildables/capbank/capbank.jpg
  rgbGen lightingDiffuse
 }
 {
  map models/buildables/capbank/capbank_lights.jpg
  blendFunc add
  rgbGen wave sin 0.9 0.1 0 1.5
 }
}

/*  MISCELLANEOUS SHADERS
 * Various shaders used for various purposes.
 */
gfx/cuboid/prebuild_alien
{
 cull none
 {
  map gfx/cuboid/prebuild_alien.tga
  alphaFunc GE128
  tcMod turb 0.9 0.1 0 0.1
  tcMod scroll 0.05 -0.08
  tcMod rotate 7
 }
 {
  map gfx/cuboid/prebuild_alien.tga
  alphaFunc GE128
  tcMod turb 0.9 0.1 0.5 0.1
  tcMod scroll -0.03 0.07
  tcMod rotate -3
 }
}

gfx/2d/basivision
{
 {
  map gfx/2d/basivision_1.jpg
  blendFunc GL_ONE_MINUS_DST_COLOR GL_ONE_MINUS_SRC_COLOR
 }
}

gfx/2d/basivisionBlip
{
 {
  clampmap gfx/2d/basivision_blip.jpg
  blendFunc add
  tcMod rotate 26
 }
 {
  clampmap gfx/2d/basivision_blip.jpg
  blendFunc add
  tcMod rotate -49
 }
}

gfx/2d/basivisionFlare
{
 {
  clampmap gfx/2d/basivision_flare.jpg
  tcMod stretch sin 1 0.1 0 140
  blendFunc add
 }
}

gfx/misc/unpowered_prebuild
{
 cull disable
 {
  map models/buildables/telenode/rep_cyl.jpg
  blendfunc add
  rgbGen wave sin 0.1 0.05 0 123.45
  tcMod scroll 0.2 0
 }
}

lightning01
{
 nopicmip
 {
  map gfx/lightning/lightning01.jpg
  blendFunc add
 }
}

