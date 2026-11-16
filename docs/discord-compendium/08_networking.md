# networking

Selected from 473 candidate messages (ranked by length, links, Halo relevance; capped at ~260 KB). Chronological. Channels: Foundry, Halo 5 Animation Extractor, Osoyoos, 💬╎general-help, 💾╎autodesk, 💾╎substance-suite, 📜╎ripping, 🔍╎texturing-and-shading, 🦴╎animation, 🦴╎rigging

- **2021-03-22** `plastered_crab` [📜╎ripping] I feel like that would be helpful too! I'll get started on that sometime soon. I'd love for it to start with Halo CEA model ripping tools, Halo CE - 4 model ripping tools, Halo 2 - ODST sound ripping tools, Reach - 4 sound ripping tools and have it go in order
- **2021-03-24** `juniper_regruth` [📜╎ripping] Is anyone able to get the Tech suit or the visor for the Halo 4 Multiplayer models?
- **2021-04-09** `surasia` [📜╎ripping] https://github.com/Gravemind2401/Reclaimer/releases/tag/v1.5.245
  [embed] Release v1.5.245 · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v1.5.245 support latest version of MCC
  fix h4x cubemaps
  fix h4x node maps
  add support for h2x cubemap unwrapping
  add support for model LODs in h2x
- **2021-04-12** `killzone64` [🔍╎texturing-and-shading] youll notice if you import the multiplayer flags in halo 1 or 2 you only get the pole
- **2021-04-14** `rcnfl` [📜╎ripping] Anyone mind helping me rip my Reach model from MCC on PC? I tried a week ago and it was odd as I have never ripped models before
- **2021-04-19** `_gallium` [📜╎ripping] If the original value was `66` and you got that by fully ripping, you will get the same result as if you reach `66` via other means.
- **2021-04-20** `williaso` [📜╎ripping] But for ODST they were like "Yeah we're slapping a game together in less than 9 months who gives a fuck"
- **2021-04-20** `plastered_crab` [📜╎ripping] We plan on having an info channel dedicated to informing people about it
- **2021-04-20** `nobleofdeath16` [🔍╎texturing-and-shading] Could probably pull the projectile from Reach and use that too. Given where the hand is suppose to sit , Spiker animations would place the hand always at the drum and scrunch the arm a bit up, causing some clipping
- **2021-04-20** `vuthakral` [📜╎ripping] Hey, I just noticed my old Halo Online elite rips are on the Google Drive for this server, I'm just letting you guys know I fucked up the alphas included on those. Honestly, at best I'd hope to have it removed so people don't use them because of how fucked up they become if you use those alphas for specular masking (the intended thing for them)
- **2021-04-21** `williaso` [📜╎ripping] Probably just leftover files someone forgot to get rid of since it's based on Halo 4's multiplayer, or maybe a rig dependency or something
- **2021-04-22** `err_0r` [🔍╎texturing-and-shading] It's good to see someone else who's trying to do right by the H4 ODST stuff. I reached the same conclusion you did about the striping/paintwork being the biggest issue. It doesn't *feel* odst to me, so I also brought it through substance to see if I could get something more acceptable out of it. Admittedly it's some of my older substance work so it's not quite as refined as some of my newer projects but none the less I'm relatively happy with the texturework I got out of it. 
  
  An interesting bit of the project that I've used in some of my newer stuff since is that this is what I call a hybrid texture, combining the original textures with my own. I imported the diffuse as well as grayscaled channels of the specularity/metallic/smoothness, and set it up so it looked as it did in game with shine and everything, and then used the original color code masks to set where my paintwork went. This way I got rid of the horrid stripe nonsense without having to worry about the thousand little details, keeping my layer count down and computer happy.
  [attachments] unknown.png
- **2021-04-22** `err_0r` [🔍╎texturing-and-shading] Now, as a more general question, I know that H2A stuff is a little hard to get a hold of, in particular I'm looking for weapons. I've gotten a hold of an H2A sniper rifle textures which seem to go onto the reach sniper mesh,  as well as the magnum posted here in the server. My issue is that I'm having a hell of a time trying to figure out how to make these things look right in unity. I'm in the process of trying to cobble together shaders for it using Amplify shader editor but I'm not sure what data is what when it comes to these textures.
  For both weapons I've got an RGB (diffuse/albedo) and an RGB (spec?), as well as the normal map which I've already made sure is converted to Unity specs. 
  My question is if anyone's got a good idea of how these textures end up making the end result in the game. If I use the 2nd rgb map as specular what I find is that some areas are much too dark while others are far too white, and of course there's no roughness or smoothness. I've seen plenty of halo texture sets stick smoothness/roughness in the alpha of the diffuse while unity tends to stick it in alpha of the spec or metallic map but in this case there's no alpha anywhere and none of my attempts at figuring out what data is what is giving me visually satisfactory results. Does anyone have any good ideas about what the weapon shaders from H2A look like, how they work?
- **2021-04-22** `adorabirb` [🔍╎texturing-and-shading] Halo 2 Anniversary Multiplayer uses Albedo, a control with the Metallic, Ao, and smoothness, and a normal map
- **2021-04-26** `reversesideofthemoon` [💬╎general-help] @Surasia only the multiplayer files or it has the campaign files as well?
- **2021-04-26** `surasia` [💬╎general-help] Multiplayer map files. Currently there is no way to rip campaign other than ninjaripper
- **2021-04-27** `err_0r` [🔍╎texturing-and-shading] I personally decompiled and recompiled the cc textures so that I didn't have that data in the alpha, swapping it over to another channel so I could better visualise what the masks were doing, but the shader I tossed together in amplify shader editor ended up looking like this.
  [attachments] unknown.png
- **2021-04-28** `fiycsf` [🔍╎texturing-and-shading] The Halo 3 style ODST shoulders are obtainable without ripping however, on the McCarthy Sketchfab page from the Spartan model. 
  
  Afaik the rest of the ODST parts minus helmet attachments and a wrist part (that are also McCarthy Bros) are AbiSv and not intended for people to rip or have.
- **2021-04-30** `williaso` [💬╎general-help] https://www.youtube.com/watch?v=qMsqhMzEeBg You can do this
  [embed] What if Halo Multiplayer had intros like Titanfall? https://www.youtube.com/watch?v=qMsqhMzEeBg Lets call this a... proof of concept? I used a bunch of animations from Mixamo, with only a little bit of hand keying myself, so it's kind of rough.
  
  
  
  People in this video:
  Captain Jacob Keyes
  Spartan 1
  
- **2021-05-03** `_gallium` [💬╎general-help] In the materials section of the import settings you need to point it to the root path of where the bitmaps got exported, usually the location that has directories like `cinematics  fx  levels  multiplayer  objects  rasterizer  shaders`
- **2021-05-07** `dyland64` [🔍╎texturing-and-shading] It won't be complex, To reiterate 
  the armor is the original spartan II green but faded to near black/grey as if they color wore off, 
  It doesn't need to be overly detailed (pain chipping etc)
  I want the text to be exactly in the same Format as Spartan 2s, but based on the Identity Duval-025
  Black stripe on the top of Helmet and that is about it
- **2021-05-15** `surdavow` [🦴╎animation] Same, although the big problem with the elite models in Reach is that the horselink armor keeps overlapping with their feet.
- **2021-05-17** `djcheef` [💬╎general-help] where can I find my custom spartan or the multiplayer armor in reclaimer?
- **2021-05-17** `dogbrainludus` [🔍╎texturing-and-shading] it seems like they actually had a grand time doing the map packs for other games or even H4's multiplayer
- **2021-05-17** `Deleted User` [🦴╎animation] hello all umm i need help ripping animation files from halo 3 odst.map files
  bonobo seems to extract it but when i go to the folder
  it no work
  i am ripping models and animations from odst so i can put it in halo ce i know i could just rip everything out and just port it over
  but i am trying to put the animations on halo ce models instead
  so i thought renaming the skeleton of ce marines to that of odst marines for the animations would work then run some scripts to import it but idk what do you guys think?
  kinda wish there was a tutorial on this
- **2021-05-19** `williaso` [🦴╎animation] I definitely wanna do some more Titanfall-related stuff, (mostly more intros like I did) but also one of a spartan in first person jumping into the back of a pelican as it begins taking off and getting helped up by someone inside, like the Epilogue stuff in Titanfall multiplayer
- **2021-05-19** `supervigy` [📜╎ripping] My reclaimer won’t install so u need someone to rip my halo 4 multiplayer model
- **2021-05-23** `fiycsf` [🔍╎texturing-and-shading] If the two of you intend for it to be for reach, carefully swapping the substance project's workflow to something closer (the shader within substance will need to be changed too) will make it much simpler for you to get a result you're content with.
- **2021-05-29** `fiycsf` [🔍╎texturing-and-shading] Don't you even joke my dude. BT is best. 
  
  Titanfall 2 is probably the last game I played that felt truely complete. Good satisfying campaign, proper multiplayer, customization, no extra pre-order or paid dlc rubbed in the players face.
  
  I miss those days.
- **2021-05-29** `thelifeweaver` [🔍╎texturing-and-shading] I think it'll be a guessing game and hoping someone smarter than me figures out a base shader for it in unreal at somepoint later down the line.
- **2021-06-02** `surasia` [🔍╎texturing-and-shading] From R93_Sniper:
  halo 4 doesnt have a dedicated CC map
  it has a control map, and there's 2 types
  the one for Multiplayer spartans is as follows:
  R - Specular
  G - Diffuse
  B - Gloss
  A- Color Change
  for the Color Change, White is Primary Color, Grey is Secondary, Black is Uncolorable
- **2021-06-03** `muffintastic` [📜╎ripping] quite new to all this; where might i be able to find high quality texture(s) for the halo reach ghost, especially the overlay/detail texture? the ones i ripped using Reclaimer are only 1024x1024 (PC MCC)
  [attachments] MCC-Win64-Shipping_6lOVUiCReF.jpg
- **2021-06-03** `chromacore` [📜╎ripping] H2A ghost is the same as reach, you'll be able to pull a higher res normal map, but diffuse and detail textures you won't find
- **2021-06-03** `sculpted_stache` [🔍╎texturing-and-shading] I'm not sure about that, they seem to be putting alot of emphasis on the coating system, why make the coatings if most of your multiplayer maps are red and blue
- **2021-06-06** `ubald007` [💬╎general-help] I prob should have asked in ripping I just wanted to try out reclaimer I wanted to check out the new elites I dont even know if they are in H3 or flight
- **2021-06-15** `soulburnin` [🔍╎texturing-and-shading] H4 assets aren’t really something I use much so I haven’t dedicated much time to them
- **2021-06-19** `neckzilla` [📜╎ripping] https://youtu.be/iHxHPWdQr7k?t=2
  [embed] Parametric modifiers Performance:  3ds Max vs Blender vs Houdini https://www.youtube.com/watch?v=iHxHPWdQr7k&start=2 A comparission with a parametric modifier over a 3d scan in Houdini, blender and 3ds max. 
  The scan has 2M polys. 
  Plays fluid in 3dsmax (15 fps)
  Houdini around 5fps.
  Blender doesnt r
- **2021-06-23** `deg4397` [💬╎general-help] @swagster hey, i was wondering with your halo reach model blend file, for the emblems, how would one go about setting up those? I understand swapping out the mat in the shader but some of them appear messed up, and also not in one piece?
- **2021-06-26** `chiefster_4014` [📜╎ripping] the textures for the multiplayer spartans didn't look all that different from the in-game ones last i checked, other than slightly less compression and some not having alphas
- **2021-06-26** `surasia` [📜╎ripping] From R93_Sniper:
  halo 4 doesnt have a dedicated CC map
  it has a control map, and there's 2 types
  the one for Multiplayer spartans is as follows:
  R - Specular
  G - Diffuse
  B - Gloss
  A- Color Change
  for the Color Change, White is Primary Color, Grey is Secondary, Black is Uncolorable
