// Human Player Model

models/players/human_base/h_base
{
	cull disable
	{
		map models/players/human_base/h_base.tga
		rgbGen lightingDiffuse
		alphaFunc GE128
	}
}

models/players/human_base/h_helmet
{
	cull disable
	{
		map models/players/human_base/h_helmet.tga
		rgbGen lightingDiffuse
		alphaFunc GE128
	}
}





// Jetpack

models/players/human_base/jetpack_flash
{
	sort additive
	{
		map	models/players/human_base/jetpack_flash.jpg
		blendfunc GL_ONE GL_ONE
		tcMod scroll 10.0 0.0
	}
}





// Defense Computer

models/buildables/dcc/comp_pipes
{
	{
		map models/buildables/dcc/comp_pipes.tga
		rgbGen lightingDiffuse
		tcGen environment 
	}
}

models/buildables/dcc/comp_display
{
	{
		map models/buildables/dcc/comp_grad.tga
		rgbGen identity
		tcMod scroll 0 1
	}
	{
		map models/buildables/dcc/comp_display.tga
		blendfunc gl_one gl_src_alpha
		rgbGen identity
	}
}





// Medipad

models/buildables/medistat/red_cross
{
  {
    map models/buildables/medistat/red_cross.tga
    blendfunc gl_one_minus_dst_color gl_one
  }
}

models/buildables/medistat/medi_cone
{
  cull none

  {
    map models/buildables/medistat/medi_cone.jpg
    blendfunc add
    rgbGen lightingDiffuse
    tcMod scroll 1 0
  }
  {
    map models/buildables/telenode/sparkles.tga
    blendfunc add
    rgbGen identity
    rgbGen wave noise 0 1 35 1 
    tcMod scale 2 2
    tcMod scroll 1 0.5
  }
}





// Reactor Core

models/buildables/reactor/reactor_main
{
	{
		map models/buildables/reactor/reactor_main.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/reactor/reactor_glow.tga
		blendfunc add
		rgbGen wave sin 0 1 0 0.5 
	}
}

models/buildables/reactor/reactor_meter
{
	{
		map models/buildables/reactor/reactor_meter.tga
		rgbGen lightingDiffuse
	}
}

models/buildables/reactor/reactor_display
{
	{
		map models/buildables/reactor/reactor_display.tga
		rgbGen identity
	}
}

models/buildables/reactor/reactor_bolt
{
	cull disable
	{
		map models/buildables/reactor/reactor_bolt.tga
		blendfunc add
		rgbGen identity
		tcMod scroll 2 0
	}
}

models/buildables/repeater/energy
{
	{
		map models/buildables/repeater/energy.tga
		rgbGen wave sawtooth 0.3 1 0 0.5 
		tcMod scale 2 1
		tcMod scroll 0 1
	}
}

models/buildables/repeater/repeator_panel
{
	{
		map models/buildables/repeater/repeator_panel.tga
		rgbGen identity
	}
}

models/buildables/arm/arm_panel2
{
	{
		map models/buildables/arm/arm_panel2.tga
		rgbGen identity
	}
}

models/buildables/arm/arm_panel3
{
	{
		map models/buildables/arm/arm_panel3.tga
		rgbGen identity
	}
}





// Stasis

models/mapobjects/stasis/chamber
{
	{
		map models/mapobjects/stasis/chamber.tga
		rgbGen vertex
	}
}

models/mapobjects/stasis/lifemeter
{
	{
		map models/mapobjects/stasis/lifemeter.tga
		rgbGen vertex
	}
}

models/mapobjects/stasis/lifemeter2
{
	{
		map models/mapobjects/stasis/meters.tga
		rgbGen identityLighting
		tcMod scroll 0.5 0
	}
	{
		map models/mapobjects/stasis/lifemeter2.tga
		alphaFunc GE128
	}
}

