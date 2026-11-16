# rigging animation

Selected from 933 candidate messages (ranked by length, links, Halo relevance; capped at ~260 KB). Chronological. Channels: Extracting Halo Reach Anims, Halo 5 Animation Extractor, Halo Asset Blender Development Toolset, LinearAnimator, UGX Pipeline (Importer), Unreal Engine Blender Tools, 💬╎general-help, 💾╎autodesk, 💾╎substance-suite, 📜╎ripping, 🔍╎texturing-and-shading, 🦴╎animation, 🦴╎rigging

- **2021-03-22** `err_0r` [📜╎ripping] hmmm. That is good to know. Having variants is always good I argue, gives you more options to start from. I'm mostly focusing in the support drone right now because it's what's most pressing for my current project, for reference I mean this little guy floating alongside the odst team in this image: https://www.halopedia.org/MQ-96_Support_Drone#/media/File:ODST-Sunray_04.png
  [embed] MQ-96 Support Drone https://www.halopedia.org/MQ-96_Support_Drone The MQ-96 Support Drone is a drone known to be used by the crew of the UNSC Spirit of Fire. Equipped with light defensive measures, the drone is ideal for providing both cover and reconnaissance. The MQ-96 boasts impressive compatibil
- **2021-03-22** `err_0r` [📜╎ripping] Surasia, I haven't used the blender shader before, I do my shader work in unity, so if you could verify this for me: Is the reach shader block in the image that Brooen posted supposed to not be connected at all to the output? IT looks to me like the material isn't set up right
- **2021-03-22** `err_0r` [📜╎ripping] Yeah, Brooen, you connect the reach shader node right into the surface connector to the output and should be able to see changes, but you have to connect all of the maps that Surasia specied to the inputs on the reach shader node similar to how you connected the diffuse to the principled bdsf node
- **2021-04-20** `vinwarrior` [📜╎ripping] I think this genuinely boils down to semantic disagreements. Everybody seems to acknowledge that the Halo 5 assets have normal issues, right? And that they can be alleviated with modifiers and simple shader tweaks, which (obviously) can and should be shared for new people. What's the issue here? What am I missing?
- **2021-04-20** `fiycsf` [📜╎ripping] So this is a Halo 4 ODST that has been substanced, On the left side, the thing, the right, it in a half setup mirror.
  [attachments] VRChat_1920x1080_2020-04-05_06-48-51.png
- **2021-04-20** `nobleofdeath16` [🔍╎texturing-and-shading] Could probably pull the projectile from Reach and use that too. Given where the hand is suppose to sit , Spiker animations would place the hand always at the drum and scrunch the arm a bit up, causing some clipping
- **2021-04-21** `williaso` [📜╎ripping] Probably just leftover files someone forgot to get rid of since it's based on Halo 4's multiplayer, or maybe a rig dependency or something
- **2021-04-22** `err_0r` [🔍╎texturing-and-shading] It's good to see someone else who's trying to do right by the H4 ODST stuff. I reached the same conclusion you did about the striping/paintwork being the biggest issue. It doesn't *feel* odst to me, so I also brought it through substance to see if I could get something more acceptable out of it. Admittedly it's some of my older substance work so it's not quite as refined as some of my newer projects but none the less I'm relatively happy with the texturework I got out of it. 
  
  An interesting bit of the project that I've used in some of my newer stuff since is that this is what I call a hybrid texture, combining the original textures with my own. I imported the diffuse as well as grayscaled channels of the specularity/metallic/smoothness, and set it up so it looked as it did in game with shine and everything, and then used the original color code masks to set where my paintwork went. This way I got rid of the horrid stripe nonsense without having to worry about the thousand little details, keeping my layer count down and computer happy.
  [attachments] unknown.png
- **2021-04-22** `err_0r` [🔍╎texturing-and-shading] Now, as a more general question, I know that H2A stuff is a little hard to get a hold of, in particular I'm looking for weapons. I've gotten a hold of an H2A sniper rifle textures which seem to go onto the reach sniper mesh,  as well as the magnum posted here in the server. My issue is that I'm having a hell of a time trying to figure out how to make these things look right in unity. I'm in the process of trying to cobble together shaders for it using Amplify shader editor but I'm not sure what data is what when it comes to these textures.
  For both weapons I've got an RGB (diffuse/albedo) and an RGB (spec?), as well as the normal map which I've already made sure is converted to Unity specs. 
  My question is if anyone's got a good idea of how these textures end up making the end result in the game. If I use the 2nd rgb map as specular what I find is that some areas are much too dark while others are far too white, and of course there's no roughness or smoothness. I've seen plenty of halo texture sets stick smoothness/roughness in the alpha of the diffuse while unity tends to stick it in alpha of the spec or metallic map but in this case there's no alpha anywhere and none of my attempts at figuring out what data is what is giving me visually satisfactory results. Does anyone have any good ideas about what the weapon shaders from H2A look like, how they work?
- **2021-04-23** `holymosey` [💬╎general-help] can someone help me get newly added marines to move around on a campaign map in halo reach
  i'd prefer them to follow me until they die
  i got the marines in "exodus" right at the beginning but they just stand there and talk shit
- **2021-04-26** `sculpted_stache` [🔍╎texturing-and-shading] @swagster hey man, whenever I pose your port of the reach spartans, they have these weird artifacts
- **2021-04-26** `zippingrocks` [🔍╎texturing-and-shading] If you wouldn't mind sharin! I'll make sure you're one of the few who get the Halo Wars pack first. We're also going to use the animations from in-game for the elite! The Renders will be HOLY.
- **2021-04-30** `williaso` [💬╎general-help] https://www.youtube.com/watch?v=qMsqhMzEeBg You can do this
  [embed] What if Halo Multiplayer had intros like Titanfall? https://www.youtube.com/watch?v=qMsqhMzEeBg Lets call this a... proof of concept? I used a bunch of animations from Mixamo, with only a little bit of hand keying myself, so it's kind of rough.
  
  
  
  People in this video:
  Captain Jacob Keyes
  Spartan 1
  
- **2021-05-02** `alcidine` [🦴╎rigging] @swagster when you add ik to the reach spartans, will it be possible to parent the new skeleton to the preexisting objects?
- **2021-05-02** `gravemind2401` [📜╎ripping] you can get animations from halo 3/odst/reach xbox files using a tool called bonobo. not sure if it specifically works for cutscenes though
- **2021-05-04** `samu0621` [🦴╎rigging] 🤔 odd that youre having issues. I dont remember having issues with the "ranger" elite rig. https://cdn.discordapp.com/attachments/414984223901089792/727569452522143824/unknown.png
  [embed] https://cdn.discordapp.com/attachments/414984223901089792/727569452522143824/unknown.png
- **2021-05-06** `allindawoods` [💬╎general-help] "chief — 03/31/2021
  Halo: Reach Elites with all armors, pre-setup textures, rig and shading.
  https://drive.google.com/file/d/1lEKWe4WHANJD4DmsHLjIsyq9_XPRi6tC/view?usp=sharing"
  how do i export these elites with their materials??
  [embed] Reach Elites.zip https://drive.google.com/file/d/1lEKWe4WHANJD4DmsHLjIsyq9_XPRi6tC/view?usp=sharing%22
- **2021-05-08** `soulburnin` [💬╎general-help] If I remember right, you need to make a new (blank/black) image in the image editor, create an image texture node in the shader editor set to your new texture, then bake the diffuse, being sure to disable direct and indirect lighting (setting it to color only). Baking is available in the render settings tab
- **2021-05-09** `nocturnalsonder` [💬╎general-help] Yepp, and I havent tested if boneworks likes custom shaders but It worked fine with the Halo 2 MP shader so
- **2021-05-09** `chiefster_4014` [💬╎general-help] put the Alpha of the Diffuse into the Alpha input of the shader. then go to the Material Properties on the right, scroll down a bit until you see Viewport Display, and change the Blend Mode to Alpha Blend
  also, there's no need to use the invert node for the normal map, the shader inverts the normals on it's own.
- **2021-05-13** `nobleofdeath16` [🦴╎rigging] Its basically a backwards way that I think reach will allow the rig since it's the only way you can see it in Blender is if the file is imported in as a ame.
- **2021-05-14** `nobleofdeath16` [🔍╎texturing-and-shading] Would anyone in here be interested in doing the texture work for a Stanchion model? It would be for porting into Halo: Reach, and is a model provided by @SkookumProps. We are planning the separation of the parts right now for the nodes to go over the Sniper Rifle model.
- **2021-05-15** `surdavow` [🦴╎animation] Aight, I'll share something here. This is an animation featuring worts and all from March. https://youtu.be/2CqCECbulsE
  [embed] Distant Targets https://www.youtube.com/watch?v=2CqCECbulsE A Sangheili spots some targets from a distance, but constant intervention prevents him from just shooting, or perhaps he is just unwilling to with that Carbine???
  
  This project has been something I've been working on since the end of Januar
- **2021-05-15** `soi_7` [🦴╎animation] Infinite Elites seem to be in the right direction, but I personally think the peak Elite design hasn't been reached yet in the mainline games
- **2021-05-15** `soi_7` [🦴╎animation] https://twitter.com/AbiSV1/status/1358801256584646657?s=20
  [embed] https://twitter.com/AbiSV1/status/1358801256584646657?s=20 Halo 2 Elite (HD) short animation + turnaround
  
  This project was a collaborative effort between some members of the community, credit to:
  
  @Stu_FX for the wonderful animation work
  @Jafet_Meza for the awesome music
  @EzzellVFX for the floor te
- **2021-05-15** `surdavow` [🦴╎animation] Not necessarily animation but this is what I have done for my Energy sword shader. https://twitter.com/Devarture24/status/1343666291677122560
  [embed] https://twitter.com/Devarture24/status/1343666291677122560 This took some experimentation to look right, I might share it later once I'm satisfied with the results. It needs some more adjustments and sparks/smoke before that happens. https://t.co/ZiBmVus0OM
- **2021-05-17** `Deleted User` [🦴╎animation] hello all umm i need help ripping animation files from halo 3 odst.map files
  bonobo seems to extract it but when i go to the folder
  it no work
  i am ripping models and animations from odst so i can put it in halo ce i know i could just rip everything out and just port it over
  but i am trying to put the animations on halo ce models instead
  so i thought renaming the skeleton of ce marines to that of odst marines for the animations would work then run some scripts to import it but idk what do you guys think?
  kinda wish there was a tutorial on this
- **2021-05-20** `zad_47` [💬╎general-help] https://www.moddb.com/games/halo-2/downloads/bonobo-version-1003
  [embed] Bonobo [Version 1.0.0.3] Halo2/3/ODST/Reach Animation Extractor file https://www.moddb.com/games/halo-2/downloads/bonobo-version-1003 Bonobo [Version 1.0.0.3] is a Halo2/3/ODST/Reach Animation Extractor used to extract the animations from those 4 halo games, this can be found on halomaps.org, this c
- **2021-05-23** `nobleofdeath16` [🔍╎texturing-and-shading] reach doesnt like roughness, metallics or heights so its a process to figure out what will work together to make it look right,
- **2021-05-24** `pointdevice` [🔍╎texturing-and-shading] reach's shader is pbr, its cook torrance
  metallic and specular can both be pbr, and metallic gets converted to specular in the shader
  reach uses specular roughness, but the roughness isnt exposed to a texture, its just a flat scalar in the material properties
- **2021-05-26** `williaso` [🔍╎texturing-and-shading] How much work would it be to convert a Reach texture into PBR? Like, all the information is there, right, it's just a matter of mucking with it in Photoshop or something?
  
  Or maybe I just have no idea how textures work and that's not possible
- **2021-05-31** `surasia` [🔍╎texturing-and-shading] Reach's in game shader multiplies random stuff with each other so reverse engineering it is needed to make it look right
- **2021-06-01** `surasia` [📜╎ripping] https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] General-101/Halo-Asset-Blender-Development-Toolset https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2021-06-12** `Deleted User` [💬╎general-help] Well it's not really "needed" but apparently they were really cool and would have completed the dreamweb tagset a bit more than as we see it today but like I said not really needed but it supposedly had the the actor tag for the ranger elite that flys and a bunch of halo 3 weapons but no dice and I've asked all the og people of the post and they don't got it maybe masterz 1337 but that's a long shot and I've bothered enough people
- **2021-06-23** `fiycsf` [🔍╎texturing-and-shading] Going to assume your intended use case is VRC.
  
  to roughly repeate what I said to another asking about elites from KI2:```
  
  If you want to colour, like MP models for Spartans or Elites player models from 3/Reach/4/H2A/5, you'd need someone familiar with making something custom and compliant with colour modification via a control.
  
  This is something that some people can definitely do, but I wouldn't say it's a quick process. Would involve potentially manually painting out colour zones.
  
  (I'm capable of doing it, but the way I'd want to do it right would take long enough that well, I wouldn't do it for free personally.)
  ```
  Decimation on that scale, and then creation of a normal map would be required as well in this case, for the model to look sharp.
- **2021-06-23** `pointdevice` [🦴╎rigging] there are a way to get reach animations, and once you have them in an editor you should be able to put them in what ever format you with
- **2021-07-01** `plasmagiant` [💬╎general-help] Hey guys, I'm looking for the Jackal Walking animation from Reach. Is there a collection of FBX files that have them?
- **2021-07-05** `soulburnin` [💬╎general-help] @PointDevice @Deeto do either of you know anything about the "thinness" parameters in Reach? I'm trying to figure them out but can't quite seem to get it right.
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
- **2021-07-19** `sheevo.` [🦴╎rigging] can anyone explain to me how the rig on these works? https://drive.google.com/file/d/1PFXOflkJ8HYOddUL-zzsf2qDDUjMXgkx/view?usp=sharing
  [embed] Reach Spartans.zip https://drive.google.com/file/d/1PFXOflkJ8HYOddUL-zzsf2qDDUjMXgkx/view?usp=sharing
- **2021-07-21** `plastered_crab` [💬╎general-help] Tbh it would be much easier to learn if you extracted H3 chief with his animations and practiced with that instead of trying to attach animations to a ninja ripped model
- **2021-07-21** `sswampp` [💬╎general-help] I haven't messed with h3 first person animations since like 2011 (and never in blender, only max) but iirc the game handles a lot of this with nodes
- **2021-07-30** `ember.cinder.ash` [💬╎general-help] ```
      TGA = TARGA. There's little reason to use this format now, unless you need compatibility with an app that only accepts TARGA
      TIFF = Tagged Image File Format. This is one of the most complex image formats, and it can hold more kinds and depth of information than almost any other format. The standard is owned and maintained by Adobe.
      PNG = Portable Network Graphics. This is an open standard. It's for losslessly compressing 24-bit+alpha graphics, often for the web. It's somewhat of a successor to GIF, but it does not support animation, at least not without extensions like APNG.
  ```
- **2021-08-01** `piratepenguin.` [💬╎general-help] Sorry for late reply, but yeah Plastered's tutorial has been a god sent. I've was able to get it to work just fine for the reach model. Maybe it was I was using a wrong animation for the ODST rig. Don't know I'll have to test again when I have the ability too
- **2021-08-04** `oodleberry` [💬╎general-help] not sure without looking at your rig, XBXR made a reach spartan .blend with IK set up, you can grab it from the Halo Model Resource Discord: https://discord.com/channels/671183775454986240/682573027124445245/859410141309173760
- **2021-08-17** `plastered_crab` [💬╎general-help] But if you want animations too I can show you how to do that along with ripping the rig for the elite so you can actually move his body around
- **2021-08-24** `piratepenguin.` [📜╎ripping] Mkay, since anniversary uses reach models (for covenant) would it be possible to use anniversary's animation on reach's models?
- **2021-08-24** `sswampp` [📜╎ripping] some of them could work with minimal issues after a bit of work depending on what you're doing with them (h2 classic animations + reach models)
- **2021-08-29** `piratepenguin.` [💬╎general-help] Does anyone have the Guta animations from Reach? I've tried ripping them via bonobo, but I get an error every time I try just for the Guta, everything else rips just fine
- **2021-08-30** `rylietw` [💬╎general-help] I'm trying to import two animations from reach, specifically the scene in which Jorge sacrifices himself. Jorges animation is imported and as it should be, but whenever I try to import Noble Six's animations they always go onto jorge. is there a way to stop this from happening?
- **2021-08-30** `fesiug` [💬╎general-help] Trying to import the H3 first person AR along with the spartan first person arms and importing an animation on top will give me an error
- **2021-09-06** `rylietw` [💬╎general-help] Anyone know what is causing this? I'm 90% sure I've done the texturing right but in render view I get this. In material preview the Elite looks normal.
  [attachments] unknown.png
- **2021-09-17** `_gallium` [💬╎general-help] Prototype things like composition and scene layout with Eevee since it's nice and fast, do the final render with Cycles.
  
  Use OpenImageDenoise for cycles renders, it's slower than OptiX but better quality.
  
  Photography guides have a lot of good information, theory and more when it comes to framing shots since it's basically virtual photography.
  
  Make sure your monitors are properly (as best you can) colour calibrated, if not what might look nice and bright to you will be dark as fuck to others. Do test renders and display them on multiple devices to help check.
- **2021-09-23** `file1431` [🔍╎texturing-and-shading] The color of what will be the armor (you can say the light orange color) is composed of 3 variations of an orange, wear, dirt are made with smart materials and after that, we proceeded to make modifications so that the same are not geometrically perfect, then by masks add bases in what would be the channels that are not the color or base, the viewer is simply using the textures of Halo 5 just add one that I did where certain parts limit the relief and another reflection.
- **2021-09-26** `_lhc` [🔍╎texturing-and-shading] hmm, i can see where you're coming from. it honestly depends what kind of vibe you're going for. to me, it looks super gritty and realistic right now, and much more like metal, and not the plastic-y look h5 normally has. reach is a bit less grunge-y but again it depends what you're going for. i *really* like it, though
- **2021-09-26** `elitecaden` [🦴╎rigging] Hello, I'm trying to rig an elite model but not all of the parts of the model are sticking together for some reason https://cdn.discordapp.com/attachments/652002794894655518/891711870476615720/2021-09-26_10-43-55.mp4
  [embed] https://cdn.discordapp.com/attachments/652002794894655518/891711870476615720/2021-09-26_10-43-55.mp4
- **2021-09-26** `oodleberry` [🦴╎rigging] chief has all the reach elites set up in a blend here:https://discord.com/channels/819709630540021810/821676515099279374/826686044123430932
  however if you want to rig it yourself,  it looks like the parts you are having trouble with probably aren't assigned to the correct vertex groups
- **2021-10-05** `arithen` [🔍╎texturing-and-shading] Hello, I'm looking through the Halo Archive Drive, and I'm fining it incredibly difficult to attach the necessary textures to the models I have from that drive.
  
  At the moment, I'm just manually adding in each file, but I can't seem to find any of the visors from Halo Reach.
  
  Can someone point me in the right direction?
- **2021-10-05** `arithen` [🔍╎texturing-and-shading] Maybe I'm just understanding how they function. Yes, they are in there (as I just discovered), but the one I'm working on right now in blender is titled "Spartan_Visor_Kurt" and I don't see that specific material. 
  
  Nevertheless, the one you just linked is working, despite the name change.
  
  Would I need to use a color ramp node to decide the visor color then?
- **2021-10-14** `saltyace` [🦴╎rigging] So, I have *no idea* how to rig things, but I'm using the customizable reach file to create a vrchat avatar. I need to make the legs and spine straight to be compatible with full body, but the bones are twisted and when I straighten it one way, it gets wonky in other ways.
- **2021-10-15** `ubald007` [🦴╎rigging] I know the reach model doesn't have a sep under suit, in max you can copy and past the rigging info to said objects like helmets chest pieces
- **2021-10-15** `fiycsf` [🦴╎rigging] Ace, I'm not certain where you got the one you have, but I've poked the stock reach rig and well, it doesn't play nicely with VRC I'm afraid.
  
  It's why I tend to recommend my h5 skeleton over fighting through the rest.
- **2021-10-18** `chunch` [💬╎general-help] ||supposedly its just the reach mark V if I remember right, no such models are in the game's files (yet)||
- **2021-10-19** `takuat` [💬╎general-help] Heyo, I'm a complete beginner so I've probably done this out of order but I'm trying to hook up the 405th SPI on to H2A Undersuit to potentially put in H3. I've been able to cut it up into appropriate pieces and loosely position it correctly but none of it is attached to the rig yet yet, but I also haven't textured it yet. Is there a particular recommendation on which to do first?
  [attachments] unknown.png
- **2021-10-19** `sswampp` [💬╎general-help] https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2021-10-22** `itzhouze` [💬╎general-help] hello there. i need some help, getting a campaign to multiplayer map conversion done (Halo 3).
  
  my plan was to make a part of the first halo3 campaign map (010_jungle) playable as a multiplayer map.
  
  i changed the map id to the mp one, set the gametype to multiplayer, set the Zone Sets => "set_null" to the configuration of the right part ("set_path_c"), deleted all other unused zone sets, deleted the following sections: Globals, Scripting data, source files, scripts
  
  then i opened up the scenario in sapien and added a respawn point tag to the map.
  
  then i build a map file out of it via: "tool build-cache-file levels\solo\010_jungle\010_jungle"
  i noticed, that the console log said: "loaded script 'global_scripts'" and some other scripts... (maybe i forgot to remove some?)
  
  the build then succeeded, but when i tried to load it up in halo3, it throws a "Fatal Error!" and crashes.
  
  I hope somebody can help me out with this! 🙂
- **2021-10-26** `sierra060822` [🔍╎texturing-and-shading] Needed a semi worn suit for a render soon, so i retextured the h3 Mk 6 and H3 is always a pain for me to retexture for whatever reason but i THINK i got a good design and setup for my purpose, which is what's on the helmet alone right now just to test it
  [attachments] MKL64.png
