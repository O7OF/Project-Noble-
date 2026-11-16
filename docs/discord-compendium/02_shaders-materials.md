# shaders materials

Selected from 2624 candidate messages (ranked by length, links, Halo relevance; capped at ~260 KB). Chronological. Channels: Blender .max Importer, Halo 4 Material Importer, Halo 5 Data Importer, 💬╎general-help, 💾╎autodesk, 💾╎substance-suite, 📜╎ripping, 🔍╎texturing-and-shading, 🦴╎animation, 🦴╎rigging

- **2021-03-22** `err_0r` [📜╎ripping] Alright, basically there IS a way to find the exact color value of green and red that you'd need, but it requires a deep dive into reaches code because just as you can change the color because of the tint color box, there's a line of code in the game files somewhere that stated what hex code was fed into that tint mask for that material that got put on that transport bird. It's a bit of a pain to get those numbers so as long as you can already get color onto the model, you'd be best served trying to match the colors as best you can by hand.
- **2021-03-22** `err_0r` [📜╎ripping] Surasia, I haven't used the blender shader before, I do my shader work in unity, so if you could verify this for me: Is the reach shader block in the image that Brooen posted supposed to not be connected at all to the output? IT looks to me like the material isn't set up right
- **2021-03-22** `err_0r` [📜╎ripping] Yeah, Brooen, you connect the reach shader node right into the surface connector to the output and should be able to see changes, but you have to connect all of the maps that Surasia specied to the inputs on the reach shader node similar to how you connected the diffuse to the principled bdsf node
- **2021-03-22** `err_0r` [📜╎ripping] @swagster Chief, generally the way textures work is that textures that aren't color based, as in they are actually a bunch of black to white masks all crammed into one texture, which is normal maps, color mask maps, and often times things like smoothness and metallic, are all stored in linear. The only things still using sRGB are diffuse textures, and the color values of specular data when you're using a specular shader, which is important for us because halo games use a really odd type of specular/metallic hybrid
- **2021-03-30** `surasia` [📜╎ripping] https://github.com/Gravemind2401/Reclaimer/releases/tag/v1.5.244
  [embed] Release v1.5.244 · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v1.5.244 fix collada material URIs
  support batch extract for H5
  support WMA sounds on H2 xbox
  fix some H3 bsps not reading
  fix H1 bsps not batch extracting
- **2021-04-09** `surasia` [📜╎ripping] https://github.com/Gravemind2401/Reclaimer/releases/tag/v1.5.245
  [embed] Release v1.5.245 · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v1.5.245 support latest version of MCC
  fix h4x cubemaps
  fix h4x node maps
  add support for h2x cubemap unwrapping
  add support for model LODs in h2x
- **2021-04-16** `wavecommotion` [💬╎general-help] Ohhhh I saw that and I thought it was just an image added by error and now, I just realized I needed those shaders in the helpful resources section but which one do I need for the odst raven? I already tried with the h3 but I still can't extract the bump map it's like it just skips for some reason
- **2021-04-17** `chromacore` [🔍╎texturing-and-shading] Made my own reach shader in blender. Also set it up so I can bake it out to pbr colored spec https://media.discordapp.net/attachments/668244844769968129/833153942346137600/Capture.PNG?width=526&height=701
  [embed] https://media.discordapp.net/attachments/668244844769968129/833153942346137600/Capture.PNG?width=526&height=701
- **2021-04-19** `_gallium` [💬╎general-help] I never looked too far into reach's skin shading because it was honestly primitive, even for its time. At a glance it's similar to specular, but not quite. If you look at it it's whitest where skin oils tend to accumulate the most, so maybe it's masking between two sets of parameters? Hitting it with a frame debugger would be the easiest for me to figure out, but again. Don't really care much for reach's skin shading.
- **2021-04-19** `fiycsf` [📜╎ripping] The mindset some have, of "I've been doing it longer" is very dangerous.
  
  There's many people who have loads of area's of expertise. But that doesn't mean things are being done correctly, or that important things cannot be overlooked.
  
  There was a community that for over a year really believed that the blue channel of the control was related to the coloration of Spartans for halo 5. They were not convinced otherwise, until they had a unity shader to play with that proved it.
- **2021-04-20** `vinwarrior` [📜╎ripping] I think this genuinely boils down to semantic disagreements. Everybody seems to acknowledge that the Halo 5 assets have normal issues, right? And that they can be alleviated with modifiers and simple shader tweaks, which (obviously) can and should be shared for new people. What's the issue here? What am I missing?
- **2021-04-20** `vuthakral` [📜╎ripping] Hey, I just noticed my old Halo Online elite rips are on the Google Drive for this server, I'm just letting you guys know I fucked up the alphas included on those. Honestly, at best I'd hope to have it removed so people don't use them because of how fucked up they become if you use those alphas for specular masking (the intended thing for them)
- **2021-04-22** `err_0r` [🔍╎texturing-and-shading] It's good to see someone else who's trying to do right by the H4 ODST stuff. I reached the same conclusion you did about the striping/paintwork being the biggest issue. It doesn't *feel* odst to me, so I also brought it through substance to see if I could get something more acceptable out of it. Admittedly it's some of my older substance work so it's not quite as refined as some of my newer projects but none the less I'm relatively happy with the texturework I got out of it. 
  
  An interesting bit of the project that I've used in some of my newer stuff since is that this is what I call a hybrid texture, combining the original textures with my own. I imported the diffuse as well as grayscaled channels of the specularity/metallic/smoothness, and set it up so it looked as it did in game with shine and everything, and then used the original color code masks to set where my paintwork went. This way I got rid of the horrid stripe nonsense without having to worry about the thousand little details, keeping my layer count down and computer happy.
  [attachments] unknown.png
- **2021-04-22** `err_0r` [🔍╎texturing-and-shading] Now, as a more general question, I know that H2A stuff is a little hard to get a hold of, in particular I'm looking for weapons. I've gotten a hold of an H2A sniper rifle textures which seem to go onto the reach sniper mesh,  as well as the magnum posted here in the server. My issue is that I'm having a hell of a time trying to figure out how to make these things look right in unity. I'm in the process of trying to cobble together shaders for it using Amplify shader editor but I'm not sure what data is what when it comes to these textures.
  For both weapons I've got an RGB (diffuse/albedo) and an RGB (spec?), as well as the normal map which I've already made sure is converted to Unity specs. 
  My question is if anyone's got a good idea of how these textures end up making the end result in the game. If I use the 2nd rgb map as specular what I find is that some areas are much too dark while others are far too white, and of course there's no roughness or smoothness. I've seen plenty of halo texture sets stick smoothness/roughness in the alpha of the diffuse while unity tends to stick it in alpha of the spec or metallic map but in this case there's no alpha anywhere and none of my attempts at figuring out what data is what is giving me visually satisfactory results. Does anyone have any good ideas about what the weapon shaders from H2A look like, how they work?
- **2021-04-27** `err_0r` [🔍╎texturing-and-shading] Alright, so it looks like you're trying to use the color maps for the mcc h3 odsts? I was working on getting those bad Larrys into Unity maybe three days ago. Comparing what I understand are the mcc menu textures to those of the original game, I don't see any functional difference in the shader used, they just seem to be of a higher resolution compared to what was in game.
- **2021-05-04** `pointdevice` [📜╎ripping] the actual shader code would be what accesses it, would still be part of the mesh, getting nsight setup so i can investigate both, unfortunately, i dont think i can get the shaders as what i would need to decompile them, the ones in mccpc are dx11 compiled hlsl, and the ones on console vary, the only ones that can some what be decompiled, at least last i checked, are dx9 shaders, which only halo online would use, but i dont think that would be using any vert colors, so not much use, but i can try working some other magic on it
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
- **2021-05-15** `surdavow` [🦴╎animation] Not necessarily animation but this is what I have done for my Energy sword shader. https://twitter.com/Devarture24/status/1343666291677122560
  [embed] https://twitter.com/Devarture24/status/1343666291677122560 This took some experimentation to look right, I might share it later once I'm satisfied with the results. It needs some more adjustments and sparks/smoke before that happens. https://t.co/ZiBmVus0OM
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
- **2021-05-24** `the_heavynator` [🔍╎texturing-and-shading] ok so technically speaking yes, cook-torrance is a physically correct specular model because its a microfacet specular model however that doesn't mean reach is using "physically based rendering" as that's the name of the standard that didn't exist until Disney created it in 2013
- **2021-05-24** `the_heavynator` [🔍╎texturing-and-shading] the reason they use a "roughness" value is that's the name given to it simply because its a microfacet model. also specular in the context of reach and most games pre 2013 is its literally referring to how shiny parts of the texture are
- **2021-05-26** `williaso` [🔍╎texturing-and-shading] How much work would it be to convert a Reach texture into PBR? Like, all the information is there, right, it's just a matter of mucking with it in Photoshop or something?
  
  Or maybe I just have no idea how textures work and that's not possible
- **2021-05-28** `malikamadhu` [🔍╎texturing-and-shading] As well if you want to go further, although this ventures into modifying the geometry, having some geometry (two sets of faces) set underneath where the emissives are presently, having the emissive infusion material set on the inner most faces, and a glass shader on the 2nd set of faces and the faces you have the emissives applied to presently, so it looks like there's a solid layer of glass holding the infusion gel in.
- **2021-05-29** `pointdevice` [🔍╎texturing-and-shading] :/ its quite a bit difficult, since changing the engine to take colored spec requires editing the main shader and either adding colored spec in as a gbuffer and then using another as a flag, or just gutting metallic and making it just work in colored spec, either way requires a bit of code work and messing with the engine source
  you can try guessing where metals are based on the color spec and then having it mix that back into the diffuse to make a pseudo basecolor and metalic map if you want to keep it all in the material editor
- **2021-05-31** `soulburnin` [🔍╎texturing-and-shading] The core difference is that the one available in the drive is running off a Principled BSDF, which won’t yield accurate results. Which is fine if that’s not what you want. Chroma’s is accurately shaded, matching the various parameters in Reach, like analytical specular contribution, analytical roughness, albedo blending, etc.
- **2021-06-02** `surasia` [🔍╎texturing-and-shading] From R93_Sniper:
  halo 4 doesnt have a dedicated CC map
  it has a control map, and there's 2 types
  the one for Multiplayer spartans is as follows:
  R - Specular
  G - Diffuse
  B - Gloss
  A- Color Change
  for the Color Change, White is Primary Color, Grey is Secondary, Black is Uncolorable
- **2021-06-15** `soulburnin` [🔍╎texturing-and-shading] The UNSC weapons have a bluish tint to them, and that’s what I get from some of the materials. Still need to sort out blending though. There doesn’t seem to be a value for that like 3/Reach, so I’m wondering if it’s actually done through the control
- **2021-06-22** `pointdevice` [🔍╎texturing-and-shading] yeah, this a perfectly fine place to ask questions like that
  although im going to completely disagree with the tweet, but i can offer some advice for what i would do
  the first thing is since fortnite is Metallic/rough, its *base color* not *diffuse*, not a huge difference, but makes sense when you think that base color is the sum of specular and diffuse
  the base color really isn't the issue, having flat values there is perfectly fine *and actually proper for PBR since in microfacet models we describe probability of properties of the microfacets in the sampled pixel* 
  fortnite being made on unreal means that your shader inputs are roughness, metallic, dielelectric specular, base color and normal
  the main issue i saw was in how they did roughness and dielectric specular
  you can start by layering some noise on the roughness, and reducing the intensity of dielectric specular if you just want it to look more 'realistic'
  
  also fortnite is PBR, its jsut stylized texturing
- **2021-06-23** `fiycsf` [🔍╎texturing-and-shading] Going to assume your intended use case is VRC.
  
  to roughly repeate what I said to another asking about elites from KI2:```
  
  If you want to colour, like MP models for Spartans or Elites player models from 3/Reach/4/H2A/5, you'd need someone familiar with making something custom and compliant with colour modification via a control.
  
  This is something that some people can definitely do, but I wouldn't say it's a quick process. Would involve potentially manually painting out colour zones.
  
  (I'm capable of doing it, but the way I'd want to do it right would take long enough that well, I wouldn't do it for free personally.)
  ```
  Decimation on that scale, and then creation of a normal map would be required as well in this case, for the model to look sharp.
- **2021-06-23** `deg4397` [💬╎general-help] @swagster hey, i was wondering with your halo reach model blend file, for the emblems, how would one go about setting up those? I understand swapping out the mat in the shader but some of them appear messed up, and also not in one piece?
- **2021-06-26** `surasia` [📜╎ripping] From R93_Sniper:
  halo 4 doesnt have a dedicated CC map
  it has a control map, and there's 2 types
  the one for Multiplayer spartans is as follows:
  R - Specular
  G - Diffuse
  B - Gloss
  A- Color Change
  for the Color Change, White is Primary Color, Grey is Secondary, Black is Uncolorable
- **2021-07-06** `JunkAirplane64` [🔍╎texturing-and-shading] in which I used
  [embed] Halo 5 Shader; Includes Weapons, vehicles, armors, campaign characters and more! Shader by Devarture
  https://drive.google.com/file/d/1KI_k7D64GgJc68HcEVhyv51BYdiqs4Qe/view?usp=sharing
- **2021-07-12** `lavadeeto` [🔍╎texturing-and-shading] Reach/ h3 basically were a form of pbr, albeit some shader models cheaper then the other, but principle is the same. I think people tend to associate pbr to realistic when pbr is just a lighting model that's based on realsim.
- **2021-07-15** `pointdevice` [🔍╎texturing-and-shading] some times, i really dont like to.
  I look at the stuff around the shading a lighting in the render pipeline and i make some pretty ok guesses, which is how i did stuff like the halo 3 area specular term for lava, and finding where in vertex buffer h2a stores its vertex color, which is now a feature in reclaimer because of bugging gravemind.  There are some cases where i have fully dissassembled a shader but its a pain, and still doesnt answer some questions like how light map data is stored and used, and it is a very technical and manual process...
  
  Also nsight is the best debugger
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
- **2021-07-18** `surasia` [🔍╎texturing-and-shading] I use some weird fresnel shader that i have no idea how it works and it seems to be pretty close to halo 5. It should be at #🔗╎shaders-resources  in the halo 5 shader pack. I believe devarture shared it but doesnt know the original creator of the shader
- **2021-07-20** `emerald3050` [💬╎general-help] the whole reason im doing this is because i wanted to make something similar to this https://www.youtube.com/watch?v=e9H-dbOPNvQ&t=17s
  [embed] Porting the Halo 2 Anniversary Player Model into Unity https://www.youtube.com/watch?v=e9H-dbOPNvQ&start=17 Want to get that cool looking Spartan from Halo 2 Anniversary into Unity? This video explains just how!
  
  Music: 
  Tell Me Quietly by Nick Macdisi
  https://www.youtube.com/watch?v=tQiBR231nbQ
  
  CC
- **2021-07-22** `fiycsf` [🔍╎texturing-and-shading] If you're looking at halo 5 controls as a whole, you're not getting a proper understanding of them.
  
  The red and green channels of the image are colour zones, blue channel is metallic, and alpha is smoothness.
  
  So when you look at it as a whole, well, you're not really getting a great representation of what is actually occuring.
- **2021-07-24** `acearkadios` [💬╎general-help] I'm Having a problem with the Halo Reach Shader (The one posted in the Shaders Channel). It works perfectly in eevee, but in cycles I don't know what's up. I haven't had a problem like quite like this before, so I don't know where to start for diagnosing the problem. I tried playing around with it a little but to no avail.
  [attachments] unknown.png
- **2021-08-22** `crashlegacy14` [🔍╎texturing-and-shading] Alright, so this is a general texturing question I have.
  
  If I'm working with a large texture covering multiple types of materials, and I want to use detail textures to, well, detail things. What's more performance efficient, one material using multiple masks to layer on detail textures (like the color control masks are used for primary secondary colors), or partitioning the different materials into different ... material slots? Uhg, I don't know a better way of describing that.  Is there even a noticeable difference in performance?
- **2021-08-22** `chromacore` [🔍╎texturing-and-shading] https://cdn.discordapp.com/attachments/668244844769968129/878855177170472980/Palmer.png Got my non MP armor shader working. Still working on the visors tho.
  [embed] https://cdn.discordapp.com/attachments/668244844769968129/878855177170472980/Palmer.png
- **2021-08-24** `R93_Sniper` [🔍╎texturing-and-shading] [embed] A .blend file containing some Halo shaders I've made over time. The file will be updated as I fix/add things, so check back every now and then. There's one shader for objects, vehicles, etc and a work in progress one for terrain. 
  Shaders available for Halo 3, ODST, Reach and 4. 
  https://drive.googl
- **2021-08-29** `the_heavynator` [🔍╎texturing-and-shading] reach roughness maps are used by setting analytical_roughness to 1,  material_texture_black_roughness to 0.01 (non 0 to prevent nan artifacts), enabling the use_material_texture toggle (usually constraint 0) under float constraints and finally putting a texture with the roughness in the alpha into the material_texture input.
- **2021-09-09** `plastered_crab` [🔍╎texturing-and-shading] I usually use a different Halo 3 shader, but for that one I assume you plug that first orange block's Color into Diffuse, get the detail diffuse texture and plug its Color into Detail Diffuse, and Zbump Color into Diffuse bump
- **2021-09-23** `file1431` [🔍╎texturing-and-shading] The color of what will be the armor (you can say the light orange color) is composed of 3 variations of an orange, wear, dirt are made with smart materials and after that, we proceeded to make modifications so that the same are not geometrically perfect, then by masks add bases in what would be the channels that are not the color or base, the viewer is simply using the textures of Halo 5 just add one that I did where certain parts limit the relief and another reflection.
- **2021-10-05** `arithen` [🔍╎texturing-and-shading] Maybe I'm just understanding how they function. Yes, they are in there (as I just discovered), but the one I'm working on right now in blender is titled "Spartan_Visor_Kurt" and I don't see that specific material. 
  
  Nevertheless, the one you just linked is working, despite the name change.
  
  Would I need to use a color ramp node to decide the visor color then?
- **2021-10-10** `_gallium` [🔍╎texturing-and-shading] Start by using a more compatible specular setup like this https://discord.com/channels/819709630540021810/860683808105168916/891231633003380746
  
  base colour goes into diffuse colour, specular colour goes into colour on both glossy nodes, normal goes into everything, roughness into that port on both glossy nodes too.
- **2021-10-20** `itzhouze` [💬╎general-help] Hey there. I need some help. I exported the halo 3 multiplayer map: "warehouse" structure_bsp file to an AMF file using Reclaimer. Then i modified the geometry in blender. Now i exported the modified file as an ASS file. Whats the correct steps to get an structure_bsp file? i tried using tool=>structure, but after 5 minutes it just throws many errors, missing shaders and triangle errors and wont build. Hope you can help me out 🙂
- **2021-10-24** `sierra060822` [🦴╎animation] yeeee all ripped and reused stuff from halo reach and ported to blender! i may do a little shader fuckery for the armor before i finish the final render but im actually planning on rendering it tonight while i sleep, now that i've finally bought more storage for projects cause i had like none and had to stop making stuff  😅
- **2021-10-26** `sswampp` [💬╎general-help] https://github.com/num0005/h3-shader-compiler-fix
  [embed] GitHub - num0005/h3-shader-compiler-fix: A small DLL that fixes too... https://github.com/num0005/h3-shader-compiler-fix A small DLL that fixes tool's usage of the Halo 3 shader compiler. - GitHub - num0005/h3-shader-compiler-fix: A small DLL that fixes tool's usage of the Halo 3 shader compiler.
- **2021-11-03** `itzhouze` [🔍╎texturing-and-shading] hey there. is here a blender pro? i need some help getting my blender materials exported as a texture (to generate a shader for halo3)
  i have many materials in blender and need to get them exported. (every .dat has its own material)
- **2021-11-07** `itzhouze` [💬╎general-help] Hey there! i have a long list of materials in blender and want to create a h3 shader for every material. Is there a good automated way to create a shader for every material? The only difference in the .shader will be the ALBEDO base_map filepath.
  
  I tried opening the .shader file and manipulate the filepath it was looking for. But after modifying it, the shader file wont open again in guerilla (i think its corrupted).
- **2021-11-07** `surasia` [💬╎general-help] https://github.com/arkryl/Blader
  [embed] GitHub - arkryl/Blader: Generate (as of now, empty) Halo .shader fi... https://github.com/arkryl/Blader Generate (as of now, empty) Halo .shader files from FBX files - GitHub - arkryl/Blader: Generate (as of now, empty) Halo .shader files from FBX files
- **2021-11-08** `fiycsf` [💬╎general-help] You'll want to pack some of the materials together. 
  
  Vrchat being unity, is affected by draw call count. Think of materials + meshes as a rough number.
  
  A reach Spartan can easily be packed down to 3 materials, or a few more if you don't want to do everything. I'd highly recommend doing so in order to make the model more performabt than just poor due to material count.
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
- **2021-11-28** `piratepenguin.` [💬╎general-help] https://c20.reclaimers.net/h1/tags/shader/ here's two hope this helps fam ❤️
  [embed] shader (abstract tag) https://c20.reclaimers.net/h1/tags/shader/ Shader tags define the visual appearance of models in the game world. The shader tag class itself is...