models/mapobjects/stasis/window
{
	{
		map models/mapobjects/stasis/window.tga
		blendfunc filter
	}
	{
		map models/mapobjects/stasis/bubbles.tga
		blendfunc add
	
		tcMod scroll 0 0.2
		tcMod scale 2 1
	}
}

models/mapobjects/stasis/flowpipe
{
	{
		map models/mapobjects/stasis/flowpipe.tga
		
		tcMod scale 5 5
		tcMod scroll 1 0
	}
	{
		map models/buildables/overmind/ref2.tga
		blendfunc filter
		
		tcGen environment 
	}
}

models/mapobjects/stasis/bubbles
{
	{
		map models/mapobjects/stasis/bubbles.tga
		rgbGen wave noise 0 1 0 1 
		tcMod scroll 0 0.5
	}
}





// Telenode

models/buildables/telenode/telenode_top
{
	{
		map models/buildables/telenode/telenode_top.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/overmind/ref2.tga
		blendfunc filter
		rgbGen identity
		tcGen environment 
	}
}

models/buildables/telenode/energy
{
	{
		map models/buildables/telenode/energy.tga
		rgbGen wave inversesawtooth 0.2 0.4 0 1 
		tcMod rotate 10
	}
}

models/buildables/telenode/rep_cyl
{
	cull disable
	{
		map models/buildables/telenode/rep_cyl.jpg
		blendfunc add
		rgbGen lightingDiffuse
		tcMod scroll 0.2 0
	}
	{
		map models/buildables/telenode/lines2.tga
		blendfunc add
		rgbGen identity
		tcMod scroll 0 0.2
	}
}

models/buildables/telenode/telenode_parts
{
	{
		map models/buildables/telenode/telenode_parts.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/overmind/ref2.tga
		blendfunc filter
		rgbGen identity
		tcGen environment 
	}
}





// Tesla Generator

models/buildables/tesla/tesla_main
{
	{
		map models/buildables/tesla/tesla_main.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/overmind/ref2.tga
		blendfunc filter
		rgbGen identity
		tcGen environment 
	}
}

models/buildables/tesla/tesla_ball
{
	{
		map models/buildables/tesla/tesla_ball.tga
		rgbGen lightingDiffuse
		tcGen environment 
	}
}

models/buildables/tesla/tesla_grill
{
	{
		map models/buildables/tesla/tesla_grill.tga
		rgbGen wave sin 0 1 0 0.4 
	}
}

models/buildables/tesla/tesla_spark
{
	cull disable
	{
		map models/buildables/tesla/tesla_spark.tga
		blendfunc add
		rgbGen identity
	}
}

models/ammo/tesla/tesla_bolt
{
	cull disable
	{
		map models/ammo/tesla/tesla_bolt.tga
		blendfunc add
		rgbGen vertex
		tcMod scroll 0.2 0
	}
	{
		map models/ammo/tesla/tesla_bolt.tga
		blendfunc add
		rgbGen wave sin 0 1 0 5 
		tcMod scroll 0.5 0
		tcMod scale -1 1
	}
}





// Turret

models/buildables/mgturret/t_flash
{
	cull disable
	{
		map models/buildables/mgturret/t_flash.tga
		blendfunc add
		rgbGen wave square 0 1 0 10 
	}
}

models/buildables/mgturret/turret_coil
{
	cull disable
	{
		map models/buildables/mgturret/turret_coil.tga
		rgbGen lightingDiffuse
		alphaFunc GE128
	}
}

models/buildables/mgturret/turret_shiny
{
	{
		map models/buildables/mgturret/turret_shiny.tga
		rgbGen lightingDiffuse
	}
	{
		map models/buildables/mgturret/ref_map.tga
		blendfunc filter
		rgbGen identity
		tcGen environment 
	}
}





// Construction Kit //

models/weapons/ckit/rep_cyl
{
	cull disable
	{
		map models/weapons/ckit/rep_cyl.jpg
		blendfunc add
		tcMod scroll 0.2 0
	}
	{
		map models/weapons/ckit/lines2.jpg
		blendfunc add
		tcMod scroll 0 -0.2
	}
}