- **2021-06-26** `soulburnin` [📜╎ripping] The app will have the shader info, or at least simplified versions of it, similar to the Destiny 3D viewer stuff. I haven’t kept up with the Destiny model server in a while, but apparently they can extract the shader info just fine.
- **2021-06-26** `athydraws` [📜╎ripping] Hey i'm trying to see if i can find the albedos (I've noticed albedos for other models tend to end in diff) for the multiplayer armors but all i can find are the controls and normals for them, really not sure what controls maps are but i'd really appreciate any pointers
- **2021-07-02** `dylm8` [💬╎general-help] Just wondering if I can rip my H3 Multiplayer spartan and use it in SFM using Reclaimer?
- **2021-07-03** `fiycsf` [📜╎ripping] Ripping from a game you can buy and pay for should be considered differently.
  
  You can buy the game, and artists and devs were paid for their work.
  
  When you take models from a mod like that, you are taking something you didn't pay for, that you were effectively granted a specific use for, for free.
  
  In most cases, if the author is cool with anyone playing with them, they just put them up somewhere if asked.
  
  Eg; https://sketchfab.com/3d-models/halo-reach-ma37-assault-rifle-c494ab2a4acb439ea626319aa7224b6c
  
  ***TLDR, I also wouldn't recommend.***
  [embed] Halo Reach - MA37 Assault Rifle - Download Free 3D model by Noe.Vah... https://sketchfab.com/3d-models/halo-reach-ma37-assault-rifle-c494ab2a4acb439ea626319aa7224b6c Faithful (I hope) recreation of the Halo Reach MA37 Assault Rifle
  Model (High and ‘Low’ poly) by: Soda (https://www.artstation.com/u65
- **2021-07-06** `r93_sniper` [📜╎ripping] Halo 4's multiplayer spartan control maps are RGBA Spec/Diffuse/Gloss/ColorChange
- **2021-07-07** `garfunkle3479` [📜╎ripping] One more thing to ask about Reclaimer and its compatibility with the H5 modules, are we still currently unable to pull files for the multiplayer armors or is that just a memory issue I'm currently experiencing? The model not only fails to load, but it doesnt even register. Even selecting *extract all* just creates an empty file folder.
- **2021-07-08** `hunter_ap` [🦴╎animation] Wanted to say thank you for that tutorial on getting animations out of the different Halo games, was something I was hoping would finally be doable after so many years of ripping assets from different games
  One question though: I'm getting errors when trying to extract some first person weapon animations, namely these from Reach:
  ```
  jmad -> objects\characters\spartans\fp\weapons\pistol\fp_magnum\fp_magnum
  jmad -> objects\characters\spartans\fp\weapons\support_high\fp_spartan_laser\fp_spartan_laser
  ```
  For the magnum, I get the error `Value cannot be null` for `Parameter name: source`, and for the splaser I get `Index was out of range` for `Parameter name: index`
  This seems to happen on any MP, FF, or SP map file I load
- **2021-07-16** `Average_Username` [💬╎general-help] so i've been trying to get bonobo working using the tutorial 
  but bonobo just refuses to open for me and i get this
  [embed] Thanks to @PlasmaGiant for taking the time to provide a brief rundown of ripping animations with Bonobo and porting them to Blender!
  
  "So what i found first was this program called Bonobo, an animation extractor program. It essentially reads the .map files for the Bungie Era games for you to either 
- **2021-07-18** `ember.cinder.ash` [💬╎general-help] in which case it's tied to an auth server, so disable your network and rollback the time in your BIOS
- **2021-07-20** `b1rb.` [📜╎ripping] does anyone have a guide for ripping the menu models with umodel? i want to rip the new odst stuff. but im not sure where its located. do i need to change these settings?
  [attachments] unknown.png
- **2021-07-20** `sswampp` [💬╎general-help] halo 2 anniversary multiplayer is not the same game as halo 2 anniversary singleplayer
- **2021-07-23** `_gallium` [💬╎general-help] With iGPUs VRAM is a little weird, since they have a dedicated amount (likely the 32mb mentioned) and are also able to allocate more based on how much regular RAM is free.
- **2021-07-28** `oragani` [📜╎ripping] So once the Infinite multiplayer beta is released tomorrow does anyone know if it’ll work with Reclaimer?
- **2021-07-30** `ember.cinder.ash` [💬╎general-help] ```
      TGA = TARGA. There's little reason to use this format now, unless you need compatibility with an app that only accepts TARGA
      TIFF = Tagged Image File Format. This is one of the most complex image formats, and it can hold more kinds and depth of information than almost any other format. The standard is owned and maintained by Adobe.
      PNG = Portable Network Graphics. This is an open standard. It's for losslessly compressing 24-bit+alpha graphics, often for the web. It's somewhat of a successor to GIF, but it does not support animation, at least not without extensions like APNG.
  ```
- **2021-08-10** `sswampp` [💬╎general-help] could use the mkv armor that's in multiplayer if you wanted something like classic armor?
- **2021-08-17** `plastered_crab` [💬╎general-help] But if you want animations too I can show you how to do that along with ripping the rig for the elite so you can actually move his body around
- **2021-08-23** `rcnfl` [📜╎ripping] so it's been a while since i ripped anything, but I just ripped Halo3/4 Elites. Anyone know what the spartan file name is for multiplayer models?
- **2021-08-29** `piratepenguin.` [💬╎general-help] Does anyone have the Guta animations from Reach? I've tried ripping them via bonobo, but I get an error every time I try just for the Guta, everything else rips just fine
- **2021-08-31** `benji3892` [💬╎general-help] Hello everyone, I’m really new to blender and I’m just wanting to use it for screenshots of my halo reach character. I have the file to open where you can select what pieces of armor you want on your character. So I got him the way I wanted and then deleted all of the other pieces and export him as a bpx file. When I try to import him on any other map that I have made he comes up textureless. Like I said I am ridiculously new, how I do go about either keeping the textures on him or applying the textures after importing? Or is that not possible and I need to just use the file where the creator is every time?
- **2021-09-09** `daglassman4797` [💬╎general-help] So I'm looking at the Halo 4 Multiplayer Spartans .blend file, and I was wondering what's a good way to remove the player logo on the shoulder?
- **2021-09-18** `malikamadhu` [💬╎general-help] Question for those who are more proficient in texturing: how would I go about getting the H3 mcc techsuit textures mapped to the h5 techsuit?
  I've tried shrink wrapping the h3 mesh to h5 and baking, but I keep running into either borked textures or a transparency glitch which corrupts baking anything on that file
- **2021-10-09** `aqua_himura` [📜╎ripping] mostly hoping for this since theres some things in hw2 that would not work with h3 animations
- **2021-10-13** `Deleted User` [💬╎general-help] So, I downloaded the halo 4 multiplayer spartans, yet I can only seem to access the Recruit model, can any point me to the otherS?
- **2021-10-13** `Deleted User` [💬╎general-help] https://drive.google.com/file/d/1I_5W9eiBMQfZH2AktsrsoQmG965CSU0S/view
  [embed] Halo 4 Multiplayer Spartans.zip https://drive.google.com/file/d/1I_5W9eiBMQfZH2AktsrsoQmG965CSU0S/view
- **2021-10-18** `unavailable1995` [📜╎ripping] do i have to grab the multiplayer or single player maps if i wanna open the seasonal armor?
- **2021-10-18** `unavailable1995` [📜╎ripping] i downloaded all the shared files that is needed and a multiplayer map just so i can open it, and when i look in search for mastercheif, i get nothing but i might be using a bad map
- **2021-10-19** `headphones1999` [💬╎general-help] Does anyone know the location where the multiplayer emblems are stored in MCC?
- **2021-10-19** `unavailable1995` [📜╎ripping] i've finally figured how to find the halo 3 multiplayer spartan model, but the selection is only the vanilla models i have not seen any of the season 8?
- **2021-10-19** `unavailable1995` [📜╎ripping] i found it after typing the tag master, and looked under the render model having 3 options of legs arms or full body with the parts i can check and uncheck and when i look into the parts for like the head it goes from base to odst.
- **2021-10-20** `itzhouze` [💬╎general-help] Hey there. I need some help. I exported the halo 3 multiplayer map: "warehouse" structure_bsp file to an AMF file using Reclaimer. Then i modified the geometry in blender. Now i exported the modified file as an ASS file. Whats the correct steps to get an structure_bsp file? i tried using tool=>structure, but after 5 minutes it just throws many errors, missing shaders and triangle errors and wont build. Hope you can help me out 🙂
- **2021-10-22** `itzhouze` [💬╎general-help] hello there. i need some help, getting a campaign to multiplayer map conversion done (Halo 3).
  
  my plan was to make a part of the first halo3 campaign map (010_jungle) playable as a multiplayer map.
  
  i changed the map id to the mp one, set the gametype to multiplayer, set the Zone Sets => "set_null" to the configuration of the right part ("set_path_c"), deleted all other unused zone sets, deleted the following sections: Globals, Scripting data, source files, scripts
  
  then i opened up the scenario in sapien and added a respawn point tag to the map.
  
  then i build a map file out of it via: "tool build-cache-file levels\solo\010_jungle\010_jungle"
  i noticed, that the console log said: "loaded script 'global_scripts'" and some other scripts... (maybe i forgot to remove some?)
  
  the build then succeeded, but when i tried to load it up in halo3, it throws a "Fatal Error!" and crashes.
  
  I hope somebody can help me out with this! 🙂
- **2021-10-22** `itzhouze` [💬╎general-help] Can someone help me out with the Sapien Error message: 
  -ERROR- spawning: Failed to find a valid spawn location!
  
  i tried converting a campaign map part to a multiplayer map. already added a "respawn_point", respawn zone and slayer_spawn using sapien
  
  but it wont spawn me!! 😦
- **2021-10-22** `itzhouze` [💬╎general-help] Hmm now i got my spawn working, but i got the damn FATAL ERROR when i convert it back to a .map and load it via multiplayer 😦
- **2021-10-29** `jkr3527` [💬╎general-help] Hi everyone! 
  A document in a google drive lead me to this server.
  I've been looking for the Halo Reach uncs marines, does anyone know where they are stored in the drive?
  
  Who ever made the drive, thank you so much for all your effort, it has been really helpful!
- **2021-10-29** `sswampp` [💬╎general-help] looks like it's not on the drive so I'll have to look at ripping it when I get to reuploading the reach assets
- **2021-11-05** `sswampp` [💬╎general-help] when ripping the sbsp you can extract most of the stuff you need (in H3 and later) with this button
- **2021-11-08** `tikala1` [💬╎general-help] Hey sorry to ping you after 5 days but did you solve your issue with the pink ODST?
- **2021-11-22** `refraynn` [📜╎ripping] I'm not gonna be doing much for now, but there will be a database of the different drives in a JSON file and then if you do, for example,
  
  ``r!reach <type>`` (Type being vehicle, weapons, and sound effects)
  
  It'll grab a link to the specific folder and send it in a message to you with some additional information.
  
  It'll be like that for all of the Halo games.
  
  Another part will be the documentation on ripping, such as 'How do I rip?', so you can do 
  
  ``r!docs ripping`` and it'll give you an embed with information on ripping.
- **2021-11-25** `sswampp` [📜╎ripping] https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-2-1
  [embed] fbx-sdk-2020-2-1 https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-2-1
- **2021-12-01** `xsoulstacex` [💬╎general-help] Hey all! I'm trying to use Reclaimer and Architect to extract the multiplayer spartan from Halo 3. I plan on unchecking all the armor pieces except for my spartans armor set so I can pose/animate him in blender for videos or thumbnails. My first hurdle atm is Reclaimer crashing every single time I try to save anything anywhere. I've been troubleshooting for hours and would appreciate any help 🙂
  
  I am trying to save as an obj extracting only the selected geometry, I installed Architect properly and followed the install guide to a T
- **2021-12-01** `xsoulstacex` [💬╎general-help] Thanks for pointing me the right way R93! I got my model in blender working fine. Do you know how I would now begin ripping the textures and applying those? I was told I'll have to make my own visor shader especially if I want a new age 343 animated visor
- **2021-12-01** `xsoulstacex` [💬╎general-help] Thanks 🙂 so your video covers getting the shaders, applying them to armor, making sure they match in game color options, and getting the back piece? I only ask because if this goes well with my Spartan, I will eventually want to be able to get my friends spartans as well with different everything. In that case I think I'd want to download your models and shaders provided and remove the excess, or is it still easier to rip them each? I've got ripping them figured out down pat now.
  
  I'll ignore armor effects for now but would you also happen to have a lead on getting the animated visors  textures? I can deal w default visor if I got everything else working
- **2021-12-01** `xsoulstacex` [💬╎general-help] Anyone have any leads for me on ripping / recreating the animated visors from halo 3? I was thinking that making a custom gradient texture (or whatever the visor looks like) and writing a shader to scroll through the texture's y coordinates would be doable, that's all that 343 are doing in engine anyways. Thing is I don't know how to do that in blender. Any visor texture would be a start, Reclaimer didn't give me one. I think I can ignore the back piece for now
- **2021-12-01** `xsoulstacex` [💬╎general-help] Any of my problems sound easy to solve to you? The incorrect bone mirroring has a video showing my struggle above here. I figured selecting armor colors or adding color to the visor should be easiest. I'm hoping the back piece will be as easy as ripping it from the game separately and gluing it on the back
- **2021-12-01** `flipdark95` [📜╎ripping] Nice, just wasn't sure about it because I don't recall seeing her character model in-game on the multiplayer yet.
- **2021-12-02** `bios_0307` [📜╎ripping] Oh btw, in some engines there’s an option to mark a texture as a normal map (Unity has it, for one). Typically that option tells the engine to repack the texture and drop one of the channels, and it later gets the third channel reconstructed. When you have a different texture packed into the normal map blue, it’ll end up dropping that texture completely. Something to keep in mind if anyone ends up making a Unity shader for this stuff.
- **2021-12-02** `xsoulstacex` [💬╎general-help] Think specular maps are worth using for the multiplayer spartan? They hardly make a difference but it's like the only piece of the puzzle I haven't used. Feel like I might as well throw it on
- **2021-12-02** `xsoulstacex` [🔍╎texturing-and-shading] I'm working on shading this multiplayer model I dug up. He came without a visor shader. Any thoughts on how to better replicate the animated visor? Basically just need that teal color with a purple bar I can animate using the texture coordinate/mapping node. Seems simple enough I'm just very new to shading like this
  [attachments] unknown.png, H3_TealDream_Visor_Icon.png, unknown.png
- **2021-12-03** `captnkoda` [💬╎general-help] Brand new here, where's the first port of call to extract multiplayer spartans from Halo 3?
- **2021-12-03** `xsoulstacex` [🔍╎texturing-and-shading] Is it possible to take my Blender shaders into Unity? New to texturing and did all my good texture work for a multiplayer spartan using nodes in blender. Used the Bungie Halo Shader in place of Principled BSDF so I could set the appropriate colors onto the armor. Because the diffuse maps came without color, I added it in blender. I just need to know how to get him looking the same in unity as he does in blender
- **2021-12-06** `stealthproductionz` [💬╎general-help] great question, i tried to install infinite campaign early and it only downloads the multiplayer, using xbox app because gamepass ultimate, is that supposed to happen or is something else goin on?
- **2021-12-07** `flipdark95` [🔍╎texturing-and-shading] @Grand Bacon Hey man, with your Infinite multiplayer shader, do I append the Nodetree or the entire material to use it?
- **2021-12-09** `Deleted User` [💬╎general-help] But I uninstalled the multiplayer and campaign on Xbox and now it shows the campaingn screen.
- **2021-12-10** `nanosyl` [💬╎general-help] @Surasia hey so it says everything is checked in the visibility box when i try to render some of the models in the multiplayer spartans but when i render the prosthetics and some other stuff they dont show up despite being checked
- **2021-12-11** `venator3d` [📜╎ripping] Are there any collections of ripped multiplayer assets of all the gear so far?
- **2021-12-14** `crashlegacy14` [🔍╎texturing-and-shading] actually, that's making me wonder what they do with the MKVI helmet and other that have flashlights on them ... wait, this is multiplayer models and you can't use the flashlight.
- **2021-12-15** `peestain37` [💬╎general-help] ive only messed with assembly in halo 2 to find out i can ruin multiplayer matches
- **2021-12-17** `brooen` [📜╎ripping] if ur ripping h4 ur prob gonna also want the shader from #🔗╎shaders-resources
- **2021-12-17** `thejudsub` [🔍╎texturing-and-shading] Yep. Thats exactly the thing im referencing with the blender parallax build.
  https://devtalk.blender.org/t/parallax-occlusion-mapping/15774
  [embed] Parallax Occlusion Mapping https://devtalk.blender.org/t/parallax-occlusion-mapping/15774 There has been interest in supporting parallax occlusion mapping (POM) in Blender for some time. See for example https://developer.blender.org/T68477, https://blender.community/c/rightclickselect/t9bbbc/ and Pa
- **2021-12-17** `the_heavynator` [🔍╎texturing-and-shading] its the halo 3 shader but a lot of the values related to water shaping like wave sharpness just dont do anything
- **2021-12-20** `gho57x90` [💬╎general-help] Initially, I was attempting to extract sound stuff from the Halo 4 network test beta, but after finding out it won't let me extract anything and attempting to extract from the Forge World map in Halo: Reach (MCC), I can't get it to work from there either.
- **2021-12-29** `makimaplushielabourgaming` [🦴╎animation] Thats probably just be making too much time for the elite to get back into its original pose, im not very great with looping animations atm
- **2021-12-30** `jeddyjedi` [🔍╎texturing-and-shading] This seems to be the place to ask, for the infinite multiplayer models blend file how would i make a Camo Pattern
- **2021-12-31** `john.h.blacksad` [💬╎general-help] Very new to ripping and I keep getting this error everytime I try to open a .map file from reach in reclaimer
  [attachments] unknown.png
- **2022-01-05** `tpg` [🔍╎texturing-and-shading] @Surasia fukin 343i changed workflows between campaign and multiplayer
- **2022-01-05** `brooen` [💬╎general-help] get the shader for the game you are ripping from in #🔗╎shaders-resources
- **2022-01-09** `crashlegacy14` [🔍╎texturing-and-shading] so I'm messing with models from another game and I'm running into packed textures labeled MRA and I can't figure out what the A stands for. The obvious choices - AO or Alpha - are unlikely as the same game also uses SRAO and O maps. The latter being dedicated alpha maps and often crop up in texture sets including MRA textures
- **2022-01-10** `theshenaniganizer` [💬╎general-help] Sooo
  Every time I've tried to get on Halo Infinite's multiplayer on my wifi, it says "No ping to our datacenters detected". My connection is perfect, resetting it does nothing, verifying game files does nothing, reinstalling my fucking network adapter driver does nothing, and reconfiguring my DNS settings does nothing. I could really use some help here, been bashing my head against a wall since release
- **2022-01-13** `sabsnake` [📜╎ripping] Unsure if I can ask this here or not, please no bannerino.
  Has anyone had success in Ripping Halo Reach Maps/MP Maps with Textures?
  ( If so, what tool did you use? ) I used Reclaimer and Adjutant and got 0 Textures, just Grey.
- **2022-01-14** `sculpted_stache` [🔍╎texturing-and-shading] you use a combination of a noise texture and mapping node to mix the main shader with a snow shader
- **2022-01-16** `speedeagle` [🔍╎texturing-and-shading] i doubt its the model, im just using a multiplayer spartan from halo infinite
- **2022-01-21** `theeldestboy` [🔍╎texturing-and-shading] https://www.artstation.com/artwork/xY31YY
  [embed] Halo Infinite - Wetness, Evan Liaw https://www.artstation.com/artwork/xY31YY Water and wetness was a huge team effort especially with a large Campaign and Multiplayer maps.  Huge thanks the Squirtle Squard.  Steve Carroll, Mahreen Fatima, and Mitchel Clayton from Campaign.  Stuart Cunningham for all
- **2022-01-26** `sswampp` [💬╎general-help] https://www.halopedia.org/Timberland#/media/File:Halo_Combat_Evolved-Timberland.jpg
  [embed] Timberland https://www.halopedia.org/Timberland Timberland is a multiplayer level originally exclusive to Halo: Combat Evolved for PC and Halo Mac. It was remade as Ridgeline in the Anniversary Map Pack for Halo: Reach and was later included in The Master Chief Collection.
- **2022-01-29** `deskclaw` [📜╎ripping] tbf there's probably dedicated tools for Bioshock on xentax or elsewhere
- **2022-01-30** `theishikawarin` [🔍╎texturing-and-shading] It's for colouring multiplayer spartans and elites like you see in-game, it's just a mask for the primary and secondary colours
- **2022-01-30** `acidmanrpgz` [📜╎ripping] His so how would one extract their multiplayer spartan from Halo Infinite (not sure if right channel)
- **2022-02-11** `williaso` [🔍╎texturing-and-shading] Now just plug your PNG into the corresponding color, if you want to adjust the scaling use a texture coordinate+mapping node, and if you want to change the color use a color ramp
- **2022-02-12** `phoondos` [📜╎ripping] idek if you can use the high def mode in multiplayer but the screenshots that i see of blood gulch look so much higher quality than what i managed to rip with reclaimer
- **2022-02-13** `sledgehammer_` [💬╎general-help] So I know this may be a stupid question, but is it possible to load maps into blender directly from a halo game? Like say for instance I wanted to make a blender file of one of infinite's multiplayer maps?
- **2022-02-18** `r93_sniper` [💬╎general-help] https://opencarnage.net/index.php?/topic/8479-edit-multiplayer-map-thumbnails-loading-screens-map-names/
- **2022-02-28** `coarforge` [📜╎ripping] Probably in the campaign module, unless they also get used in multiplayer. I haven't looked at campaign stuff yet because I don't have it
- **2022-03-13** `knitrex` [💬╎general-help] How do you hide the Red Spartan emblem on the left shoulder of the Halo 4 Multiplayer Spartans?
- **2022-03-13** `thejudsub` [📜╎ripping] can anyone point me to a tutorial for ripping things from H2A? And possibly how to rip maps from H3? If not halo3, then infinite is fine as long as i can get forerunner environment assets.
  I wanted to check out H2A ripping because i got the prophet model from the drive, but the UVs and weights were pretty messed up so i wanted to fix it up myself. And i need halo 3 stuff because i need the map where the prophet of truth tries to fire the rings in the ark.
  But im open to building the scene myself with infinite assets. I can figure out the shaders myself
- **2022-03-14** `urium86` [🔍╎texturing-and-shading] https://www.artstation.com/artwork/4X1Ren
  [embed] Halo Infinite : Character and Vehicle Materials, Rogelio Olguin https://www.artstation.com/artwork/4X1Ren I did more than what is here for the character a vehicle materials but these are the ones I liked.  Some were from source material edited.  The Surfacing team was dedicated to create materials s
- **2022-03-18** `shadoworcslayer` [🦴╎animation] I need to practice more. Make a few animations dedicated soley to walking cycles. Though walking cycles are the bane of my existence.
- **2022-04-12** `sswampp` [💬╎general-help] I must admit I've not touched ~~surasia's infinite packs yet~~ any infinite assets since the initial release of the multiplayer
- **2022-04-15** `omgwtfun` [💬╎general-help] I noticed I was getting it in Reach and 2AMultiplayer, and looking more into it, I seem to be getting worse performance everywhere
- **2022-04-16** `omgwtfun` [💬╎general-help] I asked this yesterday but it got kinda buried
  Has anyone else had a lower framerate for some MCC games since this patch?
  I haven't tested everything but for CE, Reach, and 2A multiplayer my frames are significantly reduced compared to before the patch
- **2022-04-16** `casual.lunatic.12420` [💬╎general-help] For me H2 is kind of slow. Tried running some H3-H4 multiplayer and got mad frame dips. I can hold 60 fps decent but it crashes into the ground any time I do something. Reach and CE were the only games that worked fine.
- **2022-04-19** `chromacore` [🔍╎texturing-and-shading] If your shader supports color, you can input the hex number in the typing slot above the color picker. Should be where all the sliders are inside the color selection.
- **2022-04-22** `nick930` [💬╎general-help] I'm afraid this archive doesn't have what I'm looking for. The only dialog in those files were multiplayer announcer soundbites. I'm looking for these specific quotes from the campaign
  
  "Giving the covenant back their bomb"
  
  "Chief get inside and gear up. We're taking this fight to the surface"
  
  "Don't make a girl a promise you know you can't keep"
  
  "Sir. Finishing this Fight"
  
  can't find any clean versions of these quotes without SFX/Music overlapping the voice recordings. Can anyone help with this?
- **2022-04-27** `fiycsf` [💬╎general-help] If you don't want to shell out for one with a dedicated GPU, make sure you get a ryzen system.
  
  Intel's integrated graphics are terrible at least, that's how they've been for over a decade.
  
  Might be changing, but I wouldn't trust it.
- **2022-05-01** `masterz1337` [💬╎general-help] Maybe next one will focus on netcode and more cross platform play… and fixing 60 FPS bugs
- **2022-05-03** `Deleted User` [📜╎ripping] https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-2-1
- **2022-05-04** `tpg` [📜╎ripping] I assume Season 2 props (like the new multiplayer menu) are still on the docket to be ripped?
- **2022-05-15** `thebigdaddy1518` [🦴╎rigging] Theres a server dedicated to getting models from Fortnite, they had the Moon Knight within a few hours of its release so they keep up to date pretty quick.
- **2022-05-19** `jadedqrow` [🔍╎texturing-and-shading] ```
  
                                                              --!Spec gloss blender Shader Tutorial!--
  
  
  
                                                                  *Make 2 principle BSDF shaders.
  
                                                 *1st Principled BSDF--Set everything 0 except alpha and connect albedo.
  
                                  *2nd Principled BSDF--Connect the colored spec map to the color and make specular and metallic 1.
  
                                    *2nd Principled BSDF--Connect glossy map to invert node, connect invert node to roughness.
  
                                     *2nd Principled BSDF--Connect normal map to normal on 2nd shader only! leave first empty.
  
                             *All--Make a Add Shader node, connect the 1st Principled to the first Slot, 2nd Principled to the 2nd Slot.
  
                              *All-- Connect add shader to Material output And It should work Just fine Edit invert on gloss if needed.
  
                                *2nd Principled BSDF--If you have to add Alpha to the material, Connect it to the 2nd Princ shader only.
  
  
                                           
  
  
  
                                                               --!New Shader Successfully Learned!--
  ```
                                                                                     ***IF you Have any questions, Ping me with them so i'll see them, otherwise I hope this Helps***
  
                                                         ...[cut]
- **2022-05-19** `mini_knox` [💬╎general-help] Wow that will be a dedicated project. May have to shelve this for a bit
- **2022-05-23** `silent1meow` [🔍╎texturing-and-shading] wip retexture of the H2A multiplayer shotgun
  always liked the design but the original textures kinda doo doo
  [attachments] unknown.png
- **2022-05-24** `cosmostune` [💬╎general-help] hey fellas, someone has the background for the multiplayer lobby ripped?
- **2022-05-27** `unggoy619` [🦴╎animation] Hey guys trying to use the child of constraint to have the ODST hold the SMG properly for animations but the hand on the grip seems to have this clipping issue. Would appreciate any help at all 🙂
  [attachments] Desktop_2022.05.27_-_22.50.07.01.mp4
- **2022-05-27** `ramiere` [🦴╎animation] To me it looks like the clipping only happens when the gun is in a position that the hand cant reach
- **2022-05-28** `elitecaden` [📜╎ripping] Has anyone ripped or somehow have the forge world multiplayer maps from OG Halo Reach?
- **2022-06-01** `keeeeeeeeen` [💬╎general-help] currently this requires putting the nma into the shader twice, once with a different UV map, hoping to find a way to contain this entirely in the shader nodegroup
- **2022-06-01** `plastered_crab` [📜╎ripping] So basic rundown on what to expect. The current ripping tools are only able to 
  1. Rip the higher poly models of Blue Team and Team Osiris but bad mesh normals
  2. Rip the multiplayer armors from Halo Waypoint but the models aren't as high poly as they should be
- **2022-06-04** `randomando2902` [💾╎autodesk] Hello there. New to the server! I'm an aspiring 3D artist working mainly in Maya. Looking to make some animations with Halo Reach assets. Where do I start?
- **2022-06-05** `gameoverload` [🦴╎animation] All I am saying is, if you want a more clear recording, given FPS desync, and possibly color desync, it might be better to record from the device itself.
- **2022-06-07** `silent1meow` [🔍╎texturing-and-shading] there's also this infinite master chief that includes a multiplayer version which removes the 117 and any other damage chief had
  https://discord.com/channels/819709630540021810/821676515099279374/973494436946382878
- **2022-06-07** `hex5582` [🦴╎rigging] Hello I was hoping someone would be able to help me out, I downloaded the halo reach civilians pack from Crab’s drive and I was trying to re rig them and give them an IK rig
- **2022-06-10** `forwarduntodawn` [💬╎general-help] I'd like to get my hands on the multiplayer emblems from the MCC, can anyone help with that?
- **2022-06-18** `kylemarkinson` [📜╎ripping] Huh, interesting. I would have thought there would have been a directory for assets that are used all throughout the game, such as Spartan armors and weapons that also appear in multiplayer
- **2022-06-26** `0rbz1` [💬╎general-help] Also, any way to get the infinite multiplayer maps into blender or not yet?
- **2022-07-09** `sierrad421` [🔍╎texturing-and-shading] Hey, quick question, and I dunno if I've asked it before, but can the textures and stuff for the Multiplayer Mark VI from the 1.0 pack be used on the 2.0 pack Master Chief? Thanks in advance.
- **2022-07-10** `nemesiswmd` [🔍╎texturing-and-shading] hi im new to the server and just downloaded the infinite spartans pack i exported masterchief as obj to put it in element 3d does anyone know if there is a way to export with textures when i try to retexture it in element everything is the wrong color or i might be putting it in the wrong place but im brand new to 3d so any advice will help thankyou
- **2022-07-11** `rafaleeater` [💬╎general-help] he asked how to do it in Unity, I would either write a shader to do it on a mesh or figure out how to do it using looping particles
- **2022-07-14** `omgwtfun` [💬╎general-help] Does anyone know of any good theory or design guides for making good maps? (either single player or multiplayer)
  Trying to relearn halos tools because I did like 2 things with them and I feel like I don't know what to do now design wise :')
