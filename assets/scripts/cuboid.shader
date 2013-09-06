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

/*  GENERIC DESTRUCTION ANIMATION SHADERS
 * These shaders are applied on model if the
 * cuboid is damaged.
 */
models/cuboid/cracks_0
{
 {
  map models/cuboid/cracks_0.jpg
  blendFunc filter
 }
}

models/cuboid/cracks_1
{
 {
  map models/cuboid/cracks_1.jpg
  blendFunc filter
 }
}

models/cuboid/cracks_2
{
 {
  map models/cuboid/cracks_2.jpg
  blendFunc filter
 }
}

/*  CUBOID-SPECIFIC TEXTURES
 * These shaders are applied to cuboid's model.
 */

models/cuboid/glass/glass
{
 {
  map models/cuboid/glass/glass
  blendFunc filter
  rgbGen lightingDiffuse
 }
}

models/cuboid/concrete/concrete
{
 {
  map models/cuboid/concrete/concrete.jpg
  rgbGen lightingDiffuse
 }
}

models/cuboid/ladder/ladder
{
 {
  map models/cuboid/ladder/ladder.jpg
  rgbGen lightingDiffuse
 }
}

models/cuboid/organic/organic
{
 {
  map models/cuboid/organic/organic.jpg
  rgbGen lightingDiffuse
 }
}
models/cuboid/organic/organic_1
{
 {
  map models/cuboid/organic/organic.jpg
  rgbGen lightingDiffuse
 }
 {
  map models/cuboid/organic/organic_fx.jpg
  blendFunc add
  rgbGen wave sin 0.2 0.1 0 0.1
 }
}

models/cuboid/organic/organic_2
{
 {
  map models/cuboid/organic/organic.jpg
  rgbGen lightingDiffuse
 }
 {
  map models/cuboid/organic/organic_fx.jpg
  blendFunc add
  rgbGen wave sin 0.4 0.15 0 0.14
 }
}

models/cuboid/organic/organic_3
{
 {
  map models/cuboid/organic/organic.jpg
  rgbGen lightingDiffuse
 }
 {
  map models/cuboid/organic/organic_fx.jpg
  blendFunc add
  rgbGen wave sin 0.8 0.2 0 0.19
 }
}

models/cuboid/slime/slime
{
 {
  map models/cuboid/slime/slime.jpg
  rgbGen lightingDiffuse
  tcMod turb 0.75 0.25 0 0.07
  tcGen environment
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

