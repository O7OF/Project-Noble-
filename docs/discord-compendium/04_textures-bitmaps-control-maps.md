# textures bitmaps control maps

Selected from 1655 candidate messages (ranked by length, links, Halo relevance; capped at ~260 KB). Chronological. Channels: Blender DDS, Halo 5 Data Importer, PhxTools, Reclaimer, Source I/O, 💬╎general-help, 💾╎autodesk, 💾╎substance-suite, 📜╎ripping, 🔍╎texturing-and-shading, 🦴╎animation, 🦴╎rigging

- **2021-03-22** `err_0r` [📜╎ripping] Does anyone with more familiarity with the halo wars 2 assets know if there's any meaningful difference in the assets used for the support drones that jackrabbits and the Forge hog have as part of their upgrade path, and the single unit 'Hank' that follows around the sunray odst team? 
  The support drone isn't in the drive yet but I'm inquiring because I've always been intrigued by the visuals of team sunray, the fact they are wearing a mismash of different odst armors and I'm wanting to recreate the team using higher quality assets and texture work to make it all fit visually, and I would like to get my hands on the support drone so I can get Hank in there.
- **2021-03-22** `err_0r` [📜╎ripping] Alright, basically there IS a way to find the exact color value of green and red that you'd need, but it requires a deep dive into reaches code because just as you can change the color because of the tint color box, there's a line of code in the game files somewhere that stated what hex code was fed into that tint mask for that material that got put on that transport bird. It's a bit of a pain to get those numbers so as long as you can already get color onto the model, you'd be best served trying to match the colors as best you can by hand.
- **2021-03-22** `err_0r` [📜╎ripping] @swagster Chief, generally the way textures work is that textures that aren't color based, as in they are actually a bunch of black to white masks all crammed into one texture, which is normal maps, color mask maps, and often times things like smoothness and metallic, are all stored in linear. The only things still using sRGB are diffuse textures, and the color values of specular data when you're using a specular shader, which is important for us because halo games use a really odd type of specular/metallic hybrid
- **2021-03-22** `err_0r` [📜╎ripping] it seems to me that for reach, the shite in the alpha of the diffuse is both smoothness, as well as a value that gets multiplied with the diffuse rgb itself and fed into the specular inputs
- **2021-04-14** `killzone64` [📜╎ripping] the other advantage is for performance. the game engine only needs to load that object once (as well as its uv coordinates, textures, etc). so you can make hundreds of copies of that object with next to no performance hit. (this is put to great use in halo reach)
- **2021-04-17** `chromacore` [🔍╎texturing-and-shading] Made my own reach shader in blender. Also set it up so I can bake it out to pbr colored spec https://media.discordapp.net/attachments/668244844769968129/833153942346137600/Capture.PNG?width=526&height=701
  [embed] https://media.discordapp.net/attachments/668244844769968129/833153942346137600/Capture.PNG?width=526&height=701
- **2021-04-19** `_gallium` [💬╎general-help] I never looked too far into reach's skin shading because it was honestly primitive, even for its time. At a glance it's similar to specular, but not quite. If you look at it it's whitest where skin oils tend to accumulate the most, so maybe it's masking between two sets of parameters? Hitting it with a frame debugger would be the easiest for me to figure out, but again. Don't really care much for reach's skin shading.
- **2021-04-20** `vuthakral` [📜╎ripping] Hey, I just noticed my old Halo Online elite rips are on the Google Drive for this server, I'm just letting you guys know I fucked up the alphas included on those. Honestly, at best I'd hope to have it removed so people don't use them because of how fucked up they become if you use those alphas for specular masking (the intended thing for them)
- **2021-04-21** `krios24` [🔍╎texturing-and-shading] Found what I can safely guess is one of the only rips of the Halo 4 ODST armor via a gmod addon. The old XBox 360 textures are trash so I’m using it as a test dummy for reacquainting myself with Substance Painter. I remember liking the armor’s look when it came out but not the texture/color scheme they gave it. So I’m trying to re-envision it in the OG style
  [attachments] image0.png
- **2021-04-22** `err_0r` [🔍╎texturing-and-shading] It's good to see someone else who's trying to do right by the H4 ODST stuff. I reached the same conclusion you did about the striping/paintwork being the biggest issue. It doesn't *feel* odst to me, so I also brought it through substance to see if I could get something more acceptable out of it. Admittedly it's some of my older substance work so it's not quite as refined as some of my newer projects but none the less I'm relatively happy with the texturework I got out of it. 
  
  An interesting bit of the project that I've used in some of my newer stuff since is that this is what I call a hybrid texture, combining the original textures with my own. I imported the diffuse as well as grayscaled channels of the specularity/metallic/smoothness, and set it up so it looked as it did in game with shine and everything, and then used the original color code masks to set where my paintwork went. This way I got rid of the horrid stripe nonsense without having to worry about the thousand little details, keeping my layer count down and computer happy.
  [attachments] unknown.png
- **2021-04-22** `err_0r` [🔍╎texturing-and-shading] Now, as a more general question, I know that H2A stuff is a little hard to get a hold of, in particular I'm looking for weapons. I've gotten a hold of an H2A sniper rifle textures which seem to go onto the reach sniper mesh,  as well as the magnum posted here in the server. My issue is that I'm having a hell of a time trying to figure out how to make these things look right in unity. I'm in the process of trying to cobble together shaders for it using Amplify shader editor but I'm not sure what data is what when it comes to these textures.
  For both weapons I've got an RGB (diffuse/albedo) and an RGB (spec?), as well as the normal map which I've already made sure is converted to Unity specs. 
  My question is if anyone's got a good idea of how these textures end up making the end result in the game. If I use the 2nd rgb map as specular what I find is that some areas are much too dark while others are far too white, and of course there's no roughness or smoothness. I've seen plenty of halo texture sets stick smoothness/roughness in the alpha of the diffuse while unity tends to stick it in alpha of the spec or metallic map but in this case there's no alpha anywhere and none of my attempts at figuring out what data is what is giving me visually satisfactory results. Does anyone have any good ideas about what the weapon shaders from H2A look like, how they work?
- **2021-04-22** `fiycsf` [🔍╎texturing-and-shading] What you are talking about is much simpler to do then you likely think. 
  
  Especially with a program like substance.
  
  I believe I have a practical example on my drive, where I redid the mark 6 to have the black zones we'd expect on the armor that were missing.
  
  I know the colour changing Spartan I've co put together in VRChat uses that control.
- **2021-04-26** `lobbster` [🔍╎texturing-and-shading] Would someone mind giving me a hand throwing some textures on the Jorge Turret from Reach? I think I have all the ingame textures but I'm absolute garbo at adding multiple textures to something
- **2021-04-27** `err_0r` [🔍╎texturing-and-shading] Alright, so it looks like you're trying to use the color maps for the mcc h3 odsts? I was working on getting those bad Larrys into Unity maybe three days ago. Comparing what I understand are the mcc menu textures to those of the original game, I don't see any functional difference in the shader used, they just seem to be of a higher resolution compared to what was in game.
- **2021-05-01** `the_heavynator` [📜╎ripping] would it be possible to implement multi uv and vertex color extraction for bsps? halo 4 and h2amp make extensive use of vertex colors for overlay meshes and multi uvs for better detail map alignment. halo 3 and reach also makes use of uv2 for its water masks/main color.
- **2021-05-01** `gravemind2401` [📜╎ripping] speaking of shaders, is most of the texture assignment done at shader level in h4? h3 had the shader templates which specifies which textures got used how, but all h4 has is the texture list
- **2021-05-06** `allindawoods` [💬╎general-help] "chief — 03/31/2021
  Halo: Reach Elites with all armors, pre-setup textures, rig and shading.
  https://drive.google.com/file/d/1lEKWe4WHANJD4DmsHLjIsyq9_XPRi6tC/view?usp=sharing"
  how do i export these elites with their materials??
  [embed] Reach Elites.zip https://drive.google.com/file/d/1lEKWe4WHANJD4DmsHLjIsyq9_XPRi6tC/view?usp=sharing%22
- **2021-05-08** `soulburnin` [🔍╎texturing-and-shading] If I were doing the textures, I'd do some of the following:
  
  1) Increase the base roughness so it's not 90 - 100% glossy
  2) Vary some of the roughness sizes
  3) Introduce some more color variation; doesn't have to be too much, but enough so the paint doesn't look 100% the same
  4) Vary the edge wear. Think about where it should be and where it shouldn't. Should occluded areas have the same amount as outer plates? Should each edge have the same size of wear pattern?
- **2021-05-08** `pointdevice` [💬╎general-help] for baking, use a diffuse and an emissive shader, make a blank texture for the output, and have an image texture node selected on each material that goes to that texture, use diffuse for normal maps, and emit for any color/data, some stuff with doing linear color space gets a little iffy, and will need some messing around with.  for baking you have to set the project to cycles, and then there is a bake menu in the render properties.
- **2021-05-12** `nobleofdeath16` [🔍╎texturing-and-shading] Texture wizards! I am at a standstill on a mod. After comparing the Halo Reach Shotguns texture files with what I have from Revenge's Double Barrel Shotgun, im not sure how to proceed. The reach only uses a normal zbump and a diffuse for all the weapon models but im looking at a lot more in his textures here.
- **2021-05-13** `cmdrbrannick` [💬╎general-help] Hey, was wondering, do we have the Halo Reach Jackal Shield available? I did find a model in MEGA link, but the texture doesn't seem to work. Wasn't sure if I should ask here or model-request.
- **2021-05-14** `nobleofdeath16` [🔍╎texturing-and-shading] Would anyone in here be interested in doing the texture work for a Stanchion model? It would be for porting into Halo: Reach, and is a model provided by @SkookumProps. We are planning the separation of the parts right now for the nodes to go over the Sniper Rifle model.
- **2021-05-15** `soi_7` [🦴╎animation] https://twitter.com/AbiSV1/status/1358801256584646657?s=20
  [embed] https://twitter.com/AbiSV1/status/1358801256584646657?s=20 Halo 2 Elite (HD) short animation + turnaround
  
  This project was a collaborative effort between some members of the community, credit to:
  
  @Stu_FX for the wonderful animation work
  @Jafet_Meza for the awesome music
  @EzzellVFX for the floor te
- **2021-05-23** `nobleofdeath16` [🔍╎texturing-and-shading] Thats awesome! @Kalvlidan did the texture work for this gun ive been fiddling with. He did a fantastic job on it; but its just how finnicky reach is with PBR is what makes it difficult
- **2021-05-23** `fiycsf` [🔍╎texturing-and-shading] If the two of you intend for it to be for reach, carefully swapping the substance project's workflow to something closer (the shader within substance will need to be changed too) will make it much simpler for you to get a result you're content with.
- **2021-05-23** `nobleofdeath16` [🔍╎texturing-and-shading] So what ive been told by @Surasia is Reach doesn't use specular. So then what workflow DOES reach operate off of is my next question. I have the bitmaps from the normal sniper and they are vastly different than what Kal made.
- **2021-05-24** `the_heavynator` [🔍╎texturing-and-shading] @NobleofDeath16 so the issue with making textures for reach (and by extension every other game that predates the pbr standard) is you have to think about the textures in a fundamentally different way. 
  with pbr the base/diffuce color represents the absorbed and scattered light and the specular represents the reflected light and roughness/smoothness is self explanatory and the majority of the final look requires it to be lit in some way.
  with reaches lighting and shading system the "base_map" represents a vast majority of the final look of an object with no attention payed to whether the texture makes sense under real lighting conditions, the specular in this case is only defining how shiny parts of that texture are. this system is so basic that most textures "specular" is a gray scaled version if the texture shoved into the alpha.
