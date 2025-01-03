### This repository is held under the HL1 SDK License provided by Valve Corporations.

#### features that are yet to be added:
- camera punch when taking fall damage
- fix crouching
- mp5 grenade
- monster ai (only scripted walking works)
- some func_ entities
- platform entities and trains (that includes mister john_train too)
- seamless level transition (only thing left to do is figure out how to pass spawnflags info to client and stuff)
- fix move_to_origin and drop_to_floor (its already "fixed", and by that i mean i did a very lazy hack to hook functions to the engine, i need to figure out why these functions make the engine freak out)

to compile the code you'll need to use visual studio 2022, project file is located at "hlalpha-reversed/dlls/hl/hl.sln"
maybe i'll port it to visual c++ 4 in the future