- **2021-11-06** `arithen` [💬╎general-help] I'm trying to use Bonobo for the first time, but can't seem to find the correct Cache file to be able to actually view anything. what am I missing? (looking for Halo Reach elite animations
- **2021-11-07** `arithen` [💬╎general-help] Still can't figure out how to use Bonobo Properly. I can't seem to open any .map files, including the xbox ones. All I'm looking for is an Elite walking animation from Reach
- **2021-11-30** `unordinal` [💬╎general-help] Right, I forgot about albedo blend - I'm a bit confused still when it comes to that though, as elite armor specifically has a tertiary color that is not in the color map at all and yet applies to the fresnel tint.
  
  Actually, now that I type that out, does albedo blend set to one change it to ignore the CC map?
- **2021-11-30** `unordinal` [💬╎general-help] But I'm still confused on where the fresnel color is taken from the tertiary color. Lemme show an example: the Elite SpecOps has a green fresnel, and that same color is listed in the color change block under slot 2, despite there being no blue in the elite color control map, so the fresnel color in the shader must be passed that color from the color change block in some way, right?
  
  Maybe I'm just not fully understanding so I'll play around a bit and re-read your visualization a few times
  [attachments] unknown.png, unknown.png
- **2021-12-01** `xsoulstacex` [💬╎general-help] Hey all! I'm trying to use Reclaimer and Architect to extract the multiplayer spartan from Halo 3. I plan on unchecking all the armor pieces except for my spartans armor set so I can pose/animate him in blender for videos or thumbnails. My first hurdle atm is Reclaimer crashing every single time I try to save anything anywhere. I've been troubleshooting for hours and would appreciate any help 🙂
  
  I am trying to save as an obj extracting only the selected geometry, I installed Architect properly and followed the install guide to a T
- **2021-12-01** `xsoulstacex` [💬╎general-help] Thanks for pointing me the right way R93! I got my model in blender working fine. Do you know how I would now begin ripping the textures and applying those? I was told I'll have to make my own visor shader especially if I want a new age 343 animated visor
- **2021-12-01** `elitecaden` [🦴╎rigging] > How does one IK Rig a Reach Elite?
  @Haken Studios  I’ve been following this tutorial: https://youtu.be/Pt3-mHBCoQk. I’ve only done the legs so far
  [embed] Blender 2.8 Inverse Kinematics In 2 Minutes! https://www.youtube.com/watch?v=Pt3-mHBCoQk Hey everyone! Today I'll be showing you how to create inverse kinematics for your character in Blender 2.8 in 2 minutes!
  
  If you enjoyed this video, please don't forget to like and subscribe for more content!
  Yo
- **2021-12-01** `hakenstudios` [🦴╎rigging] Do you mind uploading the IK rigged reach elites when you finish it? I can’t get it to work cause of the horse link.
- **2021-12-01** `xsoulstacex` [💬╎general-help] Any of my problems sound easy to solve to you? The incorrect bone mirroring has a video showing my struggle above here. I figured selecting armor colors or adding color to the visor should be easiest. I'm hoping the back piece will be as easy as ripping it from the game separately and gluing it on the back
- **2021-12-01** `xsoulstacex` [💬╎general-help] I'll try to replug everything one to one but right off the bat I dont read Base Color on the Bungie Halo Shader. Might it be Diffuse? or Color Control?
  [attachments] unknown.png
- **2021-12-02** `xsoulstacex` [💬╎general-help] My goose is cooked, I've done all I can without further support. Stuck on getting the chest runes to glow as well, and getting the visor to function with color. I could also worry about my bones not rotating symmetrically or my back piece not existing, but if I can get my lights and visor it should be close enough to call it good
- **2021-12-02** `xsoulstacex` [💬╎general-help] Color me depressed and in denial because I can't believe Blender is limited that way. You very well could be correct about that though. If that is the case then my only options would be to make everything uniform to the chest runes, or perhaps try and put some carefully placed planes over them and parent them to the same bone?
  
  I'm not ready to throw in the towel just yet on him
- **2021-12-03** `girldiseased` [📜╎ripping] can’t wait to reposition the halo reach shoulder pieces in the right place **since 343 cant seem to ever do it right**
- **2021-12-04** `unordinal` [💬╎general-help] I'm trying to extract parts of a level from Halo Reach via Reclaimer but the .scenario_structure_bsps are just showing up as a single giant pyramid in the model viewer. Is Reclaimer broken for that right now or am I being dumb?
- **2021-12-08** `richter3d` [💬╎general-help] This feels like the right channel but please let me know if there's a better one to ask, is there anything like this that works for MK VII from Infinite? This was a game changer back in the day for cosplay and with the armor progression I don't think I will ever find the permutations to make "my" Spartan. Thank you!
  https://veegie.com/Vanity/
  [embed] Vanity - Halo Reach Player Image Generator https://veegie.com/Vanity/ Vanity is a player image generator for Bungie's video game Halo Reach
- **2021-12-08** `young_gusano` [💬╎general-help] high key having a tough time with the Halo infinite rig. I am too used to working with the Reach/4 models that are easy to pose. Anyone have any tips?
- **2021-12-10** `zidiath` [🦴╎animation] Do we have any tool to convert animations between different game rigs, example Reach spartan animation on 3s rig
- **2021-12-14** `slimbopulous` [📜╎ripping] does anyone know where the player animations are stored in halo 3 odst map files? does it use the same animgraph as the one from 3?
- **2021-12-15** `unordinal` [🔍╎texturing-and-shading] In your spartan file, go to File > Append. Navigate to the downloaded Halo5Shaders.blend file. When you find it, double-click it and navigate to NodeTree and then the Halo 5 shader. Select it and click append at the bottom right.
  
  In your material for the spartan, press Shift + A to add a new node. Go to 'Group' and select the Halo 5 shader group you appended. Connect the inputs and output appropriately.
- **2021-12-17** `thejudsub` [🔍╎texturing-and-shading] Yep. Thats exactly the thing im referencing with the blender parallax build.
  https://devtalk.blender.org/t/parallax-occlusion-mapping/15774
  [embed] Parallax Occlusion Mapping https://devtalk.blender.org/t/parallax-occlusion-mapping/15774 There has been interest in supporting parallax occlusion mapping (POM) in Blender for some time. See for example https://developer.blender.org/T68477, https://blender.community/c/rightclickselect/t9bbbc/ and Pa
- **2021-12-21** `aqua_himura` [💬╎general-help] hw2 goblin that uses h3 grunt animations(since i currently lack the tools to get the actual animations
- **2021-12-23** `_hauki_` [💬╎general-help] For easier removal you can rotate the head bone in weight paint mode by pressing R and moving the lights off of the chest so you can reach them better
- **2021-12-25** `el_perro_de_mario_bros` [🦴╎animation] Hi !
  I am starting modding again and searching videos/tuto to learn how to import animation to 3
  In particular I need reach elites models and animations. I also need reach 1st person animation
  Does anyone have that ?
- **2021-12-28** `javenator` [🔍╎texturing-and-shading] Are all the textures for the infinite spartans supposed to be set to non-color, or do the color masks need to be set sRGB?
- **2021-12-29** `makimaplushielabourgaming` [🦴╎animation] Thats probably just be making too much time for the elite to get back into its original pose, im not very great with looping animations atm
- **2021-12-31** `xenomorph_xx121` [💬╎general-help] I got a chance to look at the Halo Infinite elite file and got access to it but I can't access the rig. It says "Not yet implemented"
  Any idea how to fix it?
- **2022-01-04** `slimbopulous` [📜╎ripping] what programs are able to rip the animations from reach? i know bonobo exists, but its not compatible with both 360 and MCC versions
- **2022-01-08** `elitecaden` [🦴╎rigging] I'm trying to mirror the left side of the elite skeleton to the other side, but when I do the symmetry button it just spawns in the mirrored bones 180 degrees and won't work properly when I put it in place using Edit Mode
  [attachments] image5.PNG, image6.PNG
- **2022-01-08** `average_username` [🦴╎rigging] I understand if you want to do it yourself but someone has already rigged reach elites
  https://discord.com/channels/819709630540021810/821676515099279374/906982038920396871
- **2022-01-08** `elitecaden` [🦴╎rigging] I'm using the rigged models from chief who posted the halo reach elites, but they didn't have Inverse Kinematics which I added to one side of the skeleton
- **2022-01-08** `opr_23b` [💬╎general-help] I wanted to ask a question regarding Halo 2 assets, texture stuff, and whatnot.
  
  So, I've extracted the model of the Elite using reclaimer, and I've imported it into blender with rigging, textures, and whatnot. (I personally preferred to rip the model myself rather than download it from a previously existing source.
  And to cut to the question itself without bloating with too much information, is it at all possible to use some sort of node setup that uses the RGB textures from the game, in order to easily retexture the Elite?
  Basically, you set a specific color in some sort of option, and it retextures the elite using the RGB mask as a "base"
  [attachments] arms_color_change.png
- **2022-01-12** `.bombwolf` [🦴╎rigging] What the heck. Who ever created the weight paints for these models screwed up lol.
  On the Marine models from Reach, the knee fixer and the foot bone
- **2022-01-13** `inocturne` [🦴╎rigging] i have a question about the infinite elite rig how do you like make it go for one bone instead of both arms for example
- **2022-01-15** `gingerjesus.` [🦴╎rigging] Has there been anything about the Infinite Elite's rigging already? I've got an issue with the general's rigging
- **2022-01-16** `obedientsole64` [💬╎general-help] As in why did sirasia has to make the rig so stupid I mean look at chief's Reach Spartan rigs every bone in the bodie act's like an actual skeleton but
  [attachments] Spartans.png
- **2022-01-20** `inocturne` [🦴╎rigging] question on the bones that grouped on the elite / marines rig how do you make it into one group like so you only move the forearm part
- **2022-01-26** `gingerjesus.` [💬╎general-help] How do you save your scenes in Blender without making whole stinking copies of your .blend files?
  I wouldn't wanna have to have multiple copies of the Infinite Elite for every pose I make when that thing's .blend file is a gigabyte big
- **2022-01-28** `nooncall` [💬╎general-help] i cant seem to export the halo reach spartans the colour turns whiteand has a weird pose like this when i do export
- **2022-01-28** `frostkota` [💬╎general-help] So I'm trying to make my custom Sangheili for VRC I downloaded the the Halo Infinite Elites uploaded by Sierra and Rigged by Zangh and so I got a couple questions about them since I'm very new to this do I need both the Meta Rig and Regular Rig and if I dont need both which one would be better? afterwards what next steps do I need to do to just simply implement the avatar itself into VRC I do plan to add other stuff a bit down the line such as RGB so i can change the color of the armors itself and the emissions and items such as energy swords and animations but I just want to priortize on actually getting the body / armor into VRC
- **2022-01-30** `acidmanrpgz` [📜╎ripping] His so how would one extract their multiplayer spartan from Halo Infinite (not sure if right channel)
- **2022-01-31** `piratepenguin.` [🦴╎rigging] Has anyone got the female reach marines rigged? I've been trying for a while & they just dead ass look odd, I've attempted multiple times so idk if it's an issue on  my end or if that's just how it is in game
- **2022-01-31** `piratepenguin.` [🦴╎rigging] Yeah that's what I figured, the rig for the female marines in reach is just.....awful lol. Shoulders, & back are always messed up badly, I just might end up using a male marine skele in it's place
- **2022-02-02** `elitecaden` [🦴╎rigging] I've put IK's on the halo reach elites, I just need to find a way to put the IK control rig under it so the model can move from place to place
- **2022-02-02** `elitecaden` [🦴╎rigging] https://drive.google.com/file/d/1691hRKybxLjbUxLcCpO0x68vFglxqh3X/view?usp=sharing Link to the minor elite with IK's to hopefully make it easier for you to know what I'm looking for
  [embed] Elite Minor with IKs.blend https://drive.google.com/file/d/1691hRKybxLjbUxLcCpO0x68vFglxqh3X/view?usp=sharing
- **2022-02-02** `elitecaden` [🦴╎rigging] If that for some reason doesn’t work you can try this https://drive.google.com/file/d/192fvKMQgWK1zWrt7VTOEG6u3Sbel12yw/view?usp=sharing it’s just chiefs elite file with a few extra bones that create the ik’s
  [embed] Elite IK chief elites - Copy.blend https://drive.google.com/file/d/192fvKMQgWK1zWrt7VTOEG6u3Sbel12yw/view?usp=sharing
- **2022-02-03** `williaso` [🔍╎texturing-and-shading] I just can't get this invisibility shader to look good. I'm trying to make a Halo invisibility effect but it never turns out right. 😭
- **2022-02-05** `nobleofdeath16` [🦴╎animation] That ives me some ground to work off of since I was already familiar with working with it in Reach this prior year. 
  
  @Vuthakral ,was Fubih's animations they made for it open source or are the unavailable? Being able to just swap it to the Halo 3 first person rig would save a ton of work on the mod; as that would just leave matching up textures and dialing in damage values/mechanics.
- **2022-02-05** `nobleofdeath16` [🦴╎animation] Oh neat, okay. I just got the base model scaled to where I think it should be in comparison to the base H3 shotgun. Now to get it rigged.
- **2022-02-17** `theonedutchguy` [💾╎autodesk] Well, I have a version of Marcus Lehto's MK V that I rigged myself, but I don't have/know of any other spartan models with Maya rigs
- **2022-02-25** `coarforge` [💬╎general-help] If you meant it in the graphics programming sense, you usually at least have a vertex shader and a fragment shader. The vertex shader runs for each vertex in the drawing pass and generally does stuff like applying the transformation and projection matrices (and probably some other stuff). The fragment shader again runs for each fragment and outputs a color. Depending on the depth, it either gets ignored, or put into the buffer that is currently being drawn to.
- **2022-02-26** `bumbusatepadang` [🦴╎rigging] i am just wondering can you change his arms and legs into IK heres the rig
  Halo Infinite Spartans organized, shaded and rigged. [Includes MKVII Core, MKV (B) Core, Yoroi Core, MKVI GEN III Core, Spartan Agryna Core] with the ability to mix them.
  
  - Render By @mrgruntington -
  
  - Shader by @Grand Bacon -
  
  - Rig by @SkyBox -
  
  Download; https://mega.nz/file/piQljIjL#rSEzZ47GYmJi1vNiA9PE2SvTnO5fzdYIryD8BIMJUJ0 #archive-infinite-packs plz i need help
  [embed] 434.5 MB file on MEGA https://mega.nz/file/piQljIjL
- **2022-03-02** `Deleted User` [🦴╎animation] https://cdn.discordapp.com/attachments/781624958769692703/948663704755519538/0001-0075.mp4 quick test animation of an odst
  [embed] https://cdn.discordapp.com/attachments/781624958769692703/948663704755519538/0001-0075.mp4
- **2022-03-05** `arionis` [💬╎general-help] Okay, so, if I want to make the UA Security shoulder from REACH part of the Infinite Spartan, am I doing an Object Parent, to the shoulder (which didn't seem to work, but maybe I did it wrong?) or a Bone Parent? I want to be able to pose the Spartan without having to go back and pose the shoulder.
  [attachments] Parenting.png
- **2022-03-05** `defultimate12k` [💬╎general-help] question
  when i was watching the video about exporting Halo render models from Reclaimer i followed all the right steps but for some reason it doesnt keep the color of the rendered model heres an example
  The middle one is the video from Plastered Crab and his came out exactly like that but mine didnt
  are there are certain settings i need to change or configure that i missed?
  @Plastered_CrawDAD
  [attachments] unknown.png, unknown.png
- **2022-03-07** `hypersonic_falcon` [🦴╎rigging] How would one go about making an IK rig for the Reach Spartans pack from #📂╎community-upload-download?
- **2022-03-12** `williaso` [🦴╎animation] @Ramiere Here's the actual animation from Reach, (clipped to the proper time) https://youtu.be/tIPG3T3fUhU?t=38
  
  See how it's more exaggerated, and drawn out, so you can tell clearly what's happening and how weighty it feels?
  [embed] Halo Reel Assassinations & Animations https://www.youtube.com/watch?v=tIPG3T3fUhU&start=38 Yo, thanks for watching. Idk what you're doing here but hey, you do you. 
  
  I don’t take credit for the animations shown; this is just a showcase
  
  ———————————————————————————
  
  iFunny: https://ifunny.co/user/Can
- **2022-03-13** `thejudsub` [📜╎ripping] can anyone point me to a tutorial for ripping things from H2A? And possibly how to rip maps from H3? If not halo3, then infinite is fine as long as i can get forerunner environment assets.
  I wanted to check out H2A ripping because i got the prophet model from the drive, but the UVs and weights were pretty messed up so i wanted to fix it up myself. And i need halo 3 stuff because i need the map where the prophet of truth tries to fire the rings in the ark.
  But im open to building the scene myself with infinite assets. I can figure out the shaders myself
- **2022-03-18** `get2dachopra` [💬╎general-help] Hey there - so I’m trying to recreate my Infinite Spartan with the Scorpion Punch armor coating (does anyone know the color code btw?); how do I just recolor just the right arm, and how do I move the helmet attachment to his left side? Thanks! 
  
  (I’m also new to Blender so apologies if this is a noob question)
  [attachments] InfiniteSpartans.png
- **2022-03-24** `fiycsf` [💬╎general-help] @Jockey your easiest method is to take what's here;
  
  https://discord.com/channels/819709630540021810/821676515099279374/920177041222008832
  
  (Not the mark 7 that's already on the rig, replace that and you will end up re-rigging just the parts that are not in the rigged for VRC, in the blend from Surasia)
  
  Put your modified on top of the one I've done, weight the new parts solid to relevant bone, renmame the UVs so that you can join the meshes, and then follow the readme to tpose, and use the shader by R93 and Point Device.
- **2022-03-25** `woefulbard` [💬╎general-help] anyone know how many frames long the halo reach energy sword animation is before it starts to loop?
- **2022-03-27** `oc729` [🦴╎rigging] I can't speak for those as I've never rigged anything other than H5 models or Infinite weapons. If you're looking to rig human characters from Infinite, i.e the pilot, you should probably try rigify since they have face bones in their rigs. For Reach, there's already a pack with all the armors so idk what you would want, unless it's also human character models. Only issue with Rigify is in my message above with the weapon. The rig parents some parts of the model, but not everything.
- **2022-04-04** `saltyace` [📜╎ripping] So, I'm working on Tempest from Reach, right. And I've got the all the main geometry and stuff. But there's a bunch of foliage. And I have no idea where it is. I've only ever used reclaimer once to pull the main geometry with its basic textures. I have no idea what I'm doing.
- **2022-04-09** `the_heavynator` [🔍╎texturing-and-shading] also if your converting them to halo3 assets h3 doesn't support coloured specular maps so you might have to do a bit of editing to get them to look right
- **2022-04-12** `defultimate12k` [💬╎general-help] if I wanted to use my custom halo reach spartan re-attach the model to the bone structure of the halo reach character downloaded from reclaimer and use the animations
- **2022-04-16** `aflyingdorito` [💬╎general-help] Does Bonobo work with Reach? i have got animations working for Halo 3 and ODST while any loading of reach maps say "Cache is from an unsupported game build".
- **2022-04-22** `chicanopacman` [💬╎general-help] New to blender and 3d stuff in general, been using reach models that were pre rigged and they were easier to use
- **2022-04-23** `crashlegacy14` [🔍╎texturing-and-shading] so right now I'm working on a shader for the recently ripped GB02 models and they have a ton of masks staked in a single channel. extracting the individual masks has proven … tricky. right now my process is to use a color ramp to produce and RGBA image and break that down as normal, however I'm getting some bleed and I'm not sure how to address it.
- **2022-04-24** `xxpythonxx` [💬╎general-help] Are there bones in the h3 model’s arms? Because to me it looks like there’s just bones in the armor and not the arms?
- **2022-05-01** `keeeeeeeeen` [💬╎general-help] So I'm having this strange issue where the colour maps for my chief model in blender are displaying differently between Eevee and Cycles. He's supposed to have two white stripes on his forearm, which displays correctly in Eevee, but when viewed in cycles they merge together into one solid line for some reason. Here's a side by side view for comparison:
  [attachments] unknown.png
- **2022-05-02** `superintendent.` [🦴╎rigging] How do the elite legs work? Do the bones follow the path of the mesh or do theydo something like this?
  [attachments] IMG_20220501_215516.jpg
- **2022-05-02** `Deleted User` [💬╎general-help] also you can use "false color" to see how lit everything is. from what i remember you want everything between blue to yellow, with purple/black being under exposed and red/white being over exposed, but if thats wrong im sure someone will correct me
  
  **of course** sometimes for artistic choice you want things to be over/under exposed
  [attachments] unknown.png, false_color.png
- **2022-05-02** `xxpythonxx` [🦴╎rigging] In my experience elite legs follow the path of the mesh but it depends on how it was weight painted
- **2022-05-04** `fiycsf` [💬╎general-help] Otherwise I'd recommend doing the reach one of you're just after a decent one to tinker.
  
  Not too many materials, scaled right it feels okay in VR etc.
  
  @Gallium you got any reach one images handy?
- **2022-05-07** `Deleted User` [💬╎general-help] GUYS, a friend of mine named @Deleted User wanted help with the Halo Reach pack models that has the IK rigs. He kinda has this issue where he tries to pose the model like the Pelvis bone for example and move it around. The entire model moves along with it. Please go ahead and DM @Deleted User! Yeye? Thanks
- **2022-05-07** `Deleted User` [🦴╎rigging] Hey guys anyone can help me with the reach models when I move the pelvis up and down it doesn't bend the model but instead moves the entire model much like a root bone
- **2022-05-13** `classytm` [🔍╎texturing-and-shading] I don't wanna mess this part up, correct me if I'm wrong but I should append the following: Halo Infinite Shader 2.3 - Infinite Color - Infinite Masking Sorter 6 way - Infinite matts. Right?
  [attachments] unknown.png
- **2022-05-18** `kalawaki` [🦴╎rigging] How do I apply a preexisting rig to an unrigged model? The Jega ‘Rdomnai rig is giving me serious issues and I’d ideally like to slap the normal H:I Elite rig on him if at all possible.
- **2022-05-18** `kalawaki` [🦴╎rigging] I'd ideally like to just slap the normal HI Elite rig on him since I have had no issues whatsoever with that one.
- **2022-05-18** `thebigdaddy1518` [🦴╎rigging] Weight issues like that honestly shouldn't be too hard to fix, from what I can see they only need a small amount of adjustment. Like Williaso said, I think you should look up a weight painting tutorial. Its all universal so it sould translate well onto the elite, and the armor actually should make it easier. But theres waayyyy too much weight coming from his bones into the others. And I think alternatively you could try Blender's Auto Rig system, Its not 100% but its not terrible
- **2022-05-19** `plastered_crab` [💬╎general-help] Map porting can be a PAIN and can take a while to get right. Just importing them from Reclaimer isn't enough as the textures require additional work and need to be manually scaled/tiled as well as setup with a proper Halo shader. After that you need to add lights to your scene. If lights are on the inside the pics you sent would probably still be black as the outside isn't lit probably due to a number of reasons
- **2022-05-19** `Deleted User` [💬╎general-help] Oh I made this animation with the reach model let me know what you think?https://twitter.com/momentous_is_up/status/1526564149739802630
  [embed] https://twitter.com/momentous_is_up/status/1526564149739802630 The Last Recon Spartan Is Somewhere I seen him.
  
  YT - https://t.co/kB0LalJJbR
- **2022-05-26** `sir_swagshire` [📜╎ripping] So I've always been interested in 3d animation and I'd like to use the reach models to experiment. I've been trying to set up Reclaimer to rip some models out of Reach MCC on PC but I've ran into issues setting it up. I ran the v1.7.309 installer from Gravemind's mediafire link and it is unable to load the mission .map files saying "It's a valid .map but my not be a supported version" I'm not sure where to find the ReadMe either so I bet it's something nice and simple that I'm missing lol. Any advice?
- **2022-06-02** `r93_sniper` [📜╎ripping] For a Comprehensive Halo 5 pack, i'd like it set up as such:
  1. Have all Armors set up with Names, Materials, Rigging, and Skin Weights
  2. Have it set up in 3DS Max with Layers
  3. Have it set up in Blender with Collections
  4. Have a Full FBX with everything exported in a monolithic stack
  5. Have Each individual Armor Set exported as a Seperate FBX
  6. Have all the textures for each individual armor set named correctly and located in their own relevant folder
  7. [Optional] Have a comprehensive blender shader set up
- **2022-06-04** `randomando2902` [💾╎autodesk] Hello there. New to the server! I'm an aspiring 3D artist working mainly in Maya. Looking to make some animations with Halo Reach assets. Where do I start?
- **2022-06-07** `hex5582` [🦴╎rigging] Hello I was hoping someone would be able to help me out, I downloaded the halo reach civilians pack from Crab’s drive and I was trying to re rig them and give them an IK rig
- **2022-06-09** `gameoverload` [💬╎general-help] For the pack and or Goblin's shader, the pose has an action pose and rest pose, to put it back into the base pose, switch to the rest pose, and when you want to actually pose, use the action pose.
  Also the Spartan armor is just saved by saving your Blender project, like any other object in Blender, and color saves the same way since it just from changing the shader of the armor piece your customizing.
- **2022-06-12** `koax117` [💬╎general-help] Mixamo can actually recognize some armatures. I got it working on reach Spartans and elite rigs. Elites are a little wonky. But better than have to animate walk cycles yourself
- **2022-06-14** `rafaleeater` [💬╎general-help] https://www.moddb.com/games/halo-3/downloads/bonobo-version-1003
  [embed] Bonobo [Version 1.0.0.3] Halo2/3/ODST/Reach Animation Extractor file https://www.moddb.com/games/halo-3/downloads/bonobo-version-1003 Bonobo [Version 1.0.0.3] is a Halo2/3/ODST/Reach Animation Extractor used to extract the animations from those 4 halo games, this can be found on halomaps.org, this c
  [attachments] unknown.png
- **2022-06-15** `gameoverload` [💬╎general-help] Through the Blender Spartan 2.0 Pack, via it's Bake outputs on the right of the Shader node where the BSDF connects to the material output. Although you will want to have an image texture node somewhere on each armor you wish to bake out textures for that has a new 1k, 2k, or 4k image ready for the baking of textures.
- **2022-06-15** `deityeternal` [🦴╎rigging] Need some help re-rigging or just fixing my broken ODST rig, I figure I can try to use Vat's baseline marine rig and turn it into an IK but if someone can hold my hand on this issue I'd appreciate it. I've never bump into an issue like this but the IK constraints refuse to work, I can't release the pack broken so I'm turning to the community form help being that it's a very niche issue and there are experienced riggers here.
  [attachments] Needs_Re-Rigging.png, IK_work.png
- **2022-06-18** `bag_of_nickels` [💬╎general-help] ive been following the tutorials but im having the problem of my spartan showing up as purple in places now matter what color i want him to be, other colors and textures are not coming in right. is there a step im missing or something. it could be my laptop too but i thought id ask before assuming that.
- **2022-06-18** `Deleted User` [📜╎ripping] Not so much encrypted.
  
  I'll explain it using Halo.
  
  Essentially the files are located inside of another file. You want an ODST from Halo 3? You gotta rip it from the mission with an ODST in it. Its not encrypted so much as it is buried - its just a case of having the right shovel to dig it out more than it is a case of having the right key for the door.
- **2022-06-24** `nakaicoviecanon` [💬╎general-help] Question: does anyone have any good model poses of the Brute Warrior, Elite Mercenary, and the two different Hunter variants from Halo Infinite’s campaign?
- **2022-06-30** `rafaleeater` [🦴╎rigging] small issue with the elite in #deleted-channel: the rig is quite wonky, and imports all bones completely separate upon being appended
  [attachments] 2022-06-30_11-30-13.mp4
- **2022-06-30** `Deleted User` [💬╎general-help] basically every node (the squares in the shader tab) comes with an imput on the left (and its always the left side) and an output on the right (and its always the right)
  
  depending on how you use them is how they work but just think of them like a production line receiving the info, doing something to it then sending that new info onto the next stop
- **2022-07-03** `r93_sniper` [🔍╎texturing-and-shading] if you're using the infinite shader (which judging from the masks you are), the goal was to make one that emulates the look of what halo infinite does as closely as possible. They use, at max, 6 layers from the two CC masks and then 2 more from the ASG map. Unless there comes an instance where they actually use an extra channel (and subsequently actually add it to the game/shading system), there's no purpose in trying to make the shader accept that mask
  
  If you're just trying to add a decal stripe there's a few methods to do so, 
  1. Pick the zone that it most closely matches the color that you want the stripe to be and make it part of that mask while removing it from whatever overlaps
  2. Create a decal plane and attach it to the model and UV it to have only the decal stripe (and alpha or additive the rest)
  3. Modify the shader for your own local use to accept an extra mask
- **2022-07-07** `unordinal` [🔍╎texturing-and-shading] CHAM is a bit trickier - the red channel looks to be a color factor (MixRGB with the base color and mode set to something like Overlay); green channel _might_ be height? The others I'm not 100% on; do you happen to have any reference images of a xeno in that game with gore? I'm looking for some right now
- **2022-07-09** `incuriouslog` [💬╎general-help] Sorry if I'm completely off on this, but after a quick look it seems to be using a few shaders to generate that starfield rather than a texture. If you go to the World Properties tab on the bottom right and remove what's there you can then select "Background" then click the yellow dot to the left of colour and choose "Image Texture" which will then let you load an image file.
- **2022-07-11** `gameoverload` [🔍╎texturing-and-shading] Basically you just need to use Goblin's shader, and add an unconnected image texture node in the first material your baking out from, make a new image in that node that is 1k , 2k, or 4k in size to fit whatever you need, specifically that scale squared, like 256x256.
  Then you need to copy that node to every material of every object you wish to bake out textures for.
  Then you go back to that first material you were working with, connect one of the bake outputs to the material view node to the right of the shader.
  Then go into your render tab, set the rendering in the bake tab on the side to emit rather than combined, set the 16 tiling below that to 32, then hit bake.
- **2022-07-12** `plastered_crab` [💬╎general-help] This goes over the different parts that you actually need @VinWarrior 
  https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-07-14** `plastered_crab` [💬╎general-help] https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-07-15** `incuriouslog` [🔍╎texturing-and-shading] I just did something I'm very proud of, yet am fairly certain I did in a completely dumbass way, and was wondering if anyone would care to translate my stupidity into correctness.
  I wanted to add a PNG with transparency to the ODST chest attachment for the Mk. Vb core, but I also wanted it to match the distressed look of the metal.
  So I opened the symbol in Photoshop, then added the asg bitmap for the chestpiece to another layer. I lined it up with where the symbol was going to be, created a layer mask on the symbol layer, then applied one of the channels (red, I think) inverted as the layer mask. This matched the distress perfectly with the chestpiece. I saved it as a PNG with transparency.
  I then imported the PNG as a plane into Blender. I added a subdivision modifier and then a shrinkwrap modifier, set to project onto the surface of the chestpiece with a .0005 offset. I sized and positioned the plane, then for good measure made sure it cast no shadow, and paired it to the chestbone of the rig so it moves when I pose it.
  It works exactly how I need it to.
  How should I have done this properly?
  [attachments] blender_22.07.15_21.44.33.920.jpg
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
- **2022-07-18** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-07-20** `exporush` [🔍╎texturing-and-shading] I haven’t looked into cause I just haven’t needed to yet and I’m more just curious right now. Does the current shader system for the infinite Spartans support the camos patterns and stuff we see in game?
- **2022-07-27** `starman0012` [🦴╎animation] I made the rig and weighted the meshes back in December I believe, haven’t worked out all the kinks quite yet.
  The jetpack is from one of the halo reach pack here.
  I also borrowed Surasia material setup.
- **2022-07-29** `.tylar` [🦴╎animation] Since an Elite is a lot bigger and heavier than a normal person I'd suggest spacing out the key poses a bit more and see where the sweet spot lies
- **2022-07-29** `elitecaden` [🦴╎animation] Here is a picture of what the rig looks like and here is the file I made for myself that has the ik rig https://drive.google.com/file/d/1QF7cQlTFasZaNgWT8WQTK-bvgoSxjlR1/view?usp=sharing
  [embed] Elite IK chief elites ALPHA.blend https://drive.google.com/file/d/1QF7cQlTFasZaNgWT8WQTK-bvgoSxjlR1/view?usp=sharing
  [attachments] image102.PNG
- **2022-07-29** `gingerjesus.` [🦴╎animation] Eh actually similar situation in some melee motions, just more purposeful with how you move
  For example, right handed overhead swing with a sword, your motion starts with your left side facing the opponent; go to swing, and rotate your posture from left to right
  The shifting weight would emphasize attack power, so if the Elite switched his footing *as* he swung his Plasma Rifle, it would have more power(granted a miss), and would allow him to come into the later grapple transitioning to the next side from what he already switched to
- **2022-07-30** `sunksi` [🦴╎rigging] https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-07-30** `gameoverload` [🔍╎texturing-and-shading] If you read the outputs of the shader, not quite, a shader built in Blender will not work in another graphics engine, without replicating it within that engine's graphical structure.
  In the case of Unity' that is Unity's visual shader code system, or Amplify for a plugin of similar design.
  However Unity comes with a standard shader, which if you read the texture inputs of, will tell you what texture maps you need. Why does this matter?
  If you read the outputs of the shader in Blender, it will tell you what it can 'emit' on the mesh, and guess what Blender's graphics engine, Cycles, can do.
  It can bake out the emitting visuals of a selected mesh.
- **2022-07-30** `gameoverload` [🔍╎texturing-and-shading] Ah, it appears the reach shader doesn't include extra outputs, which are on the right side.
  Inputs on the left, outputs on the right.
  You would need to use Infinite armor then, as the shader those use include extra outputs, particular for getting textures used in other engines.
- **2022-08-01** `guhgunk` [📜╎ripping] hoping for some help with basic fp animation importing into blender. i've imported H3 chief fp arms and assault rifle into blender and am trying to import the ready anim. i get an error on anim import that "Node 'camera_control' not found in an existing armature". is there another model i need to bring into the scene from the map files? thanks!
- **2022-08-01** `baconator650` [💾╎autodesk] Sorry should have clarified, that this is working with animations that were extracted from Halo and then imported. I’ve used animation save/load and find it too limited when working with multiple assets in a large scene. Animation layers work best (for my needs). Experimenting with Blender’s animation importer I noticed it automatically puts animation into a layer.
- **2022-08-03** `elitecaden` [🦴╎rigging] Does anyone know how to make the feet keep the Track To constraint but also follow the rotation of the master rig (the square that is around the elite)? I've tried to do copy rotation on the foot bone onto the master rig but that makes the foot mess up when I try to point it down
  [attachments] 2022-08-03_00-09-40.mp4
- **2022-08-04** `elitecaden` [🦴╎animation] Counter animate? Also I started off animating the elite in place until a walk cycle was complete then just looped it and then moved the master rig. And the master rig is moving using linear interpolation
- **2022-08-04** `gregory0701` [📜╎ripping] I got it from ODST, but it seems that the AR and BR share the same animation. In ODST its called rifle reload, in Halo 3 its called rifle BR or something
- **2022-08-07** `atomixshield` [💬╎general-help] does anyone know how to parent a halo weapon to a spartan/elite rig so that when I animate it the weapon moves with it.
- **2022-08-07** `gameoverload` [🔍╎texturing-and-shading] @fable law 458 It isn't just CE that does this right? Cause I thought we looked at the shader code of H2 before.
  @Gregory I would open H3EK and see what it shows ya also.
- **2022-08-08** `2nafish117` [📜╎ripping] https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2022-08-15** `fiycsf` [💾╎substance-suite] If you just want something Promethean like, that you can do right now, try the spartan hellcat set, but remove the techsuit.
- **2022-08-17** `plastered_crab` [💬╎general-help] https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-08-17** `plastered_crab` [💬╎general-help] https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-08-18** `creepatore` [💬╎general-help] Hey so, slowly i've been working on a (mostly) faithful recreation of H3 Chief using infinite chief. This is like, textures and what not. One of the final steps I'm trying to get right is remaking the visor to look like 3's, but can't seem to get the proportions right. I'd appreciate any help or advice on how to do so
  
  Pictured: a piece of the project in question
  [attachments] unknown.png
- **2022-08-18** `snapchipper` [🦴╎rigging] i've been trying to get this H3 rigged model rerigged for H2, is it as simple as renaming the bones?
- **2022-08-19** `aqua_himura` [🦴╎rigging] Recently tried to retarget the h2a brute to the h3 and h2c ones(which worked but also didn't work due to the different poses), wondering if it's possible to make it work
- **2022-08-19** `gameoverload` [🦴╎rigging] You would need to basically do it the tedious way round, and repose the H2A one to the pose of the H3 and H2C ones individually or if the two use the same pose, same pose for both, then match the vertex groups to the H3 and H2C one, again, individual copies for both if their different vertex groups wise, otherwise, make the same vertex group changes for both, then you simply change the rig over to the corrosponding rig you wish to test on.
- **2022-08-20** `chromacore` [💬╎general-help] It's a high poly of the reach space collar for the Marines. The black one on the right is the actual one from reach.
- **2022-08-23** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-08-25** `gameoverload` [🔍╎texturing-and-shading] I mean the best Infinite Shader to use right now is Goblin's ( Voblin) Halo Infinite Shader 2.5.
  Since it works on most if not all Infinite things available at this point.
- **2022-08-30** `tristindiscoarts` [💬╎general-help] Hey is there a reason why https://drive.google.com/file/d/17t8UOy1r4w1yxt_JcGBBspQVP2qATFTr/view?usp=sharing Halo Reach Brutes. Fully rigged and textured and with their power armor. (Not including weapons and humans) is slightly purple?
  [embed] Brutes.zip https://drive.google.com/file/d/17t8UOy1r4w1yxt_JcGBBspQVP2qATFTr/view?usp=sharing
- **2022-09-04** `gameoverload` [💬╎general-help] By tools, I simply mean any specified tools, not assets, that you wish for someone helping you to use.
  Other than that, things like do you want the models used to be accurate to Halo 3/ODST?
  In that case the person helping you will need a Halo 3/ODST shader to work with, which only two people here have done the research to recreate in Blender's node graph based shader system.
- **2022-09-04** `gameoverload` [💬╎general-help] I would actually suggest Chroma, since he already built a working H3 one if memory serves me right.
- **2022-09-06** `rikfastly` [📜╎ripping] ahh
  ok
  I downloaded all of them from reach and halo 4
  at this point would I have to just scower through the map files looking for which one contains the first person animation?
- **2022-09-08** `blakjewellio` [💾╎autodesk] Ok so how do I solve the problem of the massive web of Riggs and the fact that all the Spartan gear seems to be overlapping on to each other
- **2022-09-08** `blakjewellio` [💾╎autodesk] on its defense I like the rigs I can find made in maya im lokin at this chat log and asia said 343 uses maya wonder if I can find some of those How did he rip infinites spartans
- **2022-09-08** `r93_sniper` [💾╎autodesk] otherwise i can send you the infinite reach core rig i have and you'll need to mess with getting the helmets and armor set up
- **2022-09-08** `r93_sniper` [💾╎autodesk] since weight painting in the way people use it here could mean "Hey i'm using the weight paint tool", "hey i'm trying to skin a mesh", or "hey i need to know if i have a weights in this mesh"
- **2022-09-13** `palpythefrog` [🦴╎animation] Oh i see on legendary there is a spec ops elite with those rangers thats explains its yeah you right
- **2022-09-13** `squiggilysquid` [💾╎autodesk] Does anyone know if any rigs were ripped from Halo Infinite and uploaded anywhere? Want to try get the elites rigged up and am reluctant to create my own!
- **2022-09-13** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-09-14** `kyex_` [💬╎general-help] This rig is so weird to me, and I prefer the rig attached to the Halo Reach Spartan Pack, can I get some help?
- **2022-09-15** `r93_sniper` [🦴╎animation] ```full\path\to\map.map
  EditTag tag\here.jmad
  ExtractAnimation all "C:\your_folder_here"
  
  for example:
  G:\Steam\steamapps\common\Halo The Master Chief Collection\halo3\maps\original360maps\010_jungle.map
  EditTag objects\vehicles\pelican\pelican.jmad
  ExtractAnimation all "C:\Program Files (x86)\Steam\steamapps\common\H3EK\extracted_h3_anims"```
- **2022-09-15** `zangh` [🦴╎animation] I think that was because Saber probably couldn't map the Reach mandibles on the CE rig so they just left them in "t-pose" making them clip through the helmet.
- **2022-09-15** `blakjewellio` [💾╎autodesk] I found a few elites on sketch fab but I haven’t seen if they’re very well rigged I know there’s one that cost money that specifically says it’s for Maya and it’s pretty cheap for a model if you find it just look at description
- **2022-09-15** `blakjewellio` [💾╎autodesk] https://sketchfab.com/3d-models/halo-elite-minor-ca4504cbd0f548d8aaefa31f41db97a2
  [embed] Halo Elite Minor - 3D model by Jawley3D https://sketchfab.com/3d-models/halo-elite-minor-ca4504cbd0f548d8aaefa31f41db97a2 Halo Elite Minor made using the following:
  
  
  Maya - Rigging and Rendering
  Substance Painter - Texturing and Baking
  ZBrush - Sculpting
  
  This model is now available on my Gumroad!
  
- **2022-09-16** `frostbitecinematics` [💾╎autodesk] I do agree it really sucks when nothing is really available to use right out of the box, and you have to go learn a bunch of technical things yourself just to even begin making art. That's why this Discord is a pretty ebic place though for Halo stuffs, lots of cool people do the cool technical things for you and you download the cool things and just make cool art.
- **2022-09-16** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-09-29** `elitecaden` [🦴╎animation] When I get home I can send you it. I just used chiefster’s halo reach elites and turned it into an ik rig
- **2022-10-01** `xandion` [💬╎general-help] Does this look... right, for an Elite face?
  I've been mixing with the material types for the past hour now and I'm indecisive.
  [attachments] unknown.png
- **2022-10-03** `trystt` [💾╎autodesk] Okay looks like this is a bit of an FAQ but I'm just gonna ask to put it in my own words, to be absolutely clear.
  The assets from https://discord.com/channels/819709630540021810/821145601416364063/821146113062731776 (Or anywhere in the #🔗╎all-halo-asset-resources section. I am looking to just put together parts for my own Spartan in Maya for the purposes of posing/modeling for artwork.
  Is Maya capable of using the files? Or is Blender basically a necessity?
- **2022-10-04** `scarabaeidaeee` [🦴╎animation] im not an animation expert by any means, but my suggestion to make it a little more natural run cycle looking would be to lessen the time the elite's feet are in contact with the ground, and to make each step to step flow together a little smoother
- **2022-10-04** `hex5582` [🦴╎animation] Why don’t you just load up halo reach, run around as an elite then use theater to analyze the animation?
- **2022-10-06** `elitecaden` [🦴╎animation] Hunter looks like it’s covered in foam. Animation of the pelican looks weird because it looks like it’s moving just on a single axis and how it goes in reverse a bit then speeds up forward all of the sudden. Animation of the 2 pilot heads looked like they were in sync with each other which looks a bit off. The standing elite holding the mangler needs to be more fluid. Especially at the chest where it looks stiff. Try maybe having the chest flow with the arm movements and have the chest do breathing motions (like tilting up and back down). The lighting in space needs work as it looks boring. But the lighting on the ground scenes look awesome
- **2022-10-06** `jaeger1648` [🔍╎texturing-and-shading] Hello, I was told my message might fit better over here
  
  I'm very new to blender and I'm using the reach spartan pack. I couldn't figure out how to export an fbx with any materials/textures at all. Actually the decals exported with it but that's it. Now I kinda know it's because I need to texture bake?
  I found a video on how to texture bake, but ever time I try to smart UV project it messes up the textures, shifts them around on the model a bit. It seems it's because it's not unwrapping right, but I have no clue how to fix it.
  
  I'm about to go to bed, but any help would be greatly appreciated!
- **2022-10-10** `exporush` [🦴╎rigging] So a week or so ago I tried to make a custom Sangheili using this pack but for some reason the rig was weird and wouldn’t really let me move the fingers much. Then moving the arms seemed a bit weird (at least compared to what I’m used to with other packs). 
  
  Any idea how I could fix the hands and figures? It’s probably just me being stupid and not knowing something about rigs and shit. 
  
   https://discord.com/channels/819709630540021810/821676515099279374/906982038920396871
- **2022-10-10** `elitecaden` [🦴╎animation] It is impossible for me to get the right arm high up like in that Halo 4 elite animation reel of the elite running. The shoulder bone will keep going too far and make the animation of the arm look weird
  [attachments] ezgif.com-gif-maker2.gif
- **2022-10-12** `gameoverload` [💬╎general-help] The one thing you will have a problem with however is the skeleton your using to mocap vs the skeleton ( or skeletons in the case of having an infinite helmet over a reach body ) of the assets you have, neither will be synced up unless you used a skeleton for all 3 that is the same, otherwise you might run into issues getting all 3 to sync up properly.
- **2022-10-15** `plastered_crab` [📜╎ripping] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-10-17** `elitecaden` [🦴╎animation] I know my running animation is very non elite, but I wanted to try a running style I'm more familiar with before doing the elite running style. You think what I have in that video is good enough?
- **2022-10-23** `sierra060822` [💬╎general-help] so I'm at a loss rn with trying to export H3 ODST's Dare, someone mentioned the one I posed had a messed up rig and it did so i went to fix it but I've just finishing trying so many different import and export methods and it still persists! If anyone else wouldn't mind trying to export it lmk if its broken for you as well!
- **2022-10-23** `vat_` [🦴╎animation] Hey guys, yall see this? https://youtu.be/iXndc_RLr0Y
  [embed] Halo: REACH showreel https://www.youtube.com/watch?v=iXndc_RLr0Y Here is a bulk of the shots/move sets I did for REACH. It was a great project to work on! 
  
  All material copyright Microsoft Game Studios and Bungie Studios. 
  
  To be viewed for entertainment purposes only.
- **2022-10-27** `elitecaden` [🦴╎animation] Really funny animation to see the elite bouncing like a frog 😂
  
  The only thing I’ll change with this though is having the elite movement when he lands on the ground look less robotic-like and flow more if that makes sense
- **2022-10-29** `gameoverload` [💬╎general-help] I doubt it is the shader given their is a lot less there, from what I have seen of it, compared to the Infinite one.
  The other thing is getting properly what Halo 5 uses, given we don't exactly have all the actual sampler references and such known yet.
  So that would need to be known before the right fixes can be applied.
- **2022-11-01** `gameoverload` [💬╎general-help] I meant more that it is not currently updated to the current Blender shader capabilities.
  Also it matters if he is new to the Unreal material graph , reason I am warning him if he wishes to have those capabilities, especially since he will need to add those himself unless the project gets updated in enough time.
- **2022-11-03** `fable_law_458` [🔍╎texturing-and-shading] @Banned .Casey Oh, my bad. If you're looking for the shader types, go with the first thing I told what you need to do in my first paragraph above, go to guerilla and open that shader you found and scroll down and you should see what type of shader it is if that's what you're looking for.
- **2022-11-05** `chalk33_8677` [🔍╎texturing-and-shading] What I do is
  1) new image file called bake, I've been doing 2048 but thats just a quality thing
  2) select each object and create a new uv map for it
  3) smart uv unwrap everything (probably best to make sure there's no overlap, it can make things screwy)
  4) go into the shader window for each material I want to bake and add an image texture node, using the new bake image file (without hooking it up), and then make sure it's selected
  5) bake
  6) plug the image textures into the output, and then activate the new uv maps to make everything show up right
- **2022-11-07** `gameoverload` [💬╎general-help] The honest way for shading is honestly ripping apart game assets and putting them together accurately using as much information as you can on them as that will teach you the most on things, as it did me. As long as you have the right tools to view shader code or capture data.
  Basically, you will want to learn some stuff on HLSL and Assembly, which is what your effectively creating in a node graph. Unity in particularly even lets you see what your making in node graph as HLSL.
  I would give ya more of my personalized information, but I have yet to really start my 'Practical Shading Compendium' to a full extent to just hand ya that.
  Texture quality is really down to time taken and purpose, what is a texture doing. Plus if you want the most quality out of say simple BW masking or RGB channel packed masking for example, you would need a high poly model that can be a lot more polygonal specific on that masking, rather than a low poly model that texture was baked to be put on.
  Animations are a different story since your getting into rigging and literal animating, which is not my forte, and you would need someone else to really tell ya more.
- **2022-11-08** `gloorby` [💬╎general-help] Okay so I want your guys opinions on something. I'm attempting to make a Halo animation but I'm unsure of which software to use. I'm tossing up between Blender or UE5 but I'm just not sure. I'm far more experienced with UE5 but I can't import many of the assets I want to use due to the Halo Infinite shader. On the other hand Blender is much easier to animate in plus the assets are all there but I find it harder to make scenes and the render times are insanely long even for a 30 second clip. What do you guys think I should do??
- **2022-11-12** `waketheshark` [🔍╎texturing-and-shading] Not sure if this is the right place for this but I'm trying to use Reclaimer for exporting Reach maps. I followed what I can find and downloaded the shared reach maps as well as the level I want to open. I have them all in one folder extracted from their zip folders. I can open the level I want in the 'Map Viewer' window with all of its folders. But I'm getting an error: "System.IO.InvalidDataException: Found invalid data while decoding" when trying to open one of the render_model files like in the youtube tutorial and I can't wrap my head around how to fix it
- **2022-11-15** `gameoverload` [💬╎general-help] You will still want the control change to match, as it matches the materials baked into your Infinite Sentinel. Even if the materials are baked, it will also handle the color masking too, which will match the materials. It will look like hell otherwise, same for your other baked out textures, which you will need to do correctly, or your Sentinel won't look right at all with what Halo 3 needs.
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
- **2022-11-20** `elitecaden` [🦴╎animation] I know the legs aren't moving like elite legs would move but I couldn't get the legs to the correct rotation with the ik rig I have
  [attachments] Elite_Running.mp4
- **2022-11-20** `jadedqrow` [🦴╎animation] The run animation is a bit to fast, as snipes said the feet don’t connect enough. I also think your not portraying the elite having any weight so it just looks floaty. I’d recommend having some slight hit and chest rotations and more variation in the cycle, tho I can’t explain how to rn or how much you would need
- **2022-11-20** `wraith0912` [🦴╎animation] there was someone else who posted an elite running animation a while ago that I think you could look to for an idea as to how they move
- **2022-11-30** `gameoverload` [💬╎general-help] OBJ basically holds your model, and one UV set, and that is about it on the important parts.
  
  FBX on the other hand can hold your model, armature, multiple UV sets, texture data ( that is properly assigned so even 3D can read it ), some standardized shader data, and other bits that are dependent on what you are working with.
  
  Basically FBX is best to use in every way.
  
  GLTF is similar, but just simply open source, but not as robust as FBX given FBX has been around as long as Autodesk has used it as a format.
- **2022-12-04** `sarge3096` [🦴╎rigging] is it possible to parent all the joints to the right bones with the halo 3 ik rig? I've tried all the normal ways and while the pelvis moves the armor, the arms don't move the arms and the legs won't move the legs. Or, is it possible to set up the halo reach rig as an ik?
  [attachments] image.png
- **2022-12-08** `gameoverload` [💬╎general-help] Basically you'd be recreating the texture set minus the normal map.
  Meaning you would need to create the ASG mask, ala AO, Scratch, Grime masking all in one texture via Substance Painter, and then creating RGB RGB masking to fit the helmet so it is 6 zones max in two seperate textures Mask 1 and Mask 0.
  
  However as Goblin said, no point in doing that when Infinite has done this for you via the Reach armor core, no need to basically buy Substance Painter and learn how to use that just to do something already done in Infinite.
  
  Which he is right, most minus a few pieces exist for the Reach core.
- **2022-12-08** `stumpyj` [🦴╎animation] Is anyone here familiar with both 3ds max scripting and blender scripting? I am converting a script from max to blender, and everything is great until I do animations. Both of these animations were extracted using the same .gr2 data (left: max - imported to blender as fbx, right: blender). Both the good animation and the bad work on either rig identically to eachother, telling me the rig is not the issue.
  [attachments] image.png
- **2022-12-08** `exporush` [💬╎general-help] I always feel stupid when watching this video cause I can never get it right. So what am I supposed to change when when he says to change the extension?
  https://youtu.be/r_5suSLUucU?t=35
  [embed] How to Port Coatings to Blender from Halo Infinite [100% Accurate] https://www.youtube.com/watch?v=r_5suSLUucU&start=35 Swatches: https://mega.nz/folder/yWoCxTrZ#0emVAwxOqA85fg9fePy3tA/folder/7XYBVCxT
  
  make sure all of these are in a /materials folder.
  
  CoatingTool: https://mega.nz/file/oqwx0LRJ#cL4
- **2022-12-21** `blue.nasca` [🔍╎texturing-and-shading] If I wanted to add like, for example, halo reach style hazard decals to the infinite models, I'd be editing the ASG texture right?
- **2022-12-24** `r93_sniper` [📜╎ripping] ```full\path\to\map.map
  EditTag tag\here.jmad
  ExtractAnimation all "C:\your_folder_here"
  
  for example:
  G:\Steam\steamapps\common\Halo The Master Chief Collection\halo3\maps\original360maps\010_jungle.map
  EditTag objects\vehicles\pelican\pelican.jmad
  ExtractAnimation all "C:\Program Files (x86)\Steam\steamapps\common\H3EK\extracted_h3_anims"```
- **2022-12-25** `gameoverload` [🔍╎texturing-and-shading] ```
  Here, the specular reflection and smoothness are defined by a colour and the Smoothness slider. No Texture has been assigned, so the specular and smoothness level is constant across the whole surface. This is not always desirable, particularly in the case where your Albedo Texture maps to a variety of different areas on your model (also known as a Texture atlas).
  The same model, but with a specular map assigned, instead of using a constant value.
  The same model, but with a specular map assigned, instead of using a constant value.
  
  Here, a Texture map controls the specularity and smoothness. This allows the specularity to vary across the surface of the model. Notice the edges have a higher specular effect than the centre, there are some subtle colour responses to the light, and the area inside the lettering no longer has specular highlights. Pictured to the right are the RGB channels controlling the specular colour and strength, and the Alpha channel controlling the smoothness.
  ```
- **2023-01-08** `zangh` [🦴╎rigging] hey if anyone here is experienced with rigify I need some help. The ODST I kitbashed with the humgen model and rig seems to be giving weird weight results on the back armor. I went to go check the weights and even removed them just in case and there were none, yet the back armor still wants to follow the neck and I'm not sure why. I think it has something to do with the tweak bones, but there doesn't seem to be a way for me to unparent it from the tweak bone.
  [attachments] ODST_fucked_up_back.png
- **2023-01-09** `kelcken` [💬╎general-help] Trying to bake some textures for the Infinite Banished vehicles In blender using the ones uploaded here, and the Wraith, Ghost, and Chopper have all worked fine, but the Banshee is getting a blue tint and not baking  the emissives at all.
  
  I've looked at the shader setup (identical) as well as the bake settings (also identical) and I can't figure out what the issue is. I've also tried out several different influence settings, but none of them come out right on the banshee
  
  If anyone has any ideas, please let me know! Thank you in advance!
  
  First image is of the wraith (working as intended) and the second is the banshee (not working)
  
  Bake results in the bottom left corner.
  [attachments] RenderWraith.PNG, RenderBanshee.PNG
- **2023-01-11** `k1d650` [🦴╎rigging] If anyone could help it would be much appreciated.  I'm trying to pose my Reach spartan while following the only tutorial I could find.  The video seems to be specific to Infinite Spartans, as the rig for the Reach spartans looks different, and the commands they use don't seem to line up.  I'm brand new to Blender and was hoping someone could give me a hand.  Thanks!
- **2023-01-12** `elitecaden` [🦴╎rigging] I’d suggest you follow a tutorial that makes a really simple rig first so you know how they work and how to use them and then go use the more complicated looking rigs like the Halo Reach Spartan rig
- **2023-01-12** `drackilina123` [💬╎general-help] Does anyone know where to get a good hologram shader or how to mimic one? I'm trying to make a H5 Spartan with Holographic effects. Any help will be lovely ^^
- **2023-01-16** `e.nine` [📜╎ripping] Same as the Elite. I am away from my computer right now but should be back in like 20 minutes. I feel like the textures are regular images
- **2023-01-16** `.moonzen.` [📜╎ripping] is there a newer way of ripping reach animations? i used the video tutorial and it just keeps saying "no valid armature" and stuff like that when i import the jma. i think bonobos scuffed or something
- **2023-01-17** `r93_sniper` [📜╎ripping] https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2023-01-18** `e.nine` [📜╎ripping] I got him blue, but now whenever I try to import the animation, it loads in separately instead of attaching to my Elite
  [attachments] image.png
- **2023-01-20** `rubyqueen0` [💬╎general-help] Im guessing this question has been asked a few times (or something in this vain).  Im looking to get the halo reach/infinite Spartans tho blender to unity to vrc. I have a "good" understanding of the unity side of making avatars but am having a lot of trouble getting it over from blender to unity. missing texters, bones braking, what's the profred shader and so on. If there's anyone that can run me tho the best way to go tho this proses that would be wonderful. (can dm) Sorry if this is the wrong place to ask
- **2023-01-21** `dryym` [🦴╎rigging] I am putting a quick temporary rig on my elite that I still need to finish. And rigging can actually be quite fun.
  [attachments] ElitePose.png
- **2023-01-23** `jonathanborges3d` [💬╎general-help] Hi! What is the file of halo reach that contains the assassination animations? Or there are ripped halo reach animations somewhere? Thanks in advance.
- **2023-01-23** `plastered_crab` [💬╎general-help] There is a tutorial on how to rip animations from Reach, however the tool doesn't support all animations from reach as that was never finished
- **2023-01-23** `plastered_crab` [Extracting Halo Reach Anims] I might be grabbing the wrong data as those animations might be part of the list of the ones from reach that don't work
- **2023-01-27** `kaboose1066` [💬╎general-help] anyone here have a good H3 Elite voice? I need a VA for a character but damn, they are so hard to get right, it's only a couple lines, or anyone know anyone who can do it faithfully?
- **2023-01-27** `tacohombre` [💬╎general-help] the elite voicelines for h3 were pretty edited weren't they? that might be why you can't get them right
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
- **2023-02-13** `pajamas007` [💬╎general-help] Hi, I was wondering if anyone had or knows a fix for the Halo Infinite elite rig, the Hand IK bone doesn't work, but the upper arm bone does work, but it doesn't move the rest of the arm.
  [attachments] 2023-02-13_20-46-48_Trim.mp4
- **2023-02-28** `falloutgrim` [🔍╎texturing-and-shading] within Fallout that im working in the combat armor has its diffuse with no color the one on the far left, it has that color pallet on the far right as the color, im asking how i can use that pallet and apply it in unity? it is really odd it dose this and i mite not be able to do this in unity, its really weird it uses that pallet to determine its color
  [attachments] image.png, image.png
- **2023-03-05** `denzeus.` [📜╎ripping] Exactly what I felt when I discovered that the H2A elites and brutes don't have eyeballs (or face bones, in the elite's case)
- **2023-03-07** `brussell09` [🦴╎rigging] Anyone know if there is a way to copy attributes from one control to another within a UE5 control rig? For example I have the one turret rigged nicely. I can quickly create multiple controls but they are obviously all the default red circles. Id like to copy and paste the sizing, color and attributes to each. Any tips?
- **2023-03-09** `elitecaden` [🦴╎animation] @SODAZ Loved how the ending of your animation tied in with the ending of Halo Reach. Seeing Noble Six use the Mac Cannon and the Spartan seeing the Pillar of Autumn flying away was amazing
- **2023-03-12** `whiskeyace16` [💬╎general-help] Rigging and while making render i know the armor you gotta put them on yourself it not like the Reach armor pack
- **2023-03-22** `baneofjustin` [🦴╎animation] I loved seeing the animation but I think what stood out to me most (problem wise) was how the elite didn’t reach to the push off when the spartan jumped off its head, the ground makes sense since it’s just a plane, but a head is a moving thing with physics, there should be some reaction to the jump and it doesn’t seem like there really was
- **2023-03-27** `pajamas007` [📜╎ripping] ah, okay, that sounds cool
  i'm in need of reach maps anyways for my animation hehe
  but ill def start working on them now