- **2021-05-24** `the_heavynator` [🔍╎texturing-and-shading] so in my various pokings with some materials in reach i've (as well as @PointDevice & @ChromaCore ) been able to determine what a lot of the material values do which might be useful.
  Albedo color: tints the albedo (clamped 0-1).
  analytical_specular_contribution: defines how bright the direct specular highlight is
  approximate_specular_type: nothing as far as i can tell
  diffuse_coefficient: defines how bright the final diffuse color will be 
  specular_coefficient: defines how bright the final specular color will be (direct, fresnel, env and area)
  specular_tint: tints the specular (clamped 0-1).
  fresnel_color: defines the color of the specular fresnel effect.
  fresnel_curve_steepness: defines the strength of the fresnel effect (not quite sure of the type of effect it has on the fresnel however)
  area_specular_contribution: defines how bright the area specular effect is (appears to make the specular map glow with the color from the indirect diffuce light). 
  environment_map_specular_contribution: defines how bright the cube map reflection is.
  roughness: appears to do absolutely nothing as far as we could tell
  analytical_roughness: defines the roughness of the direct specular highlight (uses the beckman ndf so most visual change is anything below .5)
  albedo_blend: defines how much the diffuce tints the specular
  env_tint_color: tints the cube map reflection color
  env_roughness_offset: a *verry* course control of how blurry the cubemap will appear (values from 0-0.5 have no visual chang ...[cut]
- **2021-05-24** `pointdevice` [🔍╎texturing-and-shading] reach's shader is pbr, its cook torrance
  metallic and specular can both be pbr, and metallic gets converted to specular in the shader
  reach uses specular roughness, but the roughness isnt exposed to a texture, its just a flat scalar in the material properties
- **2021-05-24** `the_heavynator` [🔍╎texturing-and-shading] the reason they use a "roughness" value is that's the name given to it simply because its a microfacet model. also specular in the context of reach and most games pre 2013 is its literally referring to how shiny parts of the texture are
- **2021-05-26** `williaso` [🔍╎texturing-and-shading] How much work would it be to convert a Reach texture into PBR? Like, all the information is there, right, it's just a matter of mucking with it in Photoshop or something?
  
  Or maybe I just have no idea how textures work and that's not possible
- **2021-05-28** `vuthakral` [📜╎ripping] I was loading m020 specifically, but I meant that the 360 map was missing elite zealot textures, and I don't recall ever seeing the zealot ingame in Halo 4 (granted I havent played Halo 4 long enough to really say that with any level of authority), but loading the same map from MCC provides its textures
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
- **2021-06-22** `celtyshy03` [🔍╎texturing-and-shading] https://drive.google.com/drive/folders/1PL6FAVgrliM562oMTtX7-KCiynrX_e_S in here are the MK VII Chest plates for male and female and I need help texturing it, it needs a UV map and needs a normal and color map and a standard white texture with the detail like all the other halo textures, if someone can do that for me that would be amazing and nice as IDK how to do textures from scratch, I need the chest plate too look like the image kinda but able to keep that halo ingame feel, and if someone can delete the vertexes that wont be seen if on a model that would be nice so I can drop the poly and vertices count.
  [attachments] 881b3e4791459d6e148fb08cc3a4b8a5.png
- **2021-06-26** `chiefster_4014` [📜╎ripping] the textures for the multiplayer spartans didn't look all that different from the in-game ones last i checked, other than slightly less compression and some not having alphas
- **2021-06-26** `bifflive` [💬╎general-help] Hey guys quick question. So I’m completely new to blender and making 3D art etc. so if I want to change the color of a Spartan model or an elite would I have to change the color of the texture in photoshop or just add color to the model itself in blender? Also any tips would be very appreciated! Thank you!
- **2021-06-26** `surasia` [📜╎ripping] From R93_Sniper:
  halo 4 doesnt have a dedicated CC map
  it has a control map, and there's 2 types
  the one for Multiplayer spartans is as follows:
  R - Specular
  G - Diffuse
  B - Gloss
  A- Color Change
  for the Color Change, White is Primary Color, Grey is Secondary, Black is Uncolorable
- **2021-07-03** `fiycsf` [📜╎ripping] Ripping from a game you can buy and pay for should be considered differently.
  
  You can buy the game, and artists and devs were paid for their work.
  
  When you take models from a mod like that, you are taking something you didn't pay for, that you were effectively granted a specific use for, for free.
  
  In most cases, if the author is cool with anyone playing with them, they just put them up somewhere if asked.
  
  Eg; https://sketchfab.com/3d-models/halo-reach-ma37-assault-rifle-c494ab2a4acb439ea626319aa7224b6c
  
  ***TLDR, I also wouldn't recommend.***
  [embed] Halo Reach - MA37 Assault Rifle - Download Free 3D model by Noe.Vah... https://sketchfab.com/3d-models/halo-reach-ma37-assault-rifle-c494ab2a4acb439ea626319aa7224b6c Faithful (I hope) recreation of the Halo Reach MA37 Assault Rifle
  Model (High and ‘Low’ poly) by: Soda (https://www.artstation.com/u65
- **2021-07-18** `surasia` [🔍╎texturing-and-shading] Texture importing to reach is super easy, you can use forerunners tool to convert DDS into .tagc and just import it into assembly with your custom name
  
  Also that's pretty interesting
- **2021-07-18** `pointdevice` [🔍╎texturing-and-shading] No this seems to be a leftover feature from halo3 beta, if i had to guess vram, or overhead from texture samplers would have been the reason they didnt in either games retail version
- **2021-07-18** `surasia` [🔍╎texturing-and-shading] Quote: --- Original message by: Waffles
  There are two types of control bitmaps, type 1 is for materials with an RGB/ARGB diffuse map. Type 2 is for materials without a separate diffuse map, the diffuse maps are grey scale and stored in channel data.
  
  control type 1 bitmaps
  R=specular
  G=specular-exponent
  B=reflection
  
  control type 2 bitmaps
  R=specular/reflection
  G=diffuse
  B=specular-exponent
  A=dual colour-change (black is secondary, white is primary)
  
  Any bitmaps with the suffix multi, are control type 1. Examples of type 1 are usually weapon control maps/props/scenery. Examples of type 2 are all MP character models.
  
  Also, illumination data is stored in the diffuse alpha channel where it applies. (Use common sense to tell what is transparency and what is illumination).
   
  Diffuse maps
  RGB=diffuse colour
  A=illumination source
  
  Otherwise h4 relies heavily on materials and default bitmaps for all other illumination (MP Spartans). They use geometry for shape, and a white texture that can be colour-changed.
  
  Halo 4 isn't limited by pre-set bitmap configurations, the new material system allows for more flexibility in choosing what bitmap channel is what source (colour change/illumination/transparency)
- **2021-07-18** `surasia` [🔍╎texturing-and-shading] (Original message by FiyCsf)
  
  Halo 5 MP Spartans have :
  
  Colour:
  The Base White that is hue'd to the players colour by the control
  
  The Control Armor:
  For MP Spartans Armor and Techsuit:
  R - Colour Zone 1 Mask
  G - Colour Zone 2 Mask
  B - Metallic
  A - Smoothness
  
  The Control Visors: 
  Visors are different and may require more tinkering:
  
  R - Has the Hex Pattern - Likely A Metallic or Specular
  G - Visor is Full White
  B - Normally dark and full of artifacts, not sure if useful
  A - Contains scratches - May be Smoothness still.
  
  Normal:
  This is Direct X from the game. Flip the Green channel to get it to OpenGL for Using in Blender
  
  
  My Recommendation for Colouring:
  Multiply the Colour image's by Your preferred colour using the Colour Zone 1 or 2. This would be the way consistant with halo 5, if the intention is to replicate the game.
- **2021-07-22** `fiycsf` [🔍╎texturing-and-shading] If you're looking at halo 5 controls as a whole, you're not getting a proper understanding of them.
  
  The red and green channels of the image are colour zones, blue channel is metallic, and alpha is smoothness.
  
  So when you look at it as a whole, well, you're not really getting a great representation of what is actually occuring.
- **2021-07-30** `ember.cinder.ash` [💬╎general-help] ```
      TGA = TARGA. There's little reason to use this format now, unless you need compatibility with an app that only accepts TARGA
      TIFF = Tagged Image File Format. This is one of the most complex image formats, and it can hold more kinds and depth of information than almost any other format. The standard is owned and maintained by Adobe.
      PNG = Portable Network Graphics. This is an open standard. It's for losslessly compressing 24-bit+alpha graphics, often for the web. It's somewhat of a successor to GIF, but it does not support animation, at least not without extensions like APNG.
  ```
- **2021-08-02** `plastered_crab` [💬╎general-help] Also you might achieve better texturing if you use the Bungie Halo Shader as well as use Assembly to look at all the other textures that surface uses and the tiling and scaling that needs to be applied to each thing
- **2021-08-09** `kaespoon` [💬╎general-help] Probably a very smooth brain question. Looking for props from Halo Reach and there are the MEGA files, but all I see in there are objects and not any textures whatsoever. I assume I am looking in the wrong place, thank you!
- **2021-08-22** `crashlegacy14` [🔍╎texturing-and-shading] Alright, so this is a general texturing question I have.
  
  If I'm working with a large texture covering multiple types of materials, and I want to use detail textures to, well, detail things. What's more performance efficient, one material using multiple masks to layer on detail textures (like the color control masks are used for primary secondary colors), or partitioning the different materials into different ... material slots? Uhg, I don't know a better way of describing that.  Is there even a noticeable difference in performance?
- **2021-08-23** `malikamadhu` [💬╎general-help] Merci
  
  I dug a bit through the files, and I don't know if I'm missing something, but are the h3 MP tech suit diffuse and normal maps only 1024x1024? I thought the mcc menu textures were higher resolution?
- **2021-08-26** `Deleted User` [🔍╎texturing-and-shading] Anyone managed to find the high res textures for the reach Corvette's hull / interior, I've been looking for hours and the one I did find was for the low Polly version with blurry textures, need them for a project, would massively appreciate any advice
- **2021-08-28** `the_heavynator` [💬╎general-help] so im trying to import 2 separate textures into reach but for some reason importing through assembly will only import the first one, if i try to import another one it just duplicates the first one
- **2021-08-29** `the_heavynator` [🔍╎texturing-and-shading] reach roughness maps are used by setting analytical_roughness to 1,  material_texture_black_roughness to 0.01 (non 0 to prevent nan artifacts), enabling the use_material_texture toggle (usually constraint 0) under float constraints and finally putting a texture with the roughness in the alpha into the material_texture input.
- **2021-08-31** `benji3892` [💬╎general-help] Hello everyone, I’m really new to blender and I’m just wanting to use it for screenshots of my halo reach character. I have the file to open where you can select what pieces of armor you want on your character. So I got him the way I wanted and then deleted all of the other pieces and export him as a bpx file. When I try to import him on any other map that I have made he comes up textureless. Like I said I am ridiculously new, how I do go about either keeping the textures on him or applying the textures after importing? Or is that not possible and I need to just use the file where the creator is every time?
- **2021-09-09** `plastered_crab` [🔍╎texturing-and-shading] I usually use a different Halo 3 shader, but for that one I assume you plug that first orange block's Color into Diffuse, get the detail diffuse texture and plug its Color into Detail Diffuse, and Zbump Color into Diffuse bump
- **2021-09-18** `malikamadhu` [💬╎general-help] Question for those who are more proficient in texturing: how would I go about getting the H3 mcc techsuit textures mapped to the h5 techsuit?
  I've tried shrink wrapping the h3 mesh to h5 and baking, but I keep running into either borked textures or a transparency glitch which corrupts baking anything on that file
- **2021-09-19** `malikamadhu` [💬╎general-help] It's your call and i can appreciate either decision. At any rate though, I would still need to figure out what's going wrong so I can deal with doing the same for the other textures accordingly: reach techsuit to h5 for MkV and Halo Recruit chief for MkVI Mod (gen3 is getting hard surface details)
- **2021-09-23** `file1431` [🔍╎texturing-and-shading] The color of what will be the armor (you can say the light orange color) is composed of 3 variations of an orange, wear, dirt are made with smart materials and after that, we proceeded to make modifications so that the same are not geometrically perfect, then by masks add bases in what would be the channels that are not the color or base, the viewer is simply using the textures of Halo 5 just add one that I did where certain parts limit the relief and another reflection.
- **2021-10-04** `soulburnin` [🔍╎texturing-and-shading] so the Spartan MP armors have the following textures
  
  CC map w/ alpha channel, suffixed as _d
  Control map, with metal in R channel, AO in G channel, and gloss in the alpha channel
  Specular color map, labeled as _s
- **2021-10-05** `arithen` [🔍╎texturing-and-shading] Hello, I'm looking through the Halo Archive Drive, and I'm fining it incredibly difficult to attach the necessary textures to the models I have from that drive.
  
  At the moment, I'm just manually adding in each file, but I can't seem to find any of the visors from Halo Reach.
  
  Can someone point me in the right direction?
- **2021-10-17** `mehrunesdaegan` [📜╎ripping] When I extracted the new Season 8 armors using Reclaimer, the color control maps all seem to be blank. Like void of anything as if they're an empty png. 
  Tested extracting other armors and their bitmaps and it seemed to work fine.
  I do something wrong or is it random on what armors actually have CC?
  Any help would be appreciated I'm new to the program so apologies if I missed something crucial.
- **2021-10-19** `takuat` [💬╎general-help] Heyo, I'm a complete beginner so I've probably done this out of order but I'm trying to hook up the 405th SPI on to H2A Undersuit to potentially put in H3. I've been able to cut it up into appropriate pieces and loosely position it correctly but none of it is attached to the rig yet yet, but I also haven't textured it yet. Is there a particular recommendation on which to do first?
  [attachments] unknown.png
- **2021-10-19** `tpg` [💬╎general-help] https://store.steampowered.com/news/app/1454900/view/3007833468616271134 it do be a spicy update
  [embed] Substance 3D Painter 2021 - Substance 3D Painter 7.3 - Steam News https://store.steampowered.com/news/app/1454900/view/3007833468616271134 New 3D warp projection and updated color picker on the menu!
- **2021-10-21** `orangejambo` [💬╎general-help] I'm seeing a lot of fun errors here.
  Just a quick surface level question, is it overly difficult to import custom assets into H3 w/ the newly released toolset? Lets say... a textured cube?
- **2021-10-24** `extreme_narwhal` [💬╎general-help] Hey there, fairly new to blender and trying my hand at porting maps. I was able to find an fbx file w/ textures and bitmaps of a certain map i'm trying to pull into halo 3. I was able to create a .ass off the files then used the H3EK tool fast to get a proper scenario but i'm getting far too many errors for duplicate triangles and collision edges. I had to scale up the map a bit and im wondering if scaling would cause those issues.
- **2021-10-26** `sierra060822` [🔍╎texturing-and-shading] Needed a semi worn suit for a render soon, so i retextured the h3 Mk 6 and H3 is always a pain for me to retexture for whatever reason but i THINK i got a good design and setup for my purpose, which is what's on the helmet alone right now just to test it
  [attachments] MKL64.png
- **2021-11-03** `itzhouze` [🔍╎texturing-and-shading] hey there. is here a blender pro? i need some help getting my blender materials exported as a texture (to generate a shader for halo3)
  i have many materials in blender and need to get them exported. (every .dat has its own material)
- **2021-11-04** `itzhouze` [🔍╎texturing-and-shading] I need some help in Blender. Getting all the materials from an imported .ldraw file exported as .tif. I want to create a Shader in Guerilla from this materials. Any help would be appreciated 🙂
- **2021-11-08** `manusanram` [💬╎general-help] Hey guys, hope you're having a great evening.
  I wanted to ask you guys for help.
  I'm practicing a hologram shader for Cortana, but I've some issues with the hair.
  The hair already uses alpha transparency but apart from that, I want for it to be transparent like a real hologram (Don't know if I'm explaining myself, maybe look like Halo 4)
  [attachments] unknown.png
- **2021-11-08** `manusanram` [🔍╎texturing-and-shading] Hey guys, hope you're having a great evening.
  I wanted to ask you guys for help.
  I'm practicing a hologram shader for Cortana, but I've some issues with the hair.
  The hair already uses alpha transparency but apart from that, I want for it to be transparent like a real hologram (Don't know if I'm explaining myself, maybe look like Halo 4)
  [attachments] unknown.png
- **2021-11-09** `prompthorizon12` [🔍╎texturing-and-shading] Lads I need help, I've been texturing the reach ccs battle cruiser but something has been bothering me. Do you guys know how to clean up these textures and just leave the lights ?
  [attachments] Unfinished_CCS.png
- **2021-11-11** `tpg` [🔍╎texturing-and-shading] https://www.youtube.com/watch?v=ZOHNRlrd1Ak&list=PLBX-X8mPyxIpM3dszDoS2V5KQ_cdqGsGa
  [embed] Texture Maps Explained -  Essential for All Texture Artists https://www.youtube.com/watch?v=ZOHNRlrd1Ak In this texturing video tutorial, we go through what the various texture maps are, such as diffuse/albedo, ambient occlusion, curvature, displacement, bump, and many more.
  
  - Shader Maps -
  Albedo/
- **2021-11-12** `unordinal` [📜╎ripping] Is there any difference between Xbox Reach and MCC Reach in regards to model meshes and textures? Wondering if I should wait for a Reclaimer update or just download the Xbox map files and use those.
- **2021-11-17** `williaso` [🔍╎texturing-and-shading] Would anyone be able to re-texture the Reach Mark VB with this armor coating? (cough since we're not allowed to have the Infinite version yet cough) 
  
  I don't need it crazy similar or anything, mostly just the same colors, I'm just going to be using the model for posing references so it doesn't need to have super realistic looking textures or anything
  
  I can pay if need be, I guess.
  [attachments] unknown-4.png
- **2021-11-30** `unordinal` [💬╎general-help] But I'm still confused on where the fresnel color is taken from the tertiary color. Lemme show an example: the Elite SpecOps has a green fresnel, and that same color is listed in the color change block under slot 2, despite there being no blue in the elite color control map, so the fresnel color in the shader must be passed that color from the color change block in some way, right?
  
  Maybe I'm just not fully understanding so I'll play around a bit and re-read your visualization a few times
  [attachments] unknown.png, unknown.png
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
- **2021-12-01** `xsoulstacex` [💬╎general-help] Thanks for pointing me the right way R93! I got my model in blender working fine. Do you know how I would now begin ripping the textures and applying those? I was told I'll have to make my own visor shader especially if I want a new age 343 animated visor
- **2021-12-01** `xsoulstacex` [💬╎general-help] Thanks 🙂 so your video covers getting the shaders, applying them to armor, making sure they match in game color options, and getting the back piece? I only ask because if this goes well with my Spartan, I will eventually want to be able to get my friends spartans as well with different everything. In that case I think I'd want to download your models and shaders provided and remove the excess, or is it still easier to rip them each? I've got ripping them figured out down pat now.
  
  I'll ignore armor effects for now but would you also happen to have a lead on getting the animated visors  textures? I can deal w default visor if I got everything else working
- **2021-12-01** `xsoulstacex` [💬╎general-help] Anyone have any leads for me on ripping / recreating the animated visors from halo 3? I was thinking that making a custom gradient texture (or whatever the visor looks like) and writing a shader to scroll through the texture's y coordinates would be doable, that's all that 343 are doing in engine anyways. Thing is I don't know how to do that in blender. Any visor texture would be a start, Reclaimer didn't give me one. I think I can ignore the back piece for now
- **2021-12-02** `bios_0307` [📜╎ripping] Oh btw, in some engines there’s an option to mark a texture as a normal map (Unity has it, for one). Typically that option tells the engine to repack the texture and drop one of the channels, and it later gets the third channel reconstructed. When you have a different texture packed into the normal map blue, it’ll end up dropping that texture completely. Something to keep in mind if anyone ends up making a Unity shader for this stuff.
- **2021-12-02** `plastered_crab` [💬╎general-help] Good morning! And I see that, he is looking great! I would say for your runes, if you can't find where the emissive textures for that is, I would make your own! Make a copy of the color textures, then paint over the rune area with a light blue or whatever color the emissives usually are, then color the rest black and plug that newly created texture into the emissive on the shader
- **2021-12-02** `ubald007` [💾╎autodesk] Thanks, I had to separate the rubber or (Tech suit) from the armor on the sub mesh level. It was pretty simple, I still don't know how blender dose it with just textures but oh well.. Nobody wanted to explain so I had to make my own solution. Ill dm you a pic when I have a chance. I just got some great news from a H3 stand point so Infinite will have to go on the back burner for now
- **2021-12-03** `xsoulstacex` [💬╎general-help] Hey I'm trying to export my spartan as an fbx but every time I do he loses all of his textures! I think it has something to do with replacing the principled BSDF with the bungie shader on most objects. Some like the visor don't use the bungie halo shader though and still come out gray. Followed all the fbx export tutorials I could youtube up and still my textures dont stick.
  
  Help?
- **2021-12-03** `xsoulstacex` [🔍╎texturing-and-shading] Is it possible to take my Blender shaders into Unity? New to texturing and did all my good texture work for a multiplayer spartan using nodes in blender. Used the Bungie Halo Shader in place of Principled BSDF so I could set the appropriate colors onto the armor. Because the diffuse maps came without color, I added it in blender. I just need to know how to get him looking the same in unity as he does in blender
- **2021-12-12** `fiycsf` [💬╎general-help] I honestly wouldn't recommend trying it either.
  
  Reach models have heavy metal damage in the model's normal map and specular data, unlike 4/5/infinite.
  
  This means that if you substance paint them, it'd be like taking a spray can to the top of heavily damaged armor.
  
  You can't just delete that damage and wear, so it'd be very hard to make good maps for it that don't have that effect.
  
  It's why I'd honestly recommend using 5's reach Esq parts for substance painting over using actual reach.
- **2021-12-14** `sierra060822` [🔍╎texturing-and-shading] if anyone knows wtf is wrong please tell me, idk if im just missing a texture or what but NONE of the channels on the Elite Officer Helmet contain a channel for its emission
- **2021-12-16** `surasia` [💬╎general-help] https://gitlab.com/FiyCsf/open-halo-5-spartan-shader-for-unity
  [embed] Fiy Csf / Open Halo 5 Spartan Shader for Unity https://gitlab.com/FiyCsf/open-halo-5-spartan-shader-for-unity Created By PointDevice And FiyCsf For Use With Halo 5 Spartan Models in Unity. This should play nicely with all Halo 5 MP Spartan Textures etc,...
- **2021-12-16** `fiycsf` [💬╎general-help] While I'm sure people will yell "but game accurate!";
  
  You can achieve a decent results with Reach maps by just plugging the specular in the alpha into the specular of Principaled bdsf, and the same for the metallic and smoothness (once you invert it to rough) for Halo 5.
  
  You cannot do that at all with infinite's workflow.
- **2021-12-20** `headphones1999` [🔍╎texturing-and-shading] Can someone help me with some Halo 4 weapon texture questions. What is the alpha of the diffuse texture used for? What do each of the color channels used for in the specular_roughness texture?
- **2021-12-20** `soulburnin` [🔍╎texturing-and-shading] Okay. Halo 4 has two different types of control textures.
  
  Type 1, generally used for weapons and props:
  
  R: Specular
  G: Specular-Exponent
  B: Reflection mask
  
  Type 2, for colorable objects like Spartan armor:
  
  R: Specular
  G: Diffuse
  B: Specular-Exponent
  A: Color change, where white is primary, grey is secondary
- **2021-12-24** `young_gusano` [🔍╎texturing-and-shading] I usually have to Micky mouse the fuck out of the textures and either swap or delete them entirely. I managed to get the Elite head and body working by giving it the textures of the arms that work
- **2021-12-26** `young_gusano` [🔍╎texturing-and-shading] @Deskclaw Hey bro, I logged onto blender and tried doing the instance thing. It’s a near complete success! I’m fixing a lot of the broken textures and so far my only issue was with the Elite face but even then, I had just made my own texture for the face. Thanks a million man 😎
- **2021-12-28** `unordinal` [🔍╎texturing-and-shading] If it's anything like gradient masks in other games, the red channel is probably used to pick a color from a specific gradient map. Unfortunately I believe there's no actual textures for it - similar to there being no albedo textures - so I'm not sure there's a way to completely and accurately recreate it atm. I believe you'd use the texture properly by plugging the gradientmask red channel into the 'fac' of a Color Ramp that's set up with some colors. 
  
  It's probably a similar case with the blue roughness channel in the mask as well (though obviously with that one it'd be limited to the single channel value).
- **2022-01-01** `trainspottingvagabond` [💬╎general-help] @Surasia Sorry to bother you, but do you know where I can find all the textures for the Reach ODST helmet? I downloaded the Reach MCC zip, and they're not in the Marine Bitmap folder.
  [attachments] eacc99250b42953d0bc90f0b8fe82d73.png
- **2022-01-01** `coarforge` [🔍╎texturing-and-shading] https://github.com/Coreforge/blender-halo-infinite/tree/dev Make sure to download the dev branch (master is a bit out of date). It works with bacons shader (and should work with others too), but you need to rename the image texture nodes for it to work, as they are just called "Image Texture" by default.
  [embed] GitHub - Coreforge/blender-halo-infinite at dev https://github.com/Coreforge/blender-halo-infinite/tree/dev Contribute to Coreforge/blender-halo-infinite development by creating an account on GitHub.
- **2022-01-01** `coarforge` [🔍╎texturing-and-shading] I can quickly, but I'm off to bed after that. You go onto the github page and select the dev branch (the link I sent already has that branch selected). You then download the code as a .zip file, which you can then install as an addon in the user preferences in blender. To be able to import textures, you need to install the module texture2ddecoder into blenders python environment by running a few commands in the python console in blender (the commands are in the readme.md). You can then go to import -> Halo infinite rendermodel and import a rendermodel. If you want to import more then just the 3D model (so textures), you have to set the root path property to the root path of where you extracted the modules to (the filechooser doesn't work, so you have to copy in the path). Same for the shader .blend if you want it to automatically apply that shader.
- **2022-01-08** `opr_23b` [💬╎general-help] I wanted to ask a question regarding Halo 2 assets, texture stuff, and whatnot.
  
  So, I've extracted the model of the Elite using reclaimer, and I've imported it into blender with rigging, textures, and whatnot. (I personally preferred to rip the model myself rather than download it from a previously existing source.
  And to cut to the question itself without bloating with too much information, is it at all possible to use some sort of node setup that uses the RGB textures from the game, in order to easily retexture the Elite?
  Basically, you set a specific color in some sort of option, and it retextures the elite using the RGB mask as a "base"
  [attachments] arms_color_change.png
- **2022-01-09** `.bombwolf` [💬╎general-help] And does anyone know what the ODST's suit would be named? I know it isnt the cloth like the marines because all the references have a spartan like rubber, just not the same texture.
- **2022-01-11** `tbgamer2099` [🔍╎texturing-and-shading] could someone pls send me the texture files of the Yoroi armor but in this color pls?  I already have a Yoroi spartan armor on my unity but I need its original Textures instead of the textures you get with the models.
  [attachments] Capture3.PNG, Capture4.PNG
- **2022-01-12** `activelydrowsed` [💬╎general-help] what do I need to input in the materials box for textures to appear. Do I need the file location their, like ``C:\Users\Desktop\elite\bitmap`` ? I have them extracted but can't get blender to see them.
  [attachments] Capasdasdture.PNG
- **2022-01-13** `sabsnake` [📜╎ripping] Unsure if I can ask this here or not, please no bannerino.
  Has anyone had success in Ripping Halo Reach Maps/MP Maps with Textures?
  ( If so, what tool did you use? ) I used Reclaimer and Adjutant and got 0 Textures, just Grey.
- **2022-01-17** `coarforge` [💬╎general-help] I'm not entirely sure what you mean, but Infinite doesn't have albedo textures like you normally might have, instead, there are a bunch of control maps (usually asg, control 0 and control 1, though there are a few other ones too) which control different aspects of the material and get combined into the material output, which is done by a shader. Normal maps are still normal BC5 normal maps though.
- **2022-01-21** `theeldestboy` [🔍╎texturing-and-shading] https://www.artstation.com/artwork/xY31YY
  [embed] Halo Infinite - Wetness, Evan Liaw https://www.artstation.com/artwork/xY31YY Water and wetness was a huge team effort especially with a large Campaign and Multiplayer maps.  Huge thanks the Squirtle Squard.  Steve Carroll, Mahreen Fatima, and Mitchel Clayton from Campaign.  Stuart Cunningham for all
- **2022-01-27** `ubald007` [🔍╎texturing-and-shading] This is for everyone, Work-flow tip. As we mostly deal with extractions and we all know they are not always perfect!, the textures some times get messed up. these images are to help you see the errors that sometimes happen with extraction's. A. is to show you that sometimes the alignment can be off sometimes very little. Also being the Green channel in this case highlighting the edge on the ASG_Control map is a great guide, makes it easy to spot and fairly easy to correct as shown in image B. So in your work-flow before building your shader use the Control map to help you fix this misalignment. I hope this helps all of you!! CHEERs..
  [attachments] A.jpg, B.jpg
- **2022-02-05** `nobleofdeath16` [🦴╎animation] That ives me some ground to work off of since I was already familiar with working with it in Reach this prior year. 
  
  @Vuthakral ,was Fubih's animations they made for it open source or are the unavailable? Being able to just swap it to the Halo 3 first person rig would save a ton of work on the mod; as that would just leave matching up textures and dialing in damage values/mechanics.
- **2022-02-12** `phoondos` [📜╎ripping] is there a shader for blender that works with halo 1 textures? i know about chief's halo shaders but i havent used them yet, so im clueless as to which games they deal with
- **2022-02-25** `coarforge` [💬╎general-help] A texture is basically an image (in theory it can hold any kind of data in usually 1-3 Dimensions), while a shader is a small program that tells how to render something or does some other preprocessing for rendering. A shader can just take in positional and texture data and apply the texture (and do some lighting work, depending on the context), but it could also generate something completely different.
- **2022-02-27** `plastered_crab` [📜╎ripping] You've already ported it as in you loaded it up into Blender? Or did you go so far as to spend like 2 months in Blender / Assembly and hand tweaking all the textures and tillable bitmaps for each and every surface on the map? 😅 the palm trees alone for that map have like 12 different textures they use as well as a huge terrain shader solely for them. Not to mention the skybox and whatnot.
- **2022-03-18** `jwgjjman` [💬╎general-help] How to get Halo 2 textures to load properly in blender? I downloaded all the Arbiter and elite . amf and . tif files from the Google drive. Imported to blender with the amf importer tool, but the textures aren't there. I tried it again with Halo 3 arbiter and that worked fine. Is there a different process for Halo?
- **2022-03-30** `surasia` [💬╎general-help] https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes1.1
  [embed] Release Material Types 1.1 · AverageTrapEnthusiast/Halo-Infinite-Sh... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes1.1 Fixed an issue with how the gradient masks are color graded.
- **2022-04-03** `opr_23b` [🔍╎texturing-and-shading] Hey, just wanted to come by and ask for some help with the Jackal model I'm working with.
  
  So, I want to get basic materials (basically giving the model it's textures without a Reach-style shader, since I intend on doing that later) on the model, but I'm unsure as to exactly what texture these completely black parts might have.
  The material for them is named "additive_gradient", and looking at some reference pics, it seems to be a sort of "glow" effect for the glowing parts of the Jackal, though I can't seem to find these textures for this glow effect.
  
  Any idea as to where they might be?
  [attachments] unknown.png
- **2022-04-04** `saltyace` [📜╎ripping] So, I'm working on Tempest from Reach, right. And I've got the all the main geometry and stuff. But there's a bunch of foliage. And I have no idea where it is. I've only ever used reclaimer once to pull the main geometry with its basic textures. I have no idea what I'm doing.
- **2022-04-09** `the_heavynator` [🔍╎texturing-and-shading] h2asp is basically specular workflow shader but the artists were still used to a non pbr workflow so its a little messy but there are 3 main textures, Diffuse, specular and normal which is fairly standard. the specular has the smoothness (inverse of roughness) value in the alpha channel. again just keep in mind the game was made during a transitionary phase to pbr workflows so some assets will look better than others.
- **2022-04-20** `squishypon3` [💬╎general-help] Took a bit of a break working on an H3 map and coming back trying to use reclaimer to get some .tiff files for blender. Anyone know why Reclaimer would have issues opening up h3 .map files on steam h3 mcc?
  [attachments] unknown.png
- **2022-04-27** `snowshankington` [💬╎general-help] Hi, While looking in reclaimer at the reach club Errea area these yellow textures are everywhere, am I missing a necessary install or is it just the way it is? in the game these where all different textures I believe
  [attachments] reachtextures.JPG
- **2022-04-30** `awsus` [💬╎general-help] I cant seem to get textures to appear on the Reach MCC props, I import them using the .amf addon and they appear black/glossy. I watched Crab's tutorial that's linked above, and I tried to adjust the texture formats from tif to png and dds and no luck. Any idea what might be wrong?
- **2022-05-02** `awsus` [💬╎general-help] I couldnt find the textures for the Reach assets in #🔗╎all-halo-asset-resources but I did just get the prop pack from #deleted-channel and it worked great! I appreciate you getting back to me
- **2022-05-05** `chunch` [🔍╎texturing-and-shading] https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/ModularShader2.3
  [embed] Release Halo Infinite Modular Shader (2.3) · AverageTrapEnthusiast/... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/ModularShader2.3 A fully revamped version of the modular shader!
  Now includes node groups that can be used to convert older halo games models an
- **2022-05-05** `plastered_crab` [💬╎general-help] That would be sweet! If you have a hard time with the textures being setup (as they are quite different than other games) check out the shader by @Chunch or wait for the huge Spartan pack coming out soon that @Surasia it's working on
- **2022-05-05** `chunch` [🔍╎texturing-and-shading] https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes1.1
  [embed] Release Material Types 1.1 · AverageTrapEnthusiast/Halo-Infinite-Sh... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes1.1 Fixed an issue with how the gradient masks are color graded.
- **2022-05-08** `fiycsf` [💾╎substance-suite] So yes, and no.
  
  Let's take H5s. Halo 5 has half it's mirrored UVs off the 1:1 space.
  
  That half cannot be painted on directly and inherits from the 1:1 space. 
  
  Some games (or bad rips) place everything outside the 1:1 space, and in order to substance paint, you actually ***have to*** shift the things you are working with back into the 1:1 space when they will have started outside it for some reason.
  
  The real thing to ensure that you are moving it without actually changing what it would relatively sit on top of. 
  
  Going back to halo 5, if you wanted to seperate out the mirrored side and paint it seperately, you'd likely want to move UVs around.
- **2022-05-11** `silent1meow` [💾╎substance-suite] Finally got around to finishing the whole thing
  https://twitter.com/Silent1Meow/status/1524470624457961472
  [embed] https://twitter.com/Silent1Meow/status/1524470624457961472 Finally finished retexturing the Infinite MKV (B) core and also used the Halo 5 Operator helmet. Overall I'm pretty happy with how it turned out.
  #Halo #SubstancePainter #blender 
  
  -Rendered in Blender 3.0 Cycles & Edited in Photoshop-
- **2022-05-12** `soulburnin` [📜╎ripping] I don’t have any issue grabbing weapons from H2AC, but the normals are so beyond botched I don’t bother with them. Fortunately a lot of them are just recycled Reach and H4 assets that you can just slap the textures on
- **2022-05-13** `classytm` [🔍╎texturing-and-shading] I don't wanna mess this part up, correct me if I'm wrong but I should append the following: Halo Infinite Shader 2.3 - Infinite Color - Infinite Masking Sorter 6 way - Infinite matts. Right?
  [attachments] unknown.png
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
- **2022-05-19** `plastered_crab` [💬╎general-help] Map porting can be a PAIN and can take a while to get right. Just importing them from Reclaimer isn't enough as the textures require additional work and need to be manually scaled/tiled as well as setup with a proper Halo shader. After that you need to add lights to your scene. If lights are on the inside the pics you sent would probably still be black as the outside isn't lit probably due to a number of reasons
- **2022-05-28** `chunch` [🔍╎texturing-and-shading] https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes1.1
  [embed] Release Material Types 1.1 · AverageTrapEnthusiast/Halo-Infinite-Sh... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes1.1 Fixed an issue with how the gradient masks are color graded.
- **2022-06-01** `soulburnin` [🔍╎texturing-and-shading] I'm not really familiar with the mod tools, but in Blender I would set the texture to linear/non-color, then adjust the channels individually with a math node set to multiply to reduce the visibility of those lines. Not sure if that's something you can do with the H3 tools
- **2022-06-02** `r93_sniper` [📜╎ripping] For a Comprehensive Halo 5 pack, i'd like it set up as such:
  1. Have all Armors set up with Names, Materials, Rigging, and Skin Weights
  2. Have it set up in 3DS Max with Layers
  3. Have it set up in Blender with Collections
  4. Have a Full FBX with everything exported in a monolithic stack
  5. Have Each individual Armor Set exported as a Seperate FBX
  6. Have all the textures for each individual armor set named correctly and located in their own relevant folder
  7. [Optional] Have a comprehensive blender shader set up
- **2022-06-08** `basedregalia` [🔍╎texturing-and-shading] Hey guys, so I just started modeling stuff in blender about a week ago and I've been using Aesprite to texture with. I still have absolutely no idea what I was doing, and I was wondering how to add a shader that can change the color of the armor here to whatever hex/RGB value I give it. Is there a good resource to learn this kind of stuff when I'm just getting started?
  [attachments] spartan.png
- **2022-06-09** `koax117` [🔍╎texturing-and-shading] Going to echo what Vorch set. at least in the screenshot you provided, you're only affecting the color control alpha. 
  You've got a Diffuse Alpha, Detail Map 1 Alpha, Detail Map 2 Alpha, and the regular Alpha itself. 
  Personally for me, when working with another person's shader/project file, it's a lot of trial and error to get the result I want, because I can't read the person's mind on how they set it up. 
  My immediate thought would be to combine/mix the alpha's of both emblems to go directly into the alpha node itself to mask out the black that you are wanting to remove
- **2022-06-13** `rufflestukov` [💾╎autodesk] Hello, I've been trying to use the Halo 2 Anniversary Chief model in Maya for a render and I really don't know how to use the textures, I mean, I only know where to put the normals and the color texture, but in the render it looks bad, any ideas?
- **2022-06-13** `evactouch` [💬╎general-help] (Surasia S2 Infinite Pack, Blender 3.2)
  How can I add a camo onto armor while retaining the color controls?
  My first thought was to hook up a image texture and apply them to the Zone Top/Bot/Mid, and while it does apply the camo texture, it removes color control. And I am attempting to do this with a mostly greyscale and mostly transparent digital camo texture so I can easily change its color.
- **2022-06-15** `gameoverload` [💬╎general-help] Through the Blender Spartan 2.0 Pack, via it's Bake outputs on the right of the Shader node where the BSDF connects to the material output. Although you will want to have an image texture node somewhere on each armor you wish to bake out textures for that has a new 1k, 2k, or 4k image ready for the baking of textures.
- **2022-06-17** `frostbitecinematics` [📜╎ripping] It's kind of like how you set up the clone markings. Basically each channel of the _W map contains a black and white mask, one for each weather effect. E.g red might be sand, green could be snow, blue could be wet mud when you walk into rivers, etc. I don't use Blender much so I'm going off of what I know from doing it in Unreal Engine, but you essentially use the mask to LERP between the base texture and the weather colour and normal map texture. In Blender, I believe the MixRGB node is the same as UE's LERP. So the mask goes into the LERP Alpha (MixRGB's Value?), the A input of the MixRGB is your base colour and the B of the Mix RGB is a mud texture. Then set the same thing up to blend between the model's base normal map and the weather effect normal map, e.g the mud normals.
- **2022-06-18** `bag_of_nickels` [💬╎general-help] ive been following the tutorials but im having the problem of my spartan showing up as purple in places now matter what color i want him to be, other colors and textures are not coming in right. is there a step im missing or something. it could be my laptop too but i thought id ask before assuming that.
- **2022-06-30** `Deleted User` [💬╎general-help] @Tusk basically each node changes some thing then feeds that info to the next
  
  the first node just tells the programe "ok we have a black and white texture"
  the next node says "ok i have revieved the texture, im changing the white to red"
  the final node goes "i have recieved the red texture i will now make it metal and shiny" and then its fed to the final shader output
  
  i hope this helps but it also might do fuck all
  [attachments] unknown.png
- **2022-07-01** `keeeeeeeeen` [💬╎general-help] Sorry it's late here and I haven't been feeling too great as of late which is why I haven't been so active but best way to quickly set it up would be from inside the Spartan pack, click File-->Append---Select the Toon Shader .blend file --> Nodegroup --> toon shader 1.1. from there then head to the shading tab, select the armor piece you want to toon shade, you should see a node titled along the line of HIMS with 4 texture nodes plugged in. Use shift+a and search and add the toon shader node, plug in the correctly named texture nodes from HIMS into the toon shader input, plug the green output into the material output node and you're done :)
- **2022-07-03** `r93_sniper` [🔍╎texturing-and-shading] if you're using the infinite shader (which judging from the masks you are), the goal was to make one that emulates the look of what halo infinite does as closely as possible. They use, at max, 6 layers from the two CC masks and then 2 more from the ASG map. Unless there comes an instance where they actually use an extra channel (and subsequently actually add it to the game/shading system), there's no purpose in trying to make the shader accept that mask
  
  If you're just trying to add a decal stripe there's a few methods to do so, 
  1. Pick the zone that it most closely matches the color that you want the stripe to be and make it part of that mask while removing it from whatever overlaps
  2. Create a decal plane and attach it to the model and UV it to have only the decal stripe (and alpha or additive the rest)
  3. Modify the shader for your own local use to accept an extra mask
- **2022-07-09** `incuriouslog` [💬╎general-help] Sorry if I'm completely off on this, but after a quick look it seems to be using a few shaders to generate that starfield rather than a texture. If you go to the World Properties tab on the bottom right and remove what's there you can then select "Background" then click the yellow dot to the left of colour and choose "Image Texture" which will then let you load an image file.
- **2022-07-10** `nemesiswmd` [🔍╎texturing-and-shading] hi im new to the server and just downloaded the infinite spartans pack i exported masterchief as obj to put it in element 3d does anyone know if there is a way to export with textures when i try to retexture it in element everything is the wrong color or i might be putting it in the wrong place but im brand new to 3d so any advice will help thankyou
- **2022-07-11** `gameoverload` [🔍╎texturing-and-shading] Basically you just need to use Goblin's shader, and add an unconnected image texture node in the first material your baking out from, make a new image in that node that is 1k , 2k, or 4k in size to fit whatever you need, specifically that scale squared, like 256x256.
  Then you need to copy that node to every material of every object you wish to bake out textures for.
  Then you go back to that first material you were working with, connect one of the bake outputs to the material view node to the right of the shader.
  Then go into your render tab, set the rendering in the bake tab on the side to emit rather than combined, set the 16 tiling below that to 32, then hit bake.
- **2022-07-11** `gameoverload` [🔍╎texturing-and-shading] A simple list of ones you will find most often.
  Normal Map - Adds extra detail to a model where none may have existed.
  [ Often used with a low poly version of a model to reduce resource draw. ]
  [ Requires a Normal Map node in Blender and if the NM is a a Direct X NM, the Green channel needs inverted. ]
  Roughness Map - Adds pre-baked lighting information to a model.
  Metallic Map - Tells the engine where to add metallic properties to a model and how much of that metallic property.
  Albedo - The color information of a model.
  Diffuse - Color information with some form of lighting information baked in to the alpha.
  AO ( Ambient Occlusion ) - Adds pre-baked shadow information to a model.
  ID Map - Some form of information that tells the model where something may need to be, that could be procedural color, procedural extra normal detail, etc.
  [ Can be called a Tint Mask or Color Control Mask if that is what it pertains to do. ]
- **2022-07-15** `incuriouslog` [🔍╎texturing-and-shading] I just did something I'm very proud of, yet am fairly certain I did in a completely dumbass way, and was wondering if anyone would care to translate my stupidity into correctness.
  I wanted to add a PNG with transparency to the ODST chest attachment for the Mk. Vb core, but I also wanted it to match the distressed look of the metal.
  So I opened the symbol in Photoshop, then added the asg bitmap for the chestpiece to another layer. I lined it up with where the symbol was going to be, created a layer mask on the symbol layer, then applied one of the channels (red, I think) inverted as the layer mask. This matched the distress perfectly with the chestpiece. I saved it as a PNG with transparency.
  I then imported the PNG as a plane into Blender. I added a subdivision modifier and then a shrinkwrap modifier, set to project onto the surface of the chestpiece with a .0005 offset. I sized and positioned the plane, then for good measure made sure it cast no shadow, and paired it to the chestbone of the rig so it moves when I pose it.
  It works exactly how I need it to.
  How should I have done this properly?
  [attachments] blender_22.07.15_21.44.33.920.jpg
- **2022-07-16** `sunksi` [🔍╎texturing-and-shading] Just append the piece of halo 4 armor from the blend into a blend with the Prefect helmet
  Copy the entire shader over to the prefect helmet
  And replace the appropriate textures
- **2022-07-26** `recl8015` [📜╎ripping] Using the new Halo 2 Anniversary mod tool to extract the skyboxes for various sentinelhq BSPs. it gets me the render model but does not pull the textures attached to it. I can extract the render data of og H2 via Tool just fine. Am porting Containment to H3 and trying to get different skybox variants for it, including H2A.
- **2022-07-29** `elitecaden` [🦴╎animation] Here is a picture of what the rig looks like and here is the file I made for myself that has the ik rig https://drive.google.com/file/d/1QF7cQlTFasZaNgWT8WQTK-bvgoSxjlR1/view?usp=sharing
  [embed] Elite IK chief elites ALPHA.blend https://drive.google.com/file/d/1QF7cQlTFasZaNgWT8WQTK-bvgoSxjlR1/view?usp=sharing
  [attachments] image102.PNG
- **2022-07-30** `gameoverload` [🔍╎texturing-and-shading] If you read the outputs of the shader, not quite, a shader built in Blender will not work in another graphics engine, without replicating it within that engine's graphical structure.
  In the case of Unity' that is Unity's visual shader code system, or Amplify for a plugin of similar design.
  However Unity comes with a standard shader, which if you read the texture inputs of, will tell you what texture maps you need. Why does this matter?
  If you read the outputs of the shader in Blender, it will tell you what it can 'emit' on the mesh, and guess what Blender's graphics engine, Cycles, can do.
  It can bake out the emitting visuals of a selected mesh.
- **2022-07-30** `gameoverload` [🔍╎texturing-and-shading] Ah, it appears the reach shader doesn't include extra outputs, which are on the right side.
  Inputs on the left, outputs on the right.
  You would need to use Infinite armor then, as the shader those use include extra outputs, particular for getting textures used in other engines.
- **2022-08-07** `gregory0701` [🔍╎texturing-and-shading] I can open the .shader in guerilla and get a page showing the albedo and all that, and what textures are set to what variable, but I don't see anything that looks like code
- **2022-08-08** `hamporker` [💬╎general-help] Hey, I'm porting Temple from Perfect Dark and Golden Eye into Halo 3. Does anyone have any knowledge of lighting in H3EK? I got bitmaps and shaders working, but I want shadows in the indoor areas like  in the original N64 map.
  [attachments] Capture1.JPG, Capture2.JPG
- **2022-08-08** `heavensc` [🦴╎rigging] https://cdn.discordapp.com/attachments/869803909953245246/1006241357343502427/unknown.png Trying to move an ODST model I downloaded here into FBX to upload to mixamo, but can't seem to do it. Textures won't bake.
  [embed] https://cdn.discordapp.com/attachments/869803909953245246/1006241357343502427/unknown.png
- **2022-08-08** `hydranaught4444` [💬╎general-help] the texture i'm using uses a hologram shader in reach meaning weather it has transparency or not it'll be transparent anyway so the outline doesn't matter but i need it to not have the outline
- **2022-08-13** `caia.` [🔍╎texturing-and-shading] How exactly does the grunt illum shader work?
  
  -There doesn't seem to be a dedicated texture for it in the bitmaps folder
  -Guerilla says that it's using backpack.tif
  -The UVs (see pic) are in places that don't produce anything that make sense
  -Reclaimer shows circles and lines that appear to be textured and not generated.
  
  What gives?
  [attachments] 2022-08-13_08_12_27-Blender__C__Users_sunst_Documents_Development_Halo_Halo_2_objects_characters_gr.png
- **2022-08-13** `vat_` [💬╎general-help] If you using unity, you either need to bake out new textures in blender, as the ones in the blend file are procedural, or use this shader set up by point/r93sniper https://github.com/R93Sniper/HaloInfiniteVRCShader. Even tho its for vrc, im pretty sure ill work in general for unity stuff.
- **2022-08-13** `vat_` [💬╎general-help] Halo infinite dose not use any form of diffuse or albedo. Its, again, all procedural, and the blend file you have uses a shader that convert those textures into a workable format that can be customized with dtail maps, but has to be baked out
- **2022-08-13** `soundflare` [💬╎general-help] Trying to use the skin shader that was in #🔗╎shaders-resources for Halo Infinite, am trying to work on a Banished Brute Chieftain and started on the face but.... Have no idea what to do correctly as am not familiar with the skin shader designations on it, nor the texture stuff primarily from Infinite. Any assistance would be appreciated
- **2022-08-15** `fiycsf` [💾╎substance-suite] You'd need to correct the softness first with blender knowledge. 
  
  That's not a substance thing.
  
  Game models look sharp when the have a low poly with normal map. 
  Low poly + normal map = close to high poly asset
  
  You are looking at the low poly without any of its maps or materials. They cannot be obtained for anything not in the PC multiplayer of halo 5. You will not find them for that character, unlike spartans.
- **2022-08-18** `creepatore` [💬╎general-help] Hey so, slowly i've been working on a (mostly) faithful recreation of H3 Chief using infinite chief. This is like, textures and what not. One of the final steps I'm trying to get right is remaking the visor to look like 3's, but can't seem to get the proportions right. I'd appreciate any help or advice on how to do so
  
  Pictured: a piece of the project in question
  [attachments] unknown.png
- **2022-08-25** `fiycsf` [💾╎substance-suite] I don't even blame the guy. I've gotten into the habit of using painter on a literal square to deal with transparency for decal stuff because the old version of Photoshop I have handles alpha like trash by comparison.
- **2022-08-25** `gameoverload` [🔍╎texturing-and-shading] Well Diffuse is normally a combination of Albedo or color, and either a baked in map on top of that, or something in the alpha of it that pertains to light or shadow in some form, or something entirely different like color masking.
  A normal texture requires the use of a normal map node, with the normal texture connected to it and then the normal map connected to a normal input.
  The AO would require a MixRGB which multiplies the Diffuse by the AO.
  Metallic into the metallic input.
  Smoothness into an inverse node then into roughness, since smoothness is the inverse of roughness.
  Then the tint index sounds like a tinting mask of some kind.
  Also you should not be using a Halo 5 armor shader for this, it is not built for how Gears Tactics works, it is built for how Halo 5 works.
  You would need to build your own shader for it.
- **2022-08-29** `nascarlaser1` [💬╎general-help] ok so there are two ways of doing it. Both are easy, ones just a bit quicker if you plan to do this often/have specific needs. 
  
  The default/no addon way: go to a website like polyhaven (my preffered site):https://polyhaven.com/hdris or 3d assets.https://3dassets.one/#type=hdri&order=latest
  
  From there, find an HDRI you like and download the HDR or EXR (depending on website) file.  1K is good if you mainly need lighting, 2-4k is good if you want a background too. 8-16 is just overkill imo and will make your file far larger than it has to be, but if you feel like you need it you can download it too, size doesn't change the steps.
  
  After downloading it, swap your node texture window from "object" to "World". Grab an "Environment texture" node, and then just op your HDR/EXR file into that, and connect it to the color input of the already-made "background" node. 🙂
  [embed] HDRIs • Poly Haven https://polyhaven.com/hdris The Public 3D Asset Library
- **2022-08-30** `tristindiscoarts` [💬╎general-help] Hey is there a reason why https://drive.google.com/file/d/17t8UOy1r4w1yxt_JcGBBspQVP2qATFTr/view?usp=sharing Halo Reach Brutes. Fully rigged and textured and with their power armor. (Not including weapons and humans) is slightly purple?
  [embed] Brutes.zip https://drive.google.com/file/d/17t8UOy1r4w1yxt_JcGBBspQVP2qATFTr/view?usp=sharing
- **2022-09-04** `piratepenguin.` [🔍╎texturing-and-shading] Exactly what 93 recommended, I personally use paint.net, but the process is really easy in paint (don't know how it is in the others, but I imagine it's the same) there's a tool called magic wand. With it you'll be able to highlight the amount of the image you'd like to change than you can use the recolor tool evenly change the color of the image. From there if you'd like to edit further you can layer the modified image on top of the original and blend the multiple textures together to get the best of the two versions into one new image
- **2022-09-07** `indigox86` [💬╎general-help] Hey hi, I'm sorry I'm probably breaking some rule but I have pretty much nowhere else to turn to. I can't seem to find an image or download of the default/placeholder texture of halo 3 for example, maybe I'm just dumb but is it up anywhere? Does anyone here have it/can help me get my hands on it?
  An example of what I'm talking about
  https://www.halopedia.org/Da_Vinci_Code_objects#/media/File:H3_Sandtrap_DaVinci_2.jpg
  [embed] Da Vinci Code objects https://www.halopedia.org/Da_Vinci_Code_objects The Da Vinci Code objects are objects that appear in several levels in Halo 2, Halo 3, and Halo 3: ODST. These objects are textured with a default UV-unwrapped texture (used to render textures onto 3D models). The texture is calle
- **2022-09-11** `gameoverload` [💬╎general-help] You would need to create a whole new Infinite Shader material for it to work properly, which requires a gradientmask and a detail normal.
  The gradient mask requiring a color map and a roughness map to work properly when put into a texture.
  You also don't need to make a whole new material just to have carbon fiber if you download and append the material types Goblin made, since it includes a similar material to carbon fiber already, then you just go to the material node to the left of the zone you wish to change, and simply switch the material that node is using.
- **2022-09-15** `blakjewellio` [💾╎autodesk] https://sketchfab.com/3d-models/halo-elite-minor-ca4504cbd0f548d8aaefa31f41db97a2
  [embed] Halo Elite Minor - 3D model by Jawley3D https://sketchfab.com/3d-models/halo-elite-minor-ca4504cbd0f548d8aaefa31f41db97a2 Halo Elite Minor made using the following:
  
  
  Maya - Rigging and Rendering
  Substance Painter - Texturing and Baking
  ZBrush - Sculpting
  
  This model is now available on my Gumroad!
  
- **2022-09-22** `spartanj104` [📜╎ripping] Hello, 
  I saw that there was no "Infinite Brute Pack" so I stick to the task...even though I've never done that in my life and I am struggling xD.
  I've already ripped the game, output the raw models, sorted through Blender and now I come to the part I hate the most: the textures/shader/UV.
  I try to watch tutorials on this but it doesn't help me understand how you managed the textures like on "Infinite Spartans Pack" I need someone to show me the light 😅 🥲 
  Thank you in advance ✋  🙂
- **2022-09-22** `coarforge` [📜╎ripping] Infinite uses control maps. There is a description of how they work in #📑╎control-map-guide if you want to create your own shader, or you could probably use goblins shader as a starting point (or bacons shader). I haven't tried either of those shaders with the brute models, but they should probably work with some tweaking (the rat for example did work with bacons shader without any modifications, but it's probably not super accurate)
- **2022-10-06** `elitecaden` [🦴╎animation] Hunter looks like it’s covered in foam. Animation of the pelican looks weird because it looks like it’s moving just on a single axis and how it goes in reverse a bit then speeds up forward all of the sudden. Animation of the 2 pilot heads looked like they were in sync with each other which looks a bit off. The standing elite holding the mangler needs to be more fluid. Especially at the chest where it looks stiff. Try maybe having the chest flow with the arm movements and have the chest do breathing motions (like tilting up and back down). The lighting in space needs work as it looks boring. But the lighting on the ground scenes look awesome
- **2022-10-06** `jaeger1648` [🔍╎texturing-and-shading] Hello, I was told my message might fit better over here
  
  I'm very new to blender and I'm using the reach spartan pack. I couldn't figure out how to export an fbx with any materials/textures at all. Actually the decals exported with it but that's it. Now I kinda know it's because I need to texture bake?
  I found a video on how to texture bake, but ever time I try to smart UV project it messes up the textures, shifts them around on the model a bit. It seems it's because it's not unwrapping right, but I have no clue how to fix it.
  
  I'm about to go to bed, but any help would be greatly appreciated!
- **2022-10-06** `gameoverload` [💬╎general-help] If you really wanted to create your own elements that utilize the Halo Infinite shader, you would want to try that with a Halo Infinite character asset if your just looking to make your own materials for example, or you would want to try with another game's  character asset if you wanted to learn how to make your own textures and materials for it.
- **2022-10-10** `cameraguy` [🔍╎texturing-and-shading] I have the H3 ODST and the armor is not colored in the stock OG textures (like the Rookie armor) what is the best way to color it?
  I'm trying to get the other characters armor accurate
- **2022-10-12** `cameraguy` [🔍╎texturing-and-shading] I have the H3 ODST and the armor is not colored in the stock OG textures (like the Rookie armor) what is the best way to color it?
  I'm trying to get the other characters armor accurate
  [attachments] Screenshot_20221011-214506__01.jpg
- **2022-10-16** `pepperman` [💬╎general-help] hey guys, im looking to bake down some textures from blender for use in-engine (halo)
  I've done it before for a few things, but the current model is giving me some issues due to the UV setup
  But i just noticed these outputs from the infinite shader node, are they what i think they're for, and if so how do I go about actually using them? im not very familiar with blender's shader nodes past the very basics
  cheers
  [attachments] unknown.png
- **2022-10-18** `chalk33_8677` [💬╎general-help] ~~if this is the wrong spot, punt me pls~~
  
  So I'm wanting to get various halo character models into tabletop simulator, which wants a .obj and a single image file for the textures. I was playing around with the Infinite Spartan blender stuff to test, couldn't figure out the texture part (even remotely, was 100% lost). I'm looking at CE stuff (plastered_crab's gdrive resources) now—I'm wanting to start with ce stuff anyway—and I just have *no clue* where to even *start* on these. Multiple tif files (I at least recognize the format), amf, fbx. Where do I start with this stuff?
  
  I've tried opening up CE Keyes in Blender and everything just looks absolutely wonky. Reach Keyes (obj) looks better, but colors are all dark
- **2022-10-20** `chalk33_8677` [💬╎general-help] So I can post the tutorial I used, but the process is more or less
  
  1. Smart UV unwrap
  2. Make new image file and bake to it in diffuse mode
  
  I've tried exporting to obj after this, which is what they screenshot is, otherwise using the original obj doesn't work either. I've also tried doing the h3 Spartan asset Pack, but it wouldn't actually let me bake, so
- **2022-10-20** `alanticsea` [🔍╎texturing-and-shading] I don't understand where I'm going wrong with baking normal maps using substance painter. Started by making a lowpoly and highpoly model in blender to test. made the lowpoly shade smooth, uv unwrapped the lowpoly, triangulated both the lowpoly and highpoly, checked to make sure the bake looked good in substance, exported normal map to blender, made sure normal map in blender was set to "non-color" with the correct nodes, and now it looks like this garbage.
  
  why
  [attachments] Adobe_Substance_3D_Painter_04J6hZq13l.png, blender_YUWYO7MZWF.png
- **2022-10-30** `neapaw` [💬╎general-help] i just finally got the elite files open and im trying to change the color of the eyes themselves. I am new to blender and i  know its not the bitmap that needs the modifications. assistance please?
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
- **2022-11-07** `hausdev` [🔍╎texturing-and-shading] I'm a brainlet when it comes to most 3D stuff, and I've been focusing on CEA so my H2A knowledge is a bit rusty, but in the texture definitions json, i beleive there's a field called "preset" or something along those lines. It might be called something else idk
  
  But I think once you have that name, you can find that file under Shader Definitions in index, and that'll have the HLSL code. Iirc it's polymorphic, so theres a base shader that other shaders build on top of, so you'll probably need to navigate a few different files to find what you're looking for. There's also a lot of mathematical functions in the "common" shader file, if that helps
- **2022-11-07** `gameoverload` [🔍╎texturing-and-shading] If that is not what your looking for, at that point you will need to then rip all the geo, and make sure all that is set up as you see in game, then begin to grab all of the need textures, texture data like tiling, colors, etc, then you will need to find all of the shader code data, ala in the Reach Tools, for the environment shading you need for that level, then you would build a shader from that, then after that, you input all of the textures and values you have into that shader.
- **2022-11-13** `waketheshark` [📜╎ripping] I'm following the tutorial to export from reclaimer and to import into blender. I've exported the .amf for m50_000, and within the same folder, exported all bitmaps of m50_000. But when I import into blender with the AMF importer (making sure that it's looking for tiff files), nothing is textured. Is there something extra I need to do for Structures? I have all of the shared reach maps: campaign, mainmenu, and shared.
  [attachments] image.png
- **2022-11-13** `ricky764` [💬╎general-help] How do you extract textures from Halo Reach? Thanks to R93_Sniper I was able to get the model of the Jackal and Skirmisher from Halo Reach, but they don't have textures, so I'd be interested in getting them from the game itself.
- **2022-11-14** `gameoverload` [🔍╎texturing-and-shading] Okay, so, bit of shading knowledge until this goes in the compendium.
  ```
  Mask Map
  __________
  A mask, the defining texture of a procedural or granular shading system. A mask map allows one to 'mask off' a section of a mesh in order to specify where a specific other map or texture will go.
  
  The map itself will be in black and white, and the function will be engine specific, either black or white for where something goes. Now because a mask is black and white it can be compiled into the channel of a texture, allowing for 4 different masks to be packed into one texture or image.
  
  The masks themselves can be used to mask off a number of different shading techniques, from color or albedo, to specularity, as well as roughness. It can also be used very specifically for emission if the emission on a mesh is not its own material.
  
  A number of games use this system, from Halo Infinite, to Jedi Fallen Order, to even older games like XCOM 2. Each using this system to 'mask off' a number of different shading techniques.
  ```
- **2022-11-15** `gameoverload` [💬╎general-help] To add on, you will need to know the exact textures and the maps within those textures that Halo 3 uses. Those are held in the #📑╎control-map-guide , but if your not willing to press a button, here is what Sniper wrote.
  ```
  Halo 3, ODST, and Reach
  
  Halo 3, ODST, and Reach all are basically the same thing as Halo 2's set up. However there are some minor differences here and there and sometimes there are additional textures (edited)
  [9:50 PM]
  Here are the textures as they show up for these three games
  Diffuse
  RGB - Diffuse
  Alpha - Spec or Opacity
  
  Normal
  RGB - DirectX Formatted Normal (Flip Green channel in Blender/Unity)
  
  CC
  R - Primary Color Change Mask
  G - Secondary Color Change Mask
  B - Nothing
  
  Illum
  RGB - Self Illumination
  
  Material
  R - Specular Coefficient
  G - Albedo Blend
  B - Environment Contribution
  A - Roughness
  
  Not everything has an Illum texture, nor does everything have a material texture. Illum textures are not masks, they are simply the colors used fully for the self illumination (emissive) of a material. When the "Material" texture is used, the Diffuse will not have anything in the specular channel (usually)
  ```
  https://discord.com/channels/819709630540021810/924467210909528064/924493386411552848
- **2022-11-15** `gameoverload` [💬╎general-help] You will still want the control change to match, as it matches the materials baked into your Infinite Sentinel. Even if the materials are baked, it will also handle the color masking too, which will match the materials. It will look like hell otherwise, same for your other baked out textures, which you will need to do correctly, or your Sentinel won't look right at all with what Halo 3 needs.
- **2022-11-15** `gameoverload` [💬╎general-help] It needs as you see above, a base map, which is the RGB Diffuse you see named, as a white or base color map, with the alpha holding specular data if not held in the material texture.
  A normal.
  A color control properly masking the materials associated on the model.
  Illum, which is emission.
  Then your material texture, which holds specular data, albedo blending mapping, environmental contribution mapping, and roughness.
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
- **2022-11-15** `gameoverload` [💬╎general-help] In other words your used to Halo 1, 2 and 3's ( plus Reach ) shading models.
  Those still have relevence in the basic concept sense, but shading doesn't just come down to textures and the maps held within, but the model your dealing with and the shading model your dealing with.
  Your model is your mesh input data.
  Your textures and values are your shader input data.
  Your shading model is the math that takes those two and outputs the final result to the engine and game, resulting in what you see on screen.
  Without one of those two resources and the knowledge of how they function you will be either creating new texture or guessing how they function based on the standards you know.
  You can not have the math and still be able to figure out the shading, you are just guessing and estimating how it works at that point, thankfully we have most of the Halo title dev tools to work with on that front, aside from Reach.
- **2022-11-15** `gameoverload` [💬╎general-help] Bungie's Models, Textures, and the Shading Model each game used ( The math in between. ) Although you might not know the math part if all you used was a shader nodegroup and called it a day.
- **2022-11-17** `gameoverload` [🔍╎texturing-and-shading] Either bake the textures out in Blender to standard ones that are static ones, or make your own shader to work with the base ones..or use an existing shader like the Unity project Halo Infinite shader.
- **2022-11-18** `grandbacon` [🔍╎texturing-and-shading] *I've* only ever used my old infinite shader on MP armor as that's what it was made to do, I don't know how it works on the elite and it definetly won't work in unity since it's a blender shader do if you want it to work in unity you'll have to bake some textures
- **2022-11-30** `gameoverload` [💬╎general-help] OBJ basically holds your model, and one UV set, and that is about it on the important parts.
  
  FBX on the other hand can hold your model, armature, multiple UV sets, texture data ( that is properly assigned so even 3D can read it ), some standardized shader data, and other bits that are dependent on what you are working with.
  
  Basically FBX is best to use in every way.
  
  GLTF is similar, but just simply open source, but not as robust as FBX given FBX has been around as long as Autodesk has used it as a format.
- **2022-12-01** `gameoverload` [💬╎general-help] Sounds like it either couldn't find certain files for something while finding others, and that cascaded into not being able to find a bunch of stuff which crashed it, or it couldn't find anything properly and crashed, or it found everything it could, was too much, and crashed.
  
  In which case, you may have to reassign textures manually in shader so not to crash Blender trying to do it all at once.
  
  Also a pink texture issue, at least for the Spartan Pack, can also be due to a missing HDRI.
- **2022-12-08** `gameoverload` [💬╎general-help] Basically you'd be recreating the texture set minus the normal map.
  Meaning you would need to create the ASG mask, ala AO, Scratch, Grime masking all in one texture via Substance Painter, and then creating RGB RGB masking to fit the helmet so it is 6 zones max in two seperate textures Mask 1 and Mask 0.
  
  However as Goblin said, no point in doing that when Infinite has done this for you via the Reach armor core, no need to basically buy Substance Painter and learn how to use that just to do something already done in Infinite.
  
  Which he is right, most minus a few pieces exist for the Reach core.
- **2022-12-13** `gameoverload` [💬╎general-help] It is not setting it to default.
  1  - armor needs the camera ( render ) icon set to on in the outliner.
  2 - the helmet and other similar issue items need their mask 0 pulled from the input if it is blank, and that other textures are set to non-color. Secondly if a texture shows as pink, it needs to be reassigned to the texture node since that means the texture was originally unfound.
- **2022-12-25** `gameoverload` [🔍╎texturing-and-shading] ```
  Here, the specular reflection and smoothness are defined by a colour and the Smoothness slider. No Texture has been assigned, so the specular and smoothness level is constant across the whole surface. This is not always desirable, particularly in the case where your Albedo Texture maps to a variety of different areas on your model (also known as a Texture atlas).
  The same model, but with a specular map assigned, instead of using a constant value.
  The same model, but with a specular map assigned, instead of using a constant value.
  
  Here, a Texture map controls the specularity and smoothness. This allows the specularity to vary across the surface of the model. Notice the edges have a higher specular effect than the centre, there are some subtle colour responses to the light, and the area inside the lettering no longer has specular highlights. Pictured to the right are the RGB channels controlling the specular colour and strength, and the Alpha channel controlling the smoothness.
  ```
- **2022-12-25** `gameoverload` [💬╎general-help] Well I was gonna say that 3 supports a number of stuff, not just what you have on the default aggressor shader, which you can change to match your needs. Specifically you can change these controls to change what maps you can use on any shader you have.
  ```
  Diffuse
  RGB - Diffuse
  Alpha - Spec or Opacity
  
  Normal
  RGB - DirectX Formatted Normal (Flip Green channel in Blender/Unity)
  
  CC
  R - Primary Color Change Mask
  G - Secondary Color Change Mask
  B - Nothing
  
  Illum
  RGB - Self Illumination
  
  Material
  R - Specular Coefficient
  G - Albedo Blend
  B - Environment Contribution
  A - Roughness
  ```
  [attachments] image.png
- **2022-12-28** `gameoverload` [💬╎general-help] Baking defautly through Blender is kinda f'ed, so you will will simply need to make an unconnected texture node, make a new texture at resolution^2 ( 512x512 for example ).
  
  Then make value or RGB nodes for each of the values you need for each of the shader elements your using, and set those to the values or colors used on your BSDF.
  
  Then you connect each of those nodes one by one to the surface input, baking via the emit option for each of those, saving the images you get externally as you do each one.
  The normal map you need is then the easy part, since you simply set the bake to normal and go from there.
  
  Make sure you have your unconnected texture node selected every time you go to press the bake button as well. Making sure you do so for every material slot your baking on to the blank temporary bake texture.
- **2023-01-09** `kelcken` [💬╎general-help] Trying to bake some textures for the Infinite Banished vehicles In blender using the ones uploaded here, and the Wraith, Ghost, and Chopper have all worked fine, but the Banshee is getting a blue tint and not baking  the emissives at all.
  
  I've looked at the shader setup (identical) as well as the bake settings (also identical) and I can't figure out what the issue is. I've also tried out several different influence settings, but none of them come out right on the banshee
  
  If anyone has any ideas, please let me know! Thank you in advance!
  
  First image is of the wraith (working as intended) and the second is the banshee (not working)
  
  Bake results in the bottom left corner.
  [attachments] RenderWraith.PNG, RenderBanshee.PNG
- **2023-01-09** `gameoverload` [💬╎general-help] The way you would actually want to do it is create bake outputs in the shader, and connect those to the material preview input one by one per bake, and then use the emit baking option instead, since the other Blender options rely on scene lighting, but is still worse than just the emit option which just bakes out the emitted color on the model, depending on what is being output.
- **2023-01-10** `gameoverload` [💬╎general-help] Well you wouldn't just need to generate a color map, you would also need to generate the rest of the following:
  https://discord.com/channels/819709630540021810/924467210909528064/928462459944185906
  However, depending on, you can use certain maps from the original asset depending on what it used in the textures of that asset.
- **2023-01-21** `blue.nasca` [🔍╎texturing-and-shading] so im struggling to get a particular shader effect going
  i'm trying to have a piece of armor gradient from the painted armor color to steel, like how carter's legs go from blue to steel
  i've tried color ramps and gradient textures but basically instead of a progression down the texture from one color to the next, its just blending the two together and turning the entire thing one color, which isn't what i want 
  is there anything i can use to make the effect im looking for or am I just SOL?
- **2023-01-24** `hydranaught4444` [💬╎general-help] well it's kinda strange because the sentinel does its illumination texture completely differently but since you can't actually see it on the texture i just hope it works properly if i use it in another game like reach
- **2023-02-08** `faminemc` [🔍╎texturing-and-shading] hey so its probably just my pc but I was using the halo reach spartans pack to make my own spartan and it was fine but when I started trying to make my own halo reach spartan and I turn on textures my blender freezes
- **2023-02-22** `planetalexanderprojects` [🔍╎texturing-and-shading] Download can be found here:
  https://www.405th.com/forums/threads/procedural-armour-texture-setup-for-blender.53457/#:~:text=Cycles%20render%20engine.-,22/02/23,-%3A%20Version%202
  [embed] Procedural Armour Texture Setup for Blender https://www.405th.com/forums/threads/procedural-armour-texture-setup-for-blender.53457/ 22/02/23: Version 2 is available, download at the end of the post!
  
  I recently made a Belos Horan helmet from MCC that was to be used in a render of a spartan. To make 
- **2023-02-28** `flerp` [📜╎ripping] So, I have every map, model, texture, and sound from H1, H2, and H3, but want to rip Infinite
  There any tools available for extraction? I assume It's structured differently, unless it's still a continuation of modifying the Havok engine
- **2023-03-15** `jacc97` [💬╎general-help] So with this entire drive folder containing h3 assets with readily available textures, is it just for the overall map where your tutorial comes into play or is it advised to do the same for e.g weapons?  @Plastered_CrawDAD
- **2023-03-16** `Deleted User` [🔍╎texturing-and-shading] im trying to port some textures from blender and they show up like this, what am i meant to do?(its a sangheili ultra helmet from halo infinite)
  https://i.gyazo.com/ebdb483e1769d654bf85530562a03b0b.png
  [embed] https://i.gyazo.com/ebdb483e1769d654bf85530562a03b0b.png
- **2023-03-20** `adorabirb` [📜╎ripping] use umodel. set to version 4.21 and go to the shared texture folder. you'll see H1 and H3 in there. it'll contain all the skins and high res pbr textures for CE from the MCC menu
- **2023-03-22** `jona_r6` [💾╎substance-suite] This is gonna sound **FUCKING** stupid but will https://discord.com/channels/819709630540021810/819730994343968788/1063477492075212830 help with imitating wear/armor textures from infinite?
- **2023-03-30** `icedcoffee4849` [💬╎general-help] hallo, i'm trying to bake some camo textures, and this is just on the color channel, but the texture (only on the camo part) ends up being too bright, i have no clue why. any ideas? also, in case you're wondering what the other nodes are, i'm replacing the node wrangler by a different custom node that's supposed to make a similar function but with avoiding tiling, i also tried it without it, and with the original node wrangler and its the same thing so i dont think it's affecting it in any way
  edit: color ramp is also missing because i want to leave the texture in greyscale, i dont want to change the colors of it. same result with or without it
  edit 2: HELP AAAA
  [attachments] image.png, Chest_Color.png, image.png
- **2023-04-01** `onemadwomble` [🔍╎texturing-and-shading] Need help. Trying to use textures from Halo Infinite ODST Pack. Don't know what the channels in mask_0_control or mask_1_control are for. I believe the asg_control is red=specular, green=roughness. Is there any documentation that might shed light on how I might setup an unreal shader? Would the best approach be to reverse engineer blender shader?
- **2023-04-03** `plastered_crab` [📜╎ripping] This is for Halo 3 and not reach tho so it's a bit different, but still the same where you have to track down the values in mod tools and find each texture for each material
- **2023-04-04** `haloboxstudios` [💬╎general-help] Hey, uhhhh- really stupid question from a really stupid person here. I downloaded the newest spartan pack today and extracted the pack and textures into a new folder on my laptop, but when I open up the Blender program all my assets are locked in a solid gray color. Am I missing something here?
- **2023-04-05** `scouter407del` [🔍╎texturing-and-shading] https://github.com/Scouter407/HaloInfiniteToPBRMetalRough-SpecGlossConverter/releases
  [embed] Releases · Scouter407/HaloInfiniteToPBRMetalRough-SpecGlossConverter https://github.com/Scouter407/HaloInfiniteToPBRMetalRough-SpecGlossConverter/releases A substance painter file that takes the ASG, control zero, control one, and normal map to output the base color as the masks all combined with a 
- **2023-04-06** `scouter407del` [💾╎substance-suite] https://github.com/Scouter407/HaloInfiniteToPBRMetalRough-SpecGlossConverter/releases
  [embed] Releases · Scouter407/HaloInfiniteToPBRMetalRough-SpecGlossConverter https://github.com/Scouter407/HaloInfiniteToPBRMetalRough-SpecGlossConverter/releases A substance painter file that takes the ASG, control zero, control one, and normal map to output the base color as the masks all combined with a 
- **2023-04-10** `stanlyhalo` [📜╎ripping] yup... also it helps me dive into why dev made maps take longer... cus back then (like in h3), we just use textures, in infinite, it'll mostly be individually designed pieces for that specific map piece...
- **2023-04-17** `gameoverload` [💬╎general-help] So lets see:
  
  Diffuse Colour [ Funny British moment. ] - Diffuse Colour: Color - Simply Color
  
  Specular Gloss: Vec4 - Specular Values in RGB + Gloss Value in Alpha
  
  Metallic: Vec 4 - Specular Values in RGB + Specular Intensity ( Metallic ) in Alpha
  
  Clear Coat: Vec 4 - Clear Coat Spec RGB + Clear Coat Gloss Intensity in Alpha
  
  Emissive: Vec 4 - Emissive RGB + Emissive Intensity in Alpha
  
  Diffuse Wrap: Vec 4 - Unsure
  
  Damage: Vec 4 - Unsure
  
  Damaged Blend: Vec 4 - Unsure
  
  Damaged Colour: Color - Damaged Color
  
  Spec Gloss, Metallic, Clear Coat Override - Assuming this is to mix all 3 Spec RGB values together in some form.
- **2023-04-21** `gameoverload` [🔍╎texturing-and-shading] You'd need to use a specific compression scheme and a standard shader setup in UE if your baking things down, which doesn't use the original normal map anyway, but a baked down one from the shader in Blender.
  
  Otherwise you'd need to make a shader to use all of the original textures as it does in Blender via the Halo Infinite Material Shader, presumably based on the HI Material Shader since that is the most accurate one at the minute.
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
- **2023-04-22** `tipsy.panda` [🔍╎texturing-and-shading] Maybe someone can help me out alittle bit, Ive got some models for the Mjolnir Mark-V[B] and its ODST variants from halo infinite, But all its texture files are merged together. Is there a chance someone has them separated out?
- **2023-04-26** `gameoverload` [💬╎general-help] You'd have to look at what textures are listed, what their names are, and then if they don't list the individual channel held maps, then you'd need to look at each channel on the model and determine what is what map, which you can use the current shader as reference as the shader I built has shader specific outputs to show specific shader inputs like metal, roughness, AO, and color. Then make your nodes from that information.
  
  You can use my shader as reference if need be.
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
- **2023-05-13** `gameoverload` [🔍╎texturing-and-shading] Okay, so some things I am noticing:
  1 - Make sure to have put in the alpha of the diffuse into the alpha for the haircards.
  2 - Make sure your using rocket_haircards.
  3 - The textures seem to be base color / bc, AO / ao, roughness / ro, unknown / tr, metallic / m, Mask / mk, and others I am not sure about.
  4 - Make sure all of your meshes have auto-smooth on and a weighted normal modifier added to them.
  
  All in all these rips seems not all that good, certain pieces are missing, the mouths and other parts seem to be all messed up in terms of some parts in some parts of meshes, and other parts are their own meshes, textures are missing..all that.
  
  Honestly I even am not sure what needs to all be done to fix those issues since I'd need to deep dive into the tools ( I also don't own the game ) available. 
  
  Really honestly you'd need to debug the game to really know what is to what and what your missing, especially with hair, textures, shading, etc, as none of what I am seeing here is really telling you much nor the tools will most likely tell you much at all if this is all they can pull.
- **2023-05-15** `harambesfistvictim` [🔍╎texturing-and-shading] Not sure what happened here. But all I did was copy the visor set up to another helmet after changing the color of the visor pattern, saved. Went to change the color back and it's all messed up. Almost like something in the textures is magnified or rotated ( Which I did nothing of) So I deleted my visor pattern set up. Still fucked. Imported the helmet from another older file to copy the shader set up. Fucked still. what happen & how do I fix?
  [attachments] Screenshot_2023-05-15_001438.jpg
- **2023-05-18** `zerfer_zplus` [🔍╎texturing-and-shading] Hello, I'm trying to create a toon effect for my spartan. I have created this following a tutorial, but the result is black. If I connect the BSDF point to Color point of Emission I get an error. How can I resolve? I want have the original texture + toon effect.
  [attachments] Immagine_2023-05-18_112229.png, Spartan-01_Lineart5.png
- **2023-05-22** `gameoverload` [💬╎general-help] Well in that case for Unity you'd need to build custom shaders and all of that to work with the textures as they are, which means you'd need shader info from existing shaders to make the shaders in Unity via something like Amplify. As Halo Reach won't just be plug and play at all with Unity default shading.
  
  Plus yes, you will need to apply textures, they won't all be applied on import into Blender, only the Diffuse.
- **2023-05-22** `gameoverload` [💬╎general-help] You'd need shader information and material information reference from the mod tools, which is what anyone wanting Reach shading has to do for all the proper parameters and such. Since those textures work in a specific way that you'd need that mod tools given information for, other than that you can see about using the existing Halo 3 shaders as reference that any maps here may be using, as Reach was built off 3, and then work from there to tweak what you use as reference from there, again in Unity though you'd need to build all custom shaders for that.
  
  TLDR you'd be going into creating a ton of shader stuff that depending on how much you want to go through with this, is going to be a lot of work for Reach environment stuff.
- **2023-05-22** `felixvyra` [🔍╎texturing-and-shading] It didn't help.
  I understand what color is what map now but I have no idea how to combine them.
  I tried 2 different Halo 4 shader node groups that I found in some of the uploaded pre textured assets but even those look incorrect
- **2023-05-28** `a.hypocrite` [💬╎general-help] I've come to bother you all about something arbitrary that I'm just now noticing and can't figure out-
  
  > *Attempting to add camos to individual Spartans- these camos are pre-colored (images ripped straight from google) and are applied through an image texture and plugging the color nodes into the appropriate zone.*
  > 
  > *When shown in material view, the textures are applied. But when transitioning to rendered view, they're replaced with a silver color. For the sake of ruling out lighting not interacting well with blue tones, I've also tested with a model using a darker-colored camouflage. Same issue is occuring.*
  
  How can I fix this? Am I forgetting something somewhere?
  [attachments] 1.png, 3.png, 4.png, 5.png
- **2023-05-31** `metamoq0395` [🔍╎texturing-and-shading] This seems like a better place to ask than #💬╎general-help, i got a mirage IIC im trying to export out, and iv gotten a way to bake the textures onto a png so i can take them to unity. However, I need this camo as well. Is there someway i can find the texture file used for it and plug it into the shader editor in blender? Im a total noob when it comes to the shader editor btw, i just need it for this one use.
  [attachments] Halo_Infinite_Screenshot_2023.05.28_-_08.56.10.92.png
- **2023-06-03** `Deleted User` [🔍╎texturing-and-shading] I’m using this on blender https://drive.google.com/file/d/1aRVcA0V5R5gTeSkuzGjYH5z0hQ0kTspZ/view?usp=sharing and I cannot get the textures to come up at all I’m just trying to get the textures up for this
  [embed] Reach Spartans.zip https://drive.google.com/file/d/1aRVcA0V5R5gTeSkuzGjYH5z0hQ0kTspZ/view?usp=sharing
  [attachments] IMG_6683.jpg
- **2023-06-09** `___swan` [🔍╎texturing-and-shading] My bad if this is the wrong channel. I'm new to extracting Halo models, is there a way to automatically assign correct textures to Reach models in blender or do I just have to guess by name?
- **2023-06-12** `_ashrowan` [🔍╎texturing-and-shading] Fellow shader wizards I need your help setting up shaders for a Halo 5 Sparton for friend in blender. My goal is to bake the textures so I can optimize it for for VRChat. Both Recluse Male and MK V Alpha have the R and G channels inversed in the Open Halo 5 unity shader for a "Fake Zone 3" Not really sure how to apply that in blender.
  [attachments] image.png, image.png
- **2023-06-12** `fiycsf` [🔍╎texturing-and-shading] It was done, because a bunch of people that had no idea what they were doing way back made a halo shader that used the metallic channel as a colour zone. Obviously that didn't work well, yet some people insisted "halo 5 has 3 colour zones"
  
  So I made it work so people who were wrong, would still switch.
  
  Halo 5 is the easiest to convert to unity standard.
  
  Red and green are colour zones one and two.
  
  Blue is metallic 
  
  Alpha is smoothness,
- **2023-06-22** `oakleykulu` [🔍╎texturing-and-shading] Does anybody per chance have VTFEdit?  Laptop is not allowing me to get it off game banana so at this point I'm willing to reach out for some assistance on converting a single texture from a VTF to a JPG/PNG file.  For the one person that has VTFEdit, send me a DM and I'll send you the VTF file for converting!
- **2023-06-22** `chunch` [🔍╎texturing-and-shading] https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/ModularShader2.3
  [embed] Release Halo Infinite Modular Shader (2.3) · AverageTrapEnthusiast/... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/ModularShader2.3 A fully revamped version of the modular shader!
  Now includes node groups that can be used to convert older halo games models an
- **2023-06-27** `gameoverload` [💬╎general-help] https://ambientcg.com/view?id=SkyOnlyHDRI032
  
  For a Sky Only HDRI, I would suggest this one, as you don't need the sky to be a desert color, as that is done through reflections of the terrain in your environment.
  
  I would suggest downloading the `2K-HDR.exr` version specifically.
  [embed] Sky-Only HDRI 032 on ambientCG https://ambientcg.com/view?id=SkyOnlyHDRI032 Asset 'Sky-Only HDRI 032' on ambientCG. Get this asset and 1000+ more PBR materials and textures for free under the Public Domain license.
- **2023-07-05** `nosensemade` [💬╎general-help] Hey I am new to this and looking to work on a few spartan renders for a myself and a few mates. Does anyone know where to find a reach style operator helmet? Btw I am using the halo infinite blender assets and for the sake of ease I am looking for the helmet with infinite style textures.
- **2023-07-07** `spacemonqee` [💬╎general-help] is there a way in blender to bake the final look of a Infinite model into one texture for each channel (diffuse, roughness etc.) even though it has multiple materials and UV sets? for example the scorpion cannon consists of decals, grilles and several other geo, I want to bake it down so I have one shader for the whole cannon with all the decals and greebles and stuff baked down to one texture
- **2023-07-10** `gameoverload` [💬╎general-help] Then you will want to look at Sniper's table for how Reach textures are set up:
  https://discord.com/channels/819709630540021810/924467210909528064/924493386411552848
- **2023-07-11** `fiycsf` [🔍╎texturing-and-shading] ***Arma 3 Workflow is as follows***:
  
  **"$textureSet_AS" - 'Pink Ambient Occlusion'**
  ```
  R - N/A White Channel 
  G - Ambient Occlusion
  B - N/A White Channel 
  ```
  **"$textureSet_CO" - Base Colour Image |[** ***OR*** **]| "$textureSet_CA" - Base Colour with Transparency in Alpha**
  ```
  RGB: Base Colour
  A - Opacity (This channel isn't here for CO's, just CA)
  ```
  **"$textureSet_NOHQ" - Normal**
  ```
  RGB: DirectX Formated Normal Map
  ```
  **"$textureSet_SMDI" - Specular Roughness Map**:
  ```
  R - N/A White Channel 
  G - Specular (Black and White Specular AKA Specular Level)
  B - Glossiness
  ```
- **2023-07-13** `chunch` [🔍╎texturing-and-shading] https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/ModularShader2.9
  [embed] Release Halo Infinite Modular Shader (2.9) · AverageTrapEnthusiast/... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/ModularShader2.9 HALO INFINITE MODEL SHADER VER 2.9
  Changes:
  Fixed the issue where having one value put into all three color slots breaks the fi
- **2023-07-15** `Deleted User` [💬╎general-help] How do I say this; I put the bump and tiff on the zan_seawall material for last resort, and I got everything set up correctly according to a video I found, but the textures are all stretched and there are two color options in guerilla which just makes it more confusing, I'll send some screenshots if I can
- **2023-07-16** `themeta.` [💬╎general-help] Okay so question, I'm trying to make the H3 EVA helmet visor textures on the halo 5 EVA visor but as you can guess the texture just isn't matching up and looking crazy like, what would I need to do to fix that?
- **2023-07-24** `wildbucko` [🔍╎texturing-and-shading] Hey could anyone help me with Baking textures for export?  I am using Halo Reach armour and I wanted to bake the materials into one for export, but following a tutorial, I smart UV project and it messes up the textures
- **2023-07-29** `aparttimehuman` [🔍╎texturing-and-shading] okay has anyone messed with the H3 Spartan in Unity? The right shoulder makes no sense and I can't get the textures right. When I put mp_masterchief on it, the texture isn't right at all, but when I put personal Decals on it just does this. The back of the piece looks right but the rest of it just looks like this and I don't know how to fix it.
  [attachments] image.png
- **2023-07-31** `gameoverload` [💬╎general-help] Which would require MCC to use the mod tools, but I would suggest it if you want those textures to work at least similarly to how they work in Reach itself..or at least seeing how they work in Reach itself anyway..
- **2023-08-07** `gameoverload` [💬╎general-help] You'd have to set up an actual shader for it, unless you wanna do your own, which if that is the case, check #📑╎control-map-guide for Halo 5's texture packing order as a start to making one of your own.
- **2023-08-27** `gameoverload` [💬╎general-help] That is why I explained things for the script bit and piece, but he was going to the shader tab like we were already going to, to look at the textures to make sure any were blank, again, like I was already having him do.
  
  He wasn't finding his own solution there, that was with the script stuff he thought about, but directed him away from and explained since that wasn't the issue, while for the texture stuff, I was pointing him to that.
  
  So I was doing what you do also since I have to take that approach at work also, but the texture part was not on him, I pointed him to that instead.
- **2023-09-08** `ProBot ✨` [📜╎ripping] #Halo Infinite Model and Texture Extraction
  Goes over everything you need to get it working step by step
  
  I made sure to start from absolute scratch as if nothing was installed too so you can see the full process (only takes around 5 mins of actual installing and setup. I just spent a bit of extra time to show it in action and give some extra tips on finding certain models)
  
  Find the video here: https://youtu.be/AOStYxiDLis
  
  Get CR4B Tool here: https://github.com/PlasteredCrab/Halo-CR4B-Tool
  [embed] How to rip Halo Infinite Models and Textures with CR4B Tool and HIRT https://www.youtube.com/watch?v=AOStYxiDLis This video goes over how to extract Halo Infinite models and textures (even batch extracting) using HIRT CLI and CR4B Tool.
  
  It is a step by step guide which I start from complete scratch
- **2023-09-10** `adv_eon` [📜╎ripping] Has anyone ripped the textures for Jorge's chaingun (Including normal and control maps) from Halo 5? I want to see if those textures could be repurposed for the Reach version
- **2023-09-10** `adv_eon` [📜╎ripping] I think you'll appreciate an update on this given the effort you went to to find where I could get the textures, but the textures are compatible with the version of the weapon in Reach minus the heatguard on the weapon (Which is due to those using unique textures from Reach. But safe to say it doesn't look bad
  [attachments] HighresScreenshot00487.png, HighresScreenshot00488.png
- **2023-09-15** `gameoverload` [💬╎general-help] Again, I get that, no need to say it twice.
  Anyway, you need to duplicate your color ramp so you have 3 of them, pressing the little top left arrow on each of them to minimize them so you have space to place them there.
  
  Then you need to connect your image texture node to each of those color ramps, with the first being already done, then you need to connect each of those color ramps to each of the 3 color inputs for that zone.
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
- **2023-09-24** `10vepaw` [💬╎general-help] So I am not sure what to do the halo infinite warthog doesn’t have any color textures its just normals and red and green textures 
  
  I’ve imported the model to roblox and I’ve gotten it to work just like in the game
  All The wheels turn, the turret functions and it has a suspension chassis
  [attachments] image.png
- **2023-10-03** `felixvyra` [🔍╎texturing-and-shading] Hey Guys.
  
  I haven't been on this server for some time as I have been doing some other stuff, and although this isn't related to Halo I have a bit of a texturing issue and I don't know a better place to ask for help.
  
  So. The last few days I have been trying to rip some assets from a game called Rainbow Six Siege. It's a long and complex process so I'm not going to go into it right now but TLDR: the guns have different skins and some of the "Lower quality *universal*" skins use a simple image that get's applied to all the guns, instead of having a different base color map for each one.
  My problem seems to be with the UV mapping. In the Images provided below you will see how the gun looks completely fine using the "no skin" base color map but If I try to apply the "Peacock" skin only SOME parts of the gun look correct.
  
  This is super weird and not something I have ever encountered before.
  I tried to look for some additional data that I might have missed but there is nothing. No extra information in the alpha or anything in the ripped files.
  
  Any help would be appreciated.
- **2023-10-06** `Deleted User` [💬╎general-help] Has anyone worked with the Halo Reach Forge World rip?
  i grabbed it from the MEGA.nz folder, and ive been trying to repair it for use in a Unity Based Social VR game, i did the sky on my own, its not exactly the same but its close, however, i do not know how ill make the terrain as its using textures to blend multiple textures together
- **2023-10-09** `caldoric` [🔍╎texturing-and-shading] @ayomide I'm just trying to get a color control ("cc") texture map fixed. The alpha channel is set to one and should be zero, but I'm unable to get at it in GIMP. The rest of the avatar (coloring, etc) is something I can deal with myself.
  
  Also, my profile clearly states "ask before DMing, especially if I don't know you". I don't even know how you sent me a DM request, since you're definitely not on my friends list.
- **2023-10-16** `gameoverload` [💬╎general-help] You don't need to do any of the location, rotation, nor scale stuff in UE since as you can see, none of the default values for that were changed at all in Blender.
  
  You also do not need to invert the normal map, unless it was baked out as OpenGL ( You can change this in Blender's Cycles bake settings. ), otherwise, it should be DirectX by default.
  
  You don't need any of the strengths unless you have them in Blender.
  
  Also make sure your non-color maps have sRGB turned off in UE.
- **2023-10-21** `gameoverload` [🔍╎texturing-and-shading] Except what you mean is HI is just doing one shader, then ( without seeing it as a reference in HIRT ) is slapping over another shader over that as needed, and swapping back and forth depending on rendering needs, often keeping on the other say for example in the customization menu when your flipping through customization items.
  
  Probably combining that all into a texture then the main actual referenced shader uses that data for maybe when your in a match or something...
- **2023-10-24** `macul` [🔍╎texturing-and-shading] This might be the wrong channel and it might be impossible but I was wanting to take one of the halo reach models in the Reach Spartans pack and make it into a vrchat avatar. Part of my problem is that i need to bake the textures into the current model. Not a huge deal except it's already texture painted and the uv map overlaps. Can't unwrap it without serious distortion. Any advice?
- **2023-10-26** `lubby94` [💬╎general-help] I managed to get the basics of exporting figured out, but I'm running into a couple of issues. 
  
  - I can't figure out how to import the textures properly into Unity. I got the spartan file from the old Delta Index server before it died, and the way textures are set up is the images are all alphas, and then you edit the colors in "zones," and I have no idea how to properly import the alphas with the color zones into Unity.
  
  - Half of the spartan's right arm is missing for some reason. I don't know if Unity is reading the faces as missing, reversed or what. I merged the armor on the body as one mesh and did the same with the helmet, maybe that messed up the mesh somehow. When I tested importing the spartan into Unity with all of the armor pieces as separate meshes, the arm issue seemed to be fixed.
  
  I'd really appreciate suggestions, but if it takes too long to explain how to fix these issues, that's totally fine. I can just research it online myself.
  [attachments] image.png, Blender__E__Pictures_VRChat_Avatars_Epsilon_Spartan_EpsilonSpartan_3.blend_10_26_2023_5_07_23_AM.png, Epsilon_Spartan_-_Untitled_-_PC_Mac__Linux_Standalone_-_Unity_2019.4.31f1_Personal___DX11__10_26_2023_5_06_23_AM.png, Blender_E__Pictures_VRChat_Avatars_Epsilon_Spartan_EpsilonSpartan_3.blend_10_26_2023_5_02_26_AM.png
- **2023-10-28** `ProBot ✨` [📜╎ripping] #Halo Infinite Model and Texture Extraction
  Goes over everything you need to get it working step by step
  
  I made sure to start from absolute scratch as if nothing was installed too so you can see the full process (only takes around 5 mins of actual installing and setup. I just spent a bit of extra time to show it in action and give some extra tips on finding certain models)
  
  Find the video here: https://youtu.be/AOStYxiDLis
  
  Get CR4B Tool here: https://github.com/PlasteredCrab/Halo-CR4B-Tool
  [embed] How to rip Halo Infinite Models and Textures with CR4B Tool and HIRT https://www.youtube.com/watch?v=AOStYxiDLis This video goes over how to extract Halo Infinite models and textures (even batch extracting) using HIRT CLI and CR4B Tool.
  
  It is a step by step guide which I start from complete scratch
- **2023-10-31** `charmingquark09` [🔍╎texturing-and-shading] if you are using blender, the way that I have duplicate textures linked is to go into the **"shading"** tab, and at the top of the grey workspace there is this little beachball-looking icon. Click on the beachball and a drop-down menu appears. Either make a new **material **or edit an existing one and name it something easy to type. Click on each object you want to have that texture **separately **(as far as i know, blender wont let you assign a texture to multiple objects at once... which sucks. please correct me if im wrong lol) and using the beachball drop-down menu, apply the texture. 
  
  The good thing about this method is, you can edit just one material and it applies to everything you've assigned to it. Good for having a primary/secondary color that you can change on the fly, or to test out color schemes. To get rid of un-used materials, I believe there is an extension in blender that lets you delete all unused materials, but i cant remember what its called.
  [attachments] image.png
- **2023-11-11** `ProBot ✨` [📜╎ripping] #Halo Infinite Model and Texture Extraction
  Goes over everything you need to get it working step by step
  
  I made sure to start from absolute scratch as if nothing was installed too so you can see the full process (only takes around 5 mins of actual installing and setup. I just spent a bit of extra time to show it in action and give some extra tips on finding certain models)
  
  Find the video here: https://youtu.be/AOStYxiDLis
  
  Get CR4B Tool here: https://github.com/PlasteredCrab/Halo-CR4B-Tool
  [embed] How to rip Halo Infinite Models and Textures with CR4B Tool and HIRT https://www.youtube.com/watch?v=AOStYxiDLis This video goes over how to extract Halo Infinite models and textures (even batch extracting) using HIRT CLI and CR4B Tool.
  
  It is a step by step guide which I start from complete scratch
- **2023-11-12** `dgenpoly` [💬╎general-help] Hello everyone, is there a tool I can use to batch extract bitmaps from HCE and H3? I've been figuring out mapping and H2 has the (halo2-color-plate-extractor) tool and it's made making a map for H2 so much easier. Is there a tool or even a workflow to batch extract bitmaps to a usable format?
- **2023-11-24** `aels224` [🔍╎texturing-and-shading] Kinda weird but wanted to do a little message for the HW1 fans that would like to use the models, since there wasn't a already made shader setup I worked on my own.
  
  Despite not having found all textures the setup works perfectly as long you keep the "Custom Split Normal Data", it helps for the Shiny Normal issue.
  
  Here's a little example of the model with that setup and how easy it is to put the textures.
  [attachments] image.png, image.png
- **2023-12-01** `Deleted User` [🔍╎texturing-and-shading] Hey everyone, I'm trying to bake out the textures on this ODST in Blender here for the use in Maya's Arnold Renderer. It's for a school project. I've noticed that when the UV isn't in the 1 to 1 square I can't bake out maps properly. (They come out all black). And if I try to unwrap the UV again it messed up the texture influence. (Wrinkles on clothing will be horizontal instead of verticle, ect.) So here's the major question. How do I bake out a texture when the UV it's self isn't set in the 1 to 1 position on the UV editor. Sorry for the long post.
  [attachments] image.png
- **2023-12-17** `nihren` [🔍╎texturing-and-shading] Figure I'll try asking in here. Need some help with textures. I'm trying to upscale textures, but if I upscale the Color & Control textures as is it doesn't always work. Worked fine with the HI Grunt, however it's completely breaking with the H5 Grunt. Tried extracting the color channels, upscaling them individually, and then recombining them via nodes in Blender, and it gets close, but not quite there.
- **2023-12-17** `nihren` [🔍╎texturing-and-shading] I try not to touch the normal map, even for upscaling. And any time I've tried it breaks, so I leave it alone. Some models I've worked with recently had the texture detail, like scratches and all that, under a mask or control, but that doesn't seem to be the case here so I'm focusing on the Color texture. Unfortunately, feels like an effort in futility. I tried the Halo 4 Grunt, which is set up similarly, and what solved the problem there was I simply used just one of the extracted channels, in this case the Red channel, and then used the original texture as the alpha, and it worked perfectly. Tried it on the H5 Grunt, and it didn't work.
- **2023-12-21** `baphomathedude` [🔍╎texturing-and-shading] Hello! Im having an issue Baking the textures for the Halo Infinite Elite Armors, everytime I bake the textures, any textures other then the body textures, it comes out either blank or black if saved. Has anyone figured out whats causing this?
  [attachments] image.png
- **2023-12-22** `coarforge` [💬╎general-help] I don't know what aftereffects needs, but you typically can't just export a model with textures, as each application generally uses different shader systems. Some formats (like fbx and dae) can keep some texture and shader information, but both applications have to support it with the same names, and you have to use a shader that can be exported properly (not sure how well blender can usually do that)
- **2023-12-22** `kaespoon` [💬╎general-help] I'm actually looking more for a steel gray so it looks like how it does in Reach, but the HEX colors from Reach end up a lot brighter due to the different texture and shader styles.
- **2024-01-02** `amsierra` [🔍╎texturing-and-shading] Does anyone happen to know where exactly i can pull texture scale data from in assembly for each texture in a material? For example im doing one of the terrain_blend materials for a halo reach map and instead of going by eye like i used to do for the tiling on each texture i was wondering if i can get the game accurate scaling somewhere in the shader data for each texture using assembly
  [attachments] forgemaphalo.png
- **2024-01-05** `tomhogenberg` [💬╎general-help] general help needed with the importing of the sabre from reach, using it as a placeholder/first iteration for an animation. maybe someone knows specifically what causes the texture glitching and behavior with reach assets. its not reliant on the texture(s) and materials applied. anyone has a clue, if this is halo (reach) asset related?
  [attachments] Screenshot_2024-01-05_135220.png
- **2024-01-14** `vantablonde` [💬╎general-help] Damn, can I post my version of elite models on the download forum? I just added a new texture to the elite model from Halo Infinite and changed the colors, absolutely nothing special, but it looks good.  and I wanted other users to be able to use my option if desired.
  
  On the right is the original, on the left is my work
  [attachments] RaVYrdUTyJ8.png, vUldvneU1dE.png, image.png, image.png
- **2024-01-17** `dagnammit00` [📜╎ripping] Does anyone else have a problem with the cr4b tool when trying to import Halo 3 multiplayer spartan textures? I keep getting a message that says "bpy_prop_collection[key]:key "shader" not found"
- **2024-01-18** `_tobyr` [🔍╎texturing-and-shading] Hi all, I recently downloaded CanadianDragon's Noble Team models to mess around with and I'm having some trouble with the materials. Most of the model is showing up as white, both in material preview and rendering. For some reason the shins on Jorge's model are colored in fine, but the rest of the model is white.
  It looks like the materials are set up so that the textures provide detailing to the armor, and the color comes from elsewhere. I say this because the diffuse textures have no color, just detailing, with dark highlights along armor ridges and transparent where there is no detailing. These detailing highlights show up on the entire model, but the army green color only shows up on Jorge's shins -- the rest of the model is colored white. This is puzzling because the shins are part of the same model that composes the whole torso & lower body -- the materials and textures cover those anatomy parts. But only Jorge's shins are colored.
  Oh, and side note -- all the stuff that works uses a .tiff textures in the materials. Some of the stuff uses .png textures in their materials and none of that stuff works.
  I have enough experience in Blender to poke around and make general deductions about what's going on, but not experienced enough to do more than that. I also don't fully understand how to work with materials pulled from game files, with the ways stuff gets bundled into different channels of a single texture to be more efficient. If anyone has personal experience and can hel ...[cut]
- **2024-01-19** `frostbitecinematics` [💬╎general-help] yeah. Basically the texture image files are all fair game to use because they are owned by 343, but the actual node logic in Blender used to combine all the properties together (like colour, scratch and dirt intensity, etc) is technically copyright-able by whoever setup the node graphs.
- **2024-01-21** `dagnammit00` [📜╎ripping] I'm trying to import the Halo 3 multiplayer spartan textures, and I keep getting a message that says "bpy_prop_collection[key]:key "shader" not found". Anyone got any ideas?
  [attachments] ineedhelp.png
- **2024-01-23** `theishikawarin` [🔍╎texturing-and-shading] Halo 5's Assault Rifle and Phantom have their control textures set to sRGB for example for proper shading, meanwhile other stuff like the Carbine, Spartans and Pelicans absolutely have them set to non-colour
- **2024-01-24** `nihren` [🔍╎texturing-and-shading] Feel like this might be the channel to ask in. Does anyone know what the best way to make PBR textures is? The more I look at these H3 vehicles with their PBR textures the more I feel like the rest that don't have them could use them.
- **2024-01-27** `.reaper64` [🔍╎texturing-and-shading] Excuse me I need some help with a armor texture for Reach, you see I am creating one but I don't know if I am doing it right, if I got the back spine, shoes, and shoulder colors for it to be a secondary.
- **2024-02-05** `.williamfaucher` [💬╎general-help] Hey folks, so I'm a bit of a blender noob here. I've got the amazing Infinite Pelican model here: https://discord.com/channels/819709630540021810/958797365085630494/983603593892487239
  But I was wondering if anyone knows how I can export that into Unreal, while preserving the texture paths? I am fully aware I need to rebuild the shader in Unreal, but if at least the textures were still linked it would be a solid starting point.
- **2024-02-10** `gameoverload` [💬╎general-help] Use this, it includes a normal map, both GL and DX:
  https://ambientcg.com/view?id=Tiles072
  You also have funny white version:
  https://ambientcg.com/view?id=Tiles071
  *Technically you could greyscale either base colors and adjust the color via that.
  [embed] Tiles 072 on ambientCG https://ambientcg.com/view?id=Tiles072 Asset 'Tiles 072' on ambientCG. Get this asset and 1000+ more PBR materials and textures for free under the Public Domain license. Tiles 071 on ambientCG https://ambientcg.com/view?id=Tiles071 Asset 'Tiles 071' on ambientCG. Get this asse
- **2024-02-12** `cbunny20` [💬╎general-help] so i just finished adding all of the recoloring how do i export the spartan so i can use it in unity and have all the texture and armor color changes still intact. do i need to compile the color changes i made?? what do i do
- **2024-02-14** `chiefster_4014` [💬╎general-help] There's no way to export the color than to do it with baking, but there is a shader for Unity that should work for what you need.
  https://gitlab.com/FiyCsf/open-halo-reach-spartan-shader-for-unity
  [embed] Cameron "Fiy" F. / Open Halo Reach Spartan Shader for Unity  · GitLab https://gitlab.com/FiyCsf/open-halo-reach-spartan-shader-for-unity This should play nicely with all Halo Reach MP Spartan Textures etc, and is written using the amplify shader editor and is clearly commented should anyone wish to.
- **2024-02-17** `tacticalmeme6640` [🔍╎texturing-and-shading] Anyone know anything about how Ready or Not handles materials? I don't know shit about fuck when it comes to material stuff so I have no idea what I would do with this texture which is apparently supposed to be for metallic, roughness, and AO all at once. The model also came with base color and normal textures and those are easy enough to just throw into the standard principled bsdf since I'm not going for game accuracy, I just need to find out what to do with this
  [attachments] SWAT_Head_Cowboy_OcclusionRoughnessMetallic.png
- **2024-02-17** `vamparical` [🔍╎texturing-and-shading] Hey! Would anyone happen to have any ideas of how to incorporate a Halo Wars 2 Accurate Jerome-092 chest stripe onto the Halo Infinite Mark IV? I first thought to turning to making it a colour zone on one of the Masks but the chestplate is mirrored! Any ideas?
  [attachments] 1600px-HW2-Jerome-03.webp, image.png, image.png
- **2024-02-19** `chiefster_4014` [💬╎general-help] Make sure both materials have the image texture you're baking onto, then use the outputs labeled Diffuse, Metallic, etc. on the shader to bake. You'll have to set bake as Emit for everything except the normal, which for that you would just use the BSDF output for rather than the Normal output, and set bake type to Normal.
  Also when you bake the normal, if you're on Blender 3.6+ you'll need to set the model to shade smooth rather than shade auto smooth. You can set it back to shade auto smooth after baking the normal.
- **2024-02-21** `gameoverload` [🔍╎texturing-and-shading] It goes Zone 0 ( Black below all zones ) then Zone 1 - 6, then scratch is calculated per Zone via the scratch mask as the mask for where the scratch effect occurs, which in game is 1 value for scratch intensity, but is divided into a per zone value in the Blender shader, and then grime is on top of that via the grime mask in the ASG.
  
  The actual names of the base textures in the actual shader are as follows:
  Base Occlusion ( ASG )
  Material ID 0 ( Mask 0 ) ( Zone 1 to 3 )
  Material ID 1 ( Mask 1 ) ( Zone 4 to 6 )
  Base Normal ( Normal Map )
- **2024-02-21** `fiycsf` [🔍╎texturing-and-shading] In the case of Halo 5 the problem people are seeing is due to compression.
  
  Red and green channels contain colour zone 1 and 2.
  
  Blue is metallic
  Alpha is smoothness
  
  There's fringing around the colour zones due to strong compression artifacts, and often this results in bad colour blending when using a shader.
- **2024-02-22** `atlasguitar` [💬╎general-help] Can anyone help/point me in the right direction to properly understand basically how to port a model into H3? 
  
  I'm very new, so I'm finding it a bit difficult to find resources that relate to exactly what I want to do. Lots of MP based stuff, but I mostly want to mess with Campaign, so those kinds of things are harder to find than your run of the mill "here's how to make a weapon do X" kinds of tutorials.
  
  Specifically, I'm trying to wrap my head around porting H2A models to H3 (and presumably, the process for actually porting things into H3 in a workable state will be similar for different sources?). 
  
  I've extracted files using Index, everything's good in Blender, but I'm iffy after that. For example, I was able to create a new weapon for H3 that's the H2A Battle Rifle, and the model ports and loads in Sapien, but I can't get first person to work (telling me a matching node can't be found, which sounds to me like I need to have different naming conventions in the skeleton (if that's what it's called for a weapon) to match H3's BR model naming conventions?) and I'm having a particularly difficult time finding resources regarding bitmaps and shaders, so I can't really get the model textured. 
  
  Another thing I was trying was with the H2A Chief model, and the problem I run into there is a "no animation graph" error and I can't seem to find much info about how to port new models to use existing animations.
- **2024-02-22** `ProBot ✨` [📜╎ripping] #Halo Infinite Model and Texture Extraction
  Goes over everything you need to get it working step by step
  
  I made sure to start from absolute scratch as if nothing was installed too so you can see the full process (only takes around 5 mins of actual installing and setup. I just spent a bit of extra time to show it in action and give some extra tips on finding certain models)
  
  Find the video here: https://youtu.be/AOStYxiDLis
  
  Get CR4B Tool here: https://github.com/PlasteredCrab/Halo-CR4B-Tool
  [embed] How to rip Halo Infinite Models and Textures with CR4B Tool and HIRT https://www.youtube.com/watch?v=AOStYxiDLis This video goes over how to extract Halo Infinite models and textures (even batch extracting) using HIRT CLI and CR4B Tool.
  
  It is a step by step guide which I start from complete scratch
- **2024-02-24** `sukimoro.` [💬╎general-help] Hey, for like the spartan pack with the reach decals, would i do what she did? like have them as seperate objects and just put the ASG and UV maps (in this image is what im talking about)
  [attachments] image.png
- **2024-02-26** `coarforge` [💬╎general-help] At the top of the shader editor, you can also select other material here (so you don't have to switch back and forth between the 3D viewport and the shader editor constantly). In the shader editor (you have to have your cursor inside of it), press `shift+A` (someone please correct me if this keybind changed in newer versions, I'm still used to the old ones), go to Texture -> Image Texture
  [attachments] image.png
- **2024-02-26** `coarforge` [💬╎general-help] If you have everything set up, select the Image Texture node you want to bake to (just click on it in the shader editor) and then in the Properties Panel click the bake button (the panel where you set up the settings under `BLENDER PREFERENCES`). Depending on your blender version, the settings might be under the Render Properties tab instead of the Scene Settings. Also make sure you have cycles selected as the render engine instead of eevee
- **2024-03-06** `josephfilmography` [📜╎ripping] https://www.youtube.com/watch?v=opkK2wbyaT4&t=369s
  [embed] Halo Wars Reach mod now with MkIV armour. https://www.youtube.com/watch?v=opkK2wbyaT4&start=369 I made a low poly version of the cinematic model for the MkIV armour and got it all textured and rigged up. There's still some slight skinning issues that need polish but nothing major. Also the backpack 
- **2024-03-07** `coarforge` [💬╎general-help] I also have model export in infiniteExplorer, which exports some material parameters too (though HIRT I think also does rigs iirc, so it might be better to use that for now). If you open a mode (or sbsp) and go to the mesh viewer tab, you can click on `add all Meshes to Exporter` (or go through individual meshes if you want to). Then, under `Tools -> Export current scene`, you can export those models, which will also export some material parameters in a json file in the same directory you save the dae in (some info is currently still missing regarding material styles, but all parameters from the material and shader variant should be there).
  Using `Tools -> Export Bitmaps for current Scene`, you can also export all bitmaps used by materials used by currently loaded models. 
  I know it's unfortunately not very intuitive to use, I'm just really not a UI designer
- **2024-03-08** `goajango` [💬╎general-help] Hello, i have download the spartan pack 3 of this video (https://www.youtube.com/watch?v=LdmoBY08LTM&t=69s), and I would like to export it to 3ds max. I tried to export it to fbx and obj but the textures are not implemented in 3ds max. Does anyone have a solution ?
  [embed] Adding Custom Camo to your Spartan (Halo Infinite Spartan Pack 2) https://www.youtube.com/watch?v=LdmoBY08LTM [Link to over 1,000 greyscale camo patterns in description]
  
  [NOTE] For new versions of the Spartan Pack you need to duplicate your camo plugin nodes. If you connect 1 camo shader node to al
- **2024-03-09** `celestial_pasta` [🔍╎texturing-and-shading] I downloaded the Halo Infinite Weapons Packs, the models work fine but none of the textures seem to have a colour, they all appear metallic. I haven't a clue how the shader setup works, maybe someone could explain why it's not working?
  [attachments] image.png, BR_textured.png
- **2024-03-10** `coarforge` [📜╎ripping] A quick script to set up materials from an infiniteExplorer export. You need to load the script in blender and run it in the editor after putting in the correct paths at the top. With my test shader, you also need to append the material `testmat_color` to the file first (that shader is everything but correct). I haven't written any documentation for the shaderdb file yet, but basically, it maps material parameters given in the exported json to parameters in the blender material (currently, only image textures, UV map indicies starting at 0, booleans indicating if UV3 (UV2 when starting at 0) should be used and float values are supported, I haven't added colors yet). 
  If there are multiple shaders listed, the script calculates a score for each shader by incrementing the score for each parameter the shader uses that is present in the material by one and decrementing it by 2 for each parameter the material doesn't have but the shader has if it's not marked as optional. If the score is 0 or lower, the shader isn't used. This is far from a great way to do this, but it was quick to do.
  There's also currently no material style info being exported by iE yet, but at least for maps, not too many materials seem to have it anyways from what I've seen
  [attachments] matsetup.py, shaderdb.json, mat.blend
- **2024-03-19** `ljzman10` [💬╎general-help] Hello, does anyone know the best way to export the halo reach Spartans pack into Unreal Engine with all the textures and the meshes separated from each other not combined into one object.
- **2024-03-21** `mutespybird` [🔍╎texturing-and-shading] Does anyone here know how to get Halo Infinite base colors for stuff, I want to port stuff into Unity but have no idea on how to get the actual base textures (excluding normals, and the usual roughness and metallics) into Unity cause to my knowledge there’s no shader setup that supports the way Infinite does textures
- **2024-03-23** `kelcken` [🔍╎texturing-and-shading] Almost have the process for baking these figured out, but running into a few snags I can't quite figure out
  
  First image, bake type is combined, and almost perfect except some areas of the color are darker than others, and there are some strange lines running through the carbon fiber. Figure this has something to do with lighting influence (right now just have the world lighting set up with one of the studio lights so the bake is basically the same as the viewport shading)
  
  Second image, just baking as diffuse, which solves the discoloration issue somewhat (a few issues on some areas of the texture), but the extra stuff (carbon fiber/strap textures) does not appear, so they must be tied to the lighting as well.
  
  Any thoughts on how I can get this working? Been tinkering with it all day, adjusting and testing settings to no avail 🤔 ( did watch the above video to see if they covered anything that seemed relevant, but it didn't seem to offer anything helpful in this scenario)
  [attachments] BaseTorsoBeltex.png, BaseTorsoBeltex2.png
- **2024-03-23** `kelcken` [🔍╎texturing-and-shading] That is in full bright, that's outside of the game in a modeling tool for setting up the meshes
  
  Just the actual details/textures on the suit don't end up looking as good as the one on the left when just baking the color map. Probably just an issue with not being the same format
  
  I'll keep tinkering until I figure out some way to move forward, I appreciate the help though!
- **2024-03-23** `kelcken` [🔍╎texturing-and-shading] That makes sense, but the issue I've had is that the details only seem to show up on the normal maps (when I switch the outs to metallic, rough, etc to bake, the carbon fiber details won't show up), and the normals don't seem to make it distinct enough, so that's why I was trying to bake them into the diffuse
  
  Ended up rebaking all of the normals with some tweaked settings. It's not *quite* where I'd want it, but it's much better, so I'm gonna run with it for now. May manually add the fiber texture with photoshop based on the normal. Will be a bit of a chore (30 color options, I'll have to do it for each), but it'll get the job done
- **2024-03-25** `coarforge` [💬╎general-help] Hi, please use english in this server. You're also much more likely to get an answer that way.
  For your question (I think, I only know very little french), infinite for a lot of things only uses control maps for the materials, so you need a shader that uses those. Someone probably has made one for unreal you could use, or you can bake the textures in blender so that you get the usual textures
- **2024-03-25** `devil426` [💬╎general-help] So I combined a desert eagle with a halo reach magnum, I think it looks cool but it's a static prop no moving parts that and I don't know how to texture or rig so yeah that's fun
  [attachments] Echos_prototype_handcannon.png
- **2024-03-26** `zadeinator` [📜╎ripping] ```halo2-color-plate-extractor "G:\Steam Games 4 quickboy edition\steamapps\common\H2EK\tags" "G:\Steam Games 4 quickboy edition\steamapps\common\H2EK\data" "G:\Steam Games 4 quickboy edition\steamapps\common\H2EK\tags\scenarios\objects\covenant\military\capital_ship\bitmaps\capital_ship_a.bitmap"```
- **2024-03-28** `.crackonos` [💬╎general-help] I dowloaded the Halo Reach models from the all halo assest resources channel, and i have absolutely no idea how to apply textures to the models in Blender, any help please?
  [attachments] Blender_3_28_2024_1_27_17_PM.png
- **2024-03-31** `coastsilly` [💬╎general-help] Having issues trying to get models from halo reach into unity would anyone know??.. im like new to this (halo stuff) ive been doing destiny and i wanna mix some halo into my learning curve. i have my model i want but idk how to like get the textures and stuff into it
- **2024-03-31** `chiefster_4014` [💬╎general-help] When you export as FBX from Blender, make sure to tick the Limit To Visible Objects box so only the armor you want exports.
  Also, textures will usually never import on their own, especially since the spartans have a custom nodegroup for their textures to work. You'll need to find a shader for Unity made for the textures.
  [attachments] image.png
- **2024-03-31** `zadeinator` [💬╎general-help] https://www.youtube.com/watch?v=MGIBLPIz4oM
  
  ngl man this was the second result when i googled "texturing models in unity"
  [embed] Unity Tutorial 4 - How to apply materials, shaders and textures to ... https://www.youtube.com/watch?v=MGIBLPIz4oM This video tutorial explains how to create materials in Unity so that you can change the colour of 3D objects. The video also explains how to use shaders and image textures textures in 
- **2024-04-05** `taskmaster5571` [🔍╎texturing-and-shading] Another question for when someone has a sec. idk why baking is absolutely killing my braincells but this UV map is absolutely jacked is there a way to get these all within the UV map so when I bake all the textures are pulled? in the ODST helmet I posted above some of the textures are missing because I think the UV map has faces outside the UV itself. I try switching the uv to another one which of course just broke the placement of the textures then I tried baking these textures onto a brand new UV but still same issue of the texture being moved around
  [attachments] image.png
- **2024-04-08** `xandion` [🔍╎texturing-and-shading] I've got a sorta general question, is there a way to figure out uh... I guess a way to describe it would be material scaling for things like tiled building surfaces/ground textures from ODST and Reach?
- **2024-04-16** `shady.pl` [🔍╎texturing-and-shading] Ok basically to people who havent read #💾╎blender recently, I cannot for the life of me figure out how to fix the textures for the DMR from the reach UNSC Weapons by chiefster, basically there is a really bad seam and something up with the thing and thanks to ALG we're basically at this point which the issue seems to be the material/textures in the files for the DMR
  [attachments] DMRHELP.blend
- **2024-05-05** `unbatedspy8` [💬╎general-help] I was customizing a Halo Reach Spartan model and when I saved it didn't keep the paint scheme I put on the armor chest piece. I switched to the shading tab, used object mode to select the chest, and then switched to texture paint to color the chest but it did not save the paint scheme but saved the armor attachments I had put on the model. Am I painting the model wrong? I'm completely new to Blender so I have no clue.
- **2024-05-17** `zepumpkineater` [💬╎general-help] https://docs.google.com/spreadsheets/d/1N860qHHlzj7zL2FHCnz4VcGCRSGOxnxa63pKZDwf3Sc/edit#gid=0
  
  Control + F for "Halo" here, you'll find a big mega file with a bunch of assets from Reach
  [embed] Adams Archive (and other) links https://docs.google.com/spreadsheets/d/1N860qHHlzj7zL2FHCnz4VcGCRSGOxnxa63pKZDwf3Sc/edit Information
  
  Models, Textures, Audio, Anims,Sites, Software, Unity, 3D printing, Etc,File list,Extraction guides
  Rip slot 1: Sakuna: Of Rice and Rui...
- **2024-05-18** `chiefster_4014` [💬╎general-help] you're baking it incorrectly. you need to use the Color/Metallic etc. outputs on the shader to bake by plugging them into the Material Output. Once you do that you need to set the Bake Type to Emit.
  For normal maps you'll just want to connect the green BSDF output into Material Output, then set the bake type to Normal. Though before baking you will also have to duplicate the mesh, go into the Object Data Properties and under Geometry Data hit Clear Custom Split Normals Data as that can cause the normal map to not bake correctly.
- **2024-05-24** `gameoverload` [💬╎general-help] Easiest thing to do is set up via CR3B tool, and then unpack the textures from there, and use the texture paths in the shader as reference to the texture files themselves, but you can also view the textures in the image viewer in Blender as well.
- **2024-05-25** `varokist` [💬╎general-help] Hey im back!
  
  I was trying to extract the raw Gravity Hammer texture from the reach weapons pack, but this is what i got instead.
  
  It's transperent and im trying to get this in the actual color and not transperency.
  [attachments] gravity_hammer.png
- **2024-06-04** `zadeinator` [🔍╎texturing-and-shading] https://github.com/SnowyMouse/halo2-color-plate-extractor
  
  This is the texture tool in question btw @Nero 
  
  Should have instructions on how to use it there, but if not lmk
  [embed] GitHub - SnowyMouse/halo2-color-plate-extractor: Halo 2 tag color p... https://github.com/SnowyMouse/halo2-color-plate-extractor Halo 2 tag color plate extractor. Contribute to SnowyMouse/halo2-color-plate-extractor development by creating an account on GitHub.
- **2024-06-05** `chiefster_4014` [🔍╎texturing-and-shading] Infinite's spartan materials are procedural so there's only masks. There's no albedo, roughness, etc. until the masks are used & combined inside a shader. to get what you need you'll have to bake out the albedo, roughness & such from the shader in Blender.
- **2024-06-13** `rook` [💬╎general-help] Hey folks; doing some work for a friend of mine getting his Spartan into Unity/VRChat; it seems to use a lot of shaders/nodes inside of Blender (Named Halo Infinite Shader 3.0) rather than textures (I.E the ones I unpacked). Is there a way to get these packed into reglar PNG files to pull them into Unity? I've tried Baking but it just says that  "No valid selected objects"
- **2024-06-24** `cythecy_` [🔍╎texturing-and-shading] If it helps at all, I'm using Foundation as a guide to figure out which textures are required and using JubSub's Reach shader nodegroup.
  So far it *appears* to be going alright.
  [attachments] image.png
- **2024-06-30** `chiefster_4014` [💬╎general-help] textures don't transfer with models. you'll need to bring in the textures you need manually and set them up
  there's a reach shader for unity you can use somewhere here, I think by either PointDevice or FiyCsf
- **2024-06-30** `chiefster_4014` [💬╎general-help] im not sure what you mean by "node blend". if you're talking about the textures they're just in one of the files that came with the Reach pack.
  yes, you'll need a shader for it to work in Unity. here's one, it says it's for h3 but should work the same for most reach assets.
  https://discord.com/channels/819709630540021810/821051966058004501/881146101632213032
- **2024-07-02** `o.v.e.rh.a.u.l` [💾╎substance-suite] Hello good night ppl. I'm trying to upload some guns from the Assets Packs called *HALO 5 Guardians - Weapons* But when i open on Blender it looks normal and 500% nice. But when I export to Substance to make new textures. The guns looks like this \/
  [attachments] image.png
- **2024-07-05** `ifhq` [💬╎general-help] I'd like to point out that this helmet has an Identical Node setup to the CQB helmet from a reach spartan pack found here, and there it works just fine, but my baking tool DOESNT work in that one, so im stuck between the shader not working in one setup, but the baking tool works fine, and the shader working in another, but the baking tool throws up a bunch of errors and doesnt bake anything properly
- **2024-07-25** `spartanjoe193` [💬╎general-help] To the OpenGl folks. WHat did I mess up here (first texture is cubemap option in OpenGL)
  
  DirectX (from a Halo CE psh):
  ```
  SamplerState TexS0 : register(s0);
  #ifndef FIRST_MAP_IS_CUBE
  Texture2D Texture0 : register(t0);
  #else
  TextureCube Texture0 : register(t0);
  #endif```
  
  My half-assed OpenGL:
  ```
  // test glsl
  
  gsampler2D t0;
  #ifndef FIRST_MAP_IS_CUBE
  sampler2d t0;
      \\if FIRST_MAP_IS_CUBE isn't checked then t0's just a 2d map like t1-3 
  #else
  samplercube t0;
  #endif```
- **2024-08-06** `r93_sniper` [Reclaimer] Tool created by @Gravemind2401 to extract models, textures, and sounds from various halo games. Read the github link for more information on what is and isnt supported, as well as #📰╎reclaimer-updates for any pertinent news.
  
  Program: https://github.com/Gravemind2401/Reclaimer
  Addon Downloads, including AMF importer script: https://www.mediafire.com/folder/dabvr2j5ofe73/Reclaimer_Downloads
  [embed] GitHub - Gravemind2401/Reclaimer: A Halo game file explorer https://github.com/Gravemind2401/Reclaimer A Halo game file explorer. Contribute to Gravemind2401/Reclaimer development by creating an account on GitHub. MediaFire https://www.mediafire.com/folder/dabvr2j5ofe73/Reclaimer_Downloads MediaFire
- **2024-08-06** `r93_sniper` [Source I/O] Blender tool to work with files from source engine directly in blender
  https://github.com/REDxEYE/SourceIO
  [embed] GitHub - REDxEYE/SourceIO: SourceIO is an Blender(3.4+) addon for i... https://github.com/REDxEYE/SourceIO SourceIO is an Blender(3.4+) addon for importing source engine textures/models/maps - REDxEYE/SourceIO
- **2024-08-06** `gameoverload` [Blender DDS] `Blender`
  Multiple DXGI format support.
  Multiple texture type support.
  https://github.com/matyalatte/Blender-DDS-Addon
  [embed] GitHub - matyalatte/Blender-DDS-Addon: Blender addon to import and ... https://github.com/matyalatte/Blender-DDS-Addon Blender addon to import and export dds textures. Contribute to matyalatte/Blender-DDS-Addon development by creating an account on GitHub.
- **2024-08-07** `blackandfan` [PhxTools] Tool used to extract and repack HW1 eras (Alpha, 360 and DE), also able to convert XMB(Alpha,360,DE and HW2) back into a readable XML format.
  https://github.com/KornnerStudios/HaloWarsDocs/releases
  [embed] Releases · HaloMods/HaloWarsDocs https://github.com/KornnerStudios/HaloWarsDocs/releases Documentation related to modding Halo Wars. Contribute to HaloMods/HaloWarsDocs development by creating an account on GitHub.
- **2024-08-07** `reallocal` [💾╎substance-suite] hey everyone, how should i go about setting up halo infinite textures in substance painter? i'd like to port some infinite weapons to a unity game but i'm unable to use the VRChat unity shader as the game requires a specific shader be used for lighting reasons, so setting them up in SP then exporting from there with the export profile intended for that shader works out best for me. i'm just stuck on setting up textures
- **2024-08-07** `reallocal` [🔍╎texturing-and-shading] hey everyone, how should i go about setting up halo infinite textures in substance painter? i'd like to port some infinite weapons to a unity game but i'm unable to use the VRChat unity shader as the game requires a specific shader be used for lighting reasons, so setting them up in SP then exporting from there with the export profile intended for that shader works out best for me. i'm just stuck on setting up textures
- **2024-08-21** `brooen` [Halo 5 Data Importer] converted bitmaps 
  https://drive.google.com/file/d/1_tcvTZyZgA60lORBwv6jyR8s4GR9AhHv/view?usp=sharing
  [embed] Halo 5 Campaign Textures.zip https://drive.google.com/file/d/1_tcvTZyZgA60lORBwv6jyR8s4GR9AhHv/view?usp=sharing
- **2024-08-21** `alteredtech` [💾╎substance-suite] Started messing around with substance painter yesterday. I was curious what the process was called with these types of textures? This is the grenade from the halo infinite weapon armory assets. I messed around with them in blender and know that these masks allow you to adjust the base color without having an actual color texture. I am not sure what to search to learn how to replicate these.
  [attachments] olympus_grenade_frag_asg_control.png, olympus_grenade_frag_mask_0_control.png, olympus_grenade_frag_mask_1_control.png, olympus_grenade_frag_normal.png
- **2024-08-26** `asimplefruit` [💬╎general-help] no idea if this is the best place to ask but! I am trying to make a combination/reference to the unit patch of Pararescue, so I am trying to find an ODST unit patch, the one with the skull, drop pod and all that that I can use without having to go full out and make a new one; Most of this is concept stuff to see how it looks and if people would understand it as a reference, so I could maybe get a texture done for it on arma 3
- **2024-08-26** `masterj2001` [🔍╎texturing-and-shading] I am insanely biased with this recommendation, but if you're looking to build your own textures from the ground up for the spartan armour, I have a 2 part Quixel Mixer tutorial that explains how to make textures for spartan/Halo armour specifically: https://www.youtube.com/watch?v=eYQpmpadpPs
  [embed] Mixer - Master Chief Helmet Texturing Tutorial | Episode 5 | Textur... https://www.youtube.com/watch?v=eYQpmpadpPs In this episode, we import our Master Chief Helmet model from Blender into Quixel Mixer and set it up with our previously baked normal, ambient occlusion, and material ID bakes.
  
  We'll 
- **2024-09-08** `a1real` [💬╎general-help] does anyone know the canonical height or a rough estimate of the height of a jackal shield? im trying to make a custom jackal shield in roblox and i wanna make it look "right"
  
  also does anyone know where to find the textures/models for jackal shields for reach and 3? i can't find them in the drive
- **2024-09-19** `cwallen` [💾╎substance-suite] any ideas as to why substance might be exporting a blank specular for only one of my texture sets on a model? they all come out fine and then there's just the one that's black. it still has the alpha though? its definitely working fine on the model in substance. just not keeping any info on the export. wondering if it's something related to the halo 3 shader? @JackOfChaos
  [attachments] image.png, image.png, image.png, image.png, image.png, image.png
- **2024-09-23** `r93_sniper` [💾╎substance-suite] this is a tutorial i like to send to newbies a lot: https://www.youtube.com/watch?v=RQ-hRk0WHJ8
  [embed] Introduction to Substance Painter - Ultimate Beginners Guide https://www.youtube.com/watch?v=RQ-hRk0WHJ8 This is an elaborate tutorial to help you get started with Substance Painter. Check out our full 4-hour premium series for more training!
  
  https://flippednormals.com/downloads/introduction-to-sub
- **2024-09-24** `ghostthemercenary` [💬╎general-help] Hey so I got a bit of an issue I see coming up on some of the models I have in my library 
  
  For some spartan or ODST or etc models I find them having a silver texture instead of the one they originally have
  
  Example: if I have an ODST with black gloves. Now I have one with silver gloves but I never touched the textures or the model in a while
- **2024-09-30** `frankr2994` [📜╎ripping] Couple questions, do we have any sort of resources that say what some of these different texture maps are? One of the things that I couldn't figure out last night what was what a macro color map was. 
  
  Second question was what all plugins are available for reclaimer? I can only find mentione of architect and nothing else.
  
  Oh and I guess I do have a third question. How are we reading the shader code? Or is that all part of your development process and a mile above my pay grade?
- **2024-09-30** `chiefster_4014` [📜╎ripping] 1. There's #📑╎control-map-guide, though it only describes the main texture types. For the ones that aren't known you can figure them out by looking through the shader code (if it's available) or experimenting.
  
  2. I'm not sure, though I think that's the only one that was made. If you're just ripping from the games then Reclaimer should come with all you need.
  
  3. Shader code can be taken from MCC's mod tools (CE, H2, H3/ODST, HR, H4, H2AMP) or can be extracted from the game (CEA, H2ASP). For H5, Infinite and any others, it's currently not possible to get the shader code.
- **2024-10-01** `frankr2994` [📜╎ripping] Briefly looking I'm surprised at the amount of information and tools available for shader disassembly . 
  I really do need to stop going down rabbit holes though. I'm supposed to be setting guns up in unreal for a game mod that I already have ported in 😅. 
  
  At some point I will be more curious of the processes used in the various model ripping programs that we have for halo. I am noticing that with h2a and h4 any material using multiple textures or having a texture coordinate map will not pull over into blender. It would be truly awesome to get something sorted out to automatically pull some of the more advanced materials in at atleast a basic layout to aid in transfer to unreal for me. My material building knowledge is low so having a reference in blender is basically a requirement.
- **2024-10-04** `scrubbanubnub` [💬╎general-help] trying to help a friend get the h3 prowler with textures into blender, but i ran into an issue trying to test and see if https://github.com/General-101/Halo-Asset-Blender-Development-Toolset is what he needs
  is there another tool for that now or am i just doing something wrong
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2024-10-11** `purplmunkii` [💾╎substance-suite] https://www.artstation.com/marketplace/p/vD87A/rgb-mask-splitter-for-substance-painter
  [embed] ArtStation - RGB Mask Splitter for Substance Painter | Resources https://www.artstation.com/marketplace/p/vD87A/rgb-mask-splitter-for-substance-painter Resources - RGB Mask Splitter for Substance Painter, USD $0.00. It's a simple tool for splitting RGB masks.Tested with SP 11.3.2 might not work on l
- **2024-10-19** `gwenwyn` [🔍╎texturing-and-shading] hello! i need help finding a texture from the reach spartans. for some reason one of my cc textures is fully transparent for the grenadier chest piece (file name: spartan_chest_joshua_cc.png) instead of showing the proper cc file. was wondering if anybody had it or could help me find it, since for some reason its the only thing thats screwing up the colors.
- **2024-10-21** `badblizard` [💬╎general-help] Help. So I've been having this issue and I can't find a work around so I'm in the Halo archive Google drive and I downloaded multiple Halo CE and Halo 2 models, they have the textures but I can't change the Spartans armor color (or any of the characters colors for that matter). Also most of the Halo 1 and 2 maps grounds are flipped upside down and I can't find a way to fix that either. Can one of you go through the process of how to change the color of the characters because the way I've tried it's makes all of the textures the same color besides when I mess with the visor.
- **2024-10-25** `fyko_tsunovai` [💬╎general-help] This^
  
  I've got the models, but I can't seem to get the armor pieces separated, so it just applies the colors and textures and such to the entire model. I'm trying to make my own custom colored variants of the Halo Reach model elites 🙂
- **2024-10-29** `zachafinackus` [🔍╎texturing-and-shading] Hello! I'm new to using Blender and was inspiried by bdoesstuff and some other Halo artists to give it a try. I've downloaded Surasia's Spartan Pack and thanks to Crab's videos I've done quite well in getting the armor set up and recolored to my preferences. 
  
  One question I did have for anyone familiar with the model, is how do I remove/lessen the dirt and grime texture on the armor that comes default? I can see where to change it's color, but I haven't been able to figure out how to tone it down a little. I think it does look good on the upper half of the model, but it's a bit much on the legs where they're almost entirely brown due to the dirt.
  
  Thank you for your time, and feel free to ping me if you can answer!
- **2024-10-30** `chiefster_4014` [🔍╎texturing-and-shading] Halo Infinite uses procmats for armor, weapons and other things, so you'd have to bake out the color, roughness, metallic, etc. using the baking outputs of the Infinite shader in Blender to get them.
- **2024-11-04** `chiefster_4014` [🔍╎texturing-and-shading] Yes, the spartan armor in H4 has the base color, or diffuse in the green channel of the _control textures, presumably to save on resources.
  Instead of using Blender to get base color out, it would probably be easier to separate the green channel out of the control using something like gimp or photoshop.
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
- **2024-11-20** `gameoverload` [🔍╎texturing-and-shading] Out of the ones you will need to bake from in Cycles to the active image in materials:
  BSDF ( For baking the normal, use the normal bake option )
  // For everything else, use emit.
  Bake Color
  Bake Metallic
  Bake Roughness
  Bake Emissive ( Turn White For Emissives )
  Bake AO
  
  Then to make things easier, in like GIMP or such, pack your metallic, roughness, and AO into one texture, in the order of R - AO, G - Roughness, B - Metallic.
  Its usually best to do it that way for compression reasons.
  
  Then just use the texture channel enums for those texture inputs in Godot.
  [attachments] image.png
- **2024-11-28** `gameoverload` [🔍╎texturing-and-shading] Yes, also the masks setup ( using a split color ), is R - emissive, G - Specular Power ( Gloss ), B - Specular Intensity
  
  Meaning for Specular Power, using a math node set to subtract, with A as 1 and B with your specular power input into that, and then input that into roughness.
  
  For Specular intensity, use a map range set to from min .5 and from max 1, then input the result into metallic. However also, take the result of the map range node, put that into a mix set to float, as the factor, with A as .5 and B as 1, and connect the result of that to IOR Level in the Specular category.
  
  For emissive, connect that to a math node set to multiply, with the B as your emissive intensity, and connect the result of that to the emissive strength.
- **2024-12-01** `mikuspartan` [🔍╎texturing-and-shading] Does anyone know were the textures for the OG Halo 2 ODST in the (Steam Version) MCC files, because at this point I swear I have looked everywhere in the files for them I can't find them, I got the model but still can't get the textures, can someone tell me where the textures are of the ODST or if anyone has a copy of the textures please send me a copy please
- **2024-12-05** `chiefster_4014` [💬╎general-help] you can place a decal onto a mesh with its textures using this method:
  https://www.youtube.com/watch?v=MBy62M7TcgY
  this way it'll be affected by dirt, scratches, roughness, etc. newer infinite shaders should also have an input for the decals near the bottom of the shader nodegroup, so once you do the UV setup you can just plug it in and it'll work.
  [embed] How to Overlay an Icon within a Material (Blender Tutorial) https://www.youtube.com/watch?v=MBy62M7TcgY In this Blender tutorial I will show you how to overlay an icon or image within a material, using Blender's shader nodes.
  ● Metal Texture: https://www.cgbookcase.com/textures/old-iron-01/
  ● Electr
- **2024-12-08** `thoro.blend` [💬╎general-help] https://youtu.be/6qXfCM1z4HA?si=1mulOC5tT4W-zecP
  
  wondering if there was a way to do this in the composter?
  
  so instead of rendering an object with the shader I can just apply the effect onto the image itself?
  [embed] Realistic RGB Screen Shader (Blender Tutorial) https://www.youtube.com/watch?v=6qXfCM1z4HA TV Model:
  https://www.blendswap.com/blend/22217
  
  RBG Texture:
  https://drive.google.com/file/d/1rFZHxuYm67flzAWoM_mCknYfKLyiGJPO/view?usp=drivesdk
  
  #blender #blender3d #blendermodeling #blendertutorial
- **2024-12-12** `frankr2994` [🔍╎texturing-and-shading] Ya I've had all that rolling for awhile. Which is how I came onto asking about the combo maps. I see them listed for use in the material but I don't understand how it's used. 
  
  You mention shader code. How do I view that? Is it in foundation?
- **2024-12-13** `cheezballjones` [💬╎general-help] Hey all - Fairly new here, just trying to figure out some Blender stuff, using Halo Models. I am currently just working on changing the poses and such and now I'm trying to colour the model. I've done so in Blender, using the Sculpt mode and just painting each piece, individually. Now my model is looking pretty cool but I can't figure out how to bake the single image texture of the colors that are on the model. Can someone help me by chance?
- **2024-12-22** `paradigm_.` [💬╎general-help] If a model is imported from a video game and it uses udims, but blender stacks all the uvs on one tile when imported, making the color maps chaotic, is there a way to fix this? @ᴡɪʟᴅ ᴛᴀʟᴇꜱ  is having the issue if anyone knows a solution for him. Imports a nomad hazmat from rust and it clearly uses udims but when imported into blender all the UVs are on one tile messing up the entire texture set
- **2024-12-31** `8_pin_eren` [💬╎general-help] Hi yall, nice to meet ya! Dunno if the question has been asked yet, but is there a way I may be able to convert the WH40K Space Marine 2 textures from Strix's weapon pack to PBR correctly? Seems like my normal maps haven't worked correctly even after inverting them in gimp to attempt to make em Open-GL, and the metallicroughness are way too light? 
  
  I've had to resort in attempting to bake the textures and got some progress, but they just don't have the same quality as the textures with the 40K shader included in the .blend file unfortunatley. 
  
  I gtg to sleep rn so I'll be unable to respond quickly unfortunatley, but if someone has answer or method on how I could fix the Space Marine 2 textures correctly, or how to bake them right, that's be pretty baller!
- **2024-12-31** `papyrus95` [💬╎general-help] Hello, I haven't been using Blender for long, but I've been trying to learn using some of the models from this server. I have mainly been using it for renders, but recently I've been trying to use the texture painting feature that it has.
  
  I've only had a small amount of practice with it, but enough for it to work with some projects I've been working on, but I am stuck here. I am using the Halo Infinite Skirmisher that was uploaded by Palpy, and I am trying to make the blue regions on the legs match the techsuit/undersuit seen on the shoulders.
  
  The models UV seems to be unwrapped, and wasn't inside the image square, but dragging the mesh down didn't seem to implement the change I was looking for, despite the mask 0 seeming to have correct color zones.
  [attachments] image.png, image.png
- **2025-01-01** `chiefster_4014` [💬╎general-help] for H4's spartans, the diffuse is in the green channel of the control map and the alpha channel contains the color change zones. White areas are for primary color and grey areas are for secondary color.
  If you don't want to bother with messing with the control map there's a H4 shader you can get for Unity here: https://gitlab.com/FiyCsf/open-halo-4-spartan-shader-for-unity
  [embed] Cameron "Fiy" F. / Open Halo 4 Spartan Shader for Unity · GitLab https://gitlab.com/FiyCsf/open-halo-4-spartan-shader-for-unity This should play nicely with all Halo 4 MP Spartan Textures etc, and is written using the amplify shader editor and is clearly commented should anyone wish to...
- **2025-01-06** `haloismylife650` [🔍╎texturing-and-shading] Just for clarification, I do create procedural textures in Blender and hand-edit some details onto them after baking alongside reordering colour channels for compatibility with H3.
- **2025-01-26** `gameoverload` [📜╎ripping] Generally its not unreal specific, but the general recommended workflow by Epic that most basic games visually follow is A/B, ORM, N, ala base color, occlusion, roughness, metallic, normal.
  Maybe throw in an opacity mask in the alpha of the base color, but otherwise thats recommended.
  
  The reason being alpha packed maps take up significantly more memory both to store as an asset and in memory during gameplay, however the upside is you get the least compression.
- **2025-02-09** `flipdark95` [🔍╎texturing-and-shading] Hey fellas, can I get some help figuring out how to make a shader that uses RGB masks to customize colours? I know Halo Infinite has similar shaders for customizing materials, but I'm trying to figure out how to do something similar for the Battlemechs from Mechwarrior 5. Wanted to start with the Centurion since it's the starting mech for the player in the campaign.
  [attachments] Color_Mask_Question.png, 20250209180725_1.jpg
- **2025-02-12** `killerboy3699` [💬╎general-help] Can someone explain to me what i am doing wrong? ive managed to bake other textures out of blender (only learnt how to 3 days ago) and now im getting this issue with the mk VI helmet from the halo reach spartan pack. im not sure if my setting are to cause this or not
  [attachments] image.png, image.png, image.png, image.png
- **2025-02-14** `kayhotic` [💬╎general-help] wanna try redoing everything proper.
  
  does anyone know how the specular stuff works on the older games? i was told the specular is forward rendering.
  
  all the AC games up until brotherhood have just 3 textures.
  
  A diffuse, A specular, and a normal.
  
  not sure how that translates into a shader proper. especially with it having color spec (sometimes) as his bracer is not supposed to be white like this
  [attachments] image.png, image.png, image.png
- **2025-02-24** `strider_au` [🔍╎texturing-and-shading] does anybody know if there's a shader setup for assets from Anvil engine (i.e. Assassins Creed, Ghost Recon)? the Mask1 dds includes metallic, roughness and a mask for changing asset colors in-game, but the diffuse is just a detail mask and doesn't really do anything color wise. trying to save myself from a headache cause I haven't really found any good resources to point me in the right direction
  [attachments] image.png
- **2025-02-27** `masterj2001` [🔍╎texturing-and-shading] https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/17
  https://projects.blender.org/blender/blender/issues/131605
  [embed] Cycles and surface normals (normalmap shading) https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/17 Somebody else had a very similar complaint/observation to this thread.  Various parts of Cycles will use the geometric normals (or smoothed geometric normals depending o
- **2025-02-28** `masterj2001` [🔍╎texturing-and-shading] I just went and contributed my findings to the thread, so hopefully that can reignite some discussion about it: https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/32
  [embed] Cycles and surface normals (normalmap shading) https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/32 Apologies if any of my terminology is off. I’m not a technical user, but I can see the impact of this issue on my work. I create assets using a pseudo-game modelling wor
- **2025-03-12** `chiefster_4014` [🔍╎texturing-and-shading] if it's a floating decal then you'll have to UV its texture onto the mesh, then use Mix nodes to combine it into the roughness, metallic, etc. this video should show the process fairly well
  https://www.youtube.com/watch?v=MBy62M7TcgY
  [embed] How to Overlay an Icon within a Material (Blender Tutorial) https://www.youtube.com/watch?v=MBy62M7TcgY In this Blender tutorial I will show you how to overlay an icon or image within a material, using Blender's shader nodes.
  ● Metal Texture: https://www.cgbookcase.com/textures/old-iron-01/
  ● Electr
- **2025-03-31** `chiefster_4014` [🔍╎texturing-and-shading] Giving the decals their own UV should let you keep the resolution high for the decals. This vid explains it well, just ignore what he does for normals unless your decals have their own. https://www.youtube.com/watch?v=MBy62M7TcgY
  [embed] How to Overlay an Icon within a Material (Blender Tutorial) https://www.youtube.com/watch?v=MBy62M7TcgY In this Blender tutorial I will show you how to overlay an icon or image within a material, using Blender's shader nodes.
  ● Metal Texture: https://www.cgbookcase.com/textures/old-iron-01/
  ● Electr
- **2025-04-13** `scalemaster117` [📜╎ripping] Interesting find. I don't recall it on Streets but I haven't fine-tooth-combed that map myself yet. Just the usual screenshots and theater viewing. In Ekur at least, the Infinite multiplayer extract of bitmap assets numbers 17,686 files all randomly named with no folder structure to narrow the search to either civilian or even Streets. Needle in a haystack to say the least. There may be other apps that can do better that I'm not aware of.
- **2025-04-16** `taggynn` [💬╎general-help] Might anyone know where I could obtain / make baked out HDRIs of the Reach Skyboxes?
  Working on a VRC world in Unity and I'd move to have as close as I can get to the OG Reach sky (specifically trying to mimic the atmosphere around Visegrad Relay) 
  I know Halo uses a lot of meshes with effects / shaders / textures for the skybox, but those effects aren't so easily mimicked (in an optimized fashion) to my knowledge, so if anyone has a method or premade skies, that would be so awesome
- **2025-04-19** `crimson_viper97` [💬╎general-help] I'm looking for the Halo Reach target locator screens for a prop I'm working on. I downloaded the UNSC Weapons filepack by @swagster  and looked through the bitmaps and they all just show up as blank when I open them in any program I have. This includes the windows explorer preview, photos, and inkscape. This is not unique to me, @Wraith0912 has replicated this on his own computer. Any help would be appreciated!
- **2025-05-06** `matrixbearer` [🔍╎texturing-and-shading] hiiii is there anyone I can ask for help with shader stuff? Cuz rn I'm trying to translate the halo reach pack textures and shaders into Mtoon material(because funny vrm spartan OC dancing caramelldansen with MateEngineX) but I'm clueless on what to do, should I attempt to bake the textures or just connext some things around?
- **2025-05-13** `gameoverload` [📜╎ripping] If it is for an existing game and not an engine tool, then you would need to bake the material down using the HIMS shader outputs, to get out appropriate textures you may need.
  
  If your in an engine tool, an option also is to recreate HIMS in that engine tool, if you go down that route instead.
- **2025-05-16** `gameoverload` [🔍╎texturing-and-shading] I am going based on the tool rips for now, the problem is they do a bunch of little stuff internally per what tjoener has found that it is honestly not practically worth it to implement every single little thing they do per asset, not so much shader stuffs, but like the per texture adjustments they make.
- **2025-05-31** `solipsol_studios` [💬╎general-help] Does anyone have the matte painting or bitmap files for the 2 different Arks and halo ring sections from the end cutscene of H3?  They dont seem to exist in the files i have.  : \  (pics for reference)
  [attachments] Screenshot_20250531-095258_Discord.jpg, Screenshot_20250531-095304_Discord.jpg, Screenshot_20250531-095308_Discord.jpg, Screenshot_20250531-095321_Discord.jpg
- **2025-06-11** `r93_sniper` [💾╎substance-suite] https://www.youtube.com/watch?v=RQ-hRk0WHJ8
  [embed] Introduction to Substance Painter - Ultimate Beginners Guide https://www.youtube.com/watch?v=RQ-hRk0WHJ8 This is an elaborate tutorial to help you get started with Substance Painter. Check out our full 4-hour premium series for more training!
  
  https://flippednormals.com/downloads/introduction-to-sub
- **2025-06-15** `gameoverload` [🔍╎texturing-and-shading] So I have not looked at the misc stuff for players yet, but this is usually the generic layout for Division 2. This then changes based on the asset type typically, or at least based on the shader used by the asset.
  ### Generic Layout
  ```
  [ Diffuse ] [ _d ]
  RGB - Albedo
  A   - Mask
  
  [ Normal ] [ _n ]
  R   - Normal Y [ High Compression ]
  G   - Normal Y [ Low Compression ]
  B   - Occlusion
  A   - Normal X
  
  [ Material ] [ _m ]
  R   - Metalness
  G   - Roughness
  B   - Detail
  A   - Mask
  ```
- **2025-06-15** `gameoverload` [🔍╎texturing-and-shading] So, it looks like it uses the equipment layout as players usually have for their other gear.
  Minus of course for the little SHD brick, which uses a standard emissive layout.
  ### Player Equipment Layout
  ```
  [ Diffuse ] [ _d ]
  RGB - Albedo
  A   - Alpha
  
  [ Normal ] [ _n ]
  R   - Normal Y [ High Compression ]
  G   - Normal Y [ Low Compression ]
  B   - Occlusion
  A   - Normal X
  
  [ Material ] [ _m ]
  R   - Metalness
  G   - Roughness
  B   - Detail
  A   - Cloth
  
  [ Color ] [ _c ]
  R   - Color 1
  G   - Color 2
  B   - Color 3
  
  [ Camo ]
  R   - Color 1
  G   - Color 2
  B   - Color 3
  ```
  ### Notes
  ```
  - Camo Uses The Color Red Channel For Placement
  ```
  ### Standard Emissive Layout
  ```
  [ Diffuse ] [ _d ]
  RGB - Albedo
  A   - Emission
  
  [ Normal ] [ _n ]
  R   - Normal Y [ High Compression ]
  G   - Normal Y [ Low Compression ]
  B   - Occlusion
  A   - Normal X
  
  [ Material ] [ _m ]
  R   - Metalness
  G   - Roughness
  B   - Detail
  A   - Null
  ```
- **2025-06-21** `ohwhen6442` [💬╎general-help] I'm trying to bake lighting on a BSP, and the reclaimer site uses the command prompt with this "python calc_lm_farm_local.py" but then I get a message that python isn't found. It's my understanding that calc_lm_farm_local.py is in the H3EK folder and I don't have to download python in order to bake lighting. Can anyone give some insight on this?
- **2025-07-06** `solipsol_studios` [💬╎general-help] Since theres so many knowledgeable people in here -
  
  Does anyone know why so many older games like Halo 3, that relied more heavily on the use of normals to imply detail - look crisper, sharper, just generally clearer than modern games that demand every single bump and crevice detail must be actually physically modelled, like Halo Infinite?  Especially when it comes to the textures.  HI gets really blurry compared to H3's clarity.
  
  This phenomenon really stuck out to me when i played Black Ops 2 right after playing BF2042.  In comparison, BO2 was a lot clearer and clean than 2042, which was super blurry in a ton of situations, just like HI.  But in BO2 the model for Harper when in the rain was so damn good, wet and glossy and you could really see every single bump and divot on his uniform thanks to the normals (it seems), it really did look better than 2042.
  
  I dont get why the new games look worse.  : \
- **2025-07-06** `badgerpants1872` [💬╎general-help] Messing with the Infinite prosthetics for use in VRC. Effectively the goal is to wear them like a pair of pants and just alpha mask out the base mesh on the shader side, so it's going to take a good bit of deformation until it fits right.
  
  And I've very quickly run into an issue (feature?) I've never seen before, where half the meshes seems to have all of their transform controls tied to a little orange dot in between the feet.
  This has the effect of causing things like upper arm mesh to swing around wildly instead of rotate in place.
  If someone knows how to go about removing this, what it is, docs to read up on, I'd appreciate it. Thanks.
  [attachments] image.png, image.png
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
- **2025-07-27** `gameoverload` [🔍╎texturing-and-shading] So that is not correct at all.
  You do not need anything connected to the metallic.
  The _r texture connects to the Specular IOR Level input, like I mentioned above, not roughness.
  All of the textures need their alpha mode changed to channel packed. The normal needs to be changed to non-color for its color space.
  
  Then for the normal, in between the texture and the normal map node, you need the nodes in the image in between, so that it flips the green channel, which will flip the data from being Direct X to Open GL.
  [attachments] image.png
- **2025-08-01** `lomnk` [📜╎ripping] Hello i was opening Mombasa Streets (h100.map) in Reclaimer to extract into blender but some of the top parts of the map are untextured as well as most of the billboards and signs. The skybox is also like this Even though I should already have the required .map files for ODST in the same directory (shared.map, mainmenu.map, and campaign.map) (the files i got from https://haloce3.com/xmf/ if that matters). 
  After exporting it anyway and opening it blender these yellow parts are just pure white. 
  Am i missing a file or should i just somehow manually find the textures for these parts myself?
  [attachments] image.png
- **2025-08-06** `marcopolo1223` [💬╎general-help] Hello all, kinda new to this but basically I am recreating my halo reach spartan with inf assets, gotten pretty far though I am trying to use the buttbag from the marine pack as a commando satchel  replacement since the base game (infinite) doesn't include it. Downloaded the file but it doesn't have any textures? Just grey. Idk if i need to find the respective texture in the folder then add it but this is where my understanding of the basics end. (using blender 4.5 which the version calls for 3.0 which may be my problem.) Any help would be much appreciated, thanks!
- **2025-08-27** `manusanram` [🔍╎texturing-and-shading] Hello to all you guys
  I was wondering something; I'm sure you guys have encountered issues with the light volume textures for Halo Reach on Reclaimer (How they are always clipped exactly in half)
  Is there a way to get the full texture? I was working on something and I want to know how to get the resources correctly
  [attachments] image.png
- **2025-08-28** `stahl` [🔍╎texturing-and-shading] I have the model already that I want, it's all set up with the correct legacy version of the Spartan and Recon armour
  
  I am just wanting it's colour to be made (applied) like this.
  
  The textures at the moment are default white.
  
  I am looking to have the correct colours from the game to be applied.
  
  I have the correct hex codes for them as well.
  
  Is that something you would be able to do for me?
  
  Thank you
  [attachments] Recon.png
- **2025-08-28** `spartanjoe193` [📜╎ripping] It's 4 textures actually I know tjis because I [recreated the shader before.](https://discord.com/channels/819709630540021810/832145913177702470/1325316242738778156) [Several times in fact](https://discord.com/channels/819709630540021810/832145913177702470/1325810147754381322 ). It's the exact same shader template as Halo 1's Jackal Shield and Portsble shield but with on slight difference.
- **2025-08-31** `djxsam23` [🔍╎texturing-and-shading] Hello! I've been playing around in Blender and in the FrostyEditor for ripping, "trying" rigging and texturing some Battlefield 4 asset and I was wondering what are the MM and D textures are for. I know that AO, CM and N are important for the 3d model but how usefull are the other 2? 
  
  Also I might need help with the texturing. At the moment I'm using the SWBF2_Materials_Version1.1 node graph in the Shader Editor in Blender to apply textures and I'd say it's okay. It does the job but I wonder if it would be better to do a custom one for Battlefield 4 instead?
  
  Thanks if someone can help me!
  [attachments] AO.png, CM.png, N.png, MM.png, D.png
- **2025-09-04** `manusanram` [🔍╎texturing-and-shading] Hello, everyone. Hope you're having a great day.
  Just wanted to ask for some help understanding how to achieve this effect.
  It's basically fake interiors as seen in the level Exodus
  I wish to achieve this effect, but I do not understand how to do it with multiple textures rather than a cubemap as a couple of tutorials I found show.
  
  NOTE: Sorry for the archaic way to take this pics, they are fro a TV, X360 with no way to upload or send to mail, and no, MCC doesn't have this effect
  [attachments] IMG20250904185320.jpg, IMG20250904185246.jpg, IMG20250904185232.jpg, IMG20250904185203.jpg, IMG20250904185023.jpg, IMG20250904184654.jpg, IMG20250904184652.jpg, IMG20250904184558.jpg
- **2025-09-05** `42manako` [🔍╎texturing-and-shading] does anyone here have expierience modding ArmA 3?? I need help with something
  I found a beautiful model of a KrAZ Shrek-1 MPV from an ArmA 3 mod I downloaded, but the texture wont apply properly.
  I used PBOManager to extract the .p3d and .paa files from the pbo, and using the Blender addon ArmA Toolbox for Blender I imported the .p3d model. I converted the .paa textures to .png successfully, but when I try to apply the texture to the 3d model, it appears as a solid color only. Any ideas?
  [attachments] Screenshot_247.png
- **2025-10-08** `halostrophe` [💬╎general-help] Is someone able to help me understand on adding decals to models? Particularly Spartan models?
  I've been following this guide https://www.youtube.com/watch?v=rUP8aAG4AQ8&t=150s but I mostly get stuck in connecting the decal's nodes to the main shader/texture
  [embed] The correct way to make decals in Blender (NO SHRINKWRAP) https://www.youtube.com/watch?v=rUP8aAG4AQ8&start=150 Someone made an addon based on the technique! Get it here: https://blendermarket.com/products/easy-decal--stamp
  
  It is a bad idea to use shrinkwraps for decals. I show you a more robust te
- **2025-10-09** `captain_ramlethal` [💬╎general-help] https://youtu.be/CeO1w9Qe6MY?si=FcVeLLZkoJUGIIwb
  [embed] [2020] Extract and import the Titanfall 2 models in blender | A ble... https://www.youtube.com/watch?v=CeO1w9Qe6MY This video will teach you how to extract the models, textures from the game files.
  Applying the textures in the shader tab in blender (part 2): https://youtu.be/UV3SxMEsMe0
  
  Download li
- **2025-10-09** `chiefster_4014` [💬╎general-help] You can't exactly port a .map file into Blender, but you can extract its contents.
  Assembly is used for modifying .map files and reading the data inside but not extracting, so you won't need it.
  Reclaimer is for extracting assets from a .map file like models and textures.
  The Halo Asset Blender Toolset is mainly used for importing or exporting assets like models and levels in the Halo 1 to ODST Editing Kits. It can't be used with .map files.
  Reclaimer should be fine if all you need is to extract models from Halo CE. It used to have issues but I believe they're all fixed by now. If you're downloading the .map files off of a site like https://haloce3.com/xmf then you need to make sure you're downloading all of the shared .map files which are also needed like campaign.map, mainmenu.map and shared.map.
- **2025-10-09** `chiefster_4014` [💬╎general-help] I don't think so, for MCC everything should work just fine. If you see any issues like missing textures from any of the exclusive maps from Halo Custom Edition then you can try swapping the shared maps from the custom_edition folder to the maps folder.
  Also, if you didn't know already you can find what each level's filename is here: https://www.halopedia.org/Map_file
  [embed] Map file https://www.halopedia.org/Map_file Map files, .map files or map cache files, are a file type employed within the Blam engine used by all first-person shooter Halo games. As their name suggests, they are the files which store data for campaign levels and maps for multiplayer, Firefight and S
- **2025-10-10** `tim_irl` [💬╎general-help] Not sure where to post this- is anyone in need of a modified version of Reclaimer that can export .RMF files with the original lightmap UV channel intact? It allows you to keep the original baked lighting from Halo 2 when you pair it with proper shader in blender or other 3d software.
- **2025-10-10** `chiefster_4014` [💬╎general-help] Also iirc General-101's Halo Blender toolset can write lightmap UVs and .bitmap tags for the Halo CE and 2 editing kits for MCC. This could be of some use for extracting the bitmaps from the editing kits.
  GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2025-10-10** `tim_irl` [💬╎general-help] I had to source the lightmap files using ninjaripper during runtime, but the original issue was the geometry itself doesn't maintain the extra uv channel meant for the lightmaps to be applied to if it's exported to RMF.
  There is a lot of interesting character and math that goes into how shadows are baked in H2 (and all the other games for that matter) so I wanted to make sure they could be preserved beyond just being an unapplyable bitmap.
  
  here is a screenshot of the same object with the lightmap applied to them on the uv meant for them vs. the same object with the overlay texture (Sans detail textures, shader nonsense, yada yada.) applied to the uv channel that is currently supported by rmf exporting.
  [attachments] image.png, image.png
- **2025-10-21** `purplmunkii` [💬╎general-help] I recommend watching some general videos to get aquinted to the workspaces if you havent already.
  
  The spartan textures are in the shader workspace (top of your screen is the tab). Youll see a big node with smaller nodes attatched to it, mess with those one
- **2025-10-31** `nihren` [🔍╎texturing-and-shading] No, asset quality on that would be abysmal. Jordan gave permission to use their remade CE models, and I wanted to seek permission from others to use their models, as well as their ports. I want to provide model options for players to swap in to replace what they want provided there's a suitable replacement. Like if people want a more classic Chief, Lehto's Mark V is there as well as his MA5B and Warthog. Scorpion is untextured, but that could be solved. Jordans got CE Covenant, but Melon also has a CE inspired Elite pack using the Infinite Elite as a base. There's some more faithful M6D models floating around(the one in the remake reminds me of the M6G2 from Halo 3 with the way the sights are on the back of it.)
- **2025-10-31** `nihren` [🔍╎texturing-and-shading] Did a full upscale of all the Reach Elite Minor textures and used Diego's Reach-on-Infinite Elite as the test for it(didn't upscale the Infinite textures, I've had issues with those before. Something breaks.) Original on the left with the game textures, not the menu textures. And the upscaled on the right.
  [attachments] image.png, image.png
- **2025-11-02** `solipsol_studios` [💬╎general-help] Probably dumb question, but where do you find Chiefs actual colors for his armor in CE?  I have all the files i think, but his texture file is a white spartan.  Im still new to this stuff and ive got no idea how he gets his color.  Same goes for the elites and grunts.  : \
- **2025-11-04** `chiefster_4014` [🔍╎texturing-and-shading] If you already have it as an image then you can give it it's own UV and apply it in the material. This vid explains it well, just ignore what he does for normals unless your decals have their own. https://www.youtube.com/watch?v=MBy62M7TcgY
  If you don't have it as an image then I can't really help since I'm not good at drawing.
  [embed] How to Overlay an Icon within a Material (Blender Tutorial) https://www.youtube.com/watch?v=MBy62M7TcgY In this Blender tutorial I will show you how to overlay an icon or image within a material, using Blender's shader nodes.
  ● Metal Texture: https://www.cgbookcase.com/textures/old-iron-01/
  ● Electr
- **2025-11-06** `thatwotwfan2023` [💬╎general-help] So I got good news and 2 bad News
  
  Good new is I figured out how to get the Textures to work and now I can see the armour glory
  
  1st bad news, its the Wrong Textures im looking for. Im looking for the campaign Textures (with 117 on the chestplate) whiles this have the Multiplayer Textures. No offense to the person that extracted both Model and Texture. So im gonna keep searching for the Campaign Textures somewhere
  
  2nd and Final issue, Both Model and Textures are very High for the avatar im making in Vrchat (im trying to make it Quest + PC) so im gonna try to figure something out. 
  
  But thank you all for the help so far
  [attachments] image-144.png
- **2025-11-07** `angoose651` [🔍╎texturing-and-shading] howdy
  my friend is trying to turn a spartan model he put together into a vrchat avatar
  it uses the "better spartan pack" and the spartan pack as a base
  hes struggling with the texturing aspect
  im not really familiar with how vrchat avatars work but i figured id ask on his behalf on how to move forward
  I think we need to turn the many different materials of the spartan model into once colour image texture, one normal map, and probably a couple other things idk
  any help would be appreciated
  (btw i think hes already figured out how to convert the rig into a vrchat-compatible one so that whole part is fine)
- **2025-11-08** `chiefster_4014` [🔍╎texturing-and-shading] Here's a link to a pretty thorough resource for the main object shader that CE uses. Different versions of CE use different channel orders for the multipurpose map (the red/green/blue texture). Yours looks like it's in the Xbox channel order which the page also describes.
  https://c20.reclaimers.net/h1/tags/shader/shader_model
  [embed] shader_model https://c20.reclaimers.net/h1/tags/shader/shader_model The shader_model tag is used for opaque materials on object models. It supports features like map an...
- **2025-11-14** `koax117` [📜╎ripping] @Scav I did my best to see if any jackal or skirmisher files I have had the materials named in the same manner, and none are. I don't know if someone else knows better.
  But i took a coating, applied it to a spartan, copied the all the shader info minus the textures, pasted it onto the jackal, and slotted the textures into it.
  be a lot of work, but definitely doable! results will be mixed though
  [attachments] image.png
- **2025-11-16** `thealmightyloafer` [🔍╎texturing-and-shading] To anyone that might be able to help. I’m trying to get my spartan I’m making from blender to unity. Everything is going to plan except one thing. The baked textures don’t line up for anything except the CORE. The core is fine but the attachments helmet, and gloves are all giving me grief. The ORM, colour and normal maps bake out fine but when applied to the model in unity they just appear black on the model. I’m following OneWeirdGuy’s tutorial and he glosses over the attachment side of things for the uv maps. If anyone can help it would be greatly appreciated
- **2025-11-16** `thealmightyloafer` [💬╎general-help] To anyone that might be able to help. I’m trying to get my spartan I’m making from blender to unity. Everything is going to plan except one thing. The baked textures don’t line up for anything except the CORE. The core is fine but the attachments helmet, and gloves are all giving me grief. The ORM, colour and normal maps bake out fine but when applied to the model in unity they just appear black on the model. I’m following OneWeirdGuy’s tutorial and he glosses over the attachment side of things for the uv maps. If anyone can help it would be greatly appreciated
- **2025-11-16** `solipsol_studios` [💬╎general-help] Probably a really simple question to answer and im just dumb lol, but where do you find Chiefs actual colors for his armor in CE files?  I have all the files i think, but his texture file is a white spartan.  Im still new to this stuff and ive got no idea how he gets his color.  Same goes for the elites and grunts.  : \
- **2025-11-20** `purplmunkii` [🔍╎texturing-and-shading] In blender theres a converter chunch made for halo 5 to infinite, somewhere in the upload folder. If you want in painter youd need to replicate the masks for the infinite shader
- **2025-11-24** `kozmetic` [💬╎general-help] I’m working on a halo infinite series right now, and in it I have animated fight scenes that I’m doing. The issue here, is that apparently with EKUR you can get multiplayer maps, but not the textures for them, so they’re just white. The animation is fine, I can handle that myself, but how would I do the background, because the camera moves around so green screening it would be incredibly difficult.
- **2025-11-24** `kozmetic` [📜╎ripping] I’m working on a halo infinite series right now, and in it I have animated fight scenes that I’m doing. The issue here, is that apparently with EKUR you can get multiplayer maps, but not the textures for them, so they’re just white. The animation is fine, I can handle that myself, but how would I do the background, because the camera moves around so green screening it would be incredibly difficult.
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
- **2025-11-30** `mrocnyzbik` [🔍╎texturing-and-shading] I try to change the Material of Quake Railgun from being the plastic soker, into something different and thought of using Halo Infinity Shader, but I need to understand how it works. This is the file I am working with. I know it needs different Masks with different levels of RGB, but I do not know what each do, and how, so understanding that would be helpful.
  [attachments] rg_ruthless_HISH.blend
- **2025-12-04** `gameoverload` [🔍╎texturing-and-shading] Okay so:
  _s [ Specular ]
  R - Dialectric Specular
  G - Roughness
  B - Metallic
  
  So, you will need to do this:
  Do not use the metallic input for one.
  
  What you want to do is change the IOR to 1000 first.
  
  Then you need two Mix Color nodes:
  
  Mix Color 1
  Factor - Metallic
  A - Diffuse
  B - Black
  
  Mix Color 2
  Factor - Metallic
  A - Dialectric Specular Multiplied By 0.08
  B - Diffuse
  
  Then Mix Color 1 goes into the Base Color input, and Mix Color 2 goes into the Specular Tint input.
  
  That should resolve your issue in regards to the specular shading.
  
  Also send the channels for the _m texture as well.
- **2025-12-05** `gameoverload` [🔍╎texturing-and-shading] For the base material side of things yes, and I would change your diffuse alpha to channel packed instead of straight, just for the sake of consistency.
  
  Now you showed me the normal and as far as I can tell, unless you looked closer to it and do see it, the normal only seems to have the base model detail, and not the micro-detail.
  
  Which is likely what that _m texture is for, to put the micro-normal detail on the model that the game uses, as well as allow for other material and color changes for each of those ares masked out.
- **2025-12-07** `koax117` [📜╎ripping] Speaking from personal experience, it starts off really daunting and is a lot. But it quickly gets easier.
  
  What I do is gather other maps people have done in the same game and got textures set up. Once I have a working shader, it's all about duplicating it to the other materials.
  
  With something like odst, it's using a lot of shared textures. Coastal highway is hella modular, so once you set up a few, the rest will follow, and be already set up. 
  
  Tackle it in chunks, and you'll get there. You got this
- **2025-12-10** `un_nonimus` [📜╎ripping] I mean specifically exporting the lightmap UV data from the bsp itself. Then also using the lightmaps for the specific bsps to essentially have accurate baked lighting for levels. 
  
  I know in reach for instance there are some lightmap bitmaps in the tags. However, when exporting the geometry using reclaimer that secondary set of uv data isn’t written for the models. So the lightmap textures are useless.
- **2025-12-11** `bojackdaggah` [🔍╎texturing-and-shading] Hello! I hope its okay to ask this here, but I'm using the Halo Archive Google Drive to import an elite onto Blender, and 'm having trouble figuring out how to add the cubemaps as well as the purple "multi" textures onto the model. I imagine its fairly simple hopefully, but I'm just not very knowledgable on material stuff. I just really wanna be able to use that cubemap shine those elites have. The way the shader is set up is just how it was automatically upon importing save for selecting the texture to use
  [attachments] image.png, image.png
- **2025-12-12** `gameoverload` [💬╎general-help] It depends on how you make the shader, how you have textures set up, and overall, how much it is doing effects wise.
  
  Generally proc mats are good for when an asset needs fairly dynamic changes where static bakes would really hold a visual or feature back. Plus it helps when many objects in a scene can use the same proc mat shader.
  
  Hell it helps if many objects on Characters even at minimum can all share the same shader.
  
  Star Citizen has a proc mat system I am looking at recently, and it gets around needing a bunch of separate object materials by being able to support up to 32 different material instances inside one shader.
- **2025-12-12** `gameoverload` [💬╎general-help] Far as I understand from the actual binary shader point of view is it uses the same material function for one, for every layer, aside from any special ones like dirt, and damage ( damage is not scratch but a special effect not used on anything except like chief far as I know. ), they only use two textures per layer, and the layers are not that demanding resolution wise.
  
  You can make the overall setup more performant technically by making one big sheet you use for each layer texture type, star citizen and helldivers do this internally, so albedo, normal, and roughness would all use one single sheet each and then use some UV math to select 1 texture in that sheet for each to use and then modify the UVs from there to be bigger or smaller, rather than sizing the texture technically.
  
  Their are a bunch of other things you can do to make performance better, but it all depends on the needs of the system and what features it needs to support.
- **2025-12-13** `tjsauce` [💬╎general-help] First I'd run it through this model, it's really got at undoing the original compression:
  
  https://openmodeldb.info/models/1x-DXTDecompressor-Source-V3
  
  Then, depending on the art program you're checking your work in, either crop and save each individual face of the cubemap, or stitch them together into a seamless cross.
  [embed] 1x DXTDecompressor Source V3 https://openmodeldb.info/models/1x-DXTDecompressor-Source-V3 A 1x ESRGAN model by JosephtheKP.
- **2025-12-20** `solipsol_studios` [🔍╎texturing-and-shading] Ok so i have a very limited idea of how most of this stuff works so im just brainstorming.  But im really curious since i have the model and all these files just sitting around.
  
  But would it be possible to take the bitmaps for CE Chief and use them on the Infinite MkV armor to make it look more like the original game?
  
  Like somehow put - 
  
  masterchief_visor.dds or cubemap mirror surface.dds for the visor -
  
  cyborg reflection armor.dds with detail corroded metal light.dds for the armor -
  
  and hex code 576725 - for the color.
  
  But like also still use Infinites normal map and texture for it.
  
  Is this insane or even possible?
- **2025-12-23** `koax117` [🔍╎texturing-and-shading] does anybody have a setup for the Halo 2 Anniversary Forge asset shader? 
  That or helping me identify the channels and what each is (CMB-Color-Metal-??) I need to make a cheat sheet for what every one of these texture maps do across all the games, I can NEVER keep it straight
  I want to say there MIGHT be a post somewhere that someone has it? but my brain is blanking
  [attachments] image.png
- **2026-01-04** `solipsol_studios` [🔍╎texturing-and-shading] Ok two small questions.
  (Hopefully easy to answer)
  
  First question:  Im editing Infinites MkV armor in blender, and i have no idea how these textures work.  Where's it get its color and stuff?  Is this like CEs multipurpose maps?  I havent seen texture files like this before.
  (See attached images)
  
  
  Second question:  When editing the CE Chief model in blender, how do you give it color?  I assume its through a node.  I have the hex code for his color, but like how do i apply that.  (Sorry if im just not seeing it)
  [attachments] -417178436_B5C75DF4_-195180619_result.png, -417178436_B776ADC0_-1062373705_result.png, -417178436_FBE075E4_-462036741_result.png
- **2026-01-04** `gameoverload` [🔍╎texturing-and-shading] So you will need the shader that is in Blender to use any of this, since HI uses a procedural material system, in that it layers on materials over the other to give a look at the end.
  
  You typically have:
  [ ASG ]
  R - Occlusion
  G - Scratch
  B - Grime
  
  [ Mask0 ]
  R - Zone 1 ( Black is Zone 0 )
  G - Zone 2
  B - Zone 3
  
  [ Mask1 ]
  R - Zone 4
  G - Zone 5
  B - Zone 6 ( Typically is Dust )
  
  [ Normal ]
  RG - Normal XY ( Requires Recalculating the Z channel. ) ( DirectX Normal )
- **2026-01-10** `gameoverload` [💬╎general-help] This is basically how Halo Infinite can have a bunch of different looking coatings for the same armor piece, because it builds the shading of a piece via layers which can have different properties and textures to achieve different looks.
  
  In fairness, it is easy once you get some basics down for Blender, but to help you find where to input the color if you haven't...
  The Top, Mid, Bottom color if you open up one of the material layers:
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
- **2026-01-11** `koax117` [💬╎general-help] one of the mods may want to give a proper answer on this, just as far as releases go (rather someone who is familiar with all the rules versus me vaguely understanding them)
  
   I hate gatekeeping as well, though I understand in some instances it is either warranted or necessary (a lot of personal work put in/legally can't). My mentality has been, if it is utilizing assets from the games directly, you have free reign to use and share them, as long as they follow the Microsoft guidelines for usage.
  Not 100% sure I'm reading it right, so clarification, you aren't intending to commercialize/profit off it, correct? I'd say as long as you're not, you're golden.
  
  If you're using anything that might of been made from scratch by someone and shared, like a custom sculpted model/texture, that would be the thing to generally check with them on.
  Beyond that, I think you're fine to share, and it'd be sick to have some like elite "boss" kinda vibes like that.
  
  as far as repainting the texture, to what extent are you planning? just colors to existing plates, or painting in new patterns and such?
  if it's the former, depending on what assets you utilize within your kitbash, that could be handled just within the shader settings. 
  but beyond that, substance painter/even photoshop would be my next thoughts
- **2026-01-12** `welxor` [💬╎general-help] Great idea, I wish I could do so but in the two packs I am using I don't see any base color where I could layer one
  I don't know if they are bake or not since I am a texture rookie :ChiefCry: 
  if they aren't: I have found a tool to bake them into unity but not one for blender to dumbify them so I can edit them more easily
  [attachments] Capture1280.png, Capture1281.png, Capture1282.png, Capture1283.png
- **2026-01-12** `welxor` [💬╎general-help] Thanks for the info, but huh , since I know nothing about masking. Of course I will search, but at the moment, would you appear to know precisely what it look like or have an inch about where to search in the puzzle that is the current model's shadings. Because there already is a mask for the gold color, but how do I make it go in other places to make the tribal like marks the armor has (in the photo above)
- **2026-01-18** `solipsol_studios` [💬╎general-help] Im just (still) trying to add a reflection cubemap onto a preexisting model. Thats all.
  
  Apparently im supposed to use an environment map node thats mapped with the reflection output of the texture coordinates node and do something with the box projection mode in the image texture node.
  
  But i have no idea what that even means in regards to adding it all to the model. I know how to add those nodes to the node area in the shading tab, but like how do i get my cubemap to apply to the model?
  
  And i dont even know if all of that stuff is actually relevant, cause im just piecing together stuff from forum answers that barely made any sense.
  
  I do know however that i need an equirectangular map, ive got that part done already.
  
  I just really need to know exactly what buttons i need to press.  : \
- **2026-01-18** `solipsol_studios` [💬╎general-help] Well i have the reflection map already, i just need to slap it onto the spartan.  In the same way that the cubemap is applied to CE Chief.  Like how its just a thing added over the texture to make it look shiney.  But im lost as to how to apply this map image to the new model.
- **2026-01-23** `empoweredsoup` [💬╎general-help] all the textures are already baked, its from one of the reach kitbashes in the community sharing, was just seeing if there was a faster way to allocate textures to material slots without just trial and error
- **2026-01-24** `koax117` [🔍╎texturing-and-shading] is there a specific shader or workflow for Combat Evolved Anniversary campaign environment assets? Is it just following Reach, or is it slightly different?
  I'm trying to follow and understand how the diffuse alpha is used versus the spec/spec alpha (when applicable), as well as the _br and _hdetm textures
  I'm using a bastardized version of a Halo 2 Anniversary shader that had similar nodes to what I *think* I needed, but I'm probably a complete fool.
  people have been requesting a number of things from CEA that I would like myself and like to try and get set up.
  any help would be greatly appreciated!
  [attachments] image.png
- **2026-01-24** `runesoldiero` [📜╎ripping] Its not a concrete list at the moment, since I'm just getting into ripping, but ideally MCC and Infinite, (Reach if neither have some textures and models i want) Far cry 5, Disney Infinity (and possibly any other Disney game with Sugar Rush related assets) and maybe some N64 or PS2 games
- **2026-02-07** `femaleomegagaster` [💬╎general-help] That sounds pretty good tbh, for the specific avi I am looking for, I will go to dizzy but I do have a good challenge for you, I am wanting to suprise my friend with a spartan avi that is both yellow and black, has a helmet that looks like its been fused with the bottom of a gas mask at either side of the chin,  complete with a flame thrower, gas grenade launcher, shotgun and pistol, I will be able to send a image of the colour scheme relatively soon, is this something you think you can do? If so, I will send over the image of the colour scheme, also as a little joke, can you possible add a German flag onto the avi as my friend is German
- **2026-02-22** `echo_mation` [📜╎ripping] Anyone know if I can pull the classic halo 2 ODST model and rig with reclaimer? I've been trying for a while and can't figure it out. I can only find the bitmap, shaders and ai. (Im using the delta halo map)
- **2026-03-04** `koax117` [💬╎general-help] As far as campaign goes, I haven't seen anything concrete out there.
  https://www.reddit.com/r/halo/comments/rfjfrf/unlike_the_pillar_of_autumn_halo_2s_cairo_station/
  ^shows a neat layout of Cairo.
  I know in some of the behind the scenes documentary videos, you can see whiteboard drawings of the map layouts in some parts. But that's WIP and not final in many instances.
  
  Multiplayer wise, this is a good reference
  https://steamcommunity.com/sharedfiles/filedetails/?l=schinese&id=2118066301
  
  that's all I can think of. Always find mission layouts cool, like this Halo 3 of Voi.
  https://www.youtube.com/watch?v=W64UxciD-gQ&t=28s
  [embed] Unlike the Pillar of Autumn, Halo 2's 'Cairo Station' level design ... https://www.reddit.com/r/halo/comments/rfjfrf/unlike_the_pillar_of_autumn_halo_2s_cairo_station/  Halo 3 - Portal at Voi - Surrounding Landscape Revealed https://www.youtube.com/watch?v=W64UxciD-gQ&start=28 Made in Blender with 3
- **2026-03-13** `korrag` [💬╎general-help] ~~I'm having some trouble getting a model setup for use in tabletop simulator.
  
  I'm using the Reach Spartan pack and the Reach UNSC Weapon pack, trying to pose my Spartan with a shotgun and export it as a file to use as a Miniature on the board. I attempted to export as an obj and found out pretty quickly that it doesn't keep the textures and I tried opening the .blend up in unity to the same effect, I tried using the Open Halo Reach Shader for Unity by Cameron "Fly" F but I couldn't get that to actually do anything when I set the normals, diffuse and control map from the Reach Spartans pack. I read somewhere that baking the textures in blender might be a solution but I'm still figuring that out.
  At this point I'm at a loss, any help would be appreciated.~~
  
  Figured it out my own damn self, if anyone needs help porting to TTS via Blender->Unity I will help.
  
  Guide incoming sometime maybe
- **2026-03-15** `shippudenleaf79` [💬╎general-help] For the cause of the either the attachment or the base armor turning black is it because I have in data there’s two bake OV Maps or something else if anyone has ever run into this problem, please reach me out or do you have any ways and methods to fix the problem you know how to solve it please add me on on here on discord if you can assist me and if you would like too.
- **2026-03-16** `r93_sniper` [💾╎substance-suite] https://www.youtube.com/watch?v=RQ-hRk0WHJ8
  https://www.youtube.com/watch?v=KMXpmsmZy38
  @Mr. Grieving
  [embed] Introduction to Substance Painter - Ultimate Beginners Guide https://www.youtube.com/watch?v=RQ-hRk0WHJ8 This is an elaborate tutorial to help you get started with Substance Painter. Check out our full 4-hour premium series for more training!
  
  https://flippednormals.com/downloads/introduction-to-sub
- **2026-03-17** `gameoverload` [💬╎general-help] Okay, I was mainly making sure you weren't using a Mac machine, as those have lower sampler limits, as one issue can occur on Mac where because of the lower sampler limit, not every texture is loaded.
  
  Also to change the material properties, you go into the shading tab, click a particular mesh, and then you either use the side properties window, or the middle shader editor window to go in and change the material properties.
- **2026-03-17** `gameoverload` [🔍╎texturing-and-shading] Do not mess with the textures, those have to be correct for them to work, so I would redo them back to how they were.
  You can use the spartan pack to figure out which texture goes back in the correct slot if you can't undo it correctly.
  
  To change the color you have to go down to the correct layer, and change the color for the layer your looking at, specifically on the nodegroup that handles that layer's properties.
- **2026-03-21** `koax117` [💬╎general-help] if you want to use those kneepads but not have the snowflake, the way I *personally* would go about it is taking the textures in the shader that have that image of the snowflake (some only have it in certain color channels) and manually painting it out in a program like photoshop to match the surrounding area.
  If you're new to blender and 3D, I recommend looking up some tutorials on texturing and materials.
  [attachments] image.png, image.png, image.png
- **2026-04-11** `koax117` [💬╎general-help] @Plastered_CrawDAD 
  weird bug with Cr4b Tool i've run into. For whatever reason, running the tool for the shaders on the Ark messes up the material for this panel. 
  I thought for the longest time it was an incorrect texture issue, but i double checked it, and the UVs, which didn't change.
  don't know if you or anyone else would have insight on this.
  this is from H3, The Covenant, bsp_050
  asset - structure_catwalk_control_lift
  shader - panel_glass
  [attachments] image.png
- **2026-04-14** `exodst.` [💬╎general-help] https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1624
  [embed] Release v2.1.1624 · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1624 new builds:
  
  added support for additional H3 and Reach Alpha builds (bitmaps and render_models only)
  
  misc bug fixes:
  
  fixed a couple H2 bitmaps giving a "bitmap format not supported" err...
- **2026-04-14** `tactiman` [💬╎general-help] I downloaded the Halo Reach Marine mod, which includes an ODST and a female marine, but when I open the file it shows me the two characters T posing and a tree view with the different meshes, textures, and whatnot
- **2026-04-18** `javenator` [🔍╎texturing-and-shading] Does anyone have a method for saving and transferring heavily modified coatings using the Halo Infinite Shader? I was loading a coating with Ekur and modifying it piece by piece for multiple characters, but recently realized I could pull everything but the base textures between characters for each region of their armor. I also tried editing the style json for a custom coating but it seems like Ekur won't load that unless I am doing it wrong. Just wondering if anyone has a faster method they are using.
- **2026-04-24** `winnower` [💬╎general-help] 1 — Most of those armors are from different games, so the textures and material setups are different. So yeah, you kind of have to eyeball it.
  
  2 — Both Arbiter armors (Halo 2 and Halo 5) weren’t made to be colorable, so you can’t really recolor them properly, it’ll look wrong unless you do custom work.
  
  3 — Jega’s armor works like any other Halo Infinite character, you just need to find the right materials and color them.
- **2026-04-25** `mardosunday2343` [💬╎general-help] so i downloaded this model and put in unity with all the right textures and when i put them on it looks really broken almost like the UV maps are done for but everything is fine so i asked my friend and he said that it has something to do with like it has like a custom blender shader or something like that but i was just wondering how to fix this because idk how ofc....
  [attachments] Screenshot_2026-04-25_091404.png
- **2026-04-26** `koax117` [📜╎ripping] Here's the Reach Gravity Hammer textures as targas.
  to get the rest, I recommend setting up Reclaimer
  https://github.com/Gravemind2401/Reclaimer
  [embed] GitHub - Gravemind2401/Reclaimer: A Halo game file explorer https://github.com/Gravemind2401/Reclaimer A Halo game file explorer. Contribute to Gravemind2401/Reclaimer development by creating an account on GitHub.
  [attachments] gravity_hammer.tga, gravity_hammer_bump.tga, gravity_hammer_illum.tga
- **2026-05-01** `papyrus95` [🔍╎texturing-and-shading] Hey there, I was wondering if anyone could help me figure out something regarding baking in Blender. I have retextured the Helldivers 2 MA5C using the shader converter found on @squeak_grenade (#1 SPI Fan) (Sorry for the ping) Halo Infinite Reach ODST Kitbash. I have already baked color, normal, roughness, and emission, but I was wondering how to bake a metallic map with this shader
- **2026-05-07** `haloismylife650` [📜╎ripping] If you mean H2ASP then you could use the cursed method of exporting the bitmap using tool (e.g., `tool export-bitmap-dds "ui\hud\bitmaps\new_hud\backgrounds\plasma_pistol_bkd" "C:\test"\`) and yoink the blue channel in the image editor of your choice. If you mean H2AMP, you're going to have to use Lord Zedd's FontPackager tool to open "font_package_icon.bin" found in "maps\fonts". When in FontPackager, you can double-click the "fixedsys-ui" entry to find the weapon icons, then right-clicking on the desired schematic will allow you to export it. I believe you're going to have to repeat the same steps for the x2 and x3 versions of the font package so you can import them into their respective font packages for Reach and H4.
- **2026-05-22** `chiefster_4014` [🔍╎texturing-and-shading] What each channel of the multipurpose map does depends on the settings set in the material's shader tag. That one appears to be in the Xbox channel order, so the red channel should be the detail mask, green channel is the self Illum mask, and the blue channel should be the reflection mask. This page should describe the shader tag for this material in more detail if you want to be accurate to CE: https://c20.reclaimers.net/h1/tags/shader/shader_model
  
  I can't recall exactly the setup to use the reflection map exactly, but I know the texture itself must be used with the Environment Texture node instead of the normal Image Texture node so it reflects properly. Iirc for the rest you just need a Geometry Node and Vector Math node set to Reflect. The Incoming and Normal outputs of the Geometry node go into the A and B input of the Vector Math node. The output of the Vector Math node then gets put into the Vector input of the Environment Texture node.
  
  I can send a visual example when I'm home, but that'll be over an hour from now.
  [embed] shader_model https://c20.reclaimers.net/h1/tags/shader/shader_model The shader_model tag is used for opaque materials on object models. It supports features like map an...
- **2026-05-28** `koax117` [💬╎general-help] Yeah, that's the World Shader. gives basic lighting and reflection detail.
  quick clip showcasing the difference between using the scene world and not with the settings up top.
  and then, if you pull up the shading tab, and set it to World, instead of Object, you can swap out the environment texture for and one you want. but its there as a base, otherwise it'll be completely flat.
  [attachments] 20260529-0021-58.5071112.mp4
- **2026-06-03** `yesimviolet` [📜╎ripping] does anyone have the textures for mp_extractor_arm/el_extractor_arm from halo 3 the tags 7z that come with the mod tool doesnt have the textures it only has the shader and i cant find them in any of the map files
- **2026-06-03** `koax117` [💬╎general-help] the way they layer the textures for the halogram shaders I don't fully understand.
  I'm still working on the H3 iterations, but the amount of animated texturework that goes into it is insane
- **2026-06-03** `koax117` [💬╎general-help] I don't have the CE mods tools installed, so I can't vouch for how any of it works sadly.
  For when I'm working in H3, I load up Guerilla and will navigate to locate the object's render.model / relevant shader. if there's any textures missing that didn't rip with the model, that's where i'd go to look for it.
  [attachments] image.png, image.png
- **2026-06-03** `koax117` [💬╎general-help] i'm still working through piecing together a good system for doing looping animated textures, but this is the system I have going from a tutorial I followed, it might give you some ideas since big beam go brrrrr. Chiefster was also a lot of help with this because its running off their H3 shader and they cleaned up the loop a bit more.
  https://youtu.be/0uMWdlz1ecc?si=8TEIpJrL0cjYh3sf
  Based on what I've seen from working with the H3 assets at least, they usually have two different noise masks they animate. that'll potentially help give you a lot of that subnoise in the beam as well.
  [embed] Looping Texture Animations in Blender - Comfee Tips https://www.youtube.com/watch?v=0uMWdlz1ecc Animate any texture's location, rotation and/or scale in a seamless loop. Stylized water shimmer bonus tutorial as well!
  
  Psalm 23:2-3 "He makes me lie down in green pastures. He leads me by still waters.
  [attachments] 20260603-1944-52.0056414.mp4
- **2026-06-03** `roman117` [🔍╎texturing-and-shading] Been banging my head against this for two days: every tutorial i find to try and explain how this works requires either a "base color" node or a "principled BSDF" thing, which this model has neither of. And when I try using the uv unwrapping method where I select a rectangle on the chest where I want the number to be, it just puts a miniature version of the entire texture there.
  
  I tried the shrinkwrapping method and it *works* but the texture is way too bright and stands out way too obviously, and if I pose the model even a little bit the texture gets all warbled and misshapen.
- **2026-06-12** `roman117` [🔍╎texturing-and-shading] Still banging my head against trying to add emblems -- might be one of the most infuriating projects I've done for something SO TINY.
  
  Found out about "color override", and I'm trying to use it. Two problems; the texture is upside down for some reason and the rotation options in the "mapping" node don't seem to be of much help. Also, the texture is mirrored on both sides of the chest. I don't know why it's doing that or how to make it stop.
  
  Also, is there a better way to move the image texture around that doesn't involve fiddling with the location and scale bars on the Mapping Node? It's so tedious trying to get it to fit where I want.
  
  Can anybody please help me? I've been stuck on this for almost two weeks and nothing I find on youtube or the server seems to help.
  [attachments] image.png
- **2026-06-13** `roman117` [🔍╎texturing-and-shading] Okay, making some progress at least. I used this tutorial I randomly stumbled across, IDK how it didn't show up through all my previous searching cuz its very good.
  https://www.youtube.com/watch?v=n7FkOaI0vPI
  
  I still have the problem of the texture sticking out like a sore thumb, have to figure out how to make it use the existing textures underneath. But there's another very annoying problem;
  
  There seems to be white lines or artifacts or something all over my model now that get worse the farther I zoom out. They change shape if I move the UV around. I have no idea what's causing this or how to get rid of them, so I'd appreciate some help...
  [embed] The Secret to Perfect Decals in Blender https://www.youtube.com/watch?v=n7FkOaI0vPI You’ve been doing stickers the hard way. Shrink wrap modifiers, floating geometry, projection nightmares—it’s all kind of a mess. So quit that. In this tutorial, I’ll show you a smarter way to apply decals in Blender
  [attachments] image.png, image.png, image.png, image.png, image.png
- **2026-06-13** `chiefster_4014` [🔍╎texturing-and-shading] What game are you asking for? If it's Halo 3 or Reach, the specular coefficient is a flat multiplication into the specular. The specular exponent controls the power of the mix between the specular color and fresnel color. A reflection mask is just a mask to control where cubemap reflections are on a material.
- **2026-06-16** `.unrelenting.` [💬╎general-help] Random question and this is my first chat here, I'm trying to figure out how to basically convert the terrain/landscape from games like Reach into Height Maps to remodify in World Machine (I know the levels are released for Blender) to give some of the landscapes we had from the original game a bit more detail and updated textures. Unfortunately I'm a complete Novice in Blender.
- **2026-06-18** `chiefster_4014` [💬╎general-help] If it's the latest spartan pack then I don't think those errors mean anything, but if you're having issues with materials being completely black/very glossy/ broken in some way then that might be the cause.
  
  Also, what do you mean by some textures aren't accessible to repaint? If you want to change the colors of the material you're on you can do it from the shader editor.
- **2026-06-29** `chiefster_4014` [🔍╎texturing-and-shading] It looks like you have the control map plugged into the color map inputs. That isn't correct unless the material info says that it's supposed to be there.
  Also, you need to change the Alpha setting from Straight to Channel Packed for all textures. If not set, the color of the texture will be multiplied by the alpha channel and can cause the color of the shader and/or the shader itself to turn black, depending on the affected texture.
- **2026-07-23** `koax117` [🔍╎texturing-and-shading] not in this instance. You just imported an .obj, not the textures, nor is that a .blend with all the shaders already set up. 
  you would have to manually set it up yourself
  there is a Reach Grunt pack with them already set up.
  https://discord.com/channels/819709630540021810/1271662648655876096
- **2026-07-31** `setriaphtfk` [🔍╎texturing-and-shading] On this model I have been working on the rip of, these maroon stripes are just plains over the top of the ship’s body. They are just slightly transparent.
  I want to try and make some sort of color mask so is matches up in materials and normals and all that. I have a plan for how to make that work, I just need to know if it’s possible and how to essentially take the stripes and project them or imprint them onto the main texture.
  
  Maybe to simplify: take a cube and a small triangle plain on one side. I want to make the triangle a part of the texture on the cube beneath it. Is that possible? How do I do it?
  [attachments] Phaeton_Class_v2.png
- **2026-07-31** `starlight_warden` [💬╎general-help] Hey everyone, wondering if anyone can offer some insight. I have Fmodel and am trying to pull all of the elite models from Campaign Evolved. I have all but the minor elite. For some reason Fmodel won't pull most of the Minor armor or skeleton. It gives a detex failed to decode texture error. I also can't find the undersuit that all of the main elites share across Campaign evolved to use in Blender. I'm new to this so still figuring things out.
- **2026-08-03** `koax117` [💬╎general-help] if you'd like, you can share screenshots of -
   - Render Properties window: Color Management at the bottom
   - Output window. Color Management and Post Processing at the bottom
   - Sun light properties: strength, exposure, etc.
   - Compositing window: if you have any exposure nodes or anything built in. 
   - World Shader: what the HDRI texture is (by clicking on it) as well as any nodes affecting it.
  
  My guess is with you using that Physical Atmosphere addon it runs a script that changes some settings to help with the rendering, but it is probably set a default lighting configuration that doesn't match your scene. I've used TrueSky in the past and it fucks with the exposure settings in my scenes a lot.
- **2026-08-04** `zoephie` [📜╎ripping] its in the latest development build https://github.com/Zoephie/Baboon/releases to access the unreal engine features you just load your campaign evolved then select chimp at the top which opens a similar view to fmodel from here just locate any skeletal mesh, or static mesh of your choice which we support extraction of .jms or pskx or if you only need the textures you just find any texture of your choice and right click it and export.
  [embed] Releases · Zoephie/Baboon https://github.com/Zoephie/Baboon/releases A native Rust GUI built on blam-tags for editing Halo MCC editing-kit tag files (Halo 3, ODST, Reach, Halo 4, H2A). Provides byte-exact tag reading and editing, asset extraction (bitmaps, geometry,...
  [attachments] image.png, image.png, image.png, image.png, image.png, image.png
- **2026-08-08** `zoephie` [📜╎ripping] can grab latest here. https://github.com/Zoephie/Baboon
  [embed] GitHub - Zoephie/Baboon: A native Rust GUI built on blam-tags for e... https://github.com/Zoephie/Baboon A native Rust GUI built on blam-tags for editing Halo MCC editing-kit tag files (Halo 3, ODST, Reach, Halo 4, H2A). Provides byte-exact tag reading and editing, asset extraction (bitmaps, geometr
- **2026-08-10** `chiefster_4014` [💬╎general-help] I'm more familiar with porting stuff to Halo 4 MCC, but I assume with both games the first thing to do would be to adjust and reposition infinite's model to have similar proportions and the same default pose as CER/H3, then swap the weights to use the weight names from either game. H3 has way less bones in its rig so you'll have to merge some of the weights or redo them. Then you'd bake the textures of the Infinite model into whatever format they need. Both can use PBR textures.
  Them you'd port each model into their respective game. Idk about CER but H3 should be mostly straightforward, you just need an addon to export to the correct format. Them you'd import the textures into the engines, setup your materials, and then that should be it if I'm not forgetting anything.
- **2026-08-15** `papyrus95` [🔍╎texturing-and-shading] Hey there, I'm taking the shoulders from the remakes Mark V and I would like to make it compatible with the Halo Infinite shader/HIMS. How would I make the ASG map, and the mask 1 map in a program such as Substance Painter if it's possible?
  [attachments] image.png
- **2026-08-15** `winnower` [💬╎general-help] Does anyone know a way to "bake" IK into an animation? I'm exporting the reach elite animations but their feet move a lot while idle so I put the bones of the feet attached to a cube via IK and it fixes it and I would like it to be like that when I export the animations
- **2026-08-16** `hunteriskindaindifferent` [🔍╎texturing-and-shading] Hello, Ive got all the reach stuff ready to import. But how do I change the colorable parts of the helmets? They all seem to reference the same diffuse texture. Im assuming I need to link it up so a color can be applied over certain parts with an alpha but I have no idea how to do that in blender
  [attachments] ACA597B8-FCD9-4C2A-9545-AD9D002D711B.png
- **2026-08-16** `chiefster_4014` [🔍╎texturing-and-shading] baking textures with the Reach shaders is kinda tricky since it isn't proper PBR. you should be able to bake a diffuse, specular color, and flat gloss map by going into the nodegroup and using specific nodes as outputs to bake from
- **2026-08-16** `koax117` [💬╎general-help] can find them here, https://github.com/77Mynameislol77/HaloAnimationRepository
  or extract them with Baboon https://github.com/Zoephie/Baboon
  retarget should work fine, just need to line up the armatures and bones
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository A Repository of ripped Halo Animations for use in MCC modding. - 77Mynameislol77/HaloAnimationRepository GitHub - Zoephie/Baboon: A native Rust GUI built on blam-tags for
- **2026-08-18** `koax117` [💬╎general-help] most commonly i see that error when the shader names or the shader itself has been modified/broken in some fashion.
  the shader names should always remain the string of numbers for ekur identification purposes.
  beyond that, I'm not certain. 
  make sure that when you set up Ekur you didn't set it up for campaign, as the textures won't have loaded. and make sure you followed the full download/deploy instructions. (always stress that for folks)
  [attachments] image.png
- **2026-08-26** `vexerart` [💬╎general-help] hello, i've reopened this file after months and some parts of my spartan are like full black and can't change the color at all, tried to find missing texture but didn't work, any ideas?
  [attachments] image.png
- **2026-08-27** `chiefster_4014` [🔍╎texturing-and-shading] are you trying to bake assets from the Halo Infinite spartan pack in #📂╎asset-packs? if so you should try just baking them manually, as regular baking addons tend to screw up with Infinite assets due to the complex shader used. you could also try using Ekur, but I'm not sure if it supports Blender 4.5
- **2026-09-02** `sev_pa_ranger` [📜╎ripping] Hey does anyone know where the h2a elite cc textures are located at in h2a as I can't seem to find them. I'm using index to look through the files but all I can find is a few for the grunts, the ranger helmet and thats about all
- **2026-09-20** `yaboipaul` [🔍╎texturing-and-shading] Looking for some help trying to import textures from one of Exminar's awesome asset packs. 
  
  So I am trying to take the textures from his uploaded Warthog model and bring them over to Maya. I have already got the model I want in Maya as an FBX (I don't need the armature) and upon cracking open the texture folder I found that none of them are labeled with defining titles that convey their contents. So I see a lot of PNG's but they are all numbers. In Maya I need to know which ones are for normals, bump maps, specular etc. 
  
  I read online that the way Blender creates color and textures is different from Maya though, so I am not even sure if this is even possible?
- **2026-09-20** `yaboipaul` [💬╎general-help] Looking for some help trying to import textures from one of Exminar's awesome asset packs. 
  
  So I am trying to take the textures from his uploaded Warthog model and bring them over to Maya. I have already got the model I want in Maya as an FBX (I don't need the armature) and upon cracking open the texture folder I found that none of them are labeled with defining titles that convey their contents. So I see a lot of PNG's but they are all numbers. In Maya I need to know which ones are for normals, bump maps, specular etc. 
  
  I read online that the way Blender creates color and textures is different from Maya though, so I am not even sure if this is even possible?
- **2026-11-01** `finny.1663.` [🔍╎texturing-and-shading] Trying to follow some tutorials on how to bake textures so me and a friend can convert the https://discord.com/channels/819709630540021810/1371126749009547336/1440493886416883723 into a usable FBX for unity. we can get it to bake properly don't know how, the shader thats used in blender doesn't follow the same tutorials that we've watched and we're also new to this.
- **2026-11-01** `chiefster_4014` [🔍╎texturing-and-shading] it's likely from having your Bake Type set to Diffuse. that's not correct for baking textures most of the time. it should be set to Emit/Emission, and you should plug the Bake outputs you need like Color, Unity Mask map and Unity Smoothness Map into the Surface input of the Material Output node on the right. doing that will bake textures correctly.
  to bake your normal map, plug the shader back into the material Output node, and set your Bake Type to Normal.
  Also, for baking, set your Render samples to 1, and turn off Denoise. all those do is increase bake type and don't provide much help. if you want more detail in your bakes, increase the resolution to something like 2048x2048 or 4096x4096.
- **2026-11-05** `solipsol_studios` [🔍╎texturing-and-shading] Feels like a long shot to ask, since it seems all of Infinites files are random numbers -
  
  but does anyone have the actual color/text visual texture bitmap for the banished tiny triangle heat lights?
  
  Cause it has brute language symbols on the texture, but i cant find files that show that part.
  : (
  
  Heres what im talking about in game, and the only texture files i have so far ->
  [attachments] Halo_Infinite_2026-10-14_19-37-43.png, Halo_Infinite_2026-10-14_19-39-14.png, Halo_Infinite_2026-10-14_19-38-10.png, -231177950_0.webp, -127757521_0.png, -1821600604_0.png
