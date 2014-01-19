gfx/marks/shadow // blob shadow
{
  polygonOffset
  {
    map gfx/marks/shadow.tga
    blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    rgbGen vertex
    alphaGen vertex
  }
}

gfx/marks/wake // foamy water around swimming players
{
  polygonOffset
  {
    clampmap gfx/marks/splash.tga
    blendFunc GL_ONE GL_ONE
    rgbGen vertex
    tcmod rotate 250
    tcMod stretch sin .9 0.1 0 0.7
    rgbGen wave sin .7 .3 .25 .5
  }
  {
    clampmap gfx/marks/splash.tga
    blendFunc GL_ONE GL_ONE
    rgbGen vertex
    tcmod rotate -230
    tcMod stretch sin .9 0.05 0 0.9
    rgbGen wave sin .7 .3 .25 .4
  }
}

gfx/marks/bullet_mrk // bullet decal
{
  polygonOffset
  {
    map gfx/marks/bullet_mrk.tga
    blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
    rgbGen exactVertex
  }
}

gfx/marks/burn_mrk // burn mark
{
  polygonOffset
  {
    map gfx/marks/burn_mrk.tga
    blendFunc GL_ZERO GL_ONE_MINUS_SRC_COLOR
    rgbGen exactVertex
  }
}

gfx/marks/plasma_mrk // plasma decal
{
  polygonOffset
  {
    map gfx/marks/plasma_mrk.tga
    blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
    rgbGen vertex
    alphaGen vertex
  }
}

creep // alien creep texture
{
  nopicmip
  polygonoffset
  {
    clampmap gfx/misc/creep.tga
    blendfunc blend
    rgbGen identityLighting
    alphaGen Vertex
  }
}

greensplat // alien acid/blood decal
{
	nopicmip
	polygonoffset
	{
		map	gfx/blood/greensplat.tga
  blendfunc blend
	}
}

greensplat2 // alien acid/blood decal
{
	nopicmip
	polygonoffset
	{
		map	gfx/blood/greenblood01frame08.tga
  blendfunc blend
	}
}

greensplat3 // alien acid/blood decal
{
	nopicmip
	polygonoffset
	{
		map	gfx/blood/greenblood01frame07.tga
  blendfunc blend
	}
}

greensplat4 // alien acid/blood decal
{
	nopicmip
	polygonoffset
	{
		map	gfx/blood/greenblood03frame07.tga
  blendfunc blend
	}
}