- **2022-07-14** `frustrated` [💬╎general-help] Mfw you only want the odst romeo sharp shooter model ready to print and everyone's asking an arm and a leg to give it you 😭 idk how to do this file ripping stuff
- **2022-07-14** `gameoverload` [💬╎general-help] I thought the server / drive already had ODST stuff ripped at this point?
  So you wouldn't have even needed to rip the models, just use the ones here and modify them as need be.
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
- **2022-07-20** `tpg` [💬╎general-help] Ok that’s good at least
  
  I’d recommend following R93’s advice here, if you’re feeling lazy there’s a dedicated shader for Infinite stuff pinned in #🎮╎unity
- **2022-07-21** `plastered_crab` [📜╎ripping] Both Halo 2 classic and Anniversary Multiplayer maps can be ripped using Reclaimer
- **2022-07-22** `soulburnin` [🔍╎texturing-and-shading] Trying to get color values for stuff from H2A files is tripping me up hard
- **2022-07-22** `rustyfool` [💬╎general-help] so when I try and render the reach marines in the ghost town map it renders them with all the attachments
- **2022-07-25** `exporush` [🔍╎texturing-and-shading] So if I were to take a model from the model dump in Crabs drive, how would I go about texturing it properly? Thinking about grabbing the multiplayer data pad and texturing it.
- **2022-07-25** `.architekton` [🔍╎texturing-and-shading] Does anyone know how to get those black outlines? I believe it's with a shader but I haven't messed around with those b4. If so can u plz ping me letting me know what it's called or how to make one. Thank you
  [attachments] Screenshot_20220725-000925_YouTube.jpg
- **2022-08-01** `guhgunk` [📜╎ripping] hoping for some help with basic fp animation importing into blender. i've imported H3 chief fp arms and assault rifle into blender and am trying to import the ready anim. i get an error on anim import that "Node 'camera_control' not found in an existing armature". is there another model i need to bring into the scene from the map files? thanks!
- **2022-08-13** `caia.` [🔍╎texturing-and-shading] How exactly does the grunt illum shader work?
  
  -There doesn't seem to be a dedicated texture for it in the bitmaps folder
  -Guerilla says that it's using backpack.tif
  -The UVs (see pic) are in places that don't produce anything that make sense
  -Reclaimer shows circles and lines that appear to be textured and not generated.
  
  What gives?
  [attachments] 2022-08-13_08_12_27-Blender__C__Users_sunst_Documents_Development_Halo_Halo_2_objects_characters_gr.png
- **2022-08-14** `heavensc` [💬╎general-help] I think of the ODST cinematic specifically, where they start in the ship before dropping down.. Are cinematic only models generally accessible?
- **2022-08-14** `cloneboops` [💬╎general-help] also im using the models from another blender file i found of the multiplayer menu background. i think it was called "fireteam loading screen"
- **2022-08-15** `fiycsf` [💾╎substance-suite] You'd need to correct the softness first with blender knowledge. 
  
  That's not a substance thing.
  
  Game models look sharp when the have a low poly with normal map. 
  Low poly + normal map = close to high poly asset
  
  You are looking at the low poly without any of its maps or materials. They cannot be obtained for anything not in the PC multiplayer of halo 5. You will not find them for that character, unlike spartans.
- **2022-08-18** `chromacore` [💬╎general-help] actually most of halo 3's armors have it. All the multiplayer is tinted even more blue.
- **2022-08-22** `scrappitus` [💬╎general-help] https://drive.google.com/file/d/1tSywXvOqH8qhbGBeCQ2ChTKSHmQ2NJPD/view?usp=sharing can these be used for vrchat?
  [embed] Halo 4 Multiplayer Spartans.zip https://drive.google.com/file/d/1tSywXvOqH8qhbGBeCQ2ChTKSHmQ2NJPD/view?usp=sharing
- **2022-08-25** `plastered_crab` [💬╎general-help] Hello! I certainly do! I can upload the ones I have that I got from folks that are full color to be in a separate folder at some point today, so expect them to be done tomorrow (and DM or ping me if I forget) 🙂
- **2022-08-27** `fiycsf` [💬╎general-help] Fiy rant's story time;
  ```
  For example, for a non halo (Vroid software made model) I had someone want to pay 30$ for a hair colour change. Took me like, 15 minutes to get the exact kinda fade they wanted. But normally, when doing on commission work, I ask for that much (30$) for the hour. So I jumped through the hoops to force the model into VRC in a desktop medium performance rank.
  
  Uploaded it on my account, let em clone and try it so they'd have something until they figured it out, and said "here's all the files, you may have to ask in various servers public help channels to get it sorted in unity for yourself, but you have it cloned from me for now, so you shouldn't have to rush."
  
  And I figured, that'd be the end of that. I did far more than was asked, on the same cost.
  No, no it was not that simple. I'd check my phone to a dozen some messages many times over the next few days asking me alone *in DM's* how to setup the model in unity that I did in under an hour, and ended up getting so fed up with the constant questioning and them not doing as explained to setup the model, coupled with phrases/behavior like I still owed them something, that I packed up the model gave it to them, taking another bit of time outside of the constant messaging, just to get them to leave me be.
  ```
  I know stories like these aren't uncommon, commission work or not, among the friends I have that have been willing to spend time teaching or setting stuff up for some. And honestly, it burns out a c ...[cut]
- **2022-09-04** `gameoverload` [💬╎general-help] By tools, I simply mean any specified tools, not assets, that you wish for someone helping you to use.
  Other than that, things like do you want the models used to be accurate to Halo 3/ODST?
  In that case the person helping you will need a Halo 3/ODST shader to work with, which only two people here have done the research to recreate in Blender's node graph based shader system.
- **2022-09-06** `plastered_crab` [📜╎ripping] and I have a tutorial for ripping reach animations as well if you need that
- **2022-09-07** `deityeternal` [🔍╎texturing-and-shading] I need help finding the textures for the Weapon Racks in Halo Infinite to include the Weapon Pads in multiplayer.
- **2022-09-08** `blakjewellio` [💾╎autodesk] Ok so how do I solve the problem of the massive web of Riggs and the fact that all the Spartan gear seems to be overlapping on to each other
- **2022-09-08** `blakjewellio` [💾╎autodesk] are any of these packs combatable for maya or is there a server like this that has more maya friendly Halo files @Surasia:AI_supersad:
- **2022-09-10** `sierrad421` [🔍╎texturing-and-shading] I'm retexturing the energy sword in the blend file (the infinite shader one, blue), and what I'd like to know is how I can get the Mapping node to output "frame / 60" properly.
- **2022-09-11** `Deleted User` [💬╎general-help] Hello, I am looking for three models for blender: Halo Reach-> Assault rifle, Magnum, Mounted Turret. idk how this server works or who i need to talk to, but i would like some help
- **2022-09-12** `wraith0912` [📜╎ripping] so uh, quick question regarding stuff from SPV3
  if something isn't in the dedicated assets channel, is it safe to just go ahead and grab it or does SPV3 still fall under the "ripping from mods is no bueno" category?
- **2022-09-14** `zangh` [🦴╎animation] Opening shot of i04 trailer wip. I still have to fix the lasers' timing and the timing overall, but it's shaping up decently so far. I couldn't find a 3d model of a glassed reach so I  yoinked the picture that Blur did from Halopedia and then cut it out in photoshop.
  [attachments] i04_Trailer_Shot_1.mp4