models/weapons/ckit/screen
{
	{
		map models/weapons/ckit/screen.jpg
	}

	{
		map models/weapons/ckit/scroll.jpg
		blendfunc add
		tcMod scroll 10 -0.4
	}
}

models/weapons/ackit/rep_cyl
{
	cull disable
	{
		map models/weapons/ackit/rep_cyl.jpg
		blendfunc add
		tcMod scroll 0.2 0
	}
	{
		map models/weapons/ackit/lines2.jpg
		blendfunc add
		tcMod scroll 0 -0.2
	}
}

models/weapons/ackit/advscreen
{
	{
		map models/weapons/ackit/advscreen.jpg
	}

	{
		map models/weapons/ackit/scroll.jpg
		blendfunc add
		tcMod scroll 10 -0.4
	}
}





// Advanced Construction Kit

models/weapons/ackit/rep_cyl
{
	cull disable
	{
		map models/weapons/ackit/rep_cyl.jpg
		blendfunc add
		rgbGen lightingDiffuse
		tcMod scroll 0.2 0
	}
	{
		map models/weapons/ackit/lines2.jpg
		blendfunc add
		rgbGen identity
		tcMod scroll 0 -0.2
	}
}

models/weapons/ackit/particle
{
	cull disable
	{
		map models/weapons/ackit/particle.jpg
		blendfunc add
		rgbGen identity
		tcMod scroll 0.02 -0.4
	}
}

models/weapons/ackit/screen
{
	{
		map models/weapons/ackit/screen.jpg
	}

	{
		map models/weapons/ackit/scroll.jpg
		blendfunc add
		rgbGen lightingDiffuse
		tcMod scroll 10.0 -0.2
	}
}

models/weapons/ackit/screen2
{
	{
		map models/weapons/ackit/screen2.jpg
	}

	{
		map models/weapons/ackit/scroll2.jpg
		blendfunc add
		rgbGen lightingDiffuse
		tcMod scroll 0.2 -10.0
	}
}





// Flamer

models/weapons/flamer/pilot
{
	sort additive
	cull disable
	{
		map models/weapons/flamer/pilot.jpg
		blendfunc GL_ONE GL_ONE
		tcMod scroll 9.0 0
	}
}

models/weapons/flamer/gas
{
	sort additive
	cull disable
	{
		map models/weapons/flamer/gas.jpg
		blendfunc GL_ONE GL_ONE
		tcMod scroll 0.04 0.02
	}
}

models/weapons/flamer/flamer
{
	sort additive
	surfaceparm trans
	{
		map models/weapons/flamer/flamer.tga
		depthWrite
		alphaFunc GE128
		rgbGen lightingDiffuse
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}





// Grenade

models/weapons/grenade/grenade_s
{
	{
		map models/weapons/grenade/energy.jpg
		rgbGen wave sawtooth 0.3 1 0 0.5 
		tcMod scale 2 1
		tcMod scroll 0 1
	}
}

gfx/grenade/flare_01
{
	{
		map gfx/grenade/flare_01.tga
		blendfunc add
	}
}





// Mass Driver

models/weapons/mdriver/glow
{
	cull disable
	{
		map models/weapons/mdriver/glow.jpg
		blendfunc GL_ONE GL_ONE
		tcMod scroll -9.0 9.0
	}
}





// Pain Saw

models/weapons/psaw/chain
{
	sort additive
	cull disable
	{
		map models/weapons/psaw/chain.jpg
		blendfunc GL_ONE GL_ONE
		tcMod scroll 1.0 -4.0
	}
}

models/weapons/psaw/battery
{
	sort additive
	cull disable
	{
		map models/weapons/psaw/chain.jpg
		blendfunc GL_ONE GL_ONE
		tcMod scroll 0.04 -0.02
	}
}