- **2021-11-29** `unordinal` [💬╎general-help] Hey, I'm trying to get the colors for Halo Reach grunts and in the biped tag there's four "Change Colors" datablocks. Inside those blocks are the actual variant color permutations. I know that the four datablocks correspond to albedo tint, specular tint, fresnel tint and environment tint but I forget the order in which they're listed. Does anyone happen to know?
- **2021-11-30** `unordinal` [💬╎general-help] Okay, I'm a little bit more confused than I was yesterday when it comes to the 'Change Colors' .biped color variant permutations for Halo Reach. Each slot seems to correspond to a color channel on the CC Map (makes sense) but they also sometimes seem to affect the diffuse, specular, fresnel and environment tints as well and I'm not sure if there's a flag I'm missing or what
  
  For example, I'm currently looking into the Skirmisher - the major has red feathers which are controlled via blue on the CC map, so it's in the third slot of the color change datablock (4 slots total, numbered 0-3 and seem to correspond to primary, secondary, tertiary and quaternary colors respectively)
  
  Problem is, there's colors set for the secondary and quaternary colors even though there's no respective colors on the CC map for them, and given that the specular tint is blacked out in the base shader and the elite armor uses those slots to control specular and fresnel tint, I'm not entirely sure how the game determines whether to use a color for a primary/secondary/etc color or a diffuse/specular/etc tint.
- **2021-11-30** `unordinal` [💬╎general-help] Right, I forgot about albedo blend - I'm a bit confused still when it comes to that though, as elite armor specifically has a tertiary color that is not in the color map at all and yet applies to the fresnel tint.
  
  Actually, now that I type that out, does albedo blend set to one change it to ignore the CC map?
- **2021-11-30** `soulburnin` [💬╎general-help] Nah, the Skrimisher/Elite armor shader breaks down like this:
  
  Diffuse blended with detail map ---> Colored based on CC map ---> Mixed with specular tint (0 means only specular tint, 1 is full albedo tint) = final specular color
  [attachments] blender_D9WTwF6eQF.png
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
- **2021-12-02** `bios_0307` [📜╎ripping] This is the Destiny 2 NPC shader for Blender, you can cannibalize the Z reconstruct from it: https://drive.google.com/file/d/1JhvfpJNIpZgXqUm1jSnMhlfZm9LymE7F/view?usp=sharing
  [embed] D2 NPC_Enemy Shader 1.4.5.blend https://drive.google.com/file/d/1JhvfpJNIpZgXqUm1jSnMhlfZm9LymE7F/view?usp=sharing
- **2021-12-02** `bios_0307` [📜╎ripping] Oh btw, in some engines there’s an option to mark a texture as a normal map (Unity has it, for one). Typically that option tells the engine to repack the texture and drop one of the channels, and it later gets the third channel reconstructed. When you have a different texture packed into the normal map blue, it’ll end up dropping that texture completely. Something to keep in mind if anyone ends up making a Unity shader for this stuff.
- **2021-12-03** `xsoulstacex` [💬╎general-help] Hey I'm trying to export my spartan as an fbx but every time I do he loses all of his textures! I think it has something to do with replacing the principled BSDF with the bungie shader on most objects. Some like the visor don't use the bungie halo shader though and still come out gray. Followed all the fbx export tutorials I could youtube up and still my textures dont stick.
  
  Help?
- **2021-12-03** `xsoulstacex` [💬╎general-help] Is it not possible to get my Blender shaders into Unity? Default diffuse maps for the armor don't give it any color I had to do a lot of work for that in Blender using the Bungie Halo Shader. I don't know how I'm going  to get my Spartan colors or emissions in unity :what:
- **2021-12-03** `xsoulstacex` [🔍╎texturing-and-shading] Is it possible to take my Blender shaders into Unity? New to texturing and did all my good texture work for a multiplayer spartan using nodes in blender. Used the Bungie Halo Shader in place of Principled BSDF so I could set the appropriate colors onto the armor. Because the diffuse maps came without color, I added it in blender. I just need to know how to get him looking the same in unity as he does in blender
- **2021-12-03** `tpg` [🔍╎texturing-and-shading] nope but you can use the shader made by @R93_Sniper and Point Device https://discord.com/channels/819709630540021810/825398188743458826/911430857384935424 unless you feel like making your own using https://assetstore.unity.com/packages/tools/visual-scripting/amplify-shader-editor-68570
  [embed] Amplify Shader Editor | Visual Scripting | Unity Asset Store https://assetstore.unity.com/packages/tools/visual-scripting/amplify-shader-editor-68570 Get the Amplify Shader Editor package from Amplify Creations and speed up your game development process. Find this & other Visual Scripting options on
- **2021-12-12** `fiycsf` [💬╎general-help] I honestly wouldn't recommend trying it either.
  
  Reach models have heavy metal damage in the model's normal map and specular data, unlike 4/5/infinite.
  
  This means that if you substance paint them, it'd be like taking a spray can to the top of heavily damaged armor.
  
  You can't just delete that damage and wear, so it'd be very hard to make good maps for it that don't have that effect.
  
  It's why I'd honestly recommend using 5's reach Esq parts for substance painting over using actual reach.
- **2021-12-15** `unordinal` [🔍╎texturing-and-shading] In your spartan file, go to File > Append. Navigate to the downloaded Halo5Shaders.blend file. When you find it, double-click it and navigate to NodeTree and then the Halo 5 shader. Select it and click append at the bottom right.
  
  In your material for the spartan, press Shift + A to add a new node. Go to 'Group' and select the Halo 5 shader group you appended. Connect the inputs and output appropriately.
- **2021-12-16** `surasia` [💬╎general-help] https://gitlab.com/FiyCsf/open-halo-5-spartan-shader-for-unity
  [embed] Fiy Csf / Open Halo 5 Spartan Shader for Unity https://gitlab.com/FiyCsf/open-halo-5-spartan-shader-for-unity Created By PointDevice And FiyCsf For Use With Halo 5 Spartan Models in Unity. This should play nicely with all Halo 5 MP Spartan Textures etc,...
- **2021-12-16** `fiycsf` [💬╎general-help] While I'm sure people will yell "but game accurate!";
  
  You can achieve a decent results with Reach maps by just plugging the specular in the alpha into the specular of Principaled bdsf, and the same for the metallic and smoothness (once you invert it to rough) for Halo 5.
  
  You cannot do that at all with infinite's workflow.
- **2021-12-17** `thejudsub` [🔍╎texturing-and-shading] Yep. Thats exactly the thing im referencing with the blender parallax build.
  https://devtalk.blender.org/t/parallax-occlusion-mapping/15774
  [embed] Parallax Occlusion Mapping https://devtalk.blender.org/t/parallax-occlusion-mapping/15774 There has been interest in supporting parallax occlusion mapping (POM) in Blender for some time. See for example https://developer.blender.org/T68477, https://blender.community/c/rightclickselect/t9bbbc/ and Pa
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
- **2021-12-28** `unordinal` [🔍╎texturing-and-shading] If it's anything like gradient masks in other games, the red channel is probably used to pick a color from a specific gradient map. Unfortunately I believe there's no actual textures for it - similar to there being no albedo textures - so I'm not sure there's a way to completely and accurately recreate it atm. I believe you'd use the texture properly by plugging the gradientmask red channel into the 'fac' of a Color Ramp that's set up with some colors. 
  
  It's probably a similar case with the blue roughness channel in the mask as well (though obviously with that one it'd be limited to the single channel value).
- **2022-01-01** `coarforge` [🔍╎texturing-and-shading] https://github.com/Coreforge/blender-halo-infinite/tree/dev Make sure to download the dev branch (master is a bit out of date). It works with bacons shader (and should work with others too), but you need to rename the image texture nodes for it to work, as they are just called "Image Texture" by default.
  [embed] GitHub - Coreforge/blender-halo-infinite at dev https://github.com/Coreforge/blender-halo-infinite/tree/dev Contribute to Coreforge/blender-halo-infinite development by creating an account on GitHub.
- **2022-01-01** `coarforge` [🔍╎texturing-and-shading] I can quickly, but I'm off to bed after that. You go onto the github page and select the dev branch (the link I sent already has that branch selected). You then download the code as a .zip file, which you can then install as an addon in the user preferences in blender. To be able to import textures, you need to install the module texture2ddecoder into blenders python environment by running a few commands in the python console in blender (the commands are in the readme.md). You can then go to import -> Halo infinite rendermodel and import a rendermodel. If you want to import more then just the 3D model (so textures), you have to set the root path property to the root path of where you extracted the modules to (the filechooser doesn't work, so you have to copy in the path). Same for the shader .blend if you want it to automatically apply that shader.
- **2022-01-12** `sunksi` [🔍╎texturing-and-shading] Yeah you can use decals, the default way just kinda sucks ass
  I use Decal machine 
  But if you dont wanna shell out 50 bucks (I dont blame you) ive heard this works
  https://amanbairwal.gumroad.com/l/ImportAsDecal
  [embed] Import As Decal - Free Blender Addon https://amanbairwal.gumroad.com/l/ImportAsDecal My All-In-One RanTools (and P-Cutter) Addon is at flat 20% OFF using the code 'newyear' . You can check it out here:https://amanbairwal.gumroad.com/l/rantoolsv2.1 Out Now!Import As Decal is a Free Addon for Blender 
- **2022-01-16** `surasia` [🔍╎texturing-and-shading] https://github.com/R93Sniper/HaloInfiniteVRCShader
  [embed] GitHub - R93Sniper/HaloInfiniteVRCShader: An open source unity shad... https://github.com/R93Sniper/HaloInfiniteVRCShader An open source unity shader designed to emulate the look of Halo Infinite&#39;s shader system - GitHub - R93Sniper/HaloInfiniteVRCShader: An open source unity shader designed to 
- **2022-01-19** `chromacore` [🔍╎texturing-and-shading] That shader was slammed together so it might not have all the necessary inputs for accuracy. Reach has a shit ton of properties that change the look. Hell there's an entire enviro spec set of properties that's still kinda fucky to do.
- **2022-01-19** `sunksi` [💬╎general-help] Can't walk you through it cause I'm about to sleep but download this and look up tutorials for it
  https://amanbairwal.gumroad.com/l/ImportAsDecal
  [embed] Import As Decal - Free Blender Addon https://amanbairwal.gumroad.com/l/ImportAsDecal My All-In-One RanTools (and P-Cutter) Addon is at flat 20% OFF using the code 'newyear' . You can check it out here:https://amanbairwal.gumroad.com/l/rantoolsv2.1 Out Now!Import As Decal is a Free Addon for Blender 
- **2022-01-21** `theeldestboy` [🔍╎texturing-and-shading] https://www.artstation.com/artwork/xY31YY
  [embed] Halo Infinite - Wetness, Evan Liaw https://www.artstation.com/artwork/xY31YY Water and wetness was a huge team effort especially with a large Campaign and Multiplayer maps.  Huge thanks the Squirtle Squard.  Steve Carroll, Mahreen Fatima, and Mitchel Clayton from Campaign.  Stuart Cunningham for all
- **2022-01-27** `ubald007` [🔍╎texturing-and-shading] This is for everyone, Work-flow tip. As we mostly deal with extractions and we all know they are not always perfect!, the textures some times get messed up. these images are to help you see the errors that sometimes happen with extraction's. A. is to show you that sometimes the alignment can be off sometimes very little. Also being the Green channel in this case highlighting the edge on the ASG_Control map is a great guide, makes it easy to spot and fairly easy to correct as shown in image B. So in your work-flow before building your shader use the Control map to help you fix this misalignment. I hope this helps all of you!! CHEERs..
  [attachments] A.jpg, B.jpg
- **2022-02-02** `elitecaden` [🦴╎rigging] It’s the same elite shaders from Chiefs halo reach elites https://drive.google.com/file/d/1lEKWe4WHANJD4DmsHLjIsyq9_XPRi6tC/view?usp=sharing
  [embed] Reach Elites.zip https://drive.google.com/file/d/1lEKWe4WHANJD4DmsHLjIsyq9_XPRi6tC/view?usp=sharing
- **2022-02-25** `coarforge` [💬╎general-help] If you meant it in the graphics programming sense, you usually at least have a vertex shader and a fragment shader. The vertex shader runs for each vertex in the drawing pass and generally does stuff like applying the transformation and projection matrices (and probably some other stuff). The fragment shader again runs for each fragment and outputs a color. Depending on the depth, it either gets ignored, or put into the buffer that is currently being drawn to.
- **2022-02-26** `bumbusatepadang` [🦴╎rigging] i am just wondering can you change his arms and legs into IK heres the rig
  Halo Infinite Spartans organized, shaded and rigged. [Includes MKVII Core, MKV (B) Core, Yoroi Core, MKVI GEN III Core, Spartan Agryna Core] with the ability to mix them.
  
  - Render By @mrgruntington -
  
  - Shader by @Grand Bacon -
  
  - Rig by @SkyBox -
  
  Download; https://mega.nz/file/piQljIjL#rSEzZ47GYmJi1vNiA9PE2SvTnO5fzdYIryD8BIMJUJ0 #archive-infinite-packs plz i need help
  [embed] 434.5 MB file on MEGA https://mega.nz/file/piQljIjL
- **2022-03-13** `thejudsub` [📜╎ripping] can anyone point me to a tutorial for ripping things from H2A? And possibly how to rip maps from H3? If not halo3, then infinite is fine as long as i can get forerunner environment assets.
  I wanted to check out H2A ripping because i got the prophet model from the drive, but the UVs and weights were pretty messed up so i wanted to fix it up myself. And i need halo 3 stuff because i need the map where the prophet of truth tries to fire the rings in the ark.
  But im open to building the scene myself with infinite assets. I can figure out the shaders myself
- **2022-03-14** `urium86` [🔍╎texturing-and-shading] https://www.artstation.com/artwork/4X1Ren
  [embed] Halo Infinite : Character and Vehicle Materials, Rogelio Olguin https://www.artstation.com/artwork/4X1Ren I did more than what is here for the character a vehicle materials but these are the ones I liked.  Some were from source material edited.  The Surfacing team was dedicated to create materials s
- **2022-03-19** `lankyb` [💬╎general-help] Hey, was wondering if someone could help me with a few things.
  
  I have a Blender version of my Halo Reach Spartan, however I'm looking at exporting it to Unity so I can do some VR things with it.
  
  I'm aware that the shaders from Blender won't transfer over to Unity, so could someone help me with the model on the Unity side with the shaders? Like I said, the model looks great on Blender, just don't want an insane quality drop when moving it to Unity
- **2022-03-21** `williaso` [🔍╎texturing-and-shading] https://blender.stackexchange.com/questions/214925/how-to-use-a-red-green-color-map-in-blender *Maybe* this'll help? @AverageUsername
  [embed] How to use a Red/Green color map in Blender? https://blender.stackexchange.com/questions/214925/how-to-use-a-red-green-color-map-in-blender (I have modified my initial question after finding out more about the normal maps.)
  I am playing around with some ripped models and I am trying to figure out ho