- **2022-09-18** `agentlouisiana` [🦴╎animation] tbf you say this like the elite model in multiplayer isn't like horribly out of place like it isn't the CE model
- **2022-09-19** `Deleted User` [📜╎ripping] Is there any program that allows you to rip Halo 2 Anniversary multiplayer maps? I have index but does it only do campaign maps?
- **2022-09-19** `johnarceane` [🔍╎texturing-and-shading] Working on a Spartan render for a friend (so don't mind the cat ears :)) and hoping there's an easy solution: how can I change the colors of these bare metal plates in the Mk7 helmet? Runs on the assumption that they're all pathed together and can be modified as one zone, I think. I want to move the teal up to those plates to make it an accent color
  [attachments] unknown.png
- **2022-09-19** `palpythefrog` [🦴╎animation] the halo 2 classic elite model in campaign isnt the same model in h2 classic multiplayer? i thought it was
- **2022-09-19** `drakua` [🦴╎animation] I thought this was about comparing Campaign Elites to multiplayer Elites?
- **2022-10-06** `jaeger1648` [🔍╎texturing-and-shading] Hello, I was told my message might fit better over here
  
  I'm very new to blender and I'm using the reach spartan pack. I couldn't figure out how to export an fbx with any materials/textures at all. Actually the decals exported with it but that's it. Now I kinda know it's because I need to texture bake?
  I found a video on how to texture bake, but ever time I try to smart UV project it messes up the textures, shifts them around on the model a bit. It seems it's because it's not unwrapping right, but I have no clue how to fix it.
  
  I'm about to go to bed, but any help would be greatly appreciated!
- **2022-10-18** `cosmostune` [🔍╎texturing-and-shading] https://www.youtube.com/watch?v=EwCvEXkhfbU
  [embed] Blender 2.8 Texturing with Alpha Tutorial https://www.youtube.com/watch?v=EwCvEXkhfbU In this #Blender  2.8 video I show a #texturing technique with alpha masking. This means Im UV unwrapping a part of the mesh, the other UV island I map to transparent part and then I use the Alpha out put of the im
- **2022-10-22** `chalk33_8677` [💬╎general-help] So I've mostly got baking and such down, but having a weird issue. The Elite is in 4 parts, the arms, head, legs, and torso. Baking all at once because i need one image. But for some reason, it keeps skipping the head even though I've got it selected the same as everything else. Done it a couple times now, keeps happening. Any ideas?
- **2022-10-23** `chalk33_8677` [💬╎general-help] I was hoping to avoid manual editing for accuracy (especially with the texture file using grey, I'm used to just hue shifting in paint.net), but boy I didn't think armor color for these was gonna get this complicated lol
- **2022-10-25** `gameoverload` [🔍╎texturing-and-shading] You can get away with any usual image format in Unity, you will just have to make sure the shader you have does all the funnies ( either through actual code, Amplify or shader graph ) for mapping the channels for normals and that your metallic is mapped to the right channel of a texture.
- **2022-10-26** `mas2terchi0ef` [💬╎general-help] Hi I have a question is it possible to extract the H2A assault rifle files from multiplayer?
- **2022-10-26** `mas2terchi0ef` [📜╎ripping] Hi I have a question is it possible to extract the H2A assault rifle files from multiplayer?
- **2022-10-26** `chiefster_4014` [💬╎general-help] You can't use Index for H2A's multiplayer, it only works for the campaign. There's a new pack of H2A multiplayer weapons in #📂╎community-upload-download
- **2022-10-28** `elitecaden` [🦴╎animation] I really love that ending animation of the elite charging up then jumping again though
- **2022-10-29** `trystt` [💬╎general-help] Ah thanks! So it should just be a matter of applying them.
  So at that point there seems to be a texture issue. I can see the checkered patterns but something (maybe shaders) is keeping them as a deep brown color no matter what colors I use
- **2022-10-30** `mas2terchi0ef` [📜╎ripping] Hi I'm wondering how to create a collision model. I'm trying to make the assault rifle from H2A multiplayer play able in the campaign 
  I got the model and textures import already
  [attachments] Screenshot_6.png
- **2022-11-02** `gameoverload` [🔍╎texturing-and-shading] I am writing my own 'Practical Shader Compendium' on the side of things for the server, so technically both.
- **2022-11-03** `fable_law_458` [🔍╎texturing-and-shading] Hi, Casey. If you're looking to see what specific material that you're looking at in CE, Go in Sapien or Standalone. Open whatever level it may be, then, type into the console `debug_camera true`. What this will do is it will display the shader name and the directory on the top left of the screen on whatever shader you're looking at that it's using. 
  
  As far as pixel shading or vertex shading you mentioned about, I don't think you can view those inside the Halo: CE's engine. You may need to speak to someone on reclaimer's discord server that might know the answer to that question.
  
  I hope this does assist you with what you're wanting to do.
  
  Best regards,
  -fable
- **2022-11-04** `jadedqrow` [🔍╎texturing-and-shading] @Soul if you don’t mind the ping, was wondering if you have any feedback since if I remember correctly you made a game accurate shader
- **2022-11-06** `trystt` [💬╎general-help] Now that just leaves the matter of getting a 3D color model. The person I'm making armor for is hoping to have it be viewable in 3D without having to download Blender.
- **2022-11-07** `gameoverload` [💬╎general-help] The honest way for shading is honestly ripping apart game assets and putting them together accurately using as much information as you can on them as that will teach you the most on things, as it did me. As long as you have the right tools to view shader code or capture data.
  Basically, you will want to learn some stuff on HLSL and Assembly, which is what your effectively creating in a node graph. Unity in particularly even lets you see what your making in node graph as HLSL.
  I would give ya more of my personalized information, but I have yet to really start my 'Practical Shading Compendium' to a full extent to just hand ya that.
  Texture quality is really down to time taken and purpose, what is a texture doing. Plus if you want the most quality out of say simple BW masking or RGB channel packed masking for example, you would need a high poly model that can be a lot more polygonal specific on that masking, rather than a low poly model that texture was baked to be put on.
  Animations are a different story since your getting into rigging and literal animating, which is not my forte, and you would need someone else to really tell ya more.
- **2022-11-11** `hydranaught4444` [🦴╎animation] well the sentinel doesn't have a dedicated  up or down button and has to look up and down to go towards the ground of fly upward but i didn't want the legs to just point straight out when it looks down
- **2022-11-15** `gameoverload` [💬╎general-help] It needs as you see above, a base map, which is the RGB Diffuse you see named, as a white or base color map, with the alpha holding specular data if not held in the material texture.
  A normal.
  A color control properly masking the materials associated on the model.
  Illum, which is emission.
  Then your material texture, which holds specular data, albedo blending mapping, environmental contribution mapping, and roughness.
- **2022-11-18** `_gallium` [🔍╎texturing-and-shading] I had promising results upscaling normal maps with a network completely retrained on normal maps rather than general imagery.
  I do suspect that a complete system designed from the ground up for vector interpolation inference would be the best. (fuck me that was a string of words right there)
- **2022-11-18** `gameoverload` [🔍╎texturing-and-shading] Basically, we need ourselves a Halo Archive server dedicated to asset storage, and asset conversion or updating.
  Trained entirely to work with as much assets as we can throw at it like Normal Maps.
- **2022-11-21** `r93_sniper` [🦴╎animation] SFM really doesnt have anything that you cant do in a dedicated DCC app
- **2022-11-22** `thechipsmotherfuckendubbo` [💬╎general-help] Heyo, I wanna grab the Halo Reach Pelican for blender. We had one, but the googledrive is down. 
  
  https://discord.com/channels/819709630540021810/821676515099279374/824506529604173845
  
  Anyone able to help? Gimme a ping, would be legendary
- **2022-11-26** `egnbv` [💬╎general-help] i need halo 5 multiplayer armor 3d models, i how do extract it? pls help me, thanks 🙂
- **2022-11-30** `kalawaki` [📜╎ripping] Is there a program for ripping Halo Reach maps? Specifically looking to get them into Blender.
- **2022-12-10** `wuslomat` [💬╎general-help] I'm looking for the icons for the HALO 3 multiplayer maps.
  I'm looking for the small image files that appear when you scroll through the 27 maps (Guardian, Cold Storage, Waterfall, etc.).
  
  Can someone help me?
- **2022-12-15** `wraith0912` [🦴╎animation] the hunch-in is definitely an exaggeration compared to previous versions but each one has the elite's head more or less covered by the top of the ghost
- **2022-12-15** `r93_sniper` [🦴╎animation] actually the idea was always that you could snipe sn elite out of the ghost from the front
- **2022-12-19** `dadysherwin1` [💬╎general-help] Heyo, I'm working on a fan-made Halo game, but I'm unsure whether I'm allowed to use assets from Halo or not (helmets, multiplayer announcer lines, etc). I've had a read of the Game Content Usage Rules (linked), though it was a little unclear so I'm still not sure.
  
  Can someone clarify whether I'm allowed to use Halo assets or not? The project I'm working on is intended to be non-commercial.
  https://www.xbox.com/en-us/developers/rules
  [embed] Game Content Usage Rules | Xbox https://www.xbox.com/en-us/developers/rules Find out how you can use things like gameplay footage, screenshots, music, and other elements of our games to make things like  machinima, videos, and other cool things.
- **2022-12-22** `baconshelf` [💬╎general-help] https://www.halopedia.org/Map_file
  [embed] Map file https://www.halopedia.org/Map_file Map files, .map files or map cache files, are a file type employed within the Blam engine used by all first-person shooter Halo games. As their name suggests, they are the files which store data for campaign levels and maps for multiplayer, Firefight and S
- **2022-12-30** `tacohombre` [📜╎ripping] where are the different coatings for the multiplayer armours in infinite stored? I've looked under here but there isn't anything except the individual pieces
- **2023-01-07** `gameoverload` [🔍╎texturing-and-shading] That is one part I do get stuck on actually, properly ripping the area that a color resides in, specifically when I am doing Blender node stuff.
- **2023-01-09** `kelcken` [💬╎general-help] Trying to bake some textures for the Infinite Banished vehicles In blender using the ones uploaded here, and the Wraith, Ghost, and Chopper have all worked fine, but the Banshee is getting a blue tint and not baking  the emissives at all.
  
  I've looked at the shader setup (identical) as well as the bake settings (also identical) and I can't figure out what the issue is. I've also tried out several different influence settings, but none of them come out right on the banshee
  
  If anyone has any ideas, please let me know! Thank you in advance!
  
  First image is of the wraith (working as intended) and the second is the banshee (not working)
  
  Bake results in the bottom left corner.
  [attachments] RenderWraith.PNG, RenderBanshee.PNG
- **2023-01-11** `k1d650` [🦴╎rigging] If anyone could help it would be much appreciated.  I'm trying to pose my Reach spartan while following the only tutorial I could find.  The video seems to be specific to Infinite Spartans, as the rig for the Reach spartans looks different, and the commands they use don't seem to line up.  I'm brand new to Blender and was hoping someone could give me a hand.  Thanks!
- **2023-01-12** `clericg` [💬╎general-help] Does anyone know where the Halo CE multiplayer gun sounds are located in MCC? I wish to extract and change the pitch
- **2023-01-16** `.moonzen.` [📜╎ripping] is there a newer way of ripping reach animations? i used the video tutorial and it just keeps saying "no valid armature" and stuff like that when i import the jma. i think bonobos scuffed or something
- **2023-01-22** `plastered_crab` [📜╎ripping] to answer your question any of the multiplayer maps contain those models
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
- **2023-02-16** `priception` [🦴╎animation] Has anyone been able to rip animations from Halo 2 Anniversary Multiplayer?
- **2023-02-17** `priception` [📜╎ripping] Has there been any updates in being able to rip Halo 2 Anniversary Multiplayer animations or no?
- **2023-02-17** `jagerd274` [💬╎general-help] somebody here have been rip emblems form multiplayer?? The Emblem generator web dosent work at this moment, athleast for me
- **2023-02-25** `walpha` [💬╎general-help] I'm having trouble finding the multiplayer Spartan coatings using olympus. What do I select to pick out the coatings from?
- **2023-02-26** `wake9090` [💬╎general-help] yeah it says this in the description of the vid:
      - Zone 7 (Damage) enables the 7th zone in models, which is usually used to add additional detail to models. This remains unused on models such as multiplayer armor.
- **2023-03-01** `kombatkid13` [💬╎general-help] Does anyone know where the download pack for these files is? Its maps from HI multiplayer, and I remember downloading it from here, but I cant find the original upload + instructions on how to use them
  [attachments] image.png
- **2023-03-05** `gameoverload` [🔍╎texturing-and-shading] You'd need to have a vertically aligned gradient going from black to light blue ( green and blue at 1 ) in the color slots of the visor material section using a color ramp and a mapping node.
- **2023-03-08** `flipdark95` [📜╎ripping] Are there any recent rips of Halo Infinite available online somewhere? @Surasia's module unpacker doesn't seem to work anymore for the most recent update of Infinite. It keeps coming back with a "Infinite Parse" error when it tries to extract what I assume is the updated multiplayer module.
- **2023-03-10** `exporush` [🔍╎texturing-and-shading] Wraith, banshee, ghost and so on are of Sangheili design which is what the kraken and the 4 ships also are. So I assume their textures would blend well with them.
- **2023-03-14** `evactouch` [🔍╎texturing-and-shading] tried again in a unmodified spartanpack, same result. even tried removing color ramp, removing mapping and texture coordinates, and that didnt work either
- **2023-03-16** `ze_du_leite` [💬╎general-help] insane dedicated community, thanks for the help, reach was originally released in ENGLISH, I don't think it's on the drive, from what I've seen it has thousands of voices and files, having them all in different languages ​​would be crazy, anyway thanks a lot, it's going to be a huge help
- **2023-04-10** `stanlyhalo` [📜╎ripping] ah yeah, the models is just what I'm after... I develop my own stuff, and was wondering how infinite is able to achieve certain forerunner pieces that look like models... but could honestly be normal mapping, vertex displacement and/or parallax mapping (that one I doubt tho unless it's some h3 style holograms).
- **2023-04-19** `existential_selkath` [🦴╎animation] sorry for just jumping in here. Anyone know how to actually import halo 4 animation files into blender or anything? The JMA files are in h4-packs but I cant figure out how to do anything with them. All the tools seem to be built for anything older than H3
- **2023-04-27** `jammysnow` [💬╎general-help] What happened to the H5 multiplayer Spartans model pack in the asset packs section? I noticed a lot of packs are missing too, am I tweakin'?
- **2023-05-01** `hobbes240` [📜╎ripping] Hi, I'm new to this community and well applications like blender and all, however I'm understanding quickly as there are a lot of tutorials out there. One issue I am having that I need help with is ripping the animation for assassinations with Bonobo for Halo Reach then transferring it to Blender. I have seen the tutorial from PlasteredCrab, however one version of Bonobo is telling me that the Reach Files are unsupported, while the other is saying it can not find the file. Bonobo is working for Halo ODST, but isn't working for Halo 3 either. I'm uncertain if maybe the application is no longer working or what. I used this link for .map files https://haloce3.com/xmf/, downloaded all of the shared files for Reach, ODST, and 3. As for the .net downloads, this one works https://www.microsoft.com/en-us/download/details.aspx?id=48130, but this one doesnt https://www.microsoft.com/en-us/download/details.aspx?id=53344. Does Anyone know what the issue could be and a solution for it?
- **2023-05-07** `garfunkle3479` [📜╎ripping] Halo 5 Multiplayer Armor Textures
  https://mega.nz/#F!Iug1iR5B!gfSRJECq_kPJfbuIpXXnhw
  Halo 5 Multiplayer Armor Models
  https://mega.nz/#F!YnwnFRQK!tiDaKXGUGasxVuMtUd64hw
  Halo 5 Blue Team Single Player Textures
  https://mega.nz/#F!wnYnnTTI!g2x90aivZmeUlJmqgLs3Hg
  Halo 5 Fireteam Osiris Single Player Textures
  https://mega.nz/#F!0qZmjBTD!H54BF7hdj6oQYXVH4CzR3w
  Halo 5 Vehicles
  https://mega.nz/#F!JmQWhJDY!oBF6f3X4qBJGxqkKPg0VNg
  Halo 5 Weapons
  https://mega.nz/#F!B3YSmSJA!b7HyQDUQRc7rxVK678c_Lg!Q2w1mTwB
  
  @TacoHombre
  [embed] MEGA https://mega.nz/ MEGA provides free cloud storage with convenient and powerful always-on privacy. Claim your free 20GB now
- **2023-05-16** `dgenpoly` [💾╎autodesk] I've been trying to get Maya to fit easier into a Halo Mapping workflow, one of the many issues I have ran across is material names and prefixes. Coming from 3dsMax I was expecting a straight forward transition, for the most part it has. Today I ran into a material issue while assigning my sky material, when I go to name it "+sky" Maya removes the "+" automatically. Has anyone else ran into this and have a fix or a workaround?
  [attachments] image.png
- **2023-05-18** `chunch` [🔍╎texturing-and-shading] To be honest it’s best to use a dedicated toon shader rather than adapting the infinite shader and putting a toon shader at the end of the original shader
- **2023-05-24** `appoarsin` [💬╎general-help] I know this isn’t a modelling/animation thing I need help with, but I can’t seem to get help on the Halo server or from google with this. I started playing Reach’s campaign for the first time last week. It crashed halfway through the first mission and now I can’t open MCC.
  [attachments] IMG_0276.jpg
- **2023-05-28** `cyq3r` [🦴╎animation] I’m actually working on an odst story that takes place right after escaping reach on the pillar of autumn
- **2023-06-03** `elitecaden` [💬╎general-help] Does anyone know where you can download the multiplayer maps at for Halo Reach? Like Brick, Wayont, etc?
- **2023-06-12** `_gallium` [🔍╎texturing-and-shading] Also during a particularly critical point in VRC's community development, the shader did what was called 'light clamping' (It restricted the RGB values to the 0-1 range) which made it look completely wrong in any environment that used HDR lighting.
  
  _The problem with this_ was that the decision was driven by money.
- **2023-06-16** `wetmycherry` [📜╎ripping] Example in halo 4 slayer or any multiplayer mode. When you Lose or Win it'll show up on the screen once the game is done
- **2023-06-21** `guga666555` [📜╎ripping] Hey, just hopped into this server, i'm a modder making a Halo Mod for an different game [Call to Arms] and i need help extracting Halo Reach animations using bonobo, however it says reach maps are not supported once i try to open 'em, tho i remember ripping reach animations a time ago. I dont seem to understand whats happing with the application, so if anyone here does know a solution i'd be really grateful
- **2023-06-22** `.andythegreat` [💬╎general-help] hi there guys wanting to setup a haloce dedicated server and wanting to know where to get the files for the server and sapp and the auto dowload of custom maps.
- **2023-07-05** `ribbons0121r121` [💬╎general-help] Trying to join my friends party on mcc, steam
  Keeps throwing a generic error 
  Any ideas how to fix? All my network settings are default
- **2023-07-06** `thoro.blend` [💬╎general-help] transparent blood splatter images on google
  
  and there is an active camo shader somewhere on the server
- **2023-07-09** `hydranaught4444` [💬╎general-help] is there any way to export a shader from one blend file to another while keeping everything set up how it was
- **2023-07-09** `kuddlykraken` [🔍╎texturing-and-shading] I figured the Multiplayer Armor was like that if there was an armor/painting customization feature in the game
- **2023-07-13** `appearedcoast` [💬╎general-help] The one on the right is Chief from the Spartan pack and the one on the left is the multiplayer Chief
- **2023-07-13** `gameoverload` [🔍╎texturing-and-shading] Just mix in each decal and its respective mask together with a mix color node, making sure each pair of decal/mask isn't overlapping using a mapping node and uv coord node.
- **2023-07-13** `gameoverload` [🔍╎texturing-and-shading] @DaddehGhost So from the looks of it, even if you switch what shader the node is using, you will need to redo all of the connections and values, so I would suggest simply bringing in a new shader nodegroup node and transfering all the colors, values, and connections to that new node from the old one, this way you still have the old node as a backup for your customization data.
- **2023-07-15** `karimrojo` [💬╎general-help] Hi guys, I'm new to the server and I wanted to ask you, do you have the halo 5 multiplayer armor pack?
- **2023-07-19** `tpg` [🔍╎texturing-and-shading] Don't use Poi for Halo assets, just don't. There's a dedicated Halo 3 Shader here: https://canary.discord.com/channels/819709630540021810/821051966058004501/881146101632213032
- **2023-07-29** `red_wolfe` [📜╎ripping] I'm trying to extract the final race level from halo 3 but ngl I'm not sure I'm understanding the tutorial correctly. I checked for the map in the google drive but it seems it's only the multiplayer maps in there and I'm looking for that one specific campaign map
- **2023-07-30** `coarforge` [💬╎general-help] Since you have hybrid graphics in your laptop, I'm kinda suspecting those. Try forcing blender to use the dedicated graphics
- **2023-07-30** `sswampp` [💬╎general-help] contains step by step instructions to force applications to use your dedicated gpu in radeon settings
- **2023-07-31** `gameoverload` [💬╎general-help] Although how Reach uses height maps may be different in usecase, as it could just be using that height data for blending in the shading, you'd have to look to see what that height mapping is being used for via the mod tools.
- **2023-08-09** `gameoverload` [🔍╎texturing-and-shading] Up to you, a scar normals wise can just use an inverted bump node using the scar mask as the height, maybe with some noise multiplied in between to create some height noise.
  Then you would just use the scar mask to lerp between face color and scar color, same for lerping between face roughness and scar roughness.
- **2023-08-19** `810dude` [🔍╎texturing-and-shading] Anyone know what textures does a Halo 3 character expect? Like a multiplayer spartan or elite
- **2023-08-29** `liquidplazmid` [💬╎general-help] I was trying to port a reach spartan model into Unity to later put into vr chat. I was able to use reach shaders I found in the server for everything else. 
  
  The decals and shield needed a different setup though. I ended up using an FX shader and it looked fairly ok
- **2023-08-30** `elitecaden` [🦴╎animation] I think you should let the Scarab take a while to get set up after it landed. Kinda like how they do in Halo 3 The Covenant mission. Also I think it’ll be a bit better if the flapping of the cape caused by the falling scarab was a bit delayed since it’ll probably take a while for the wind from the scarab landing to reach
- **2023-09-01** `xisthename8168` [💬╎general-help] Hello, i'm having a problem importing a blender file from the halo 4 multiplayer pack to unity with textures to unity 2021.3.5f1
- **2023-10-03** `felixvyra` [🔍╎texturing-and-shading] Hey Guys.
  
  I haven't been on this server for some time as I have been doing some other stuff, and although this isn't related to Halo I have a bit of a texturing issue and I don't know a better place to ask for help.
  
  So. The last few days I have been trying to rip some assets from a game called Rainbow Six Siege. It's a long and complex process so I'm not going to go into it right now but TLDR: the guns have different skins and some of the "Lower quality *universal*" skins use a simple image that get's applied to all the guns, instead of having a different base color map for each one.
  My problem seems to be with the UV mapping. In the Images provided below you will see how the gun looks completely fine using the "no skin" base color map but If I try to apply the "Peacock" skin only SOME parts of the gun look correct.
  
  This is super weird and not something I have ever encountered before.
  I tried to look for some additional data that I might have missed but there is nothing. No extra information in the alpha or anything in the ripped files.
  
  Any help would be appreciated.
- **2023-10-14** `elitecaden` [🦴╎animation] I think the portal should stay in the scene a bit longer like you can see here https://youtu.be/CI3aD7VdKRI?si=gDrt4qZTHUCuAtkb
  [embed] The UNSC Infinity Shows Up In Style https://www.youtube.com/shorts/CI3aD7VdKRI This cutscene is actually amazing and when I first saw it my lizard brain went crazy. The frigates coming out of Infinity has to be my favourite part.
  
  #halo #haloinfinite #haloinfinitemultiplayer #haloinfinitecampaign #h
- **2023-10-16** `zidiath` [📜╎ripping] Am i crazy or are the multiplayer armor for Halo 5 not in the main modules?
- **2023-10-18** `dgenpoly` [📜╎ripping] Okay, what's the name of the CE tool, and what's the best workflow to batch extract H3 textures so mapping isn't 100% blind?
- **2023-10-20** `tacohombre` [📜╎ripping] what was the process for getting the model of the multiplayer spartan from 5?
- **2023-10-21** `gameoverload` [🔍╎texturing-and-shading] Except what you mean is HI is just doing one shader, then ( without seeing it as a reference in HIRT ) is slapping over another shader over that as needed, and swapping back and forth depending on rendering needs, often keeping on the other say for example in the customization menu when your flipping through customization items.
  
  Probably combining that all into a texture then the main actual referenced shader uses that data for maybe when your in a match or something...
- **2023-10-25** `evenmanatee887` [💬╎general-help] I remember there used to be a Multiplayer Mark VI pack someone made without the decal and you could just use those textures but that was for Pack 2
- **2023-10-26** `lubby94` [💬╎general-help] I managed to get the basics of exporting figured out, but I'm running into a couple of issues. 
  
  - I can't figure out how to import the textures properly into Unity. I got the spartan file from the old Delta Index server before it died, and the way textures are set up is the images are all alphas, and then you edit the colors in "zones," and I have no idea how to properly import the alphas with the color zones into Unity.
  
  - Half of the spartan's right arm is missing for some reason. I don't know if Unity is reading the faces as missing, reversed or what. I merged the armor on the body as one mesh and did the same with the helmet, maybe that messed up the mesh somehow. When I tested importing the spartan into Unity with all of the armor pieces as separate meshes, the arm issue seemed to be fixed.
  
  I'd really appreciate suggestions, but if it takes too long to explain how to fix these issues, that's totally fine. I can just research it online myself.
  [attachments] image.png, Blender__E__Pictures_VRChat_Avatars_Epsilon_Spartan_EpsilonSpartan_3.blend_10_26_2023_5_07_23_AM.png, Epsilon_Spartan_-_Untitled_-_PC_Mac__Linux_Standalone_-_Unity_2019.4.31f1_Personal___DX11__10_26_2023_5_06_23_AM.png, Blender_E__Pictures_VRChat_Avatars_Epsilon_Spartan_EpsilonSpartan_3.blend_10_26_2023_5_02_26_AM.png
- **2023-10-29** `evactouch` [💬╎general-help] @Plastered_CrawDAD hope you're fine with the ping, is there something im missing to get reach working? or is it still too early in development to really get anythin outta reach
- **2023-11-01** `plastered_crab` [📜╎ripping] getting halograms accurate to the games is more work, but slapping a regular hologram shader on it might make it look good enough depending on what you are doing
- **2023-11-03** `r93_sniper` [🔍╎texturing-and-shading] ```
    A secondary benefit of defending all users' freedom is that improvements made in alternate versions of the program, if they receive widespread use, become available for other developers to incorporate.  Many developers of free software are heartened and
  encouraged by the resulting cooperation.  However, in the case of software used on network servers, this result may fail to come about.
  The GNU General Public License permits making a modified version and letting the public access it on a server without ever releasing its source code to the public.```
- **2023-11-03** `r93_sniper` [🔍╎texturing-and-shading] the entire point of the shader's license was to prevent gatekeeping and to enforce GCUR compliance
- **2023-11-09** `itexturel` [💬╎general-help] Anyone know how to change any created model into a hologram/AI for blender and change the overall color while keeping the tones in tact?
- **2023-11-12** `dgenpoly` [💬╎general-help] Hello everyone, is there a tool I can use to batch extract bitmaps from HCE and H3? I've been figuring out mapping and H2 has the (halo2-color-plate-extractor) tool and it's made making a map for H2 so much easier. Is there a tool or even a workflow to batch extract bitmaps to a usable format?
- **2023-11-21** `r93_sniper` [🦴╎animation] hell, you couldnt even call claymation, stop-motion, or armature-based animation as "manual" animation because all of those have their own terms dedicated to them
- **2023-11-27** `ianstl` [🔍╎texturing-and-shading] Hey all! I'm seeing posts about the broken shaders on blender 4.0, so I'm wanting to port some older assets from this server to 4.0 using the newer inifinite shaders that are compatible. I know the basics of blender/3dmodeling, but I get a little lost in the sauce with the shader nodes. Could anyone help me out with this or point me in the right direction for a tutorial? Thanks!
- **2023-11-28** `vp_snipes` [🔍╎texturing-and-shading] The normal has the Gloss map stored in the Blue channel, and your "_r" texture is your reflection map that tints the color (IE specular color mapping), so treating that as a roughness map will NOT give you the proper results.
- **2023-11-30** `terminator33442` [📜╎ripping] How does the H2A model/texture quality fair between the campaign and the multiplayer? Is it worth ripping the campaign weapons?
- **2023-11-30** `r93_sniper` [📜╎ripping] for the weapons that are the same, campaign is spec/gloss whereas multiplayer is metallic/smooth
- **2023-11-30** `terminator33442` [📜╎ripping] Like, is it worth ripping the campaign models in place of the multiplayer ones (i.e. less gross modeling/normal issues)
- **2023-11-30** `r93_sniper` [📜╎ripping] if you're using for blender renders, the multiplayer ones require less work to get looking right
- **2023-12-13** `blue.nasca` [💬╎general-help] Silly blender question - if I wanted multiple Infinite Multiplayer Spartans in one scene how would I go about doing that and having their rigs and stuff self contained? Normally when I copy a spartan from one scene and paste it into another with an already active spartan it seems to mess with the rigs
- **2023-12-30** `spukuz` [💬╎general-help] Is there a way to add the glowy bits to the halo 5 armor from the multiplayer pack?
- **2024-01-04** `artfulwalnut7` [💬╎general-help] anyone know where to find the grass / rock textures for the landscapes in halo CE multiplayer maps?
- **2024-01-17** `dagnammit00` [📜╎ripping] Does anyone else have a problem with the cr4b tool when trying to import Halo 3 multiplayer spartan textures? I keep getting a message that says "bpy_prop_collection[key]:key "shader" not found"
- **2024-01-20** `theishikawarin` [📜╎ripping] this branch of Reclaimer keeps crashing when I try to export the Halo 5 Multiplayer Spartans as AMF and I can't open RMF in Blender... pain
- **2024-01-21** `theishikawarin` [🔍╎texturing-and-shading] @TacoHombre no luck extracting the Halo 5 multiplayer spartans, extracting as AMF either crashes Reclaimer or only comes out as 445kb, RMF is fine though for some reason
- **2024-01-21** `dagnammit00` [📜╎ripping] I'm trying to import the Halo 3 multiplayer spartan textures, and I keep getting a message that says "bpy_prop_collection[key]:key "shader" not found". Anyone got any ideas?
  [attachments] ineedhelp.png
- **2024-01-22** `theishikawarin` [📜╎ripping] I did try to open the Dedicated Server's modules with the ModuleExtractor but it just errored out
- **2024-01-23** `kerplefish` [💬╎general-help] Anyone know how I can export a Halo 3 multiplayer model from blender without losing the colors? I'm trying to bring it into unreal engine
  [attachments] Screenshot_2024-01-23_155347.png, Screenshot_2024-01-23_155537.png
- **2024-01-26** `lt.pelekito` [💬╎general-help] DOES ANYONE KNOW SUBSTANCE PAINTER? oops caps... and im not deleting that. My probelm is my ship is one mesh with a seperate mesh of guns but the textures are divided and in diffrent shades of colors after just dropping ONE color onto the mesh. Please help im pretty new to this program I only use it for decals.
  [attachments] image.png
- **2024-01-30** `bionicemperor` [💬╎general-help] Understandable, I just figured I’d reach out to all the servers I’m in with smart people and hope I can find something
- **2024-02-04** `101zeus101` [💬╎general-help] Hey out of curiosity does anyone have the map for the halo infinite multiplayer base location. The one from the tutorial section. Just had an awesome idea
- **2024-02-08** `inversefrog7689` [🔍╎texturing-and-shading] Does the H2A to Infinite converter work for campaign assets like the elites or only just the multiplayer Spartans?
- **2024-02-12** `trisckit` [💬╎general-help] if i duplicate the volume + change up shader settings there's some z fighting(?) and clipping in the effect
  [attachments] image.png
- **2024-02-13** `wraith0912` [🦴╎rigging] there's another server that's dedicated to helping with that tho, I think you might find more help over there
- **2024-02-17** `atlasguitar` [💬╎general-help] Hey there, I'm just starting out tinkering around with MCC a bit, and am hoping I can get some help or pointers here. I found the H2A Chief in H3 tags on Nexus and was able to get that all sorted in the H3 campaign, and would like to learn how to go about actually making those H3 tags. I was able to find Index and export the Arbiter, but I'm not really sure where to go from there - it looks like when importing to Blender bones and meshes aren't parented (is that the right phrase? lol) but I basically don't know what I'm doing, so if anyone can help me out at all that'd be great :)
- **2024-02-18** `purplmunkii` [🔍╎texturing-and-shading] You theres a few ways i can think of. I used to just make a new uve map and edit the uvs so the decal is where id like it to be. Make sure to set the image is set to clipping and plug in the new uv map in the shader
- **2024-02-21** `gameoverload` [🔍╎texturing-and-shading] Then that UV / texcoord channel I found must be for something else that needs its own dedicated UV channel..whatever that may be.
- **2024-02-22** `xansworkshop.com` [🔍╎texturing-and-shading] god this shader is gonna lag to shit in unity I think, so many sample calls
- **2024-02-22** `frostbitecinematics` [📜╎ripping] yeah i'll have to look into how to set it all up then, so far I've only been ripping Halo Reach assets but very much interested in making a public Infinite pack for UE5
- **2024-02-23** `pendragone01` [💬╎general-help] Hey all, does anyone know if it's possible to change the permutations available in Halo Reach's Foundation? I'd like to put a Spartan Helmet on a marine, but I'm wondering if that's possible without ripping it out and creating an entirely new tag.
- **2024-02-27** `lehefe` [💬╎general-help] Sup guys, was wondering if it's possible to read game data from system memory during a custom multiplayer match.
  I basically want to construct an application that can read in-game data from memory in real time, process that data, and then pass it along to a CEF frontend (Electron) which generates UI for the user and can further pass it along to a website (provided the user is authenticated).
  
  It's kinda-sorta like an autosplitter, but not for speedrunning and instead for multiplayer. I know that we can do this for campaign, but I can't locate anything which indicates it could be done for multiplayer matches. I'm concerned that Steam would just block MCC from running, even if EAC is disabled. I'm not building this for matchmaking. Just for custom games.