- **2023-03-29** `thechipsmotherfuckendubbo` [🦴╎rigging] I hate to sound Like a pain, But I want to try and IK the Halo Reach spartans, but the Tutorial I am using confuses me. Anyone know what to do? 
  https://www.youtube.com/watch?v=Pt3-mHBCoQk
  [embed] Blender 2.8 Inverse Kinematics In 2 Minutes! https://www.youtube.com/watch?v=Pt3-mHBCoQk Hey everyone! Today I'll be showing you how to create inverse kinematics for your character in Blender 2.8 in 2 minutes!
  
  If you enjoyed this video, please don't forget to like and subscribe for more content!
  Yo
- **2023-03-30** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2023-03-30** `icedcoffee4849` [💬╎general-help] hallo, i'm trying to bake some camo textures, and this is just on the color channel, but the texture (only on the camo part) ends up being too bright, i have no clue why. any ideas? also, in case you're wondering what the other nodes are, i'm replacing the node wrangler by a different custom node that's supposed to make a similar function but with avoiding tiling, i also tried it without it, and with the original node wrangler and its the same thing so i dont think it's affecting it in any way
  edit: color ramp is also missing because i want to leave the texture in greyscale, i dont want to change the colors of it. same result with or without it
  edit 2: HELP AAAA
  [attachments] image.png, Chest_Color.png, image.png
