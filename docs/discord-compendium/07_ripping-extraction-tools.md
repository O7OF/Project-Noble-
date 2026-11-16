# ripping extraction tools

Selected from 528 candidate messages (ranked by length, links, Halo relevance; capped at ~260 KB). Chronological. Channels: Assembly, BMAX, Halo 5 Animation Extractor, Halo 5 Research, Index, PhxTools, Reclaimer, Tag Tool, 💬╎general-help, 💾╎autodesk, 💾╎substance-suite, 📜╎ripping, 🔍╎texturing-and-shading, 🦴╎animation, 🦴╎rigging

- **2021-03-16** `_gallium` [📜╎ripping] 4 is on my list of autodumps to do, after reach. May end up being of some use.
- **2021-03-17** `sierrad421` [📜╎ripping] I've started futzing around in Reclaimer, with MCC Reach, but I understand none of the instructions. They're really confusing.
- **2021-03-22** `plastered_crab` [📜╎ripping] I feel like that would be helpful too! I'll get started on that sometime soon. I'd love for it to start with Halo CEA model ripping tools, Halo CE - 4 model ripping tools, Halo 2 - ODST sound ripping tools, Reach - 4 sound ripping tools and have it go in order
- **2021-03-24** `plastered_crab` [📜╎ripping] I should have all reach sound files extracted in the main Google drive already :)
- **2021-03-24** `surasia` [📜╎ripping] there's no data currently reclaimer can reach, that'd be a better explanation
- **2021-03-30** `surasia` [📜╎ripping] https://github.com/Gravemind2401/Reclaimer/releases/tag/v1.5.244
  [embed] Release v1.5.244 · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v1.5.244 fix collada material URIs
  support batch extract for H5
  support WMA sounds on H2 xbox
  fix some H3 bsps not reading
  fix H1 bsps not batch extracting
- **2021-03-31** `ruv9999` [📜╎ripping] hi can i ask something? im about to trying extract Halo Reach all noble team with no helmet model(except emile) where can i find those models?
- **2021-04-04** `tessakitty7758` [📜╎ripping] Hii, does anyone know if the wombat has ever been ripped from Halo 3/ODST?
- **2021-04-09** `surasia` [📜╎ripping] https://github.com/Gravemind2401/Reclaimer/releases/tag/v1.5.245
  [embed] Release v1.5.245 · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v1.5.245 support latest version of MCC
  fix h4x cubemaps
  fix h4x node maps
  add support for h2x cubemap unwrapping
  add support for model LODs in h2x
- **2021-04-14** `rcnfl` [📜╎ripping] Anyone mind helping me rip my Reach model from MCC on PC? I tried a week ago and it was odd as I have never ripped models before
- **2021-04-16** `wavecommotion` [💬╎general-help] Ohhhh I saw that and I thought it was just an image added by error and now, I just realized I needed those shaders in the helpful resources section but which one do I need for the odst raven? I already tried with the h3 but I still can't extract the bump map it's like it just skips for some reason
- **2021-04-19** `_gallium` [📜╎ripping] If the original value was `66` and you got that by fully ripping, you will get the same result as if you reach `66` via other means.
- **2021-04-20** `vuthakral` [📜╎ripping] Hey, I just noticed my old Halo Online elite rips are on the Google Drive for this server, I'm just letting you guys know I fucked up the alphas included on those. Honestly, at best I'd hope to have it removed so people don't use them because of how fucked up they become if you use those alphas for specular masking (the intended thing for them)
- **2021-04-21** `krios24` [🔍╎texturing-and-shading] Found what I can safely guess is one of the only rips of the Halo 4 ODST armor via a gmod addon. The old XBox 360 textures are trash so I’m using it as a test dummy for reacquainting myself with Substance Painter. I remember liking the armor’s look when it came out but not the texture/color scheme they gave it. So I’m trying to re-envision it in the OG style
  [attachments] image0.png
- **2021-04-23** `athenalittleowl` [📜╎ripping] is there a chance that the fire team raven armor has already been ripped, i wanna bring my new H3 spartan into another application and need the files
- **2021-04-26** `surasia` [💬╎general-help] Multiplayer map files. Currently there is no way to rip campaign other than ninjaripper
- **2021-04-27** `err_0r` [🔍╎texturing-and-shading] I personally decompiled and recompiled the cc textures so that I didn't have that data in the alpha, swapping it over to another channel so I could better visualise what the masks were doing, but the shader I tossed together in amplify shader editor ended up looking like this.
  [attachments] unknown.png
- **2021-04-28** `fiycsf` [🔍╎texturing-and-shading] The Halo 3 style ODST shoulders are obtainable without ripping however, on the McCarthy Sketchfab page from the Spartan model. 
  
  Afaik the rest of the ODST parts minus helmet attachments and a wrist part (that are also McCarthy Bros) are AbiSv and not intended for people to rip or have.
- **2021-05-01** `aqua_himura` [💬╎general-help] tried to view the reach spartan in the most recent version of reclaimer
  [attachments] unknown.png
- **2021-05-01** `the_heavynator` [📜╎ripping] would it be possible to implement multi uv and vertex color extraction for bsps? halo 4 and h2amp make extensive use of vertex colors for overlay meshes and multi uvs for better detail map alignment. halo 3 and reach also makes use of uv2 for its water masks/main color.
- **2021-05-01** `gravemind2401` [📜╎ripping] there’s plenty of vertex types that do include colour and multiple uvs but they don’t get used for any of the geometry reclaimer exports
- **2021-05-04** `gravemind2401` [📜╎ripping] if the vertices visible in reclaimer have color information i can only assume its calculated or part of post processing
- **2021-05-04** `pointdevice` [📜╎ripping] the actual shader code would be what accesses it, would still be part of the mesh, getting nsight setup so i can investigate both, unfortunately, i dont think i can get the shaders as what i would need to decompile them, the ones in mccpc are dx11 compiled hlsl, and the ones on console vary, the only ones that can some what be decompiled, at least last i checked, are dx9 shaders, which only halo online would use, but i dont think that would be using any vert colors, so not much use, but i can try working some other magic on it
- **2021-05-04** `plastered_crab` [🔍╎texturing-and-shading] @deleted-role anyone have any idea on how to get a shader setup for an extracted map? @Deleted User is trying to get a map extracted shaded properly using Maya, but I have no knowledge about Maya 🤷🏻‍♂️
- **2021-05-06** `dashiv` [💬╎general-help] I am pretty inexperienced in blender so this is going to be interesting, but can someone tell me how to do this? in this video at around the 3:20 mark, he insets the ends of the reach thruster. I had a game rip file (cant remember if it was from the archive or not) and tried deleting every face so i could have the thruster by itself. then when i tried a few different methods, it always screwed with the model and deformed it or added protruding geometry. Is there an easy way to do it? also after the 3:05 mark, he rotates the thruster of the reach jetpack. this isn't crucial, i just thought the origin or rotation was neat and useful. https://www.youtube.com/watch?v=2b89MxacEl8
  [embed] Something about Sharkface https://www.youtube.com/watch?v=2b89MxacEl8 A pretty pointless video to wet the palette during a dry spell.
  
  My DeviantArt: https://www.deviantart.com/eproth
  
  #redvsblue
- **2021-05-09** `plastered_crab` [💬╎general-help] You might be able to get a rigged one from the Reach dump at #📂╎community-upload-download:)
- **2021-05-09** `gravemind2401` [📜╎ripping] someone i dont know the name of (only communicated indirectly) was able to dump the normal palette colour values from the executable, so reclaimer now has the data to lookup which index is which rgb value
- **2021-05-16** `plastered_crab` [💬╎general-help] That is a very ambitious project! I'm personally not experienced in assembly, but I know people do some magic with modding, though as for ui from Reach into Halo 3 would be a question for @stevieroth or some other crazy modders here about that question 🤔
- **2021-05-17** `djcheef` [💬╎general-help] where can I find my custom spartan or the multiplayer armor in reclaimer?
- **2021-05-17** `ubald007` [💬╎general-help] Hey guys, just a little info for you.  the Killer instinct textures work with the Halo 5 extracts lol, for the Arbiter and the elite minor. CHEERs
- **2021-05-17** `pigswines` [📜╎ripping] Alright thanks sm, I knew the maps and all that, I’ve ripped models before from 3 and reach, but not CE and it had no “render_model”
- **2021-05-17** `Deleted User` [🦴╎animation] hello all umm i need help ripping animation files from halo 3 odst.map files
  bonobo seems to extract it but when i go to the folder
  it no work
  i am ripping models and animations from odst so i can put it in halo ce i know i could just rip everything out and just port it over
  but i am trying to put the animations on halo ce models instead
  so i thought renaming the skeleton of ce marines to that of odst marines for the animations would work then run some scripts to import it but idk what do you guys think?
  kinda wish there was a tutorial on this
- **2021-05-19** `supervigy` [📜╎ripping] My reclaimer won’t install so u need someone to rip my halo 4 multiplayer model
- **2021-05-20** `zad_47` [💬╎general-help] https://www.moddb.com/games/halo-2/downloads/bonobo-version-1003
  [embed] Bonobo [Version 1.0.0.3] Halo2/3/ODST/Reach Animation Extractor file https://www.moddb.com/games/halo-2/downloads/bonobo-version-1003 Bonobo [Version 1.0.0.3] is a Halo2/3/ODST/Reach Animation Extractor used to extract the animations from those 4 halo games, this can be found on halomaps.org, this c
- **2021-05-23** `err_0r` [💬╎general-help] Has anyone had an issue with Reclaimer refusing to extract? I select a folder in the reach map file, press extract, and the folder I'm extracting to is empty. Same thing with the new map batch extract. I'm using the current .295 build.
- **2021-05-31** `chromacore` [🔍╎texturing-and-shading] Yeah it doesn't have every value from the reach assembly stuff, but I use it to bake out the textures into PBR colored specular to use in games like vrchat
- **2021-06-02** `r93_sniper` [🔍╎texturing-and-shading] you could probably decompile her shader with assembly or something of similar
- **2021-06-03** `muffintastic` [📜╎ripping] quite new to all this; where might i be able to find high quality texture(s) for the halo reach ghost, especially the overlay/detail texture? the ones i ripped using Reclaimer are only 1024x1024 (PC MCC)
  [attachments] MCC-Win64-Shipping_6lOVUiCReF.jpg
- **2021-06-06** `ubald007` [💬╎general-help] I prob should have asked in ripping I just wanted to try out reclaimer I wanted to check out the new elites I dont even know if they are in H3 or flight
- **2021-06-06** `pointdevice` [📜╎ripping] pulling in Halo3 Chief from 005_intro.map from MCC, using the latest release build of reclaimer from github, and the script posted above using the listed settings(should be the defaults)
  [attachments] unknown.png