- **2024-02-27** `lehefe` [💬╎general-help] Reposting this:
  Sup guys, was wondering if it's possible to read game data from system memory during a custom multiplayer match.
  I basically want to construct an application that can read in-game data from memory in real time, process that data, and then pass it along to a CEF frontend (Electron) which generates UI for the user and can further pass it along to a website (provided the user is authenticated).
  
  It's kinda-sorta like an autosplitter, but not for speedrunning and instead for multiplayer. I know that we can do this for campaign, but I can't locate anything which indicates it could be done for multiplayer matches. I'm concerned that Steam would just block MCC from running, even if EAC is disabled. I'm not building this for matchmaking. Just for custom games.
  
  I just want to read memory, not write to it. The specific application for this would be to track player positions and lap times in Halo: Reach and Halo 4 races.
- **2024-02-28** `brooen` [🔍╎texturing-and-shading] https://cedeagler.gumroad.com/l/cegbj?layout=profile&recommended_by=library
  [embed] Blender interior mapping shader V2.0 https://cedeagler.gumroad.com/l/cegbj?layout=profile&recommended_by=library QoL Update: Now this shader should work with planes no matter its rotation.Thanks to Youtube user "@waelbeji" for letting me know the solution.You can either download the updated version 
- **2024-02-28** `the_friendly_spy` [💬╎general-help] Uh is there any way to get my multiplayer spartan from reach? ive been googling for like 2 hours now and i cant get an answer
- **2024-03-08** `freelancermontana` [💬╎general-help] Hey, how can I make this mesh to volume look more...like the vapour trail from a jet, and less...grey and smokey? Is there a way to color the volumes? Google isn't helping much.
  [attachments] image.png