- **2023-03-30** `nihilmancer` [💬╎general-help] I also downloaded the Elite pack rigged by Zang, but I can't seem to pose anything assymetrically, even the IK handles for the feet or the FK points on the jaw move the opposite one
- **2023-04-06** `varlox_verum` [💬╎general-help] Not sure if this is in the right place to ask this but does anyone have a Sangheilios map or inside a Covenant ship I could use? Trying to make something with my Sangheili. Hell a banished base or ship works as well.
- **2023-04-17** `frederick2223` [💬╎general-help] https://discord.com/channels/819709630540021810/967828504605503508/1010305715589627915 is the Elite at the start when I boot up the client supposed to be all magenta and shit?
- **2023-04-19** `existential_selkath` [🦴╎animation] sorry for just jumping in here. Anyone know how to actually import halo 4 animation files into blender or anything? The JMA files are in h4-packs but I cant figure out how to do anything with them. All the tools seem to be built for anything older than H3
- **2023-04-21** `gameoverload` [🔍╎texturing-and-shading] Unreal doesn't need it due to Unreal calculating Normal Z-depth on its own under the right compression scheme with a Normal Map with only XY Normals in it, or you do it in a shader if your using say BC7 compression to hold more stuff in one texture and need high quality still, which the normals should be in the Red and Alpha of a texture for if your going that route.
- **2023-04-21** `gameoverload` [🔍╎texturing-and-shading] Also quick explanation on maps:
  
  Albedo: Base Color
  
  Metallic: Mask For Metalness 
  [ Base Color <> Black Environment Reflections ]
  
  Specular: Mask For Specular
  [ Base Color or Specular Color <> Base Color or Specular Color With Environment Reflections ]
  
  Roughness: Surface Roughness
   > Glossiness: Surface Gloss
   [ Opposite Of Roughness ]
   > Smoothness: Surface Smoothness
   [ Opposite Of Roughness ]
  
  Emission Color: Emission Color
  [ Emission Value Raises Brightness ]
  
  Emission Mask: Emission Value
  [ Emission Color Changes Mask Color ]
  [ Can be multiplied by a value for changing Emission Value based on Mask. ]
  
  Alpha: Transparency Cutoff or Blend
  
  Normal: Per-Pixel XYZ Vectors of Baked Down Geometry
  [ Made From High Poly ]
  [ Meant For Mid or Low Poly ]
  [ Z Can Be Removed And Recalculated By Shader ]
  
  Ambient Occlusion: Baked Down Soft Shadows
  
  Custom Mask: Custom mask for special inputs, like for masking procedural materials or other for other custom material purposes.
- **2023-05-01** `hobbes240` [📜╎ripping] Hi, I'm new to this community and well applications like blender and all, however I'm understanding quickly as there are a lot of tutorials out there. One issue I am having that I need help with is ripping the animation for assassinations with Bonobo for Halo Reach then transferring it to Blender. I have seen the tutorial from PlasteredCrab, however one version of Bonobo is telling me that the Reach Files are unsupported, while the other is saying it can not find the file. Bonobo is working for Halo ODST, but isn't working for Halo 3 either. I'm uncertain if maybe the application is no longer working or what. I used this link for .map files https://haloce3.com/xmf/, downloaded all of the shared files for Reach, ODST, and 3. As for the .net downloads, this one works https://www.microsoft.com/en-us/download/details.aspx?id=48130, but this one doesnt https://www.microsoft.com/en-us/download/details.aspx?id=53344. Does Anyone know what the issue could be and a solution for it?
- **2023-05-13** `gameoverload` [🔍╎texturing-and-shading] Okay, so some things I am noticing:
  1 - Make sure to have put in the alpha of the diffuse into the alpha for the haircards.
  2 - Make sure your using rocket_haircards.
  3 - The textures seem to be base color / bc, AO / ao, roughness / ro, unknown / tr, metallic / m, Mask / mk, and others I am not sure about.
  4 - Make sure all of your meshes have auto-smooth on and a weighted normal modifier added to them.
  
  All in all these rips seems not all that good, certain pieces are missing, the mouths and other parts seem to be all messed up in terms of some parts in some parts of meshes, and other parts are their own meshes, textures are missing..all that.
  
  Honestly I even am not sure what needs to all be done to fix those issues since I'd need to deep dive into the tools ( I also don't own the game ) available. 
  
  Really honestly you'd need to debug the game to really know what is to what and what your missing, especially with hair, textures, shading, etc, as none of what I am seeing here is really telling you much nor the tools will most likely tell you much at all if this is all they can pull.
- **2023-05-24** `appoarsin` [💬╎general-help] I know this isn’t a modelling/animation thing I need help with, but I can’t seem to get help on the Halo server or from google with this. I started playing Reach’s campaign for the first time last week. It crashed halfway through the first mission and now I can’t open MCC.
  [attachments] IMG_0276.jpg
- **2023-05-24** `theidiot5651` [💬╎general-help] https://www.youtube.com/watch?v=Jm39-vYl1zo&t=86s
  
  https://www.youtube.com/watch?v=LdmoBY08LTM
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo&start=86 This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan p
- **2023-05-25** `felixvyra` [📜╎ripping] I'm probably going to torrent it. I'm not using it for any commercial purposes and I'm not sure if I'll like it.
  I'm also not a big fan of giving out credit card information when it's not necessary + knowing Adobe they are going to brake it if I pay for it like they did with photoshop and those universal color thing I forgot the name of.
- **2023-05-27** `squishysquidums` [💬╎general-help] I apologize if this is a dumb question, I'm trying to repaint my spartan collection noble team figures to be as close to their in game counterparts as I can get them and I was wondering what the easiest way to get like a turnable t-pose of the in game models for the members of noble team from reach would be. So I can see what decals, stripes, scuffs or whatever else go where but I'm not exactly experienced with this kind of thing so any guidance would be much appreciated. Please and thank you.
- **2023-05-28** `cyq3r` [🦴╎animation] I’m actually working on an odst story that takes place right after escaping reach on the pillar of autumn
- **2023-06-12** `fiycsf` [🔍╎texturing-and-shading] Open Halo 5, as Gallium is getting at, is open in that it's intended to be edited in shader graph. Hell it's commented to a degree that people called me something for
- **2023-06-13** `gameoverload` [🔍╎texturing-and-shading] Right, I guess I will need to figure out how to make a metal mask from the model given, which is the highest quality game ready version available in the developer mod tools.
  
  Given the issue is it isn't like I have any high poly of it other anything, which is why I tried converting the specular map to a metallic map mathmatically in the shader.
- **2023-06-13** `gameoverload` [🔍╎texturing-and-shading] Right, which is the issue, which I may simply have to not really bang my head at by pulling the shader nodes from the original shader that created the specular color from the greyscale spec map instead of trying to make it work in the metal workflow..
  
  Cause I can't really convert what I am trying to convert to metal from the looks of what they explain above.
- **2023-06-21** `guga666555` [📜╎ripping] Hey, just hopped into this server, i'm a modder making a Halo Mod for an different game [Call to Arms] and i need help extracting Halo Reach animations using bonobo, however it says reach maps are not supported once i try to open 'em, tho i remember ripping reach animations a time ago. I dont seem to understand whats happing with the application, so if anyone here does know a solution i'd be really grateful
- **2023-06-27** `thoro.blend` [💬╎general-help] if you are using the reach marine, the bones are still there after the append. They are just hidden for some reason.
- **2023-06-27** `sukimoro.` [🔍╎texturing-and-shading] Looking for someone to help me make infinite masks for the right robot arm from halo reach! if i dont respond DM me im really bad at responding :(
  [attachments] image.png
- **2023-07-05** `setriaphtfk` [📜╎ripping] I’d like to rip a handful of maps from Reach (at least one in particular to make some animations) but I don’t know how, could someone point me in the direction to get started learning how?
- **2023-07-05** `r93_sniper` [📜╎ripping] https://www.youtube.com/watch?v=aYnVWsOZHlE&t
  same idea applies to reach
  [embed] How to port Halo 3 models into Blender (Full in-depth process) https://www.youtube.com/watch?v=aYnVWsOZHlE&start=0 Hey sorry this video is so long, I wanted to stuff in as many examples of the full process as possible to really give you all an idea of what exactly you need to do to get the models lo
- **2023-07-08** `commanderprime` [💬╎general-help] I have just downloaded the Halo Infintie Elite file and I followed the steps to get it from IK to FK (or whatever the opposite of the default is) but when I export it and reimport it to a new Blender file the fingers dont move with the rest of the body.
- **2023-07-08** `ak47fu2` [🦴╎rigging] Not trying to sound really dumb here but how do I repose the elite using this rig? I have no clue how rigging works lol
  [attachments] ElitePose.png
- **2023-07-09** `deltahalocontrolroomcore` [💬╎general-help] Got a quick question-
  
  Doing a render for a couple people that won a championship. One of them has a knife on their wrist, which means I have to unparent it from the original armature and apply it to the new one with new vertex groups and all. Should be the easiest thing in the world. However, the instant I tab into edit mode, the shader *instantly* breaks. I know it's not an issue with Blender, or something I'm doing out of the ordinary (because the Mirage pack can tab in and out of edit mode just fine), which means this issue is present only with assets in Spartan Pack 3.5. Does anyone know a fix or workaround to this issue?
  [attachments] image.png, image.png
- **2023-07-10** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2023-07-11** `pmarc0011` [🦴╎rigging] ok so.... I bet you get this question often here but looking through I didn't find a solution that worked so: 
  I'm kitbashing some odst pieces on the Mirage core made by Dan, I appended all the pieces and once I deleted the rigs of the appended pieces they oriented themselves almost perfectly to the SPI model and its rig. Buuut if I parent them no weight is present; automatic weights kinda work but always leave stuff behind. What do you think is the best course? Just automatic w. and then paint what is missing? Did I forget something? (I'm a noob in Blender 😅 )
- **2023-07-13** `appearedcoast` [💬╎general-help] The one on the right is Chief from the Spartan pack and the one on the left is the multiplayer Chief
- **2023-07-17** `aparttimehuman` [🔍╎texturing-and-shading] Hey would anyone happen to know;
  
  A) How I change the color of the line in Blender 
  
  and B) what texture png this shoulder plate needs to look right in Unity? Every one of the textures listed on it I've grabbed the PNG for and none of them work. (the only personal decal file I could find was "mp_masterchief_personal_decals_cc" which just makes it that blue)
  [attachments] image.png, image.png, image.png, image.png
- **2023-07-25** `mrgruntington` [🦴╎rigging] https://azagwen.gumroad.com/l/skEOo
  [embed] Minecraft simple Promo Rigs https://azagwen.gumroad.com/l/skEOo A pair of Minecraft promo-style rigs of Alex and Steve. Made for Blender, using no drivers and shader-driven face rigs. The faces can be modified to match any skin easily by moving their empty objects in the collection "Shader Empties" 
- **2023-07-27** `tacoco53` [🦴╎rigging] im tryna make an avatar for vrc with the halo infinte elites with halo reach body idk how to make a more simple rig to assign the bones so it can be a humanoid rig in unity?
- **2023-07-29** `aparttimehuman` [🔍╎texturing-and-shading] okay has anyone messed with the H3 Spartan in Unity? The right shoulder makes no sense and I can't get the textures right. When I put mp_masterchief on it, the texture isn't right at all, but when I put personal Decals on it just does this. The back of the piece looks right but the rest of it just looks like this and I don't know how to fix it.
  [attachments] image.png
- **2023-08-02** `purplmunkii` [🦴╎animation] i have an animation from reach and i have the marine model that was uploaded here. the marine pack is using the game rig. how would i copy over the animation to the model rig? retargeting?
- **2023-08-14** `omnipotentdebate` [💬╎general-help] So yeah copy those and drop them into the material on the spartan, then right before the shader viewer add a mix node and plug the shader you just added to the top and the one for the spartan on the bottom
- **2023-08-16** `exitity` [💬╎general-help] I was wondering if there was an easy way to adjust what parts of the armor are colored by the Secondary Color, particularly for the Halo 3 Spartan rig? I was trying to see if I could adjust the coloring to look like the Freelancers from Red vs. Blue Seasons 9-10, where the armor is modeled after Halo 3 put the color patterns are more like Halo 2 (like the armor is Halo 3 armor but their secondary color is coloring those parts on the wrist and the parts on the side of their thighs rather than the shoulders, and also the small bit on the helmet rather than the long stripe). I only know the basics of Blender so if its complicated I probably won't bother, but if its easy that would be cool.
- **2023-08-16** `redthedge` [💬╎general-help] So I'm trying to pose my Spartan (using the Reach pack)
  
  Whenever I try posing a limb, it doesn't work and I'm not sure why. I'm in pose mode, but the rotate gui (the x y z circles) doesn't appear
  [attachments] image.png
- **2023-08-19** `exitity` [💬╎general-help] Is there a way to easily convert the FK rigs to IK rigs? Specifically for the Halo 3 or Halo Reach Spartan blender rigs.
- **2023-08-27** `bl00df4rt` [💬╎general-help] Looking for a good Halo 3 Shader for Unity, specifically for rocks and other tileables as I'm porting a map and I want it to look as accurate to its in-game counterpart as possible. I've looked in shaders-resources and the only  "Halo 3 Shader" I see is the Halo 3 *Spartan* Shader, but since I'm porting a *map*, this shader does not work for me. I am unable to adjust the colors properly and the map ends up not looking right.
  
  I'm asking if the Halo 3 Spartan Shader is the only Halo 3 Shader for Unity, and if there is a Halo 3 Shader that'd be better for maps, I would appreciate being pointed in the right direction.
- **2023-08-28** `koi191` [🦴╎animation] Anybody know if any of the Halo Spartan packs have functioning IK rigs? I was playing around the with Reach pack and it seems to be only FK (which is fine in itself) but the rig is kinda scuffed so that you can't paste a pose X-flipped. Orrr maybe I'm just too much of a Blender noob to use it correctly
- **2023-08-29** `vesper_291` [🔍╎texturing-and-shading] i need help, i have no idea how im supposed to custom color my halo armor. IDK if people do it in substance, blender or somewhere else
- **2023-09-02** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo?si=7V7QIHIQTpvSh9IS
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2023-09-04** `plastered_crab` [📜╎ripping] https://github.com/77Mynameislol77/HaloAnimationRepository
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository A Repository of ripped Halo Animations for use in MCC modding. - GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of ripped Halo Animations for use in MCC m
- **2023-09-10** `adv_eon` [📜╎ripping] Has anyone ripped the textures for Jorge's chaingun (Including normal and control maps) from Halo 5? I want to see if those textures could be repurposed for the Reach version
- **2023-09-15** `r93_sniper` [🦴╎animation] https://github.com/77Mynameislol77/HaloAnimationRepository
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository A Repository of ripped Halo Animations for use in MCC modding. - GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of ripped Halo Animations for use in MCC m
- **2023-09-17** `caldoric` [💬╎general-help] Well, I got a weird one. With the Reach Spartan pack, if you try to bring the chest-knife's bones into Edit mode, the bones themselves (as well as the knife and sheath) suddenly show up at the origin point, between the spartan's feet.
- **2023-09-21** `spukuz` [🦴╎rigging] i'm trying to make a IK rig for the reach elites but the foot doesn't stay on the floor, how can i fix this? 👁️
  [attachments] image.png
- **2023-09-21** `Deleted User` [💬╎general-help] I'm having issue with the Halo Reach marine models, it doesn't seem to have a bone and the female Halo reach marine's hair is missing textures.
  [attachments] Screenshot_2023-09-21_211430.png
- **2023-09-22** `sangeilo` [💬╎general-help] Hello everyone, it's been less than 20 minutes since I joined the discord, but I've been ''playing'' with Halo renders, models and custom themes for years, with the 3ds max, Maya, and other programs.
  I have been downloading models from the page ''https://www.405th.com/forums/'' for years, special mention, since that is where I was able to get everything (a great and wonderful job)
  
  Subject: help with blender textures.
  I have downloaded the Halo Infinte weapons package, and I have exported the models to FBX correctly and functionally in case they have bones, but the textures are not exported, the models are blank, and I have seen several tutorials on how to export with textures, already either by the ''unpack'' method or by Export fbx, path, copy tab and activate the ''embed textures'', and neither works.
  
  The packages have textures but their color is different, and in the blender their color is corrected through nodes or so I think, is there a way to export those correct textures or could someone help me figure it out,
  In the image that I can see below, the texture at the bottom left is what I have, but in the preview on the right it appears correctly, is there a way to export that correct image as a texture
  (I already tried the bake and it didn't work)
  thx for the help
  -my english are so simple, so i use traductor for this-
  [attachments] texture.png
- **2023-09-22** `frockliffe` [💬╎general-help] Hey y’all!
  
  Fenner here! (The Canadian dude from Corridor Digital who’s making that Halo short film) 
  https://www.youtube.com/watch?v=9nlfZE3AZRk&t=638s
  
  Long time no see! My co-creator Jonah and I have been lurking around the last few months while working on our short film Halo: Cold Storage and have made a ton of progress!
  We’re at a point where we’re really happy with our script and have completed the initial animatics and scene blocking for the entire film.
  
  The biggest thing slowing down progress right now bringing shots to completion is the final motion capture and character animation process. I’m admittedly a mediocre at best animator and a worse rigger and have hit a bit of a wall with my capacity to animate the characters in the film to the level of quality I want to achieve.
  
  I know there are a ton of talented artists around here and wanted to see if anybody would be willing to lend a hand with rigging/weighting some characters to work smoothly with the Rokoko motion capture suit we’ve been using for the actor’s performances. 
  Or alternatively in a larger capacity coming onboard the project as a lead character animator.
  
  We’d definitely be more than happy to financially compensate anyone for their work and give full credit as well. 
  
  We want to make sure it’s an enjoyable collaborative process; just a huge fan of Halo and trying to make something awesome to pay tribute to the franchise we all love. 🙂
  
  If anyone is interested in coming on board the project please fee ...[cut]
- **2023-09-22** `plastered_crab` [💬╎general-help] Hey Fenner, I will broadcast this out in our announcements channel as well as send it along to others whom I know do animations so they can reach out if they are interested 🙂 Glad to see the project continue!
- **2023-09-24** `your_average_titan` [🦴╎rigging] (new to this) from what i understand i need to get the shoulder bone thats the parent to the rest of the arm but there is no shoulder bone that works with the arm, basicly im asking what do i put in the shoulder slot (this is the halo reach marine btw)
  [attachments] Screenshot_2023-09-24_151922.png
- **2023-09-29** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo?si=cKIiV8tuybx8RE_G
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2023-10-02** `Deleted User` [💬╎general-help] hey. i'm using MrKoix's Halo CE Spartan model (as I haven't really found an alternative that's IK rigged and has shaders + shield), but I'm unable to figure out how to actually use/enable the shield. How can i turn the shield shader on/off?
- **2023-10-03** `felixvyra` [🔍╎texturing-and-shading] Hey Guys.
  
  I haven't been on this server for some time as I have been doing some other stuff, and although this isn't related to Halo I have a bit of a texturing issue and I don't know a better place to ask for help.
  
  So. The last few days I have been trying to rip some assets from a game called Rainbow Six Siege. It's a long and complex process so I'm not going to go into it right now but TLDR: the guns have different skins and some of the "Lower quality *universal*" skins use a simple image that get's applied to all the guns, instead of having a different base color map for each one.
  My problem seems to be with the UV mapping. In the Images provided below you will see how the gun looks completely fine using the "no skin" base color map but If I try to apply the "Peacock" skin only SOME parts of the gun look correct.
  
  This is super weird and not something I have ever encountered before.
  I tried to look for some additional data that I might have missed but there is nothing. No extra information in the alpha or anything in the ripped files.
  
  Any help would be appreciated.
- **2023-10-09** `banditotortino` [🦴╎rigging] I've been trying to fix a nevk bone on my reach model but for some reason the rotation always gets screwed up is there any fix?
- **2023-10-13** `gunk337` [💬╎general-help] This is more of asking for a favor than for help, but can someone give me the model for the map Lone Wolf from reach for blender? I know there's a lengthy process which can make it look good and like how it does in reach but I'm not trying to do all that right now and I know someone with experience could do it 80x faster than someone new to blender and this whole process.
- **2023-10-26** `lubby94` [💬╎general-help] I managed to get the basics of exporting figured out, but I'm running into a couple of issues. 
  
  - I can't figure out how to import the textures properly into Unity. I got the spartan file from the old Delta Index server before it died, and the way textures are set up is the images are all alphas, and then you edit the colors in "zones," and I have no idea how to properly import the alphas with the color zones into Unity.
  
  - Half of the spartan's right arm is missing for some reason. I don't know if Unity is reading the faces as missing, reversed or what. I merged the armor on the body as one mesh and did the same with the helmet, maybe that messed up the mesh somehow. When I tested importing the spartan into Unity with all of the armor pieces as separate meshes, the arm issue seemed to be fixed.
  
  I'd really appreciate suggestions, but if it takes too long to explain how to fix these issues, that's totally fine. I can just research it online myself.
  [attachments] image.png, Blender__E__Pictures_VRChat_Avatars_Epsilon_Spartan_EpsilonSpartan_3.blend_10_26_2023_5_07_23_AM.png, Epsilon_Spartan_-_Untitled_-_PC_Mac__Linux_Standalone_-_Unity_2019.4.31f1_Personal___DX11__10_26_2023_5_06_23_AM.png, Blender_E__Pictures_VRChat_Avatars_Epsilon_Spartan_EpsilonSpartan_3.blend_10_26_2023_5_02_26_AM.png
- **2023-11-08** `tannerfree` [🦴╎animation] Agreed it’s a much better improvement. Next 2 tips: first related to this specific animation. When you go through and polish your segments of the body try instilling some offset between the chest, legs, arms. Running and walking is a constant state of falling and catching our selfs. Our feet contact the ground and our body falls over top of it. Try offsetting your hip, torso, or chest controllers off even by 1 frame after. Then ask questions like at a persons most extreme poses do the feet reach it first or do the arms? Do they get there at the same time? Then offset accordingly. The offsets will add some dynamics to the motion and make things a bit less uniform.
- **2023-11-11** `sswampp` [🔍╎texturing-and-shading] I don't know what it's supposed to be off the top of my head and don't currently have H3EK installed to check
- **2023-11-12** `agentriker` [🔍╎texturing-and-shading] can someone help me with this issue, i need the thigh on the right to behave like the thigh on the left. a few other armor pieces are having the same issue when i append the shader, but its most aparent here. ive seen this problem b4 i just dont remember how to fix it, ik im suposed to go somewhere and invert or mess with values?
  [attachments] Screenshot_2075.png
- **2023-11-16** `mozria` [💬╎general-help] I have a mod called Halo: Reach - Sky on Fire. It's a Firefight map based on Beachhead that has about a half-dozen fortifications made out of sandbags that are important for the gameplay. However, because they are composed of hundreds of individual sandbag units they are a big drain on map resources.
- **2023-11-16** `plastered_crab` [💬╎general-help] https://youtu.be/Q_LMjZkp1jo?si=HLXDWdE7qbLv0sLz
  [embed] BEGINNER Blender Forge Tutorial | Mjolnir Forge Editor (Halo MCC) https://www.youtube.com/watch?v=Q_LMjZkp1jo Yes you heard me right! You can use Blender to Forge in Halo Reach! This new tool gives the user the ability to manipulate Forge objects in Blender and then "export" them into your Halo The 
- **2023-11-17** `xandion` [💬╎general-help] You can either join the decal to the model or parent it to the bone. Parenting to the object *should* work as you've described, but I haven't needed to do it in 4.0 yet to know for certain.
  
  You could also use the infinite shader's decal slot down at the bottom by just making a .png/.tga texture with your decals and plugging it in there.
- **2023-11-19** `gameoverload` [💬╎general-help] In Blender terms, its a Mix ( Color, Vector, Float ).
  In UE terms, its the same, a Lerp. ( If I remember right. )
  In Unity terms, it depends on whether your using a third party shader tool, or Unity's own native shader tool.
- **2023-11-20** `katyusha117` [💬╎general-help] I'm trying to use a shoulder piece from the Reach assets, I've already used it, but for whatever reason, it's not showing the material right. (I grabbed the same material from the one that's working here)
  [attachments] image.png