- **2022-03-24** `fiycsf` [💬╎general-help] @Jockey your easiest method is to take what's here;
  
  https://discord.com/channels/819709630540021810/821676515099279374/920177041222008832
  
  (Not the mark 7 that's already on the rig, replace that and you will end up re-rigging just the parts that are not in the rigged for VRC, in the blend from Surasia)
  
  Put your modified on top of the one I've done, weight the new parts solid to relevant bone, renmame the UVs so that you can join the meshes, and then follow the readme to tpose, and use the shader by R93 and Point Device.
- **2022-03-25** `chunch` [💬╎general-help] https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes
  [embed] Release Material Types · AverageTrapEnthusiast/Halo-Infinite-Shader... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes Material Types for Halo Infinite Shaders
- **2022-03-30** `surasia` [💬╎general-help] https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes1.1
  [embed] Release Material Types 1.1 · AverageTrapEnthusiast/Halo-Infinite-Sh... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes1.1 Fixed an issue with how the gradient masks are color graded.
- **2022-04-03** `opr_23b` [🔍╎texturing-and-shading] Hey, just wanted to come by and ask for some help with the Jackal model I'm working with.
  
  So, I want to get basic materials (basically giving the model it's textures without a Reach-style shader, since I intend on doing that later) on the model, but I'm unsure as to exactly what texture these completely black parts might have.
  The material for them is named "additive_gradient", and looking at some reference pics, it seems to be a sort of "glow" effect for the glowing parts of the Jackal, though I can't seem to find these textures for this glow effect.
  
  Any idea as to where they might be?
  [attachments] unknown.png
- **2022-04-09** `the_heavynator` [🔍╎texturing-and-shading] h2asp is basically specular workflow shader but the artists were still used to a non pbr workflow so its a little messy but there are 3 main textures, Diffuse, specular and normal which is fairly standard. the specular has the smoothness (inverse of roughness) value in the alpha channel. again just keep in mind the game was made during a transitionary phase to pbr workflows so some assets will look better than others.
- **2022-04-10** `user_t0o108i6` [💬╎general-help] no worries - which one of these shaders should i be using? not sure which https://github.com/Theathkill9512/Halo-Infinite-Modular-Shader/releases
  [embed] Releases · AverageTrapEnthusiast/Halo-Infinite-Shader-Resources https://github.com/Theathkill9512/Halo-Infinite-Modular-Shader/releases A Collection of various Halo Infinite Shaders I have worked on (and am working on)  - AverageTrapEnthusiast/Halo-Infinite-Shader-Resources
- **2022-04-11** `silent1meow` [🔍╎texturing-and-shading] modular shader is pretty cool as you basically have freedom to control almost everything individually (i forget if your already able to do that in bacons) by being able to change the color, edge wear, metallic and dirt of each zone but on top of that also change the material so the outer base of the model is zone 1, you plug in a smooth metal material and it'll give exactly what it says smooth metal. but for the grip you plug in something like a leather material and it well it looks like leather and also able to change the color of dirt, metallic and roughness which is cool too
  [attachments] unknown.png, unknown.png, unknown.png, unknown.png
- **2022-04-22** `wraith0912` [🔍╎texturing-and-shading] kind of a stupid question; how would I go about setting the visors in the infinite spartans pack to be emissive?
  I saw the earlier advice about duplicating the shader and then plugging color 1 into the emissive port but I've got no idea what that means or how to do it
- **2022-05-04** `garfunkle3479` [📜╎ripping] https://github.com/Theathkill9512/Halo-Infinite-Modular-Shader/releases
  [embed] Releases · AverageTrapEnthusiast/Halo-Infinite-Shader-Resources https://github.com/Theathkill9512/Halo-Infinite-Modular-Shader/releases A Collection of various Halo Infinite Shaders I have worked on (and am working on)  - AverageTrapEnthusiast/Halo-Infinite-Shader-Resources
- **2022-05-04** `fiycsf` [💬╎general-help] If your asking about the infinite pelican, for Vrchat, my personal opinion is likely TLDR'd as "don't". 
  
  (It's not a good thing to cut one's teeth on, start simpler, unless you're a bit of a masochist)
  
  The infinite pelican is like, 30MB of mesh alone last I checked, and would need to be broken up well as to not render all at once,  (Occlusion culling) or else the framerate would be pretty hard hit. This is for mesh alone.
  
  For materials, the pelican as Kayhotic says, also uses a lot of shader work for it's materials, so if you don't have something like amplify shader editor, or shader coding knowledge, it'll be hard pressed to look even close to what you see in infinite/blender.
  
  It also makes use of some clever height tricks for fake depth in panel lines and bolts and dips in the hull, that aren't easily replicated in unity.
- **2022-05-05** `chunch` [🔍╎texturing-and-shading] https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/ModularShader2.3
  [embed] Release Halo Infinite Modular Shader (2.3) · AverageTrapEnthusiast/... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/ModularShader2.3 A fully revamped version of the modular shader!
  Now includes node groups that can be used to convert older halo games models an
- **2022-05-05** `plastered_crab` [💬╎general-help] That would be sweet! If you have a hard time with the textures being setup (as they are quite different than other games) check out the shader by @Chunch or wait for the huge Spartan pack coming out soon that @Surasia it's working on
- **2022-05-05** `chunch` [🔍╎texturing-and-shading] https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes1.1
  [embed] Release Material Types 1.1 · AverageTrapEnthusiast/Halo-Infinite-Sh... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes1.1 Fixed an issue with how the gradient masks are color graded.
- **2022-05-07** `gameoverload` [💬╎general-help] It won't since it is a object put into a blend file, anyway, you'd need to go into the shading tab, then go and find someone that made a shader for whatever that is, if it is Halo, then you should be able to find one that works with it.
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
- **2022-06-02** `r93_sniper` [📜╎ripping] For a Comprehensive Halo 5 pack, i'd like it set up as such:
  1. Have all Armors set up with Names, Materials, Rigging, and Skin Weights
  2. Have it set up in 3DS Max with Layers
  3. Have it set up in Blender with Collections
  4. Have a Full FBX with everything exported in a monolithic stack
  5. Have Each individual Armor Set exported as a Seperate FBX
  6. Have all the textures for each individual armor set named correctly and located in their own relevant folder
  7. [Optional] Have a comprehensive blender shader set up
- **2022-06-03** `fiycsf` [📜╎ripping] If you want to get most helmets to have the visor on the same material, you'll have to use the fact that the visor is white in the one colour channel and another channel to do some math to try and figure out an assumed visor location where the control image is more yellow.
  
  Open halo 5 does it ATM
- **2022-06-03** `tenuyu` [🔍╎texturing-and-shading] Does anyone here use poiyomi to do their spartan shaders? I wanted to optimize it all down to one material so it wouldn't be so resource heavy and I failed to consider atlasing down the model would make the HaloInfinite shader not work properly on it. Would love to get help if anyone is experienced with it. (Been tinkering for a few days on and off and just cannot get a good look)
- **2022-06-05** `silent1meow` [💬╎general-help] no worries
  here's all of the shaders
  https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases
  [embed] Releases · AverageTrapEnthusiast/Halo-Infinite-Shader-Resources https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases A Collection of various Halo Infinite Shaders I have worked on (and am working on)  - AverageTrapEnthusiast/Halo-Infinite-Shader-Resources
- **2022-06-09** `gameoverload` [💬╎general-help] For the pack and or Goblin's shader, the pose has an action pose and rest pose, to put it back into the base pose, switch to the rest pose, and when you want to actually pose, use the action pose.
  Also the Spartan armor is just saved by saving your Blender project, like any other object in Blender, and color saves the same way since it just from changing the shader of the armor piece your customizing.
- **2022-06-09** `koax117` [🔍╎texturing-and-shading] Going to echo what Vorch set. at least in the screenshot you provided, you're only affecting the color control alpha. 
  You've got a Diffuse Alpha, Detail Map 1 Alpha, Detail Map 2 Alpha, and the regular Alpha itself. 
  Personally for me, when working with another person's shader/project file, it's a lot of trial and error to get the result I want, because I can't read the person's mind on how they set it up. 
  My immediate thought would be to combine/mix the alpha's of both emblems to go directly into the alpha node itself to mask out the black that you are wanting to remove
- **2022-06-13** `gameoverload` [💬╎general-help] The coatings don't work like that, what they are, are those hex color values and materials that are in different zones on the Spartan in game, that are procedural handled by the game itself through a Shader similar to the one Goblin made.
- **2022-06-15** `gameoverload` [💬╎general-help] Through the Blender Spartan 2.0 Pack, via it's Bake outputs on the right of the Shader node where the BSDF connects to the material output. Although you will want to have an image texture node somewhere on each armor you wish to bake out textures for that has a new 1k, 2k, or 4k image ready for the baking of textures.
- **2022-06-15** `deityeternal` [🦴╎rigging] Need some help re-rigging or just fixing my broken ODST rig, I figure I can try to use Vat's baseline marine rig and turn it into an IK but if someone can hold my hand on this issue I'd appreciate it. I've never bump into an issue like this but the IK constraints refuse to work, I can't release the pack broken so I'm turning to the community form help being that it's a very niche issue and there are experienced riggers here.
  [attachments] Needs_Re-Rigging.png, IK_work.png
- **2022-06-17** `frostbitecinematics` [📜╎ripping] It's kind of like how you set up the clone markings. Basically each channel of the _W map contains a black and white mask, one for each weather effect. E.g red might be sand, green could be snow, blue could be wet mud when you walk into rivers, etc. I don't use Blender much so I'm going off of what I know from doing it in Unreal Engine, but you essentially use the mask to LERP between the base texture and the weather colour and normal map texture. In Blender, I believe the MixRGB node is the same as UE's LERP. So the mask goes into the LERP Alpha (MixRGB's Value?), the A input of the MixRGB is your base colour and the B of the Mix RGB is a mud texture. Then set the same thing up to blend between the model's base normal map and the weather effect normal map, e.g the mud normals.
- **2022-06-18** `koax117` [🔍╎texturing-and-shading] easiest thing to do would be take these into something like photoshop with the color picker
  https://www.halopedia.org/Armor_coatings_%28Halo_Infinite%29
  [embed] Armor_coatings_%28Halo_Infinite%29 https://www.halopedia.org/Armor_coatings_%28Halo_Infinite%29 Armor coatings, also known as armor skins, are an element of player customization in Halo Infinite. They are a seven layer system that change the materials and coloration of a player's armor permutation.
- **2022-06-20** `incuriouslog` [💬╎general-help] I'm using the Surasia spartan pack, and was wondering if there's a way to make the world shader apply only to reflective surfaces? I don't like it applying lighting to the whole scene, I'm using my own lights for that, but because of it, all my spartan visors look black because there's nothing in the scene to reflect.
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
- **2022-07-12** `plastered_crab` [💬╎general-help] This goes over the different parts that you actually need @VinWarrior 
  https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-07-14** `plastered_crab` [💬╎general-help] https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-07-15** `zoptic` [🔍╎texturing-and-shading] @Chunch I have a suggestion for your next version of the shader. Clamp the roughness and metallic maps. The reason why is when tweaking the roughness value through the material swatches any very negative or positive values actually end up cancelling out the roughness from the grime/dust/other layers on top. It can produce some weird results. It should at least be a preventative thing if anything does make a tweak that goes into weird value ranges.
  [attachments] unknown.png
- **2022-07-18** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-07-29** `tpg` [💬╎general-help] over at https://discord.gg/TkycpmwD2R and while they are made for VRChat mainly (also lack the newer techsuits) they should work fine for non-VRChat projects
  
  also Amplify based shader already made and ready to go: https://github.com/R93Sniper/HaloInfiniteVRCShader
  [embed] GitHub - R93Sniper/HaloInfiniteVRCShader: An open source unity shad... https://github.com/R93Sniper/HaloInfiniteVRCShader An open source unity shader designed to emulate the look of Halo Infinite&#39;s shader system - GitHub - R93Sniper/HaloInfiniteVRCShader: An open source unity shader designed to 
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
- **2022-08-04** `chunch` [🔍╎texturing-and-shading] https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes2.1
  [embed] Release Material Types 2.1 · AverageTrapEnthusiast/Halo-Infinite-Sh... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes2.1 Redid the way the roughness is calculated (again) for near 1:1 game accuracy.
- **2022-08-05** `soundflare` [💬╎general-help] Any current Reach shaders at the moment? Am just trying to properly learn blender as best as i can, only just need to understand the different shaders for this is all, along with potentially how to make the Corvette bridges glass see through for a proper visible space backdrop when able. Cause I'm basically wanting to recreate this from the campaign honestly, either with the Halo Reach backdrop or a black void of space backdrop potentially with what i'm currently doing in Blender. Any advice is appreciated ^^
  [attachments] 8-5-2022_4-49-46_PM-2bx1khlc.png
- **2022-08-07** `fable_law_458` [🔍╎texturing-and-shading] @Banned .Casey I'm pretty sure it's in the shaders folder when your in the main mod tools folder in any of the halo's like in the image provided.
  
  Once your in the shaders folder, go to the `fx.bin` file and open it with any text editor you desire and you should see shader code there from what I know. If not, look at the other files in the shaders folder.
  [attachments] Capture.JPG
- **2022-08-07** `fable_law_458` [🔍╎texturing-and-shading] I'm pretty sure in guerilla you can look at shader code. Maybe if you open a .shader tag in halo 2 or 3 there maybe code there. If not, I would look elsewhere in the mod tools. I could have sworn there was shader code in the mod tools somewhere.
- **2022-08-08** `hamporker` [💬╎general-help] Hey, I'm porting Temple from Perfect Dark and Golden Eye into Halo 3. Does anyone have any knowledge of lighting in H3EK? I got bitmaps and shaders working, but I want shadows in the indoor areas like  in the original N64 map.
  [attachments] Capture1.JPG, Capture2.JPG
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
- **2022-08-17** `plastered_crab` [💬╎general-help] https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-08-17** `plastered_crab` [💬╎general-help] https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-08-20** `Deleted User` [📜╎ripping] https://youtu.be/r_5suSLUucU
  [embed] How to Port Coatings to Blender from Halo Infinite [100% Accurate] https://www.youtube.com/watch?v=r_5suSLUucU Swatches: https://mega.nz/folder/yWoCxTrZ#0emVAwxOqA85fg9fePy3tA/folder/7XYBVCxT
  
  make sure all of these are in a /materials folder.
  
  CoatingTool: https://mega.nz/file/oqwx0LRJ#cL4vRS7QPj08
- **2022-08-22** `brianthefirelord` [💬╎general-help] The tutorial in question:
  https://youtu.be/r_5suSLUucU
  [embed] How to Port Coatings to Blender from Halo Infinite [100% Accurate] https://www.youtube.com/watch?v=r_5suSLUucU Swatches: https://mega.nz/folder/yWoCxTrZ#0emVAwxOqA85fg9fePy3tA/folder/7XYBVCxT
  
  make sure all of these are in a /materials folder.
  
  CoatingTool: https://mega.nz/file/oqwx0LRJ#cL4vRS7QPj08
- **2022-08-23** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-08-25** `gameoverload` [🔍╎texturing-and-shading] Well Diffuse is normally a combination of Albedo or color, and either a baked in map on top of that, or something in the alpha of it that pertains to light or shadow in some form, or something entirely different like color masking.
  A normal texture requires the use of a normal map node, with the normal texture connected to it and then the normal map connected to a normal input.
  The AO would require a MixRGB which multiplies the Diffuse by the AO.
  Metallic into the metallic input.
  Smoothness into an inverse node then into roughness, since smoothness is the inverse of roughness.
  Then the tint index sounds like a tinting mask of some kind.
  Also you should not be using a Halo 5 armor shader for this, it is not built for how Gears Tactics works, it is built for how Halo 5 works.
  You would need to build your own shader for it.
- **2022-08-25** `gameoverload` [🔍╎texturing-and-shading] Their are very rare cases another shading system will magically work with what your dealing with, and this time is not one of them.
  For example, Halo shaders tend to be similar in nature, not the same, but similar, as a number of Halo games use fake metalness to get the effect of a metal material.
  Anyway, I can help you create a shader for Tactics when I am home, as I have enough understanding to help you.
- **2022-09-04** `gameoverload` [💬╎general-help] Your probably gonna wanna learn Blender anyway just to know the general process, and it isn't so much making models as presumably working with the existing models from Halo 3, which have all been pulled out at this point one way or the other, or can be pulled out.
  The shader portion would be the harder part, but I know a couple people here have worked on Blender shaders for 3 specifically, unless your doing this in a different engine.
- **2022-09-04** `gameoverload` [💬╎general-help] By tools, I simply mean any specified tools, not assets, that you wish for someone helping you to use.
  Other than that, things like do you want the models used to be accurate to Halo 3/ODST?
  In that case the person helping you will need a Halo 3/ODST shader to work with, which only two people here have done the research to recreate in Blender's node graph based shader system.
- **2022-09-04** `gameoverload` [💬╎general-help] Yes, but be warned, Chief is doing like a bunch of different stuff already, like he always does, so it would take him a fair amount of time to research and put together a H3 shader, and he would probably ask for my help eventually in understanding the HLSL bits.
  Be warned with Chroma also, he doesn't like sharing his stuff to much, since he tweaks it a lot on the daily, and he is also busy himself with projects like Chief does.
- **2022-09-11** `gameoverload` [💬╎general-help] You would need to create a whole new Infinite Shader material for it to work properly, which requires a gradientmask and a detail normal.
  The gradient mask requiring a color map and a roughness map to work properly when put into a texture.
  You also don't need to make a whole new material just to have carbon fiber if you download and append the material types Goblin made, since it includes a similar material to carbon fiber already, then you just go to the material node to the left of the zone you wish to change, and simply switch the material that node is using.
- **2022-09-11** `chunch` [💬╎general-help] https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes2.1
  [embed] Release Material Types 2.1 · AverageTrapEnthusiast/Halo-Infinite-Sh... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes2.1 Redid the way the roughness is calculated (again) for near 1:1 game accuracy.
- **2022-09-12** `chunch` [🔍╎texturing-and-shading] This is the best one tho @The Dog https://youtu.be/r_5suSLUucU
  [embed] How to Port Coatings to Blender from Halo Infinite [100% Accurate] https://www.youtube.com/watch?v=r_5suSLUucU Swatches: https://mega.nz/folder/yWoCxTrZ#0emVAwxOqA85fg9fePy3tA/folder/7XYBVCxT
  
  make sure all of these are in a /materials folder.
  
  CoatingTool: https://mega.nz/file/oqwx0LRJ#cL4vRS7QPj08
- **2022-09-13** `Deleted User` [🔍╎texturing-and-shading] https://github.com/R93Sniper/HaloInfiniteVRCShader
  [embed] GitHub - R93Sniper/HaloInfiniteVRCShader: An open source unity shad... https://github.com/R93Sniper/HaloInfiniteVRCShader An open source unity shader designed to emulate the look of Halo Infinite&#39;s shader system - GitHub - R93Sniper/HaloInfiniteVRCShader: An open source unity shader designed to 
- **2022-09-13** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-09-16** `saigo4k` [💬╎general-help] It is literally this one
  https://github.com/Theathkill9512/Halo-Infinite-Modular-Shader/releases
  [embed] Releases · AverageTrapEnthusiast/Halo-Infinite-Shader-Resources https://github.com/Theathkill9512/Halo-Infinite-Modular-Shader/releases A Collection of various Halo Infinite Shaders I have worked on (and am working on)  - AverageTrapEnthusiast/Halo-Infinite-Shader-Resources
- **2022-09-16** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-09-22** `spartanj104` [📜╎ripping] Hello, 
  I saw that there was no "Infinite Brute Pack" so I stick to the task...even though I've never done that in my life and I am struggling xD.
  I've already ripped the game, output the raw models, sorted through Blender and now I come to the part I hate the most: the textures/shader/UV.
  I try to watch tutorials on this but it doesn't help me understand how you managed the textures like on "Infinite Spartans Pack" I need someone to show me the light 😅 🥲 
  Thank you in advance ✋  🙂
- **2022-09-22** `coarforge` [📜╎ripping] Infinite uses control maps. There is a description of how they work in #📑╎control-map-guide if you want to create your own shader, or you could probably use goblins shader as a starting point (or bacons shader). I haven't tried either of those shaders with the brute models, but they should probably work with some tweaking (the rat for example did work with bacons shader without any modifications, but it's probably not super accurate)
- **2022-10-06** `jaeger1648` [🔍╎texturing-and-shading] Hello, I was told my message might fit better over here
  
  I'm very new to blender and I'm using the reach spartan pack. I couldn't figure out how to export an fbx with any materials/textures at all. Actually the decals exported with it but that's it. Now I kinda know it's because I need to texture bake?
  I found a video on how to texture bake, but ever time I try to smart UV project it messes up the textures, shifts them around on the model a bit. It seems it's because it's not unwrapping right, but I have no clue how to fix it.
  
  I'm about to go to bed, but any help would be greatly appreciated!
- **2022-10-06** `gameoverload` [💬╎general-help] If you really wanted to create your own elements that utilize the Halo Infinite shader, you would want to try that with a Halo Infinite character asset if your just looking to make your own materials for example, or you would want to try with another game's  character asset if you wanted to learn how to make your own textures and materials for it.
- **2022-10-15** `plastered_crab` [📜╎ripping] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-10-16** `pepperman` [💬╎general-help] hey guys, im looking to bake down some textures from blender for use in-engine (halo)
  I've done it before for a few things, but the current model is giving me some issues due to the UV setup
  But i just noticed these outputs from the infinite shader node, are they what i think they're for, and if so how do I go about actually using them? im not very familiar with blender's shader nodes past the very basics
  cheers
  [attachments] unknown.png
- **2022-10-20** `alanticsea` [🔍╎texturing-and-shading] I don't understand where I'm going wrong with baking normal maps using substance painter. Started by making a lowpoly and highpoly model in blender to test. made the lowpoly shade smooth, uv unwrapped the lowpoly, triangulated both the lowpoly and highpoly, checked to make sure the bake looked good in substance, exported normal map to blender, made sure normal map in blender was set to "non-color" with the correct nodes, and now it looks like this garbage.
  
  why
  [attachments] Adobe_Substance_3D_Painter_04J6hZq13l.png, blender_YUWYO7MZWF.png
- **2022-10-23** `vat_` [🦴╎animation] Hey guys, yall see this? https://youtu.be/iXndc_RLr0Y
  [embed] Halo: REACH showreel https://www.youtube.com/watch?v=iXndc_RLr0Y Here is a bulk of the shots/move sets I did for REACH. It was a great project to work on! 
  
  All material copyright Microsoft Game Studios and Bungie Studios. 
  
  To be viewed for entertainment purposes only.
- **2022-10-25** `exporush` [💬╎general-help] How do I get my laptop open the template.py file using notepad like in this video? 
  
  https://www.youtube.com/watch?v=r_5suSLUucU
  [embed] How to Port Coatings to Blender from Halo Infinite [100% Accurate] https://www.youtube.com/watch?v=r_5suSLUucU Swatches: https://mega.nz/folder/yWoCxTrZ#0emVAwxOqA85fg9fePy3tA/folder/7XYBVCxT
  
  make sure all of these are in a /materials folder.
  
  CoatingTool: https://mega.nz/file/oqwx0LRJ#cL4vRS7QPj08
- **2022-10-29** `gameoverload` [💬╎general-help] I doubt it is the shader given their is a lot less there, from what I have seen of it, compared to the Infinite one.
  The other thing is getting properly what Halo 5 uses, given we don't exactly have all the actual sampler references and such known yet.
  So that would need to be known before the right fixes can be applied.
- **2022-11-03** `fable_law_458` [🔍╎texturing-and-shading] Hi, Casey. If you're looking to see what specific material that you're looking at in CE, Go in Sapien or Standalone. Open whatever level it may be, then, type into the console `debug_camera true`. What this will do is it will display the shader name and the directory on the top left of the screen on whatever shader you're looking at that it's using. 
  
  As far as pixel shading or vertex shading you mentioned about, I don't think you can view those inside the Halo: CE's engine. You may need to speak to someone on reclaimer's discord server that might know the answer to that question.
  
  I hope this does assist you with what you're wanting to do.
  
  Best regards,
  -fable
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
- **2022-11-07** `hausdev` [🔍╎texturing-and-shading] I'm a brainlet when it comes to most 3D stuff, and I've been focusing on CEA so my H2A knowledge is a bit rusty, but in the texture definitions json, i beleive there's a field called "preset" or something along those lines. It might be called something else idk
  
  But I think once you have that name, you can find that file under Shader Definitions in index, and that'll have the HLSL code. Iirc it's polymorphic, so theres a base shader that other shaders build on top of, so you'll probably need to navigate a few different files to find what you're looking for. There's also a lot of mathematical functions in the "common" shader file, if that helps
- **2022-11-07** `gameoverload` [🔍╎texturing-and-shading] If that is not what your looking for, at that point you will need to then rip all the geo, and make sure all that is set up as you see in game, then begin to grab all of the need textures, texture data like tiling, colors, etc, then you will need to find all of the shader code data, ala in the Reach Tools, for the environment shading you need for that level, then you would build a shader from that, then after that, you input all of the textures and values you have into that shader.
- **2022-11-08** `gloorby` [💬╎general-help] Okay so I want your guys opinions on something. I'm attempting to make a Halo animation but I'm unsure of which software to use. I'm tossing up between Blender or UE5 but I'm just not sure. I'm far more experienced with UE5 but I can't import many of the assets I want to use due to the Halo Infinite shader. On the other hand Blender is much easier to animate in plus the assets are all there but I find it harder to make scenes and the render times are insanely long even for a 30 second clip. What do you guys think I should do??
- **2022-11-10** `nihalrahman` [🔍╎texturing-and-shading] you can easily swap the normal distribution to be simpler or different ( and many games do ), just cause cook-torrance expects to use beckmann, doesnt mean it does, ofcourse, reach may just use that but howd i know, thats why im asking
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
- **2022-11-18** `grandbacon` [🔍╎texturing-and-shading] *I've* only ever used my old infinite shader on MP armor as that's what it was made to do, I don't know how it works on the elite and it definetly won't work in unity since it's a blender shader do if you want it to work in unity you'll have to bake some textures
- **2022-11-27** `gameoverload` [🔍╎texturing-and-shading] It could also be, as Goblin said, be colored spec, which is when your specular type is colored rather than greyscale dielectric.
  You would connected your base color to a diffuse BSDF, then your colored spec to the base color input of a principled BSDF, then you would use an add shader node to add the outputs of the other two shader nodes.
  Lastly, you would plug the normal map into both normal map inputs of the two shaders.
- **2022-11-30** `jadedqrow` [📜╎ripping] It’s called reclaimer. I don’t have any links on hand but discord has a search function you can use, you’ll have to make your own shaders for it too look game accurate though don’t think anyones got specific ones for reach maps
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
- **2022-12-08** `exporush` [💬╎general-help] I always feel stupid when watching this video cause I can never get it right. So what am I supposed to change when when he says to change the extension?
  https://youtu.be/r_5suSLUucU?t=35
  [embed] How to Port Coatings to Blender from Halo Infinite [100% Accurate] https://www.youtube.com/watch?v=r_5suSLUucU&start=35 Swatches: https://mega.nz/folder/yWoCxTrZ#0emVAwxOqA85fg9fePy3tA/folder/7XYBVCxT
  
  make sure all of these are in a /materials folder.
  
  CoatingTool: https://mega.nz/file/oqwx0LRJ#cL4
- **2022-12-12** `gameoverload` [💬╎general-help] Now you would create a map in say Blender if your using the Halo Blender tool set, then use all the other tools to handle the materials and such, but otherwise, you use the mod tools to look at most of the data for a level, while the source shader code would be read in something like C++.
- **2022-12-24** `gameoverload` [💬╎general-help] Well if your baking metallic, you should be doing it from the ending node that creates what you have as metallic, not from the Principled BSDF. That is why Goblin keeps saying it works, because the Spartan shader has an output called Metallic that goes from the ending metallic creating node just before the BSDF, to the Metallic output of the shader.
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
- **2022-12-27** `surasia` [📜╎ripping] https://github.com/Surasia/Infinite-Coating-Tool/releases/tag/2.0 this is the newest version
  [embed] Release Infinite Coating Tool 2.0 · Surasia/Infinite-Coating-Tool https://github.com/Surasia/Infinite-Coating-Tool/releases/tag/2.0 Brand new template including HIMS 2.7, new shader math for swatches and GrimeSwatch support.
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
- **2023-01-12** `gameoverload` [💬╎general-help] Given the XCOM 2 Objective UI shading is built to handle the Diffuse ( Base Color ) and the Normal Map of a mesh by default, the only other thing missing that would need to be built in is the Metalness, Smoothness ( Roughness ) and color changing elements of Halo 5's shading, which is not hard by any means really, but it depends on whether you want the shading combined, have a switch to go between being the shading of Halo 5, and this, or being able to turn on or off the shading going overtop the Halo 5 shading.
  
  Depends on what you need.
- **2023-01-18** `.bombwolf` [💬╎general-help] If I remember there is a limit in it that's hard set.
  You can either crank up the emission of the actual lights (if you don't want them lighting up the nearby area, you can use a "Light Path" node in the shader. Drag camera ray into a mix shader node with the one you want to glow/see and the one you want to emit in the world.
  Second option is to render out the emissive pass separately, and use that data in another editing software to make glows to overlay over your render)
- **2023-01-20** `rubyqueen0` [💬╎general-help] Im guessing this question has been asked a few times (or something in this vain).  Im looking to get the halo reach/infinite Spartans tho blender to unity to vrc. I have a "good" understanding of the unity side of making avatars but am having a lot of trouble getting it over from blender to unity. missing texters, bones braking, what's the profred shader and so on. If there's anyone that can run me tho the best way to go tho this proses that would be wonderful. (can dm) Sorry if this is the wrong place to ask
- **2023-01-21** `blue.nasca` [🔍╎texturing-and-shading] so im struggling to get a particular shader effect going
  i'm trying to have a piece of armor gradient from the painted armor color to steel, like how carter's legs go from blue to steel
  i've tried color ramps and gradient textures but basically instead of a progression down the texture from one color to the next, its just blending the two together and turning the entire thing one color, which isn't what i want 
  is there anything i can use to make the effect im looking for or am I just SOL?
- **2023-02-22** `planetalexanderprojects` [🔍╎texturing-and-shading] Download can be found here:
  https://www.405th.com/forums/threads/procedural-armour-texture-setup-for-blender.53457/#:~:text=Cycles%20render%20engine.-,22/02/23,-%3A%20Version%202
  [embed] Procedural Armour Texture Setup for Blender https://www.405th.com/forums/threads/procedural-armour-texture-setup-for-blender.53457/ 22/02/23: Version 2 is available, download at the end of the post!
  
  I recently made a Belos Horan helmet from MCC that was to be used in a render of a spartan. To make 
- **2023-02-26** `wake9090` [💬╎general-help] So if I wanted to import the coating redacted records from the reach core. Do I use `M:\__chore\gen__\objects\characters\spartan_armor\coatings\reach\reach_spartan_style{ct}.materialstyles` and select it in the style or is there a different place I have to find it? And if this is correct, would the material path then be `M:\objects\characters\spartan_armor\coatings\reach\`?
- **2023-03-21** `gameoverload` [📜╎ripping] Not when you want all the assets to look cohesive, which you will run into the problem of with using photo-realistic mega-scans vs other actual Halo 4 shaded assets alongside each other, unless you know exactly how Halo 4 is shaded, which you could create a similar shader for if you know that information that can use that mega-scan data.
- **2023-03-26** `plastered_crab` [📜╎ripping] in that case you will likely need a few things
  1: the .map files of the level !maps
  2: the mod tools for Reach from Steam so you can see what the values are for each material
  3: to use Reclaimer to rip the level geo and get it into Blender
- **2023-03-27** `plastered_crab` [📜╎ripping] I'm in progress with a tool that aims to do most of the work for the user with the help of shader folks like Soul and Chiefster making the shaders. The tool detects the material names and maps all the nodes and values read straight from the raw binary of the Halo game files
- **2023-03-30** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2023-04-01** `onemadwomble` [🔍╎texturing-and-shading] Need help. Trying to use textures from Halo Infinite ODST Pack. Don't know what the channels in mask_0_control or mask_1_control are for. I believe the asg_control is red=specular, green=roughness. Is there any documentation that might shed light on how I might setup an unreal shader? Would the best approach be to reverse engineer blender shader?
- **2023-04-05** `scouter407del` [🔍╎texturing-and-shading] https://github.com/Scouter407/HaloInfiniteToPBRMetalRough-SpecGlossConverter/releases
  [embed] Releases · Scouter407/HaloInfiniteToPBRMetalRough-SpecGlossConverter https://github.com/Scouter407/HaloInfiniteToPBRMetalRough-SpecGlossConverter/releases A substance painter file that takes the ASG, control zero, control one, and normal map to output the base color as the masks all combined with a 
- **2023-04-06** `scouter407del` [💾╎substance-suite] https://github.com/Scouter407/HaloInfiniteToPBRMetalRough-SpecGlossConverter/releases
  [embed] Releases · Scouter407/HaloInfiniteToPBRMetalRough-SpecGlossConverter https://github.com/Scouter407/HaloInfiniteToPBRMetalRough-SpecGlossConverter/releases A substance painter file that takes the ASG, control zero, control one, and normal map to output the base color as the masks all combined with a 
- **2023-04-10** `stanlyhalo` [📜╎ripping] ah yeah, the models is just what I'm after... I develop my own stuff, and was wondering how infinite is able to achieve certain forerunner pieces that look like models... but could honestly be normal mapping, vertex displacement and/or parallax mapping (that one I doubt tho unless it's some h3 style holograms).
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
- **2023-04-17** `gameoverload` [💬╎general-help] So that means it would most likely be:
  
  Diffuse Colour [ Funny British moment. ] - Diffuse Colour: Color - Simply Color
  
  Specular Gloss: Vec4 - Gloss [ Global ] , Spec RGB [ RGB ]
  
  Metallic: Vec 4 - Spec Intensity [ Global ], Spec RGB [ RGB ]
  
  Clear Coat: Vec 4 - Clear Coat Gloss [ Global ], Clear Coat Spec RGB [ RGB ]
  
  Emissive: Vec 4 - Emissive Intensity [ Global ], Emissive RGB [ RGB ]
  
  Diffuse Wrap: Vec 4 - Unsure
  
  Damage: Vec 4 - Damaged Intensity [ Global ]
  
  Damaged Blend: Vec 4 - Blend Intensity [ Global ], Unsure [ RGB ]
  
  Damaged Colour: Color - Damaged Color
  
  Spec Gloss, Metallic, Clear Coat Override: Float - Assuming this is to mix all 3 Spec RGB values together in some form.
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
- **2023-04-26** `gameoverload` [💬╎general-help] You'd have to look at what textures are listed, what their names are, and then if they don't list the individual channel held maps, then you'd need to look at each channel on the model and determine what is what map, which you can use the current shader as reference as the shader I built has shader specific outputs to show specific shader inputs like metal, roughness, AO, and color. Then make your nodes from that information.
  
  You can use my shader as reference if need be.
- **2023-05-06** `aeivindur` [🔍╎texturing-and-shading] I just downloaded the Platinum Anniversary MKVII Coating from Surasia: https://discord.com/channels/819709630540021810/821676515099279374/1015186265592844288
  and am about to attempt to apply it to my spartan, and I wanted to pop in here to get any advice on how I can streamline the application of these shaders to armor parts from the halo infinite spartan pack? I've manually input color values on a couple spartans before with custom colors but now I want to make the process a *little* less tedious lol
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
- **2023-05-16** `dgenpoly` [💾╎autodesk] I've been trying to get Maya to fit easier into a Halo Mapping workflow, one of the many issues I have ran across is material names and prefixes. Coming from 3dsMax I was expecting a straight forward transition, for the most part it has. Today I ran into a material issue while assigning my sky material, when I go to name it "+sky" Maya removes the "+" automatically. Has anyone else ran into this and have a fix or a workaround?
  [attachments] image.png
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
- **2023-05-22** `gameoverload` [💬╎general-help] Well in that case for Unity you'd need to build custom shaders and all of that to work with the textures as they are, which means you'd need shader info from existing shaders to make the shaders in Unity via something like Amplify. As Halo Reach won't just be plug and play at all with Unity default shading.
  
  Plus yes, you will need to apply textures, they won't all be applied on import into Blender, only the Diffuse.
- **2023-05-22** `gameoverload` [💬╎general-help] You'd need shader information and material information reference from the mod tools, which is what anyone wanting Reach shading has to do for all the proper parameters and such. Since those textures work in a specific way that you'd need that mod tools given information for, other than that you can see about using the existing Halo 3 shaders as reference that any maps here may be using, as Reach was built off 3, and then work from there to tweak what you use as reference from there, again in Unity though you'd need to build all custom shaders for that.
  
  TLDR you'd be going into creating a ton of shader stuff that depending on how much you want to go through with this, is going to be a lot of work for Reach environment stuff.
- **2023-05-22** `felixvyra` [🔍╎texturing-and-shading] It didn't help.
  I understand what color is what map now but I have no idea how to combine them.
  I tried 2 different Halo 4 shader node groups that I found in some of the uploaded pre textured assets but even those look incorrect
- **2023-05-24** `theidiot5651` [💬╎general-help] https://www.youtube.com/watch?v=Jm39-vYl1zo&t=86s
  
  https://www.youtube.com/watch?v=LdmoBY08LTM
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo&start=86 This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan p
- **2023-05-28** `a.hypocrite` [💬╎general-help] I've come to bother you all about something arbitrary that I'm just now noticing and can't figure out-
  
  > *Attempting to add camos to individual Spartans- these camos are pre-colored (images ripped straight from google) and are applied through an image texture and plugging the color nodes into the appropriate zone.*
  > 
  > *When shown in material view, the textures are applied. But when transitioning to rendered view, they're replaced with a silver color. For the sake of ruling out lighting not interacting well with blue tones, I've also tested with a model using a darker-colored camouflage. Same issue is occuring.*
  
  How can I fix this? Am I forgetting something somewhere?
  [attachments] 1.png, 3.png, 4.png, 5.png
- **2023-06-12** `_ashrowan` [🔍╎texturing-and-shading] Fellow shader wizards I need your help setting up shaders for a Halo 5 Sparton for friend in blender. My goal is to bake the textures so I can optimize it for for VRChat. Both Recluse Male and MK V Alpha have the R and G channels inversed in the Open Halo 5 unity shader for a "Fake Zone 3" Not really sure how to apply that in blender.
  [attachments] image.png, image.png
- **2023-06-12** `fiycsf` [🔍╎texturing-and-shading] It was done, because a bunch of people that had no idea what they were doing way back made a halo shader that used the metallic channel as a colour zone. Obviously that didn't work well, yet some people insisted "halo 5 has 3 colour zones"
  
  So I made it work so people who were wrong, would still switch.
  
  Halo 5 is the easiest to convert to unity standard.
  
  Red and green are colour zones one and two.
  
  Blue is metallic 
  
  Alpha is smoothness,
- **2023-06-12** `fiycsf` [🔍╎texturing-and-shading] https://docs.google.com/document/d/1m9FAWTaYM7M5MmCnKYz4UxhvQSTQp_wNW33ruYctWOM/edit?usp=drivesdk
  [embed] Unity Standard Shader and H5 Spartans https://docs.google.com/document/d/1m9FAWTaYM7M5MmCnKYz4UxhvQSTQp_wNW33ruYctWOM/edit?usp=drivesdk Using Unity Standard Shader and Halo 5 Spartans Breakdown   Introduction  Want to show up for everyone in VRC who hides shaders, or look good on Quest with Quest Li
- **2023-06-12** `fiycsf` [🔍╎texturing-and-shading] To put it short, it has horrible lighting setting, and mostly ignores the lighting around it.
  
  This has directly resulted in a massive portion of "Vrchat creators" who have zero understanding of basic world lighting, and a habit of saying "standard sucks, look how it looks here, use Poiyomi"
  
  Rather then just setting up basic lighting in worlds in unity, uneducated people shared misinformation, which resulted in widespread use of a massively overcomplicated, poor preforming catch all shader that looks terrible when compared to something actually intended to do the specific job
- **2023-06-14** `gingerjesus.` [🦴╎animation] I'm really late, but I strongly recommend Blender. Modern tools & technology, you can use/develop assets that aren't restrained by the likes of the Source engine. Source engine itself(for HL2, Gmod, SFM, such forth) has various hardcoded limitations for assets that effectively make cloth-sim impossible for example. It does not support modern shader methods such as PBR, forcing experts to use very roundabout methods to emulate it. Lastly, SFM itself has memory leak problems that can be *very* tedious to deal with when making a scene with a massive variety of assets or even a number of high-quality assets.
- **2023-06-22** `chunch` [🔍╎texturing-and-shading] https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/ModularShader2.3
  [embed] Release Halo Infinite Modular Shader (2.3) · AverageTrapEnthusiast/... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/ModularShader2.3 A fully revamped version of the modular shader!
  Now includes node groups that can be used to convert older halo games models an
- **2023-06-27** `gameoverload` [💬╎general-help] https://ambientcg.com/view?id=SkyOnlyHDRI032
  
  For a Sky Only HDRI, I would suggest this one, as you don't need the sky to be a desert color, as that is done through reflections of the terrain in your environment.
  
  I would suggest downloading the `2K-HDR.exr` version specifically.
  [embed] Sky-Only HDRI 032 on ambientCG https://ambientcg.com/view?id=SkyOnlyHDRI032 Asset 'Sky-Only HDRI 032' on ambientCG. Get this asset and 1000+ more PBR materials and textures for free under the Public Domain license.
- **2023-07-09** `deltahalocontrolroomcore` [💬╎general-help] Got a quick question-
  
  Doing a render for a couple people that won a championship. One of them has a knife on their wrist, which means I have to unparent it from the original armature and apply it to the new one with new vertex groups and all. Should be the easiest thing in the world. However, the instant I tab into edit mode, the shader *instantly* breaks. I know it's not an issue with Blender, or something I'm doing out of the ordinary (because the Mirage pack can tab in and out of edit mode just fine), which means this issue is present only with assets in Spartan Pack 3.5. Does anyone know a fix or workaround to this issue?
  [attachments] image.png, image.png
- **2023-07-10** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
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
- **2023-07-12** `setriaphtfk` [💬╎general-help] I downloaded this shortly before you responded and it seems to work
  https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes2.1
  [embed] Release Material Types 2.1 · AverageTrapEnthusiast/Halo-Infinite-Sh... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/MaterialTypes2.1 Redid the way the roughness is calculated (again) for near 1:1 game accuracy.
  All assets posted here are posted for free, thus 
  [attachments] image.png
- **2023-07-13** `chunch` [🔍╎texturing-and-shading] https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/ModularShader2.9
  [embed] Release Halo Infinite Modular Shader (2.9) · AverageTrapEnthusiast/... https://github.com/AverageTrapEnthusiast/Halo-Infinite-Shader-Resources/releases/tag/ModularShader2.9 HALO INFINITE MODEL SHADER VER 2.9
  Changes:
  Fixed the issue where having one value put into all three color slots breaks the fi
- **2023-07-15** `sswampp` [💬╎general-help] to do it properly would take a lot of explanation, but if you have to do it manually and not with the CR4B tool, you want to get the shader info (for each material) from the game either with assembly or with the editing kit (guerilla) and copy it over to a blender shader/nodegroup that's set up for game accuracy
- **2023-07-15** `Deleted User` [💬╎general-help] How do I say this; I put the bump and tiff on the zan_seawall material for last resort, and I got everything set up correctly according to a video I found, but the textures are all stretched and there are two color options in guerilla which just makes it more confusing, I'll send some screenshots if I can
- **2023-07-19** `tpg` [🔍╎texturing-and-shading] Don't use Poi for Halo assets, just don't. There's a dedicated Halo 3 Shader here: https://canary.discord.com/channels/819709630540021810/821051966058004501/881146101632213032
- **2023-07-29** `plastered_crab` [💬╎general-help] You can use this to convert those cubemaps
  https://youtu.be/dpMQr59vSys
  [embed] How to install and use Cubemap Converter Blender Addon https://www.youtube.com/watch?v=dpMQr59vSys I made this Blender Addon for converting Cubemaps to Equirectangular maps and making it as easy as possible. This video will show you how to install it and use it. Hope it helps!
  
  Link to tool: https:/
- **2023-08-09** `brooen` [💬╎general-help] https://www.youtube.com/watch?v=dpMQr59vSys this is the conversion tool
  [embed] How to install and use Cubemap Converter Blender Addon https://www.youtube.com/watch?v=dpMQr59vSys I made this Blender Addon for converting Cubemaps to Equirectangular maps and making it as easy as possible. This video will show you how to install it and use it. Hope it helps!
  
  Link to tool: https:/
- **2023-08-09** `chiefster_4014` [🔍╎texturing-and-shading] That tutorial and those shaders are old, you'll want to use the CR4B tool now instead since it's much faster to get things set up.
  https://www.youtube.com/watch?v=749xwvg-GK4
  [embed] CR4B Tool Setup Guide (Game Accurate H3/ODST/Reach Importer) https://www.youtube.com/watch?v=749xwvg-GK4 GitHub Link: https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool
  
  Halo Archive Discord: https://discord.gg/haloarchive
  
  Completely Ready 4 Blender (CR4B) Tool
  This is a tool I have spent the l
- **2023-08-10** `plastered_crab` [💬╎general-help] CR4B is something I've been working on for about 9 months now and reach support just started as it took this long to program most of the Halo 3 logic and tag binary reading from the mod tools haha. Not to mention the hundreds of hours put into the shaders by Chiefster and now also Soulburnin with a huge chunk of the reach shaders it uses 🙂
- **2023-08-27** `bl00df4rt` [💬╎general-help] Looking for a good Halo 3 Shader for Unity, specifically for rocks and other tileables as I'm porting a map and I want it to look as accurate to its in-game counterpart as possible. I've looked in shaders-resources and the only  "Halo 3 Shader" I see is the Halo 3 *Spartan* Shader, but since I'm porting a *map*, this shader does not work for me. I am unable to adjust the colors properly and the map ends up not looking right.
  
  I'm asking if the Halo 3 Spartan Shader is the only Halo 3 Shader for Unity, and if there is a Halo 3 Shader that'd be better for maps, I would appreciate being pointed in the right direction.
- **2023-08-27** `gameoverload` [💬╎general-help] That is why I explained things for the script bit and piece, but he was going to the shader tab like we were already going to, to look at the textures to make sure any were blank, again, like I was already having him do.
  
  He wasn't finding his own solution there, that was with the script stuff he thought about, but directed him away from and explained since that wasn't the issue, while for the texture stuff, I was pointing him to that.
  
  So I was doing what you do also since I have to take that approach at work also, but the texture part was not on him, I pointed him to that instead.
- **2023-08-29** `liquidplazmid` [💬╎general-help] I was trying to port a reach spartan model into Unity to later put into vr chat. I was able to use reach shaders I found in the server for everything else. 
  
  The decals and shield needed a different setup though. I ended up using an FX shader and it looked fairly ok
- **2023-09-01** `momomainia` [💬╎general-help] Okay well here's the situation that I'm encountering. That model pack isn't compatible with Olympus (not even completely sure what that is yet) and I need to use this shader pack (https://discord.com/channels/819709630540021810/1072705765208506369/1072705765208506369) on the armor. But from my understanding you need to use Olympus to use the shaders. So my question is how do I get those shaders onto the models from the Spartan pack??
- **2023-09-02** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo?si=7V7QIHIQTpvSh9IS
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2023-09-29** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo?si=cKIiV8tuybx8RE_G
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2023-10-02** `Deleted User` [💬╎general-help] hey. i'm using MrKoix's Halo CE Spartan model (as I haven't really found an alternative that's IK rigged and has shaders + shield), but I'm unable to figure out how to actually use/enable the shield. How can i turn the shield shader on/off?
- **2023-10-16** `gameoverload` [💬╎general-help] You don't need to do any of the location, rotation, nor scale stuff in UE since as you can see, none of the default values for that were changed at all in Blender.
  
  You also do not need to invert the normal map, unless it was baked out as OpenGL ( You can change this in Blender's Cycles bake settings. ), otherwise, it should be DirectX by default.
  
  You don't need any of the strengths unless you have them in Blender.
  
  Also make sure your non-color maps have sRGB turned off in UE.
- **2023-10-21** `gameoverload` [🔍╎texturing-and-shading] Except what you mean is HI is just doing one shader, then ( without seeing it as a reference in HIRT ) is slapping over another shader over that as needed, and swapping back and forth depending on rendering needs, often keeping on the other say for example in the customization menu when your flipping through customization items.
  
  Probably combining that all into a texture then the main actual referenced shader uses that data for maybe when your in a match or something...
- **2023-10-28** `apexrainbow163` [💬╎general-help] For some reason 343 decided to have colors that don’t actually go full color on the armor sets. This is most notable on the Achilles lower half where if you want to have full white, it’ll just be gray. 
  
  In short, some armor sets don’t go full color, this even happens in Game with the halo 5 armor sets. 
  
  Try using the infinite converter shader
- **2023-10-31** `charmingquark09` [🔍╎texturing-and-shading] if you are using blender, the way that I have duplicate textures linked is to go into the **"shading"** tab, and at the top of the grey workspace there is this little beachball-looking icon. Click on the beachball and a drop-down menu appears. Either make a new **material **or edit an existing one and name it something easy to type. Click on each object you want to have that texture **separately **(as far as i know, blender wont let you assign a texture to multiple objects at once... which sucks. please correct me if im wrong lol) and using the beachball drop-down menu, apply the texture. 
  
  The good thing about this method is, you can edit just one material and it applies to everything you've assigned to it. Good for having a primary/secondary color that you can change on the fly, or to test out color schemes. To get rid of un-used materials, I believe there is an extension in blender that lets you delete all unused materials, but i cant remember what its called.
  [attachments] image.png
- **2023-11-03** `r93_sniper` [🔍╎texturing-and-shading] https://github.com/R93Sniper/HaloInfiniteVRCShader/blob/master/LICENSE
  [embed] HaloInfiniteVRCShader/LICENSE at master · R93Sniper/HaloInfiniteVRC... https://github.com/R93Sniper/HaloInfiniteVRCShader/blob/master/LICENSE An open source unity shader designed to emulate the look of Halo Infinite's shader system - R93Sniper/HaloInfiniteVRCShader
- **2023-11-19** `gameoverload` [💬╎general-help] In Blender terms, its a Mix ( Color, Vector, Float ).
  In UE terms, its the same, a Lerp. ( If I remember right. )
  In Unity terms, it depends on whether your using a third party shader tool, or Unity's own native shader tool.
- **2023-12-03** `purplmunkii` [🦴╎rigging] https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2023-12-09** `gameoverload` [💬╎general-help] Well the major change was how specularity is handled now, particularly in the input change of now the Specular Level and Specular Color are two seperate inputs, and Blender has now the habit, with old enough BSDFs, to tack on a mix multiply node which sets the B input to the original Specular Color value they used, because originally it was just the Specular input, which was internally the Specular Level * Specular Color, which is what the mix multiply node is doing that they tack on.
  
  That is the issue, in full context.
- **2023-12-11** `Deleted User` [💬╎general-help] using reclaimer rn and trying to open shaders on construct.map (h3) but to no avail.... do i need to extract the bink, images, info from their respective folders or should reclaimer be able to access them if they are within the .map's folder
- **2023-12-11** `Deleted User` [💬╎general-help] ```ArgumentOutOfRangeException while porting 'levels\solo\070_waste\shaders\waste_panel_pipe_blue.shader':
          Index was out of range. Must be non-negative and less than the size of the collection.
  Parameter name: index
  
  
  ArgumentOutOfRangeException while porting 'levels\multi\construct\construct.scenario_structure_bsp':
          Index was out of range. Must be non-negative and less than the size of the collection.
  Parameter name: index
  
  
  ArgumentOutOfRangeException while porting 'levels\multi\construct\construct.scenario':
          Index was out of range. Must be non-negative and less than the size of the collection.
  Parameter name: index
  
  ERROR: Index was out of range. Must be non-negative and less than the size of the collection.
  Parameter name: index```
- **2023-12-17** `nihren` [🔍╎texturing-and-shading] I try not to touch the normal map, even for upscaling. And any time I've tried it breaks, so I leave it alone. Some models I've worked with recently had the texture detail, like scratches and all that, under a mask or control, but that doesn't seem to be the case here so I'm focusing on the Color texture. Unfortunately, feels like an effort in futility. I tried the Halo 4 Grunt, which is set up similarly, and what solved the problem there was I simply used just one of the extracted channels, in this case the Red channel, and then used the original texture as the alpha, and it worked perfectly. Tried it on the H5 Grunt, and it didn't work.
- **2024-01-02** `amsierra` [🔍╎texturing-and-shading] Does anyone happen to know where exactly i can pull texture scale data from in assembly for each texture in a material? For example im doing one of the terrain_blend materials for a halo reach map and instead of going by eye like i used to do for the tiling on each texture i was wondering if i can get the game accurate scaling somewhere in the shader data for each texture using assembly
  [attachments] forgemaphalo.png
- **2024-01-03** `thoro.blend` [💬╎general-help] Alright, so I've seen a bunch of people have a primary and secondary "shader?" to use for the spartans. 
  
  How can I set this up as I still haven't figured out how to yet, other than making an rgb node and plugging it into every slot on *each* piece.
- **2024-01-05** `tomhogenberg` [💬╎general-help] general help needed with the importing of the sabre from reach, using it as a placeholder/first iteration for an animation. maybe someone knows specifically what causes the texture glitching and behavior with reach assets. its not reliant on the texture(s) and materials applied. anyone has a clue, if this is halo (reach) asset related?
  [attachments] Screenshot_2024-01-05_135220.png
- **2024-01-13** `grimreaperking` [🔍╎texturing-and-shading] So, I'm using the spartan pack, to create custom camos, and whenever I do the color gets all "washed out" compared to what it should be, not quite sure what I'm doing wrong/missing here. I'll add a rendered picture in eevee, cycles, a screenshot of how it looks in material preview and the node setup.
  [attachments] eevee.png, cycles.png, image.png, image.png
- **2024-01-18** `_tobyr` [🔍╎texturing-and-shading] Hi all, I recently downloaded CanadianDragon's Noble Team models to mess around with and I'm having some trouble with the materials. Most of the model is showing up as white, both in material preview and rendering. For some reason the shins on Jorge's model are colored in fine, but the rest of the model is white.
  It looks like the materials are set up so that the textures provide detailing to the armor, and the color comes from elsewhere. I say this because the diffuse textures have no color, just detailing, with dark highlights along armor ridges and transparent where there is no detailing. These detailing highlights show up on the entire model, but the army green color only shows up on Jorge's shins -- the rest of the model is colored white. This is puzzling because the shins are part of the same model that composes the whole torso & lower body -- the materials and textures cover those anatomy parts. But only Jorge's shins are colored.
  Oh, and side note -- all the stuff that works uses a .tiff textures in the materials. Some of the stuff uses .png textures in their materials and none of that stuff works.
  I have enough experience in Blender to poke around and make general deductions about what's going on, but not experienced enough to do more than that. I also don't fully understand how to work with materials pulled from game files, with the ways stuff gets bundled into different channels of a single texture to be more efficient. If anyone has personal experience and can hel ...[cut]
- **2024-01-19** `frostbitecinematics` [💬╎general-help] You can use the game models no problem because nobody here owns them, ***but*** from what I've seen, most of the asset packs that people put together all try to use somebody else's master materials (since why would you want to remake shaders again from scratch when the work is already done).
  
  AFAIK, the shaders that the community rely on have got a licence that doesn't allow them to be used for paid commission work. If that's the case, you'd probably need to just recreate the shader logic yourself based off of the existing shaders out there. That way it's fine to charge for commissions without breaking the shader creator's licences.
- **2024-01-24** `nihren` [🔍╎texturing-and-shading] Feel like this might be the channel to ask in. Does anyone know what the best way to make PBR textures is? The more I look at these H3 vehicles with their PBR textures the more I feel like the rest that don't have them could use them.
- **2024-02-05** `.williamfaucher` [💬╎general-help] Hey folks, so I'm a bit of a blender noob here. I've got the amazing Infinite Pelican model here: https://discord.com/channels/819709630540021810/958797365085630494/983603593892487239
  But I was wondering if anyone knows how I can export that into Unreal, while preserving the texture paths? I am fully aware I need to rebuild the shader in Unreal, but if at least the textures were still linked it would be a solid starting point.
- **2024-02-08** `matta_klark` [💬╎general-help] Does anyone have set up the H3 Albatross file available on the Drive?
  If so, how can I get rid of the seam that appears on the middle when normal map Is applied?
  I've tried deleting one half of the model, then mirroring the other, reversing normals, moved the uvs, but no luck. It seems like the geometry Is busted somehow.
- **2024-02-10** `gameoverload` [💬╎general-help] Use this, it includes a normal map, both GL and DX:
  https://ambientcg.com/view?id=Tiles072
  You also have funny white version:
  https://ambientcg.com/view?id=Tiles071
  *Technically you could greyscale either base colors and adjust the color via that.
  [embed] Tiles 072 on ambientCG https://ambientcg.com/view?id=Tiles072 Asset 'Tiles 072' on ambientCG. Get this asset and 1000+ more PBR materials and textures for free under the Public Domain license. Tiles 071 on ambientCG https://ambientcg.com/view?id=Tiles071 Asset 'Tiles 071' on ambientCG. Get this asse
- **2024-02-14** `chiefster_4014` [💬╎general-help] There's no way to export the color than to do it with baking, but there is a shader for Unity that should work for what you need.
  https://gitlab.com/FiyCsf/open-halo-reach-spartan-shader-for-unity
  [embed] Cameron "Fiy" F. / Open Halo Reach Spartan Shader for Unity  · GitLab https://gitlab.com/FiyCsf/open-halo-reach-spartan-shader-for-unity This should play nicely with all Halo Reach MP Spartan Textures etc, and is written using the amplify shader editor and is clearly commented should anyone wish to.
- **2024-02-17** `tacticalmeme6640` [🔍╎texturing-and-shading] Anyone know anything about how Ready or Not handles materials? I don't know shit about fuck when it comes to material stuff so I have no idea what I would do with this texture which is apparently supposed to be for metallic, roughness, and AO all at once. The model also came with base color and normal textures and those are easy enough to just throw into the standard principled bsdf since I'm not going for game accuracy, I just need to find out what to do with this
  [attachments] SWAT_Head_Cowboy_OcclusionRoughnessMetallic.png
- **2024-02-19** `chiefster_4014` [💬╎general-help] Make sure both materials have the image texture you're baking onto, then use the outputs labeled Diffuse, Metallic, etc. on the shader to bake. You'll have to set bake as Emit for everything except the normal, which for that you would just use the BSDF output for rather than the Normal output, and set bake type to Normal.
  Also when you bake the normal, if you're on Blender 3.6+ you'll need to set the model to shade smooth rather than shade auto smooth. You can set it back to shade auto smooth after baking the normal.
- **2024-02-19** `spiritiudex` [💾╎substance-suite] The only real advice I can give is normal variation (subtle scratches and other details), small roughness variation maybe for dust, and anchor points for edge pronunciation and recess shading. Idk how much you already know so I’m just throwing that all out there. There isn’t too much you can do with under suits for spartans unless you want to change the actual color
- **2024-02-21** `gameoverload` [🔍╎texturing-and-shading] It goes Zone 0 ( Black below all zones ) then Zone 1 - 6, then scratch is calculated per Zone via the scratch mask as the mask for where the scratch effect occurs, which in game is 1 value for scratch intensity, but is divided into a per zone value in the Blender shader, and then grime is on top of that via the grime mask in the ASG.
  
  The actual names of the base textures in the actual shader are as follows:
  Base Occlusion ( ASG )
  Material ID 0 ( Mask 0 ) ( Zone 1 to 3 )
  Material ID 1 ( Mask 1 ) ( Zone 4 to 6 )
  Base Normal ( Normal Map )
- **2024-02-21** `fiycsf` [🔍╎texturing-and-shading] Open Halo 5 (unity shader made for the Vrchat lazy) has had it handled without white seams for several years now. 
  
  I can't even remember the exact method, but if you have amplify shader editor you should be able to crack it open and see what order it does things.
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
- **2024-02-22** `pendragone01` [💬╎general-help] Hello everyone! I stumble upon this discord while looking for some specific answers about modding Halo, and I'm stoked to be here! Right now I'm trying, in Blender, to change the color of certain shaders on the marine I've ripped from Halo Reach, and I could really use some help. If y'all could point me towards any specific tutorials or documentation on it, I'd really appreciate it.
- **2024-02-26** `coarforge` [💬╎general-help] If you have everything set up, select the Image Texture node you want to bake to (just click on it in the shader editor) and then in the Properties Panel click the bake button (the panel where you set up the settings under `BLENDER PREFERENCES`). Depending on your blender version, the settings might be under the Render Properties tab instead of the Scene Settings. Also make sure you have cycles selected as the render engine instead of eevee
- **2024-03-07** `coarforge` [💬╎general-help] I also have model export in infiniteExplorer, which exports some material parameters too (though HIRT I think also does rigs iirc, so it might be better to use that for now). If you open a mode (or sbsp) and go to the mesh viewer tab, you can click on `add all Meshes to Exporter` (or go through individual meshes if you want to). Then, under `Tools -> Export current scene`, you can export those models, which will also export some material parameters in a json file in the same directory you save the dae in (some info is currently still missing regarding material styles, but all parameters from the material and shader variant should be there).
  Using `Tools -> Export Bitmaps for current Scene`, you can also export all bitmaps used by materials used by currently loaded models. 
  I know it's unfortunately not very intuitive to use, I'm just really not a UI designer
- **2024-03-08** `goajango` [💬╎general-help] Hello, i have download the spartan pack 3 of this video (https://www.youtube.com/watch?v=LdmoBY08LTM&t=69s), and I would like to export it to 3ds max. I tried to export it to fbx and obj but the textures are not implemented in 3ds max. Does anyone have a solution ?
  [embed] Adding Custom Camo to your Spartan (Halo Infinite Spartan Pack 2) https://www.youtube.com/watch?v=LdmoBY08LTM [Link to over 1,000 greyscale camo patterns in description]
  
  [NOTE] For new versions of the Spartan Pack you need to duplicate your camo plugin nodes. If you connect 1 camo shader node to al
- **2024-03-08** `frostbitecinematics` [🔍╎texturing-and-shading] In Chunch's halo infinite HIMS shaders, I came across this here which controls the BotColor , MidColor and TopColor blending. Does this serve a purpose or can it be skipped when recreating the shaders in other software? AFAIK the colour can be swapped out by a gradientmap+curve-based camoflague setup but it doesn't require this BotMidTop blending to work afaik. So i'm just wondering what other setups DO rely on this, or whether I can just ignore it and only have 1x colour param per-zone?
  [attachments] image.png
- **2024-03-08** `frostbitecinematics` [🔍╎texturing-and-shading] I know the functionality is in there at the very least for game accuracy since I've seen references to the BotMidTop Color params in some of the material py scripts (in Olympus maybe?). Idk what it gets used for in-game though. It seems redundant because the gradientmap+curve-driven approach gives you the exact same capabilities right? Only difference being ofc that the curve approach lets you use "unlimited" colours, vs BotMidTop which only lets you blend between 3 colours. But if you just add 3x colours to a curve, is the output of that setup basically identical and therefore interchangeable?
- **2024-03-09** `celestial_pasta` [🔍╎texturing-and-shading] I downloaded the Halo Infinite Weapons Packs, the models work fine but none of the textures seem to have a colour, they all appear metallic. I haven't a clue how the shader setup works, maybe someone could explain why it's not working?
  [attachments] image.png, BR_textured.png
- **2024-03-10** `coarforge` [📜╎ripping] A quick script to set up materials from an infiniteExplorer export. You need to load the script in blender and run it in the editor after putting in the correct paths at the top. With my test shader, you also need to append the material `testmat_color` to the file first (that shader is everything but correct). I haven't written any documentation for the shaderdb file yet, but basically, it maps material parameters given in the exported json to parameters in the blender material (currently, only image textures, UV map indicies starting at 0, booleans indicating if UV3 (UV2 when starting at 0) should be used and float values are supported, I haven't added colors yet). 
  If there are multiple shaders listed, the script calculates a score for each shader by incrementing the score for each parameter the shader uses that is present in the material by one and decrementing it by 2 for each parameter the material doesn't have but the shader has if it's not marked as optional. If the score is 0 or lower, the shader isn't used. This is far from a great way to do this, but it was quick to do.
  There's also currently no material style info being exported by iE yet, but at least for maps, not too many materials seem to have it anyways from what I've seen
  [attachments] matsetup.py, shaderdb.json, mat.blend
- **2024-03-21** `mutespybird` [🔍╎texturing-and-shading] Does anyone here know how to get Halo Infinite base colors for stuff, I want to port stuff into Unity but have no idea on how to get the actual base textures (excluding normals, and the usual roughness and metallics) into Unity cause to my knowledge there’s no shader setup that supports the way Infinite does textures
- **2024-03-21** `chunch` [🔍╎texturing-and-shading] https://github.com/R93Sniper/HaloInfiniteVRCShader
  [embed] GitHub - R93Sniper/HaloInfiniteVRCShader: An open source unity shad... https://github.com/R93Sniper/HaloInfiniteVRCShader An open source unity shader designed to emulate the look of Halo Infinite's shader system - R93Sniper/HaloInfiniteVRCShader
- **2024-03-23** `kelcken` [🔍╎texturing-and-shading] That makes sense, but the issue I've had is that the details only seem to show up on the normal maps (when I switch the outs to metallic, rough, etc to bake, the carbon fiber details won't show up), and the normals don't seem to make it distinct enough, so that's why I was trying to bake them into the diffuse
  
  Ended up rebaking all of the normals with some tweaked settings. It's not *quite* where I'd want it, but it's much better, so I'm gonna run with it for now. May manually add the fiber texture with photoshop based on the normal. Will be a bit of a chore (30 color options, I'll have to do it for each), but it'll get the job done
- **2024-03-29** `zadeinator` [🦴╎animation] I honestly did some bootleg shader stuff and a boolean modifier for mine
  
  https://discord.com/channels/819709630540021810/830517755157020692/1221320525134172220
- **2024-03-29** `hydranaught4444` [💬╎general-help] doing shader stuff in blender is always fun ik how to do it in 3 but then i feel like a caveman in blender https://discord.com/channels/819709630540021810/832145913177702470/1028805622008909915😂
  [embed] Discord - A New Way to Chat with Friends & Communities https://discord.com/channels/819709630540021810/832145913177702470/1028805622008909915%F0%9F%98%82 Discord is the easiest way to communicate over voice, video, and text.  Chat, hang out, and stay close with your friends and communities.
- **2024-03-31** `chiefster_4014` [💬╎general-help] When you export as FBX from Blender, make sure to tick the Limit To Visible Objects box so only the armor you want exports.
  Also, textures will usually never import on their own, especially since the spartans have a custom nodegroup for their textures to work. You'll need to find a shader for Unity made for the textures.
  [attachments] image.png
- **2024-03-31** `zadeinator` [💬╎general-help] https://www.youtube.com/watch?v=MGIBLPIz4oM
  
  ngl man this was the second result when i googled "texturing models in unity"
  [embed] Unity Tutorial 4 - How to apply materials, shaders and textures to ... https://www.youtube.com/watch?v=MGIBLPIz4oM This video tutorial explains how to create materials in Unity so that you can change the colour of 3D objects. The video also explains how to use shaders and image textures textures in 
- **2024-04-05** `dg5312` [💬╎general-help] ```maxscript
  -- Create a box object
  boxObj = Box()
  
  -- Create a Standard material with default properties
  mat = StandardMaterial()
  mat.name = "Default_Material" -- Set material name (optional)
  mat.diffuse = color 128 128 128 -- Default diffuse color (gray)
  mat.specular = color 0 0 0 -- Default specular color (black)
  mat.glossiness = 20 -- Default glossiness (20 is the default in 3ds Max)
  mat.opacity = 100 -- Default opacity (100 is fully opaque)
  mat.selfIllumColor = color 0 0 0 -- Default self-illumination color (black)
  
  -- Assign the material to the box object
  boxObj.material = mat```
- **2024-04-16** `shady.pl` [🔍╎texturing-and-shading] Ok basically to people who havent read #💾╎blender recently, I cannot for the life of me figure out how to fix the textures for the DMR from the reach UNSC Weapons by chiefster, basically there is a really bad seam and something up with the thing and thanks to ALG we're basically at this point which the issue seems to be the material/textures in the files for the DMR
  [attachments] DMRHELP.blend
- **2024-04-25** `gameoverload` [💬╎general-help] I would watch for anything license wise when it comes to using existing packages by the way.
  
  Other than that, typically it goes:
  DX11 and lower - HLSL
  DX12 and higher - DXIL / SPIRV
  Vulkan - SPIRV / GLSL
  OpenGL - GLSL
  
  SPIRV usually can be used as a medium for compiling down to many different hardware specific types from your source shader data.
  
  If you do go with Vulkan, which is a recommended suggestion for PC, know that it is basically OpenGL but better basically, especially considering how old OpenGL is. Things needed a bit of a refresh eventually.
  
  For a game reference for each of these:
  DX11 - Dead Space Remake
  DX12 - Halo Infinite
  Vulkan - Doom Eternal
  OpenGL - Doom 2016
- **2024-05-05** `xbitxblurryx` [💬╎general-help] so im working with a toon shader and using halo helmets as a starting point. I need some suggestions on how to make it look better, because to me it feel off to look at because of it looking a bit jagged here and there rather than flat.
  [attachments] Screenshot_9.png, image.png
- **2024-05-05** `setht1204` [🔍╎texturing-and-shading] https://www.youtube.com/watch?v=y_lRfuCg1NA Seems to be the resource we're looking for. I don't believe there's a way to just toggle different coatings on and off - it needs to be done through the Shader Editor. Little tedious but it's easy enough.
  [embed] Blender basics for making your own Spartan https://www.youtube.com/watch?v=y_lRfuCg1NA This video goes over the very bare bones knowledge needed to have fun playing with cross core ideas and making fun coatings for your dream Spartan in Blender. This will be a very beginner friendly tutorial as well
- **2024-05-17** `trisckit` [🔍╎texturing-and-shading] okay i forgot i was posting in animation and there's a whole channel here if someone else would know
  this halo reach warthog looks gr8 in cycles with the exception of these metal bits on the turret (looks fine in eevee/material view) , not sure what the cause is
  [attachments] image.png
- **2024-05-18** `chiefster_4014` [💬╎general-help] you're baking it incorrectly. you need to use the Color/Metallic etc. outputs on the shader to bake by plugging them into the Material Output. Once you do that you need to set the Bake Type to Emit.
  For normal maps you'll just want to connect the green BSDF output into Material Output, then set the bake type to Normal. Though before baking you will also have to duplicate the mesh, go into the Object Data Properties and under Geometry Data hit Clear Custom Split Normals Data as that can cause the normal map to not bake correctly.
- **2024-05-20** `spartanatic` [💬╎general-help] ```Make your Halo Infinite Spartan in Blender```
  Tutorial by Plastered Crab
  https://www.youtube.com/watch?v=y_lRfuCg1NA
  
  Posing Tutorial by Plastered Crab
  https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] Blender basics for making your own Spartan https://www.youtube.com/watch?v=y_lRfuCg1NA This video goes over the very bare bones knowledge needed to have fun playing with cross core ideas and making fun coatings for your dream Spartan in Blender. This will be a very beginner friendly tutorial as well
- **2024-05-24** `gameoverload` [💬╎general-help] Easiest thing to do is set up via CR3B tool, and then unpack the textures from there, and use the texture paths in the shader as reference to the texture files themselves, but you can also view the textures in the image viewer in Blender as well.
- **2024-06-05** `chiefster_4014` [🔍╎texturing-and-shading] Infinite's spartan materials are procedural so there's only masks. There's no albedo, roughness, etc. until the masks are used & combined inside a shader. to get what you need you'll have to bake out the albedo, roughness & such from the shader in Blender.
- **2024-06-08** `dyland64` [💬╎general-help] Looking for an H3 Blender Shader Setup. Something to accurately represent objects, levels, etc 
  I recall there being a blend file with some to pick from but I've had a hard drive failure since downloading it and can't seem to find it anymore.
- **2024-06-13** `rook` [💬╎general-help] Hey folks; doing some work for a friend of mine getting his Spartan into Unity/VRChat; it seems to use a lot of shaders/nodes inside of Blender (Named Halo Infinite Shader 3.0) rather than textures (I.E the ones I unpacked). Is there a way to get these packed into reglar PNG files to pull them into Unity? I've tried Baking but it just says that  "No valid selected objects"
- **2024-06-16** `cythecy_` [🔍╎texturing-and-shading] Just got here after the server was suggested by NotPipi.
  
  I've been successfully pulling models through Reclaimer and it brings along the TIFs for the different image maps. I'm trying to port them into Blender using the AMF importer, successful, but I'm having issues figuring out how to properly set up the shader graphs for various models. CE seems relatively straight forward but later games have different detail maps and such and I just can't figure out how to get it done correctly. I did Boneyard from Reach and the ground looked completely wrong just mixing the different maps together with the correct scaling on each.
- **2024-06-17** `gameoverload` [🔍╎texturing-and-shading] The mod tools for MCC, which are free by the way, are very clear on how stuff works, materials and shaders included, literally the source shader code can be viewed in the tools.
  
  Now the real question is which game are you particularly pulling from?
- **2024-06-25** `cythecy_` [🔍╎texturing-and-shading] Just want to double-check, I'm working with Reach assets in Blender.
  Can I assume that all the environment mapping for a single level would be the same?
  So same cubes, tints, roughness?
  I'm using the shader graph found in the Falcon asset, credit says @Soul made it
- **2024-06-28** `lt_crunch236` [🔍╎texturing-and-shading] I hope this is the right place for a question like this, but does anyone know anything about H3EK not letting you change shaders for existing render models? I am duplicating the MP Chief biped to have two different color schemes. But I can’t have the different color schemes because they share the same shader which is locked to the render model. Thanks again.
- **2024-06-30** `chiefster_4014` [💬╎general-help] im not sure what you mean by "node blend". if you're talking about the textures they're just in one of the files that came with the Reach pack.
  yes, you'll need a shader for it to work in Unity. here's one, it says it's for h3 but should work the same for most reach assets.
  https://discord.com/channels/819709630540021810/821051966058004501/881146101632213032
- **2024-07-05** `ifhq` [💬╎general-help] I'd like to point out that this helmet has an Identical Node setup to the CQB helmet from a reach spartan pack found here, and there it works just fine, but my baking tool DOESNT work in that one, so im stuck between the shader not working in one setup, but the baking tool works fine, and the shader working in another, but the baking tool throws up a bunch of errors and doesnt bake anything properly
- **2024-07-08** `smialy_1926` [💬╎general-help] anyone know how bungie got their teleporter to look like a volume shader? i have the file for the energy beam but it's only the one on the top, the one thats really bright at the bottom isnt there.
  
  they have to be different because it looks so perfect form the top while the other energy beam is clearly just panels stacked next to each other.
  i just want to know for sure how they did this trick
  [attachments] image.png, image.png, image.png, image.png
- **2024-07-16** `cythecy_` [💬╎general-help] (realised I posted in the wrong chat)
  Bit of a rookie with Blender. Does anyone know how to deal with these black patches?
  Models are ripped straight from Halo 3/ODST, shaders are copies of what's available in the asset packs but I'm trying to move everything over to a single Blender file.
  [attachments] image.png, image.png, image.png
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
- **2024-08-06** `r93_sniper` [Blender .max Importer] Tool to import .max files directly into blender
  https://extensions.blender.org/add-ons/io-scene-max/
  [embed] Import Autodesk MAX (.max) https://extensions.blender.org/add-ons/io-scene-max/ Import 3DSMAX meshes & materials
- **2024-08-07** `reallocal` [💾╎substance-suite] hey everyone, how should i go about setting up halo infinite textures in substance painter? i'd like to port some infinite weapons to a unity game but i'm unable to use the VRChat unity shader as the game requires a specific shader be used for lighting reasons, so setting them up in SP then exporting from there with the export profile intended for that shader works out best for me. i'm just stuck on setting up textures
- **2024-08-07** `reallocal` [🔍╎texturing-and-shading] hey everyone, how should i go about setting up halo infinite textures in substance painter? i'd like to port some infinite weapons to a unity game but i'm unable to use the VRChat unity shader as the game requires a specific shader be used for lighting reasons, so setting them up in SP then exporting from there with the export profile intended for that shader works out best for me. i'm just stuck on setting up textures
- **2024-08-16** `cythecy_` [🔍╎texturing-and-shading] I'm using the HRCategory: cook torrance node group that seems to be in quite a few different assets posted here for Reach stuff.
  As far as I can tell this whitish glow is coming from the glancing specular, but I'm not finding anything in Foundation that has that same name, can anyone help figure out what values I'm meant to be using here?
  [attachments] image.png
- **2024-08-21** `brooen` [Halo 5 Data Importer] This is a Blender addon that reads halo 5 tags and imports them
  https://github.com/Brooen/Halo-5-Data-Importer
  
  **Current Tag Support**
  .material
  .structure_lights
  [embed] GitHub - Brooen/Halo-5-Data-Importer: Tool that reads halo 5 .mater... https://github.com/Brooen/Halo-5-Data-Importer Tool that reads halo 5 .material files and imports them to Blender - Brooen/Halo-5-Data-Importer
  [attachments] image.png
- **2024-08-26** `masterj2001` [🔍╎texturing-and-shading] I am insanely biased with this recommendation, but if you're looking to build your own textures from the ground up for the spartan armour, I have a 2 part Quixel Mixer tutorial that explains how to make textures for spartan/Halo armour specifically: https://www.youtube.com/watch?v=eYQpmpadpPs
  [embed] Mixer - Master Chief Helmet Texturing Tutorial | Episode 5 | Textur... https://www.youtube.com/watch?v=eYQpmpadpPs In this episode, we import our Master Chief Helmet model from Blender into Quixel Mixer and set it up with our previously baked normal, ambient occlusion, and material ID bakes.
  
  We'll 
- **2024-09-02** `gameoverload` [🔍╎texturing-and-shading] @Masterj2001 So I had a thought about making just a custom node instead of trying to make an entire custom build of Blender, especially as I could more easily add to that, and also add some features as well honestly.
  
  Either just a custom normal map node, or a custom principled BSDF that integrates the normal map stuff into it directly.
  
  It won't fix the cycles rendering stuff, but it at least makes sure anything normal map wise in the shader is dealt with, aside from the bump map correction which is a settings toggle rather than anything in shader.
- **2024-09-12** `wraith0912` [🔍╎texturing-and-shading] toon shader: https://discord.com/channels/819709630540021810/1151332408298320013/1151332408298320013
  H2A converter: https://discord.com/channels/819709630540021810/1127464384785031230/1127464384785031230
- **2024-09-19** `cwallen` [💾╎substance-suite] any ideas as to why substance might be exporting a blank specular for only one of my texture sets on a model? they all come out fine and then there's just the one that's black. it still has the alpha though? its definitely working fine on the model in substance. just not keeping any info on the export. wondering if it's something related to the halo 3 shader? @JackOfChaos
  [attachments] image.png, image.png, image.png, image.png, image.png, image.png
- **2024-09-30** `frankr2994` [📜╎ripping] Couple questions, do we have any sort of resources that say what some of these different texture maps are? One of the things that I couldn't figure out last night what was what a macro color map was. 
  
  Second question was what all plugins are available for reclaimer? I can only find mentione of architect and nothing else.
  
  Oh and I guess I do have a third question. How are we reading the shader code? Or is that all part of your development process and a mile above my pay grade?
- **2024-09-30** `chiefster_4014` [📜╎ripping] 1. There's #📑╎control-map-guide, though it only describes the main texture types. For the ones that aren't known you can figure them out by looking through the shader code (if it's available) or experimenting.
  
  2. I'm not sure, though I think that's the only one that was made. If you're just ripping from the games then Reclaimer should come with all you need.
  
  3. Shader code can be taken from MCC's mod tools (CE, H2, H3/ODST, HR, H4, H2AMP) or can be extracted from the game (CEA, H2ASP). For H5, Infinite and any others, it's currently not possible to get the shader code.
- **2024-10-01** `frankr2994` [📜╎ripping] Briefly looking I'm surprised at the amount of information and tools available for shader disassembly . 
  I really do need to stop going down rabbit holes though. I'm supposed to be setting guns up in unreal for a game mod that I already have ported in 😅. 
  
  At some point I will be more curious of the processes used in the various model ripping programs that we have for halo. I am noticing that with h2a and h4 any material using multiple textures or having a texture coordinate map will not pull over into blender. It would be truly awesome to get something sorted out to automatically pull some of the more advanced materials in at atleast a basic layout to aid in transfer to unreal for me. My material building knowledge is low so having a reference in blender is basically a requirement.
- **2024-10-14** `gameoverload` [📜╎ripping] I can't give you one right now, but its pretty simple thing of going to the material your dealing with in Foundation, look at the shader it references, then go find that in Foundation, and open that shader ( Make sure your in Expert Mode ) then you can see the shader code for that shader from there.
  
  The complexity comes with replicating that material in your program of choice with the shader code you see.
- **2024-10-14** `gameoverload` [📜╎ripping] Well its all depending on the shader and how its built, no one example will work for every one, if you need a DX source code reference, here is the microsoft learn section for all of that information.
  https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl
- **2024-10-28** `purplmunkii` [💬╎general-help] https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2024-11-05** `winnower` [🔍╎texturing-and-shading] I dont think there "standardised" one simply depending on the models like spartan armors or props and npcs have different shaders cuz it suits them better but for what I have seen every spartan pack usus the same someone did a good shader so that is the standar one
- **2024-11-17** `spartans384` [💾╎substance-suite] So I'm gonna hope someone's encountered this issue before. I erased some clasps (normal mapped) on this Spartan forearm, but it's left this smooth area that sticks out among the untouched portions of the normal map, which have a bit of graininess to them. Is there any way to blend this together to make it pretty much disappear?
  [attachments] Normal_map_blending_2.PNG, normal_map_blending_setup.PNG, Normal_map_blending.PNG
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
- **2024-11-22** `failedmango` [🔍╎texturing-and-shading] Im wondering if someone can explain something with a normal map to me, perhaps I'm doing something really weird.
  Im working with the H2A Odst model and I'm baking the material into mesh for the purpose of 3d printing using subdivision mult and displace following this tutorial: https://www.youtube.com/watch?v=AOdpGgqIub8
  but the edges are really dull/inconsistent.
  Is it possible to sharpen the edges in the normal map to make it *cut* deeper into the mesh?
  [embed] Convert Normal Map to Geometry (for 3D printing or sculpting) https://www.youtube.com/watch?v=AOdpGgqIub8 If you have a model that fakes detail using a Normal map, it can be interesting to convert it to ‘real’ geometry that you can continue sculpting on, or for use in 3D printing. This shows you how
  [attachments] image.png
- **2024-11-28** `gameoverload` [🔍╎texturing-and-shading] Yes, also the masks setup ( using a split color ), is R - emissive, G - Specular Power ( Gloss ), B - Specular Intensity
  
  Meaning for Specular Power, using a math node set to subtract, with A as 1 and B with your specular power input into that, and then input that into roughness.
  
  For Specular intensity, use a map range set to from min .5 and from max 1, then input the result into metallic. However also, take the result of the map range node, put that into a mix set to float, as the factor, with A as .5 and B as 1, and connect the result of that to IOR Level in the Specular category.
  
  For emissive, connect that to a math node set to multiply, with the B as your emissive intensity, and connect the result of that to the emissive strength.
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
- **2024-12-22** `den4iktea` [💬╎general-help] Hi everyone, I’m experiencing an issue with shader compilation in Halo Infinite on my PC. The process gets stuck at 10% and never completes. I’ve tried updating drivers, verifying game files, reinstalling the game. Does anyone have a reliable solution for this issue?
- **2024-12-31** `8_pin_eren` [💬╎general-help] Hi yall, nice to meet ya! Dunno if the question has been asked yet, but is there a way I may be able to convert the WH40K Space Marine 2 textures from Strix's weapon pack to PBR correctly? Seems like my normal maps haven't worked correctly even after inverting them in gimp to attempt to make em Open-GL, and the metallicroughness are way too light? 
  
  I've had to resort in attempting to bake the textures and got some progress, but they just don't have the same quality as the textures with the 40K shader included in the .blend file unfortunatley. 
  
  I gtg to sleep rn so I'll be unable to respond quickly unfortunatley, but if someone has answer or method on how I could fix the Space Marine 2 textures correctly, or how to bake them right, that's be pretty baller!
- **2025-01-01** `chiefster_4014` [💬╎general-help] for H4's spartans, the diffuse is in the green channel of the control map and the alpha channel contains the color change zones. White areas are for primary color and grey areas are for secondary color.
  If you don't want to bother with messing with the control map there's a H4 shader you can get for Unity here: https://gitlab.com/FiyCsf/open-halo-4-spartan-shader-for-unity
  [embed] Cameron "Fiy" F. / Open Halo 4 Spartan Shader for Unity · GitLab https://gitlab.com/FiyCsf/open-halo-4-spartan-shader-for-unity This should play nicely with all Halo 4 MP Spartan Textures etc, and is written using the amplify shader editor and is clearly commented should anyone wish to...
- **2025-01-03** `serfais` [💬╎general-help] Questions on HIMS:
  
  1) Is HIMS worth it for rendering my infinite models, or should I just stick with the normal Halo Infinite - Spartan Pack? It's just a custom shader, right? Last time I even touched anything that rhymed with the word "shader" was for ENBs in FNV, so I'm confused lol.
  
  2) Will HIMS look weird for my Spartan if I render them in a scene with other assets that don't use HIMS, or is that shader only meant for renders of standalone Spartans? If so, is HIMS abled to be easily disabled and reenabled whenever?
  
  3) I want to put together some Infinite Spartans that use Mark-V(B), perfect for the Halo Infinite - Mark V [B] HIMS 3.0 upload, but I want to also give them some accessories from other cores, like knives, pouches, or maybe even a shoulder or two. Will this look strange if I port those accessories over to the HIMS 3.0 version?
  
  4) Might use humgen for my Spartan's faces sometime. Will it look weird if it's with armor that uses HIMS?
  
  Thanks in advance
- **2025-01-26** `gameoverload` [📜╎ripping] Generally its not unreal specific, but the general recommended workflow by Epic that most basic games visually follow is A/B, ORM, N, ala base color, occlusion, roughness, metallic, normal.
  Maybe throw in an opacity mask in the alpha of the base color, but otherwise thats recommended.
  
  The reason being alpha packed maps take up significantly more memory both to store as an asset and in memory during gameplay, however the upside is you get the least compression.
- **2025-02-09** `flipdark95` [🔍╎texturing-and-shading] Hey fellas, can I get some help figuring out how to make a shader that uses RGB masks to customize colours? I know Halo Infinite has similar shaders for customizing materials, but I'm trying to figure out how to do something similar for the Battlemechs from Mechwarrior 5. Wanted to start with the Centurion since it's the starting mech for the player in the campaign.
  [attachments] Color_Mask_Question.png, 20250209180725_1.jpg
- **2025-02-09** `chiefster_4014` [🔍╎texturing-and-shading] There's info on what the channels do on C20: https://c20.reclaimers.net/h1/tags/shader/shader_model/
  [embed] shader_model https://c20.reclaimers.net/h1/tags/shader/shader_model/ The shader_model tag is used for opaque materials on object models. It supports features like map an...
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
- **2025-02-27** `purplmunkii` [🦴╎rigging] A. You need to scale the armature not the mesh
  B. Select the armature go to pose mode
  C. Id recommend watching this: https://www.youtube.com/watch?v=Jm39-vYl1zo. Its old but still mostly relative
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2025-02-27** `masterj2001` [🔍╎texturing-and-shading] https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/17
  https://projects.blender.org/blender/blender/issues/131605
  [embed] Cycles and surface normals (normalmap shading) https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/17 Somebody else had a very similar complaint/observation to this thread.  Various parts of Cycles will use the geometric normals (or smoothed geometric normals depending o
- **2025-02-27** `gameoverload` [🔍╎texturing-and-shading] To be fair, most game engines tend to just override the geometry normals with the shader normals when setting up an object, so if you still need to use the geometry normals for something, you would do it in the shader.
  
  So its actually fine as an option, plus their drawback of 'BSDF is more than one shader' have they not heard of saving that data in a temporary vector then using it for all normal related data inputs?
- **2025-02-27** `gameoverload` [🔍╎texturing-and-shading] Genuinely, this just sounds like a bad idea, because then your wasting the use of the shader normals.
  ```
  Yes, this is a known limitation. When sampling lights we include how the light relates to the position and more importantly the normals of the mesh to help decide whether or not to sample the light.
  
  The normals we use are the normals prior to any normal mapping. Which means in your cube scene, we use the "smooth normals".
  
  This causes issue for your scene as the normals on the top of the cube "curve" around to the side of the cube, causing parts of the light to be considered "behind the surface" (because they're behind the plane formed from the smooth normals) and thus won't be sampled. This then leads to the directions and amount of light being sampled changing across the "flat surface" on the top of the cube and ultimately leads to the issues you observse.
  ```
- **2025-02-28** `masterj2001` [🔍╎texturing-and-shading] I just went and contributed my findings to the thread, so hopefully that can reignite some discussion about it: https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/32
  [embed] Cycles and surface normals (normalmap shading) https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/32 Apologies if any of my terminology is off. I’m not a technical user, but I can see the impact of this issue on my work. I create assets using a pseudo-game modelling wor
- **2025-03-03** `masterj2001` [🔍╎texturing-and-shading] @Banned .Casey Alright, so I got Gleb Alexandrov and Aidy Burrows to notice this and both Curtis Holt and Andrew Price suggested that I make a post on Right Click Select, which I have now done (yes, I have spread this thing around like wild fire, you'd never suspect that I work in marketing 😂 ) https://blender.community/c/rightclickselect/l8yj/
  
  So I think by this stage, I have done everything in my power to get this heard and seen by the relevant audiences. I'm also waiting to hear back about the MK VI GEN3 model scene being tested within this branch: https://projects.blender.org/blender/blender/pulls/135380
  [embed] Toggleable Shader Normals in Blender https://blender.community/c/rightclickselect/l8yj/ In Cycles, the handling of normal maps currently relies on a combination of mes… WIP: Replace bump correction algorithm to better respect normal maps https://projects.blender.org/blender/blender/pulls/135380 **Pr
- **2025-03-12** `chiefster_4014` [🔍╎texturing-and-shading] if it's a floating decal then you'll have to UV its texture onto the mesh, then use Mix nodes to combine it into the roughness, metallic, etc. this video should show the process fairly well
  https://www.youtube.com/watch?v=MBy62M7TcgY
  [embed] How to Overlay an Icon within a Material (Blender Tutorial) https://www.youtube.com/watch?v=MBy62M7TcgY In this Blender tutorial I will show you how to overlay an icon or image within a material, using Blender's shader nodes.
  ● Metal Texture: https://www.cgbookcase.com/textures/old-iron-01/
  ● Electr
- **2025-03-18** `walpha` [💬╎general-help] Hey, fellas. I've got a rather... interesting issue on my end. So, I made a custom coating using an Ekur export as a base. I've been trying to transfer the coating over to the Spartans in MrGruntington's pack. But... when I try to transfer the exact coating shader to the other armor, it's missing some details. I've posted some screenshots to show what I'm talking about. The first one is what it looks like on the new armor, and the second is what it's supposed to look like. It almost seems like there's some layer of detail missing, but I have no idea what could be causing it, considering I just copied the exact shader over to the other armor. Do you guys have any ideas on how to fix this?
  [attachments] image.png, image.png
- **2025-03-31** `chiefster_4014` [🔍╎texturing-and-shading] Giving the decals their own UV should let you keep the resolution high for the decals. This vid explains it well, just ignore what he does for normals unless your decals have their own. https://www.youtube.com/watch?v=MBy62M7TcgY
  [embed] How to Overlay an Icon within a Material (Blender Tutorial) https://www.youtube.com/watch?v=MBy62M7TcgY In this Blender tutorial I will show you how to overlay an icon or image within a material, using Blender's shader nodes.
  ● Metal Texture: https://www.cgbookcase.com/textures/old-iron-01/
  ● Electr
- **2025-04-13** `zangh` [🦴╎animation] as panzer was saying eevee is honestly pretty good for rendering as well these days. especially once you got the light probes figured our you can even get reflections near the level of cycles. especially with some good shader work in, it can make the Reach models pop. left has custom shaders I made, right is vanilla Reach shaders
  [attachments] image.png, image.png
- **2025-04-16** `taggynn` [💬╎general-help] Might anyone know where I could obtain / make baked out HDRIs of the Reach Skyboxes?
  Working on a VRC world in Unity and I'd move to have as close as I can get to the OG Reach sky (specifically trying to mimic the atmosphere around Visegrad Relay) 
  I know Halo uses a lot of meshes with effects / shaders / textures for the skybox, but those effects aren't so easily mimicked (in an optimized fashion) to my knowledge, so if anyone has a method or premade skies, that would be so awesome
- **2025-04-23** `katratatat` [💬╎general-help] Heyo, I've been trying to track down certain bump maps for an art project, i'm going to make a t-shirt and bag from custom printed fabric, and i've been huntign for the bump / bit map for the banshee (any will do but preferably halo reach or 4) I've been skimming through here but having a really hard time finding it
- **2025-05-06** `matrixbearer` [🔍╎texturing-and-shading] hiiii is there anyone I can ask for help with shader stuff? Cuz rn I'm trying to translate the halo reach pack textures and shaders into Mtoon material(because funny vrm spartan OC dancing caramelldansen with MateEngineX) but I'm clueless on what to do, should I attempt to bake the textures or just connext some things around?
- **2025-05-13** `gameoverload` [📜╎ripping] If it is for an existing game and not an engine tool, then you would need to bake the material down using the HIMS shader outputs, to get out appropriate textures you may need.
  
  If your in an engine tool, an option also is to recreate HIMS in that engine tool, if you go down that route instead.
- **2025-05-16** `gameoverload` [🔍╎texturing-and-shading] I am going based on the tool rips for now, the problem is they do a bunch of little stuff internally per what tjoener has found that it is honestly not practically worth it to implement every single little thing they do per asset, not so much shader stuffs, but like the per texture adjustments they make.
- **2025-06-05** `brooen` [Halo 4 Material Importer] Imports Halo 4 .material files into Blender
  
  https://github.com/Brooen/Halo-4-Material-Importer
  [embed] GitHub - Brooen/Halo-4-Material-Importer: Imports Halo 4 .material ... https://github.com/Brooen/Halo-4-Material-Importer Imports Halo 4 .material files into Blender. Contribute to Brooen/Halo-4-Material-Importer development by creating an account on GitHub.
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
- **2025-06-23** `crystals_den` [💬╎general-help] For coloring, where there's the line of 4 circles near the top right, go to the 3rd from the left, which will apply all the material shaders and stuff
  Then if you want to change the color of things, go to the "shading" tab in the very top bar (or the + > General > Shading if it's not there already), and if its a model from somewhere else, there's usually already color node trees to play around with (some have drop downs for what actually is the object color)
  And if it's your own model, there's a bunch of easy tutorials you can find for changing the material settings there, and the default node setup it spawns if you make a new material is usually fine for most purposes
- **2025-07-06** `solipsol_studios` [💬╎general-help] Since theres so many knowledgeable people in here -
  
  Does anyone know why so many older games like Halo 3, that relied more heavily on the use of normals to imply detail - look crisper, sharper, just generally clearer than modern games that demand every single bump and crevice detail must be actually physically modelled, like Halo Infinite?  Especially when it comes to the textures.  HI gets really blurry compared to H3's clarity.
  
  This phenomenon really stuck out to me when i played Black Ops 2 right after playing BF2042.  In comparison, BO2 was a lot clearer and clean than 2042, which was super blurry in a ton of situations, just like HI.  But in BO2 the model for Harper when in the rain was so damn good, wet and glossy and you could really see every single bump and divot on his uniform thanks to the normals (it seems), it really did look better than 2042.
  
  I dont get why the new games look worse.  : \
- **2025-07-06** `badgerpants1872` [💬╎general-help] Messing with the Infinite prosthetics for use in VRC. Effectively the goal is to wear them like a pair of pants and just alpha mask out the base mesh on the shader side, so it's going to take a good bit of deformation until it fits right.
  
  And I've very quickly run into an issue (feature?) I've never seen before, where half the meshes seems to have all of their transform controls tied to a little orange dot in between the feet.
  This has the effect of causing things like upper arm mesh to swing around wildly instead of rotate in place.
  If someone knows how to go about removing this, what it is, docs to read up on, I'd appreciate it. Thanks.
  [attachments] image.png, image.png
- **2025-07-13** `chunch` [🔍╎texturing-and-shading] This file has a proper doom the dark ages shader
  https://drive.google.com/file/d/1uWTj_Gv3xhJ3RIpDarfyx3-pWYl-sQdM/view?usp=sharing
  [embed] TDA - Doom Slayer.zip https://drive.google.com/file/d/1uWTj_Gv3xhJ3RIpDarfyx3-pWYl-sQdM/view?usp=sharing
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
- **2025-08-05** `frankr2994` [📜╎ripping] Not that I'm aware of. And I think for something like h3 reclaimer is the only option. For something like h4 you can use reclaimer and or foundry . They work a little different both being good tools. But typically if I have all my geometry and materials I'm happy....so someone else might have better input
- **2025-08-05** `krystalize` [📜╎ripping] ok, 2 questions: 
  1. Are you the person who made a Halo 3 shader file for blender? I saw a video from Plastered Crab and he linked a shader file but the google drive link is dead, but he named someone call Chief
  2. the pre-placed obj's does importing the files into blender automatically put them into their place or is this something that is needed to be done manually?
- **2025-08-17** `sandwichslayer14` [💬╎general-help] help how do i paint the parts in blue, every time i click them it selects the right thigh armor plate in the shader editor, please tell me what im doing wrong i will answer any questions, im using the Halo Infinite - Spartan Pack 2.0 SOLVED*
  [attachments] helpmediscordpls.png
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
- **2025-09-14** `chiefster_4014` [🔍╎texturing-and-shading] this is just due to Halo 3's lighting and shaders, and also the possibility of the Halo 3 Blender pack not shading things correctly since it's pretty old. Though to be honest, you can only get so close in Blender/Helldivers since they use completely different shaders and lighting to Halo 3.
  You may have to just edit the colors to get an approximation.
  Here are some screenshots that show Halo 3 with albedo color only vs full lighting.
  [attachments] halo3_sapien_ELXTWog8jv.png, P3eNLwacfR.png, halo3_sapien_46P70fogcg.png, POWnrCN5h0.png
- **2025-09-19** `xen0kid` [🔍╎texturing-and-shading] Heyo, recently downloaded the Halo Online weapons .blend for a mod, and I can't imagine there's a way to perfectly remove this repetitive hexagon pattern from the PP normal map. I think it's the highest fidelity reach design PP available, 2k normals. If anyone knows black magic and is willing to help clean this up, I'll owe you a soul or two
  [attachments] PlasmaPistol_n.tga, image.png
- **2025-10-08** `watermelon2364` [💬╎general-help] Bouta go rip some game skyboxes from Halo because I seriously tried so hard to figure out how the devs made the classic ring shape going into the horizon but cannot find any resources at all. My question is, are the skybox files in a cubemap or hdri format or just like impossible to extract?
- **2025-10-08** `halostrophe` [💬╎general-help] Is someone able to help me understand on adding decals to models? Particularly Spartan models?
  I've been following this guide https://www.youtube.com/watch?v=rUP8aAG4AQ8&t=150s but I mostly get stuck in connecting the decal's nodes to the main shader/texture
  [embed] The correct way to make decals in Blender (NO SHRINKWRAP) https://www.youtube.com/watch?v=rUP8aAG4AQ8&start=150 Someone made an addon based on the technique! Get it here: https://blendermarket.com/products/easy-decal--stamp
  
  It is a bad idea to use shrinkwraps for decals. I show you a more robust te
- **2025-10-09** `captain_ramlethal` [💬╎general-help] https://youtu.be/CeO1w9Qe6MY?si=FcVeLLZkoJUGIIwb
  [embed] [2020] Extract and import the Titanfall 2 models in blender | A ble... https://www.youtube.com/watch?v=CeO1w9Qe6MY This video will teach you how to extract the models, textures from the game files.
  Applying the textures in the shader tab in blender (part 2): https://youtu.be/UV3SxMEsMe0
  
  Download li
- **2025-10-10** `tim_irl` [💬╎general-help] Not sure where to post this- is anyone in need of a modified version of Reclaimer that can export .RMF files with the original lightmap UV channel intact? It allows you to keep the original baked lighting from Halo 2 when you pair it with proper shader in blender or other 3d software.
- **2025-10-10** `tim_irl` [💬╎general-help] I had to source the lightmap files using ninjaripper during runtime, but the original issue was the geometry itself doesn't maintain the extra uv channel meant for the lightmaps to be applied to if it's exported to RMF.
  There is a lot of interesting character and math that goes into how shadows are baked in H2 (and all the other games for that matter) so I wanted to make sure they could be preserved beyond just being an unapplyable bitmap.
  
  here is a screenshot of the same object with the lightmap applied to them on the uv meant for them vs. the same object with the overlay texture (Sans detail textures, shader nonsense, yada yada.) applied to the uv channel that is currently supported by rmf exporting.
  [attachments] image.png, image.png
- **2025-10-15** `dr.pwoil` [💬╎general-help] oh yup my bad, so if i remember correctly, i tried to :
  - expend the fog to the entire scene cause at first the halo in the background was not in it
  - added a second light outside at a different angle to have less shadow on the top left corner of the halo (so where i have the flickering)
  - added a transparent BDSF with a Mix shader on the window (at least of part of it since if i'm correct the top glass comes with 2 parts)
  - increased the Clip end setting on my camera
  - in the object tab, unchecked the diffuse and shadow checks, then unchecked glossy, transmission and volume scatter as well from both parf of the window in the ray visibility tab
  - Increased samples, i started at 16, tried few frames a 64, did not solved anything, went back to 32 to save some  render time
  - a bunch of other things i can't remember exactly, i was just mostly messing with shaders options but nothing worked so it's back to default
  
  I hope it's understandable, as i said, big beginner here, i'm using blender mostly as a software that allows me to play with models for scenery and animation
- **2025-10-21** `purplmunkii` [💬╎general-help] I recommend watching some general videos to get aquinted to the workspaces if you havent already.
  
  The spartan textures are in the shader workspace (top of your screen is the tab). Youll see a big node with smaller nodes attatched to it, mess with those one
- **2025-10-22** `katbug` [🔍╎texturing-and-shading] https://www.youtube.com/watch?v=LdmoBY08LTM
  [embed] Adding Custom Camo to your Spartan (Halo Infinite Spartan Pack 2) https://www.youtube.com/watch?v=LdmoBY08LTM [Link to over 1,000 greyscale camo patterns in description]
  
  [NOTE] For new versions of the Spartan Pack you need to duplicate your camo plugin nodes. If you connect 1 camo shader node to al
- **2025-10-31** `chiefster_4014` [💬╎general-help] Not really. You'll need to use the shader code from the mod tools to recreate their shaders.
  If you use the Foundry addon it'll set up all the shaders and colors automatically, though you'll need to install the Halo 4 mod tools
- **2025-11-04** `chiefster_4014` [🔍╎texturing-and-shading] If you already have it as an image then you can give it it's own UV and apply it in the material. This vid explains it well, just ignore what he does for normals unless your decals have their own. https://www.youtube.com/watch?v=MBy62M7TcgY
  If you don't have it as an image then I can't really help since I'm not good at drawing.
  [embed] How to Overlay an Icon within a Material (Blender Tutorial) https://www.youtube.com/watch?v=MBy62M7TcgY In this Blender tutorial I will show you how to overlay an icon or image within a material, using Blender's shader nodes.
  ● Metal Texture: https://www.cgbookcase.com/textures/old-iron-01/
  ● Electr
- **2025-11-05** `chiefster_4014` [📜╎ripping] If you have the Reach, H4 or H2AMP mod tools installed and extracted, you can simply drag & drop a sky's .scenery tag into Blender and Foundry will automatically set up all of the shaders for you. If you're reimporting back into the tools make sure to use the ``Setup as Asset`` option when importing.
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
- **2025-11-21** `koax117` [📜╎ripping] nearest i can tell, is that the new assets are the only ones broken, I tried setting it to the same shader as the other ones (looks like 144 new items-ish), checked the material names, and yeah, new stuff still imports black when trying to change with Ekur. og stuff is fine at least.
  @mrgruntington apologies if you're not the right one to ask, and its someone related more to ekur or the HIMS shader, but any idea why the new content breaks in ekur? Base cores are fine
  [attachments] image.png, image.png, image.png
- **2025-11-23** `reidodoreido` [🔍╎texturing-and-shading] I'm trying to add a visor pattern onto my spartan, and It's weird.
  
  I grouped the nodes I wanted, created a copy of the one I wanted while giving it a new name and label, then appended that shader group into the spartan model. When I attach the node group to the visor I get this...
  [attachments] Visor_Issue.JPG
- **2025-11-25** `chiefster_4014` [📜╎ripping] these are the nodes i use now. i plan to make them simpler in the future but for now it's essentially a bunch of nodegroups that need to be connected or added together depending on the shader values given in the Halo 3 mod tools. the result is pretty accurate to the game, but it's kinda tedious to set up. if you want i can do it for you when i have more time tomorrow
  [attachments] chiefster_h3_shader_collection.blend
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
- **2025-12-09** `gameoverload` [🔍╎texturing-and-shading] For Halo 5, last time I looked into it, no one had looked into the formats and processes used for its shaders.
  I wish I could get realtime frame info from Halo 5, but trying to link to a UWP app is not possible as far as I have tried.
  Maybe this is a time for me to finally look into Halo 5's various shader file formats and processes, best I am able to anyway.
- **2025-12-09** `gameoverload` [🔍╎texturing-and-shading] I think what would need looked , for HI, to get something more usable is two fold, is figuring out the commands they used for the compilers, the compilers which I don't know if they store those in the games files ( even just in the folders for the game ), I do not remember, and then figuring out how the engine interprets the actual shader binaries when used, if it even reads them any differently, because if not, then really, it just would require the compiler processes part, which I think was the missing factor, as I just guessed the commands they used for compiling those shaders, rather than knew what they used in the first place.
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
- **2025-12-26** `koax117` [📜╎ripping] are you referring to the nodes for the HIMS shader? because, that's pretty much standard across the board for Infinite assets.
  
  as far as the ids, some folks have been working on a list
  https://discord.com/channels/819709630540021810/819709677243858975/1388217557646971110
- **2025-12-28** `gameoverload` [💬╎general-help] Redshift has a shader graph system through redshift.
  https://help.maxon.net/c4d/en-us/Content/_REDSHIFT_/html/Cinema+4D+ShaderGraph.html
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
- **2026-01-18** `solipsol_studios` [💬╎general-help] Im just (still) trying to add a reflection cubemap onto a preexisting model. Thats all.
  
  Apparently im supposed to use an environment map node thats mapped with the reflection output of the texture coordinates node and do something with the box projection mode in the image texture node.
  
  But i have no idea what that even means in regards to adding it all to the model. I know how to add those nodes to the node area in the shading tab, but like how do i get my cubemap to apply to the model?
  
  And i dont even know if all of that stuff is actually relevant, cause im just piecing together stuff from forum answers that barely made any sense.
  
  I do know however that i need an equirectangular map, ive got that part done already.
  
  I just really need to know exactly what buttons i need to press.  : \
- **2026-01-18** `solipsol_studios` [💬╎general-help] Well i have the reflection map already, i just need to slap it onto the spartan.  In the same way that the cubemap is applied to CE Chief.  Like how its just a thing added over the texture to make it look shiney.  But im lost as to how to apply this map image to the new model.
- **2026-01-24** `koax117` [🔍╎texturing-and-shading] is there a specific shader or workflow for Combat Evolved Anniversary campaign environment assets? Is it just following Reach, or is it slightly different?
  I'm trying to follow and understand how the diffuse alpha is used versus the spec/spec alpha (when applicable), as well as the _br and _hdetm textures
  I'm using a bastardized version of a Halo 2 Anniversary shader that had similar nodes to what I *think* I needed, but I'm probably a complete fool.
  people have been requesting a number of things from CEA that I would like myself and like to try and get set up.
  any help would be greatly appreciated!
  [attachments] image.png
- **2026-01-25** `gameoverload` [💬╎general-help] So, those are just XY normals with the Z channel filled in as 1.
  You just need to take the XY normals and recalculate them if your making a shader in say unreal or unity, or if your using Blender, assuming this is meant for a spartan, then you can just load up the spartan pack that has this normal used in it for that particular piece.
- **2026-02-23** `fredxvi` [🔍╎texturing-and-shading] Question for anybody, I am using the Spartan Mega pack and trying to model a spartan. The issue I am running into is that I cannot get the iridescent almost shine to the armor in the shader path. Any suggestions? I am new to shader work, any suggestions? I am using cycles for the rendering method.
  [attachments] image.png, image.png, image.png, image.png
- **2026-03-01** `chiefster_4014` [🔍╎texturing-and-shading] the channel/name/equivalent sorting is kinda awkward to read
  h3 CC maps can use RGB but it shows it only using RG
  reach CC maps can use RG but it shows it only using R
  i think the order for the h3 layer1 map is wrong, the roughness and metallic should be swapped. also the specular, blend, environment, roughness layout next to it is only used for H3's original in-game cook torrance shader. the in-game PBR shader 343 made is just specular & roughness.
- **2026-03-01** `gameoverload` [🔍╎texturing-and-shading] If you have a reference for the first two, do let me know in that case, for the H3 Layer 1 that should be correct, but I will double check, but, even if only the specular and roughness are used by the game, I still included those unused references, in case someone had need of the unused data.
- **2026-03-03** `okom` [🔍╎texturing-and-shading] The node arrangement of the Halo Infinite Shader 3.1.2 on Spartans ripped from gamertag with Ekur looks like this. Any plans on fixing it, and aware if it's an Ekur issue specifically? Have to fish out specific nodes to find connections for modifying the output. @Chunch
  [attachments] 2026-03-03_blender-uwrf.png
- **2026-03-09** `castscarab9` [🔍╎texturing-and-shading] greetings guys! im trying to attempt a very particular look for a shader, and im stumped on how to turn off the effect or reverse what its doing. if anyone can be of assistance that would be great! 
  
  sorry for being a bit vague, will provide more details if anyone reaches out.
- **2026-03-13** `korrag` [💬╎general-help] ~~I'm having some trouble getting a model setup for use in tabletop simulator.
  
  I'm using the Reach Spartan pack and the Reach UNSC Weapon pack, trying to pose my Spartan with a shotgun and export it as a file to use as a Miniature on the board. I attempted to export as an obj and found out pretty quickly that it doesn't keep the textures and I tried opening the .blend up in unity to the same effect, I tried using the Open Halo Reach Shader for Unity by Cameron "Fly" F but I couldn't get that to actually do anything when I set the normals, diffuse and control map from the Reach Spartans pack. I read somewhere that baking the textures in blender might be a solution but I'm still figuring that out.
  At this point I'm at a loss, any help would be appreciated.~~
  
  Figured it out my own damn self, if anyone needs help porting to TTS via Blender->Unity I will help.
  
  Guide incoming sometime maybe
- **2026-03-17** `gameoverload` [💬╎general-help] Okay, I was mainly making sure you weren't using a Mac machine, as those have lower sampler limits, as one issue can occur on Mac where because of the lower sampler limit, not every texture is loaded.
  
  Also to change the material properties, you go into the shading tab, click a particular mesh, and then you either use the side properties window, or the middle shader editor window to go in and change the material properties.
- **2026-03-19** `dodopro444` [🔍╎texturing-and-shading] Im trying to make the jet engine look that you see on the pelican, and my maps and whatnot are done, however when i move the bones on my rig, for some reason the object moves but the fire effect doesn't move with the object. from what i can tell, the fire effect stays in place and as i rotate the wings and whatnot, it stays while the box leaves it behind when moving. It's done on a shader, face orientation is fine, my maps attached mb for the blurriness hope someone could help
- **2026-04-11** `koax117` [💬╎general-help] @Plastered_CrawDAD 
  weird bug with Cr4b Tool i've run into. For whatever reason, running the tool for the shaders on the Ark messes up the material for this panel. 
  I thought for the longest time it was an incorrect texture issue, but i double checked it, and the UVs, which didn't change.
  don't know if you or anyone else would have insight on this.
  this is from H3, The Covenant, bsp_050
  asset - structure_catwalk_control_lift
  shader - panel_glass
  [attachments] image.png
- **2026-04-18** `javenator` [🔍╎texturing-and-shading] Does anyone have a method for saving and transferring heavily modified coatings using the Halo Infinite Shader? I was loading a coating with Ekur and modifying it piece by piece for multiple characters, but recently realized I could pull everything but the base textures between characters for each region of their armor. I also tried editing the style json for a custom coating but it seems like Ekur won't load that unless I am doing it wrong. Just wondering if anyone has a faster method they are using.
- **2026-04-24** `winnower` [💬╎general-help] 1 — Most of those armors are from different games, so the textures and material setups are different. So yeah, you kind of have to eyeball it.
  
  2 — Both Arbiter armors (Halo 2 and Halo 5) weren’t made to be colorable, so you can’t really recolor them properly, it’ll look wrong unless you do custom work.
  
  3 — Jega’s armor works like any other Halo Infinite character, you just need to find the right materials and color them.
- **2026-05-01** `papyrus95` [🔍╎texturing-and-shading] Hey there, I was wondering if anyone could help me figure out something regarding baking in Blender. I have retextured the Helldivers 2 MA5C using the shader converter found on @squeak_grenade (#1 SPI Fan) (Sorry for the ping) Halo Infinite Reach ODST Kitbash. I have already baked color, normal, roughness, and emission, but I was wondering how to bake a metallic map with this shader
- **2026-05-13** `koax117` [🔍╎texturing-and-shading] if you're working with the Halo Infinite Spartans, objects like helmets or the thighs will often have multiple material slots (i.e. odst helmet, odst visor). 
  From there, the different nodes in the shader editor have the colors inside them. Select the node tab, or expand the node to edit the color settings.
  [attachments] 20260513-1855-50.5471221.mp4
- **2026-05-22** `chiefster_4014` [🔍╎texturing-and-shading] What each channel of the multipurpose map does depends on the settings set in the material's shader tag. That one appears to be in the Xbox channel order, so the red channel should be the detail mask, green channel is the self Illum mask, and the blue channel should be the reflection mask. This page should describe the shader tag for this material in more detail if you want to be accurate to CE: https://c20.reclaimers.net/h1/tags/shader/shader_model
  
  I can't recall exactly the setup to use the reflection map exactly, but I know the texture itself must be used with the Environment Texture node instead of the normal Image Texture node so it reflects properly. Iirc for the rest you just need a Geometry Node and Vector Math node set to Reflect. The Incoming and Normal outputs of the Geometry node go into the A and B input of the Vector Math node. The output of the Vector Math node then gets put into the Vector input of the Environment Texture node.
  
  I can send a visual example when I'm home, but that'll be over an hour from now.
  [embed] shader_model https://c20.reclaimers.net/h1/tags/shader/shader_model The shader_model tag is used for opaque materials on object models. It supports features like map an...
- **2026-06-03** `koax117` [💬╎general-help] this was utilizing Foundry's Halo Reach halogram shader. (note that its multiple materials as well, so, that is something to check for the CE version)
  was a long arduous process of locating the tags (in my instance from H3) and copying them into the Reach tags to import via foundry into blender. 
  chiefster's got an updated for H3 that i am working on for my stuff though.
  i don't know how the CE halogram shaders are handled, if any differently. the only other thing I could suggest is either 
  A- install the CE mod tools if you haven't and check the tags for that shader
  B- see if there are any other CE accurate holograms floating around that are set up right to use as a base
- **2026-06-03** `koax117` [💬╎general-help] I don't have the CE mods tools installed, so I can't vouch for how any of it works sadly.
  For when I'm working in H3, I load up Guerilla and will navigate to locate the object's render.model / relevant shader. if there's any textures missing that didn't rip with the model, that's where i'd go to look for it.
  [attachments] image.png, image.png
- **2026-06-03** `roman117` [🔍╎texturing-and-shading] I'm trying to add decals to a model from the Halo Infinite Spartan Pack, and I'm using this guide for it using the first method, but I'm stuck at the part where he says to "mix into the base color"; there is no "base color" node anywhere that I can find.
  
  I'm completely new to blender so I'm probably missing something obvious...
  https://www.youtube.com/watch?v=TIkY5SFAusU
  [embed] 4 Methods for Adding Decals in Blender (Tutorial) https://www.youtube.com/watch?v=TIkY5SFAusU In this Blender tutorial I will show you 4 different methods for adding decals to your objects: Shader Nodes, Shrinkwrap Modifier, Stencil Brush, and Decal Master Addon.
  
  ▶️ Decal Master (Affiliate Link): h
  [attachments] image.png
- **2026-06-03** `koax117` [💬╎general-help] i'm still working through piecing together a good system for doing looping animated textures, but this is the system I have going from a tutorial I followed, it might give you some ideas since big beam go brrrrr. Chiefster was also a lot of help with this because its running off their H3 shader and they cleaned up the loop a bit more.
  https://youtu.be/0uMWdlz1ecc?si=8TEIpJrL0cjYh3sf
  Based on what I've seen from working with the H3 assets at least, they usually have two different noise masks they animate. that'll potentially help give you a lot of that subnoise in the beam as well.
  [embed] Looping Texture Animations in Blender - Comfee Tips https://www.youtube.com/watch?v=0uMWdlz1ecc Animate any texture's location, rotation and/or scale in a seamless loop. Stylized water shimmer bonus tutorial as well!
  
  Psalm 23:2-3 "He makes me lie down in green pastures. He leads me by still waters.
  [attachments] 20260603-1944-52.0056414.mp4
- **2026-06-04** `chiefster_4014` [🔍╎texturing-and-shading] Trying to get Halo mats to look good in stuff that can't be given custom shaders is a pain for me. This looks pretty close though.
  Giving the diffuse for the armor material a slight blue tint might help bring back the blueish specular color.
- **2026-06-04** `koax117` [🔍╎texturing-and-shading] ah. I *attempted* to take a stab at a Shadow of War/Mordor shader a couple years back.
  there was an article on facepunch back in the day i have bookmarked that broke it down a little bit, but it looks like that site is gone now. You can read a little bit on it on the waybackmachine, but I don't know if it'll have all the information you might be looking for.
  https://web.archive.org/web/20210731210806/https://forum.facepunch.com/t/shadow-of-mordor-ripped-models-please-share-if-you-have-them-in-any-kind/227892
  [embed] Shadow of Mordor ripped models - please share if you have them in a... https://web.archive.org/web/20210731210806/https://forum.facepunch.com/t/shadow-of-mordor-ripped-models-please-share-if-you-have-them-in-any-kind/227892 Hello! I browsed the forum and found a heap of game models, usable not only 
  [attachments] image.png, image.png
- **2026-06-13** `roman117` [🔍╎texturing-and-shading] Okay, making some progress at least. I used this tutorial I randomly stumbled across, IDK how it didn't show up through all my previous searching cuz its very good.
  https://www.youtube.com/watch?v=n7FkOaI0vPI
  
  I still have the problem of the texture sticking out like a sore thumb, have to figure out how to make it use the existing textures underneath. But there's another very annoying problem;
  
  There seems to be white lines or artifacts or something all over my model now that get worse the farther I zoom out. They change shape if I move the UV around. I have no idea what's causing this or how to get rid of them, so I'd appreciate some help...
  [embed] The Secret to Perfect Decals in Blender https://www.youtube.com/watch?v=n7FkOaI0vPI You’ve been doing stickers the hard way. Shrink wrap modifiers, floating geometry, projection nightmares—it’s all kind of a mess. So quit that. In this tutorial, I’ll show you a smarter way to apply decals in Blender
  [attachments] image.png, image.png, image.png, image.png, image.png
- **2026-06-13** `chiefster_4014` [🔍╎texturing-and-shading] What game are you asking for? If it's Halo 3 or Reach, the specular coefficient is a flat multiplication into the specular. The specular exponent controls the power of the mix between the specular color and fresnel color. A reflection mask is just a mask to control where cubemap reflections are on a material.
- **2026-06-18** `chiefster_4014` [💬╎general-help] If it's the latest spartan pack then I don't think those errors mean anything, but if you're having issues with materials being completely black/very glossy/ broken in some way then that might be the cause.
  
  Also, what do you mean by some textures aren't accessible to repaint? If you want to change the colors of the material you're on you can do it from the shader editor.
- **2026-06-29** `chiefster_4014` [🔍╎texturing-and-shading] It looks like you have the control map plugged into the color map inputs. That isn't correct unless the material info says that it's supposed to be there.
  Also, you need to change the Alpha setting from Straight to Channel Packed for all textures. If not set, the color of the texture will be multiplied by the alpha channel and can cause the color of the shader and/or the shader itself to turn black, depending on the affected texture.
- **2026-07-23** `koax117` [🔍╎texturing-and-shading] not in this instance. You just imported an .obj, not the textures, nor is that a .blend with all the shaders already set up. 
  you would have to manually set it up yourself
  there is a Reach Grunt pack with them already set up.
  https://discord.com/channels/819709630540021810/1271662648655876096
- **2026-07-27** `felixvyra` [💬╎general-help] Hi.
  
  Does anyone know anything about the original Halo combat evolved campaign shader pipeline and if anyone has been able to make a decent approximation of it in Blender?
  Along with some accurate Halo Combat Evolved rips.
  
  I mean the original look of the covenant surfaces specifically for the weapons and vehicles and armors is so unique and I would really love to inspect them more closely in Blender.
- **2026-07-30** `scavhyena` [🔍╎texturing-and-shading] Do we have any tutorials on putting decals with color on meshes that use the Halo Infinite style materials? I searched the whole server and all I could find was people asking. Wanted to put scorch marks on a model and paint blood stains on.
- **2026-07-31** `setriaphtfk` [🔍╎texturing-and-shading] On this model I have been working on the rip of, these maroon stripes are just plains over the top of the ship’s body. They are just slightly transparent.
  I want to try and make some sort of color mask so is matches up in materials and normals and all that. I have a plan for how to make that work, I just need to know if it’s possible and how to essentially take the stripes and project them or imprint them onto the main texture.
  
  Maybe to simplify: take a cube and a small triangle plain on one side. I want to make the triangle a part of the texture on the cube beneath it. Is that possible? How do I do it?
  [attachments] Phaeton_Class_v2.png
- **2026-08-03** `koax117` [💬╎general-help] if you'd like, you can share screenshots of -
   - Render Properties window: Color Management at the bottom
   - Output window. Color Management and Post Processing at the bottom
   - Sun light properties: strength, exposure, etc.
   - Compositing window: if you have any exposure nodes or anything built in. 
   - World Shader: what the HDRI texture is (by clicking on it) as well as any nodes affecting it.
  
  My guess is with you using that Physical Atmosphere addon it runs a script that changes some settings to help with the rendering, but it is probably set a default lighting configuration that doesn't match your scene. I've used TrueSky in the past and it fucks with the exposure settings in my scenes a lot.
- **2026-08-04** `felixvyra` [📜╎ripping] Wait really? What do you mean by that?
  I mean I have found out that there are basically 2 types of shaders.
  .shader and .template_shader.
  As far as i understand both are required to fully recreate it.
  But I can only see input paramters in the H2 mod tool.
  When you mean "uncompiled" do you mean like HLSL or DXBC?
- **2026-08-10** `chiefster_4014` [💬╎general-help] I'm more familiar with porting stuff to Halo 4 MCC, but I assume with both games the first thing to do would be to adjust and reposition infinite's model to have similar proportions and the same default pose as CER/H3, then swap the weights to use the weight names from either game. H3 has way less bones in its rig so you'll have to merge some of the weights or redo them. Then you'd bake the textures of the Infinite model into whatever format they need. Both can use PBR textures.
  Them you'd port each model into their respective game. Idk about CER but H3 should be mostly straightforward, you just need an addon to export to the correct format. Them you'd import the textures into the engines, setup your materials, and then that should be it if I'm not forgetting anything.
- **2026-08-15** `papyrus95` [🔍╎texturing-and-shading] Hey there, I'm taking the shoulders from the remakes Mark V and I would like to make it compatible with the Halo Infinite shader/HIMS. How would I make the ASG map, and the mask 1 map in a program such as Substance Painter if it's possible?
  [attachments] image.png
- **2026-08-16** `chiefster_4014` [🔍╎texturing-and-shading] try using this pack instead. it's got accurate shaders so the color zones work correctly.
  https://discord.com/channels/819709630540021810/1491122330838106253
- **2026-08-16** `chiefster_4014` [🔍╎texturing-and-shading] baking textures with the Reach shaders is kinda tricky since it isn't proper PBR. you should be able to bake a diffuse, specular color, and flat gloss map by going into the nodegroup and using specific nodes as outputs to bake from
- **2026-08-18** `koax117` [💬╎general-help] most commonly i see that error when the shader names or the shader itself has been modified/broken in some fashion.
  the shader names should always remain the string of numbers for ekur identification purposes.
  beyond that, I'm not certain. 
  make sure that when you set up Ekur you didn't set it up for campaign, as the textures won't have loaded. and make sure you followed the full download/deploy instructions. (always stress that for folks)
  [attachments] image.png
- **2026-08-24** `felixvyra` [🔍╎texturing-and-shading] Hi.
  
  I have browsed through all of the uploaded assets packs recently and have noticed that there is a huge range of different game specific shaders made by different people and many version of them.
  
  I saw usernames like: Chunch, ChormaCore, Sierra, Chiefster, Soulburnin pop up multiple times.
  
  What I am wondering is can someone create a thread in the asset-upload channel that has all of the most up to date per game Blender shaders and preferably with detailed documentation on how to use them and how they work?
  Maybe someone who has contact to these people?
  
  I saw things like:
  Halo Infinite (HIMS)
  Halo 5
  Halo 5 Beta
  Halo 4
  Halo Reach
  Halo 3
  Halo 2A SP
  Halo 2A MP
  Halo CEA
- **2026-08-27** `mak_74m` [💬╎general-help] Can anyone help me figure out how to use the halo infinite material shader? I want the security shoulder pad to have the same color and finish as the MK-V shoulder pad. here's the nodes for each pad (In order of mentioning). I'm just not sure what to make of them. I used this pack as a base https://discordapp.com/channels/819709630540021810/1433303088944517191 and the security shoulder from this pack https://discordapp.com/channels/819709630540021810/1491692419463254026
  [attachments] Screenshot_2026-08-27_091331.png, Screenshot_2026-08-27_091409.png, Screenshot_2026-08-27_091428.png
- **2026-08-27** `chiefster_4014` [🔍╎texturing-and-shading] are you trying to bake assets from the Halo Infinite spartan pack in #📂╎asset-packs? if so you should try just baking them manually, as regular baking addons tend to screw up with Infinite assets due to the complex shader used. you could also try using Ekur, but I'm not sure if it supports Blender 4.5
- **2026-09-06** `koax117` [📜╎ripping] Hologram shaders are a bitch.
  I've set up the halo ring hologram shader in my io8 pack you can pull from, just need to swap for each ring.
  https://discord.com/channels/819709630540021810/1517616133492969574
- **2026-09-08** `gameoverload` [💬╎general-help] So, Blender currently does not give you light and render pass information as part of the shader editor, unlike say a game engine in comparison.
  So, Bungie era games that do a bunch of their own custom render functions have to be approximated in various cases.
  My hope is with Blender adding light and render pass information in 5.3 or 5.4 that we can be a little more accurate with shading approximations.
  
  Halo Infinite becomes easier only because its using modern rendering techniques which Blender has more built in to work with.
- **2026-09-17** `koax117` [💬╎general-help] your packs are fantastic. its just spelling out little things like this would help. one thing I did in my asset libraries is I made it that if someone opened up the actual .blend instead of just using it as an asset library, there's a tutorial page in the text editor. You could add a window on the default layout that explains things.
  For my comprehensive tutorial, I've got an outline written up going through all the steps. It's primarily focused on Infinite Spartans, setting up Ekur, using the Spartan Pack, selecting attachments, modifying shaders, poses, etc. Though I've thought about doing brief asides for some of the other prominent packs that differ (Reach, H4/5, etc.) - so can definitely include things for yours if you think its warranted
  [attachments] image.png
- **2026-09-20** `yaboipaul` [🔍╎texturing-and-shading] Looking for some help trying to import textures from one of Exminar's awesome asset packs. 
  
  So I am trying to take the textures from his uploaded Warthog model and bring them over to Maya. I have already got the model I want in Maya as an FBX (I don't need the armature) and upon cracking open the texture folder I found that none of them are labeled with defining titles that convey their contents. So I see a lot of PNG's but they are all numbers. In Maya I need to know which ones are for normals, bump maps, specular etc. 
  
  I read online that the way Blender creates color and textures is different from Maya though, so I am not even sure if this is even possible?
- **2026-09-20** `yaboipaul` [💬╎general-help] Looking for some help trying to import textures from one of Exminar's awesome asset packs. 
  
  So I am trying to take the textures from his uploaded Warthog model and bring them over to Maya. I have already got the model I want in Maya as an FBX (I don't need the armature) and upon cracking open the texture folder I found that none of them are labeled with defining titles that convey their contents. So I see a lot of PNG's but they are all numbers. In Maya I need to know which ones are for normals, bump maps, specular etc. 
  
  I read online that the way Blender creates color and textures is different from Maya though, so I am not even sure if this is even possible?
- **2026-09-27** `koax117` [🔍╎texturing-and-shading] Pretty sure there isn't an upload of Countdown with fully set up shaders in the server, that's why. not sure what upload you're referring to
  
  If you use Foundry with the mod tools, you can import in the levels with the shaders 90% set up to be game accurate, though some aspects like decals take further work.
  
  https://github.com/ILoveAGoodCrisp/Foundry
  [embed] GitHub - ILoveAGoodCrisp/Foundry: A Blender addon supporting asset ... https://github.com/ILoveAGoodCrisp/Foundry A Blender addon supporting asset creation for Halo Reach, Halo 4, and Halo 2 Anniversary Multiplayer - ILoveAGoodCrisp/Foundry
  [attachments] image.png
- **2026-10-06** `koax117` [📜╎ripping] you can choose to export ALL the geo or just select, depending on your needs.
  if you import the bsp into Blender 3.6 first, you can use the Cr4btool addon to setup the materials. it isn't up-to-date/1:1, but gets you much closer to accuracy.
  https://github.com/PlasteredCrab/Halo-CR4B-Tool
  it'll require some additional setup, but it's what I've used as the baseboard for all my H3 rips
  [embed] GitHub - PlasteredCrab/Halo-CR4B-Tool: One-click game-accuracy tool... https://github.com/PlasteredCrab/Halo-CR4B-Tool One-click game-accuracy tool for Halo 3/ODST/Reach assets in Blender that also exports assets from every Halo game - PlasteredCrab/Halo-CR4B-Tool
- **2026-11-01** `finny.1663.` [🔍╎texturing-and-shading] Trying to follow some tutorials on how to bake textures so me and a friend can convert the https://discord.com/channels/819709630540021810/1371126749009547336/1440493886416883723 into a usable FBX for unity. we can get it to bake properly don't know how, the shader thats used in blender doesn't follow the same tutorials that we've watched and we're also new to this.
- **2026-11-01** `chiefster_4014` [🔍╎texturing-and-shading] it's likely from having your Bake Type set to Diffuse. that's not correct for baking textures most of the time. it should be set to Emit/Emission, and you should plug the Bake outputs you need like Color, Unity Mask map and Unity Smoothness Map into the Surface input of the Material Output node on the right. doing that will bake textures correctly.
  to bake your normal map, plug the shader back into the material Output node, and set your Bake Type to Normal.
  Also, for baking, set your Render samples to 1, and turn off Denoise. all those do is increase bake type and don't provide much help. if you want more detail in your bakes, increase the resolution to something like 2048x2048 or 4096x4096.