- **2024-03-20** `sirsparrow` [💬╎general-help] Hey all, looking for an animator that can deal exclusively in animating faces and expressions, as I know its a very tough thing to work with, wanna have on dedicated animator for it. If you have interest as this is for quite a large project, please DM me? Could do with the help
- **2024-03-21** `big_squish` [🦴╎rigging] So I downloaded the Halo 4 multiplayer armor asset from below and it’s not weight painted, do I need to weight paint it myself because CATS isn’t working?
- **2024-03-25** `coarforge` [💬╎general-help] Hi, please use english in this server. You're also much more likely to get an answer that way.
  For your question (I think, I only know very little french), infinite for a lot of things only uses control maps for the materials, so you need a shader that uses those. Someone probably has made one for unreal you could use, or you can bake the textures in blender so that you get the usual textures
- **2024-04-04** `m_att.` [💬╎general-help] does anyone know how to port the Halo: Reach Anchor 9 Multiplayer map to Source Filmmaker?
- **2024-04-04** `m_att.` [💬╎general-help] are there any halo: reach multiplayer maps that have been ported to blender?
- **2024-04-04** `zophynoy` [💾╎autodesk] im not sure if it has, ill find out though. the specific h5 blue team as ik so im assuming it also has fk, however i also have the multiplayer spartan h4 blend file and that one does not even have controls
- **2024-04-28** `coarforge` [📜╎ripping] Since the campaign was separated from multiplayer, it doesn't have to use the same tag layouts anymore (and afaik it doesn't, it's an older version).
- **2024-04-28** `dustyoo10` [📜╎ripping] Does fallout 76 use the createabot skeleton for most robots? because unlike f4 there's no dedicated rigs that I can find for robots like protectrons
- **2024-04-29** `mechacommando` [💬╎general-help] This is a server dedicated to ripping Halo models. Just search “spartan pack” in #📂╎asset-upload-download-forum . I’m busy and I can’t get them for you right now.
- **2024-05-15** `freelancermontana` [🦴╎animation] Ugh I *love* the Halo 2A multiplayer shotgun, except for the lack of the side ejection port.
- **2024-05-21** `trisckit` [🦴╎animation] maybe it's just a hard thing in general unless you have a dedicated looping / long stretch of map
- **2024-05-25** `chiefster_4014` [💬╎general-help] go to the top-left and click File, Open Map, then select a .map file to open (campaign.map, multiplayer.map and shared.map cannot be opened.)
- **2024-06-12** `kn343._52282` [💬╎general-help] Is there any way to change the armor color with the Halo 4 Multiplayer Spartan pack?
- **2024-06-13** `kn343._52282` [💬╎general-help] Is there any way to change the armor color with the Halo 4 Multiplayer Spartan pack?
- **2024-06-16** `cythecy_` [🔍╎texturing-and-shading] Just got here after the server was suggested by NotPipi.
  
  I've been successfully pulling models through Reclaimer and it brings along the TIFs for the different image maps. I'm trying to port them into Blender using the AMF importer, successful, but I'm having issues figuring out how to properly set up the shader graphs for various models. CE seems relatively straight forward but later games have different detail maps and such and I just can't figure out how to get it done correctly. I did Boneyard from Reach and the ground looked completely wrong just mixing the different maps together with the correct scaling on each.
- **2024-06-25** `cythecy_` [🔍╎texturing-and-shading] Just want to double-check, I'm working with Reach assets in Blender.
  Can I assume that all the environment mapping for a single level would be the same?
  So same cubes, tints, roughness?
  I'm using the shader graph found in the Falcon asset, credit says @Soul made it
- **2024-06-28** `spartanjoe193` [💬╎general-help] Does the owner allow third party uploads? Gonna put the banshee with damage states as a separate fbx file along with dedicated folders for the garbage crates
- **2024-07-01** `nobletitan` [📜╎ripping] Has anyone been able to rip the campaign and multiplayer maps of Halo Wars and Halo Wars 2? I have downloaded all the files from the archive but I cannot seem to locate any meshes for maps, I can find bases, vehicles, weapons, even character models, but I cannot find meshes for maps. Are the meshes within the files or am I crazy?
- **2024-07-06** `dieofyes` [💬╎general-help] hi, i just got here, where would i go to download the halo: reach armor files to recreate multiplayer spartans in blender? if that even is a thing, i just got blender and im still figuring out how to use it. thanks
- **2024-07-09** `juliociraptor` [💬╎general-help] Hello everyone! I'm trying to import animations from the Jackal in Halo Reach into Blender, but I can't. Everything works fine with the Jackal from Halo 3, and I followed the tutorial here on the Discord server and in the YouTube video: 'https://www.youtube.com/watch?v=oIIHsOifz60&t=151s'."
  [embed] Halo Animation Ripping / Porting [Blender] https://www.youtube.com/watch?v=oIIHsOifz60 This video goes in-depth with the process of extracting Halo animations and porting them into Blender. I provide all the useful tools you need in order to do this as well as a .blend file with my Tactical Moa for 
- **2024-07-20** `eidod23` [📜╎ripping] does anyone got the skybox images for multiplayer maps i looked throught the google drive and no luck
- **2024-07-27** `gameoverload` [💬╎general-help] Characters being fairly simple ( except for multiplayer cause different studio ) and environments being a mix of simple and complex.
- **2024-07-28** `purplmunkii` [🦴╎rigging] Since theres a huge part in the back of the head that has no dedicated bone its struggling to parent them to the face
- **2024-08-06** `r93_sniper` [Osoyoos] GUI tool to assist in developing halo mods via the HEKs
  https://github.com/num0005/Osoyoos-Launcher
  [embed] GitHub - num0005/Osoyoos-Launcher: A W.I.P launcher aiming to suppo... https://github.com/num0005/Osoyoos-Launcher A W.I.P launcher aiming to support all current and future versions & revisions of the Halo Editing Kit. - num0005/Osoyoos-Launcher
- **2024-08-06** `r93_sniper` [Foundry] Blender toolset for Reach+ modding made by @Crisp 
  https://github.com/ILoveAGoodCrisp/Foundry
  [embed] GitHub - ILoveAGoodCrisp/Foundry: A Blender addon supporting asset ... https://github.com/ILoveAGoodCrisp/Foundry A Blender addon supporting asset creation for Halo Reach, Halo 4, and Halo 2 Anniversary Multiplayer - ILoveAGoodCrisp/Foundry
- **2024-08-09** `happymadman` [🔍╎texturing-and-shading] Working on a vehicle in Reach but I don't understand shaders beyond slapping on a diffuse map and calling it good enough
  [attachments] Screenshot_75.png
- **2024-08-23** `elitecaden` [🦴╎animation] You should look at reference of how a ship, especially one that big will move. A good reference I can think of right now is the cutscene of the Pillar of Autumn taking flight in this video: https://www.youtube.com/watch?v=QShcXPD7w2Q&ab_channel=LethalThreat. Also you should lower your Clip End value. You should have only as much as you need or things can start looking weird in your viewport
  [embed] Halo Reach: Pillar of Autumn- Helping Keyes Escape https://www.youtube.com/watch?v=QShcXPD7w2Q
- **2024-08-27** `sirsparrow` [💬╎general-help] Sad part is bud, you don't. There is no legit way unless you physically network or get connections already established in the industry to do that. I've already been going down that path with my own production.
  
  If it's about financials, it's very clear what we can and can't do in there
- **2024-09-03** `die_rise.` [💬╎general-help] Yo I need help with something if anyone know how to get flyable pelicans in multiplayer with anti cheat on that'll be amazing
- **2024-09-19** `cwallen` [💾╎substance-suite] any ideas as to why substance might be exporting a blank specular for only one of my texture sets on a model? they all come out fine and then there's just the one that's black. it still has the alpha though? its definitely working fine on the model in substance. just not keeping any info on the export. wondering if it's something related to the halo 3 shader? @JackOfChaos
  [attachments] image.png, image.png, image.png, image.png, image.png, image.png