- **2023-11-20** `abranca38` [🦴╎animation] I need help. I’ve been trying to animate a walk cycle for a Halo Reach Spartan in Blender, but when I try to paste a reflected pose, it keeps coming out wrong.
- **2023-11-27** `ianstl` [🔍╎texturing-and-shading] Hey all! I'm seeing posts about the broken shaders on blender 4.0, so I'm wanting to port some older assets from this server to 4.0 using the newer inifinite shaders that are compatible. I know the basics of blender/3dmodeling, but I get a little lost in the sauce with the shader nodes. Could anyone help me out with this or point me in the right direction for a tutorial? Thanks!
- **2023-11-28** `palpythefrog` [🦴╎rigging] yes i did, the rig i didnt make, it was from an very old elite pack and i just reused that rig for mine
- **2023-11-28** `palpythefrog` [🦴╎rigging] well it was rigged to the same models, but it had super old inaccurate shaders, poor weight painting in some areas, and didnt include every elite, i just redid the whole thing except for rigging
- **2023-11-30** `mlettuce54` [🔍╎texturing-and-shading] so, issue with the EOD CBRN pipes on DeanThePidgeon's MK V (B) set- baking the textures using SimpleBake seems to override the normal colour with the colour used for the grime? It doesn't do this for any of the other pieces, just this one, anyone have a clue how to fix it?
  
  left is before bake, right is after
  [attachments] image.png, image.png
- **2023-12-03** `purplmunkii` [🦴╎rigging] https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2023-12-11** `weavelcorrosion` [💬╎general-help] I need help right now. (Before you ask, I found the Halo 3 armors. And the blend file is fucking AWESOME!) But I'd like to ask one simple thing, how do I color the spartan, and how do I export only the shown objects in blender? (Blender Newbie here.)
- **2023-12-13** `blue.nasca` [💬╎general-help] Silly blender question - if I wanted multiple Infinite Multiplayer Spartans in one scene how would I go about doing that and having their rigs and stuff self contained? Normally when I copy a spartan from one scene and paste it into another with an already active spartan it seems to mess with the rigs
- **2023-12-15** `mr.chief4199` [🦴╎rigging] Trying to rig an elite, and I have all the bones properly placed (for the most part) and when I go in and out of pose and rest model, they shift terribly, why?
  This rig is the one used in the TauCeti_elites.blend model.
  Why the body is halo 2 anniversary elite.
  [attachments] IMG_4747.mov
- **2023-12-20** `setriaphtfk` [💬╎general-help] There are two ways that I do it and it’s far simpler. If this helmet is directly from the Reach Pack all you need to do is.
  - Delete the rig that helmet imports with it.
  - Select the helmet
  - Select the rig
  - Open the parenting options with the hot key and select “empty groups”
  I discovered this on accident.
  
  If that fails
  - Select the helmet
  - Select the rig
  - Enter pose mode
  - Select the head bone
  - Parent hot key and at the bottom is an option for “bone” it will bind to the bone you have selected currently.
  I found this out when trying to mirror utilities and wrist attachments.
- **2023-12-24** `sora1222` [🦴╎animation] hey i relize there are many talented ppl here if possable does some1 have the Spartan animations files from halo reach
- **2023-12-24** `sora1222` [📜╎ripping] hey i relize there are many talented ppl here if possable does some1 have the Spartan animations files from halo reach
- **2023-12-25** `amsierra` [🦴╎animation] Speaking of! I just re started this project finally, using HR animations. Just remastering the final cutscene. https://www.youtube.com/watch?v=bshivViVWbA
  [embed] Halo Reach: Lone Wolf Remastered in Blender WIP https://www.youtube.com/watch?v=bshivViVWbA This is a remaster of the final halo reach cutscene with noble six in their final moments, recreated in blender and in 4k. Still a work in progress! I plan to add audio effects, post effects (fog, dust, etc),
- **2024-01-05** `tomhogenberg` [💬╎general-help] general help needed with the importing of the sabre from reach, using it as a placeholder/first iteration for an animation. maybe someone knows specifically what causes the texture glitching and behavior with reach assets. its not reliant on the texture(s) and materials applied. anyone has a clue, if this is halo (reach) asset related?
  [attachments] Screenshot_2024-01-05_135220.png
- **2024-01-07** `purplmunkii` [💬╎general-help] if its for private use, like a still or an animation, im sure its fine since its listed as free. if its for anything more in depth id always reach out to the creator and ask for explicit confirmation
- **2024-01-08** `discombobulated2023` [🦴╎animation] https://youtu.be/qkq8HocZUEg
  [embed] MA37 Set https://www.youtube.com/watch?v=qkq8HocZUEg Rig: Bearbones
  MA37 Model: Noe.Vah 
  SFX: Patrick Sutton
  Other Models: 343/Bungie/The Halo Archive
  Music: Halo: Reach - Overture
  
  Originally was for the halo collab but didn't finish before the deadline. Thank you again to Patrick Sutton and Cattag
- **2024-01-14** `vantablonde` [💬╎general-help] Damn, can I post my version of elite models on the download forum? I just added a new texture to the elite model from Halo Infinite and changed the colors, absolutely nothing special, but it looks good.  and I wanted other users to be able to use my option if desired.
  
  On the right is the original, on the left is my work
  [attachments] RaVYrdUTyJ8.png, vUldvneU1dE.png, image.png, image.png
- **2024-01-18** `_tobyr` [🔍╎texturing-and-shading] Hi all, I recently downloaded CanadianDragon's Noble Team models to mess around with and I'm having some trouble with the materials. Most of the model is showing up as white, both in material preview and rendering. For some reason the shins on Jorge's model are colored in fine, but the rest of the model is white.
  It looks like the materials are set up so that the textures provide detailing to the armor, and the color comes from elsewhere. I say this because the diffuse textures have no color, just detailing, with dark highlights along armor ridges and transparent where there is no detailing. These detailing highlights show up on the entire model, but the army green color only shows up on Jorge's shins -- the rest of the model is colored white. This is puzzling because the shins are part of the same model that composes the whole torso & lower body -- the materials and textures cover those anatomy parts. But only Jorge's shins are colored.
  Oh, and side note -- all the stuff that works uses a .tiff textures in the materials. Some of the stuff uses .png textures in their materials and none of that stuff works.
  I have enough experience in Blender to poke around and make general deductions about what's going on, but not experienced enough to do more than that. I also don't fully understand how to work with materials pulled from game files, with the ways stuff gets bundled into different channels of a single texture to be more efficient. If anyone has personal experience and can hel ...[cut]
- **2024-01-19** `frostbitecinematics` [💬╎general-help] yeah. Basically the texture image files are all fair game to use because they are owned by 343, but the actual node logic in Blender used to combine all the properties together (like colour, scratch and dirt intensity, etc) is technically copyright-able by whoever setup the node graphs.
- **2024-01-27** `.reaper64` [🔍╎texturing-and-shading] Excuse me I need some help with a armor texture for Reach, you see I am creating one but I don't know if I am doing it right, if I got the back spine, shoes, and shoulder colors for it to be a secondary.
- **2024-02-02** `acidmanrpgz` [💬╎general-help] Yeah, because Reach and Infinite have similar art styles, but the difference in graphical fidelity is night and day
- **2024-02-12** `clipse3danimator` [💬╎general-help] Hey everyone is the 3D files of reach all rigged?
  
  https://discord.com/channels/819709630540021810/958797302229766234/967826306320445511
- **2024-02-15** `chiefster_4014` [💬╎general-help] I think you'd have to separate the left arm bones of the Reach rig, parent the robot arm to the separated rig, resize it and move it to be where it should be on infinite's model, then join the separated rig it to Infinite's rig. You'll also probably have to delete the left arm bones for Infinite's rig, so they don't get in the way.
- **2024-02-17** `tacticalmeme6640` [🔍╎texturing-and-shading] Anyone know anything about how Ready or Not handles materials? I don't know shit about fuck when it comes to material stuff so I have no idea what I would do with this texture which is apparently supposed to be for metallic, roughness, and AO all at once. The model also came with base color and normal textures and those are easy enough to just throw into the standard principled bsdf since I'm not going for game accuracy, I just need to find out what to do with this
  [attachments] SWAT_Head_Cowboy_OcclusionRoughnessMetallic.png
- **2024-02-17** `atlasguitar` [💬╎general-help] Hey there, I'm just starting out tinkering around with MCC a bit, and am hoping I can get some help or pointers here. I found the H2A Chief in H3 tags on Nexus and was able to get that all sorted in the H3 campaign, and would like to learn how to go about actually making those H3 tags. I was able to find Index and export the Arbiter, but I'm not really sure where to go from there - it looks like when importing to Blender bones and meshes aren't parented (is that the right phrase? lol) but I basically don't know what I'm doing, so if anyone can help me out at all that'd be great :)
- **2024-02-19** `machinesnerve` [💬╎general-help] does anyone else have problems with the current halo reach spartans not being able to properly animate with the rigs they have?\
- **2024-02-20** `trisckit` [💬╎general-help] map porting process via cr4b is good for halo reach/3/odst right
  
  is there an equivalent tool/setup/guide for doing the same with older game maps like halo ce and 2? I mostly just wanna get the old CE version of Hang Em High into blender
- **2024-02-21** `xansworkshop.com` [🔍╎texturing-and-shading] If it's not related to alpha I'd need more info to be useful (and I am distracted by translating the halo infinite shader into unity right now)
- **2024-02-22** `atlasguitar` [💬╎general-help] Can anyone help/point me in the right direction to properly understand basically how to port a model into H3? 
  
  I'm very new, so I'm finding it a bit difficult to find resources that relate to exactly what I want to do. Lots of MP based stuff, but I mostly want to mess with Campaign, so those kinds of things are harder to find than your run of the mill "here's how to make a weapon do X" kinds of tutorials.
  
  Specifically, I'm trying to wrap my head around porting H2A models to H3 (and presumably, the process for actually porting things into H3 in a workable state will be similar for different sources?). 
  
  I've extracted files using Index, everything's good in Blender, but I'm iffy after that. For example, I was able to create a new weapon for H3 that's the H2A Battle Rifle, and the model ports and loads in Sapien, but I can't get first person to work (telling me a matching node can't be found, which sounds to me like I need to have different naming conventions in the skeleton (if that's what it's called for a weapon) to match H3's BR model naming conventions?) and I'm having a particularly difficult time finding resources regarding bitmaps and shaders, so I can't really get the model textured. 
  
  Another thing I was trying was with the H2A Chief model, and the problem I run into there is a "no animation graph" error and I can't seem to find much info about how to port new models to use existing animations.
- **2024-02-22** `chiefster_4014` [💬╎general-help] If you use export the .render_model of the asset you want to replace from the mod tools, you can use its rig and markers, which should be easier than trying to use the ones that come with H2A. You'll just have to make sure the bone names match before replacing. If they don't, you can rename the bone names and they should work.
  You'll also have to make sure your asset matches the scale of H3.
- **2024-02-22** `pendragone01` [💬╎general-help] Hello everyone! I stumble upon this discord while looking for some specific answers about modding Halo, and I'm stoked to be here! Right now I'm trying, in Blender, to change the color of certain shaders on the marine I've ripped from Halo Reach, and I could really use some help. If y'all could point me towards any specific tutorials or documentation on it, I'd really appreciate it.
- **2024-02-23** `xansworkshop.com` [🔍╎texturing-and-shading] Oh right, bit of a status update on the shader garbage here, due to the aforementioned limits of Unity coupled with performance concerns I am now writing a standalone app to bake textures automatically in bulk. This way you don't have to do the tedium of setting up every single material in blender for a bake when it already has proper UVs
- **2024-02-25** `elitecaden` [🦴╎animation] I think the walking animations are bouncy and during the assassination I feel the spartan takes its time to kill the elite
- **2024-03-03** `frostbitecinematics` [📜╎ripping] Anybody know if it's possible to find the exact scale of certain characters using a tool, e.g maybe reclaimer?
  
  E.g in my case, I know that Halo: Reach's Spartans use the same skeleton as regular humans but are scaled by 1.2x. Jorge however is even bigger than the rest of Noble Team. How would I find his exact rig scale value, if possible?
- **2024-03-06** `josephfilmography` [📜╎ripping] https://www.youtube.com/watch?v=opkK2wbyaT4&t=369s
  [embed] Halo Wars Reach mod now with MkIV armour. https://www.youtube.com/watch?v=opkK2wbyaT4&start=369 I made a low poly version of the cinematic model for the MkIV armour and got it all textured and rigged up. There's still some slight skinning issues that need polish but nothing major. Also the backpack 
- **2024-03-07** `coarforge` [💬╎general-help] I also have model export in infiniteExplorer, which exports some material parameters too (though HIRT I think also does rigs iirc, so it might be better to use that for now). If you open a mode (or sbsp) and go to the mesh viewer tab, you can click on `add all Meshes to Exporter` (or go through individual meshes if you want to). Then, under `Tools -> Export current scene`, you can export those models, which will also export some material parameters in a json file in the same directory you save the dae in (some info is currently still missing regarding material styles, but all parameters from the material and shader variant should be there).
  Using `Tools -> Export Bitmaps for current Scene`, you can also export all bitmaps used by materials used by currently loaded models. 
  I know it's unfortunately not very intuitive to use, I'm just really not a UI designer
- **2024-03-08** `frostbitecinematics` [🔍╎texturing-and-shading] In Chunch's halo infinite HIMS shaders, I came across this here which controls the BotColor , MidColor and TopColor blending. Does this serve a purpose or can it be skipped when recreating the shaders in other software? AFAIK the colour can be swapped out by a gradientmap+curve-based camoflague setup but it doesn't require this BotMidTop blending to work afaik. So i'm just wondering what other setups DO rely on this, or whether I can just ignore it and only have 1x colour param per-zone?
  [attachments] image.png
- **2024-03-08** `frostbitecinematics` [🔍╎texturing-and-shading] I know the functionality is in there at the very least for game accuracy since I've seen references to the BotMidTop Color params in some of the material py scripts (in Olympus maybe?). Idk what it gets used for in-game though. It seems redundant because the gradientmap+curve-driven approach gives you the exact same capabilities right? Only difference being ofc that the curve approach lets you use "unlimited" colours, vs BotMidTop which only lets you blend between 3 colours. But if you just add 3x colours to a curve, is the output of that setup basically identical and therefore interchangeable?
- **2024-03-21** `big_squish` [🦴╎rigging] So I downloaded the Halo 4 multiplayer armor asset from below and it’s not weight painted, do I need to weight paint it myself because CATS isn’t working?
- **2024-03-23** `kelcken` [🔍╎texturing-and-shading] Almost have the process for baking these figured out, but running into a few snags I can't quite figure out
  
  First image, bake type is combined, and almost perfect except some areas of the color are darker than others, and there are some strange lines running through the carbon fiber. Figure this has something to do with lighting influence (right now just have the world lighting set up with one of the studio lights so the bake is basically the same as the viewport shading)
  
  Second image, just baking as diffuse, which solves the discoloration issue somewhat (a few issues on some areas of the texture), but the extra stuff (carbon fiber/strap textures) does not appear, so they must be tied to the lighting as well.
  
  Any thoughts on how I can get this working? Been tinkering with it all day, adjusting and testing settings to no avail 🤔 ( did watch the above video to see if they covered anything that seemed relevant, but it didn't seem to offer anything helpful in this scenario)
  [attachments] BaseTorsoBeltex.png, BaseTorsoBeltex2.png
- **2024-03-25** `devil426` [💬╎general-help] So I combined a desert eagle with a halo reach magnum, I think it looks cool but it's a static prop no moving parts that and I don't know how to texture or rig so yeah that's fun
  [attachments] Echos_prototype_handcannon.png
- **2024-03-29** `hydranaught4444` [💬╎general-help] doing shader stuff in blender is always fun ik how to do it in 3 but then i feel like a caveman in blender https://discord.com/channels/819709630540021810/832145913177702470/1028805622008909915😂
  [embed] Discord - A New Way to Chat with Friends & Communities https://discord.com/channels/819709630540021810/832145913177702470/1028805622008909915%F0%9F%98%82 Discord is the easiest way to communicate over voice, video, and text.  Chat, hang out, and stay close with your friends and communities.
- **2024-03-31** `machinesnerve` [💬╎general-help] so ive been moving from sfm to blender and am just so confused, im havinf alot of trouble moving my viewport around and with moving my halo reach models into poses lol, kinda feel like ive made a mistake.
- **2024-04-01** `Deleted User` [🦴╎animation] idk its this vid https://youtu.be/3_BXoz7UtAg?si=YPIC_X7JuANGdY5X
  [embed] RETURN - Halo: Reach Animation https://www.youtube.com/watch?v=3_BXoz7UtAg This short was made in celebration of Halo: Reach releasing on PC!
  Special thanks to Luis Humanoide for another incredible musical piece! Check out his channel!
  https://www.youtube.com/channel/UCp7r4PJY4f81hQPAu_ghbjQ
  
  Suppor
- **2024-04-01** `ubald007` [💾╎autodesk] I don't know much about Maya but in Max if you really need to rerig your already riged character skinwrap is a handy tool, you may need to google and see if Maya has a similar modifier. As R93_Sniper said if exporting from blender save as FBX or Dae either will keep the rig in tact and you should be able to just do your animation then export to Ue..
- **2024-04-03** `finjodon` [💬╎general-help] Hello my Halo Brothers!
  
  Was curious to check in here after rendering my old Reach look :fingerguns: 
  
  Are there possibly blender files that the community's shared where the model is already posed in an immersive environment (perhaps with weapons), where one could essentially trade out the armor & color selections for their own look?
  
  I assume I'd need to ensure that I have both the models (as I currently do) and the Reach weapons downloaded in order to open such a blender file?
  
  Basically just trying to get a cool shot of my old spartan for a pfp!
  [attachments] image.png
- **2024-04-04** `zophynoy` [💾╎autodesk] im not sure if it has, ill find out though. the specific h5 blue team as ik so im assuming it also has fk, however i also have the multiplayer spartan h4 blend file and that one does not even have controls
- **2024-04-20** `its.basil` [🦴╎animation] I was fiddling around with the Infinite Elite Pack 2.2's rig since I like it a lot, ended up making a little animation
  [attachments] Elite_Animation_Test0001-0120.mp4
- **2024-04-22** `Deleted User` [🦴╎rigging] I've rigged an elite and odst before but they didn't work very well. Is there some sort of strategy when it comes to rigging armored models?
- **2024-04-29** `mechacommando` [💬╎general-help] This is a server dedicated to ripping Halo models. Just search “spartan pack” in #📂╎asset-upload-download-forum . I’m busy and I can’t get them for you right now.
- **2024-05-05** `setht1204` [🔍╎texturing-and-shading] https://www.youtube.com/watch?v=y_lRfuCg1NA Seems to be the resource we're looking for. I don't believe there's a way to just toggle different coatings on and off - it needs to be done through the Shader Editor. Little tedious but it's easy enough.
  [embed] Blender basics for making your own Spartan https://www.youtube.com/watch?v=y_lRfuCg1NA This video goes over the very bare bones knowledge needed to have fun playing with cross core ideas and making fun coatings for your dream Spartan in Blender. This will be a very beginner friendly tutorial as well
- **2024-05-11** `profz3d` [📜╎ripping] i have trouble trying to rip halo reach animation
  when i try to import the .jma animation file i got this error
  
  anyone know how to fix this? or what i do wrong?
  [attachments] image.png
- **2024-05-17** `trisckit` [🔍╎texturing-and-shading] okay i forgot i was posting in animation and there's a whole channel here if someone else would know
  this halo reach warthog looks gr8 in cycles with the exception of these metal bits on the turret (looks fine in eevee/material view) , not sure what the cause is
  [attachments] image.png
- **2024-05-20** `spartanatic` [💬╎general-help] ```Make your Halo Infinite Spartan in Blender```
  Tutorial by Plastered Crab
  https://www.youtube.com/watch?v=y_lRfuCg1NA
  
  Posing Tutorial by Plastered Crab
  https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] Blender basics for making your own Spartan https://www.youtube.com/watch?v=y_lRfuCg1NA This video goes over the very bare bones knowledge needed to have fun playing with cross core ideas and making fun coatings for your dream Spartan in Blender. This will be a very beginner friendly tutorial as well
- **2024-05-21** `inversefrog7689` [💬╎general-help] Anyone willing to help me getting this to install to blender? I have the .zip file, but I assume it doesn't install like a normal addon since it did nothing when I went that route.
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2024-05-24** `sneakyzaku` [💬╎general-help] While im focused on finishing up the foundry set, I have had requests for items from halo 2 and reach as well, so if its a multi-purpose tool it makes my life alot simpler.
- **2024-05-24** `gameoverload` [💬╎general-help] The best support for the tool right now is for Halo 3/ODST, their is some support for Reach, but its minimal, and some support for Infinite, but its mega broken.
- **2024-05-27** `bionicemperor` [💬╎general-help] you were looking for the recon helmet interior right? I don't think there are any specific references, but you could take a look at the reach opening cutscene to get a feel for what the interior *may* look like (as the interior for all of Rech's helmets are just the Mk V[b]) additionally if you just need a good look at the neck connector piece cuff thing, the Halo 3 starry night trailer gives probably the best look at the bottom of a mjlonir helmet (to my knowledge) Other than that maybe look at what cosplayers have done or try to reference 3d files made for cosplay
  [attachments] image.png, latest.png
- **2024-05-31** `soma4069` [📜╎ripping] Has anyone ripped the animations for completing a match/leveling up in the original reach? 
  
  So far I was only able to find clips for inheritor and the warrant officer promotion animations so far
  [attachments] RPReplay_Final1717153998.mov, RPReplay_Final1717206687.mov
- **2024-06-02** `zadeinator` [💬╎general-help] https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2024-06-04** `powerhousegaming` [💬╎general-help] is there something I can use to rip CE classic models, I forgot that 343 just repurposed Reach and a few other models, so they aren't actually accurate
- **2024-06-06** `gregory0701` [📜╎ripping] Is there a way to import animations from Halo Reach into blender? I did it in the past but its no longer working with the Halo Dev assets jma importer
- **2024-06-09** `gameoverload` [💬╎general-help] https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl
  [embed] High-level shader language (HLSL) - Win32 apps https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl HLSL is the C-like high-level shader language that you use with programmable shaders in DirectX.
- **2024-06-16** `cythecy_` [🔍╎texturing-and-shading] Just got here after the server was suggested by NotPipi.
  
  I've been successfully pulling models through Reclaimer and it brings along the TIFs for the different image maps. I'm trying to port them into Blender using the AMF importer, successful, but I'm having issues figuring out how to properly set up the shader graphs for various models. CE seems relatively straight forward but later games have different detail maps and such and I just can't figure out how to get it done correctly. I did Boneyard from Reach and the ground looked completely wrong just mixing the different maps together with the correct scaling on each.
- **2024-06-23** `solipsol_studios` [💬╎general-help] Any modders super familiar with CE and Reach able to point me in the right direction in the mod tools?  I just need to aquire info about the games.
  
  Im trying to get data on how Reachs reticle bloom is animated on the hud specifically, in either the code or the tags.
  
  And im trying to get data on why, in CE, the camera angle effects the warthog angle when going off a jump, again info from either the code or tags.
  
  (Sorry i posted this in the wrong channel earlier)
- **2024-06-25** `cythecy_` [🔍╎texturing-and-shading] Just want to double-check, I'm working with Reach assets in Blender.
  Can I assume that all the environment mapping for a single level would be the same?
  So same cubes, tints, roughness?
  I'm using the shader graph found in the Falcon asset, credit says @Soul made it
- **2024-06-28** `lt_crunch236` [🔍╎texturing-and-shading] I hope this is the right place for a question like this, but does anyone know anything about H3EK not letting you change shaders for existing render models? I am duplicating the MP Chief biped to have two different color schemes. But I can’t have the different color schemes because they share the same shader which is locked to the render model. Thanks again.
- **2024-06-30** `freelancermontana` [🦴╎animation] Oh, another question, should I be deleting that "final" pose for now and just keyframing forward pose by pose until I reach it anyway?
- **2024-07-07** `retrogasm2000` [💾╎autodesk] anyone knows how i can properly reflect the halo ce color variant cubemaps into the base elite mesh using the gearbox multipurpose map?
  [attachments] image.png
- **2024-07-07** `freelancermontana` [🦴╎animation] I'm more just trying to get the gestures and whatnot down. I *was* gonna have the background guy's shoulders slump in disappointment before he lowers the gun for a sec, but I can't really get that to look the way I want with this H3 rig.
- **2024-07-09** `juliociraptor` [💬╎general-help] Hello everyone! I'm trying to import animations from the Jackal in Halo Reach into Blender, but I can't. Everything works fine with the Jackal from Halo 3, and I followed the tutorial here on the Discord server and in the YouTube video: 'https://www.youtube.com/watch?v=oIIHsOifz60&t=151s'."
  [embed] Halo Animation Ripping / Porting [Blender] https://www.youtube.com/watch?v=oIIHsOifz60 This video goes in-depth with the process of extracting Halo animations and porting them into Blender. I provide all the useful tools you need in order to do this as well as a .blend file with my Tactical Moa for 
- **2024-07-10** `juliociraptor` [💬╎general-help] "Well, actually I'm creating a player model for GMod, with custom animations from the Halo games. I've managed to import animations into Blender following the tutorial step-by-step. The steps I followed were as follows: I downloaded the Reclaimer and Bonobo programs, and downloaded the ".map" map files to extract models and animations. So far, so good. After obtaining the "amf" model of the jackal and the animations in "JMA," I use a Blender addon to import and export files from Halo games. Here's where my problem with the model and animations from Halo Reach arises. While following these steps, I successfully imported the model and animations of the Halo 3 Jackal. However, when I try to import the model and animations of the Jackal from Halo Reach, Blender shows an error message: "Internal error: <class 'ValueError'>(could not convert string to float: '0,9999999')."
- **2024-07-13** `chiefster_4014` [🦴╎rigging] you can select all of the vertices of your mesh and remove them from whichever vertex group they're not supposed to be in
  if you got the banshee model from Index you shouldn't be having this issue tbh. you also shouldn't have to re-assign the weighting since the H2/H3 rigs are identical iirc.