- **2021-06-11** `thelifeweaver` [📜╎ripping] The aforementioned demo BTW ||(https://youtu.be/MMkLdWBn5Ew) ||
  [embed] Ninja Ripper 2 beta Quick Guide https://www.youtube.com/watch?v=MMkLdWBn5Ew Ninja Ripper 2 is an experimental utility for extracting geometry from 3D game levels and exploring them in a 3D editor such as Blender, 3D Max, etc. Use cases could be, for example, exploring hard-to-reach places in a game 
- **2021-06-14** `brooen` [💬╎general-help] https://youtu.be/UzZk5Wi2FQc
  [embed] [#1] Halo MCC - How to Shoot ROCKETS From Your Assault Rifle! (Weap... https://www.youtube.com/watch?v=UzZk5Wi2FQc How to Mod Halo #1: In this tutorial I'll show you the basics of modding weapons in Halo Reach on the Master Chief Collection using Assembly. This is How To Mod Halo!
  
  ▪▪▪▪▪▪▪▪▪▪▪▪▪▪▪▪▪
- **2021-06-16** `Deleted User` [📜╎ripping] Can somebody possibly rip me the Halo 3 hud binoculars or better yet Halo Reach?
- **2021-06-18** `vinwarrior` [📜╎ripping] Currently trying to pull the Spade truck from Halo Reach and its normals seem to be messed up. Attempted both the Winter Contingency version and the Forge World version. Both rip with incorrect normals. I have 'import normals' checked on, and other vehicles from Reach haven't had the same issues. Everything seems to import fine except for the Spade.
  [attachments] unknown.png
- **2021-06-26** `soulburnin` [📜╎ripping] The app will have the shader info, or at least simplified versions of it, similar to the Destiny 3D viewer stuff. I haven’t kept up with the Destiny model server in a while, but apparently they can extract the shader info just fine.
- **2021-06-29** `zippingrocks` [💬╎general-help] Anyone have the Elite and Ripa Moramee from these packs? Looking to do high quality remakes for Modding.
  [attachments] halo-wars-covenant-warriors.jpg, H3_WHC_S1_squad4.jpg, apiaqkhxq__37191.1588710069.jpg
- **2021-07-02** `dylm8` [💬╎general-help] Just wondering if I can rip my H3 Multiplayer spartan and use it in SFM using Reclaimer?
- **2021-07-03** `fiycsf` [📜╎ripping] Ripping from a game you can buy and pay for should be considered differently.
  
  You can buy the game, and artists and devs were paid for their work.
  
  When you take models from a mod like that, you are taking something you didn't pay for, that you were effectively granted a specific use for, for free.
  
  In most cases, if the author is cool with anyone playing with them, they just put them up somewhere if asked.
  
  Eg; https://sketchfab.com/3d-models/halo-reach-ma37-assault-rifle-c494ab2a4acb439ea626319aa7224b6c
  
  ***TLDR, I also wouldn't recommend.***
  [embed] Halo Reach - MA37 Assault Rifle - Download Free 3D model by Noe.Vah... https://sketchfab.com/3d-models/halo-reach-ma37-assault-rifle-c494ab2a4acb439ea626319aa7224b6c Faithful (I hope) recreation of the Halo Reach MA37 Assault Rifle
  Model (High and ‘Low’ poly) by: Soda (https://www.artstation.com/u65
- **2021-07-05** `pointdevice` [💬╎general-help] and heres the entire decompiled shader for the pixel shader for that draw call
  [attachments] HaloR_Fuel_Rod_Glass_shader_asm.txt
- **2021-07-05** `pointdevice` [💬╎general-help] if you need anything else, feel free to reach out, normally i wouldnt dump assembly code like this, in favor of a more finalized result, but my heads a bit out of it today, and i dont really have the mental capacity to walk through the whole thing rn
- **2021-07-06** `ruv9999` [💬╎general-help] for reach im using m05 and m20. for 3 im using 010_jungle. a few month ago my reclaimer has no issue. until im using it yesterday it has an issue that i don't know why.
- **2021-07-07** `garfunkle3479` [📜╎ripping] One more thing to ask about Reclaimer and its compatibility with the H5 modules, are we still currently unable to pull files for the multiplayer armors or is that just a memory issue I'm currently experiencing? The model not only fails to load, but it doesnt even register. Even selecting *extract all* just creates an empty file folder.
- **2021-07-08** `hunter_ap` [🦴╎animation] Wanted to say thank you for that tutorial on getting animations out of the different Halo games, was something I was hoping would finally be doable after so many years of ripping assets from different games
  One question though: I'm getting errors when trying to extract some first person weapon animations, namely these from Reach:
  ```
  jmad -> objects\characters\spartans\fp\weapons\pistol\fp_magnum\fp_magnum
  jmad -> objects\characters\spartans\fp\weapons\support_high\fp_spartan_laser\fp_spartan_laser
  ```
  For the magnum, I get the error `Value cannot be null` for `Parameter name: source`, and for the splaser I get `Index was out of range` for `Parameter name: index`
  This seems to happen on any MP, FF, or SP map file I load
- **2021-07-08** `mrchromed` [🦴╎animation] Reach Magnum can't be extracted unfortunately. Bonobo is still a WIP... well, maybe. LaikaGlove hasn't been seen since last year, and afaik he didn't leave the source code, so we can't even improve the program 😦
- **2021-07-09** `pointdevice` [🔍╎texturing-and-shading] looking like a good start, when i get some time, and im not on my the phone with my bank for 6 hours, i can take a look at the dumped shader assembly i posted and see if i can help you get some thing closer
- **2021-07-12** `oragani` [📜╎ripping] I'm using Reclaimer to export the forge map from Reach but all the materials are messed up. Any clue why?
- **2021-07-13** `oragani` [📜╎ripping] Also, I exported Forge world from Reach using Reclaimer but it didn't come with any foliage. Any clue how to get it?
- **2021-07-15** `chromacore` [🔍╎texturing-and-shading] I think most people use assembly for the halo games, but actually decompiling the shader code into ASCII or something there hasn't been much besides glances
- **2021-07-15** `pointdevice` [🔍╎texturing-and-shading] some times, i really dont like to.
  I look at the stuff around the shading a lighting in the render pipeline and i make some pretty ok guesses, which is how i did stuff like the halo 3 area specular term for lava, and finding where in vertex buffer h2a stores its vertex color, which is now a feature in reclaimer because of bugging gravemind.  There are some cases where i have fully dissassembled a shader but its a pain, and still doesnt answer some questions like how light map data is stored and used, and it is a very technical and manual process...
  
  Also nsight is the best debugger
- **2021-07-15** `pointdevice` [🔍╎texturing-and-shading] The output of assembly(the program) for its shaders is different and is basically useless as its raw shader binary, and unless you have memorized every hlsl instruction in binary, good luck
- **2021-07-16** `Average_Username` [💬╎general-help] so i've been trying to get bonobo working using the tutorial 
  but bonobo just refuses to open for me and i get this
  [embed] Thanks to @PlasmaGiant for taking the time to provide a brief rundown of ripping animations with Bonobo and porting them to Blender!
  
  "So what i found first was this program called Bonobo, an animation extractor program. It essentially reads the .map files for the Bungie Era games for you to either 
- **2021-07-18** `surasia` [🔍╎texturing-and-shading] Texture importing to reach is super easy, you can use forerunners tool to convert DDS into .tagc and just import it into assembly with your custom name
  
  Also that's pretty interesting
- **2021-07-19** `plastered_crab` [🔍╎texturing-and-shading] In Assembly it is labeled as a fur shader and the alpha mask looked like it had like 3 layers to it?
- **2021-07-20** `jadedqrow` [🔍╎texturing-and-shading] i completely decompiled and fucked with the shader to islolate the issue having to rebuild some of it, may have made more issues but yeah
- **2021-07-20** `09williamsad` [📜╎ripping] And ODST Engineer (Not rookie though) apparently, I must have not noticed when I batch ripped it a while ago, dae and amf with blender and max.
  [attachments] unknown.png
- **2021-07-20** `b1rb.` [📜╎ripping] does anyone have a guide for ripping the menu models with umodel? i want to rip the new odst stuff. but im not sure where its located. do i need to change these settings?
  [attachments] unknown.png
- **2021-07-21** `plastered_crab` [💬╎general-help] Tbh it would be much easier to learn if you extracted H3 chief with his animations and practiced with that instead of trying to attach animations to a ninja ripped model
- **2021-07-21** `artistify` [💬╎general-help] why is all model from ODST is triangular cubes?
  and how do i use reclaimer to rip them????? 
                      !!!!!!! HELP!!!!!!!
- **2021-07-21** `detestment` [💬╎general-help] Does anyone know how to rip your custom reach spartan model from the customization screen on mcc using reclaimer?
- **2021-07-23** `acearkadios` [💬╎general-help] For the "Halo Reach MCC' Dump, Where do I find the texture for the Spartan Shields? I've been diggin around, but can't find it.
- **2021-07-28** `oragani` [📜╎ripping] So once the Infinite multiplayer beta is released tomorrow does anyone know if it’ll work with Reclaimer?
- **2021-08-02** `plastered_crab` [💬╎general-help] Also you might achieve better texturing if you use the Bungie Halo Shader as well as use Assembly to look at all the other textures that surface uses and the tiling and scaling that needs to be applied to each thing
- **2021-08-17** `plastered_crab` [💬╎general-help] But if you want animations too I can show you how to do that along with ripping the rig for the elite so you can actually move his body around
- **2021-08-23** `rcnfl` [📜╎ripping] so it's been a while since i ripped anything, but I just ripped Halo3/4 Elites. Anyone know what the spartan file name is for multiplayer models?
- **2021-08-23** `Deleted User` [📜╎ripping] Would anyone be able to help me figure out how to rip a couple models from RainbowSix Siege? Or point me the way there? I want to rip the Caveira Elite and Kali Elite Skins. I've looked into a file extractor tool (I'm still trying to learn how to even open/use it), but if anyone knows already that would be amazing :frogePray:
- **2021-08-24** `sculpted_stache` [📜╎ripping] anyone know how to find the halo reach/halo 3 forge props in reclaimer
- **2021-08-28** `the_heavynator` [💬╎general-help] so im trying to import 2 separate textures into reach but for some reason importing through assembly will only import the first one, if i try to import another one it just duplicates the first one
- **2021-08-29** `piratepenguin.` [💬╎general-help] Does anyone have the Guta animations from Reach? I've tried ripping them via bonobo, but I get an error every time I try just for the Guta, everything else rips just fine
- **2021-09-29** `masterz1337` [🔍╎texturing-and-shading] Does anyone have a library of 3, Reach, CEA, and even H4 forerunner textures?
  I rather not have to dump each game
- **2021-10-15** `fiycsf` [🦴╎rigging] (and once again, I started with a fresh rip, the original reach skeleton. Not sure if yours is)
- **2021-10-17** `mehrunesdaegan` [📜╎ripping] When I extracted the new Season 8 armors using Reclaimer, the color control maps all seem to be blank. Like void of anything as if they're an empty png. 
  Tested extracting other armors and their bitmaps and it seemed to work fine.
  I do something wrong or is it random on what armors actually have CC?
  Any help would be appreciated I'm new to the program so apologies if I missed something crucial.
- **2021-10-17** `the_heavynator` [📜╎ripping] Also just a heads up don't bother extracting the reach armour models from the menu, there UVs are fucked
- **2021-10-18** `.chevyj` [📜╎ripping] so I'm trying to rip the new season 8 models for halo reach in, do they just re-use the same textures for both 3 and reach in the unreal viewer?
- **2021-10-20** `itzhouze` [💬╎general-help] Hey there. I need some help. I exported the halo 3 multiplayer map: "warehouse" structure_bsp file to an AMF file using Reclaimer. Then i modified the geometry in blender. Now i exported the modified file as an ASS file. Whats the correct steps to get an structure_bsp file? i tried using tool=>structure, but after 5 minutes it just throws many errors, missing shaders and triangle errors and wont build. Hope you can help me out 🙂
- **2021-10-22** `saltyace` [📜╎ripping] Has no one ever ripped the Reach level New Alexandria? I've been looking everywhere for Club Errara, but I can't find it.
- **2021-10-24** `sierra060822` [🦴╎animation] yeeee all ripped and reused stuff from halo reach and ported to blender! i may do a little shader fuckery for the armor before i finish the final render but im actually planning on rendering it tonight while i sleep, now that i've finally bought more storage for projects cause i had like none and had to stop making stuff  😅
- **2021-10-24** `claytonium531` [💬╎general-help] Someone have time to teach me how I can model a custom map for halo reach in blender? And also to rip models from reach?
- **2021-10-29** `_gallium` [💬╎general-help] Welcome to the archive 😀 
  
  Have a look down in the File Dump & Links and Resources sections.
  
  A quick search in #📂╎community-upload-download `in: community-upload-download marine` pulls up some of the Reach marines that have been ported & set up.
- **2021-10-29** `sswampp` [💬╎general-help] looks like it's not on the drive so I'll have to look at ripping it when I get to reuploading the reach assets
- **2021-11-05** `refraynn` [📜╎ripping] i'm trying to extract halo reach models with assembly but none of them are working, is there another way to do it to .obj / .dae / .fbx?
- **2021-11-05** `sswampp` [📜╎ripping] if it's an older build of reclaimer (months old) then the map headers changed and you need the new build of reclaimer to rip from reach
- **2021-11-05** `sswampp` [📜╎ripping] looks like the latest mcc update broke reclaimer's reach map support too
- **2021-11-05** `sswampp` [💬╎general-help] when ripping the sbsp you can extract most of the stuff you need (in H3 and later) with this button
- **2021-11-10** `spartanatic` [💬╎general-help] I ripped the Halo Reach Tribute Room but I’m not sure if it’s worth putting together. It’s missing a lot of things
- **2021-11-10** `sswampp` [📜╎ripping] because you'd need to check in assembly (or HEK's guerilla) what the shader parameters you want to match are
- **2021-11-12** `unordinal` [📜╎ripping] Is there any difference between Xbox Reach and MCC Reach in regards to model meshes and textures? Wondering if I should wait for a Reclaimer update or just download the Xbox map files and use those.
- **2021-11-12** `unordinal` [📜╎ripping] yeah, I used to have them downloaded but deleted them after a while of Reach being on PC due to the very few amount of times I needed them and Reclaimer existing
- **2021-11-20** `celtyshy03` [📜╎ripping] question whats the name of the mission in reach that has club errara because id like to rip it from it
- **2021-11-22** `refraynn` [📜╎ripping] I'm not gonna be doing much for now, but there will be a database of the different drives in a JSON file and then if you do, for example,
  
  ``r!reach <type>`` (Type being vehicle, weapons, and sound effects)
  
  It'll grab a link to the specific folder and send it in a message to you with some additional information.
  
  It'll be like that for all of the Halo games.
  
  Another part will be the documentation on ripping, such as 'How do I rip?', so you can do 
  
  ``r!docs ripping`` and it'll give you an embed with information on ripping.
- **2021-11-25** `pigswines` [💬╎general-help] If you know how to use reclaimer you can easily get the reach troopers
- **2021-11-25** `chromacore` [💬╎general-help] There's an export bitmaps button next to the export mesh buttons in reclaimer. Then you'd have to setup your textures in the shader editor.
- **2021-11-26** `bonepunk` [📜╎ripping] so i want to play around with the halo reach infinite models and i noticed that some armor elements are missing has some one found the missing stuff or did I do something wrong when I extracted the models?
  [attachments] blender_OCAyJ90DcQ.png
- **2021-11-26** `sswampp` [💬╎general-help] you can use assembly to check the shader information and replicate it in blender
- **2021-11-27** `sswampp` [💬╎general-help] generally you're going to have to replicate the materials using assembly for shader info anyway
- **2021-11-28** `Deleted User` [💬╎general-help] So, I used reclaimer and got my hands on the files for the Noble Actual mission of reach, but its all in seperate objects. Is there a file of the entire map already put together that I can extract? Or do I need to reassemble it myself?
- **2021-11-28** `yhormv` [💬╎general-help] When I port the Halo 3 odst H100 map file into Reclaimer, all the maps are yellow with no textures.
- **2021-11-28** `piratepenguin.` [💬╎general-help] https://c20.reclaimers.net/h1/tags/shader/ here's two hope this helps fam ❤️
  [embed] shader (abstract tag) https://c20.reclaimers.net/h1/tags/shader/ Shader tags define the visual appearance of models in the game world. The shader tag class itself is...
- **2021-11-30** `plastered_crab` [💬╎general-help] Reclaimer supports CE- ODSt I believe, but adjutant 4.2.0 is required for Reach/4
- **2021-11-30** `unordinal` [🔍╎texturing-and-shading] After some looking into it and help from Soulburnin, here's how the Halo Reach object/biped `Change Colors` works:
  The `Change Colors` datablock has four elements total, numbered 0-3 (though this depends on the asset). Each element contains a list of variants, which themselves contain the variant name and two colors.
  
  The four elements of the `Change Colors` block each correspond to a color channel in the asset's color control (CC) map in the order you'd expect. The diffuse color of the object is mixed with these colors based on the CC map.
  `Element 0:` Primary color (masked by the Red channel in the CC map)
  `Element 1:` Secondary color (masked by the Green channel)
  `Element 2:` Tertiary color (masked by the Blue channel)
  `Element 3:` Quaternary color (masked by the Alpha channel)
  
  Two of these colors - the tertiary and quaternary colors - have a secondary usage for tinting the fresnel and specular respectively of some assets. This is dependent on the shader, but chances are, if there's not an explicit _specular\_tint_ parameter in the Assembly Halo shader of your asset, the tint is controlled instead through the `Change Colors` datablock in the asset's tag (whether it be a '.biped' or something else). These tints do not use the CC map.
  `Element 2:` Fresnel tint
  `Element 3:` Specular tint
- **2021-12-01** `xsoulstacex` [💬╎general-help] Hey all! I'm trying to use Reclaimer and Architect to extract the multiplayer spartan from Halo 3. I plan on unchecking all the armor pieces except for my spartans armor set so I can pose/animate him in blender for videos or thumbnails. My first hurdle atm is Reclaimer crashing every single time I try to save anything anywhere. I've been troubleshooting for hours and would appreciate any help 🙂
  
  I am trying to save as an obj extracting only the selected geometry, I installed Architect properly and followed the install guide to a T
- **2021-12-01** `xsoulstacex` [💬╎general-help] Thanks for pointing me the right way R93! I got my model in blender working fine. Do you know how I would now begin ripping the textures and applying those? I was told I'll have to make my own visor shader especially if I want a new age 343 animated visor
- **2021-12-01** `xsoulstacex` [💬╎general-help] Thanks 🙂 so your video covers getting the shaders, applying them to armor, making sure they match in game color options, and getting the back piece? I only ask because if this goes well with my Spartan, I will eventually want to be able to get my friends spartans as well with different everything. In that case I think I'd want to download your models and shaders provided and remove the excess, or is it still easier to rip them each? I've got ripping them figured out down pat now.
  
  I'll ignore armor effects for now but would you also happen to have a lead on getting the animated visors  textures? I can deal w default visor if I got everything else working
- **2021-12-01** `xsoulstacex` [💬╎general-help] Anyone have any leads for me on ripping / recreating the animated visors from halo 3? I was thinking that making a custom gradient texture (or whatever the visor looks like) and writing a shader to scroll through the texture's y coordinates would be doable, that's all that 343 are doing in engine anyways. Thing is I don't know how to do that in blender. Any visor texture would be a start, Reclaimer didn't give me one. I think I can ignore the back piece for now
- **2021-12-01** `xsoulstacex` [💬╎general-help] Any of my problems sound easy to solve to you? The incorrect bone mirroring has a video showing my struggle above here. I figured selecting armor colors or adding color to the visor should be easiest. I'm hoping the back piece will be as easy as ripping it from the game separately and gluing it on the back
- **2021-12-02** `xsoulstacex` [💬╎general-help] Theres no color for it in the Reclaimer viewport either, I assumed I exported without visor color/textures?
- **2021-12-03** `captnkoda` [💬╎general-help] Brand new here, where's the first port of call to extract multiplayer spartans from Halo 3?
- **2021-12-03** `deskclaw` [💬╎general-help] import the textures as well, then extract the materials from the FBX in unity, set the materials to Lava's shader and slot the textures in
- **2021-12-04** `unordinal` [💬╎general-help] I'm trying to extract parts of a level from Halo Reach via Reclaimer but the .scenario_structure_bsps are just showing up as a single giant pyramid in the model viewer. Is Reclaimer broken for that right now or am I being dumb?
- **2021-12-11** `venator3d` [📜╎ripping] Are there any collections of ripped multiplayer assets of all the gear so far?
- **2021-12-15** `peestain37` [💬╎general-help] ive only messed with assembly in halo 2 to find out i can ruin multiplayer matches
- **2021-12-17** `kalawaki` [📜╎ripping] Would it be possible to rip the Elite sword animations from Infinite using this? :thinkingchief:
- **2021-12-17** `brooen` [📜╎ripping] if ur ripping h4 ur prob gonna also want the shader from #🔗╎shaders-resources
- **2021-12-20** `gho57x90` [💬╎general-help] Initially, I was attempting to extract sound stuff from the Halo 4 network test beta, but after finding out it won't let me extract anything and attempting to extract from the Forge World map in Halo: Reach (MCC), I can't get it to work from there either.
- **2021-12-28** `_hauki_` [💬╎general-help] I'm tempted to grab a batch for use in training a voice for xVASynth, specifically elite voices from Infinite, but I dont have Infinite nor an extractor
- **2021-12-31** `john.h.blacksad` [💬╎general-help] Very new to ripping and I keep getting this error everytime I try to open a .map file from reach in reclaimer
  [attachments] unknown.png
- **2022-01-01** `coarforge` [🔍╎texturing-and-shading] I can quickly, but I'm off to bed after that. You go onto the github page and select the dev branch (the link I sent already has that branch selected). You then download the code as a .zip file, which you can then install as an addon in the user preferences in blender. To be able to import textures, you need to install the module texture2ddecoder into blenders python environment by running a few commands in the python console in blender (the commands are in the readme.md). You can then go to import -> Halo infinite rendermodel and import a rendermodel. If you want to import more then just the 3D model (so textures), you have to set the root path property to the root path of where you extracted the modules to (the filechooser doesn't work, so you have to copy in the path). Same for the shader .blend if you want it to automatically apply that shader.
- **2022-01-04** `slimbopulous` [📜╎ripping] what programs are able to rip the animations from reach? i know bonobo exists, but its not compatible with both 360 and MCC versions
- **2022-01-04** `plastered_crab` [📜╎ripping] Reach is almost completely supported with Bonobo, but First Person animations won't rip
- **2022-01-04** `slimbopulous` [📜╎ripping] they don't? interesting, i remember seeing a video where someone managed to rip reach viewmodel anims and port them over to ce
- **2022-01-05** `brooen` [💬╎general-help] get the shader for the game you are ripping from in #🔗╎shaders-resources
- **2022-01-08** `opr_23b` [💬╎general-help] I wanted to ask a question regarding Halo 2 assets, texture stuff, and whatnot.
  
  So, I've extracted the model of the Elite using reclaimer, and I've imported it into blender with rigging, textures, and whatnot. (I personally preferred to rip the model myself rather than download it from a previously existing source.
  And to cut to the question itself without bloating with too much information, is it at all possible to use some sort of node setup that uses the RGB textures from the game, in order to easily retexture the Elite?
  Basically, you set a specific color in some sort of option, and it retextures the elite using the RGB mask as a "base"
  [attachments] arms_color_change.png
- **2022-01-12** `activelydrowsed` [💬╎general-help] what do I need to input in the materials box for textures to appear. Do I need the file location their, like ``C:\Users\Desktop\elite\bitmap`` ? I have them extracted but can't get blender to see them.
  [attachments] Capasdasdture.PNG
- **2022-01-13** `sabsnake` [📜╎ripping] Unsure if I can ask this here or not, please no bannerino.
  Has anyone had success in Ripping Halo Reach Maps/MP Maps with Textures?
  ( If so, what tool did you use? ) I used Reclaimer and Adjutant and got 0 Textures, just Grey.
- **2022-01-15** `opr_23b` [💬╎general-help] alright, ill try that
  i actually ripped the elite myself using Reclaimer, so I might have missed something in the files
- **2022-01-15** `opr_23b` [💬╎general-help] checking the files with Reclaimer, a shader of the same name can be seen
- **2022-01-17** `vat_` [💬╎general-help] Is thier any ways to extract the exact color values and detail maps used on characters in halo infinite?
- **2022-01-19** `tpg` [🔍╎texturing-and-shading] actually @ChromaCore @Soul how important is the grey_50_precent for Reach stuff
  
  been looking deeper into Assembly and I swear almost everything uses it
- **2022-01-27** `ubald007` [🔍╎texturing-and-shading] This is for everyone, Work-flow tip. As we mostly deal with extractions and we all know they are not always perfect!, the textures some times get messed up. these images are to help you see the errors that sometimes happen with extraction's. A. is to show you that sometimes the alignment can be off sometimes very little. Also being the Green channel in this case highlighting the edge on the ASG_Control map is a great guide, makes it easy to spot and fairly easy to correct as shown in image B. So in your work-flow before building your shader use the Control map to help you fix this misalignment. I hope this helps all of you!! CHEERs..
  [attachments] A.jpg, B.jpg
- **2022-01-29** `pajamaboy` [🔍╎texturing-and-shading] So I opened the ripped spartan (Mk VII) in blender 3.1 last night but the shader appears to be broken? I tried readding the images starting with the normals but I've had no luck so far.
- **2022-01-30** `acidmanrpgz` [📜╎ripping] His so how would one extract their multiplayer spartan from Halo Infinite (not sure if right channel)
- **2022-02-11** `.bombwolf` [📜╎ripping] Which map are the Halo Reach Elites from? Cant find them in reclaimer.
- **2022-02-12** `phoondos` [📜╎ripping] idek if you can use the high def mode in multiplayer but the screenshots that i see of blood gulch look so much higher quality than what i managed to rip with reclaimer
- **2022-02-23** `hausdev` [🔍╎texturing-and-shading] went as far as to dump the shader bytecode, but i'm only gonna try digging through that as a last resort 😛
- **2022-02-27** `plastered_crab` [📜╎ripping] You've already ported it as in you loaded it up into Blender? Or did you go so far as to spend like 2 months in Blender / Assembly and hand tweaking all the textures and tillable bitmaps for each and every surface on the map? 😅 the palm trees alone for that map have like 12 different textures they use as well as a huge terrain shader solely for them. Not to mention the skybox and whatnot.
- **2022-03-05** `defultimate12k` [💬╎general-help] question
  when i was watching the video about exporting Halo render models from Reclaimer i followed all the right steps but for some reason it doesnt keep the color of the rendered model heres an example
  The middle one is the video from Plastered Crab and his came out exactly like that but mine didnt
  are there are certain settings i need to change or configure that i missed?
  @Plastered_CrawDAD
  [attachments] unknown.png, unknown.png
- **2022-03-06** `bigsteve327` [📜╎ripping] Hi everyone! I'm trying to rip lossless (or the highest possible quality) audio tracks from Halo Reach, CE (both classic & anniversary), and 4 in MCC. What would be a good starting point for each of these games? Thanks!
- **2022-03-08** `zippingrocks` [💬╎general-help] How do I extract textures for all the forge objects from reach without painfully selecting each and pressing extract bitmaps? I want the textures
  [attachments] unknown.png
- **2022-03-13** `thejudsub` [📜╎ripping] can anyone point me to a tutorial for ripping things from H2A? And possibly how to rip maps from H3? If not halo3, then infinite is fine as long as i can get forerunner environment assets.
  I wanted to check out H2A ripping because i got the prophet model from the drive, but the UVs and weights were pretty messed up so i wanted to fix it up myself. And i need halo 3 stuff because i need the map where the prophet of truth tries to fire the rings in the ark.
  But im open to building the scene myself with infinite assets. I can figure out the shaders myself
- **2022-03-19** `yhormv` [💬╎general-help] Reclaimer keeps crashing when I export Halo reach Structure Bsps, Any thoughts?
- **2022-03-21** `williaso` [🔍╎texturing-and-shading] https://blender.stackexchange.com/questions/214925/how-to-use-a-red-green-color-map-in-blender *Maybe* this'll help? @AverageUsername
  [embed] How to use a Red/Green color map in Blender? https://blender.stackexchange.com/questions/214925/how-to-use-a-red-green-color-map-in-blender (I have modified my initial question after finding out more about the normal maps.)
  I am playing around with some ripped models and I am trying to figure out ho
- **2022-03-26** `helena_the_renegade` [📜╎ripping] iv'e been told that you can rip halo reach animations how do you do it cant get it work
- **2022-04-04** `saltyace` [📜╎ripping] So, I'm working on Tempest from Reach, right. And I've got the all the main geometry and stuff. But there's a bunch of foliage. And I have no idea where it is. I've only ever used reclaimer once to pull the main geometry with its basic textures. I have no idea what I'm doing.
- **2022-04-09** `xzize98` [💬╎general-help] I was wondering if someoine could help me out for extracting audio files from halo reach
- **2022-04-10** `Deleted User` [🔍╎texturing-and-shading] i get it, thats why i rip my own stuff, but the infinite shaders are a little too much for me, with all the fine tuning and color ramps needed i just dont have the time
- **2022-04-12** `defultimate12k` [💬╎general-help] if I wanted to use my custom halo reach spartan re-attach the model to the bone structure of the halo reach character downloaded from reclaimer and use the animations
- **2022-04-14** `zippingrocks` [💬╎general-help] Trying to extract and load up old Halo Online builds. Please reach out, I'm looking for Tool, Darkloader and anything else I might need.
- **2022-04-14** `Deleted User` [💬╎general-help] Anyone know of a good way to extract XB360 assets? Trying to snag somethings from ODST in particular
- **2022-04-19** `himagh98` [💬╎general-help] i dont see the files i have to extract, i see H3EK (Extract), but idk how to extract, if someone can send a vid of them doin it that would be helfull
- **2022-04-20** `squishypon3` [💬╎general-help] Took a bit of a break working on an H3 map and coming back trying to use reclaimer to get some .tiff files for blender. Anyone know why Reclaimer would have issues opening up h3 .map files on steam h3 mcc?
  [attachments] unknown.png
- **2022-04-23** `crashlegacy14` [🔍╎texturing-and-shading] so right now I'm working on a shader for the recently ripped GB02 models and they have a ton of masks staked in a single channel. extracting the individual masks has proven … tricky. right now my process is to use a color ramp to produce and RGBA image and break that down as normal, however I'm getting some bleed and I'm not sure how to address it.
- **2022-04-27** `snowshankington` [💬╎general-help] Hi, While looking in reclaimer at the reach club Errea area these yellow textures are everywhere, am I missing a necessary install or is it just the way it is? in the game these where all different textures I believe
  [attachments] reachtextures.JPG
- **2022-04-27** `kalebnek` [📜╎ripping] @Plastered_CrawDAD hello! I am working on an ODST project. Do you or anyone else know of a way to rip the original Halo 3 ODST Loading Screen background? Any help would be greatly appreciated!
- **2022-04-29** `dervish0` [💬╎general-help] this can help you get started if you haven't seen it already https://c20.reclaimers.net/h3/level-creation/
  [embed] H3 level creation hub https://c20.reclaimers.net/h3/level-creation/ This page will serve as the hub for level creation guides. While some of these guides are intended f...
- **2022-05-01** `gravemind2401` [📜╎ripping] I looked at the midnight build a fair while back but wasn’t able to make much sense of it. I don’t remember exactly what cache builds are around from before the beta but I think Reclaimer might already open some of them, unless I’m thinking of H3.
  Unfortunately I don’t really have plans for anything specifically at the moment though due to lack of time.
- **2022-05-02** `patfanz` [🔍╎texturing-and-shading] trying to add texture files to a ripped starwars dc-15. the normal map applies fine but the color map is wonky
- **2022-05-02** `patfanz` [🔍╎texturing-and-shading] Need help with a color map on a star wars rip. Very new to texturing so im unsure what im doing
- **2022-05-04** `09williamsad` [📜╎ripping] I am redoing my Halo Infinite rip for season 2, how did people manage to export the elite model?
  It gets vert errors when I try, I thought to try catch the erroring section of code but that did not get any usable model.
- **2022-05-04** `tpg` [📜╎ripping] I assume Season 2 props (like the new multiplayer menu) are still on the docket to be ripped?
- **2022-05-04** `fiycsf` [💬╎general-help] Gonna have to re-rip a reach pelican, since the FBX I have, has toast normals....
- **2022-05-07** `brussell09` [💬╎general-help] Anyone know a place to get Halo Skybox matte paintings? Specifically reach? You can rip them but I guess they arent good quality?
- **2022-05-08** `fiycsf` [💾╎substance-suite] So yes, and no.
  
  Let's take H5s. Halo 5 has half it's mirrored UVs off the 1:1 space.
  
  That half cannot be painted on directly and inherits from the 1:1 space. 
  
  Some games (or bad rips) place everything outside the 1:1 space, and in order to substance paint, you actually ***have to*** shift the things you are working with back into the 1:1 space when they will have started outside it for some reason.
  
  The real thing to ensure that you are moving it without actually changing what it would relatively sit on top of. 
  
  Going back to halo 5, if you wanted to seperate out the mirrored side and paint it seperately, you'd likely want to move UVs around.
- **2022-05-10** `lucid.digital` [📜╎ripping] Just wondering, has anyone ripped any elite animation files from 3 - Reach?
- **2022-05-15** `Deleted User` [💬╎general-help] hi i have a problem viewing stuff in reclaimer when i click on something it says  Error loading model: objects\characters\elite\elite.render_model this is an example of it and also says System.IO.FileNotFoundException: Could not find file 'C:\Users\AlexT\Downloads\00a_introduction\mainmenu.map'. so idk if you can help me on solving this problem
- **2022-05-19** `plastered_crab` [💬╎general-help] Map porting can be a PAIN and can take a while to get right. Just importing them from Reclaimer isn't enough as the textures require additional work and need to be manually scaled/tiled as well as setup with a proper Halo shader. After that you need to add lights to your scene. If lights are on the inside the pics you sent would probably still be black as the outside isn't lit probably due to a number of reasons
- **2022-05-23** `s059_spartantanner` [💬╎general-help] For some reason the Target Locator model from Reach is still in 4 but I have no idea why someone specifically ripped the Reach Leftover instead of the halo 4 one
- **2022-05-24** `cosmostune` [💬╎general-help] hey fellas, someone has the background for the multiplayer lobby ripped?
- **2022-05-26** `sir_swagshire` [📜╎ripping] So I've always been interested in 3d animation and I'd like to use the reach models to experiment. I've been trying to set up Reclaimer to rip some models out of Reach MCC on PC but I've ran into issues setting it up. I ran the v1.7.309 installer from Gravemind's mediafire link and it is unable to load the mission .map files saying "It's a valid .map but my not be a supported version" I'm not sure where to find the ReadMe either so I bet it's something nice and simple that I'm missing lol. Any advice?
- **2022-05-28** `elitecaden` [📜╎ripping] Has anyone ripped or somehow have the forge world multiplayer maps from OG Halo Reach?
- **2022-06-01** `plastered_crab` [📜╎ripping] So basic rundown on what to expect. The current ripping tools are only able to 
  1. Rip the higher poly models of Blue Team and Team Osiris but bad mesh normals
  2. Rip the multiplayer armors from Halo Waypoint but the models aren't as high poly as they should be
- **2022-06-14** `rafaleeater` [💬╎general-help] https://www.moddb.com/games/halo-3/downloads/bonobo-version-1003
  [embed] Bonobo [Version 1.0.0.3] Halo2/3/ODST/Reach Animation Extractor file https://www.moddb.com/games/halo-3/downloads/bonobo-version-1003 Bonobo [Version 1.0.0.3] is a Halo2/3/ODST/Reach Animation Extractor used to extract the animations from those 4 halo games, this can be found on halomaps.org, this c
  [attachments] unknown.png
- **2022-06-15** `butter1734` [💬╎general-help] so i've been tweaking with the reach models from the reach spartan pack, trying to make Jorge, but i've noticed that some things are just scaled up in game for Jorge like the Pads, and the shin of his armour is differently sized and shaped from the regular base armour in game. I don't wanna be lazy and ask for someone to rip it for me, but if someone has his shit or the full FBX on hand, i would gladly appreciate it, if not i'll just go and rip from the game myself
- **2022-06-18** `Deleted User` [📜╎ripping] Not so much encrypted.
  
  I'll explain it using Halo.
  
  Essentially the files are located inside of another file. You want an ODST from Halo 3? You gotta rip it from the mission with an ODST in it. Its not encrypted so much as it is buried - its just a case of having the right shovel to dig it out more than it is a case of having the right key for the door.
- **2022-06-21** `.blackhand` [💬╎general-help] Hey everyone! I need some help in extracting Halo Reach sound files from MCC. Any help would be appreciated!
- **2022-06-25** `rafaleeater` [📜╎ripping] Did a map-wide decompile for a halo reach map (m52, new alexandria) and cant find the actual campaign level's AMF file, only extra stuff like props and falling buildings. Any help?
- **2022-07-09** `chromacore` [🔍╎texturing-and-shading] I assume you mean infinite right? You could use IRTV or the material palette extractor to get the color values. As for other games you can use Assembly and check the bipd for the enemies for coloss
- **2022-07-14** `frustrated` [💬╎general-help] Mfw you only want the odst romeo sharp shooter model ready to print and everyone's asking an arm and a leg to give it you 😭 idk how to do this file ripping stuff
- **2022-07-14** `gameoverload` [💬╎general-help] I thought the server / drive already had ODST stuff ripped at this point?
  So you wouldn't have even needed to rip the models, just use the ones here and modify them as need be.
- **2022-07-15** `tacohombre` [📜╎ripping] does reclaimer not support halo 3 odst? I updated to the latest version and this is still happening
  [attachments] unknown.png
- **2022-07-15** `tacohombre` [📜╎ripping] I just get this error when I try and open a model
  
  ```Error loading model: objects\characters\engineer\engineer.render_model
  System.IO.FileNotFoundException: Could not find file 'D:\(NAME)\Downloads\sc150\campaign.map'.
  File name: 'D:\(NAME)\Downloads\sc150\campaign.map'
     at System.IO.__Error.WinIOError(Int32 errorCode, String maybeFullPath)
     at System.IO.FileStream.Init(String path, FileMode mode, FileAccess access, Int32 rights, Boolean useRights, FileShare share, Int32 bufferSize, FileOptions options, SECURITY_ATTRIBUTES secAttrs, String msgPath, Boolean bFromProxy, Boolean useLongPath, Boolean checkHost)
     at System.IO.FileStream..ctor(String path, FileMode mode, FileAccess access)
     at Reclaimer.Blam.Halo3.ResourceIdentifier.ReadData(PageType mode, Int32 maxLength)
     at Reclaimer.Blam.Halo3.ResourceIdentifier.ReadData(PageType mode)
     at Reclaimer.Blam.Halo3.Halo3Common.<GetMeshes>d__5.MoveNext()
     at System.Collections.Generic.List`1.InsertRange(Int32 index, IEnumerable`1 collection)
     at System.Collections.Generic.List`1.AddRange(IEnumerable`1 collection)
     at Reclaimer.Blam.Halo3.render_model.ReadGeometry(Int32 lod)
     at Reclaimer.Controls.ModelViewer.SetLod(Int32 index) in X:\Documents\Visual Studio 2022\Projects\Reclaimer.Build\Reclaimer\Controls\ModelViewer.xaml.cs:line 107
     at Reclaimer.Controls.ModelViewer.LoadGeometry(IRenderGeometry geometry, String fileName) in X:\Documents\Visual Studio 2022\Projects\Reclaimer.Build\Reclaimer\Controls\ModelViewe ...[cut]
- **2022-07-21** `plastered_crab` [📜╎ripping] Both Halo 2 classic and Anniversary Multiplayer maps can be ripped using Reclaimer
- **2022-07-25** `exporush` [🔍╎texturing-and-shading] So if I were to take a model from the model dump in Crabs drive, how would I go about texturing it properly? Thinking about grabbing the multiplayer data pad and texturing it.
- **2022-07-26** `recl8015` [📜╎ripping] Using the new Halo 2 Anniversary mod tool to extract the skyboxes for various sentinelhq BSPs. it gets me the render model but does not pull the textures attached to it. I can extract the render data of og H2 via Tool just fine. Am porting Containment to H3 and trying to get different skybox variants for it, including H2A.
- **2022-07-29** `brianthefirelord` [💬╎general-help] I believe it's this one:
  https://github.com/Surasia/InfiniteCoatings/blob/main/armor/reach/reach____rch_mil_emile.json
  [embed] InfiniteCoatings/reach____rch_mil_emile.json at main · Surasia/Infi... https://github.com/Surasia/InfiniteCoatings/blob/main/armor/reach/reach____rch_mil_emile.json A collection of coatings extracted from Halo Infinite in JSON form. - InfiniteCoatings/reach____rch_mil_emile.json at main · Surasia/In
- **2022-07-30** `gui2203` [💬╎general-help] i'm trying to rip Crater from halo 3 odst with reclaimer, but i keep getting this error
  [attachments] unknown.png
- **2022-08-01** `baconator650` [💾╎autodesk] Sorry should have clarified, that this is working with animations that were extracted from Halo and then imported. I’ve used animation save/load and find it too limited when working with multiple assets in a large scene. Animation layers work best (for my needs). Experimenting with Blender’s animation importer I noticed it automatically puts animation into a layer.
- **2022-08-05** `soundflare` [💬╎general-help] Hi, am doing some stuff concerning reclaimer and I have a question concerning the covenant Corvette interior of m45 (Long Night of Solace). Am on reclaimer and it seems to only be mostly grey and stuff, is it usually like that model wise instead of purple like in game. (Fixed the issue with the yellow color all around after looking at the text tutorial)
- **2022-08-05** `soulburnin` [💬╎general-help] I'm a little wary of just releasing the shader because it's meant to be used in tandem with Assembly and eventually mod tools
- **2022-08-05** `plastered_crab` [💬╎general-help] I think a modern tutorial video with your sultry voice going over the ins and outs (kinda like my tutorial video of H3 map porting using Assembly data) and it should help the masses a lot 🙂
- **2022-08-07** `garrusflex` [💬╎general-help] Anyone else think this looks like it was straight up ripped from reach and they just stuck some extra bits on it ? Or am I just being silly ?
  
  https://www.artstation.com/marketplace/p/v9395/rocket-platform
  [embed] ArtStation - Rocket Platform | Game Assets https://www.artstation.com/marketplace/p/v9395/rocket-platform Game Assets - Rocket Platform, USD $14.95. Rocket Platform is an autonomous 3D rocket launcher. The platform is designed to engage air and ground targets the enemy. The unit is equipped with thr
- **2022-08-08** `theonedutchguy` [💬╎general-help] I don't think that it is a Halo: Reach rip, it looks different enough to me, and that Reach turret is based on a fairly commonly-seen real-life design, so I think that that other turret could very well have used the same source of inspiration
  [attachments] Mk_49_Guided_Missile_Launching_System_RIM-116_RAM_1.15.jpg
- **2022-08-10** `brooen` [🔍╎texturing-and-shading] well then download the mod tools or assembly and you can look at the color values
- **2022-08-13** `caia.` [🔍╎texturing-and-shading] How exactly does the grunt illum shader work?
  
  -There doesn't seem to be a dedicated texture for it in the bitmaps folder
  -Guerilla says that it's using backpack.tif
  -The UVs (see pic) are in places that don't produce anything that make sense
  -Reclaimer shows circles and lines that appear to be textured and not generated.
  
  What gives?
  [attachments] 2022-08-13_08_12_27-Blender__C__Users_sunst_Documents_Development_Halo_Halo_2_objects_characters_gr.png
- **2022-08-15** `chunch` [🔍╎texturing-and-shading] Cause whenever we get decompiled shader code for infinite I’m gonna have to remake my entire shader
- **2022-08-19** `gameoverload` [💬╎general-help] Depending on how you extracted the files out of that main VPK for the ODST that you have, it should have gotten everything out of it, including textures.
- **2022-08-21** `micahhalcyon` [💬╎general-help] hello i am trying to use reclaimer to get assets from Halo 3 ODST. whenever i try and open a map file i get this error. I just downloaded the program so i am on the latest edition and I also tried to update and it says i am up to date. Any Ideas?
  [attachments] Capture.PNG
- **2022-09-06** `plastered_crab` [📜╎ripping] and I have a tutorial for ripping reach animations as well if you need that
- **2022-09-08** `blakjewellio` [💾╎autodesk] on its defense I like the rigs I can find made in maya im lokin at this chat log and asia said 343 uses maya wonder if I can find some of those How did he rip infinites spartans
- **2022-09-12** `wraith0912` [📜╎ripping] so uh, quick question regarding stuff from SPV3
  if something isn't in the dedicated assets channel, is it safe to just go ahead and grab it or does SPV3 still fall under the "ripping from mods is no bueno" category?
- **2022-09-13** `squiggilysquid` [💾╎autodesk] Does anyone know if any rigs were ripped from Halo Infinite and uploaded anywhere? Want to try get the elites rigged up and am reluctant to create my own!
- **2022-09-15** `r93_sniper` [🦴╎animation] ```full\path\to\map.map
  EditTag tag\here.jmad
  ExtractAnimation all "C:\your_folder_here"
  
  for example:
  G:\Steam\steamapps\common\Halo The Master Chief Collection\halo3\maps\original360maps\010_jungle.map
  EditTag objects\vehicles\pelican\pelican.jmad
  ExtractAnimation all "C:\Program Files (x86)\Steam\steamapps\common\H3EK\extracted_h3_anims"```
- **2022-09-17** `doomman` [📜╎ripping] Is it possible to access "shared" in reclaimer? Trying to get foliage from reach but I get an error when opening shared
- **2022-09-17** `degweb` [📜╎ripping] so are the bones from ripped models from halo reach (using reclaimer) meant to be incredibly small
- **2022-09-19** `Deleted User` [📜╎ripping] Is there any program that allows you to rip Halo 2 Anniversary multiplayer maps? I have index but does it only do campaign maps?
- **2022-09-20** `gameoverload` [🔍╎texturing-and-shading] Specular and color is working, and all of the 3.0 code is there, alongside the builds of 1.0,2.0,3.0 in case I need stuff from them, otherwise I just need to finish my decompiled build of 4.0, then compile it together into a fully working 4.0.
- **2022-09-22** `spartanj104` [📜╎ripping] Hello, 
  I saw that there was no "Infinite Brute Pack" so I stick to the task...even though I've never done that in my life and I am struggling xD.
  I've already ripped the game, output the raw models, sorted through Blender and now I come to the part I hate the most: the textures/shader/UV.
  I try to watch tutorials on this but it doesn't help me understand how you managed the textures like on "Infinite Spartans Pack" I need someone to show me the light 😅 🥲 
  Thank you in advance ✋  🙂
- **2022-09-27** `somethinspartan` [💬╎general-help] Hey so I finally imported the map condemned from reach into blender, and couldn't figure out how to add my character into it without appending it, and I think it might be causing a problem for me to not have my character save it in place when I close out of blender, so the real question is, is appending a model or object the wrong way of adding a character into another project, or is it something else? (sorry for a dump text)
- **2022-10-05** `brooen` [📜╎ripping] @swagster I wonder if we can rip the reach vertex colors with mod tools now
- **2022-10-06** `gravemind2401` [💬╎general-help] if it’s imported from dae it could mean anything really, Reclaimer does include the vertex colour in collada output when possible but the game uses it for a bunch of things depending on the shader and idk what properties the importer will be setting
- **2022-10-18** `soulburnin` [🔍╎texturing-and-shading] For my cubemap shading I just run the extracted ones from Reclaimer through Flexify 2, crop out the borders, then it's done
- **2022-10-26** `mas2terchi0ef` [💬╎general-help] Hi I have a question is it possible to extract the H2A assault rifle files from multiplayer?
- **2022-10-26** `mas2terchi0ef` [📜╎ripping] Hi I have a question is it possible to extract the H2A assault rifle files from multiplayer?
- **2022-10-27** `gameoverload` [💬╎general-help] .fxo compiled shader effect files specifically that I have found no way of fully decompiling.
- **2022-10-29** `gameoverload` [💬╎general-help] Who knows what H5 is doing till we get proper decompiliation or debugging of its shader code.
- **2022-10-29** `gameoverload` [💬╎general-help] You can guestimate based on the textures and visual reference to make changes to the shader or in the making of a new one, but otherwise, their isn't much else that can be done to get it accurate till H5 shader code is decompiled or debugged.
- **2022-11-03** `fable_law_458` [🔍╎texturing-and-shading] Hi, Casey. If you're looking to see what specific material that you're looking at in CE, Go in Sapien or Standalone. Open whatever level it may be, then, type into the console `debug_camera true`. What this will do is it will display the shader name and the directory on the top left of the screen on whatever shader you're looking at that it's using. 
  
  As far as pixel shading or vertex shading you mentioned about, I don't think you can view those inside the Halo: CE's engine. You may need to speak to someone on reclaimer's discord server that might know the answer to that question.
  
  I hope this does assist you with what you're wanting to do.
  
  Best regards,
  -fable
- **2022-11-07** `gameoverload` [💬╎general-help] The honest way for shading is honestly ripping apart game assets and putting them together accurately using as much information as you can on them as that will teach you the most on things, as it did me. As long as you have the right tools to view shader code or capture data.
  Basically, you will want to learn some stuff on HLSL and Assembly, which is what your effectively creating in a node graph. Unity in particularly even lets you see what your making in node graph as HLSL.
  I would give ya more of my personalized information, but I have yet to really start my 'Practical Shading Compendium' to a full extent to just hand ya that.
  Texture quality is really down to time taken and purpose, what is a texture doing. Plus if you want the most quality out of say simple BW masking or RGB channel packed masking for example, you would need a high poly model that can be a lot more polygonal specific on that masking, rather than a low poly model that texture was baked to be put on.
  Animations are a different story since your getting into rigging and literal animating, which is not my forte, and you would need someone else to really tell ya more.
- **2022-11-07** `gameoverload` [🔍╎texturing-and-shading] If that is not what your looking for, at that point you will need to then rip all the geo, and make sure all that is set up as you see in game, then begin to grab all of the need textures, texture data like tiling, colors, etc, then you will need to find all of the shader code data, ala in the Reach Tools, for the environment shading you need for that level, then you would build a shader from that, then after that, you input all of the textures and values you have into that shader.
- **2022-11-11** `gameoverload` [💬╎general-help] It does appear all of these rips are lacking in custom split normal data, which could be causing it, it didn't come with a normal map node on the shader by default, which could also be telling of the possible cause, and the UV map is call geometry source, as if it was just the raw geometry, nothing else for any program to manipulate other than the raw geometry.
- **2022-11-11** `waketheshark` [💬╎general-help] Is there a way to access the areas that cutscenes take place in? I'm specifically wanting to access one of Reach's where Noble Six is overlooking New Alexandria. Alternatively is there a way to extract that level into blender somehow
- **2022-11-12** `waketheshark` [🔍╎texturing-and-shading] Not sure if this is the right place for this but I'm trying to use Reclaimer for exporting Reach maps. I followed what I can find and downloaded the shared reach maps as well as the level I want to open. I have them all in one folder extracted from their zip folders. I can open the level I want in the 'Map Viewer' window with all of its folders. But I'm getting an error: "System.IO.InvalidDataException: Found invalid data while decoding" when trying to open one of the render_model files like in the youtube tutorial and I can't wrap my head around how to fix it
- **2022-11-12** `waketheshark` [🔍╎texturing-and-shading] Well I put them in their own folder within the Reclaimer install under Data/Halo Reach/ does Reclaimer not like it when they're in its folder?
- **2022-11-13** `waketheshark` [📜╎ripping] I'm following the tutorial to export from reclaimer and to import into blender. I've exported the .amf for m50_000, and within the same folder, exported all bitmaps of m50_000. But when I import into blender with the AMF importer (making sure that it's looking for tiff files), nothing is textured. Is there something extra I need to do for Structures? I have all of the shared reach maps: campaign, mainmenu, and shared.
  [attachments] image.png
- **2022-11-13** `ricky764` [💬╎general-help] How do you extract textures from Halo Reach? Thanks to R93_Sniper I was able to get the model of the Jackal and Skirmisher from Halo Reach, but they don't have textures, so I'd be interested in getting them from the game itself.
- **2022-11-14** `tacohombre` [💬╎general-help] do you want to extract the models from halo ce-reach or something else?
- **2022-11-15** `gameoverload` [💬╎general-help] Better, first of all, you have c20 as a resource, which should have been your first research point.
  https://c20.reclaimers.net/h3/
  Secondly, what are you porting to Halo 3 exactly?
  [embed] Halo 3 https://c20.reclaimers.net/h3/ Halo 3's box art featuring the Master Chief  Halo 3 is the third instalment of the Halo game series ...
- **2022-11-15** `waketheshark` [📜╎ripping] Just checking in again one more timr if anyone knows what the issue is or how I can find it. A quick recap: wanting to export the map from the into cutscene of Exodus from Reach where Noble 6 is looking over New Alexandria.
  The mission file in question is m50 with the structure file being m50_000. I've exported the AMF of m50_000 as well as every bitmap from m50.map in tiff. These have all been exported to the same folder on my desktop, haven't moved the files around at all. Using the AMF importer for blender via the YouTube tutorial, I can import the map itself but the importer must fail to find the textures as there are none when switching to texture view.
  
  Following the main 1 hour tutorial from Plastered Crab, he doesn't show any troubleshooting for texture import failures with regards to map importing to blender. Another thing worth mentioning is that in the video, the Reclaimer preview of the map he is looking at is textured whereas mine is just yellow (I've made sure I have the required reach .map files).
  
  On the video a few people have reported the same issue but never reported having fixed it and Crab suggests it is the importer looking for the wrong file type. I've verified that the importer is looking for tiff bitmaps so that shouldn't be the issue.
  
  Appreciate any help and thanks for the help so far!
- **2022-11-15** `gameoverload` [💬╎general-help] Now honestly I would have suggested, to get used to working with models, textures, and shading as a whole would have been to simply start with a ripped character model in Blender, with its textures, could be whatever game you want that you can rip, then you would start trying to create the shader within the material your dealing with on that character, say the chest if you have an armored character.
  Then you would begin by figuring out what each texture is and what its constitute channels are as maps.
  Now if it is an Unreal game or something cracked open fully like a Halo title, you can also get Values used in that game as well that corresponds to whatever that value is meant to change.
  
  Then you would begin by figuring out the actual 'math' or nodes that get you the proper look at the end that the asset has in their corresponding game.
  
  For example, XCOM 2 character models from the base game, such as a soldier chest customization uses 3 textures overall mainly, with others being accessory to those 3 or used in the math itself.
  That is the:
  Diffuse ( Albedo and Roughness )
  Control ( Can not remember the actual name, but it includes Metallic and Ambient Occlusion )
  Lastly ending in the Normal texture.
  It also has a several values used in the shader the game uses that you can get via the XCOM 2 WOTC developer tools.
  
  The math itself is then easy since you can see it in node graph form in the developer tools that you would just manually recreate in Blender, as I am doing in one  ...[cut]
- **2022-11-19** `gameoverload` [🔍╎texturing-and-shading] So for H2 and H3, do they get converted to normal maps in the game as well or does Reclaimer simply do that for sake of ease of use?
  That is the real question.
- **2022-11-26** `egnbv` [💬╎general-help] i need halo 5 multiplayer armor 3d models, i how do extract it? pls help me, thanks 🙂
- **2022-11-30** `kalawaki` [📜╎ripping] Is there a program for ripping Halo Reach maps? Specifically looking to get them into Blender.
- **2022-11-30** `jadedqrow` [📜╎ripping] It’s called reclaimer. I don’t have any links on hand but discord has a search function you can use, you’ll have to make your own shaders for it too look game accurate though don’t think anyones got specific ones for reach maps
- **2022-12-03** `cameraguy` [📜╎ripping] Does anyone know how to rip the shell casing models from H3?
  (Like where I the .map I need to look)
- **2022-12-05** `kalawaki` [🔍╎texturing-and-shading] So I have successfully extracted the Sword Base campaign level
  
  I would like some tips on how to make it not look terrible, because I don’t have any clue what a shader even is much less how to make one that looks game accurate, lol
  [attachments] D3398EC8-7F38-43D8-9DFB-4347E561DA08.png
- **2022-12-08** `stumpyj` [🦴╎animation] Is anyone here familiar with both 3ds max scripting and blender scripting? I am converting a script from max to blender, and everything is great until I do animations. Both of these animations were extracted using the same .gr2 data (left: max - imported to blender as fbx, right: blender). Both the good animation and the bad work on either rig identically to eachother, telling me the rig is not the issue.
  [attachments] image.png
- **2022-12-13** `jadedqrow` [📜╎ripping] @Gravemind2401 hey sorry to bother gravemind kun, but the color lut in h4 doesn’t rip at all right, first pic is in modtools second is exported througu reclaimer
  [attachments] bfbcd7c901379e38d0ba932297aa077d.jpg, default_16_color_grading.jpg
- **2022-12-24** `r93_sniper` [📜╎ripping] ```full\path\to\map.map
  EditTag tag\here.jmad
  ExtractAnimation all "C:\your_folder_here"
  
  for example:
  G:\Steam\steamapps\common\Halo The Master Chief Collection\halo3\maps\original360maps\010_jungle.map
  EditTag objects\vehicles\pelican\pelican.jmad
  ExtractAnimation all "C:\Program Files (x86)\Steam\steamapps\common\H3EK\extracted_h3_anims"```
- **2023-01-07** `gameoverload` [🔍╎texturing-and-shading] That is one part I do get stuck on actually, properly ripping the area that a color resides in, specifically when I am doing Blender node stuff.
- **2023-01-12** `clericg` [💬╎general-help] Does anyone know where the Halo CE multiplayer gun sounds are located in MCC? I wish to extract and change the pitch
- **2023-01-16** `.moonzen.` [📜╎ripping] is there a newer way of ripping reach animations? i used the video tutorial and it just keeps saying "no valid armature" and stuff like that when i import the jma. i think bonobos scuffed or something
- **2023-01-17** `e.nine` [📜╎ripping] I used Refinery to dump the data and tags, is there another tool to get the texture? I just got a bunch of .bitmap files for the Elite's textures
- **2023-01-23** `jonathanborges3d` [💬╎general-help] Hi! What is the file of halo reach that contains the assassination animations? Or there are ripped halo reach animations somewhere? Thanks in advance.
- **2023-01-23** `plastered_crab` [💬╎general-help] There is a tutorial on how to rip animations from Reach, however the tool doesn't support all animations from reach as that was never finished
- **2023-01-27** `surasia` [🔍╎texturing-and-shading] https://github.com/Surasia/InfiniteCoatings/blob/main/armor/reach/reach____rch_mil_jorge.json
  [embed] InfiniteCoatings/reach____rch_mil_jorge.json at main · Surasia/Infi... https://github.com/Surasia/InfiniteCoatings/blob/main/armor/reach/reach____rch_mil_jorge.json A collection of coatings extracted from Halo Infinite in JSON form. - InfiniteCoatings/reach____rch_mil_jorge.json at main · Surasia/In
- **2023-02-01** `gameoverload` [💬╎general-help] We should get coar to repo some of those shader code rips so we can go through them more.
- **2023-02-08** `ryley7359` [📜╎ripping] Hey guys,
  I am new here but I have been ripping models and BSPs for a couple of months while also learning blender. I want to recreate the ODST drop sequence as seen in Halo 3 ODST but I have run into a problem. Some of the scenario_structure_bsps cannot be extracted from Reclaimer. They refuse to open in the model viewer and extracting the geometry does not work either. Keep in mind that this only happens on some BSPs and not others, for example I can extract sc110_020 - it is just sc110_030 and some other random files. I am using the latest version of Reclaimer and the xbox map files found on haloce3. I am starting to notice a pattern where it happens on every second bsp. Anyways enough rambling, hopefully someone can find a solution and thanks in advance. The output Reclaimer gives me is right here:
  ||Loading model: levels\atlas\sc110\sc110_030.scenario_structure_bsp
  Error loading model: levels\atlas\sc110\sc110_030.scenario_structure_bsp
  System.NullReferenceException: Object reference not set to an instance of an object.
     at Reclaimer.Blam.Halo3.scenario_structure_bsp.<>c.<ReadGeometry>b__57_2(TagReference lbsp)
     at System.Linq.Enumerable.SelectEnumerableIterator`2.MoveNext()
     at System.Linq.Enumerable.TryGetFirst[TSource](IEnumerable`1 source, Func`2 predicate, Boolean& found)
     at Reclaimer.Blam.Halo3.scenario_structure_bsp.ReadGeometry(Int32 lod)
     at Reclaimer.Controls.ModelViewer.SetLod(Int32 index) in X:\Documents\Visual Studio 2022\Projects\Reclaimer.Build\ ...[cut]
- **2023-02-10** `ryley7359` [📜╎ripping] Yeah I have tried that, it just shows this in the output.
  ||Loading model: levels\atlas\sc110\sc110_030.scenario_structure_bsp
  Error loading model: levels\atlas\sc110\sc110_030.scenario_structure_bsp
  System.NullReferenceException: Object reference not set to an instance of an object.
     at Reclaimer.Blam.Halo3.scenario_structure_bsp.<>c.<ReadGeometry>b__57_2(TagReference lbsp)
     at System.Linq.Enumerable.SelectEnumerableIterator`2.MoveNext()
     at System.Linq.Enumerable.TryGetFirst[TSource](IEnumerable`1 source, Func`2 predicate, Boolean& found)
     at Reclaimer.Blam.Halo3.scenario_structure_bsp.ReadGeometry(Int32 lod)
     at Reclaimer.Controls.ModelViewer.SetLod(Int32 index) in X:\Documents\Visual Studio 2022\Projects\Reclaimer.Build\Reclaimer\Controls\ModelViewer.xaml.cs:line 107
     at Reclaimer.Controls.ModelViewer.LoadGeometry(IRenderGeometry geometry, String fileName) in X:\Documents\Visual Studio 2022\Projects\Reclaimer.Build\Reclaimer\Controls\ModelViewer.xaml.cs:line 102
     at Reclaimer.Plugins.ModelViewerPlugin.DisplayModel(ITabContentHost targetWindow, IRenderGeometry model, String fileName) in X:\Documents\Visual Studio 2022\Projects\Reclaimer.Build\Reclaimer\Plugins\ModelViewerPlugin.cs:line 117||
- **2023-02-11** `os_isaac` [📜╎ripping] Forgive me if this gets asked frequently, but is it possible to rip single layers of music from the games? I'm wondering if it's possible to find just that drum sequence from the beginning of the H3 track "Black Tower"?
- **2023-02-16** `priception` [🦴╎animation] Has anyone been able to rip animations from Halo 2 Anniversary Multiplayer?
- **2023-02-17** `priception` [📜╎ripping] Has there been any updates in being able to rip Halo 2 Anniversary Multiplayer animations or no?
- **2023-02-17** `jagerd274` [💬╎general-help] somebody here have been rip emblems form multiplayer?? The Emblem generator web dosent work at this moment, athleast for me
- **2023-02-21** `cameraguy` [📜╎ripping] I'm trying to rip H3 Orbital (spacecamp) with Reclaimer and I'm getting incompetent rips
  (Missing parts of the map)
  Could someone else rip it for me
- **2023-02-28** `flerp` [📜╎ripping] So, I have every map, model, texture, and sound from H1, H2, and H3, but want to rip Infinite
  There any tools available for extraction? I assume It's structured differently, unless it's still a continuation of modifying the Havok engine
- **2023-03-03** `doomman` [📜╎ripping] How can I rip audio with reclaimer? Trying to get some reach sfx but theres no option to extract audio
- **2023-03-08** `flipdark95` [📜╎ripping] Are there any recent rips of Halo Infinite available online somewhere? @Surasia's module unpacker doesn't seem to work anymore for the most recent update of Infinite. It keeps coming back with a "Infinite Parse" error when it tries to extract what I assume is the updated multiplayer module.
- **2023-03-09** `ultrabooster` [📜╎ripping] Before I try doing it myself, I was wondering, has anyone already extracted the textures for the troopers in Reach?
- **2023-03-13** `burgereich` [📜╎ripping] Tryin' to get the reach model into Unity...
  But it dumps all the texture info, and I can't find a way for it to assign colors using the color map.
- **2023-03-16** `ze_du_leite` [💬╎general-help] Good afternoon halo fans, I'm new to the group, how are you? anyway, some experts here could solve a question for me?
  it is possible to extract the dubbing files (the voices) of the HALO REACH characters, I wanted to extract the voices in Portuguese PT BR.
  
  Can someone help me?
- **2023-03-26** `plastered_crab` [📜╎ripping] in that case you will likely need a few things
  1: the .map files of the level !maps
  2: the mod tools for Reach from Steam so you can see what the values are for each material
  3: to use Reclaimer to rip the level geo and get it into Blender
- **2023-03-27** `gameoverload` [📜╎ripping] It uses HLSL, but their stuff is compacted into special files most of the time, so if your wanting to rip the shader code from any game with mod tools, you'd need to go look at the mod tools, for 5 you'd need something like what Coar knows about, same for Infinite.
- **2023-04-03** `natespidey18` [📜╎ripping] Does anyone know how to rip a skybox with Reclaimer (ex, Halo Reach - Breakpoint)
- **2023-04-04** `haloboxstudios` [💬╎general-help] Hey, uhhhh- really stupid question from a really stupid person here. I downloaded the newest spartan pack today and extracted the pack and textures into a new folder on my laptop, but when I open up the Blender program all my assets are locked in a solid gray color. Am I missing something here?
- **2023-04-05** `kalawaki` [🔍╎texturing-and-shading] So I ripped the Sword Base campaign map a while back.
  
  Does anyone have any Halo Reach shaders I can use to make this look a little less...*this?*
  [attachments] Project.png
- **2023-04-07** `natespidey18` [📜╎ripping] does anyone know where i can find this UI within the Reclaimer Extractor? Halo Reach Sniper
  [attachments] Screenshot_681.png
- **2023-04-07** `kingmvm` [🦴╎animation] Hi guys, I'm going to be getting into this to rip assassination animations from Reach
- **2023-04-25** `gameoverload` [💬╎general-help] I have also replicated the two seperate varations of the X2 WOTC armor shader in a seperate file, ala the clear coat and opacity variants, meant for assets with clear coat or opacity shading, clear coat being used only for the Spectre enemy unit, and the opacity variant used for assets like the ripped up Reaper sleeves.
- **2023-05-01** `hobbes240` [📜╎ripping] Hi, I'm new to this community and well applications like blender and all, however I'm understanding quickly as there are a lot of tutorials out there. One issue I am having that I need help with is ripping the animation for assassinations with Bonobo for Halo Reach then transferring it to Blender. I have seen the tutorial from PlasteredCrab, however one version of Bonobo is telling me that the Reach Files are unsupported, while the other is saying it can not find the file. Bonobo is working for Halo ODST, but isn't working for Halo 3 either. I'm uncertain if maybe the application is no longer working or what. I used this link for .map files https://haloce3.com/xmf/, downloaded all of the shared files for Reach, ODST, and 3. As for the .net downloads, this one works https://www.microsoft.com/en-us/download/details.aspx?id=48130, but this one doesnt https://www.microsoft.com/en-us/download/details.aspx?id=53344. Does Anyone know what the issue could be and a solution for it?
- **2023-05-04** `cameraguy` [📜╎ripping] For some reason with Reclaimer when I Rip from 3, ODST, Reach, maps half of the props I see in the Reclaimers preview are missing in the exported .amf
  I'm using the Halo xmf website .map files
  
  Any ideas?
- **2023-05-13** `gameoverload` [🔍╎texturing-and-shading] Okay, so some things I am noticing:
  1 - Make sure to have put in the alpha of the diffuse into the alpha for the haircards.
  2 - Make sure your using rocket_haircards.
  3 - The textures seem to be base color / bc, AO / ao, roughness / ro, unknown / tr, metallic / m, Mask / mk, and others I am not sure about.
  4 - Make sure all of your meshes have auto-smooth on and a weighted normal modifier added to them.
  
  All in all these rips seems not all that good, certain pieces are missing, the mouths and other parts seem to be all messed up in terms of some parts in some parts of meshes, and other parts are their own meshes, textures are missing..all that.
  
  Honestly I even am not sure what needs to all be done to fix those issues since I'd need to deep dive into the tools ( I also don't own the game ) available. 
  
  Really honestly you'd need to debug the game to really know what is to what and what your missing, especially with hair, textures, shading, etc, as none of what I am seeing here is really telling you much nor the tools will most likely tell you much at all if this is all they can pull.
- **2023-05-14** `itslemurfan` [📜╎ripping] has anyone ripped any reach brutes ? wanted the reach chieftain for a project.
- **2023-05-21** `sierrad421` [📜╎ripping] Hey, how would I extract emblems from MCC that I can't find in the files for CE, H2, H3, etc?
- **2023-05-21** `brooen` [💬╎general-help] **CE**
  Models: 
  Shader Data: https://store.steampowered.com/app/1532190 (Guerilla)
  
  **CEA**
  Models: https://github.com/Wildenhaus/IndexV2/releases (Standalone program)
  Shader Data: https://github.com/Wildenhaus/IndexV2/releases (Standalone program)
  Auto Tool: https://drive.google.com/file/d/1HPXYhohdTeKfkupVGe2R9TrwyFxx0pkE/view?usp=share_link (Blender add-on)
   
  **H2**
  Models: 
  Shader Data: https://store.steampowered.com/app/1613450 (Guerilla)
  
  **H2A**
  Models: https://github.com/Wildenhaus/IndexV2/releases (Standalone program)
  Shader Data: https://github.com/Wildenhaus/IndexV2/releases (Standalone program)
  Auto Tool: https://drive.google.com/file/d/1PPl7rqegFW8g_Zbs1vRK20Q8t16KaAZP/view?usp=share_link (Blender add-on)
  
  **3**
  Models: https://github.com/Gravemind2401/Reclaimer/releases (Standalone program)
  Shader Data: https://store.steampowered.com/app/1695791 (Guerilla)
  Auto Tool: https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool (Blender add-on)
   
  **ODST**
  Models: https://github.com/Gravemind2401/Reclaimer/releases (Standalone program)
  Shader Data: https://store.steampowered.com/app/1695794 (Guerilla)
  Auto Tool: https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool (Blender add-on)
  
  **REACH**
  Models: https://github.com/Gravemind2401/Reclaimer/releases (Standalone program)
  Shader Data: https://store.steampowered.com/app/1695793 (Foundation)
  
  **4**
  Models: https://github.com/Gravemind2401/Reclaimer/releases (Standalone program)
  Shader Data: https://store.steampowered.com ...[cut]
- **2023-05-25** `felixvyra` [📜╎ripping] Thanks I'll take a look. But I think I figured out what was wrong. I ripped Master chief but I'm missing the specular map. I have color, control and normal. But no specular
- **2023-05-25** `urium86` [📜╎ripping] no, is just to share my last build off our tool of HIRT, that have some options to get some shader decompiled version, and in some point helps us to figured how blend w vertex work
- **2023-05-28** `a.hypocrite` [💬╎general-help] I've come to bother you all about something arbitrary that I'm just now noticing and can't figure out-
  
  > *Attempting to add camos to individual Spartans- these camos are pre-colored (images ripped straight from google) and are applied through an image texture and plugging the color nodes into the appropriate zone.*
  > 
  > *When shown in material view, the textures are applied. But when transitioning to rendered view, they're replaced with a silver color. For the sake of ruling out lighting not interacting well with blue tones, I've also tested with a model using a darker-colored camouflage. Same issue is occuring.*
  
  How can I fix this? Am I forgetting something somewhere?
  [attachments] 1.png, 3.png, 4.png, 5.png
- **2023-06-09** `___swan` [🔍╎texturing-and-shading] My bad if this is the wrong channel. I'm new to extracting Halo models, is there a way to automatically assign correct textures to Reach models in blender or do I just have to guess by name?
- **2023-06-21** `guga666555` [📜╎ripping] Hey, just hopped into this server, i'm a modder making a Halo Mod for an different game [Call to Arms] and i need help extracting Halo Reach animations using bonobo, however it says reach maps are not supported once i try to open 'em, tho i remember ripping reach animations a time ago. I dont seem to understand whats happing with the application, so if anyone here does know a solution i'd be really grateful
- **2023-06-22** `cameraguy` [📜╎ripping] I'm using the newest version of Reclaimer and I can't open reflection Cubemap .bitmaps on 3 games and I can't open .shaider files like the older version
  
  Did those features get removed or am I doing something wrong?
- **2023-06-28** `yorky.` [🔍╎texturing-and-shading] Hi, does anyone have the odst coating ripped from the game with 1-1 textures? I know there was a pack someone made with a bunch of season 1-2 coatings that was super helpful but I cant seem to find any of the new coatings anywhere
- **2023-07-05** `setriaphtfk` [📜╎ripping] I’d like to rip a handful of maps from Reach (at least one in particular to make some animations) but I don’t know how, could someone point me in the direction to get started learning how?
- **2023-07-08** `chiefster_4014` [📜╎ripping] There's no way to that I know of. The best way to get models from Reach is to just use Reclaimer with the MCC files.
- **2023-07-08** `chiefster_4014` [📜╎ripping] H3+ uses multiple UVs for water. Thankfully for H3 we can extract the original level geo with the editing kit but after that you'll have to make a new UV map for it.
- **2023-07-09** `r93_sniper` [📜╎ripping] we wont get reach support from the tools unless somebody finds a way to pull it out w/ reclaimer
- **2023-07-09** `kuddlykraken` [🔍╎texturing-and-shading] idk what that means, but I just import the model ripped from the game, bake the normal maps that come with the model, find a color scheme, mess with the smart masks and then I just mask off the areas with premade blots.
- **2023-07-15** `sswampp` [💬╎general-help] to do it properly would take a lot of explanation, but if you have to do it manually and not with the CR4B tool, you want to get the shader info (for each material) from the game either with assembly or with the editing kit (guerilla) and copy it over to a blender shader/nodegroup that's set up for game accuracy
- **2023-07-29** `red_wolfe` [📜╎ripping] I'm trying to extract the final race level from halo 3 but ngl I'm not sure I'm understanding the tutorial correctly. I checked for the map in the google drive but it seems it's only the multiplayer maps in there and I'm looking for that one specific campaign map
- **2023-07-31** `gameoverload` [💬╎general-help] I don't do Reach stuff, so the best you'd need to look if it wasn't extracted already is the Reach files looked in using Reclaimer.
- **2023-07-31** `sswampp` [💬╎general-help] you download the reach maps from that link, download reclaimer, extract the maps to a folder they can all sit in, and open a map in reclaimer
- **2023-09-04** `plastered_crab` [📜╎ripping] https://github.com/77Mynameislol77/HaloAnimationRepository
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository A Repository of ripped Halo Animations for use in MCC modding. - GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of ripped Halo Animations for use in MCC m
- **2023-09-06** `trisckit` [💬╎general-help] tried Find Missing Files, still looking bugged - not sure if it's a textures missing error or if it's something with the shader/texture setup that got messed up during download/extract. grav hammer is the only one that looks normal
  will try again with fresh download/extract though
  [attachments] image.png, image.png
- **2023-09-08** `ProBot ✨` [📜╎ripping] #Halo Infinite Model and Texture Extraction
  Goes over everything you need to get it working step by step
  
  I made sure to start from absolute scratch as if nothing was installed too so you can see the full process (only takes around 5 mins of actual installing and setup. I just spent a bit of extra time to show it in action and give some extra tips on finding certain models)
  
  Find the video here: https://youtu.be/AOStYxiDLis
  
  Get CR4B Tool here: https://github.com/PlasteredCrab/Halo-CR4B-Tool
  [embed] How to rip Halo Infinite Models and Textures with CR4B Tool and HIRT https://www.youtube.com/watch?v=AOStYxiDLis This video goes over how to extract Halo Infinite models and textures (even batch extracting) using HIRT CLI and CR4B Tool.
  
  It is a step by step guide which I start from complete scratch
- **2023-09-10** `adv_eon` [📜╎ripping] Has anyone ripped the textures for Jorge's chaingun (Including normal and control maps) from Halo 5? I want to see if those textures could be repurposed for the Reach version
- **2023-09-15** `r93_sniper` [🦴╎animation] https://github.com/77Mynameislol77/HaloAnimationRepository
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository A Repository of ripped Halo Animations for use in MCC modding. - GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of ripped Halo Animations for use in MCC m
- **2023-09-23** `chiefster_4014` [💬╎general-help] ah. it was probably in the .obj format which is kinda crap. if you have it you can use Reclaimer with H3 mcc and rip it yourself in .AMF, which will preserve the mesh normals.
- **2023-09-28** `dubs7ep_panda` [📜╎ripping] Would it be possible to rip the Shadow of Intent's Bridge/meeting room from those few H3 cutscenes?
- **2023-10-01** `plastered_crab` [📜╎ripping] https://youtu.be/xhRq8ChDz3s?si=1Iiahv19MhxdeMUi
  [embed] Halo CR4B Tool v2 5 Install Guide https://www.youtube.com/watch?v=xhRq8ChDz3s Greetings Beta testers! It is time for a new install video for the recent changes I am making so I am making this to briefly go over what has changed and how to install it.
  
  Get CR4B Tool beta here: https://github.com/Plas
- **2023-10-03** `felixvyra` [🔍╎texturing-and-shading] Hey Guys.
  
  I haven't been on this server for some time as I have been doing some other stuff, and although this isn't related to Halo I have a bit of a texturing issue and I don't know a better place to ask for help.
  
  So. The last few days I have been trying to rip some assets from a game called Rainbow Six Siege. It's a long and complex process so I'm not going to go into it right now but TLDR: the guns have different skins and some of the "Lower quality *universal*" skins use a simple image that get's applied to all the guns, instead of having a different base color map for each one.
  My problem seems to be with the UV mapping. In the Images provided below you will see how the gun looks completely fine using the "no skin" base color map but If I try to apply the "Peacock" skin only SOME parts of the gun look correct.
  
  This is super weird and not something I have ever encountered before.
  I tried to look for some additional data that I might have missed but there is nothing. No extra information in the alpha or anything in the ripped files.
  
  Any help would be appreciated.
- **2023-10-06** `Deleted User` [💬╎general-help] Has anyone worked with the Halo Reach Forge World rip?
  i grabbed it from the MEGA.nz folder, and ive been trying to repair it for use in a Unity Based Social VR game, i did the sky on my own, its not exactly the same but its close, however, i do not know how ill make the terrain as its using textures to blend multiple textures together
- **2023-10-18** `dgenpoly` [📜╎ripping] I'm sorry if this doesn't belong here, is there a tool like (halo2-color-plate-extractor) for HCE and H3?
- **2023-10-18** `dgenpoly` [📜╎ripping] Okay, what's the name of the CE tool, and what's the best workflow to batch extract H3 textures so mapping isn't 100% blind?
- **2023-10-28** `ProBot ✨` [📜╎ripping] #Halo Infinite Model and Texture Extraction
  Goes over everything you need to get it working step by step
  
  I made sure to start from absolute scratch as if nothing was installed too so you can see the full process (only takes around 5 mins of actual installing and setup. I just spent a bit of extra time to show it in action and give some extra tips on finding certain models)
  
  Find the video here: https://youtu.be/AOStYxiDLis
  
  Get CR4B Tool here: https://github.com/PlasteredCrab/Halo-CR4B-Tool
  [embed] How to rip Halo Infinite Models and Textures with CR4B Tool and HIRT https://www.youtube.com/watch?v=AOStYxiDLis This video goes over how to extract Halo Infinite models and textures (even batch extracting) using HIRT CLI and CR4B Tool.
  
  It is a step by step guide which I start from complete scratch
- **2023-10-30** `thestranger8973` [💬╎general-help] hey i was curious if i could get some help and tips with a project im starting
  so im a big fan of war games (table top ones) and as ive been playing halo wars im working on making  a custom game and was wondering if i could gets some tips on modeling or maybe extracting files from games
  mainly looking at trying to either rescuplt marines, odst ect but thought id ask if anyones got tips
- **2023-11-11** `ProBot ✨` [📜╎ripping] #Halo Infinite Model and Texture Extraction
  Goes over everything you need to get it working step by step
  
  I made sure to start from absolute scratch as if nothing was installed too so you can see the full process (only takes around 5 mins of actual installing and setup. I just spent a bit of extra time to show it in action and give some extra tips on finding certain models)
  
  Find the video here: https://youtu.be/AOStYxiDLis
  
  Get CR4B Tool here: https://github.com/PlasteredCrab/Halo-CR4B-Tool
  [embed] How to rip Halo Infinite Models and Textures with CR4B Tool and HIRT https://www.youtube.com/watch?v=AOStYxiDLis This video goes over how to extract Halo Infinite models and textures (even batch extracting) using HIRT CLI and CR4B Tool.
  
  It is a step by step guide which I start from complete scratch
- **2023-11-12** `dgenpoly` [💬╎general-help] Hello everyone, is there a tool I can use to batch extract bitmaps from HCE and H3? I've been figuring out mapping and H2 has the (halo2-color-plate-extractor) tool and it's made making a map for H2 so much easier. Is there a tool or even a workflow to batch extract bitmaps to a usable format?
- **2023-11-16** `mozria` [💬╎general-help] I'm new here. Where would I find a guide on how to properly install + configure Reclaimer, Architect and C4AB Tool to extract and work on Reach map bsps in Blender?
  I tried following the video from Plastered Crab for how to get his tool working, but I've run into a lot of problems (largely in that I flat-out cannot get Reclaimer to funtion).
  I also don't have an "Append Nodes" button after setting up the tool in Blender :/
- **2023-11-17** `plastered_crab` [💬╎general-help] Reclaimer is more so for auto-setting up materials for the games (which it only does Halo 3 and ODST so far) but it also uses special builds of Reclaimer and HIRT and IE to take in command line arguments
- **2023-11-17** `plastered_crab` [💬╎general-help] everything for extracting models and stuff for Reach can be done soley with Reclaimer itself 🙂
- **2023-11-18** `tacohombre` [📜╎ripping] For 5, most maps are able to be extracted with probably a quarter having been compressed with dark magic so they're currently unable to be opened and we can get the working maps with about all data as the other games except for vertex colour and multiple UV channels due to issues with the AMF format and extracting in other formats
- **2023-11-30** `terminator33442` [📜╎ripping] How does the H2A model/texture quality fair between the campaign and the multiplayer? Is it worth ripping the campaign weapons?
- **2023-11-30** `terminator33442` [📜╎ripping] Like, is it worth ripping the campaign models in place of the multiplayer ones (i.e. less gross modeling/normal issues)
- **2023-12-06** `unscfighter` [📜╎ripping] Anyone know how i can rip a scenerio file from h3 odst and convert it to something i can put in blender?
- **2023-12-11** `Deleted User` [💬╎general-help] using reclaimer rn and trying to open shaders on construct.map (h3) but to no avail.... do i need to extract the bink, images, info from their respective folders or should reclaimer be able to access them if they are within the .map's folder
- **2023-12-17** `nihren` [🔍╎texturing-and-shading] Figure I'll try asking in here. Need some help with textures. I'm trying to upscale textures, but if I upscale the Color & Control textures as is it doesn't always work. Worked fine with the HI Grunt, however it's completely breaking with the H5 Grunt. Tried extracting the color channels, upscaling them individually, and then recombining them via nodes in Blender, and it gets close, but not quite there.
- **2023-12-17** `nihren` [🔍╎texturing-and-shading] Result if I try to recombine the extracted channels with a Combine Color node.
  [attachments] image.png
- **2023-12-17** `nihren` [🔍╎texturing-and-shading] I try not to touch the normal map, even for upscaling. And any time I've tried it breaks, so I leave it alone. Some models I've worked with recently had the texture detail, like scratches and all that, under a mask or control, but that doesn't seem to be the case here so I'm focusing on the Color texture. Unfortunately, feels like an effort in futility. I tried the Halo 4 Grunt, which is set up similarly, and what solved the problem there was I simply used just one of the extracted channels, in this case the Red channel, and then used the original texture as the alpha, and it worked perfectly. Tried it on the H5 Grunt, and it didn't work.
- **2024-01-02** `amsierra` [🔍╎texturing-and-shading] Does anyone happen to know where exactly i can pull texture scale data from in assembly for each texture in a material? For example im doing one of the terrain_blend materials for a halo reach map and instead of going by eye like i used to do for the tiling on each texture i was wondering if i can get the game accurate scaling somewhere in the shader data for each texture using assembly
  [attachments] forgemaphalo.png
- **2024-01-02** `mikuspartan` [🔍╎texturing-and-shading] Does anyone have the Halo Reach Jetpack Textures, for the last three times the textures I have ripped have been corrupted or completely un-readable 🥺
- **2024-01-20** `theishikawarin` [📜╎ripping] this branch of Reclaimer keeps crashing when I try to export the Halo 5 Multiplayer Spartans as AMF and I can't open RMF in Blender... pain
- **2024-01-21** `theishikawarin` [🔍╎texturing-and-shading] @TacoHombre no luck extracting the Halo 5 multiplayer spartans, extracting as AMF either crashes Reclaimer or only comes out as 445kb, RMF is fine though for some reason
- **2024-01-21** `theishikawarin` [📜╎ripping] I've always found it so weird how Halo 5's control textures' colour masks are always wrong when extracted
- **2024-01-22** `theishikawarin` [📜╎ripping] I did try to open the Dedicated Server's modules with the ModuleExtractor but it just errored out
- **2024-01-29** `motulaeternus` [💬╎general-help] been looking to recreate the brushtread disruptive coating, not asking for the coating json itself since i know it'd be difficult to extract given its season 3
  
  so with that said; does anyone have an ODST camo texture for the upper part of the greaves?
- **2024-02-18** `frostbitecinematics` [📜╎ripping] I'm probs overthinking this ofc but figured I'd check juuuuuuust in case; can reclaimer be used to rip assets from games like Halo Reach MCC if you have it on Steam, or does it require buying it on a specific store or whatever?
- **2024-02-21** `gameoverload` [🔍╎texturing-and-shading] Yes, I don't know where that shader is stored unfortunately since Urium never hooked in and dumped anything that had character skin associated with it.
- **2024-02-21** `xansworkshop.com` [🔍╎texturing-and-shading] Actually, if that decompiled shader hlsl source is available, using that would help a lot because then I can clean it up too
- **2024-02-22** `xansworkshop.com` [🔍╎texturing-and-shading] Ah, so it will be impossible to fully use this shader. The data size is so complex that even on the most modern available shader target (*to unity) it exceeds max register size with more than 4 zones active at once. rip
- **2024-02-22** `atlasguitar` [💬╎general-help] Can anyone help/point me in the right direction to properly understand basically how to port a model into H3? 
  
  I'm very new, so I'm finding it a bit difficult to find resources that relate to exactly what I want to do. Lots of MP based stuff, but I mostly want to mess with Campaign, so those kinds of things are harder to find than your run of the mill "here's how to make a weapon do X" kinds of tutorials.
  
  Specifically, I'm trying to wrap my head around porting H2A models to H3 (and presumably, the process for actually porting things into H3 in a workable state will be similar for different sources?). 
  
  I've extracted files using Index, everything's good in Blender, but I'm iffy after that. For example, I was able to create a new weapon for H3 that's the H2A Battle Rifle, and the model ports and loads in Sapien, but I can't get first person to work (telling me a matching node can't be found, which sounds to me like I need to have different naming conventions in the skeleton (if that's what it's called for a weapon) to match H3's BR model naming conventions?) and I'm having a particularly difficult time finding resources regarding bitmaps and shaders, so I can't really get the model textured. 
  
  Another thing I was trying was with the H2A Chief model, and the problem I run into there is a "no animation graph" error and I can't seem to find much info about how to port new models to use existing animations.
- **2024-02-22** `chiefster_4014` [💬╎general-help] .render_model is the tag used to store model data in Halo, and for H3 you can export that data with Tool with the ``tool extract-import-info`` command. There's more info about it on the C20 site. For most things it'll extract a .ass file, which you can then import into Blender or whatever you're using.
- **2024-02-22** `pendragone01` [💬╎general-help] Hello everyone! I stumble upon this discord while looking for some specific answers about modding Halo, and I'm stoked to be here! Right now I'm trying, in Blender, to change the color of certain shaders on the marine I've ripped from Halo Reach, and I could really use some help. If y'all could point me towards any specific tutorials or documentation on it, I'd really appreciate it.
- **2024-02-22** `frostbitecinematics` [📜╎ripping] yeah i'll have to look into how to set it all up then, so far I've only been ripping Halo Reach assets but very much interested in making a public Infinite pack for UE5
- **2024-02-22** `ProBot ✨` [📜╎ripping] #Halo Infinite Model and Texture Extraction
  Goes over everything you need to get it working step by step
  
  I made sure to start from absolute scratch as if nothing was installed too so you can see the full process (only takes around 5 mins of actual installing and setup. I just spent a bit of extra time to show it in action and give some extra tips on finding certain models)
  
  Find the video here: https://youtu.be/AOStYxiDLis
  
  Get CR4B Tool here: https://github.com/PlasteredCrab/Halo-CR4B-Tool
  [embed] How to rip Halo Infinite Models and Textures with CR4B Tool and HIRT https://www.youtube.com/watch?v=AOStYxiDLis This video goes over how to extract Halo Infinite models and textures (even batch extracting) using HIRT CLI and CR4B Tool.
  
  It is a step by step guide which I start from complete scratch
- **2024-02-23** `pendragone01` [💬╎general-help] Hey all, does anyone know if it's possible to change the permutations available in Halo Reach's Foundation? I'd like to put a Spartan Helmet on a marine, but I'm wondering if that's possible without ripping it out and creating an entirely new tag.
- **2024-02-24** `gravemind2401` [Halo 5 Research] also thatll only be on the 360 maps, reclaimer doesnt have water vertices on mcc reach maps
- **2024-02-25** `gravemind2401` [Halo 5 Research] reclaimer doesnt have vertex colors mapped for any of the h5 vertex formats. also in previous games some formats used the position W as color R instead of having a separate color field and idk if theres anything in the tag data that indicates if the position W is color or not
- **2024-03-03** `frostbitecinematics` [📜╎ripping] Anybody know if it's possible to find the exact scale of certain characters using a tool, e.g maybe reclaimer?
  
  E.g in my case, I know that Halo: Reach's Spartans use the same skeleton as regular humans but are scaled by 1.2x. Jorge however is even bigger than the rest of Noble Team. How would I find his exact rig scale value, if possible?
- **2024-03-06** `atlasguitar` [💬╎general-help] Hey so I've been using Index to rip H2A assets and port them to H3, but I get this trying to open the sniper rifle to export it. Anyone able to help - either get it working, or with another H2A sniper source?
  [attachments] image.png
- **2024-03-16** `castscarab9` [📜╎ripping] is it possible for someone to extract the shield doors from halo reach forge? 
  the two way and one way shields?
- **2024-03-20** `zadeinator` [📜╎ripping] you can extract them with the mod tools and/or halo 2 color plate extractor
- **2024-03-20** `zadeinator` [📜╎ripping] Can be found here
  
  https://github.com/SnowyMouse/halo2-color-plate-extractor
  [embed] GitHub - SnowyMouse/halo2-color-plate-extractor: Halo 2 tag color p... https://github.com/SnowyMouse/halo2-color-plate-extractor Halo 2 tag color plate extractor. Contribute to SnowyMouse/halo2-color-plate-extractor development by creating an account on GitHub.
- **2024-03-26** `zadeinator` [📜╎ripping] ```halo2-color-plate-extractor "G:\Steam Games 4 quickboy edition\steamapps\common\H2EK\tags" "G:\Steam Games 4 quickboy edition\steamapps\common\H2EK\data" "G:\Steam Games 4 quickboy edition\steamapps\common\H2EK\tags\scenarios\objects\covenant\military\capital_ship\bitmaps\capital_ship_a.bitmap"```
- **2024-03-26** `zadeinator` [📜╎ripping] Oh nah you don't need that, the base colour plate extractor should be all you need
- **2024-03-28** `unscfighter` [📜╎ripping] Hey so i downloaded reclaimer and im trying to export the buildings from m50 and m52 from halo reach to blender but i dont see them anywhere in there
- **2024-04-10** `powerhousegaming` [📜╎ripping] how do I rip files from CEA?  iirc Reclaimer only does 3-Reach and Index is for 2a
- **2024-04-29** `mechacommando` [💬╎general-help] This is a server dedicated to ripping Halo models. Just search “spartan pack” in #📂╎asset-upload-download-forum . I’m busy and I can’t get them for you right now.
- **2024-05-08** `dg5312` [💬╎general-help] @Gravemind2401 
  Hi
  A friend of mine is using Reclaimer for open a custom H3 ODST map but he cant, their outpout show this.
  Is there something is doing wrong or something else we can do?
  [attachments] image-4.png
- **2024-05-11** `profz3d` [📜╎ripping] i have trouble trying to rip halo reach animation
  when i try to import the .jma animation file i got this error
  
  anyone know how to fix this? or what i do wrong?
  [attachments] image.png
- **2024-05-17** `zepumpkineater` [💬╎general-help] https://docs.google.com/spreadsheets/d/1N860qHHlzj7zL2FHCnz4VcGCRSGOxnxa63pKZDwf3Sc/edit#gid=0
  
  Control + F for "Halo" here, you'll find a big mega file with a bunch of assets from Reach
  [embed] Adams Archive (and other) links https://docs.google.com/spreadsheets/d/1N860qHHlzj7zL2FHCnz4VcGCRSGOxnxa63pKZDwf3Sc/edit Information
  
  Models, Textures, Audio, Anims,Sites, Software, Unity, 3D printing, Etc,File list,Extraction guides
  Rip slot 1: Sakuna: Of Rice and Rui...
- **2024-05-25** `varokist` [💬╎general-help] Hey im back!
  
  I was trying to extract the raw Gravity Hammer texture from the reach weapons pack, but this is what i got instead.
  
  It's transperent and im trying to get this in the actual color and not transperency.
  [attachments] gravity_hammer.png
- **2024-05-31** `soma4069` [📜╎ripping] Has anyone ripped the animations for completing a match/leveling up in the original reach? 
  
  So far I was only able to find clips for inheritor and the warrant officer promotion animations so far
  [attachments] RPReplay_Final1717153998.mov, RPReplay_Final1717206687.mov
- **2024-06-02** `gregory0701` [📜╎ripping] How does one extract sounds from reclaimer from MCC Reach? I have the sound extractor plugin installed but clicking extract on the tag seems to do nothing
- **2024-06-03** `zadeinator` [🔍╎texturing-and-shading] You can also extract the source textures with the halo 2 color plate extractor tool (i assume reclaimer only extracts post-processed textures)
- **2024-06-04** `zadeinator` [🔍╎texturing-and-shading] https://github.com/SnowyMouse/halo2-color-plate-extractor
  
  This is the texture tool in question btw @Nero 
  
  Should have instructions on how to use it there, but if not lmk
  [embed] GitHub - SnowyMouse/halo2-color-plate-extractor: Halo 2 tag color p... https://github.com/SnowyMouse/halo2-color-plate-extractor Halo 2 tag color plate extractor. Contribute to SnowyMouse/halo2-color-plate-extractor development by creating an account on GitHub.
- **2024-06-04** `powerhousegaming` [💬╎general-help] is there something I can use to rip CE classic models, I forgot that 343 just repurposed Reach and a few other models, so they aren't actually accurate
- **2024-06-08** `zadeinator` [💬╎general-help] if you can find the .bitmap in the h3ek then yes you can just extract that
- **2024-06-16** `cythecy_` [🔍╎texturing-and-shading] Just got here after the server was suggested by NotPipi.
  
  I've been successfully pulling models through Reclaimer and it brings along the TIFs for the different image maps. I'm trying to port them into Blender using the AMF importer, successful, but I'm having issues figuring out how to properly set up the shader graphs for various models. CE seems relatively straight forward but later games have different detail maps and such and I just can't figure out how to get it done correctly. I did Boneyard from Reach and the ground looked completely wrong just mixing the different maps together with the correct scaling on each.
- **2024-06-28** `zadeinator` [🔍╎texturing-and-shading] You can either extract it, change the shader names in blender and reimport, or you can enable expert mode and edit the shader names on the model directly
- **2024-07-01** `nobletitan` [📜╎ripping] Has anyone been able to rip the campaign and multiplayer maps of Halo Wars and Halo Wars 2? I have downloaded all the files from the archive but I cannot seem to locate any meshes for maps, I can find bases, vehicles, weapons, even character models, but I cannot find meshes for maps. Are the meshes within the files or am I crazy?
- **2024-07-09** `juliociraptor` [💬╎general-help] Hello everyone! I'm trying to import animations from the Jackal in Halo Reach into Blender, but I can't. Everything works fine with the Jackal from Halo 3, and I followed the tutorial here on the Discord server and in the YouTube video: 'https://www.youtube.com/watch?v=oIIHsOifz60&t=151s'."
  [embed] Halo Animation Ripping / Porting [Blender] https://www.youtube.com/watch?v=oIIHsOifz60 This video goes in-depth with the process of extracting Halo animations and porting them into Blender. I provide all the useful tools you need in order to do this as well as a .blend file with my Tactical Moa for 
- **2024-07-10** `juliociraptor` [💬╎general-help] "Well, actually I'm creating a player model for GMod, with custom animations from the Halo games. I've managed to import animations into Blender following the tutorial step-by-step. The steps I followed were as follows: I downloaded the Reclaimer and Bonobo programs, and downloaded the ".map" map files to extract models and animations. So far, so good. After obtaining the "amf" model of the jackal and the animations in "JMA," I use a Blender addon to import and export files from Halo games. Here's where my problem with the model and animations from Halo Reach arises. While following these steps, I successfully imported the model and animations of the Halo 3 Jackal. However, when I try to import the model and animations of the Jackal from Halo Reach, Blender shows an error message: "Internal error: <class 'ValueError'>(could not convert string to float: '0,9999999')."
- **2024-07-16** `cythecy_` [💬╎general-help] (realised I posted in the wrong chat)
  Bit of a rookie with Blender. Does anyone know how to deal with these black patches?
  Models are ripped straight from Halo 3/ODST, shaders are copies of what's available in the asset packs but I'm trying to move everything over to a single Blender file.
  [attachments] image.png, image.png, image.png
- **2024-07-23** `pikacommando` [💬╎general-help] I see that it's made up of 3 models called slipspace_sphere, slipspace_sphere_outer, and slipspace_sphere_centralized when I opened the odst intro c100.map in Reclaimer
- **2024-07-28** `autarchkatherlyn` [🔍╎texturing-and-shading] Side note: if you have h3ek, you can just extract the model that way too
- **2024-07-28** `spartanjoe193` [💬╎general-help] Though the articles for the Halo Reach tag groups are empty since the modding community doesn't have the same level of experience with them, same with the other MCC games except for Halo 1 because the community has so much experience modding it you can extract tags from maps and build maps for all versions of the game including the original Xbox version. .
- **2024-08-03** `xenomorph_xx121` [📜╎ripping] Is there a way I can rip the halo infinite animation files? I'm looking specifically for the elite animations
- **2024-08-06** `r93_sniper` [Index] Tool created by @Haus to extract content from Halo CEA and H2A Campaign.
  
  V1 (Only H2A Support): https://github.com/Wildenhaus/Index
  V2 (CEA and H2A): https://github.com/Wildenhaus/IndexV2
  [embed] GitHub - Wildenhaus/Index: A tool for browsing and extracting asset... https://github.com/Wildenhaus/Index A tool for browsing and extracting assets from Halo 2 Anniversary. - Wildenhaus/Index GitHub - Wildenhaus/IndexV2: A tool for browsing and extracting ass... https://github.com/Wildenhaus/IndexV
- **2024-08-06** `r93_sniper` [Reclaimer] Tool created by @Gravemind2401 to extract models, textures, and sounds from various halo games. Read the github link for more information on what is and isnt supported, as well as #📰╎reclaimer-updates for any pertinent news.
  
  Program: https://github.com/Gravemind2401/Reclaimer
  Addon Downloads, including AMF importer script: https://www.mediafire.com/folder/dabvr2j5ofe73/Reclaimer_Downloads
  [embed] GitHub - Gravemind2401/Reclaimer: A Halo game file explorer https://github.com/Gravemind2401/Reclaimer A Halo game file explorer. Contribute to Gravemind2401/Reclaimer development by creating an account on GitHub. MediaFire https://www.mediafire.com/folder/dabvr2j5ofe73/Reclaimer_Downloads MediaFire
- **2024-08-06** `r93_sniper` [Tag Tool] Tool designed to modify tags from built caches.
  https://github.com/TheGuardians/TagTool/tree/dev
  [embed] GitHub - TheGuardians/TagTool at dev https://github.com/TheGuardians/TagTool/tree/dev Contribute to TheGuardians/TagTool development by creating an account on GitHub.
- **2024-08-06** `r93_sniper` [Assembly] Tool used in the research of different halo title formats
  https://github.com/XboxChaos/Assembly
  [embed] GitHub - XboxChaos/Assembly: Multi-Generation Blam Engine Research ... https://github.com/XboxChaos/Assembly Multi-Generation Blam Engine Research Tool. Contribute to XboxChaos/Assembly development by creating an account on GitHub.
- **2024-08-06** `r93_sniper` [BMAX] Free connector app to transfer content between 3DS Max and Blender
  https://titus.gumroad.com/l/B2MAX
  [embed] Blender<->3DSMax Connector [BMAX] https://titus.gumroad.com/l/B2MAX Hello, Blender <> 3DSMAX connector(BMAX) is a two small scripts for these applications.Workflow is simple - just send and get models through FBX format.License:GNU General Public License.OS:Windows onlyInstallation:Extract "_BMAX_xx
- **2024-08-07** `blackandfan` [PhxTools] Tool used to extract and repack HW1 eras (Alpha, 360 and DE), also able to convert XMB(Alpha,360,DE and HW2) back into a readable XML format.
  https://github.com/KornnerStudios/HaloWarsDocs/releases
  [embed] Releases · HaloMods/HaloWarsDocs https://github.com/KornnerStudios/HaloWarsDocs/releases Documentation related to modding Halo Wars. Contribute to HaloMods/HaloWarsDocs development by creating an account on GitHub.
- **2024-08-11** `walkmen` [📜╎ripping] Is there a way to rip the exact armor colors used by Halo 1, 2, 3, and reach? I'm trying to look at it through IDA, but I'm not getting anywhere
- **2024-08-14** `gravemind2401` [Reclaimer] https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.0.0-beta
  Beta release of Reclaimer v2, **read the upgrade notes before installing**.
  I'm looking for feedback on any potential issues with the RMF importer for both Blender and 3DS Max users.
  [embed] Release v2.0.0-beta · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.0.0-beta Important notes on upgrading from previous versions:
  
  This version requires .NET Desktop Runtime 7 to be installed which you can get from here under the ".NET Desktop Runtime 7.0" sectio
- **2024-09-18** `vor4dorsraven` [💬╎general-help] I can't seem to open up the campaign map for Reach on reclaimer, is there another tool i need?
- **2024-09-19** `lexy.shirosaki` [💬╎general-help] Hi guys I want to open a Halo Online map for Halo3 that I found on the steam workshop with the Reclaimer tool but it said that it doesn´t accept the file or that is not valid, Is there any other way to open the file?
  
  I'm talking about this map
- **2024-09-27** `chiefster_4014` [📜╎ripping] Index was made for H2A campaign's assets as they use a different graphics engine than the multiplayer. you need to use Reclaimer to rip the multiplayer maps.
- **2024-09-30** `frankr2994` [📜╎ripping] Couple questions, do we have any sort of resources that say what some of these different texture maps are? One of the things that I couldn't figure out last night what was what a macro color map was. 
  
  Second question was what all plugins are available for reclaimer? I can only find mentione of architect and nothing else.
  
  Oh and I guess I do have a third question. How are we reading the shader code? Or is that all part of your development process and a mile above my pay grade?
- **2024-09-30** `chiefster_4014` [📜╎ripping] 1. There's #📑╎control-map-guide, though it only describes the main texture types. For the ones that aren't known you can figure them out by looking through the shader code (if it's available) or experimenting.
  
  2. I'm not sure, though I think that's the only one that was made. If you're just ripping from the games then Reclaimer should come with all you need.
  
  3. Shader code can be taken from MCC's mod tools (CE, H2, H3/ODST, HR, H4, H2AMP) or can be extracted from the game (CEA, H2ASP). For H5, Infinite and any others, it's currently not possible to get the shader code.
- **2024-10-01** `coarforge` [📜╎ripping] It's possible to also get shader code for H5 and infinite, but only bytecode, which for H5 can be decompiled with 3DMigoto, and for infinite using first dxil-spirv to convert it into spirv and then spirv-cross to decompile it further (though this is a kinda lossy process, using dxc you can keep more debug information in the disassembly)
- **2024-10-01** `frankr2994` [📜╎ripping] Briefly looking I'm surprised at the amount of information and tools available for shader disassembly . 
  I really do need to stop going down rabbit holes though. I'm supposed to be setting guns up in unreal for a game mod that I already have ported in 😅. 
  
  At some point I will be more curious of the processes used in the various model ripping programs that we have for halo. I am noticing that with h2a and h4 any material using multiple textures or having a texture coordinate map will not pull over into blender. It would be truly awesome to get something sorted out to automatically pull some of the more advanced materials in at atleast a basic layout to aid in transfer to unreal for me. My material building knowledge is low so having a reference in blender is basically a requirement.
- **2024-10-09** `_themadmonk` [📜╎ripping] Hey does anyone have the new mombasa "open world" level from odst ripped?
- **2024-10-25** `spoonsor` [💬╎general-help] Where would I be able to find rips of medals?  I am looking for H3 and Infinite
- **2024-10-26** `firedolphin` [💬╎general-help] Does anyone have extracted sound files of all the Halo Reach marine voice lines?
- **2024-10-30** `thespeedycube` [📜╎ripping] does any one know if its possible to rip the assignation animations from say like halo reach?
- **2024-11-01** `duhxroach1` [💬╎general-help] Does anybody know what I’m doing wrong?
  
  I’m new to all this blender and modding stuff. But what I wanted to do is I guess extract the halo reach marine render model into jms file to import into blender.
  [attachments] IMG_5362.jpg
- **2024-11-01** `chiefster_4014` [💬╎general-help] .JMS files can't be extracted from Reach, H4, or H2AMP as the import pipeline changed. If you want the models you can either use Reclaimer with compiled .map files or the Foundry addon for Blender if you're working with tags.
- **2024-11-01** `chiefster_4014` [💬╎general-help] Either download Reach on MCC PC or https://haloce3.com/xmf/ for the 360 .map files. For those you'll need mainmenu.map, shared.map and campaign.map to extract anything from the other .map files.
- **2024-11-02** `brooen` [📜╎ripping] @Gravemind2401 whats the extent of the new reclaimer vertex color support?
- **2024-11-04** `cythecy_` [📜╎ripping] I'm using Reclaimer to look through the Reach assets.
  Elites only have one head type (different helmets merged with the armours) and I can't tell if the Spec-Ops is supposed to be using the head or not, because the mandibles stick out if you use it and there's a huge gap if you don't.
- **2024-11-05** `krazyzark` [💬╎general-help] is there are reason why reclaimer is exporting halo reach maps in .rmf when it should be .amf
- **2024-11-18** `.r.a.m` [🦴╎animation] Does anyone know how to extract the h3 flood enemy animations from mcc?
- **2024-12-23** `viprewolf` [🔍╎texturing-and-shading] Anyone have experience with the Snowdrop Engine shaders? I'm working with rips from Frontiers of Pandora, and I can't figure out their shader.
- **2024-12-26** `viprewolf` [💬╎general-help] Alright, big question: How do I go about creating a shader for game rips like HIMS? I've ripped a bunch of assets from Frontiers of Pandora, but the shaders are kind of wack, especially on the wildlife. I have more data files than I'll ever need for each individual model and shader, and I'm sure something can be done with them, I just have no idea *what.*
- **2025-01-06** `solipsol_studios` [💬╎general-help] When it comes to making art with the assets from this server - what isnt allowed besides the obvious nsfw stuff?
  
  If I credit where all the assets and stuff came from in an art piece is that enough?  Ive never made stuff with things people have ripped before, just wanting to know what the rules are.
  
  (For context i just wanted to make a fan art of a halo streamer sitting in the drop pod bay like its the odst menu)
- **2025-01-23** `aurathedutchangeldragon` [💬╎general-help] okay i have a question
  i had this cool idea to make themed spartans with my friends for vrc and we went through in mcc h3 customisation to write down what parts we wanted.
  my actual question is, how much of the halo 3 armor assets have been ripped?
- **2025-01-26** `muskah.mp4` [💬╎general-help] If I'm just working on H3 character models, I don't need to download the "ReclaimerDebug_h5stuff.zip" do I? All of the other files show up as clean but this one H5 Debug is flagged with a "Trojan.Malware.300983.susgen" (assuming most likely false positive) but I don't play H5 or make H5 vids, so I don't wanna risk it if I don't have too lol
- **2025-01-26** `gameoverload` [💬╎general-help] Use this:
  https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.0.1536
  [embed] Release v2.0.1536 · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.0.1536 RMF exports will now include a color channel for h4 and h5 meshes that use position W
  
  Affected models will need to be re-exported
  Version 1.0.1 of the import script is required in order to 
- **2025-02-09** `chiefster_4014` [🔍╎texturing-and-shading] There's info on what the channels do on C20: https://c20.reclaimers.net/h1/tags/shader/shader_model/
  [embed] shader_model https://c20.reclaimers.net/h1/tags/shader/shader_model/ The shader_model tag is used for opaque materials on object models. It supports features like map an...
- **2025-02-10** `chiefster_4014` [📜╎ripping] most things should have at least some color. what are you trying to rip?
- **2025-02-10** `chiefster_4014` [📜╎ripping] are there any errors in the Reclaimer window? and what filetype are you exporting as? i've exported the reach spartan just fine.
- **2025-02-16** `theishikawarin` [📜╎ripping] I'm just glad I'll never have to deal with the days where I had to manually extract every bump map and cubemap again 😭
- **2025-02-22** `frankr2994` [📜╎ripping] so when I try and open a reach workshop map in reclaimer scenario opens blank nothinginess. scenario structure bsp gives me this error Loading model: levels\multi\forge_halo\forge_halo.scenario_structure_bsp
  Error loading model: levels\multi\forge_halo\forge_halo.scenario_structure_bsp
  System.ArgumentOutOfRangeException: Non-negative number required. (Parameter 'value')
     at System.IO.FileStream.set_Position(Int64 value)
     at Reclaimer.IO.EndianReader.Seek(Int64 offset, SeekOrigin origin) in C:\Users\fastc\source\repos\reclaimer2\Reclaimer.Core\IO\EndianReader.cs:line 677
     at Reclaimer.Blam.MccHaloReach.IndexItem.<ReadMetadata>g__ReadMetadataInternal|29_0[T]() in C:\Users\fastc\source\repos\reclaimer2\Reclaimer.Blam\Blam\MccHaloReach\CacheFile.cs:line 417
     at System.Lazy`1.ViaFactory(LazyThreadSafetyMode mode)
  --- End of stack trace from previous location ---
     at System.Lazy`1.CreateValue()
     at Reclaimer.Blam.MccHaloReach.IndexItem.ReadMetadata[T]() in C:\Users\fastc\source\repos\reclaimer2\Reclaimer.Blam\Blam\MccHaloReach\CacheFile.cs:line 395
     at Reclaimer.Blam.HaloReach.ResourceIdentifier.ReadData(PageType mode, Int32 maxLength) in C:\Users\fastc\source\repos\reclaimer2\Reclaimer.Blam\Blam\HaloReach\ResourceIdentifier.cs:line 42
     at Reclaimer.Blam.HaloReach.ResourceIdentifier.ReadData(PageType mode) in C:\Users\fastc\source\repos\reclaimer2\Reclaimer.Blam\Blam\HaloReach\ResourceIdentifier.cs:line 37
     at Reclaimer.Blam.HaloReach.HaloReachCommon.GetMeshes(Hal ...[cut]
- **2025-03-01** `shuttl_` [📜╎ripping] trying to install foundry to rip halo reach maps, doing everything to a T, but i get this error. ive looked in my python install and there is a numpy there but maybe im doing something wrong, this is my first time trying something like this. any help appreciated
  [attachments] image.png
- **2025-03-18** `warmachine417` [📜╎ripping] hey can anyone go and rip this satellite dish from reach for me? I've gone through reclaimer myself and got nothing
  [attachments] 20250316060935_1.png
- **2025-03-19** `zad_47` [💬╎general-help] I’m trying to extract the reach brute chieftain with reclaimer and I keep getting errors
- **2025-03-20** `codeman` [📜╎ripping] H3 is exactly what I am looking forward to extracting, so perfect! Thanks!
- **2025-03-22** `scalemaster117` [📜╎ripping] If you figure out ho to get the Campaign objects to extract, please let me know. 🙂 I had the checkbox checked in Ekur but so far no Campaign objects to be found. Only Multiplayer and Forge. Not sure what I did wrong.
- **2025-03-29** `solacianallie` [📜╎ripping] Does anyone know where i can grab a more complete set of files from reach than just using reclaimer on map files?
  I want to get stuff like the weapon schematics and things, but theres so much missing, and ive pulled all the map files using reclaimer
- **2025-04-01** `scalemaster117` [📜╎ripping] I'm beginning to figure out what objects from Halo Infinite can be identified with Ekur. The extraction gives random names to all model files and I've been loading each and trying to identify them as best I can to find what I'm after. 
  
  There's at least about 12,000 models at a rough count, maybe more. And some duplication. This is my named extraction so far. It's the first 2,195 objects I've look at so far from areas of Campaign, Multiplayer, and Forge. I have several 'unknown' in there that may change name if I can identify more accurately later. 
  
  While the naming means something to me as far as finding things, I thought that it may help some others along if they are doing the same sort of activity looking for specific Infinite models.
  [attachments] HI_Camp_MP_Frg_Ekur_Filenames1.txt
- **2025-04-01** `chiefster_4014` [📜╎ripping] for CE, you can use [General's Halo Toolset](https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master) to import the .gbxmodel tags from the mod tools directly into Blender
  For H2-ODST there are commands you can use with Tool to extract .jms files to import into Blender with the same addon. H2 has separate commands for render models, physics, collision and structures, while H3 & ODST you can just use ``extract-import-info``.
  For Reach, H4, and H2AMP, you can use [Crisp's Foundry addon](https://github.com/ILoveAGoodCrisp/Foundry) to import most tags directly into Blender, similarly to CE.
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account
- **2025-04-07** `alexvalkryie` [📜╎ripping] Anyone familiar with a method of ripping model and texture assets from Elite Dangerous?
- **2025-04-13** `scalemaster117` [📜╎ripping] Interesting find. I don't recall it on Streets but I haven't fine-tooth-combed that map myself yet. Just the usual screenshots and theater viewing. In Ekur at least, the Infinite multiplayer extract of bitmap assets numbers 17,686 files all randomly named with no folder structure to narrow the search to either civilian or even Streets. Needle in a haystack to say the least. There may be other apps that can do better that I'm not aware of.
- **2025-04-17** `frankr2994` [📜╎ripping] I will tell you that I used a program called actorcore autorig for rigging and weight painting. I did a Spartan and an elite in it. Spartan required zero corrections to be seemingly perfect (or good enough for a game) and I had to play with the elite slightly. Mainly because the model rips with the elite sort of hunched over and it doesn't like the arch in the back when automatically doing the rig.
- **2025-04-18** `theishikawarin` [📜╎ripping] Halo 3 ODST always had the most confusing mission filenames so Reclaimer's map browser is coming in clutch hard
  [attachments] 7ec4873068574caef457ab3d2fab7745.jpg
- **2025-05-11** `x_skel07` [📜╎ripping] I imported the Halo 2 Anniversary multiplayer model AMF generated from Reclaimer, and the male bodysuits have distorted deforms.
  Is there any way to fix this, or another way to extract it?
  [attachments] image.png
- **2025-05-16** `gameoverload` [🔍╎texturing-and-shading] I am going based on the tool rips for now, the problem is they do a bunch of little stuff internally per what tjoener has found that it is honestly not practically worth it to implement every single little thing they do per asset, not so much shader stuffs, but like the per texture adjustments they make.
- **2025-06-12** `chiefster_4014` [💬╎general-help] The official mod tools would have all of the shader info plus shader code for CE - H2AMP
  For H5 you'll have to use Reclaimer to see the details
- **2025-06-18** `ohwhen6442` [💬╎general-help] anyone else got a sound issue where they can't rebuild the fmod sound banks. I did it ince before and it worked, but I've added more sounds and now when I build the bank it doesn't appear anywhere. I'm using this command from the reclaimer modding site "tool export-fmod-banks "reports\reports_00\sounds_report_sizes.csv" pc sfx -bank:h3" It worked once but now doesnt build the sfx.sfb file. When you build this file shouldn't it go to the fmod folder?
- **2025-06-21** `ohwhen6442` [💬╎general-help] I'm trying to bake lighting on a BSP, and the reclaimer site uses the command prompt with this "python calc_lm_farm_local.py" but then I get a message that python isn't found. It's my understanding that calc_lm_farm_local.py is in the H3EK folder and I don't have to download python in order to bake lighting. Can anyone give some insight on this?
- **2025-06-30** `mrocnyzbik` [💬╎general-help] Question, today I found out how to extract EvE Online models (whitout downloading the game) and found a blender shader for these models. Where could I post both, for other people? Is it still going to "Asset upload download forum" with link to the people that done both?
- **2025-07-05** `chiefster_4014` [💬╎general-help] reach didn't use HDRIs so i'd suggest ripping the skybox and setting it up
- **2025-07-07** `shills6614` [📜╎ripping] I downloaded reclaimer like last week when I tried doing this, iirc I opened up the level blood gulch or one of the other multiplayer maps (I think it was the one with the two bases and arch and waterfall) to verify just to double check and got the same result.
- **2025-07-14** `zlitchufdux` [💬╎general-help] Just wanting to see if anyone knows why this error pops up when trying to rip content from the Halo 2 cut content maps?!
  
  I have all the .map files. But, I am getting this error.
  
  > Loading model: digsite\scenarios\multi\sunken_chamber\sunken_chamber.scenario_structure_bsp
  > Error loading model: digsite\scenarios\multi\sunken_chamber\sunken_chamber.scenario_structure_bsp
  > System.IO.FileNotFoundException: Could not find shared.map
  > File name: 'C:\Users\Downloads\Halo 2 Uncut-1732-1-0-1738950539\Halo 2 Uncut\maps\shared.map'
  >    at Reclaimer.Blam.Halo2.CacheFile.GetSharedCache()
  >    at Reclaimer.Blam.Halo2.IndexItem.<ReadMetadata>g__ReadMetadataInternal|32_0[T]()
  >    at Reclaimer.Blam.Halo2.IndexItem.ReadMetadata[T]()
  >    at Reclaimer.Blam.Halo2.MaterialHelper.GetTemplateArguments(shader shader, Dictionary2& options, String[]& usages, String[]& arguments)
  >    at Reclaimer.Blam.Halo2.MaterialHelper.PopulateTextureMappings(Dictionary2 bitmapCache, Material material, shader shader)
  >    at Reclaimer.Blam.Halo2.Halo2Common.GetMaterials(Halo2GeometryArgs args)+MoveNext()
  >    at System.Collections.Generic.List1.InsertRange(Int32 index, IEnumerable1 collection)
  >    at Reclaimer.Blam.Halo2.Halo2Common.GetMeshes(Halo2GeometryArgs args, List1& materials)
  >    at Reclaimer.Blam.Halo2.scenario_structure_bsp.GetModelContent()
  >    at Reclaimer.Blam.Halo2.scenario_structure_bsp.GetContent()
  >    at Reclaimer.Utilities.IContentProvider1.CachedContentProvider..ctor(IContentProvider1 p ...[cut]
- **2025-07-14** `zlitchufdux` [📜╎ripping] Has anyone figured out how to load up the Halo 2 cut content multiplayer maps into Reclaimer?! I am getting errors from them.
  
  Or if not, is it possible to load them up into the Halo 2 Editing Kit/Sapien and repackage the .map file?
- **2025-07-14** `pormithipicus` [📜╎ripping] Is there a comprehensive up to date tutorial on ripping halo 3 multiplayer maps?
- **2025-07-26** `chiefster_4014` [💬╎general-help] AMF doesn't work on the latest versions of Blender. it's also outdated. use the RMF plugin for Blender and export as RMF from Reclaimer instead.
  also, since you're trying to get Digsite stuff I'd suggest just grabbing it from their Github instead of the .map files: https://github.com/digsite
  And then use this addon to import them: https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master
  [embed] Digsite Crew https://github.com/digsite Digsite Crew has 6 repositories available. Follow their code on GitHub. GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master CE/H2/H3/ODST JMS/JMA/ASS exporter 
- **2025-08-01** `lomnk` [📜╎ripping] Hello i was opening Mombasa Streets (h100.map) in Reclaimer to extract into blender but some of the top parts of the map are untextured as well as most of the billboards and signs. The skybox is also like this Even though I should already have the required .map files for ODST in the same directory (shared.map, mainmenu.map, and campaign.map) (the files i got from https://haloce3.com/xmf/ if that matters). 
  After exporting it anyway and opening it blender these yellow parts are just pure white. 
  Am i missing a file or should i just somehow manually find the textures for these parts myself?
  [attachments] image.png
- **2025-08-05** `frankr2994` [📜╎ripping] Not that I'm aware of. And I think for something like h3 reclaimer is the only option. For something like h4 you can use reclaimer and or foundry . They work a little different both being good tools. But typically if I have all my geometry and materials I'm happy....so someone else might have better input
- **2025-08-09** `chiefster_4014` [📜╎ripping] for CE, H2, H3, ODST, Reach, H4, and H2AMP we use Reclaimer - https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1616
  for CEA and H2ASP specifically we use Index - https://github.com/Wildenhaus/IndexV2/releases
  [embed] Release v2.1.1616 · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1616 fix for certain H1 custom edition maps not opening Releases · Wildenhaus/IndexV2 https://github.com/Wildenhaus/IndexV2/releases A tool for browsing and extracting assets from Saber Interacti
- **2025-08-11** `chiefster_4014` [📜╎ripping] here's Reach's magnum animations specifically
  https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main/HaloReach-Xbox/data/objects/characters/spartans/fp/weapons/pistol/fp_magnum/fp_magnum
  [embed] HaloAnimationRepository/HaloReach-Xbox/data/objects/characters/spar... https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main/HaloReach-Xbox/data/objects/characters/spartans/fp/weapons/pistol/fp_magnum/fp_magnum A Repository of ripped Halo Animations for use in MCC modding. - 77Mynamei
- **2025-08-27** `manusanram` [🔍╎texturing-and-shading] Hello to all you guys
  I was wondering something; I'm sure you guys have encountered issues with the light volume textures for Halo Reach on Reclaimer (How they are always clipped exactly in half)
  Is there a way to get the full texture? I was working on something and I want to know how to get the resources correctly
  [attachments] image.png
- **2025-08-27** `manusanram` [🔍╎texturing-and-shading] It's called fx_godray_a_alpha, it is found on the map m45 of halo Reach and the version of Reclaimer I have is 1.7.808
- **2025-08-27** `chiefster_4014` [🔍╎texturing-and-shading] the latest version of Reclaimer is 2.1.1608. you are almost 3 years behind
  https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1608
  [embed] Release v2.1.1608 · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1608 1608 Patch
  
  Fixed some Reach workshop maps not working correctly
  Increased maximum batch worker count to 30
  Fixed H2V multiplayer maps not able to open any models
  Updated RMF importer to wor
- **2025-08-28** `yeetabix6876` [🔍╎texturing-and-shading] can anyone explain to me how the shaders work for the odst helldiver model rip?
- **2025-08-31** `djxsam23` [🔍╎texturing-and-shading] Hello! I've been playing around in Blender and in the FrostyEditor for ripping, "trying" rigging and texturing some Battlefield 4 asset and I was wondering what are the MM and D textures are for. I know that AO, CM and N are important for the 3d model but how usefull are the other 2? 
  
  Also I might need help with the texturing. At the moment I'm using the SWBF2_Materials_Version1.1 node graph in the Shader Editor in Blender to apply textures and I'd say it's okay. It does the job but I wonder if it would be better to do a custom one for Battlefield 4 instead?
  
  Thanks if someone can help me!
  [attachments] AO.png, CM.png, N.png, MM.png, D.png
- **2025-09-05** `42manako` [🔍╎texturing-and-shading] does anyone here have expierience modding ArmA 3?? I need help with something
  I found a beautiful model of a KrAZ Shrek-1 MPV from an ArmA 3 mod I downloaded, but the texture wont apply properly.
  I used PBOManager to extract the .p3d and .paa files from the pbo, and using the Blender addon ArmA Toolbox for Blender I imported the .p3d model. I converted the .paa textures to .png successfully, but when I try to apply the texture to the 3d model, it appears as a solid color only. Any ideas?
  [attachments] Screenshot_247.png
- **2025-09-13** `retrogasm2000` [📜╎ripping] guys does anyone have ripped elite with baked cubemap shaders from the original Halo CE in blender or maya file?
- **2025-09-16** `autarchkatherlyn` [📜╎ripping] with that, you can straight up import CE stuff into blender, but you'll have to extract import-info for 2 -> ODST
- **2025-10-08** `watermelon2364` [💬╎general-help] Bouta go rip some game skyboxes from Halo because I seriously tried so hard to figure out how the devs made the classic ring shape going into the horizon but cannot find any resources at all. My question is, are the skybox files in a cubemap or hdri format or just like impossible to extract?
- **2025-10-09** `captain_ramlethal` [💬╎general-help] https://youtu.be/CeO1w9Qe6MY?si=FcVeLLZkoJUGIIwb
  [embed] [2020] Extract and import the Titanfall 2 models in blender | A ble... https://www.youtube.com/watch?v=CeO1w9Qe6MY This video will teach you how to extract the models, textures from the game files.
  Applying the textures in the shader tab in blender (part 2): https://youtu.be/UV3SxMEsMe0
  
  Download li
- **2025-10-09** `chiefster_4014` [💬╎general-help] You can't exactly port a .map file into Blender, but you can extract its contents.
  Assembly is used for modifying .map files and reading the data inside but not extracting, so you won't need it.
  Reclaimer is for extracting assets from a .map file like models and textures.
  The Halo Asset Blender Toolset is mainly used for importing or exporting assets like models and levels in the Halo 1 to ODST Editing Kits. It can't be used with .map files.
  Reclaimer should be fine if all you need is to extract models from Halo CE. It used to have issues but I believe they're all fixed by now. If you're downloading the .map files off of a site like https://haloce3.com/xmf then you need to make sure you're downloading all of the shared .map files which are also needed like campaign.map, mainmenu.map and shared.map.
- **2025-10-10** `tim_irl` [💬╎general-help] Not sure where to post this- is anyone in need of a modified version of Reclaimer that can export .RMF files with the original lightmap UV channel intact? It allows you to keep the original baked lighting from Halo 2 when you pair it with proper shader in blender or other 3d software.
- **2025-10-10** `chiefster_4014` [💬╎general-help] Also iirc General-101's Halo Blender toolset can write lightmap UVs and .bitmap tags for the Halo CE and 2 editing kits for MCC. This could be of some use for extracting the bitmaps from the editing kits.
  GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2025-10-15** `rippercr.` [🦴╎animation] Hi guys, Is there a way to extract H2A Multiplayer animations? so I can import them into Blender
- **2025-10-19** `chiefster_4014` [📜╎ripping] We have access to most if not all of the assets thanks to Digsite porting them to Halo MCC's CE mod tools. You can get them from their official Github page here: https://github.com/digsite/h1
  
  Some of the assets you may need General-101's Halo Asset Blender Development Toolset to import, which you can also get here: https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  
  As for anything else that wasn't ported by them we aren't allowed to use or upload. We also aren't able to rip from those builds.
  [embed] GitHub - digsite/h1 https://github.com/digsite/h1 Contribute to digsite/h1 development by creating an account on GitHub. GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for
- **2025-10-21** `_themadmonk` [📜╎ripping] https://youtu.be/TCib8aBLLPE?si=V5lF2nFVllFDFRG6
  
  Hey does anyone know where I could find a corner piece or a T version of the covenant hallway in this animation? Looks like reach assets
  [embed] Halo: Truth and Reconciliation https://www.youtube.com/watch?v=TCib8aBLLPE I'd like to present Halo: Truth and Reconciliation
  A passion project where Masterchief has boarded a covenant ship, just like the mission in Halo Combat Evolved!
  
  This animation is a showing of love for one of my favourite vi
- **2025-10-25** `vor4dorsraven` [📜╎ripping] Given that the new remake looks to be on a modified Reach engine, y'all think it'll be easy enough to rip stuff from it?
- **2025-11-05** `chiefster_4014` [📜╎ripping] If you have the Reach, H4 or H2AMP mod tools installed and extracted, you can simply drag & drop a sky's .scenery tag into Blender and Foundry will automatically set up all of the shaders for you. If you're reimporting back into the tools make sure to use the ``Setup as Asset`` option when importing.
- **2025-11-06** `thatwotwfan2023` [💬╎general-help] So I got good news and 2 bad News
  
  Good new is I figured out how to get the Textures to work and now I can see the armour glory
  
  1st bad news, its the Wrong Textures im looking for. Im looking for the campaign Textures (with 117 on the chestplate) whiles this have the Multiplayer Textures. No offense to the person that extracted both Model and Texture. So im gonna keep searching for the Campaign Textures somewhere
  
  2nd and Final issue, Both Model and Textures are very High for the avatar im making in Vrchat (im trying to make it Quest + PC) so im gonna try to figure something out. 
  
  But thank you all for the help so far
  [attachments] image-144.png
- **2025-11-08** `chiefster_4014` [📜╎ripping] You can extract the models either from the official Halo CE Mod Tools with [this addon](https://github.com/General-101/Halo-Asset-Blender-Development-Toolset) or extract the models from MCC or another release of CE with Reclaimer. If you rip it from Reclaimer then you have to scale up the model's rig by a value of 32.8084
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2025-11-08** `chiefster_4014` [🔍╎texturing-and-shading] Here's a link to a pretty thorough resource for the main object shader that CE uses. Different versions of CE use different channel orders for the multipurpose map (the red/green/blue texture). Yours looks like it's in the Xbox channel order which the page also describes.
  https://c20.reclaimers.net/h1/tags/shader/shader_model
  [embed] shader_model https://c20.reclaimers.net/h1/tags/shader/shader_model The shader_model tag is used for opaque materials on object models. It supports features like map an...
- **2025-11-19** `pix3lpirat3` [📜╎ripping] I’m learning this whole system as a whole, I don’t have much prior Halo modding experience.
  I’ve got reading mvar files down to a point based off a few thousand Halo 3 .mvar files (Including Compressed Xbox360, as well as reach (based off a few thousand .mvar files), reach is missing some extract logic for items and maps since people make everything in forge world.
  
  Now I’ve gotta figure out how to rebuild the map and import their objects to recreate a maps environment, this is the system I’m stuck on atm
- **2025-11-20** `exodst.` [📜╎ripping] Does anyone know why when i import the JMA of a Halo 2 elite animation, it gets scaled up unreasonably - like 33.33x as much as it should be? Im trying to export these animations into unreal which will always apply the scale and then the animation will be messed up. I must be missing something. Im using the "Halo Tools Helper" blender addon and got the animations from a archive on github here: https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main/Halo2-Xbox/data
  [embed] HaloAnimationRepository/Halo2-Xbox/data at main · 77Mynameislol77/... https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main/Halo2-Xbox/data A Repository of ripped Halo Animations for use in MCC modding. - 77Mynameislol77/HaloAnimationRepository
  [attachments] image.png
- **2025-11-21** `exodst.` [📜╎ripping] Yeah, i looked all over the internet last night trying to find a response and i believe someone did something just like that, but there is alot of animations i need to go over so i was going to need something more feasible. I tried using the import scale but for some reason it was giving me problems on the original elite skeleton i got from reclaimer, even though it was the exact same as the one used for the animations. Anyway, super glad i got it all figured out.
- **2025-11-24** `exsurg3_` [💬╎general-help] Anyone that's good with weight painting stuff able to help or fix this? I've been trying to get this hand ripped from the H5 elite pack and onto a infinite rig. I've messed around with the weight painting stuff but no luck.
  [attachments] image.png
- **2025-11-25** `lytatroan` [📜╎ripping] I can't find the Halo 3 Gravemind tentacles model, how would I go with porting it to Blender?
  I was thinking of using the Reclaimer tool, I think I used it once to rip CEA's BSP's for a small render so I think it would be a good tool for ripping H3 models too.
- **2025-11-25** `lytatroan` [📜╎ripping] @swagster Can I have your Halo 3 shader? Ripping the Gravemind's model from H3
  [attachments] image.png
- **2025-11-26** `chiefster_4014` [📜╎ripping] Once you download it you need to go into its directory and run the H3EK.bat file to extract all of the tags. Then once it's done you can use guerilla.exe to open the tags
- **2025-11-30** `mrocnyzbik` [📜╎ripping] OK, here is how it is done (one way). To download it from the GMod (not other workshops) You need either:
  https://github.com/imwaitingnow/WorkshopDL (works with most things on Gmod workshop)
  or this:
  https://steamworkshopdownloader.io (works with every other workshop, when one does not work, most of the time the other does)
  Than you will get the .gma file from it, u need this to unpack it:
  https://www.moddb.com/mods/garrys-mod/downloads/gwtool-file-made-by-zombieslayer103 (it might unpack it, or said that you have to unzip it, use 7z to unpack then, do not worry about the warning it does not matter, and after unpacking put into GWTool again)
  Then you will get "model" folder in which the file you want to import to blender will be, you do that by using this Addon for Blender:
  https://github.com/REDxEYE/SourceIO (use download button below readme and mdl file import in blender)
  You might have to play with Raughness and Metalness, sometimes there arer no files for that, or they are not connected and you need to do it yourself. If there is "phonge" unconnected, it usually goes "separate color - Red node + Inverto Color into Roughness, Green node into Metalic "
  You can also unpack mdl with Crowbar and then import it into Blender with Source Tool. If you have any trouble, send a flare, I will try to help as much as I can.
  [embed] GitHub - imwaitingnow/WorkshopDL: WorkshopDL - The Best Steam Works... https://github.com/imwaitingnow/WorkshopDL WorkshopDL - The Best Steam Workshop Downl ...[cut]
- **2025-12-06** `tristangold` [🦴╎animation] Hello! Had an animation question that's also part ripping question so please feel free to let me know if it should go into that section instead. 
  
  So I have two characters (non halo humanoids) that I have in my Blender scene and I was hoping to see if I could perform a simple assassination with - I think a Spartan v Spartan multiplayer reach one would be perfect. 
  
  I'm following along with the Plastered_CrawDAD tutorial however this seems to only apply to taking the Halo models and then through Bonobo and the other tools applying the Halo assassinations on top of them. Since my models aren't Halo related with a simple Mixamo skeleton, is it possible for me to retarget the animations or would the difference in skeletons make this impossible? 
  
  How I'm envisioning the process is A) Get 2 Halo Spartan Models, B) Apply the Assassin and Assassinated animations onto both of them. C) Retarget Animations onto secondary models once in blender properly. 
  
  Wanted to see if anyone has tried this yet
- **2025-12-09** `tristangold` [📜╎ripping] quick ripping question - so I am trying to port some of the Halo Reach assassinations into blender via Foundry with the animation repository. First attempt to drag and drop Repo animation file into Blender failed and just want to understand exactly what I'm missing.
  
  1. For me to properly import the animation data - do I first need to extract it onto a spartan model first or can I rip the data and bones just directly into blender. 
  2. If I do need a model, are these findable in to Omaha directory or just easier to grab through the discord's already existing models. It's possible I've also been searching through the Model tools folder rather than Halo MCC itself, but navigating the files I've really just been coming across .hsc scripts.
  3. Is there a specific location I need to be dropping these animation files into on the Omaha folder? My drag and drop attempt spit back the error "Ensure your file is saved to your Omaha data directory"
  
  EDIT: poked around the discord some more so also gave a go at importing one of the animations through the .jms importer. Got an internal error noting the "action object has no attribute 'fcurves'
- **2025-12-09** `chiefster_4014` [🔍╎texturing-and-shading] for CE, H2, H3, ODST, Reach, H4 and H2AMP we can get the shader code from the official mod tools. For CEA and H2ASP it can be extracted with Index. For the other games it's just a lot of educated guesses afaik
- **2025-12-10** `un_nonimus` [📜╎ripping] Has anyone figured out ripping the lightmap uv data and lightmaps for reach or any of the other games for that matter?
- **2025-12-10** `un_nonimus` [📜╎ripping] I mean specifically exporting the lightmap UV data from the bsp itself. Then also using the lightmaps for the specific bsps to essentially have accurate baked lighting for levels. 
  
  I know in reach for instance there are some lightmap bitmaps in the tags. However, when exporting the geometry using reclaimer that secondary set of uv data isn’t written for the models. So the lightmap textures are useless.
- **2025-12-13** `arraymonitor` [📜╎ripping] I know Halo 3 Cant be ripped directly and u have to put the levels in reach and so on
- **2025-12-18** `chiefster_4014` [💬╎general-help] There's a repository of animations from CE to Halo 4 here:
  https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main
  I don't know of any addons for 3DS to import these, but for Blender all of the animations can be imported with the Halo Asset Blender Development Toolset addon:
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main A Repository of ripped Halo Animations for use in MCC modding. - 77Mynameislol77/HaloAnimationRepository GitHub - General-101/Halo-Asset-Blender-Development-Too
- **2025-12-26** `exodst.` [🦴╎rigging] Is this elite exported from reclaimer? I believe when i was importing the halo 2 grunt from reclaimer to blender, alot of vertices were missing from the vertex groups. I have been getting the models from the google drive ever since, i believe someone else fixed them.
- **2026-01-02** `chiefster_4014` [💬╎general-help] Is this map from Halo 3? If it is then did you extract the render data from the H3EK or did you get it some other way?
- **2026-01-02** `rdt4` [💬╎general-help] yea i extracted it from h3 using reclaimer. and also used cr4b tool. still had the prob with both
- **2026-01-02** `chiefster_4014` [💬╎general-help] Extracting it with Reclaimer isn't good for reimporting back into the game since the geometry won't be the same as what was originally imported. You should extract it from the H3EK itself instead since you get the original geometry that way.
  I don't remember if CR4B tool can do that but it might be better to just go it manually
- **2026-01-02** `rdt4` [💬╎general-help] Really? hmmm, how do i go about Extracting  from h3ek and put it into reach? then. i only have this prob with reach ive tried with h2a and it worked perfect. cr4b tool can make the maps if that is what ur talking about?
  [attachments] image.png
- **2026-01-11** `welxor` [💬╎general-help] 👋 Hi guys I have some time to kill for the week and was wondering...
  I intend to make a Sort of kitbash between the elites of the "Elite 2.3" model pack to make a custom model of Xytan'Jar Wattinree resembling to an extent the canon artworks and the model of "The winnower"'s 3d model
  With this is mind I have questions:
  ☝️Firtsestly, I have read that you here demanded for Authorization before ripping models. So. 🫴  Do you consider kit-bashing and making a model without intent for its commercialization ripping here. Do I have the right to reuse "Elite 2.3" creator's pack for my personal use?
  ✌️ Second, If I have the right to do all of the above, would you mind if I shared it once finished? Gatekeeping would be... Disappointing after all.
  
  Lastly, a slight problem 🤏 
  I am a professional designer but not professional 3d artist. And it would be nice if in your assistance you would lend me some help by **informing me if they are some more resources, plugins or perhaps tools, I could and should use**.
  Know that i am proficient in 3d modeling, sculpting, low polying, optimizing models. But a mess in UV mapping and texturing.
  **I Could really use a tip a or two about how to repaint properly the sangheili's armor texture**
  I plan to use: blender (of course) and **substance painter**. Maya and Zbrush are also options to me but not my comforts apps. I might make two models: one lowpoly with few materials and one more high quality ended.
  The only out of the pack plugin that I use is PolyQ ...[cut]
- **2026-01-23** `runesoldiero` [💬╎general-help] Also are the MCC Reach models better, or should I try ripping into Reach itself?
- **2026-01-24** `koax117` [💬╎general-help] What are you trying to rip?
  Have you looked into what is already ripped and shared in https://discord.com/channels/819709630540021810/1029858676032417875 ?
  If you're planning on utilizing reach assets, if it's something that isn't already ripped, I'd suggest using Reclaimer.
- **2026-01-24** `gameoverload` [💬╎general-help] Ekur is Infinite only like I said and handles more stuff for Infinite.
  
  Reclaimer is better if you want to handle other games, or have stuff that Ekur can't do for Infinite ( certain models with no shader yet for example. )
- **2026-01-24** `runesoldiero` [📜╎ripping] Its not a concrete list at the moment, since I'm just getting into ripping, but ideally MCC and Infinite, (Reach if neither have some textures and models i want) Far cry 5, Disney Infinity (and possibly any other Disney game with Sugar Rush related assets) and maybe some N64 or PS2 games
- **2026-01-26** `.username_ladiesman217` [🦴╎animation] *Update:
  I tried both routes, fortunately, I had the day off today.
  I downloaded The Halo Asset Blender Dev Tools set, found a Reach Rig (way too big by the way) and imported a jmm animation.
  I went and did it by "hand" but the Reach Rigs are not in the same way at all to Infinite's Noble Pack.
  I gave it a go, but after 5-6 hrs of messing with Pole Vectors childs (freaking kids) and such. I paused and then went the retarget route.
  
  I did Mixamo and it was "okay,"  I continued with Rokoko, but got nowhere, then tried it with Mwni (free retargeter I found on github), wrote a python script to extract each and every bone name from their respective armatures (OG Reach's and Infinite's) and individually retargeted every bone pair and driver I thought fit best but I got the same thing that @JACK got in 2024.
  Spent about 3 hours there, maybe 5.
  
  I introduced myself to Unreal but I just familiarized myself with the UI.
  
  
  Just to get some work to show for the day I did and will probably continue doing a combination of all three approaches.
  - I'll do subtle gesticulations by hand. (thanks purp, I do like to learn the fundamentals)
  - I'll try to get to retarget armor and the main skeleton's movements (thanks sniper, this will keep me from literally killing myself, and it'll help me work faster; I just have to make it work first).
  - And I'll continue on the roto animation route I was doing before (which basically is just using reference and then try to replicate it with your animation, in ...[cut]
- **2026-01-27** `plainfurniture` [🦴╎animation] Would anyone know if the armor lock animation (the player hitting the ground specifically) from halo reach has been ripped or re-made? Tried sleuthing for it with no luck
- **2026-02-09** `iloveagoodcrisp` [📜╎ripping] Extract import info takes an editing kit relative path or a full system path. So it will either start with `tags` or a drive.  Not `H3EK`
- **2026-02-09** `magmashark009` [📜╎ripping] but I got it from extracting the file inside the editor, and even removing "H3EK" seems to do nothing
  [attachments] image.png, image.png
- **2026-02-09** `magmashark009` [📜╎ripping] turns out all I had to do was move the folders from inside the extracted zip folder of the same name as "H3EK" to outside to the original folder where the editor was located
  [attachments] image.png
- **2026-02-22** `echo_mation` [📜╎ripping] Anyone know if I can pull the classic halo 2 ODST model and rig with reclaimer? I've been trying for a while and can't figure it out. I can only find the bitmap, shaders and ai. (Im using the delta halo map)
- **2026-02-27** `gameoverload` [📜╎ripping] So, H2A uses two different systems for its Singleplayer/Multiplayer.
  Singleplayer uses Saber3D.
  Multiplayer uses BLAM.
  
  So you need to go back to using Reclaimer for H2A MP, as it uses BLAM like usual.
- **2026-03-03** `okom` [🔍╎texturing-and-shading] The node arrangement of the Halo Infinite Shader 3.1.2 on Spartans ripped from gamertag with Ekur looks like this. Any plans on fixing it, and aware if it's an Ekur issue specifically? Have to fish out specific nodes to find connections for modifying the output. @Chunch
  [attachments] 2026-03-03_blender-uwrf.png
- **2026-03-04** `koax117` [💬╎general-help] As far as campaign goes, I haven't seen anything concrete out there.
  https://www.reddit.com/r/halo/comments/rfjfrf/unlike_the_pillar_of_autumn_halo_2s_cairo_station/
  ^shows a neat layout of Cairo.
  I know in some of the behind the scenes documentary videos, you can see whiteboard drawings of the map layouts in some parts. But that's WIP and not final in many instances.
  
  Multiplayer wise, this is a good reference
  https://steamcommunity.com/sharedfiles/filedetails/?l=schinese&id=2118066301
  
  that's all I can think of. Always find mission layouts cool, like this Halo 3 of Voi.
  https://www.youtube.com/watch?v=W64UxciD-gQ&t=28s
  [embed] Unlike the Pillar of Autumn, Halo 2's 'Cairo Station' level design ... https://www.reddit.com/r/halo/comments/rfjfrf/unlike_the_pillar_of_autumn_halo_2s_cairo_station/  Halo 3 - Portal at Voi - Surrounding Landscape Revealed https://www.youtube.com/watch?v=W64UxciD-gQ&start=28 Made in Blender with 3
- **2026-03-06** `_manchurian_` [💬╎general-help] Does anyone know where I can find a guide on halo reach custom maps, ie export from blender into hrek? There's some guides for halo 3 on the reclaimers library but not sure how much applies to reach engine if anything. Thanks.
- **2026-04-04** `ravenwhite320` [📜╎ripping] Hello there,  I was wondering if halo reach animations can be ripped and ported to blender?
- **2026-04-04** `chiefster_4014` [📜╎ripping] yes, and they have been already. you can get them from this repo: https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main
  and import them into Blender with this addon: https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main A Repository of ripped Halo Animations for use in MCC modding. - 77Mynameislol77/HaloAnimationRepository GitHub - General-101/Halo-Asset-Blender-Development-Too
- **2026-04-09** `notsnezzypaladin` [📜╎ripping] Could anyone rip the Halo Reach armor ability pucks fbx and textures for me? I've checked every where on multiple sites and servers and can't find anyone whose done it
- **2026-04-14** `exodst.` [💬╎general-help] https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1624
  [embed] Release v2.1.1624 · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1624 new builds:
  
  added support for additional H3 and Reach Alpha builds (bitmaps and render_models only)
  
  misc bug fixes:
  
  fixed a couple H2 bitmaps giving a "bitmap format not supported" err...
- **2026-04-16** `chiefmonkey822` [💬╎general-help] I am making a diorama and I have already extracted the signs from Halo 3 ODST, but I was wondering if there was a way to get the signs from Infinite from the game files :ChiefCry:
  [attachments] image.png
- **2026-04-26** `koax117` [📜╎ripping] Here's the Reach Gravity Hammer textures as targas.
  to get the rest, I recommend setting up Reclaimer
  https://github.com/Gravemind2401/Reclaimer
  [embed] GitHub - Gravemind2401/Reclaimer: A Halo game file explorer https://github.com/Gravemind2401/Reclaimer A Halo game file explorer. Contribute to Gravemind2401/Reclaimer development by creating an account on GitHub.
  [attachments] gravity_hammer.tga, gravity_hammer_bump.tga, gravity_hammer_illum.tga
- **2026-04-28** `sharklordsatan` [💬╎general-help] Does anyone know of any dedicated Resident Evil model ripping/blender servers like this one? I swear there used to be a list of similar servers somewhere in this one but I dunno if I'm hallucinating it or not
- **2026-05-22** `chiefster_4014` [🔍╎texturing-and-shading] What each channel of the multipurpose map does depends on the settings set in the material's shader tag. That one appears to be in the Xbox channel order, so the red channel should be the detail mask, green channel is the self Illum mask, and the blue channel should be the reflection mask. This page should describe the shader tag for this material in more detail if you want to be accurate to CE: https://c20.reclaimers.net/h1/tags/shader/shader_model
  
  I can't recall exactly the setup to use the reflection map exactly, but I know the texture itself must be used with the Environment Texture node instead of the normal Image Texture node so it reflects properly. Iirc for the rest you just need a Geometry Node and Vector Math node set to Reflect. The Incoming and Normal outputs of the Geometry node go into the A and B input of the Vector Math node. The output of the Vector Math node then gets put into the Vector input of the Environment Texture node.
  
  I can send a visual example when I'm home, but that'll be over an hour from now.
  [embed] shader_model https://c20.reclaimers.net/h1/tags/shader/shader_model The shader_model tag is used for opaque materials on object models. It supports features like map an...
- **2026-05-29** `koax117` [💬╎general-help] Characters, Weapons, and Vehicles have all been ripped. 
  Forge pallet is set up, somewhat. 
  Surasia is working on getting things going for multiplayer environments, but its slow going, so don't hold your breath
  (not knocking Surasia and Co. for speed, life happens, and its a lot of complex tedious work)
- **2026-06-03** `yesimviolet` [📜╎ripping] does anyone have the textures for mp_extractor_arm/el_extractor_arm from halo 3 the tags 7z that come with the mod tool doesnt have the textures it only has the shader and i cant find them in any of the map files
- **2026-06-03** `koax117` [💬╎general-help] I don't have the CE mods tools installed, so I can't vouch for how any of it works sadly.
  For when I'm working in H3, I load up Guerilla and will navigate to locate the object's render.model / relevant shader. if there's any textures missing that didn't rip with the model, that's where i'd go to look for it.
  [attachments] image.png, image.png
- **2026-06-04** `032mendicantbias` [🔍╎texturing-and-shading] Kinda random, does anyone have a shader for Shadow of War/know what kind of material setup it uses? I have some ripped stuff but some of the textures look odd.
- **2026-06-04** `koax117` [🔍╎texturing-and-shading] ah. I *attempted* to take a stab at a Shadow of War/Mordor shader a couple years back.
  there was an article on facepunch back in the day i have bookmarked that broke it down a little bit, but it looks like that site is gone now. You can read a little bit on it on the waybackmachine, but I don't know if it'll have all the information you might be looking for.
  https://web.archive.org/web/20210731210806/https://forum.facepunch.com/t/shadow-of-mordor-ripped-models-please-share-if-you-have-them-in-any-kind/227892
  [embed] Shadow of Mordor ripped models - please share if you have them in a... https://web.archive.org/web/20210731210806/https://forum.facepunch.com/t/shadow-of-mordor-ripped-models-please-share-if-you-have-them-in-any-kind/227892 Hello! I browsed the forum and found a heap of game models, usable not only 
  [attachments] image.png, image.png
- **2026-06-05** `chiefster_4014` [💬╎general-help] The addon isn't malicious. General101 simply named one of the variables in his code to ``this_is_a_horrible_hack``, probably because he knew it would stop working at some point, and it did.
  I'm not sure how to fix this error, but you might find more help in the dedicated Halo modding discord servers. You should be able to find them in #🤝╎partner-servers  with the names Halo Mods and Halo Modding Reclaimers
- **2026-07-12** `annelyzer9000` [🔍╎texturing-and-shading] I swear the Halo Reach marines had different types of camoflages for their BDU's... Does anyone know what they're called, so I can find them in Reclaimer? Just "camo" didn't turn up with a whole lot, neither did "detail_map"... Any other tags I could try? So far I only found a digital camo version
- **2026-07-27** `felixvyra` [💬╎general-help] Hi.
  
  Does anyone know anything about the original Halo combat evolved campaign shader pipeline and if anyone has been able to make a decent approximation of it in Blender?
  Along with some accurate Halo Combat Evolved rips.
  
  I mean the original look of the covenant surfaces specifically for the weapons and vehicles and armors is so unique and I would really love to inspect them more closely in Blender.
- **2026-07-28** `infiniteforges` [📜╎ripping] im building sierra 117 in ue5 and would love to connect with someone that knows how to rip assets so that i can get this level as high fidelity as possible. If youre open to helping, reach out. Thanks!
- **2026-07-31** `setriaphtfk` [🔍╎texturing-and-shading] On this model I have been working on the rip of, these maroon stripes are just plains over the top of the ship’s body. They are just slightly transparent.
  I want to try and make some sort of color mask so is matches up in materials and normals and all that. I have a plan for how to make that work, I just need to know if it’s possible and how to essentially take the stripes and project them or imprint them onto the main texture.
  
  Maybe to simplify: take a cube and a small triangle plain on one side. I want to make the triangle a part of the texture on the cube beneath it. Is that possible? How do I do it?
  [attachments] Phaeton_Class_v2.png
- **2026-08-04** `zoephie` [📜╎ripping] its in the latest development build https://github.com/Zoephie/Baboon/releases to access the unreal engine features you just load your campaign evolved then select chimp at the top which opens a similar view to fmodel from here just locate any skeletal mesh, or static mesh of your choice which we support extraction of .jms or pskx or if you only need the textures you just find any texture of your choice and right click it and export.
  [embed] Releases · Zoephie/Baboon https://github.com/Zoephie/Baboon/releases A native Rust GUI built on blam-tags for editing Halo MCC editing-kit tag files (Halo 3, ODST, Reach, Halo 4, H2A). Provides byte-exact tag reading and editing, asset extraction (bitmaps, geometry,...
  [attachments] image.png, image.png, image.png, image.png, image.png, image.png
- **2026-08-04** `felixvyra` [📜╎ripping] Hey does anyone know if reclaimer can rip shader code?
  I am looking at halo 2 shaders but there is no option to extract it or anything.
- **2026-08-04** `chiefster_4014` [📜╎ripping] Reclaimer can't rip shader code. As far as I know the only way to get Halo 2's shader code is from the MCC mod tools as Brooen said. You have to look for the .shader_template and .shader_pass tags
- **2026-08-04** `chiefster_4014` [📜╎ripping] There might be another method to get the shader code, since all that the tags have is in assembly.
- **2026-08-08** `zoephie` [📜╎ripping] can grab latest here. https://github.com/Zoephie/Baboon
  [embed] GitHub - Zoephie/Baboon: A native Rust GUI built on blam-tags for e... https://github.com/Zoephie/Baboon A native Rust GUI built on blam-tags for editing Halo MCC editing-kit tag files (Halo 3, ODST, Reach, Halo 4, H2A). Provides byte-exact tag reading and editing, asset extraction (bitmaps, geometr
- **2026-08-16** `koax117` [💬╎general-help] can find them here, https://github.com/77Mynameislol77/HaloAnimationRepository
  or extract them with Baboon https://github.com/Zoephie/Baboon
  retarget should work fine, just need to line up the armatures and bones
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository A Repository of ripped Halo Animations for use in MCC modding. - 77Mynameislol77/HaloAnimationRepository GitHub - Zoephie/Baboon: A native Rust GUI built on blam-tags for
- **2026-08-19** `moonify.sfm` [📜╎ripping] Can I have some help? I'm trying to rip the Halo Reach armor set, but there's no option to rip specific parts
- **2026-08-19** `green.knight` [Halo 5 Animation Extractor] # Halo 5 Animation Extractor v1.1
  **What is this tool?**
  This is a tool for extracting Halo 5 animations into Blender from the Halo 5 PC Forge Build or a dump of the entire game if you have one. Please check the readme for instructions on how to use!
  
  **Features**
  Supports ripping Base, Overlay, Replacement, and other animations.
  Support for composites (expect some weird results for now).
  Allows selection of only certain animation modes.
  Works with first and third person animations.
  Rips frame event data from Halo 5.
  Full integration with Foundry.
  
  **New Features**
  Fixes for Overlay animations.
  Support for Composite animations (work in progress...).
  New UI options to allow only certain animation modes to be ripped.
  Rips frame events from Halo 5. Sounds, effects, and dialog are left blank, but the events remain. Integrated with Foundry.
  Transfers animation flag properties from Halo 5 into the Foundry panel (ex. Disable Interpolation In/Out, Disable Mode IK, etc.).
  Faster importing.
  
  **Download:** https://github.com/Generalkidd/H5AnimExtractor
  [embed] GitHub - Generalkidd/H5AnimExtractor: Tool for extracting animation... https://github.com/Generalkidd/H5AnimExtractor Tool for extracting animation data from Halo 5 tags. - Generalkidd/H5AnimExtractor
  [attachments] H5_Animation_Export_Hydra.mov, H5_assassinations_in_H4.mp4
- **2026-09-01** `tom_76` [📜╎ripping] hey guys, i extracted an elite for campaign evolved but this dude have no vertex group even if he have the rig, any idea ? i extracted with fmodel in uemodel format
- **2026-09-10** `thelifeweaver` [📜╎ripping] This is going to be an unusual question - but has anyone actually tried to rip from Star Trek Elite Force or the RPG-X stuff? That is a bunch of old maps and assets including Voyager or any of the Single players levels.
- **2026-09-13** `scalemaster117` [💬╎general-help] I installed multiplayer first as per Ekur's own instructions, and then did a re-dump of the campaign level objects afterwards. From Ekur:
  
  "Are campaign assets supported?
  Yes! However, you do need to set up its own data folder and tick the "Is Campaign" option in the addon preferences, and forge import options will become unavailable." - https://github.com/TheHaloArchive/ekur
  [embed] GitHub - TheHaloArchive/ekur: A multi-purpose blender importer for ... https://github.com/TheHaloArchive/ekur A multi-purpose blender importer for Halo Infinite. - TheHaloArchive/ekur
- **2026-09-27** `fleetadmiral01` [📜╎ripping] Ripping STO models has some quirks to it. The only known means of doing it only exports to OBJ, so I'd have to completely rebuild the rig, And I'd have to manually color the armor/uniform pieces.
- **2026-09-30** `azarath__` [📜╎ripping] anyone know how to rip the first person skeleton rig used in the weapon anims in Reach(MCC), im trying to port the assault rifle to BO3 Zombies and need it but cant find it in reclaimer
- **2026-10-06** `koax117` [📜╎ripping] you can choose to export ALL the geo or just select, depending on your needs.
  if you import the bsp into Blender 3.6 first, you can use the Cr4btool addon to setup the materials. it isn't up-to-date/1:1, but gets you much closer to accuracy.
  https://github.com/PlasteredCrab/Halo-CR4B-Tool
  it'll require some additional setup, but it's what I've used as the baseboard for all my H3 rips
  [embed] GitHub - PlasteredCrab/Halo-CR4B-Tool: One-click game-accuracy tool... https://github.com/PlasteredCrab/Halo-CR4B-Tool One-click game-accuracy tool for Halo 3/ODST/Reach assets in Blender that also exports assets from every Halo game - PlasteredCrab/Halo-CR4B-Tool
- **2026-10-18** `koax117` [💬╎general-help] I don't know about Halo 3, but I know for Reach their is this blender-addon that I used once, and got it to rip a map live
  https://github.com/Waffle1434/Mjolnir-Forge-Editor
  No idea if the Foundry addon has forge support or not, but works for porting the regular maps well. https://github.com/ILoveAGoodCrisp/Foundry
  [embed] GitHub - Waffle1434/Mjolnir-Forge-Editor: A Blender tool for manipu... https://github.com/Waffle1434/Mjolnir-Forge-Editor A Blender tool for manipulating in game objects in Halo Reach Forge in The Master Chief Collection. - Waffle1434/Mjolnir-Forge-Editor GitHub - ILoveAGoodCrisp/Foundry: A Blender 
- **2026-10-26** `irone26` [📜╎ripping] Has anyone ever been able to rip weapon models and sounds from H2A multiplayer?