- **2024-09-27** `frankr2994` [📜╎ripping] So I grabbed index today and I found my multiplayer maps are stored under the groundhog directory. Is index not able to see these?
- **2024-09-27** `chiefster_4014` [📜╎ripping] Index was made for H2A campaign's assets as they use a different graphics engine than the multiplayer. you need to use Reclaimer to rip the multiplayer maps.
- **2024-09-30** `chiefster_4014` [📜╎ripping] 1. There's #📑╎control-map-guide, though it only describes the main texture types. For the ones that aren't known you can figure them out by looking through the shader code (if it's available) or experimenting.
  
  2. I'm not sure, though I think that's the only one that was made. If you're just ripping from the games then Reclaimer should come with all you need.
  
  3. Shader code can be taken from MCC's mod tools (CE, H2, H3/ODST, HR, H4, H2AMP) or can be extracted from the game (CEA, H2ASP). For H5, Infinite and any others, it's currently not possible to get the shader code.
- **2024-10-01** `frankr2994` [📜╎ripping] Briefly looking I'm surprised at the amount of information and tools available for shader disassembly . 
  I really do need to stop going down rabbit holes though. I'm supposed to be setting guns up in unreal for a game mod that I already have ported in 😅. 
  
  At some point I will be more curious of the processes used in the various model ripping programs that we have for halo. I am noticing that with h2a and h4 any material using multiple textures or having a texture coordinate map will not pull over into blender. It would be truly awesome to get something sorted out to automatically pull some of the more advanced materials in at atleast a basic layout to aid in transfer to unreal for me. My material building knowledge is low so having a reference in blender is basically a requirement.
- **2024-10-16** `magoscustodi` [📜╎ripping] Where are the multiplayer chaos marines in Index? I know someone posted a pack already but I wanted to grab them myself. I can't find the models though
- **2024-10-19** `ska.robot` [💬╎general-help] chat is there a tutorial out there for uploading the [h4 multiplayer spartans](https://discord.com/channels/819709630540021810/958797327643082812/961665625795743765) to vrchat or am i on my own. i'm quite literally as dumb as a bag of rocks bro i can't do this on my own
- **2024-10-29** `zachafinackus` [🔍╎texturing-and-shading] Hello! I'm new to using Blender and was inspiried by bdoesstuff and some other Halo artists to give it a try. I've downloaded Surasia's Spartan Pack and thanks to Crab's videos I've done quite well in getting the armor set up and recolored to my preferences. 
  
  One question I did have for anyone familiar with the model, is how do I remove/lessen the dirt and grime texture on the armor that comes default? I can see where to change it's color, but I haven't been able to figure out how to tone it down a little. I think it does look good on the upper half of the model, but it's a bit much on the legs where they're almost entirely brown due to the dirt.
  
  Thank you for your time, and feel free to ping me if you can answer!
- **2024-11-05** `cythecy_` [🔍╎texturing-and-shading] So I know there's a fair few Blender shaders floating around in various places on this server, is there any particular one that people would recommend using for Reach assets?
- **2024-12-31** `papyrus95` [💬╎general-help] Hello, I haven't been using Blender for long, but I've been trying to learn using some of the models from this server. I have mainly been using it for renders, but recently I've been trying to use the texture painting feature that it has.
  
  I've only had a small amount of practice with it, but enough for it to work with some projects I've been working on, but I am stuck here. I am using the Halo Infinite Skirmisher that was uploaded by Palpy, and I am trying to make the blue regions on the legs match the techsuit/undersuit seen on the shoulders.
  
  The models UV seems to be unwrapped, and wasn't inside the image square, but dragging the mesh down didn't seem to implement the change I was looking for, despite the mask 0 seeming to have correct color zones.
  [attachments] image.png, image.png
- **2025-01-06** `solipsol_studios` [💬╎general-help] When it comes to making art with the assets from this server - what isnt allowed besides the obvious nsfw stuff?
  
  If I credit where all the assets and stuff came from in an art piece is that enough?  Ive never made stuff with things people have ripped before, just wanting to know what the rules are.
  
  (For context i just wanted to make a fan art of a halo streamer sitting in the drop pod bay like its the odst menu)
- **2025-01-22** `mrfelixvrc` [💬╎general-help] I need help with this, cause the pack im using is halo 5 multiplayer pack by taco, and for some reason it shows this in unity even though in blender it shows its rigged, with both game and spartan rig.
  [attachments] Capture.PNG
- **2025-01-30** `arcanwolfstudios` [💾╎substance-suite] honestly why i joined this server cuz im on a halo craze an i was like fuck it imma make avatars now XD
- **2025-02-06** `kozmetic` [🦴╎rigging] no, like i said, I dont even get that far, the way the H3 elite head is modeled has a ton of overlapping bits, so it just gives an error,ive tried eiditing the helmet but it doesnt work
- **2025-02-07** `drgoosethumb` [💬╎general-help] Been trying to add this poncho to my ODST, but I can't parent it to the rig without it also following the legs or going weird when I move the arms. How can I fix it to the model while keeping it more realistic in the way it moves?
  [attachments] 16.png
- **2025-02-17** `momosenpai` [💬╎general-help] Okay so I've decided on the armor I want to use for the spartan now what step comes next after working on the multiplayer spartan's armor?
  
  Do you export the model as an fbx to load it into a new blender file for a new scene?
  
  Trying to get an idea of the pipeline
  [attachments] image.png
- **2025-02-19** `gravemind2401` [📜╎ripping] there aren’t any anniversary graphics for multiplayer maps, only the usual `.map` files
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
- **2025-02-24** `strider_au` [🔍╎texturing-and-shading] does anybody know if there's a shader setup for assets from Anvil engine (i.e. Assassins Creed, Ghost Recon)? the Mask1 dds includes metallic, roughness and a mask for changing asset colors in-game, but the diffuse is just a detail mask and doesn't really do anything color wise. trying to save myself from a headache cause I haven't really found any good resources to point me in the right direction
  [attachments] image.png
- **2025-02-27** `gameoverload` [🔍╎texturing-and-shading] Genuinely, this just sounds like a bad idea, because then your wasting the use of the shader normals.
  ```
  Yes, this is a known limitation. When sampling lights we include how the light relates to the position and more importantly the normals of the mesh to help decide whether or not to sample the light.
  
  The normals we use are the normals prior to any normal mapping. Which means in your cube scene, we use the "smooth normals".
  
  This causes issue for your scene as the normals on the top of the cube "curve" around to the side of the cube, causing parts of the light to be considered "behind the surface" (because they're behind the plane formed from the smooth normals) and thus won't be sampled. This then leads to the directions and amount of light being sampled changing across the "flat surface" on the top of the cube and ultimately leads to the issues you observse.
  ```
- **2025-03-15** `wastelander.exe` [💬╎general-help] How can I remove the grime and scorch marks from the Infinite multiplayer Gen 3 Mark VI?
- **2025-03-20** `scalemaster117` [💬╎general-help] Hello group!
  
  Ekur users...with Ekur, I'm able to see 5,818 model objects from Halo Infinite. Is that the expected total from campaign, multiplayer and forge combined? If not I can't seem to access more than that even with the experimental and campaign checkboxes checked.
  
  All filenames are seemingly random numbers...not sure if that's intended or if there's a means of translating them to identifiable items. I may be doing it the hard way and seeing what each is individually.
- **2025-03-22** `scalemaster117` [📜╎ripping] If you figure out ho to get the Campaign objects to extract, please let me know. 🙂 I had the checkbox checked in Ekur but so far no Campaign objects to be found. Only Multiplayer and Forge. Not sure what I did wrong.
- **2025-03-22** `scalemaster117` [📜╎ripping] Thanks so much! I've renamed only 558 multiplayer/forge items out of 5,818 since I started last Sunday. I can't imagine how many more campaign will be!
- **2025-04-01** `scalemaster117` [📜╎ripping] I'm beginning to figure out what objects from Halo Infinite can be identified with Ekur. The extraction gives random names to all model files and I've been loading each and trying to identify them as best I can to find what I'm after. 
  
  There's at least about 12,000 models at a rough count, maybe more. And some duplication. This is my named extraction so far. It's the first 2,195 objects I've look at so far from areas of Campaign, Multiplayer, and Forge. I have several 'unknown' in there that may change name if I can identify more accurately later. 
  
  While the naming means something to me as far as finding things, I thought that it may help some others along if they are doing the same sort of activity looking for specific Infinite models.
  [attachments] HI_Camp_MP_Frg_Ekur_Filenames1.txt
- **2025-04-01** `mercurycae01` [💬╎general-help] i've been looking a little and i hate to ask, but where is a compiled and complete halo reach texture grouping?
- **2025-04-01** `mercurycae01` [💬╎general-help] there are more in the game! i specifically just want a ping that takes me to every reach texture period '
- **2025-04-01** `chiefster_4014` [📜╎ripping] for CE, you can use [General's Halo Toolset](https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master) to import the .gbxmodel tags from the mod tools directly into Blender
  For H2-ODST there are commands you can use with Tool to extract .jms files to import into Blender with the same addon. H2 has separate commands for render models, physics, collision and structures, while H3 & ODST you can just use ``extract-import-info``.
  For Reach, H4, and H2AMP, you can use [Crisp's Foundry addon](https://github.com/ILoveAGoodCrisp/Foundry) to import most tags directly into Blender, similarly to CE.
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account
- **2025-04-07** `alexvalkryie` [📜╎ripping] Anyone familiar with a method of ripping model and texture assets from Elite Dangerous?
- **2025-04-13** `scalemaster117` [📜╎ripping] Interesting find. I don't recall it on Streets but I haven't fine-tooth-combed that map myself yet. Just the usual screenshots and theater viewing. In Ekur at least, the Infinite multiplayer extract of bitmap assets numbers 17,686 files all randomly named with no folder structure to narrow the search to either civilian or even Streets. Needle in a haystack to say the least. There may be other apps that can do better that I'm not aware of.
- **2025-04-27** `scalemaster117` [📜╎ripping] Still working on the 3,180 items left in the campaign directories. Remaining multiplayer items is a whole other matter.
- **2025-05-11** `x_skel07` [📜╎ripping] I imported the Halo 2 Anniversary multiplayer model AMF generated from Reclaimer, and the male bodysuits have distorted deforms.
  Is there any way to fix this, or another way to extract it?
  [attachments] image.png
- **2025-05-14** `snapchipper` [🔍╎texturing-and-shading] Has anyone managed to mash HIMS into looking something like the Halo 3 shiny multiplayer spartan look? Would love some help if so
- **2025-05-28** `asbojack7h` [💬╎general-help] Does anyone know if theres a mod or just a way to replace Master Chief with the Multiplayer Spartan in Halo Infinite?
- **2025-06-03** `mkz67k90` [🔍╎texturing-and-shading] hoping this is the right spot ^^ im trying to use simplebake and uvpackmaster to atalas all my textures but i cant seem to figure out how to do my maskmaps (its basically my ao, metallics and roughness in one thing).
  
  if i just chuck it in the base color it comes out without the transparency, i couldnt find anything on the internet about it sadly😔
  [attachments] image.png
- **2025-06-16** `scalemaster117` [📜╎ripping] So if you export files out of Ekur, you'll only see the number string. You can then use my list to identify what you want and save yourself time opening the over 7,000 campaign objects. There's still over 15,000 more in the multiplayer/Forge area!
- **2025-06-17** `blue.nasca` [💬╎general-help] Gen 3 Mark 6 (multiplayer)'s ankles are incorrect, not sure if it's the diffuse or normal map or what but the techsuit looks wrong compared to earlier versions. Looks like the little circle pieces are missing.
- **2025-06-27** `scalemaster117` [📜╎ripping] Ekur Infinite campaign objects. 7,547 named, none remaining of consequence. The files not named are not showing up as objects as the file sizes are too small. This concludes the campaign bunch. There are over 15,000 multiplayer objects to figure out but that will be a project for another time. Took from mid Feb to end of Jun to just get through 7,000+ campaign items. 🙂
  [attachments] message.txt
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
- **2025-07-30** `nattramai` [💬╎general-help] I am but i managed to get a hold of expos elite pack from purples other server
- **2025-07-31** `x_crimsongalaxy_x` [💬╎general-help] how does one recolour the Reach Elites? the pack I have is titled "Elite IK chief elites v1.4", if it helps
  and I would like to know if it's possible to recolour them like one recolours the multiplayer Spartans, as the colour scheme I want is from my multiplayer Elite
  if it's not, is there a different model of Reach Elites available with an IK rig that would allow me to recolour it the same as my MP character?
  (NB: Infinite-Reach Elites are not a substitute I am after. I want straight up Reach rigs)
- **2025-08-18** `r93_sniper` [🦴╎animation] looks pretty good ngl. I'd probably only really suggest two things:
  1. See if you can implement the plasma wipe effect so that instead of it just being an on/off switch, it does the effect like Reach+ where it forms from the hilt. I know this is possible in blender as I've seen people do it
  2. Maybe have the dot particles much denser. Have more of them spawn, but keeping closer to the sword. Make it look like it's vaporizing the air around it
- **2025-08-23** `marcopolo1223` [💬╎general-help] Does anyone have experience baking the halo infinite multiplayer Spartans? Having issues where the control rig is preventing it from completing. Probably did something wrong I am following One Weird Guy's Youtube video and have no experience troubleshooting. Any help is appreciated, thanks!
- **2025-08-27** `stahl` [🔍╎texturing-and-shading] I am looking for someone to help me with applying textures to a Halo 3 Spartan.
  
  Wanting it to look like how I have it in Multiplayer (Paid)
- **2025-08-27** `chiefster_4014` [🔍╎texturing-and-shading] the latest version of Reclaimer is 2.1.1608. you are almost 3 years behind
  https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1608
  [embed] Release v2.1.1608 · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1608 1608 Patch
  
  Fixed some Reach workshop maps not working correctly
  Increased maximum batch worker count to 30
  Fixed H2V multiplayer maps not able to open any models
  Updated RMF importer to wor
- **2025-08-31** `djxsam23` [🔍╎texturing-and-shading] Hello! I've been playing around in Blender and in the FrostyEditor for ripping, "trying" rigging and texturing some Battlefield 4 asset and I was wondering what are the MM and D textures are for. I know that AO, CM and N are important for the 3d model but how usefull are the other 2? 
  
  Also I might need help with the texturing. At the moment I'm using the SWBF2_Materials_Version1.1 node graph in the Shader Editor in Blender to apply textures and I'd say it's okay. It does the job but I wonder if it would be better to do a custom one for Battlefield 4 instead?
  
  Thanks if someone can help me!
  [attachments] AO.png, CM.png, N.png, MM.png, D.png
- **2025-09-02** `chiefster_4014` [🔍╎texturing-and-shading] Check if it has overlapping or duplicate meshes. Also make sure the normal map is set to non-color
- **2025-09-04** `freedomenthusiast` [💬╎general-help] hey, guys, I'm looking to make a few custom ODST minis for tabletop sim for a Halo Mythic game, and was hoping someone that's made models like this before could give me some advice
- **2025-09-12** `vor4dorsraven` [📜╎ripping] anyone know if theres a way to get at multiplayer stuff from CEA/H2A? like maps and such
- **2025-09-16** `aliencraftman` [💬╎general-help] im either blind, havent looked hard enough, or it really just doesnt exist, but do we have the reach armor models from mcc mythic season 8 for blender? ive been scouring around the server trying to see if its anywhere to download for blender but ive had no luck
- **2025-09-19** `frankr2994` [📜╎ripping] I don't think you can. I think you're limited to H5 PC which isn't that just multiplayer only?
- **2025-10-09** `chiefster_4014` [💬╎general-help] I don't think so, for MCC everything should work just fine. If you see any issues like missing textures from any of the exclusive maps from Halo Custom Edition then you can try swapping the shared maps from the custom_edition folder to the maps folder.
  Also, if you didn't know already you can find what each level's filename is here: https://www.halopedia.org/Map_file
  [embed] Map file https://www.halopedia.org/Map_file Map files, .map files or map cache files, are a file type employed within the Blam engine used by all first-person shooter Halo games. As their name suggests, they are the files which store data for campaign levels and maps for multiplayer, Firefight and S
- **2025-10-15** `rippercr.` [🦴╎animation] Hi guys, Is there a way to extract H2A Multiplayer animations? so I can import them into Blender
- **2025-11-04** `kinar8785` [💬╎general-help] Hi everyone, is there any chance someone could re-upload the link for Halo 2 Anniversary Multiplayer Elites?😩
- **2025-11-06** `thatwotwfan2023` [💬╎general-help] So I got good news and 2 bad News
  
  Good new is I figured out how to get the Textures to work and now I can see the armour glory
  
  1st bad news, its the Wrong Textures im looking for. Im looking for the campaign Textures (with 117 on the chestplate) whiles this have the Multiplayer Textures. No offense to the person that extracted both Model and Texture. So im gonna keep searching for the Campaign Textures somewhere
  
  2nd and Final issue, Both Model and Textures are very High for the avatar im making in Vrchat (im trying to make it Quest + PC) so im gonna try to figure something out. 
  
  But thank you all for the help so far
  [attachments] image-144.png
- **2025-11-11** `thatwotwfan2023` [💬╎general-help] Since i do believe the Campaign and Multiplayer Chief Model are the same. Just need the Texture
- **2025-11-16** `flamekittenidunno` [💬╎general-help] I've been here for a while and haven't said anything here, I've been trying to find where the base model for the multiplayer armor in Halo Reach is in the google drive and just haven't find it. Could someone help me out?
- **2025-11-16** `scalemaster117` [💬╎general-help] Somewhat. I tried to figure out the basics of the campaign objects. I have yet to go through Multiplayer but it's far more extensive. Here's my list to date.
  [attachments] HI_Camp_Ekur_Filenames-7547.txt
- **2025-11-16** `scalemaster117` [💬╎general-help] You're welcome. I too started by file size. Pelican is at the top of the complexity list. 🙂 I'll eventually get to the multiplayer items but there were so much more than campaign, which was my main interest anyway. Those 7,500+ items (a few are multiplayer as I started there before I realized I could get campaign items too) took me from February to June this year to catalog. Multiplayer has 15,602 items in my list....so it'll take twice as long for me to catalog!
- **2025-11-24** `zattlebone` [💬╎general-help] Ekur is an addon made by surasia here, im not where the help forums are located so have a look round the server, searching for ekur (might be in #📰╎announcements )
  
  Once installed, you need yhe game installed too, then the options should be there all named, if its just strings of numbers, cylix is a great website with all halo infinite parts and the ids of each piece and colour coating to override
- **2025-11-24** `kozmetic` [💬╎general-help] I’m working on a halo infinite series right now, and in it I have animated fight scenes that I’m doing. The issue here, is that apparently with EKUR you can get multiplayer maps, but not the textures for them, so they’re just white. The animation is fine, I can handle that myself, but how would I do the background, because the camera moves around so green screening it would be incredibly difficult.
- **2025-11-24** `kozmetic` [📜╎ripping] I’m working on a halo infinite series right now, and in it I have animated fight scenes that I’m doing. The issue here, is that apparently with EKUR you can get multiplayer maps, but not the textures for them, so they’re just white. The animation is fine, I can handle that myself, but how would I do the background, because the camera moves around so green screening it would be incredibly difficult.
- **2025-11-25** `lytatroan` [📜╎ripping] I can't find the Halo 3 Gravemind tentacles model, how would I go with porting it to Blender?
  I was thinking of using the Reclaimer tool, I think I used it once to rip CEA's BSP's for a small render so I think it would be a good tool for ripping H3 models too.
- **2025-11-25** `lytatroan` [📜╎ripping] @swagster Can I have your Halo 3 shader? Ripping the Gravemind's model from H3
  [attachments] image.png
- **2025-11-30** `bandido_` [💬╎general-help] Hey, so I'm having a little trouble with the Halo 5 multiplayer spartans pack, is there a setting I have to adjust so that when I'm trying to pose it doesn't just spasm all around the place?
- **2025-12-02** `scrapcraft` [🦴╎animation] does anyone have the halo infinite animations for multiplayer for blender?
- **2025-12-04** `mantick8368` [💬╎general-help] Hi, does anyone know where I can get the PNGs of the Halo Infinite multiplayer emblems?
- **2025-12-06** `tristangold` [🦴╎animation] Hello! Had an animation question that's also part ripping question so please feel free to let me know if it should go into that section instead. 
  
  So I have two characters (non halo humanoids) that I have in my Blender scene and I was hoping to see if I could perform a simple assassination with - I think a Spartan v Spartan multiplayer reach one would be perfect. 
  
  I'm following along with the Plastered_CrawDAD tutorial however this seems to only apply to taking the Halo models and then through Bonobo and the other tools applying the Halo assassinations on top of them. Since my models aren't Halo related with a simple Mixamo skeleton, is it possible for me to retarget the animations or would the difference in skeletons make this impossible? 
  
  How I'm envisioning the process is A) Get 2 Halo Spartan Models, B) Apply the Assassin and Assassinated animations onto both of them. C) Retarget Animations onto secondary models once in blender properly. 
  
  Wanted to see if anyone has tried this yet
- **2025-12-06** `r93_sniper` [🦴╎animation] link to plugin for ref: https://github.com/ILoveAGoodCrisp/Foundry
  [embed] GitHub - ILoveAGoodCrisp/Foundry: A Blender addon supporting asset ... https://github.com/ILoveAGoodCrisp/Foundry A Blender addon supporting asset creation for Halo Reach, Halo 4, and Halo 2 Anniversary Multiplayer - ILoveAGoodCrisp/Foundry
- **2025-12-09** `tristangold` [📜╎ripping] quick ripping question - so I am trying to port some of the Halo Reach assassinations into blender via Foundry with the animation repository. First attempt to drag and drop Repo animation file into Blender failed and just want to understand exactly what I'm missing.
  
  1. For me to properly import the animation data - do I first need to extract it onto a spartan model first or can I rip the data and bones just directly into blender. 
  2. If I do need a model, are these findable in to Omaha directory or just easier to grab through the discord's already existing models. It's possible I've also been searching through the Model tools folder rather than Halo MCC itself, but navigating the files I've really just been coming across .hsc scripts.
  3. Is there a specific location I need to be dropping these animation files into on the Omaha folder? My drag and drop attempt spit back the error "Ensure your file is saved to your Omaha data directory"
  
  EDIT: poked around the discord some more so also gave a go at importing one of the animations through the .jms importer. Got an internal error noting the "action object has no attribute 'fcurves'
- **2025-12-10** `un_nonimus` [📜╎ripping] Has anyone figured out ripping the lightmap uv data and lightmaps for reach or any of the other games for that matter?
- **2025-12-13** `tjsauce` [💬╎general-help] if you mean Unreal, I posted an image of the shader chain I used to emulate classic cubemapping
- **2025-12-21** `gameoverload` [🔍╎texturing-and-shading] This tells ya how all that works in particular, minus the multiplayer stuff:
  https://www.adriancourreges.com/blog/2016/09/09/doom-2016-graphics-study/
  [embed] DOOM (2016) - Graphics Study - Adrian Courrèges https://www.adriancourreges.com/blog/2016/09/09/doom-2016-graphics-study/ DOOM pioneered fundamental changes in game design and mechanics back in 1993, it was a world-wide phenomenon which propelled to fame iconic figures …
- **2025-12-23** `koax117` [🔍╎texturing-and-shading] does anybody have a setup for the Halo 2 Anniversary Forge asset shader? 
  That or helping me identify the channels and what each is (CMB-Color-Metal-??) I need to make a cheat sheet for what every one of these texture maps do across all the games, I can NEVER keep it straight
  I want to say there MIGHT be a post somewhere that someone has it? but my brain is blanking
  [attachments] image.png
- **2025-12-25** `thatmadmoose` [🦴╎rigging] Setting up a rig for an elite and i noticed that the bones are very desynced from the mesh, is there a way to fix this?
  [attachments] Desktop_2025.12.25_-_09.22.33.19.mp4
- **2026-01-01** `r93_sniper` [🦴╎animation] why is the one elite just walking past them? lol
  Also, bro jumping back into the bay of the turret kinda feels off
- **2026-01-08** `koax117` [📜╎ripping] i was hoping doing this would result in me finding the textures which i could plug into the HIMS shader
- **2026-01-10** `gameoverload` [💬╎general-help] Welcome to poor resource management, not even on AI or anything, its because of what xbox requires of them, and microsoft on top of that, and the fact they originally went for a hero shooter only to pivot to open world + multiplayer.
  
  Its Halo Studios, yes. The 343i folks basically got let go, but that doesn't help when all the hires will likely get let go eventually, and xbox makes them still rely heavily on contractors over full time employees.
  
  Oh, side note, if you want info on all of the customization in Halo Infinite: https://cylix.guide/
  [embed] Cylix Guide https://cylix.guide/ Halo Infinite API Explorer
- **2026-01-10** `spooky1235` [💬╎general-help] hi is there a guide i can read or watch to change the colors for halo 5 multiplayer spartans, the explanation in the post itself flew waaay over my head
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
- **2026-01-11** `koax117` [💬╎general-help] one of the mods may want to give a proper answer on this, just as far as releases go (rather someone who is familiar with all the rules versus me vaguely understanding them)
  
   I hate gatekeeping as well, though I understand in some instances it is either warranted or necessary (a lot of personal work put in/legally can't). My mentality has been, if it is utilizing assets from the games directly, you have free reign to use and share them, as long as they follow the Microsoft guidelines for usage.
  Not 100% sure I'm reading it right, so clarification, you aren't intending to commercialize/profit off it, correct? I'd say as long as you're not, you're golden.
  
  If you're using anything that might of been made from scratch by someone and shared, like a custom sculpted model/texture, that would be the thing to generally check with them on.
  Beyond that, I think you're fine to share, and it'd be sick to have some like elite "boss" kinda vibes like that.
  
  as far as repainting the texture, to what extent are you planning? just colors to existing plates, or painting in new patterns and such?
  if it's the former, depending on what assets you utilize within your kitbash, that could be handled just within the shader settings. 
  but beyond that, substance painter/even photoshop would be my next thoughts
- **2026-01-18** `jackieboy` [💬╎general-help] Anyone help me with the Halo 5 Multiplayer armor pack? Seems like a lot of weight from the In-Game rig are missing.
  [attachments] image.png
- **2026-01-19** `exodst.` [📜╎ripping] thankyou so much for these, they are a godsend. I was just combining 3 detail maps by lerping between them with a factor of 0.5 but they looked wrong, thanks to you, this ground shader that uses 3 detail maps looks game accurate 🙂
  [attachments] image.png
- **2026-01-23** `runesoldiero` [💬╎general-help] Also are the MCC Reach models better, or should I try ripping into Reach itself?
- **2026-01-24** `runesoldiero` [📜╎ripping] Its not a concrete list at the moment, since I'm just getting into ripping, but ideally MCC and Infinite, (Reach if neither have some textures and models i want) Far cry 5, Disney Infinity (and possibly any other Disney game with Sugar Rush related assets) and maybe some N64 or PS2 games
- **2026-01-25** `.username_ladiesman217` [🦴╎animation] I'm going to do that while Unreal is downloading. I also want to make use of the metahumans there, learning a new software is not that hard, right?
  But one thing at a time, first I have to transfer a JMM Reach animation to one of Infinite's rigs.
  
  Thank you both for the advice, it's helping.
- **2026-02-16** `coarforge` [📜╎ripping] look in the network tab in the devtools, there you can get the links to the glbs
- **2026-02-27** `gameoverload` [📜╎ripping] So, H2A uses two different systems for its Singleplayer/Multiplayer.
  Singleplayer uses Saber3D.
  Multiplayer uses BLAM.
  
  So you need to go back to using Reclaimer for H2A MP, as it uses BLAM like usual.
- **2026-02-27** `mesathepostalgen` [📜╎ripping] i found the multiplayer elite in index though, but where would i find the map files in the directory then?
- **2026-03-04** `koax117` [💬╎general-help] As far as campaign goes, I haven't seen anything concrete out there.
  https://www.reddit.com/r/halo/comments/rfjfrf/unlike_the_pillar_of_autumn_halo_2s_cairo_station/
  ^shows a neat layout of Cairo.
  I know in some of the behind the scenes documentary videos, you can see whiteboard drawings of the map layouts in some parts. But that's WIP and not final in many instances.
  
  Multiplayer wise, this is a good reference
  https://steamcommunity.com/sharedfiles/filedetails/?l=schinese&id=2118066301
  
  that's all I can think of. Always find mission layouts cool, like this Halo 3 of Voi.
  https://www.youtube.com/watch?v=W64UxciD-gQ&t=28s
  [embed] Unlike the Pillar of Autumn, Halo 2's 'Cairo Station' level design ... https://www.reddit.com/r/halo/comments/rfjfrf/unlike_the_pillar_of_autumn_halo_2s_cairo_station/  Halo 3 - Portal at Voi - Surrounding Landscape Revealed https://www.youtube.com/watch?v=W64UxciD-gQ&start=28 Made in Blender with 3
- **2026-03-12** `purplmunkii` [💬╎general-help] This is the quickest way. A better way is using the decal input at the bottom of the shader and with a mapping node
  The best way would be to texture it in
- **2026-03-29** `archival_mind` [💬╎general-help] I don't know if this is the right place but I figured it might be considering people here have delved into the files before and may therefore have any idea about this one. This is a multiplayer track from Halo 5. Has anyone heard it before? I've covered so many bases and am running out of guesses.
  [attachments] 1064725949_online-audio-converter.com.mp3
- **2026-04-09** `notsnezzypaladin` [📜╎ripping] Could anyone rip the Halo Reach armor ability pucks fbx and textures for me? I've checked every where on multiple sites and servers and can't find anyone whose done it
- **2026-04-11** `mr_m1ller93` [💬╎general-help] Ok does someone have baked versions of the CE spartan multiplayer colors,including  master chief?
- **2026-04-28** `sharklordsatan` [💬╎general-help] Does anyone know of any dedicated Resident Evil model ripping/blender servers like this one? I swear there used to be a list of similar servers somewhere in this one but I dunno if I'm hallucinating it or not
- **2026-05-01** `papyrus95` [🔍╎texturing-and-shading] Hey there, I was wondering if anyone could help me figure out something regarding baking in Blender. I have retextured the Helldivers 2 MA5C using the shader converter found on @squeak_grenade (#1 SPI Fan) (Sorry for the ping) Halo Infinite Reach ODST Kitbash. I have already baked color, normal, roughness, and emission, but I was wondering how to bake a metallic map with this shader
- **2026-05-01** `aurathedutchangeldragon` [💬╎general-help] hey the forum chats of #📂╎asset-packs and #📂╎asset-upload-download-forum are broken for me (rubber banding back to top) so i was wondering if anyone had the multiplayer maps Blood Gulch, Coagulation, Hemorrhage or Bloodline?
- **2026-05-15** `red.wynd` [💬╎general-help] Sorry for such a noob question. I just joined the server. Wondering if anyone can point me to where I could find these menu and advertisement assets from Reach? 🙏 Any help is appreciated! I'll start digging through the many channels now 🫡
  [attachments] HaloReach_-_World_Cuisine_menu.png, image.png
- **2026-05-29** `koax117` [💬╎general-help] Characters, Weapons, and Vehicles have all been ripped. 
  Forge pallet is set up, somewhat. 
  Surasia is working on getting things going for multiplayer environments, but its slow going, so don't hold your breath
  (not knocking Surasia and Co. for speed, life happens, and its a lot of complex tedious work)
- **2026-06-03** `koax117` [💬╎general-help] i'm still working through piecing together a good system for doing looping animated textures, but this is the system I have going from a tutorial I followed, it might give you some ideas since big beam go brrrrr. Chiefster was also a lot of help with this because its running off their H3 shader and they cleaned up the loop a bit more.
  https://youtu.be/0uMWdlz1ecc?si=8TEIpJrL0cjYh3sf
  Based on what I've seen from working with the H3 assets at least, they usually have two different noise masks they animate. that'll potentially help give you a lot of that subnoise in the beam as well.
  [embed] Looping Texture Animations in Blender - Comfee Tips https://www.youtube.com/watch?v=0uMWdlz1ecc Animate any texture's location, rotation and/or scale in a seamless loop. Stylized water shimmer bonus tutorial as well!
  
  Psalm 23:2-3 "He makes me lie down in green pastures. He leads me by still waters.
  [attachments] 20260603-1944-52.0056414.mp4
- **2026-06-03** `roman117` [🔍╎texturing-and-shading] Been banging my head against this for two days: every tutorial i find to try and explain how this works requires either a "base color" node or a "principled BSDF" thing, which this model has neither of. And when I try using the uv unwrapping method where I select a rectangle on the chest where I want the number to be, it just puts a miniature version of the entire texture there.
  
  I tried the shrinkwrapping method and it *works* but the texture is way too bright and stands out way too obviously, and if I pose the model even a little bit the texture gets all warbled and misshapen.
- **2026-06-05** `chiefster_4014` [💬╎general-help] The addon isn't malicious. General101 simply named one of the variables in his code to ``this_is_a_horrible_hack``, probably because he knew it would stop working at some point, and it did.
  I'm not sure how to fix this error, but you might find more help in the dedicated Halo modding discord servers. You should be able to find them in #🤝╎partner-servers  with the names Halo Mods and Halo Modding Reclaimers
- **2026-06-12** `roman117` [🔍╎texturing-and-shading] Still banging my head against trying to add emblems -- might be one of the most infuriating projects I've done for something SO TINY.
  
  Found out about "color override", and I'm trying to use it. Two problems; the texture is upside down for some reason and the rotation options in the "mapping" node don't seem to be of much help. Also, the texture is mirrored on both sides of the chest. I don't know why it's doing that or how to make it stop.
  
  Also, is there a better way to move the image texture around that doesn't involve fiddling with the location and scale bars on the Mapping Node? It's so tedious trying to get it to fit where I want.
  
  Can anybody please help me? I've been stuck on this for almost two weeks and nothing I find on youtube or the server seems to help.
  [attachments] image.png
- **2026-06-19** `asrieldream` [🦴╎animation] someone at HS must be inspired by thishttps://youtu.be/_AxMlgPFxao?is=heS5UigLQhS-RLp1
  [embed] Halo (WIP)  - Opening Shot - Teaser Clip https://www.youtube.com/watch?v=_AxMlgPFxao #halo #fananimation #blender3d #halomcc #haloceanniversary #animation 
  
  The opening scene to Episode 1 of my Halo CE adaptation.
  
  The Pillar of Autumn emerges from slip-space after a blind jump, narrowly escaping th
- **2026-06-26** `gameoverload` [💬╎general-help] So the rig itself will work fine in that case, as long as you set up the humanoid bone mapping correctly in Unity, you just, in that case, need to bake out your textures from the shader, if that is what you need instead.
- **2026-06-27** `anthuvion` [💬╎general-help] i found files (what looks to be a complete installation) of halo infinite multiplayer from around july 2023 sitting around on an old drive and was wondering if this could be of any use to anyone
  [attachments] Screenshot_20260627_225048.png
- **2026-07-01** `koax117` [📜╎ripping] https://sounds.spriters-resource.com/xbox_360/halo3/asset/405487/
  It's in there, you just need to dive in
  [embed] Multiplayer Sound Effects - Halo 3 - Xbox 360 https://sounds.spriters-resource.com/xbox_360/halo3/asset/405487/
- **2026-07-10** `madmax1022v2_67408` [💬╎general-help] alr but if say i downloaded one of the packs and it does fully download is that software based or what because i tried to download the halo 5 multiplayer spartan pack and nothing happened
- **2026-07-12** `fancyneer` [🦴╎animation] Cascadeur was an in-house animation program for the studio maintaining it until they made it public for others to use. The AI tag is a bit of a misnomer and most likely just there to drive up speculation since it's just running off a neural network which has been trained by the studio's own mocap data they've accrued over the past decade. Tech like this has been around for a while, albeit in very experimental states and not as publicly available.
  
  As a beginner, you're doing yourself a disservice for using it as it automates a lot of stuff an animator should be able to do by hand. What are you gonna do when you don't have access to its toolkit in your project due to technical constraints?
- **2026-07-13** `purplmunkii` [🦴╎animation] as far as i know from my animation pals, cascadaur should be treated like mocap and not a replacement for hand keying.
  
  also mentioning its not too dis-similar to something like mixamo in the sence that even with a nueral network, to an animator or a keen eye all outputs are similar.
- **2026-07-27** `snowdoom` [💬╎general-help] hey quick question y'all, do you all have a simple way to fiix that? i'm trying to fit jega right boot onto a normal infinite elite and it keeps clipping, while the left foot have the exact adjustments and fits perfectly, any tips or help?
  [attachments] image.png
- **2026-07-28** `infiniteforges` [📜╎ripping] im building sierra 117 in ue5 and would love to connect with someone that knows how to rip assets so that i can get this level as high fidelity as possible. If youre open to helping, reach out. Thanks!
- **2026-07-30** `scavhyena` [🔍╎texturing-and-shading] Do we have any tutorials on putting decals with color on meshes that use the Halo Infinite style materials? I searched the whole server and all I could find was people asking. Wanted to put scorch marks on a model and paint blood stains on.
- **2026-08-04** `adorabirb` [📜╎ripping] https://youtu.be/Iu5zXB2JIwE?t=43
  [embed] Halo 5 Sound Effects | PART 16 | Halo CE Pistol Showcase https://www.youtube.com/watch?v=Iu5zXB2JIwE&start=43 This part is dedicated to the recently released Halo CE pistol.
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
- **2026-08-19** `bigbotprime` [📜╎ripping] Anyone have a link to game-accurate Halo 3 Multiplayer Spartans and color codes for the in-game customization
- **2026-09-12** `scalemaster117` [💬╎general-help] If it's of any help, I tried to identify as many pieces in the campaign as I could to cross-reference the numbers Ekur puts out. I didn't do much from the multiplayer as yet as there's literally double the amount of objects and this campaign batch of 7,547 items took me 3+ months of evenings to do.
  [attachments] HI_Camp_Ekur_Filenames-7547.txt
- **2026-09-13** `scalemaster117` [💬╎general-help] I installed multiplayer first as per Ekur's own instructions, and then did a re-dump of the campaign level objects afterwards. From Ekur:
  
  "Are campaign assets supported?
  Yes! However, you do need to set up its own data folder and tick the "Is Campaign" option in the addon preferences, and forge import options will become unavailable." - https://github.com/TheHaloArchive/ekur
  [embed] GitHub - TheHaloArchive/ekur: A multi-purpose blender importer for ... https://github.com/TheHaloArchive/ekur A multi-purpose blender importer for Halo Infinite. - TheHaloArchive/ekur
- **2026-09-17** `lieutenantfoxtrot` [💬╎general-help] Does anyone happen to know how to change the primary and secondary colours for the Halo 3 Multiplayer Spartan model?
- **2026-09-27** `fleetadmiral01` [📜╎ripping] Ripping STO models has some quirks to it. The only known means of doing it only exports to OBJ, so I'd have to completely rebuild the rig, And I'd have to manually color the armor/uniform pieces.
- **2026-09-27** `koax117` [🔍╎texturing-and-shading] Pretty sure there isn't an upload of Countdown with fully set up shaders in the server, that's why. not sure what upload you're referring to
  
  If you use Foundry with the mod tools, you can import in the levels with the shaders 90% set up to be game accurate, though some aspects like decals take further work.
  
  https://github.com/ILoveAGoodCrisp/Foundry
  [embed] GitHub - ILoveAGoodCrisp/Foundry: A Blender addon supporting asset ... https://github.com/ILoveAGoodCrisp/Foundry A Blender addon supporting asset creation for Halo Reach, Halo 4, and Halo 2 Anniversary Multiplayer - ILoveAGoodCrisp/Foundry
  [attachments] image.png
- **2026-10-02** `r93_sniper` [🦴╎animation] could probably have his torso come down more aggressively. If you see videos people landing after jumping off of stuff like small walls and the likes, you tend to bend your knees a lot more on impact than you think. That guy's flying up about his own height into the air before landing, that would definitely be one of those cases. If you feel that's _too_ aggressive, then you could probably reduce the shift by like half and argue its cus of ODST's exoskeletons in the armor (that is actually a thing btw!)
- **2026-10-18** `koax117` [💬╎general-help] I don't know about Halo 3, but I know for Reach their is this blender-addon that I used once, and got it to rip a map live
  https://github.com/Waffle1434/Mjolnir-Forge-Editor
  No idea if the Foundry addon has forge support or not, but works for porting the regular maps well. https://github.com/ILoveAGoodCrisp/Foundry
  [embed] GitHub - Waffle1434/Mjolnir-Forge-Editor: A Blender tool for manipu... https://github.com/Waffle1434/Mjolnir-Forge-Editor A Blender tool for manipulating in game objects in Halo Reach Forge in The Master Chief Collection. - Waffle1434/Mjolnir-Forge-Editor GitHub - ILoveAGoodCrisp/Foundry: A Blender 
- **2026-10-26** `irone26` [📜╎ripping] Has anyone ever been able to rip weapon models and sounds from H2A multiplayer?