- **2024-07-13** `chiefster_4014` [🦴╎rigging] if the rigs/names are at least similar, then you could just rename the vertex groups to be in-line with H3. that way you wouldn't have to re-assign anything.
- **2024-08-03** `zangh` [🦴╎rigging] it's a nearly perfect rig! I just had some issues converting phenex's spartan pack to the rig, but I may just need to use different base packs when converting to the rig. but with using it for my latest h3 render, it's pretty much perfect. I had difficulties trying to find the root bone to use the IK/FK switcher for the limbs, but having IK fingers as an option is a godsend
- **2024-08-03** `xenomorph_xx121` [📜╎ripping] Is there a way I can rip the halo infinite animation files? I'm looking specifically for the elite animations
- **2024-08-04** `xenomorph_xx121` [💬╎general-help] I'm trying to import .JMA animations onto the H2A elite model in Blender (4.2) but the transform data keeps stretching out the model (as seen below). Does anyone know how to prevent the stretching?
  [attachments] image.png
- **2024-08-06** `r93_sniper` [Halo Asset Blender Development Toolset] Tool created by General_101 and various others for a full suite of tools related to modding halo. More information in github link
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/ CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2024-08-06** `r93_sniper` [Unreal Engine Blender Tools] Suite of blender tooling for use with Unreal Engine, made by Epic Games.
  __**Requires Linking your Epic Games Account with Github to access the link!**__
  
  Link: https://github.com/EpicGamesExt/BlenderTools
  Send to Unreal YT Playlist: <https://www.youtube.com/playlist?list=PLZlv_N0_O1gZfQaN9qXynWllL7bzX8H3t>
  UE to Rigify YT Playlist: <https://www.youtube.com/playlist?list=PLZlv_N0_O1gaxZDBH0-8A_C3OyhyLsJcE>
  
  Blender to Unreal YT Streams:
  Part 1: <https://youtu.be/c3_xUMQ6hhs>
  Part 2: <https://youtu.be/JwRPDCetfcc>
  Part 3: <https://youtu.be/yOELHvKqiHU>
  
  Release Blogpost: https://www.unrealengine.com/en-US/blog/download-our-new-blender-addons
  [embed] GitHub - EpicGamesExt/BlenderTools: Blender addons that improve the... https://github.com/EpicGamesExt/BlenderTools Blender addons that improve the game development workflow between Blender and Unreal. - EpicGamesExt/BlenderTools Download our new Blender addons https://www.unrealengine.com/en-US/blo
- **2024-08-07** `blackandfan` [UGX Pipeline (Importer)] Io_scene_hw is a Blender addon that can import most HWDE models and animations. Some of the animations come out a little janky, but can be easily fixed with a little work. Project is a WIP. Tool has been created by @stumpy.
  [attachments] io_scene_hw.zip
- **2024-08-18** `moistsaltine` [💬╎general-help] i know nothing of blender im trying to optimise for vrc  im using the spartan pack 4.0 and im currently trying to reduce my mesh numbers when ever i join the meshes the rig no longer works or atleast properly i can get it to controle the under armor peices of the reach models but thats it it doesnt controll the helm or any of the armor is there a step im missing or is there somthing i can do to make it work
- **2024-08-21** `brooen` [LinearAnimator] simple tool to help with animations, tutorial on the github page 
  https://github.com/Brooen/Halo-LinearAnimator
  [embed] GitHub - Brooen/Halo-LinearAnimator https://github.com/Brooen/Halo-LinearAnimator Contribute to Brooen/Halo-LinearAnimator development by creating an account on GitHub.
- **2024-08-23** `elitecaden` [🦴╎animation] You should look at reference of how a ship, especially one that big will move. A good reference I can think of right now is the cutscene of the Pillar of Autumn taking flight in this video: https://www.youtube.com/watch?v=QShcXPD7w2Q&ab_channel=LethalThreat. Also you should lower your Clip End value. You should have only as much as you need or things can start looking weird in your viewport
  [embed] Halo Reach: Pillar of Autumn- Helping Keyes Escape https://www.youtube.com/watch?v=QShcXPD7w2Q
- **2024-09-08** `a1real` [💬╎general-help] does anyone know the canonical height or a rough estimate of the height of a jackal shield? im trying to make a custom jackal shield in roblox and i wanna make it look "right"
  
  also does anyone know where to find the textures/models for jackal shields for reach and 3? i can't find them in the drive
- **2024-09-21** `purplmunkii` [🦴╎animation] the reach marines rigs arent too nice to animate, id go about first making the rig more usable like adding an ik
- **2024-09-30** `autarchkatherlyn` [🔍╎texturing-and-shading] Hmm I was mainly wondering if it wouldnt look too out of place if I say, put it in Halo 3 next to the stock h3 frigate (I'm not familiar with what classesand whatnot) ... I suppose I could get one of the ships from sketch or whatever from mal and see
- **2024-10-01** `frankr2994` [📜╎ripping] Briefly looking I'm surprised at the amount of information and tools available for shader disassembly . 
  I really do need to stop going down rabbit holes though. I'm supposed to be setting guns up in unreal for a game mod that I already have ported in 😅. 
  
  At some point I will be more curious of the processes used in the various model ripping programs that we have for halo. I am noticing that with h2a and h4 any material using multiple textures or having a texture coordinate map will not pull over into blender. It would be truly awesome to get something sorted out to automatically pull some of the more advanced materials in at atleast a basic layout to aid in transfer to unreal for me. My material building knowledge is low so having a reference in blender is basically a requirement.
- **2024-10-04** `scrubbanubnub` [💬╎general-help] trying to help a friend get the h3 prowler with textures into blender, but i ran into an issue trying to test and see if https://github.com/General-101/Halo-Asset-Blender-Development-Toolset is what he needs
  is there another tool for that now or am i just doing something wrong
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2024-10-09** `bongcloset` [💬╎general-help] heya, just starting out with H3 animations and am trying the device Machine elevator guide on C20. In blender, the animation looks good and the physics model, parented bone relative to the bone animates with the render. Imports over no problem to guerilla and I set up the device machine tag. Loading it in sapien has the render and physics model start in the correct positions, but running the eleveator animation leaves the physics model at the start position. Any common errors that could be the reason?
- **2024-10-14** `gameoverload` [📜╎ripping] I can't give you one right now, but its pretty simple thing of going to the material your dealing with in Foundation, look at the shader it references, then go find that in Foundation, and open that shader ( Make sure your in Expert Mode ) then you can see the shader code for that shader from there.
  
  The complexity comes with replicating that material in your program of choice with the shader code you see.
- **2024-10-14** `gameoverload` [📜╎ripping] Well its all depending on the shader and how its built, no one example will work for every one, if you need a DX source code reference, here is the microsoft learn section for all of that information.
  https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl
- **2024-10-16** `freelancermontana` [🦴╎animation] Yeah, I was waiting till I finished the rest of the animation, and I'd call it pretty much complete, so hell yeah. Managed to get this figured out on my own, too...I know that I probably did half of it (the shader magic) *wrong* but it somehow ended up working so I'm not gonna look a gift horse in the mouth.
- **2024-10-19** `.crackonos` [💬╎general-help] Can anyone tell me why the bones look like this? Im using the marine models from the halo reach asset pack, but every time I import it, it looks like this. Help pls? (I hid the model of the marine to see the bones better)
  [attachments] Screenshot_1.png
- **2024-10-28** `purplmunkii` [💬╎general-help] https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2024-10-29** `chiefster_4014` [💬╎general-help] You wouldn't import the .model, there should be a data folder with a .jms file of the Jackrabbit in it. You can import the .jms with this addon:
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account
- **2024-11-04** `cythecy_` [📜╎ripping] I'm using Reclaimer to look through the Reach assets.
  Elites only have one head type (different helmets merged with the armours) and I can't tell if the Spec-Ops is supposed to be using the head or not, because the mandibles stick out if you use it and there's a huge gap if you don't.
- **2024-11-05** `gpro9868` [💬╎general-help] I'm back gang srry but i need help with the halo infinite elite pack bc it says the rig is there but when unhidden it wont show up and idk what to do and i dont wanna learn how to make a whole new rig
  [attachments] image.png
- **2024-11-14** `chiefster_4014` [📜╎ripping] No, but you can import them with this addon. https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/ CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2024-11-20** `gameoverload` [🔍╎texturing-and-shading] Well using the above default material would require baking cause it seems like Godot doesn't have a graphical interface for material creation yet, meaning any custom materials would be actual shader code creation.
  
  Textures wise, the default material supports:
  Albedo
  AO ( Map or Packed Texture, Check the AO texture channel enum if packed. )
  Clearcoat ( Own Packed Texture, red & green )
  Detail Albedo
  Detail Mask
  Detail Normal ( OpenGL )
  Emission ( Mask )
  Metallic ( Map or Packed Texture, Has Its Own Metal texture channel enum. )
  Normal ( OpenGL, like Blender, not like Halo Infinite )
  Refraction ( Map or Packed Texture, Has Its Own Refrac texture channel enum. )
  Roughness ( Map or Packed Texture, Has Its Own Rough texture channel enum. )
  Subsurface ( Mask )
  Transmission ( Mask )
- **2024-11-22** `failedmango` [🔍╎texturing-and-shading] Im wondering if someone can explain something with a normal map to me, perhaps I'm doing something really weird.
  Im working with the H2A Odst model and I'm baking the material into mesh for the purpose of 3d printing using subdivision mult and displace following this tutorial: https://www.youtube.com/watch?v=AOdpGgqIub8
  but the edges are really dull/inconsistent.
  Is it possible to sharpen the edges in the normal map to make it *cut* deeper into the mesh?
  [embed] Convert Normal Map to Geometry (for 3D printing or sculpting) https://www.youtube.com/watch?v=AOdpGgqIub8 If you have a model that fakes detail using a Normal map, it can be interesting to convert it to ‘real’ geometry that you can continue sculpting on, or for use in 3D printing. This shows you how
  [attachments] image.png
- **2024-11-24** `.paleblue` [🦴╎animation] https://www.youtube.com/watch?v=7ukRpYDLEiY
  [embed] Halo Reach | ODST | WIP 02 https://www.youtube.com/watch?v=7ukRpYDLEiY Halo Reach fan-made animation focusing on 2 ODSTs in New Alexandria
  
  WIP 02
  
  This is a fan animation
- **2024-11-30** `x_crimsongalaxy_x` [💬╎general-help] man. building the reach spartan was *way* more intuitive than this, and i'm a blender noob. how am i supposed to do the same with the halo 5 spartan?
- **2024-12-05** `thegamingcow507` [💬╎general-help] So I need a LOT of posed models for a DnD thing (I'm working on a homebrew for me and some friends).  I need a model of all of the Covenant races and their different ranks (the only thing differencing from most ranks is color, but I'm painting them and have acknowledged this), most of the vehicles, and all of the UNSC military uniforms (ODST, standard Marine armor, everything like that).  I already have all of the Mjolnir armor variants (although very unorganized) and some Covenant units.  Is there any easier way of getting all of these files then to just search through the Asset Packs channel list?
- **2024-12-13** `cheezballjones` [💬╎general-help] Hey all - Fairly new here, just trying to figure out some Blender stuff, using Halo Models. I am currently just working on changing the poses and such and now I'm trying to colour the model. I've done so in Blender, using the Sculpt mode and just painting each piece, individually. Now my model is looking pretty cool but I can't figure out how to bake the single image texture of the colors that are on the model. Can someone help me by chance?
- **2024-12-16** `colt556` [💬╎general-help] Dunno if this is the right channel, but I downloaded a couple 'SPI' blender files but none of them work with any of the animations I have for the games, I'm not sure if they simply don't have a game rig or if maybe they require animations from Infinite (I tried H3, Reach, and 4 animations and they didn't work). So I guess my question is if there's a repository for Infinite animations to throw at the rigs to see if they work or not
- **2024-12-18** `x_crimsongalaxy_x` [💬╎general-help] have we got any texture resources for reach spartan rigs, e.g. covie blood that can be slapped onto a spartan model, any armour damage like scratches, carbon scoring, etc?
- **2024-12-31** `8_pin_eren` [💬╎general-help] Hi yall, nice to meet ya! Dunno if the question has been asked yet, but is there a way I may be able to convert the WH40K Space Marine 2 textures from Strix's weapon pack to PBR correctly? Seems like my normal maps haven't worked correctly even after inverting them in gimp to attempt to make em Open-GL, and the metallicroughness are way too light? 
  
  I've had to resort in attempting to bake the textures and got some progress, but they just don't have the same quality as the textures with the 40K shader included in the .blend file unfortunatley. 
  
  I gtg to sleep rn so I'll be unable to respond quickly unfortunatley, but if someone has answer or method on how I could fix the Space Marine 2 textures correctly, or how to bake them right, that's be pretty baller!
- **2025-01-01** `digital33` [💬╎general-help] very new to Blender and i've been modifying an odst to a Metahuman skeleton piece by piece and was wondering if there was any way to speed up my workflow.... 
  I get object from ODST rig, clear vertex groups, add modifier data transfer vertex groups from metahuman skel to object, add modifier armature to object as metahuman skel root, drop object on metahuman skel root (re-adjust proportions) transfer weights to objects and the export.... attempting to take what I learn from this to make a modular Halo Metahuman skel... probably pointless due to the new retarget system in ue5.5.1 but i dunno...
  [attachments] image.png, image.png, image.png
- **2025-01-03** `serfais` [💬╎general-help] Questions on HIMS:
  
  1) Is HIMS worth it for rendering my infinite models, or should I just stick with the normal Halo Infinite - Spartan Pack? It's just a custom shader, right? Last time I even touched anything that rhymed with the word "shader" was for ENBs in FNV, so I'm confused lol.
  
  2) Will HIMS look weird for my Spartan if I render them in a scene with other assets that don't use HIMS, or is that shader only meant for renders of standalone Spartans? If so, is HIMS abled to be easily disabled and reenabled whenever?
  
  3) I want to put together some Infinite Spartans that use Mark-V(B), perfect for the Halo Infinite - Mark V [B] HIMS 3.0 upload, but I want to also give them some accessories from other cores, like knives, pouches, or maybe even a shoulder or two. Will this look strange if I port those accessories over to the HIMS 3.0 version?
  
  4) Might use humgen for my Spartan's faces sometime. Will it look weird if it's with armor that uses HIMS?
  
  Thanks in advance
- **2025-01-09** `gameoverload` [💬╎general-help] So older model, assuming thats from Halo 3 ODST directly, so it might not have as many bones, but I don't quite remember.
  
  However that should not matter, assuming you have set up your bones correctly in the humanoid avatar configuration, ala the one where you can set the bones so that the bones match up with the regular skeleton that VRC uses internally.
  
  However, I would check the skeleton orientation in Blender, making sure its rotated in the correct axis direction, and is applied as well.
- **2025-01-12** `jadedqrow` [💬╎general-help] I can nab Keyes and the elite too if you want tho the elites pose is fucked it's on 343 tho, nuffin can do bout it
- **2025-01-16** `scrubbanubnub` [💬╎general-help] ok so because the discord search is useless and keeps giving me results for `engine` instead of `engineer`, does anyone know where a working h3 odst engineer model is that has support for the animations?
- **2025-01-22** `mrfelixvrc` [💬╎general-help] I need help with this, cause the pack im using is halo 5 multiplayer pack by taco, and for some reason it shows this in unity even though in blender it shows its rigged, with both game and spartan rig.
  [attachments] Capture.PNG
- **2025-01-26** `muskah.mp4` [💬╎general-help] Can anyone help me with getting H3 Master Chief models into Blender?
  I have Blender 4.3 and the Halo MCC Tool kit from Steam. I just don't know how to get started from there. My only experience with Blender so far is making 3D spinning logos, so I'm VERY nooby.
  I'm just looking to make small animations of Spartans to eventually use in Sony Vegas but I have no idea where to begin. Any tips or up-to-date tutorials?
- **2025-01-26** `spartanatic` [💬╎general-help] Models should be available in #📂╎community-upload-download but if you’re using animations straight from H3. You should probably use the game rig.
- **2025-01-28** `midnightzer0` [💬╎general-help] i can color the rest of the body but i literally am unable to change the color of the thighs, the boots, and the oddly specific right calf
  
  model is *Halo Infinite MKVII Core by iExaminar*
  [attachments] image.png
- **2025-02-01** `theishikawarin` [📜╎ripping] (if someone can tell me what colour mode Halo 3/Reach cubemaps are supposed to be set to tho I'd really like that)
- **2025-02-03** `kozmetic` [🦴╎rigging] Wouldn’t that be game rig tho? I’m talking ik, I don’t know how to do that, let alone for an elite model
- **2025-02-06** `kozmetic` [🦴╎rigging] So I need a IK rigged H3 elite but I cant rig it myself (been trying for days) would anyone be willing to help me out? I have the elite set up and all, I just need a rig for it, can anyone help me out?
- **2025-02-13** `thoro.blend` [💬╎general-help] https://discord.com/channels/819709630540021810/1272281885502083265
  
  Im using this odst pack and Im trying to rescale the rig, and when I do the face rig unproportionally scales as well. How would I fix this?
  [attachments] image.png
- **2025-02-14** `kayhotic` [💬╎general-help] wanna try redoing everything proper.
  
  does anyone know how the specular stuff works on the older games? i was told the specular is forward rendering.
  
  all the AC games up until brotherhood have just 3 textures.
  
  A diffuse, A specular, and a normal.
  
  not sure how that translates into a shader proper. especially with it having color spec (sometimes) as his bracer is not supposed to be white like this
  [attachments] image.png, image.png, image.png
- **2025-02-17** `momosenpai` [💬╎general-help] With that said
  
  I've tried looking around on youtube but are there any video tutorials that help you with how to pose IK bones?
  
  I've tried to understand what people are telling me here but its just not clicking for me without some sort of visual explanation to go along with it
  
  Right now I'm looking at the Reach Spartans IK & FK pack
- **2025-02-24** `strider_au` [🔍╎texturing-and-shading] does anybody know if there's a shader setup for assets from Anvil engine (i.e. Assassins Creed, Ghost Recon)? the Mask1 dds includes metallic, roughness and a mask for changing asset colors in-game, but the diffuse is just a detail mask and doesn't really do anything color wise. trying to save myself from a headache cause I haven't really found any good resources to point me in the right direction
  [attachments] image.png
- **2025-02-27** `purplmunkii` [🦴╎rigging] A. You need to scale the armature not the mesh
  B. Select the armature go to pose mode
  C. Id recommend watching this: https://www.youtube.com/watch?v=Jm39-vYl1zo. Its old but still mostly relative
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2025-02-27** `masterj2001` [🔍╎texturing-and-shading] https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/17
  https://projects.blender.org/blender/blender/issues/131605
  [embed] Cycles and surface normals (normalmap shading) https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/17 Somebody else had a very similar complaint/observation to this thread.  Various parts of Cycles will use the geometric normals (or smoothed geometric normals depending o
- **2025-03-03** `iloveagoodcrisp` [💬╎general-help] This is a general question for those who make animations / cinematics in Blender or other programs. I'm building out a system for exporting cinematics from blender to Halo Reach/Halo 4 and I'm unsure of how I should approach creating "cinematic events" (these are dialogue lines, particle effects, scripted events that play in game). 
  
  Should I try to bind these to timeline markers? Or have my own entirely separately menu with these events and let the user specify the frames they occur on? Use the video editing sequencer?
  
  Been thinking lots about the pros and cons of different approaches and struggling to decide what's best
- **2025-03-03** `masterj2001` [🔍╎texturing-and-shading] @Banned .Casey Alright, so I got Gleb Alexandrov and Aidy Burrows to notice this and both Curtis Holt and Andrew Price suggested that I make a post on Right Click Select, which I have now done (yes, I have spread this thing around like wild fire, you'd never suspect that I work in marketing 😂 ) https://blender.community/c/rightclickselect/l8yj/
  
  So I think by this stage, I have done everything in my power to get this heard and seen by the relevant audiences. I'm also waiting to hear back about the MK VI GEN3 model scene being tested within this branch: https://projects.blender.org/blender/blender/pulls/135380
  [embed] Toggleable Shader Normals in Blender https://blender.community/c/rightclickselect/l8yj/ In Cycles, the handling of normal maps currently relies on a combination of mes… WIP: Replace bump correction algorithm to better respect normal maps https://projects.blender.org/blender/blender/pulls/135380 **Pr
- **2025-03-04** `collinbishop` [🦴╎animation] Kinda wondering if anyone knows where h3 animations for FP and TP reside. Wanting to get them into Blender. - AH I see I need to use JMM to blender add-on
- **2025-03-07** `machinesnerve` [🦴╎rigging] if someone is interested im willing to commision for a new ik rig for the halo reach spartans, updated to the one like the infinite spartans
- **2025-03-18** `walpha` [💬╎general-help] Hey, fellas. I've got a rather... interesting issue on my end. So, I made a custom coating using an Ekur export as a base. I've been trying to transfer the coating over to the Spartans in MrGruntington's pack. But... when I try to transfer the exact coating shader to the other armor, it's missing some details. I've posted some screenshots to show what I'm talking about. The first one is what it looks like on the new armor, and the second is what it's supposed to look like. It almost seems like there's some layer of detail missing, but I have no idea what could be causing it, considering I just copied the exact shader over to the other armor. Do you guys have any ideas on how to fix this?
  [attachments] image.png, image.png
- **2025-03-26** `spartanatic` [🦴╎rigging] can someone who is good at rigging help me fix this wrist twist? i've got this driver(?) working for the most part but once i reach a certain twist point it flips.
  [attachments] image.png
- **2025-04-01** `chiefster_4014` [📜╎ripping] for CE, you can use [General's Halo Toolset](https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master) to import the .gbxmodel tags from the mod tools directly into Blender
  For H2-ODST there are commands you can use with Tool to extract .jms files to import into Blender with the same addon. H2 has separate commands for render models, physics, collision and structures, while H3 & ODST you can just use ``extract-import-info``.
  For Reach, H4, and H2AMP, you can use [Crisp's Foundry addon](https://github.com/ILoveAGoodCrisp/Foundry) to import most tags directly into Blender, similarly to CE.
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account
- **2025-04-07** `Deleted User` [🦴╎rigging] Can someone please tell me how to connect the armor from this odst rig to the body? https://1drv.ms/u/s!AmcZRVBcLyYs6mWFRfZj9O3aV_HU?e=5aHKCH
- **2025-04-13** `zangh` [🦴╎animation] as panzer was saying eevee is honestly pretty good for rendering as well these days. especially once you got the light probes figured our you can even get reflections near the level of cycles. especially with some good shader work in, it can make the Reach models pop. left has custom shaders I made, right is vanilla Reach shaders
  [attachments] image.png, image.png
- **2025-04-17** `frankr2994` [📜╎ripping] I will tell you that I used a program called actorcore autorig for rigging and weight painting. I did a Spartan and an elite in it. Spartan required zero corrections to be seemingly perfect (or good enough for a game) and I had to play with the elite slightly. Mainly because the model rips with the elite sort of hunched over and it doesn't like the arch in the back when automatically doing the rig.
- **2025-05-24** `mr.boooze` [💬╎general-help] heya im kind of a blender noob and i was trying to use the halo infinite elite pack for an avatar im working on, but im really stuck on trying to figure this out since the bones are in layers which ive never seen before
- **2025-05-25** `catgod970` [💬╎general-help] I'm using ekur and when i imported my spartan the helmet and shoulder is not the right color please help, dm me or @ me please
- **2025-06-03** `mkz67k90` [🔍╎texturing-and-shading] hoping this is the right spot ^^ im trying to use simplebake and uvpackmaster to atalas all my textures but i cant seem to figure out how to do my maskmaps (its basically my ao, metallics and roughness in one thing).
  
  if i just chuck it in the base color it comes out without the transparency, i couldnt find anything on the internet about it sadly😔
  [attachments] image.png
- **2025-06-11** `devo0501` [🦴╎animation] **Animation not related to Halo**
  https://www.youtube.com/watch?v=kp6QjfOUvsw - Source 2 Animation I've been working since 3 weeks. I did this animation for my boss from regular work, I would be grateful if you guys could leave a like, comment to help this animation reach wider audience ||so I can get a rise lol || thanks
  [embed] Backrooms - Beginning https://www.youtube.com/watch?v=kp6QjfOUvsw The Backrooms Project. Orpheus Engineering Group. This is just a Beginning. This is where it started.
  
  
  
  11/06/[̴̅̏D̵̛͗A̴͐͘T̴́͝A̸̙̾ ̷̿͠...
- **2025-06-23** `crystals_den` [💬╎general-help] For coloring, where there's the line of 4 circles near the top right, go to the 3rd from the left, which will apply all the material shaders and stuff
  Then if you want to change the color of things, go to the "shading" tab in the very top bar (or the + > General > Shading if it's not there already), and if its a model from somewhere else, there's usually already color node trees to play around with (some have drop downs for what actually is the object color)
  And if it's your own model, there's a bunch of easy tutorials you can find for changing the material settings there, and the default node setup it spawns if you make a new material is usually fine for most purposes
- **2025-07-05** `.dyse` [💬╎general-help] I just got a 3D printer and want to get my Reach armor printed out in a minifig pose.  Is there a guide for that?
- **2025-07-06** `solipsol_studios` [💬╎general-help] Since theres so many knowledgeable people in here -
  
  Does anyone know why so many older games like Halo 3, that relied more heavily on the use of normals to imply detail - look crisper, sharper, just generally clearer than modern games that demand every single bump and crevice detail must be actually physically modelled, like Halo Infinite?  Especially when it comes to the textures.  HI gets really blurry compared to H3's clarity.
  
  This phenomenon really stuck out to me when i played Black Ops 2 right after playing BF2042.  In comparison, BO2 was a lot clearer and clean than 2042, which was super blurry in a ton of situations, just like HI.  But in BO2 the model for Harper when in the rain was so damn good, wet and glossy and you could really see every single bump and divot on his uniform thanks to the normals (it seems), it really did look better than 2042.
  
  I dont get why the new games look worse.  : \
- **2025-07-06** `badgerpants1872` [💬╎general-help] Messing with the Infinite prosthetics for use in VRC. Effectively the goal is to wear them like a pair of pants and just alpha mask out the base mesh on the shader side, so it's going to take a good bit of deformation until it fits right.
  
  And I've very quickly run into an issue (feature?) I've never seen before, where half the meshes seems to have all of their transform controls tied to a little orange dot in between the feet.
  This has the effect of causing things like upper arm mesh to swing around wildly instead of rotate in place.
  If someone knows how to go about removing this, what it is, docs to read up on, I'd appreciate it. Thanks.
  [attachments] image.png, image.png
- **2025-07-07** `deadlykenny` [UGX Pipeline (Importer)] I've managed to fix a bug with this where you were not able to import specific UGX files due to an error with their name (Error Below)
  
  ```
  Traceback (most recent call last):
    File "C:\Users\user\AppData\Roaming\Blender Foundation\Blender\4.4\scripts\addons\io_scene_hw\__init__.py", line 916, in execute
      ImportUGX(self.filepath)
    File "C:\Users\user\AppData\Roaming\Blender Foundation\Blender\4.4\scripts\addons\io_scene_hw\__init__.py", line 129, in ImportUGX
      CreateMeshesFromUGX(CreateArmatureFromUGX(name, granny), granny, cached, vertices, indices)
    File "C:\Users\user\AppData\Roaming\Blender Foundation\Blender\4.4\scripts\addons\io_scene_hw\__init__.py", line 225, in CreateMeshesFromUGX
      mesh = bpy.data.meshes.new(names[a])
  ```
  
  The fix for this is to edit In the init PY for the io_scene_hw importer above
- **2025-07-19** `x_crimsongalaxy_x` [💬╎general-help] with Halo Reach rigs, the text better corresponds to each bone, but in Halo 5 it feels more complex to me
- **2025-07-19** `x_crimsongalaxy_x` [💬╎general-help] but then I go to a Reach rig, and as exampled here, I can easily comprehend which part of the arm I've clicked on
  [attachments] nyeh.PNG
- **2025-07-20** `x_crimsongalaxy_x` [🔍╎texturing-and-shading] .....I just realized I totally forgot to specify, the rig I'm trying to decorate with alien blood is a Reach rig
- **2025-07-21** `x_crimsongalaxy_x` [🔍╎texturing-and-shading] if anyone has any relatively easy to follow tutorials for weathering a Reach Spartan rig, I'm all ears 
  (as in, making the armour look scratched, scorched, dented, and generally dirty)
- **2025-07-25** `frankr2994` [🦴╎rigging] Didn't try it
  It can do odd hand layouts lol. I rigged up an elite without much effort in it.
  Id imagine you can add an extra set of arms without much issue
- **2025-07-26** `x_crimsongalaxy_x` [🔍╎texturing-and-shading] if there's anyone who could point me to / knows of any tutorials / how-tos for weathering a Reach Spartan rig, pls @ me & LMK (most of the tuts I could find for tweaking Spartans in Blender was for Infinite rigs. not exactly helpful to me)
  by weathering, I mean: making the armour look scratched, scorched, dented, and generally dirty. taking the overall unpolished look of Reach's armour and amplifying it
- **2025-07-26** `chiefster_4014` [💬╎general-help] AMF doesn't work on the latest versions of Blender. it's also outdated. use the RMF plugin for Blender and export as RMF from Reclaimer instead.
  also, since you're trying to get Digsite stuff I'd suggest just grabbing it from their Github instead of the .map files: https://github.com/digsite
  And then use this addon to import them: https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master
  [embed] Digsite Crew https://github.com/digsite Digsite Crew has 6 repositories available. Follow their code on GitHub. GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master CE/H2/H3/ODST JMS/JMA/ASS exporter 
- **2025-07-31** `x_crimsongalaxy_x` [🔍╎texturing-and-shading] If anyone knows how to or could point me to tutorials for weathering (making the armour look scratched / scorched / dented / dirty) a Reach Spartan rig, please feel free to @ me
  most of the tutorials I come across are for Infinite rigs, which is **not** helpful to me; neither would retexturing an Infinite rig make a worthwhile substitute, as that is not what I am after 
  All I am after is weathering a Reach Spartan
- **2025-07-31** `x_crimsongalaxy_x` [💬╎general-help] how does one recolour the Reach Elites? the pack I have is titled "Elite IK chief elites v1.4", if it helps
  and I would like to know if it's possible to recolour them like one recolours the multiplayer Spartans, as the colour scheme I want is from my multiplayer Elite
  if it's not, is there a different model of Reach Elites available with an IK rig that would allow me to recolour it the same as my MP character?
  (NB: Infinite-Reach Elites are not a substitute I am after. I want straight up Reach rigs)
- **2025-07-31** `x_crimsongalaxy_x` [💬╎general-help] does anyone know the hex code for the glowing bits on the reach elite rigs? i'm assuming that base_illum_cards is the material; i just wanna change the colour cause it doesn't look accurate
  
  these bits i mean
  [attachments] light.PNG, darker.PNG
- **2025-08-02** `x_crimsongalaxy_x` [💬╎general-help] however I now have this teeny tiny black sliver of line thing right near the Elite's elbow and I don't know what it is nor why I can't click on it to delete it
- **2025-08-04** `x_crimsongalaxy_x` [💬╎general-help] think it would make a difference with the fairly bright lights going on around the Elite? I did leave that side slightly darker with that purpose in mind
- **2025-08-08** `uwunice` [🦴╎animation] Think I can ask stuff here? 
  Want to animate a pelican, taking off and making a left or right turn...and looking around for cutscenes in games is a start but I need a bit more angles and longer shots...anybody good a good place to find them? references I mean
  
  Legit thought about doing the reach glitch on new Alexandria mission
- **2025-08-11** `chiefster_4014` [📜╎ripping] here's Reach's magnum animations specifically
  https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main/HaloReach-Xbox/data/objects/characters/spartans/fp/weapons/pistol/fp_magnum/fp_magnum
  [embed] HaloAnimationRepository/HaloReach-Xbox/data/objects/characters/spar... https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main/HaloReach-Xbox/data/objects/characters/spartans/fp/weapons/pistol/fp_magnum/fp_magnum A Repository of ripped Halo Animations for use in MCC modding. - 77Mynamei
- **2025-08-17** `sandwichslayer14` [💬╎general-help] help how do i paint the parts in blue, every time i click them it selects the right thigh armor plate in the shader editor, please tell me what im doing wrong i will answer any questions, im using the Halo Infinite - Spartan Pack 2.0 SOLVED*
  [attachments] helpmediscordpls.png
- **2025-08-18** `boxofspoons` [🦴╎animation] That's totally fair, I've been animating odsts so my head went there immediately.
  
  @Ssloppa 
  went and reviewed the h3 animations and they do hoist it up one handed but it's much more energetic and the launcher falls back down to the left hand. Kinda adding onto what sniper was saying, if you want to keep the grab so animated (kinda like how the grab is on the h3 launcher) then I think you really need to up the movement on the gun itself
- **2025-08-23** `marcopolo1223` [💬╎general-help] Does anyone have experience baking the halo infinite multiplayer Spartans? Having issues where the control rig is preventing it from completing. Probably did something wrong I am following One Weird Guy's Youtube video and have no experience troubleshooting. Any help is appreciated, thanks!
- **2025-08-29** `failedmango` [🔍╎texturing-and-shading] Can anyone familiar with the Halo reach spartan pack (IK & FK fancy feet) explain how to change all the colors of a spartan at the same time?
- **2025-08-31** `djxsam23` [🔍╎texturing-and-shading] Hello! I've been playing around in Blender and in the FrostyEditor for ripping, "trying" rigging and texturing some Battlefield 4 asset and I was wondering what are the MM and D textures are for. I know that AO, CM and N are important for the 3d model but how usefull are the other 2? 
  
  Also I might need help with the texturing. At the moment I'm using the SWBF2_Materials_Version1.1 node graph in the Shader Editor in Blender to apply textures and I'd say it's okay. It does the job but I wonder if it would be better to do a custom one for Battlefield 4 instead?
  
  Thanks if someone can help me!
  [attachments] AO.png, CM.png, N.png, MM.png, D.png
- **2025-09-14** `autarchkatherlyn` [📜╎ripping] you can import the .jms file it gives you using the [Halo Modding Toolset](https://github.com/General-101/Halo-Asset-Blender-Development-Toolset)
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2025-09-19** `endar949` [💬╎general-help] So I've never actually done much with rigged characters before. Downloaded a infinite reach elites pack and the rig is all symmetrical, moving the right arm moves the left arm also, any way to stop this? Also looking for tips to lower RAM usage when adding characters from packs. Should I delete all the hidden armors that im not using?
- **2025-10-06** `misterrturtle` [💬╎general-help] Is there any place i can get some generic poses for the halo reach spartans? or any tips or tricks for getting them into positions/poses easier?
- **2025-10-06** `monokkii` [🦴╎rigging] im having trouble with the halo 3 elite model because it has no chest bone, could someone help me figure out how to add a single chest bone to the model?
- **2025-10-06** `monokkii` [💬╎general-help] im having trouble with the halo 3 elite model because it has no chest bone, could someone help me figure out how to add a single chest bone to the model?
- **2025-10-10** `chiefster_4014` [💬╎general-help] Also iirc General-101's Halo Blender toolset can write lightmap UVs and .bitmap tags for the Halo CE and 2 editing kits for MCC. This could be of some use for extracting the bitmaps from the editing kits.
  GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2025-10-15** `dr.pwoil` [💬╎general-help] oh yup my bad, so if i remember correctly, i tried to :
  - expend the fog to the entire scene cause at first the halo in the background was not in it
  - added a second light outside at a different angle to have less shadow on the top left corner of the halo (so where i have the flickering)
  - added a transparent BDSF with a Mix shader on the window (at least of part of it since if i'm correct the top glass comes with 2 parts)
  - increased the Clip end setting on my camera
  - in the object tab, unchecked the diffuse and shadow checks, then unchecked glossy, transmission and volume scatter as well from both parf of the window in the ray visibility tab
  - Increased samples, i started at 16, tried few frames a 64, did not solved anything, went back to 32 to save some  render time
  - a bunch of other things i can't remember exactly, i was just mostly messing with shaders options but nothing worked so it's back to default
  
  I hope it's understandable, as i said, big beginner here, i'm using blender mostly as a software that allows me to play with models for scenery and animation
- **2025-10-19** `chiefster_4014` [📜╎ripping] We have access to most if not all of the assets thanks to Digsite porting them to Halo MCC's CE mod tools. You can get them from their official Github page here: https://github.com/digsite/h1
  
  Some of the assets you may need General-101's Halo Asset Blender Development Toolset to import, which you can also get here: https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  
  As for anything else that wasn't ported by them we aren't allowed to use or upload. We also aren't able to rip from those builds.
  [embed] GitHub - digsite/h1 https://github.com/digsite/h1 Contribute to digsite/h1 development by creating an account on GitHub. GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for
- **2025-10-21** `_themadmonk` [📜╎ripping] https://youtu.be/TCib8aBLLPE?si=V5lF2nFVllFDFRG6
  
  Hey does anyone know where I could find a corner piece or a T version of the covenant hallway in this animation? Looks like reach assets
  [embed] Halo: Truth and Reconciliation https://www.youtube.com/watch?v=TCib8aBLLPE I'd like to present Halo: Truth and Reconciliation
  A passion project where Masterchief has boarded a covenant ship, just like the mission in Halo Combat Evolved!
  
  This animation is a showing of love for one of my favourite vi
- **2025-10-26** `r93_sniper` [🦴╎animation] the beginning and end of the animation are the roughest and could use a bit of work to reach parity with the rest
- **2025-10-29** `chiefster_4014` [📜╎ripping] For the animations you can use this Blender addon to import them:  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2025-10-30** `gyro.stormblessed` [💬╎general-help] Hey I hope this is the right place to ask this. I am trying to use Soul's halo reach SFM models and I am having trouble trying to change the color of the armor. So far when I change the color it changes it for both the armor and the tech suit underneath, lights too. Does anyone know what I need to be doing to change specifically the primary and secondary colors on the reach armor?
- **2025-10-31** `nihren` [🔍╎texturing-and-shading] Did a full upscale of all the Reach Elite Minor textures and used Diego's Reach-on-Infinite Elite as the test for it(didn't upscale the Infinite textures, I've had issues with those before. Something breaks.) Original on the left with the game textures, not the menu textures. And the upscaled on the right.
  [attachments] image.png, image.png
- **2025-11-06** `chiefster_4014` [💬╎general-help] If I remember correctly you can't with Tool, as the Tool command to export models didn't exist until Halo 2.
  However, you can use General-101's Halo Asset Blender Development Toolset to import the .gbxmodel directly into Blender:
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2025-11-07** `angoose651` [🔍╎texturing-and-shading] howdy
  my friend is trying to turn a spartan model he put together into a vrchat avatar
  it uses the "better spartan pack" and the spartan pack as a base
  hes struggling with the texturing aspect
  im not really familiar with how vrchat avatars work but i figured id ask on his behalf on how to move forward
  I think we need to turn the many different materials of the spartan model into once colour image texture, one normal map, and probably a couple other things idk
  any help would be appreciated
  (btw i think hes already figured out how to convert the rig into a vrchat-compatible one so that whole part is fine)
- **2025-11-08** `chiefster_4014` [📜╎ripping] You can extract the models either from the official Halo CE Mod Tools with [this addon](https://github.com/General-101/Halo-Asset-Blender-Development-Toolset) or extract the models from MCC or another release of CE with Reclaimer. If you rip it from Reclaimer then you have to scale up the model's rig by a value of 32.8084
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2025-11-08** `chiefster_4014` [🔍╎texturing-and-shading] Here's a link to a pretty thorough resource for the main object shader that CE uses. Different versions of CE use different channel orders for the multipurpose map (the red/green/blue texture). Yours looks like it's in the Xbox channel order which the page also describes.
  https://c20.reclaimers.net/h1/tags/shader/shader_model
  [embed] shader_model https://c20.reclaimers.net/h1/tags/shader/shader_model The shader_model tag is used for opaque materials on object models. It supports features like map an...
- **2025-11-20** `exodst.` [📜╎ripping] Does anyone know why when i import the JMA of a Halo 2 elite animation, it gets scaled up unreasonably - like 33.33x as much as it should be? Im trying to export these animations into unreal which will always apply the scale and then the animation will be messed up. I must be missing something. Im using the "Halo Tools Helper" blender addon and got the animations from a archive on github here: https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main/Halo2-Xbox/data
  [embed] HaloAnimationRepository/Halo2-Xbox/data at main · 77Mynameislol77/... https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main/Halo2-Xbox/data A Repository of ripped Halo Animations for use in MCC modding. - 77Mynameislol77/HaloAnimationRepository
  [attachments] image.png
- **2025-11-21** `exodst.` [📜╎ripping] Yeah, i looked all over the internet last night trying to find a response and i believe someone did something just like that, but there is alot of animations i need to go over so i was going to need something more feasible. I tried using the import scale but for some reason it was giving me problems on the original elite skeleton i got from reclaimer, even though it was the exact same as the one used for the animations. Anyway, super glad i got it all figured out.
- **2025-11-21** `exodst.` [📜╎ripping] I just used this tool for importing the JMA files, not sure if thats what you mean but i didnt see that in the import options https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2025-11-21** `koax117` [📜╎ripping] nearest i can tell, is that the new assets are the only ones broken, I tried setting it to the same shader as the other ones (looks like 144 new items-ish), checked the material names, and yeah, new stuff still imports black when trying to change with Ekur. og stuff is fine at least.
  @mrgruntington apologies if you're not the right one to ask, and its someone related more to ekur or the HIMS shader, but any idea why the new content breaks in ekur? Base cores are fine
  [attachments] image.png, image.png, image.png
- **2025-11-22** `sneakyoutlaw` [🦴╎rigging] I need help rigify the reach spartan I'm using the faceless rig in the spartan and so far I'm really lost. I'm currently using 4.2 blender and can't figure it out how to connect them
  [attachments] image_2025-11-22_192550647.png
- **2025-11-23** `g1beongames` [🦴╎rigging] Hello, similar to Snek above me, I too am attempting to create a rig for the reach spartans using rigify. However, at every turn I've run into wall after wall. And I'll be honest, I'm a bit sick of it and my head hurts. So, if anyone could give me a few pointers on how I should be going about this, I'd be very thankful.
- **2025-11-24** `exsurg3_` [💬╎general-help] Anyone that's good with weight painting stuff able to help or fix this? I've been trying to get this hand ripped from the H5 elite pack and onto a infinite rig. I've messed around with the weight painting stuff but no luck.
  [attachments] image.png
- **2025-11-24** `kozmetic` [💬╎general-help] I’m working on a halo infinite series right now, and in it I have animated fight scenes that I’m doing. The issue here, is that apparently with EKUR you can get multiplayer maps, but not the textures for them, so they’re just white. The animation is fine, I can handle that myself, but how would I do the background, because the camera moves around so green screening it would be incredibly difficult.
- **2025-11-24** `kozmetic` [📜╎ripping] I’m working on a halo infinite series right now, and in it I have animated fight scenes that I’m doing. The issue here, is that apparently with EKUR you can get multiplayer maps, but not the textures for them, so they’re just white. The animation is fine, I can handle that myself, but how would I do the background, because the camera moves around so green screening it would be incredibly difficult.
- **2025-11-27** `currently_not_here` [🔍╎texturing-and-shading] pasted since I was in the wrong channel, but I'm trying to enable color on the thigh piece in the newest spartan pack, and plan on enabling color to the other pieces that aren't toggled on, but when I rearranged the nodes and set the settings to where they're supposed to be, the scratch amount is greyed out and the colors on the piece are wonky and look odd, and admittedly, I don't know what I'm doing and would appreciate some help
  [attachments] image.png
- **2025-11-30** `bandido_` [💬╎general-help] Hey, so I'm having a little trouble with the Halo 5 multiplayer spartans pack, is there a setting I have to adjust so that when I'm trying to pose it doesn't just spasm all around the place?
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
- **2025-12-18** `tristanmr` [💬╎general-help] Hello! GMOD dev here, but I was wondering if anyone still had a copy of 3DS or anything of the sort in order to port the animations fully from the original trilogy and Reach as me and my friend are making LVS vehicles functions and we need the animations in order for people to shoot properly in the passenger seats.
- **2025-12-18** `tristanmr` [🦴╎animation] Hello! GMOD dev here, but I was wondering if anyone still had a copy of 3DS or anything of the sort in order to port the animations fully from the original trilogy and Reach as me and my friend are making LVS vehicles functions and we need the animations in order for people to shoot properly in the passenger seats.
- **2025-12-18** `chiefster_4014` [💬╎general-help] There's a repository of animations from CE to Halo 4 here:
  https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main
  I don't know of any addons for 3DS to import these, but for Blender all of the animations can be imported with the Halo Asset Blender Development Toolset addon:
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main A Repository of ripped Halo Animations for use in MCC modding. - 77Mynameislol77/HaloAnimationRepository GitHub - General-101/Halo-Asset-Blender-Development-Too
- **2025-12-21** `gameoverload` [🔍╎texturing-and-shading] This tells ya how all that works in particular, minus the multiplayer stuff:
  https://www.adriancourreges.com/blog/2016/09/09/doom-2016-graphics-study/
  [embed] DOOM (2016) - Graphics Study - Adrian Courrèges https://www.adriancourreges.com/blog/2016/09/09/doom-2016-graphics-study/ DOOM pioneered fundamental changes in game design and mechanics back in 1993, it was a world-wide phenomenon which propelled to fame iconic figures …
- **2025-12-25** `thatmadmoose` [🦴╎rigging] Setting up a rig for an elite and i noticed that the bones are very desynced from the mesh, is there a way to fix this?
  [attachments] Desktop_2025.12.25_-_09.22.33.19.mp4
- **2025-12-28** `gameoverload` [💬╎general-help] Redshift has a shader graph system through redshift.
  https://help.maxon.net/c4d/en-us/Content/_REDSHIFT_/html/Cinema+4D+ShaderGraph.html
- **2026-01-01** `dinoman455` [💬╎general-help] hi there, um idk how to do this so i'll just explain what i'm asking for help with haha. I'm a small Halo Machinima Creator who would like to get into attempted animation of the sorts, i've never really touched blender and the only animation experiance i have is with Mine Imator and that was self taught and i'm only just getting better at it after 4 years of self teaching. Back to the halo stuff, i don't know how to import maps or custom skins for my characters or anything really, i'm kinda starting from scratch and would really appreciate it if i could maybe have a call with someone to maybe teach me the basics as my puny little brain only understands stuff thats been shown to me or explained as i'm doing said work. if anyone could help me out please dm me or reach out or whatever i'd really appreciate the help, also happy new year haha
- **2026-01-04** `solipsol_studios` [🔍╎texturing-and-shading] Ok two small questions.
  (Hopefully easy to answer)
  
  First question:  Im editing Infinites MkV armor in blender, and i have no idea how these textures work.  Where's it get its color and stuff?  Is this like CEs multipurpose maps?  I havent seen texture files like this before.
  (See attached images)
  
  
  Second question:  When editing the CE Chief model in blender, how do you give it color?  I assume its through a node.  I have the hex code for his color, but like how do i apply that.  (Sorry if im just not seeing it)
  [attachments] -417178436_B5C75DF4_-195180619_result.png, -417178436_B776ADC0_-1062373705_result.png, -417178436_FBE075E4_-462036741_result.png
- **2026-01-10** `superintendent5128` [💬╎general-help] Hi! Are there any resources where I can read about animation formats and compression used in Halo Reach? I've been searching for Destiny but as far as I know Reach and Destiny use similar engine
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
- **2026-01-18** `solipsol_studios` [💬╎general-help] Im just (still) trying to add a reflection cubemap onto a preexisting model. Thats all.
  
  Apparently im supposed to use an environment map node thats mapped with the reflection output of the texture coordinates node and do something with the box projection mode in the image texture node.
  
  But i have no idea what that even means in regards to adding it all to the model. I know how to add those nodes to the node area in the shading tab, but like how do i get my cubemap to apply to the model?
  
  And i dont even know if all of that stuff is actually relevant, cause im just piecing together stuff from forum answers that barely made any sense.
  
  I do know however that i need an equirectangular map, ive got that part done already.
  
  I just really need to know exactly what buttons i need to press.  : \
- **2026-01-19** `r93_sniper` [🦴╎animation] <https://www.youtube.com/watch?v=SAVCVc3Gxc0> - Will Christiansen Halo Reach Reel
  <https://www.youtube.com/watch?v=LrvMT3P0DtA> - Richard Lico Halo Reach Animation Reel
  <https://vimeo.com/366025922> - Tom Saville Halo Reach Animation Reel
  <https://vimeo.com/162164458> - Pat Gillete Halo 4 Animation Reel
  <https://www.youtube.com/watch?v=ZwJW9H8ExcQ> - Marcellis Wentz Halo 4 Animation Reel
  <https://www.youtube.com/watch?v=44TEPvvnxWc> - Will Christiansen Halo 4 Reel
  <https://www.youtube.com/watch?v=TfNHwzDBEyM> - Will Christiansen Halo 4 Scattershot
  <https://www.youtube.com/watch?v=fCkHOWpv5Iw> - Will Christiansen Halo 4 Boltshot
  <https://vimeo.com/90479330> - Josh Paraventi Halo 4 Grunt Assassinations (Last Third)
  <https://vimeo.com/151715011> - Mark Tanner Halo 4 and 5 Animation Reel
  <https://www.youtube.com/watch?v=sA9FX4OLa48> - Travis Howe Halo 5 Cinematics
  <https://vimeo.com/169548417> - Joe Garhan Halo 5 Animation Reel
  <https://vimeo.com/145357873> - Daniel Asher Halo 5 Animation Reel
  <https://vimeo.com/280405344> - Josh Edwards Halo 5 VFX Demo Reel
  <https://vimeo.com/147689811> - Nick Whitmire Halo 5 Warzone Intro Reel
  <https://vimeo.com/185665345> - Nick Whitmire Halo 5 Captured Knights Reel
  <https://vimeo.com/160949203> - Patrick Bryan Halo 5 Reel (First Half)
  <https://vimeo.com/242603459> - Kyle Dahl Halo 5 Promethean Soldier Phaeton Animations 
  <https://vimeo.com/234769508> - Kyle Dahl Halo 5 Spartan Bunkering Animations
  <https://vimeo.com/236102780> - Kyle Dahl Hal ...[cut]
- **2026-01-19** `jackmandolino` [💬╎general-help] Hey people, does anybody know if it's possible to get models with rigs of...let's say, Halo 3/Reach to work on Autodesk Maya?
- **2026-01-19** `_spoopyuwu_` [🦴╎rigging] so im using a helmet from an odst pack on the spartan pack, and its not connected to the rig, how do i do that? the rig from the odst pack also imported when i imported the helmet
- **2026-01-19** `superintendent5128` [🦴╎animation] Does anyone have the animation tag binary layout or the tag in binary format from Halo Reach plus the frame count and other metadata? I'd like to see how it works internally.
- **2026-01-25** `.username_ladiesman217` [🦴╎animation] Has anyone tried to retarget animations from the original Halo Games to Infinite's models?
  I have the original rigs from the 360 games, but I'm stuck at the moment.
  My model basically explodes, I gave it a go with the Rokoko plugin, but I'm having some issues.
  Any advice would be appreciated.
  
  At the moment I want to retarget the bubble shield drop from the og Reach game to any of the rigs on the Halo Infinite - Updated Noble Team Pack.
- **2026-01-25** `.username_ladiesman217` [🦴╎animation] I'm going to do that while Unreal is downloading. I also want to make use of the metahumans there, learning a new software is not that hard, right?
  But one thing at a time, first I have to transfer a JMM Reach animation to one of Infinite's rigs.
  
  Thank you both for the advice, it's helping.
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
- **2026-01-29** `.username_ladiesman217` [🦴╎rigging] So, you're going to have to flood it, i,e., make sure the pelvis plate is painted completely red (1.0) for the pelvis bone and completely blue (0.0) for the thighs and spine. 
  And I think that should do it.
  I say red and blue because I use blender, just do whatever the equivalent color is in your software.
- **2026-01-29** `.username_ladiesman217` [🦴╎rigging] The comp and color grade are perfect. 
  I would just suggest to fix the shoulder a little bit, John Halo is resting his weight and pushing himself off the ground, but here it kinda looks like he's shrugging.
- **2026-02-22** `echo_mation` [📜╎ripping] Anyone know if I can pull the classic halo 2 ODST model and rig with reclaimer? I've been trying for a while and can't figure it out. I can only find the bitmap, shaders and ai. (Im using the delta halo map)
- **2026-03-08** `tom3484` [🦴╎animation] Wanted to learn to animate and had an idea for a combat sequence so I looked around and found models here. Learned to rig and did some edits to the reach spartans and elite one I found. I'm overall happy with how it turned out even if it has some jank and wasn't exactly what and how long I wanted. But for my first animation I did way too much so I cut it a bit short. Regardless I'm proud of myself for learning everything and getting it done
  [attachments] SpartanEliteAnim0010000-0165.mp4
- **2026-03-09** `.paleblue` [🦴╎animation] You could do something in the material editor if the hilt and the blade are separate. Adding a mix shader with the other texture being completely transparent and keyframing the value, there's probably a better way to do it, maybe Google might be of much help. Also, animation looks very good so far!
- **2026-03-13** `korrag` [💬╎general-help] ~~I'm having some trouble getting a model setup for use in tabletop simulator.
  
  I'm using the Reach Spartan pack and the Reach UNSC Weapon pack, trying to pose my Spartan with a shotgun and export it as a file to use as a Miniature on the board. I attempted to export as an obj and found out pretty quickly that it doesn't keep the textures and I tried opening the .blend up in unity to the same effect, I tried using the Open Halo Reach Shader for Unity by Cameron "Fly" F but I couldn't get that to actually do anything when I set the normals, diffuse and control map from the Reach Spartans pack. I read somewhere that baking the textures in blender might be a solution but I'm still figuring that out.
  At this point I'm at a loss, any help would be appreciated.~~
  
  Figured it out my own damn self, if anyone needs help porting to TTS via Blender->Unity I will help.
  
  Guide incoming sometime maybe
- **2026-03-19** `dodopro444` [🔍╎texturing-and-shading] Im trying to make the jet engine look that you see on the pelican, and my maps and whatnot are done, however when i move the bones on my rig, for some reason the object moves but the fire effect doesn't move with the object. from what i can tell, the fire effect stays in place and as i rotate the wings and whatnot, it stays while the box leaves it behind when moving. It's done on a shader, face orientation is fine, my maps attached mb for the blurriness hope someone could help
- **2026-03-29** `supermegaaverage` [🦴╎animation] It should theoretically be possible for me to have a Reach spartan fight with a Halo 3 brute right? I already got both of their models and rigs in the same scene
- **2026-03-29** `archival_mind` [💬╎general-help] I don't know if this is the right place but I figured it might be considering people here have delved into the files before and may therefore have any idea about this one. This is a multiplayer track from Halo 5. Has anyone heard it before? I've covered so many bases and am running out of guesses.
  [attachments] 1064725949_online-audio-converter.com.mp3
- **2026-04-04** `chiefster_4014` [📜╎ripping] yes, and they have been already. you can get them from this repo: https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main
  and import them into Blender with this addon: https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main A Repository of ripped Halo Animations for use in MCC modding. - 77Mynameislol77/HaloAnimationRepository GitHub - General-101/Halo-Asset-Blender-Development-Too
- **2026-04-05** `vane.11` [🦴╎rigging] Yall so Idk if the rig is vr compatible do I just remake the rig entirely or is  the default good mind you I'm using halo reach assets
- **2026-04-24** `winnower` [💬╎general-help] 1 — Most of those armors are from different games, so the textures and material setups are different. So yeah, you kind of have to eyeball it.
  
  2 — Both Arbiter armors (Halo 2 and Halo 5) weren’t made to be colorable, so you can’t really recolor them properly, it’ll look wrong unless you do custom work.
  
  3 — Jega’s armor works like any other Halo Infinite character, you just need to find the right materials and color them.
- **2026-04-25** `mardosunday2343` [💬╎general-help] so i downloaded this model and put in unity with all the right textures and when i put them on it looks really broken almost like the UV maps are done for but everything is fine so i asked my friend and he said that it has something to do with like it has like a custom blender shader or something like that but i was just wondering how to fix this because idk how ofc....
  [attachments] Screenshot_2026-04-25_091404.png
- **2026-04-29** `gameoverload` [💬╎general-help] For one, blender FBX does not export textures ( GLTF does though ) however, the problem is that you will need to make sure those textures work in Unity, as Unity's standard shaders require specific textures to work, the other option then to use non-standard textures is to then make your own shader. ( GLSL / HLSL, Amplify Shader Editor, Unity Shader Graph )
- **2026-04-30** `winnower` [💬╎general-help] So it looks like I lied lol, both are from Halo 5. The one on the left is the Arbiter head, and the one on the right is just a normal Elite. Though the left one might also be from the Killer Instinct Arbiter. but still arby head.
- **2026-05-05** `mxndopro` [💬╎general-help] currently trying to use [this reach pack](https://discord.com/channels/819709630540021810/1491122330838106253) (V3 IK) and I wanted to know if there was a specific way i should move the knife here besides manually or if theres a totally dif asset im accidently ignoring in it that just has the knife in that spot 😭
  [attachments] image.png, image.png
- **2026-05-07** `haloismylife650` [📜╎ripping] If you mean H2ASP then you could use the cursed method of exporting the bitmap using tool (e.g., `tool export-bitmap-dds "ui\hud\bitmaps\new_hud\backgrounds\plasma_pistol_bkd" "C:\test"\`) and yoink the blue channel in the image editor of your choice. If you mean H2AMP, you're going to have to use Lord Zedd's FontPackager tool to open "font_package_icon.bin" found in "maps\fonts". When in FontPackager, you can double-click the "fixedsys-ui" entry to find the weapon icons, then right-clicking on the desired schematic will allow you to export it. I believe you're going to have to repeat the same steps for the x2 and x3 versions of the font package so you can import them into their respective font packages for Reach and H4.
- **2026-05-18** `izuen` [💬╎general-help] Hey all! I'm new around here. I'm working on a Halo CE silent cartographer remaster, trying to hit fill film quality. 
  
  Ideally I want to finish before the new game's release date but I still have a long way to go. To save a bit of time I would like to know if someone knows were I could get the following models:
  
  - Marine (Halo CE)
  - Elite
  - Grunt
  
  I really appreciate all the community effort and thanks in advance
  [attachments] image.png
- **2026-05-22** `eidod23` [📜╎ripping] Does anyone have the halo 2 or 3 grunt/elite/marine animation converted to unity? I saw a odst animation here and wondered if anyone got that
- **2026-05-22** `chiefster_4014` [🔍╎texturing-and-shading] What each channel of the multipurpose map does depends on the settings set in the material's shader tag. That one appears to be in the Xbox channel order, so the red channel should be the detail mask, green channel is the self Illum mask, and the blue channel should be the reflection mask. This page should describe the shader tag for this material in more detail if you want to be accurate to CE: https://c20.reclaimers.net/h1/tags/shader/shader_model
  
  I can't recall exactly the setup to use the reflection map exactly, but I know the texture itself must be used with the Environment Texture node instead of the normal Image Texture node so it reflects properly. Iirc for the rest you just need a Geometry Node and Vector Math node set to Reflect. The Incoming and Normal outputs of the Geometry node go into the A and B input of the Vector Math node. The output of the Vector Math node then gets put into the Vector input of the Environment Texture node.
  
  I can send a visual example when I'm home, but that'll be over an hour from now.
  [embed] shader_model https://c20.reclaimers.net/h1/tags/shader/shader_model The shader_model tag is used for opaque materials on object models. It supports features like map an...
- **2026-05-22** `solipsol_studios` [🔍╎texturing-and-shading] Also that webpage seemed to show the multipurpose map separated into different parts, like the alpha channel all by itself.  But i dont have that, all ive got is the combined color file i used as an example.
  
  Am i missing files or does this multipurpose map get separated automatically in blenders programming?
- **2026-05-26** `xander_1993` [🦴╎animation] First Blender animation made for an upcoming Halo 3 montage.  Not sure about the shading/color/etc.  I'm new to this.  Any help is appreciated.
  [attachments] New_skill_unlocked.mp4
- **2026-06-03** `koax117` [💬╎general-help] this was utilizing Foundry's Halo Reach halogram shader. (note that its multiple materials as well, so, that is something to check for the CE version)
  was a long arduous process of locating the tags (in my instance from H3) and copying them into the Reach tags to import via foundry into blender. 
  chiefster's got an updated for H3 that i am working on for my stuff though.
  i don't know how the CE halogram shaders are handled, if any differently. the only other thing I could suggest is either 
  A- install the CE mod tools if you haven't and check the tags for that shader
  B- see if there are any other CE accurate holograms floating around that are set up right to use as a base
- **2026-06-03** `koax117` [💬╎general-help] i'm still working through piecing together a good system for doing looping animated textures, but this is the system I have going from a tutorial I followed, it might give you some ideas since big beam go brrrrr. Chiefster was also a lot of help with this because its running off their H3 shader and they cleaned up the loop a bit more.
  https://youtu.be/0uMWdlz1ecc?si=8TEIpJrL0cjYh3sf
  Based on what I've seen from working with the H3 assets at least, they usually have two different noise masks they animate. that'll potentially help give you a lot of that subnoise in the beam as well.
  [embed] Looping Texture Animations in Blender - Comfee Tips https://www.youtube.com/watch?v=0uMWdlz1ecc Animate any texture's location, rotation and/or scale in a seamless loop. Stylized water shimmer bonus tutorial as well!
  
  Psalm 23:2-3 "He makes me lie down in green pastures. He leads me by still waters.
  [attachments] 20260603-1944-52.0056414.mp4
- **2026-06-03** `roman117` [🔍╎texturing-and-shading] Been banging my head against this for two days: every tutorial i find to try and explain how this works requires either a "base color" node or a "principled BSDF" thing, which this model has neither of. And when I try using the uv unwrapping method where I select a rectangle on the chest where I want the number to be, it just puts a miniature version of the entire texture there.
  
  I tried the shrinkwrapping method and it *works* but the texture is way too bright and stands out way too obviously, and if I pose the model even a little bit the texture gets all warbled and misshapen.
- **2026-06-14** `gameoverload` [💬╎general-help] All right, then I would keep an eye out during the week, since I am working on an MCC menu armor pack. Don't know if it will be done this week exactly, but I should be up to Halo 3 ODST at minimum by the end of the week.
- **2026-06-19** `asrieldream` [🦴╎animation] someone at HS must be inspired by thishttps://youtu.be/_AxMlgPFxao?is=heS5UigLQhS-RLp1
  [embed] Halo (WIP)  - Opening Shot - Teaser Clip https://www.youtube.com/watch?v=_AxMlgPFxao #halo #fananimation #blender3d #halomcc #haloceanniversary #animation 
  
  The opening scene to Episode 1 of my Halo CE adaptation.
  
  The Pillar of Autumn emerges from slip-space after a blind jump, narrowly escaping th
- **2026-06-29** `chiefster_4014` [🔍╎texturing-and-shading] It looks like you have the control map plugged into the color map inputs. That isn't correct unless the material info says that it's supposed to be there.
  Also, you need to change the Alpha setting from Straight to Channel Packed for all textures. If not set, the color of the texture will be multiplied by the alpha channel and can cause the color of the shader and/or the shader itself to turn black, depending on the affected texture.
- **2026-07-01** `kreh` [🦴╎animation] Hi guys, brand new to blender and animation
  
  In progress of making a jump cycle for an elite and something’s telling me I’ve been using the controls incorrectly 
  
  I’ve been moving the pole controls to position the elbows, is that the correct way of doing things?
  
  Plus using IK is a little foreign to me, are there times where I’ll have to use FK? And how would I use FK on these rigs? I’m using the Halo Infinite Elite Pack 
  
  I’d appreciate the help or a point to the right direction :)
  [attachments] Recording_2026-06-30_235330.mov
- **2026-07-02** `winnower` [🦴╎rigging] this is for halo 3 has im porting the reach elites to halo 3 so i have to import the animation anyways in halo 3 the X axis need to point at the bones so the ik can work :´)
- **2026-07-07** `xander_1993` [🦴╎rigging] Is there any updated rigs that I could drag and drop into my Spartan models imported via the Ekur tool or H3 Spartan Pack?  I've seen rigs out there that just look way more simple and eaiser to control.
- **2026-07-10** `alabamanwildman` [🦴╎animation] If you want to get into animation, there's plenty of excellent tutorials out there. I personally wouldn't jump straight into character animation, you're going to want to do basically the whole ass curriculum from animschool.com before you reach for the stars. As for cascadeur, while it does use ai to make things look a bit better, it's not a crutch and people will be able to tell if you used cascadeur for everything
- **2026-07-11** `alabamanwildman` [💬╎general-help] I think that this might be the right deal? https://cults3d.com/en/3d-model/game/grunt-ultra-halo-reach-miniature-stl?srsltid=AfmBOoqOyD7N3vwkQxN7JWWC_u37edDr04cLEDiNqm-L_cgmgdMHS7Sq
- **2026-07-12** `jellybox.` [💬╎general-help] Another option that I use for models that don't have the controls Sangheili Enjoyer is talking about is going up to that top icon (next to 'local,' it looks like a chain) and selecting individual origins instead of median point. Then you can select each bone in a single finger at once, and when you rotate them, it'll curl pretty naturally. saves a lot of time
- **2026-07-12** `fancyneer` [🦴╎animation] Cascadeur was an in-house animation program for the studio maintaining it until they made it public for others to use. The AI tag is a bit of a misnomer and most likely just there to drive up speculation since it's just running off a neural network which has been trained by the studio's own mocap data they've accrued over the past decade. Tech like this has been around for a while, albeit in very experimental states and not as publicly available.
  
  As a beginner, you're doing yourself a disservice for using it as it automates a lot of stuff an animator should be able to do by hand. What are you gonna do when you don't have access to its toolkit in your project due to technical constraints?
- **2026-07-20** `kozmetic` [🦴╎animation] Agreed with the other guys about the recoil, I like the camera movement, another thing I’ll say about the pistol is that (if you wanna be somewhat tactically accurate) have the ODST get his finger off the trigger of the pistol until he’s actually aiming it at the elite, this is a real discipline, followed by shooters to ensure they don’t accidentally pull the trigger and hit something on accident. Also, the animation tab are you animating in with beizer or constant interpolation? Because with constant (pose to pose) you can block out your poses before switching it to beizer (smooth, realistic movement)
  [attachments] IMG_0922.jpg
- **2026-07-21** `koax117` [🦴╎animation] yeah, you can use this -> https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  too import those in JMM files in and either re-assign or retarget the armatures
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2026-07-25** `purplmunkii` [💬╎general-help] totally random, @Surasia i remember you found the psd for the original boxart for ce right? if so do you know about for reach?
- **2026-07-27** `snowdoom` [💬╎general-help] hey quick question y'all, do you all have a simple way to fiix that? i'm trying to fit jega right boot onto a normal infinite elite and it keeps clipping, while the left foot have the exact adjustments and fits perfectly, any tips or help?
  [attachments] image.png
- **2026-07-28** `strafeprime` [💬╎general-help] https://youtu.be/UQVcW13qIeM If someone wants to do a Machinima in campaign evolved, i hope this tutorial helps.
  It explains how to get no weapons and the classic Reach pose
  [embed] HALO CE REMAKE TUTORIAL: Cómo andar sin armas https://www.youtube.com/watch?v=UQVcW13qIeM Este tutorial lo descubrí experimentando con el modo machinima del juego. Recicla las animaciones de Reach y básicamente toma sus ventajas en el modo machinima, aunque es más rebuscado que una combinación espec
- **2026-07-29** `x_crimsongalaxy_x` [💬╎general-help] how do I hide the tiny little UNSC decal on the RECON helmet for Reach, *without* also hiding the triangle decals on the side of the helmet? in my particular pose, the UNSC is floating off the back of the helmet and it ruins the whole thing
- **2026-07-30** `scavhyena` [🔍╎texturing-and-shading] I'm screaming right now in a mixture of joy and frustration for not seeing that input... 
  
  I'll give it a shot and report back after work, see how it worked.
  
  I want to use some plasma impact decals from Halo 3/ODST and my own custom glowing-rimmed one for a fatal beam rifle shot
- **2026-07-31** `starlight_warden` [💬╎general-help] Hey everyone, wondering if anyone can offer some insight. I have Fmodel and am trying to pull all of the elite models from Campaign Evolved. I have all but the minor elite. For some reason Fmodel won't pull most of the Minor armor or skeleton. It gives a detex failed to decode texture error. I also can't find the undersuit that all of the main elites share across Campaign evolved to use in Blender. I'm new to this so still figuring things out.
- **2026-08-04** `zoephie` [📜╎ripping] its in the latest development build https://github.com/Zoephie/Baboon/releases to access the unreal engine features you just load your campaign evolved then select chimp at the top which opens a similar view to fmodel from here just locate any skeletal mesh, or static mesh of your choice which we support extraction of .jms or pskx or if you only need the textures you just find any texture of your choice and right click it and export.
  [embed] Releases · Zoephie/Baboon https://github.com/Zoephie/Baboon/releases A native Rust GUI built on blam-tags for editing Halo MCC editing-kit tag files (Halo 3, ODST, Reach, Halo 4, H2A). Provides byte-exact tag reading and editing, asset extraction (bitmaps, geometry,...
  [attachments] image.png, image.png, image.png, image.png, image.png, image.png
- **2026-08-04** `adorabirb` [📜╎ripping] mostly the weapon animations. cause many of them are pretty much retargetted Halo 5 and Halo Infinite weapon animations with a few being modified reach animations. only ones that are entirely brand new that I can tell is the M7 and Spiker's animations. 
  
  also cause the CEVO BR55 and MA5B are using modified Infinite MA40 and BR75 anims wich would be nice to update my L4D2 ports of those weapons with.
  
  also the idle/posing/fidget anims too.
- **2026-08-04** `adorabirb` [📜╎ripping] pretty much this is what I was able to observe animation wise:
  
  Magnum - tweaked Halo 5 magnum anims 
  M7 SMG - brand new animation set
  MA5B Assault Rifle - modified H:I MA40 animation set
  BR55 Battle Rifle - modified H:I BR75 animation set
  M90 Shotgun - modified H:R M45 animation set with a overexaggerated firing recoil kick
  SRS99 Sniper - modified H:I S7 animation set
  M41B SPNKR - modified H:I M41 animation set
  
  Plasma Pistol - modified H5/H:I animation set (you can tell since the top latch is opening the wrong way for the model )
  Plasma Rifle - possibly H5's Brute Plasma Rifle animation set
  Needler - modified H:I Needler animation set
  Needler Rifle - heavily modified H:R Needler Rifle animation set
  Beam Rifle - modified H5 Beam Rifle animations
  Spiker - brand new animation set
  Energy Sword - modified H:I energy sword animation set
  Fuel Rod cannon - mix of brand new animations and H5/reach animations
  
  Sentinel Beam - modified H:I sentinel beam animation set
- **2026-08-04** `adorabirb` [📜╎ripping] and since it's on the reach fp rig I think (I have to check the files you sent) technically they could be backported to MCC
- **2026-08-05** `zoephie` [📜╎ripping] https://github.com/camden-smallwood/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - camden-smallwood/Halo-Asset-Blender-Development-Toolset: C... https://github.com/camden-smallwood/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to camden-smallwood/Halo-Asset-Blender-Development-Toolset development by creating an account o
- **2026-08-10** `chiefster_4014` [💬╎general-help] I'm more familiar with porting stuff to Halo 4 MCC, but I assume with both games the first thing to do would be to adjust and reposition infinite's model to have similar proportions and the same default pose as CER/H3, then swap the weights to use the weight names from either game. H3 has way less bones in its rig so you'll have to merge some of the weights or redo them. Then you'd bake the textures of the Infinite model into whatever format they need. Both can use PBR textures.
  Them you'd port each model into their respective game. Idk about CER but H3 should be mostly straightforward, you just need an addon to export to the correct format. Them you'd import the textures into the engines, setup your materials, and then that should be it if I'm not forgetting anything.
- **2026-08-15** `winnower` [💬╎general-help] Does anyone know a way to "bake" IK into an animation? I'm exporting the reach elite animations but their feet move a lot while idle so I put the bones of the feet attached to a cube via IK and it fixes it and I would like it to be like that when I export the animations
- **2026-08-16** `temporaryalien` [💬╎general-help] Im slightly busy on a project, but wanted to ask around anyways. is the "!drive" holding halo reach animations for marines and would it be possible to retarget them to a H2 model?
  
  (Reason is just for a placeholder model i want to test around in Unreal for a demo build)
- **2026-08-16** `koax117` [💬╎general-help] can find them here, https://github.com/77Mynameislol77/HaloAnimationRepository
  or extract them with Baboon https://github.com/Zoephie/Baboon
  retarget should work fine, just need to line up the armatures and bones
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository A Repository of ripped Halo Animations for use in MCC modding. - 77Mynameislol77/HaloAnimationRepository GitHub - Zoephie/Baboon: A native Rust GUI built on blam-tags for
- **2026-08-18** `koax117` [💬╎general-help] most commonly i see that error when the shader names or the shader itself has been modified/broken in some fashion.
  the shader names should always remain the string of numbers for ekur identification purposes.
  beyond that, I'm not certain. 
  make sure that when you set up Ekur you didn't set it up for campaign, as the textures won't have loaded. and make sure you followed the full download/deploy instructions. (always stress that for folks)
  [attachments] image.png
- **2026-08-18** `sniped2490` [💬╎general-help] Hey yall I checked the big list of 3d models for any Halo MCC/Fireteam Raven specific models but couldn't find the one I was looking for, I'm looking for the ODST/HVY Variant but specifically the one from MCC I've been looking everywhere for a model of it so I can 3d print it for a cosplay I'm wearing to Halo Fest could anyone point me in the right direction?
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
- **2026-08-20** `gavinvillanueva01` [🦴╎animation] https://youtu.be/QNz_mogWxY4
  [embed] How Halo 3 Should Had of Ended (Halo Blender Animation) https://www.youtube.com/watch?v=QNz_mogWxY4 #halo #halomcc #halo3 #blender3d #blenderanimation #animation #fancreation 
  
  I saw a short on how @SteveDownes117 (Master Chief) Recorded a take on how 343 Guilty Spark was supposed to be killed but i
- **2026-09-01** `doorsday` [💬╎general-help] hello, tried picking back up this hobby a bit and need some assistance
  can anyone explain to me why in the render preview on the left, the helmet changes colors and some attachments and stuff get removed? along with the background not showing up like it does on the right?
  im using blender and the "Reach Spartans" pack i found in a yt tutorial that i can link if necessary
  any help would be greatly appreciated
  [attachments] image.png, image.png
- **2026-09-01** `koax117` [🔍╎texturing-and-shading] okay, yeah, i noticed it too with the ODST helmet. No idea what's causing it exactly.
  easiest solution would be to just append the ODST helmet from the Spartan pack and attach it to that rig, should work fine.
  https://discord.com/channels/819709630540021810/1371126749009547336
- **2026-09-01** `tom_76` [📜╎ripping] hey guys, i extracted an elite for campaign evolved but this dude have no vertex group even if he have the rig, any idea ? i extracted with fmodel in uemodel format
- **2026-09-03** `x_crimsongalaxy_x` [💬╎general-help] so what the hell happens to my ODST's bones when i stick him in the character scene from https://discord.com/channels/819709630540021810/1420976738036486224 ?
  [attachments] wat.PNG
- **2026-09-13** `scalemaster117` [💬╎general-help] I installed multiplayer first as per Ekur's own instructions, and then did a re-dump of the campaign level objects afterwards. From Ekur:
  
  "Are campaign assets supported?
  Yes! However, you do need to set up its own data folder and tick the "Is Campaign" option in the addon preferences, and forge import options will become unavailable." - https://github.com/TheHaloArchive/ekur
  [embed] GitHub - TheHaloArchive/ekur: A multi-purpose blender importer for ... https://github.com/TheHaloArchive/ekur A multi-purpose blender importer for Halo Infinite. - TheHaloArchive/ekur
- **2026-09-17** `gameoverload` [💬╎general-help] Right, but also considering these are Halo 3 parts, might have been good to condense all of that down into one nodegroup that basically sums it down to color change, just like the regular Halo 3 stuff would have.
  
  That way people can change stuff in basic the easiest.
- **2026-09-17** `koax117` [💬╎general-help] your packs are fantastic. its just spelling out little things like this would help. one thing I did in my asset libraries is I made it that if someone opened up the actual .blend instead of just using it as an asset library, there's a tutorial page in the text editor. You could add a window on the default layout that explains things.
  For my comprehensive tutorial, I've got an outline written up going through all the steps. It's primarily focused on Infinite Spartans, setting up Ekur, using the Spartan Pack, selecting attachments, modifying shaders, poses, etc. Though I've thought about doing brief asides for some of the other prominent packs that differ (Reach, H4/5, etc.) - so can definitely include things for yours if you think its warranted
  [attachments] image.png
- **2026-09-18** `tomatoatovt` [💬╎general-help] Ahh, this is from my H3 pack. I must have never noticed this issue. I'll have to get that fixed. It looks like a weighting issue which should be very easy to fix
- **2026-09-20** `yaboipaul` [🔍╎texturing-and-shading] Looking for some help trying to import textures from one of Exminar's awesome asset packs. 
  
  So I am trying to take the textures from his uploaded Warthog model and bring them over to Maya. I have already got the model I want in Maya as an FBX (I don't need the armature) and upon cracking open the texture folder I found that none of them are labeled with defining titles that convey their contents. So I see a lot of PNG's but they are all numbers. In Maya I need to know which ones are for normals, bump maps, specular etc. 
  
  I read online that the way Blender creates color and textures is different from Maya though, so I am not even sure if this is even possible?
- **2026-09-20** `yaboipaul` [💬╎general-help] Looking for some help trying to import textures from one of Exminar's awesome asset packs. 
  
  So I am trying to take the textures from his uploaded Warthog model and bring them over to Maya. I have already got the model I want in Maya as an FBX (I don't need the armature) and upon cracking open the texture folder I found that none of them are labeled with defining titles that convey their contents. So I see a lot of PNG's but they are all numbers. In Maya I need to know which ones are for normals, bump maps, specular etc. 
  
  I read online that the way Blender creates color and textures is different from Maya though, so I am not even sure if this is even possible?
- **2026-09-23** `draken_warlord4769` [💬╎general-help] Default Halo Rigs, atleast in my experience, don't mirror well. They seem to have just too different bones. Like my elite rig, it just doesn't work when mirroring
- **2026-09-30** `azarath__` [📜╎ripping] anyone know how to rip the first person skeleton rig used in the weapon anims in Reach(MCC), im trying to port the assault rifle to BO3 Zombies and need it but cant find it in reclaimer
- **2026-10-02** `r93_sniper` [🦴╎animation] could probably have his torso come down more aggressively. If you see videos people landing after jumping off of stuff like small walls and the likes, you tend to bend your knees a lot more on impact than you think. That guy's flying up about his own height into the air before landing, that would definitely be one of those cases. If you feel that's _too_ aggressive, then you could probably reduce the shift by like half and argue its cus of ODST's exoskeletons in the armor (that is actually a thing btw!)
- **2026-11-01** `chiefster_4014` [🔍╎texturing-and-shading] it's likely from having your Bake Type set to Diffuse. that's not correct for baking textures most of the time. it should be set to Emit/Emission, and you should plug the Bake outputs you need like Color, Unity Mask map and Unity Smoothness Map into the Surface input of the Material Output node on the right. doing that will bake textures correctly.
  to bake your normal map, plug the shader back into the material Output node, and set your Bake Type to Normal.
  Also, for baking, set your Render samples to 1, and turn off Denoise. all those do is increase bake type and don't provide much help. if you want more detail in your bakes, increase the resolution to something like 2048x2048 or 4096x4096.
