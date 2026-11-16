# colour change cc maps

Selected from 221 candidate messages (ranked by length, links, Halo relevance; capped at ~260 KB). Chronological. Channels: 💬╎general-help, 📜╎ripping, 🔍╎texturing-and-shading, 🦴╎animation

- **2021-03-22** `err_0r` [📜╎ripping] at this stage, you have to make sure that the color mask texture that ought to be in there somewhere is connected, what you're wanting to do is use the color mask to designate which areas take color, and basically multiply or lerp the color you choose onto those areas using the masks from the cc texture as a mask
- **2021-03-22** `err_0r` [📜╎ripping] yeah, looking at those images, you have the tint color color block in the shader box, right? That *ought to* change the colors on the model according to the areas painted in the color mask texture
- **2021-03-22** `err_0r` [📜╎ripping] Alright, basically there IS a way to find the exact color value of green and red that you'd need, but it requires a deep dive into reaches code because just as you can change the color because of the tint color box, there's a line of code in the game files somewhere that stated what hex code was fed into that tint mask for that material that got put on that transport bird. It's a bit of a pain to get those numbers so as long as you can already get color onto the model, you'd be best served trying to match the colors as best you can by hand.
- **2021-03-22** `err_0r` [📜╎ripping] that'll be why in the image there's no color on the diffuse even thought it's specified in the tint colors
- **2021-03-22** `surasia` [📜╎ripping] Diffuse : Diffuse
  Diffuse Alpha : Diffuse Alpha
  Tint color 01 : Primary
  Tint color 02 : secondary
  TintMask : CC
  Normal : Normal
- **2021-03-22** `err_0r` [📜╎ripping] basically the way the cc texture works is that it tells the shader what is colored and what isnt
- **2021-04-22** `sierrad421` [🔍╎texturing-and-shading] ...and I just had another idea. Specifically, a new colormap for Reach Mark V, where the front is the primary color, and the back is the secondary (e.g. red front, black back). I got the idea from a fanfic I read where a Spartan-III had such a color scheme...
- **2021-04-22** `sierrad421` [🔍╎texturing-and-shading] ...and another thought: *tertiary colors for 2, Reach and 4, secondary and tertiary colors for CE, individually-colorizable parts...*
  
  No, no. I'm going hog wild here.
- **2021-04-23** `pyruvic_acid` [💬╎general-help] for halo reach are thes cc maps for placing the armor colors or is it the specular/metalic map?
- **2021-04-27** `lobbster` [🔍╎texturing-and-shading] I encountered this issue before with the actual Jorge model not liking color changes so it may be a problem with the actual reach texturing
- **2021-04-27** `err_0r` [🔍╎texturing-and-shading] A curious detail is that the camouflaged areas of the fp arms and the body texture (the thigh plates) have color mask areas in the alpha of the cc texture.
- **2021-04-27** `err_0r` [🔍╎texturing-and-shading] I personally decompiled and recompiled the cc textures so that I didn't have that data in the alpha, swapping it over to another channel so I could better visualise what the masks were doing, but the shader I tossed together in amplify shader editor ended up looking like this.
  [attachments] unknown.png
- **2021-05-07** `pointdevice` [📜╎ripping] just make an update version thats like, 'now with multi threading' but the multithreading is just the random color changes are on another thread
- **2021-05-13** `r93_sniper` [💬╎general-help] i had given a link with optional color change options basically same day lol
- **2021-05-15** `aqua_himura` [💬╎general-help] which color should i go with for secondary colors for a banished anti air wraith(to differentiate from the normal banished wraith)
- **2021-05-17** `soulburnin` [💬╎general-help] Looks like my old port. Add two color attribute, but use $phongtint and $envmaptint instead of $color
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
- **2021-05-24** `surasia` [🔍╎texturing-and-shading] but you gotta use color ramps etc to color instead of multiplying the CC texture
- **2021-05-25** `williaso` [🔍╎texturing-and-shading] These are multi-billion dollar supersuits that they color in bright primary colors, it doesn't matter if they're matte 😂
- **2021-06-02** `surasia` [🔍╎texturing-and-shading] From R93_Sniper:
  halo 4 doesnt have a dedicated CC map
  it has a control map, and there's 2 types
  the one for Multiplayer spartans is as follows:
  R - Specular
  G - Diffuse
  B - Gloss
  A- Color Change
  for the Color Change, White is Primary Color, Grey is Secondary, Black is Uncolorable
- **2021-06-02** `ember.cinder.ash` [🔍╎texturing-and-shading] do you know if I'm supposed to get the blue colour naturally through these maps, or do I need to pick the base tint myself?
- **2021-06-05** `r93_sniper` [🔍╎texturing-and-shading] i'd say go for a silver primary color and use red highlights. You could certainly go in and give a darker secondary though, but that really depends on what kind of theme you're going for
- **2021-06-10** `sculpted_stache` [🔍╎texturing-and-shading] would brown primary grey secondary and dark green highlights work for a color scheme?
- **2021-06-10** `sculpted_stache` [🔍╎texturing-and-shading] yeah I like them too, but I'm specifically trying not to do a black or grey primary color scheme
- **2021-06-12** `sculpted_stache` [🔍╎texturing-and-shading] I'm gonna switch the black secondary color on my last retexture to some other color
- **2021-06-15** `soulburnin` [🔍╎texturing-and-shading] I’ve figured out color correction in the .mats, and I believe possibly spec tint, but I’m not 100% on it
- **2021-06-15** `soulburnin` [🔍╎texturing-and-shading] The UNSC weapons have a bluish tint to them, and that’s what I get from some of the materials. Still need to sort out blending though. There doesn’t seem to be a value for that like 3/Reach, so I’m wondering if it’s actually done through the control
- **2021-06-16** `r93_sniper` [💬╎general-help] Xbox Multipurpose Maps are
  R = Specular
  G = Self Illumination
  B = Color Change
  A = Auxiliary Mask
  
  Whereas PC Multipurpose Maps use
  R = Auxiliary Mask
  G = Self Illumination
  B = Specular
  A = Color Change
- **2021-06-26** `surasia` [📜╎ripping] From R93_Sniper:
  halo 4 doesnt have a dedicated CC map
  it has a control map, and there's 2 types
  the one for Multiplayer spartans is as follows:
  R - Specular
  G - Diffuse
  B - Gloss
  A- Color Change
  for the Color Change, White is Primary Color, Grey is Secondary, Black is Uncolorable
- **2021-06-26** `vanitytm` [📜╎ripping] so with the color change on H4 controls, how do I manage that differently from H5s?
  in reference to the shader you setup in blender for the helmet
- **2021-06-26** `plastered_crab` [💬╎general-help] Head to the shading tab at the top of Blender and it's a pre-setup model you'll just have to change the Primary and Secondary Color blocks on the main node :)
- **2021-06-29** `vuthakral` [🔍╎texturing-and-shading] The cubemap itself doesn't get colour-tinted, rather the specular reflection on the model does
- **2021-07-02** `martyrthesaint` [🔍╎texturing-and-shading] can anyone send me the reach sniper shoulder's cc map? for whatever reason mine is just
  all black lmao
- **2021-07-06** `r93_sniper` [📜╎ripping] Halo 4's multiplayer spartan control maps are RGBA Spec/Diffuse/Gloss/ColorChange
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
- **2021-07-20** `emerald3050` [💬╎general-help] the whole reason im doing this is because i wanted to make something similar to this https://www.youtube.com/watch?v=e9H-dbOPNvQ&t=17s
  [embed] Porting the Halo 2 Anniversary Player Model into Unity https://www.youtube.com/watch?v=e9H-dbOPNvQ&start=17 Want to get that cool looking Spartan from Halo 2 Anniversary into Unity? This video explains just how!
  
  Music: 
  Tell Me Quietly by Nick Macdisi
  https://www.youtube.com/watch?v=tQiBR231nbQ
  
  CC
- **2021-07-22** `crashlegacy14` [🔍╎texturing-and-shading] I'm relatively new to blender, but,I've been toying with the idea of making some custom color control maps for different armor patterns and the like. The question I have is why do a lot of the CC textures have varrying values? Like dark reds used near weathered areas instead of just solid neutral red? Shouldn't that all be packed into the diffuse?
- **2021-07-24** `backslashmedia` [💬╎general-help] because Ive just waisted so much time rigging it to find out I can't change the secondary colour from white
- **2021-08-06** `univox.` [💬╎general-help] I've been working on a spartan using chief's pre-shaded and textured halo reach spartan thing and the grenadier chest won't change colors
  [attachments] help.png
- **2021-08-06** `soulburnin` [💬╎general-help] You’ll have to go select it, go into the shader editor, click on the CC texture for the chest piece, press N to open the properties of the texture node, and change the alpha to be channel packed.
- **2021-08-22** `crashlegacy14` [🔍╎texturing-and-shading] Alright, so this is a general texturing question I have.
  
  If I'm working with a large texture covering multiple types of materials, and I want to use detail textures to, well, detail things. What's more performance efficient, one material using multiple masks to layer on detail textures (like the color control masks are used for primary secondary colors), or partitioning the different materials into different ... material slots? Uhg, I don't know a better way of describing that.  Is there even a noticeable difference in performance?
- **2021-09-04** `aqua_himura` [💬╎general-help] kinda like the armor colors in the halo games(pretty much using a cc/color change bitmap)
- **2021-09-29** `aqua_himura` [💬╎general-help] (i probably asked something similar in the past) but how could i use a color change bitmap in blender?
  [attachments] unknown.png
- **2021-10-04** `soulburnin` [🔍╎texturing-and-shading] so the Spartan MP armors have the following textures
  
  CC map w/ alpha channel, suffixed as _d
  Control map, with metal in R channel, AO in G channel, and gloss in the alpha channel
  Specular color map, labeled as _s
- **2021-10-17** `mehrunesdaegan` [📜╎ripping] When I extracted the new Season 8 armors using Reclaimer, the color control maps all seem to be blank. Like void of anything as if they're an empty png. 
  Tested extracting other armors and their bitmaps and it seemed to work fine.
  I do something wrong or is it random on what armors actually have CC?
  Any help would be appreciated I'm new to the program so apologies if I missed something crucial.
- **2021-10-24** `r93_sniper` [💬╎general-help] its different if its Xbox Formatted or PC formatted, but you're likely to get the latter which is
  R: Auxiliary Mask
  G: Self Illumination
  B: Specular
  A: Color Change Mask
- **2021-11-05** `girldiseased` [🔍╎texturing-and-shading] and the issue still persisted with coloring the whole thing one color without any cc
- **2021-11-29** `unordinal` [💬╎general-help] Hey, I'm trying to get the colors for Halo Reach grunts and in the biped tag there's four "Change Colors" datablocks. Inside those blocks are the actual variant color permutations. I know that the four datablocks correspond to albedo tint, specular tint, fresnel tint and environment tint but I forget the order in which they're listed. Does anyone happen to know?
- **2021-11-29** `unordinal` [💬╎general-help] That lists the general shader inputs which is fine normally but it doesn't include the color change permutations for bipeds which can override those values per-biped.
- **2021-11-30** `unordinal` [💬╎general-help] Okay, I'm a little bit more confused than I was yesterday when it comes to the 'Change Colors' .biped color variant permutations for Halo Reach. Each slot seems to correspond to a color channel on the CC Map (makes sense) but they also sometimes seem to affect the diffuse, specular, fresnel and environment tints as well and I'm not sure if there's a flag I'm missing or what
  
  For example, I'm currently looking into the Skirmisher - the major has red feathers which are controlled via blue on the CC map, so it's in the third slot of the color change datablock (4 slots total, numbered 0-3 and seem to correspond to primary, secondary, tertiary and quaternary colors respectively)
  
  Problem is, there's colors set for the secondary and quaternary colors even though there's no respective colors on the CC map for them, and given that the specular tint is blacked out in the base shader and the elite armor uses those slots to control specular and fresnel tint, I'm not entirely sure how the game determines whether to use a color for a primary/secondary/etc color or a diffuse/specular/etc tint.
- **2021-11-30** `soulburnin` [💬╎general-help] After the diffuse is blended with the detail maps and colored via the CC texture, it gets mixed with the specular tint to form the final specular color
- **2021-11-30** `unordinal` [💬╎general-help] Right, I forgot about albedo blend - I'm a bit confused still when it comes to that though, as elite armor specifically has a tertiary color that is not in the color map at all and yet applies to the fresnel tint.
  
  Actually, now that I type that out, does albedo blend set to one change it to ignore the CC map?
- **2021-11-30** `soulburnin` [💬╎general-help] Nah, the Skrimisher/Elite armor shader breaks down like this:
  
  Diffuse blended with detail map ---> Colored based on CC map ---> Mixed with specular tint (0 means only specular tint, 1 is full albedo tint) = final specular color
  [attachments] blender_D9WTwF6eQF.png
- **2021-11-30** `soulburnin` [💬╎general-help] A fresnel curve value of 0 means the fresnel color will completely override the specular tint
- **2021-11-30** `soulburnin` [💬╎general-help] According to the globals tag, the elite specular tint value is #D6D4DB and the fresnel color is #E8C6A4
- **2021-11-30** `unordinal` [💬╎general-help] But I'm still confused on where the fresnel color is taken from the tertiary color. Lemme show an example: the Elite SpecOps has a green fresnel, and that same color is listed in the color change block under slot 2, despite there being no blue in the elite color control map, so the fresnel color in the shader must be passed that color from the color change block in some way, right?
  
  Maybe I'm just not fully understanding so I'll play around a bit and re-read your visualization a few times
  [attachments] unknown.png, unknown.png
- **2021-11-30** `unordinal` [💬╎general-help] I think the emissive color is directly overridden in the self-illum color in the shader, and the Ranger elite has an orange fresnel which also matches up with an orange color in the tertiary slot
- **2021-11-30** `soulburnin` [💬╎general-help] The cubemap tint on the spec ops pieces is sort of greenish, could be causing it too
- **2021-11-30** `unordinal` [💬╎general-help] I guess that mostly makes sense - I'm now assuming this is just a really strong fresnel on the skirmisher feathers and not an actual diffuse color change, though I'll have to try and verify that in-game to make sure
  (actually there's green in the CC map I just completely missed)
  [attachments] unknown.png
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
- **2021-11-30** `chromacore` [🔍╎texturing-and-shading] Pretty sure it's tinted per material. I think I remember an input there when I was making my reach shader
- **2021-11-30** `unordinal` [💬╎general-help] also really glad you posted this because it got me to re-look at the HLSL and I misread that it subtracted the albedo from the specular tint for the mix right after... time to redo my shader a bit once again hah
- **2021-11-30** `unordinal` [🔍╎texturing-and-shading] It's tinted per material when there's not a CC map (I think), otherwise the specular tint either is missing or seems to be overridden by the color in the color change
- **2021-11-30** `unordinal` [🔍╎texturing-and-shading] there's a whole conversation in #💬╎general-help but basically the individual material tint seems to be overridden in the biped color change block
- **2021-11-30** `unordinal` [🔍╎texturing-and-shading] Okay, this doesn't seem entirely correct but I'm on the right track - the secondary color sometimes controls (I'm assuming) the environment map tint as well
- **2021-11-30** `unordinal` [🔍╎texturing-and-shading] I'm testing this on the grunt right now - the grunt has a red-only CC map, and when I change the secondary color to a green there's a slight green tinge to the light reflection
- **2021-11-30** `unordinal` [🔍╎texturing-and-shading] Also looks like the secondary color env map tint was a false alarm (I used green in a space that had a bluish tint which was a terrible idea) so yeah I think that solves that completely
- **2021-12-01** `soulburnin` [🔍╎texturing-and-shading] The only addition is that, when element 3 is functioning as both specular tint and quaternary color, it’s masked by the alpha of the CC
- **2021-12-01** `soulburnin` [🔍╎texturing-and-shading] Reach visors, for instance, have their own tiny CC map with an alpha channel that isn’t pure white
- **2021-12-01** `unordinal` [🔍╎texturing-and-shading] I think the CC map alpha might just be masking the quaternary color that's being applied to the visor diffuse, not masking the specular itself
- **2021-12-01** `sswampp` [💬╎general-help] the red channel controls where the primary color goes, the green channel controls the secondary color
- **2021-12-01** `sswampp` [💬╎general-help] if you want to color the diffuse itself in gimp you'd copy the information from one channel (from the CC map) at a time and paste a color into it (in the diffuse)
- **2021-12-08** `richter3d` [💬╎general-help] This feels like the right channel but please let me know if there's a better one to ask, is there anything like this that works for MK VII from Infinite? This was a game changer back in the day for cosplay and with the armor progression I don't think I will ever find the permutations to make "my" Spartan. Thank you!
  https://veegie.com/Vanity/
  [embed] Vanity - Halo Reach Player Image Generator https://veegie.com/Vanity/ Vanity is a player image generator for Bungie's video game Halo Reach
- **2021-12-09** `.bluephoenix` [💬╎general-help] I know everybody's busy with Infinite, but if anybody could provide me with fixed CC maps for Reach's Sniper shoulders, I would really appreciate that!
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
- **2022-01-09** `.bombwolf` [💬╎general-help] Need some help from color specialists... I'm trying to color the ODST and cant get the arm pieces to look the same because they're two different textures. This would be fine, if like all the other textures, they were a kinda of tinted white, as the shoulder, but for some reason the forearm is pre colored.
- **2022-01-09** `.bombwolf` [💬╎general-help] I color it with the slight tint of yellow and it just looks desaturated
- **2022-01-11** `daniel56` [🔍╎texturing-and-shading] I assume the only way is to manually apply the colour change with your cursor
- **2022-01-20** `_gallium` [💬╎general-help] If you aren't, make sure to connect any normal map outputs to both the glass & glossy nodes, and changing the colour of the glass one will tint the visor.
- **2022-01-30** `theishikawarin` [🔍╎texturing-and-shading] It's for colouring multiplayer spartans and elites like you see in-game, it's just a mask for the primary and secondary colours
- **2022-02-06** `sunksi` [🔍╎texturing-and-shading] Color ramps and rgb mix into the primary and secondary spots act bizzarely
- **2022-02-08** `chunch` [🔍╎texturing-and-shading] You literally don’t need to do any editing in secondary software if your shader is set up right for texture baking
- **2022-03-21** `surasia` [🔍╎texturing-and-shading] my shader basically just multiplies the colors with the diffuse using CC red and green as a factor
  
  For metallic it just uses the blue channel of CC, and for roughness it inverts the alpha channel of CC
- **2022-03-26** `snapchipper` [🔍╎texturing-and-shading] would anyone be willing to add the H3 color palette hex values to #📗╎lavadeeto-resources ? It'd be useful for testing CC maps in Blender before exporting my model into MCC proper
- **2022-04-04** `garfunkle3479` [🔍╎texturing-and-shading] Back in the day, my spartan was mostly steel-gray with green or red secondaries. Then with Reach I switched over to green as my primary, and steel and red for the other colors.
- **2022-04-30** `theeldestboy` [🔍╎texturing-and-shading] Now you can just change colors etc by changing the shading slots like the reach armor
- **2022-05-13** `gameoverload` [🔍╎texturing-and-shading] I hope they make the system able to make flat color UVs though, need those foot tinting masks and such.
- **2022-05-22** `unabled5xxrex` [💬╎general-help] Can someone explain to me why I can't change the primary color of the suit?
  [attachments] Color_Prop.png
- **2022-06-01** `Deleted User` [💬╎general-help] Probably by just matching its color, since it's the tertiary color on halo 3 armors
- **2022-06-08** `unordinal` [💬╎general-help] ID map should be used to mix between different shaders and the tint map should colorize the regions of each shader
- **2022-06-09** `boostaslim` [💬╎general-help] hey! Im pretty new to this kitbasher thing and blender in general, but this kneepad wont change color or anything when I mess with the sliders and shader options. The other kneepad works fine for some reason. Any idea whats going on? Thanks!
  [attachments] unknown.png
- **2022-06-10** `sunliteflame` [💬╎general-help] So uh, im new to blender and i downloaded the reach pack by chiefster, how do i change colours of everything?
- **2022-06-15** `gameoverload` [💬╎general-help] You can also do UV, requiring the same thing as the above, but I only use those for one color channel of the image as a tinting mask.
- **2022-06-18** `koax117` [🔍╎texturing-and-shading] easiest thing to do would be take these into something like photoshop with the color picker
  https://www.halopedia.org/Armor_coatings_%28Halo_Infinite%29
  [embed] Armor_coatings_%28Halo_Infinite%29 https://www.halopedia.org/Armor_coatings_%28Halo_Infinite%29 Armor coatings, also known as armor skins, are an element of player customization in Halo Infinite. They are a seven layer system that change the materials and coloration of a player's armor permutation.
- **2022-06-19** `bag_of_nickels` [💬╎general-help] My previous problem was solved but now it seems that the secondary color for the helmet isn't showing
- **2022-07-03** `r93_sniper` [🔍╎texturing-and-shading] if you're using the infinite shader (which judging from the masks you are), the goal was to make one that emulates the look of what halo infinite does as closely as possible. They use, at max, 6 layers from the two CC masks and then 2 more from the ASG map. Unless there comes an instance where they actually use an extra channel (and subsequently actually add it to the game/shading system), there's no purpose in trying to make the shader accept that mask
  
  If you're just trying to add a decal stripe there's a few methods to do so, 
  1. Pick the zone that it most closely matches the color that you want the stripe to be and make it part of that mask while removing it from whatever overlaps
  2. Create a decal plane and attach it to the model and UV it to have only the decal stripe (and alpha or additive the rest)
  3. Modify the shader for your own local use to accept an extra mask
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
- **2022-08-11** `gameoverload` [💬╎general-help] I mean shit I got 5 white lighting lights and 2 color tinting lights in this one scene.
  [attachments] unknown.png
- **2022-08-18** `chromacore` [💬╎general-help] actually most of halo 3's armors have it. All the multiplayer is tinted even more blue.
- **2022-08-19** `vinwarrior` [💬╎general-help] Does anyone know what the specific swatch used on the primary grey of Scorpion Punch is? Not the color but the actual material swatch.
  [attachments] unknown.png
- **2022-08-23** `brooen` [💬╎general-help] if you want i have a version of the infinity hangars with a better shader and game accurate tints
- **2022-08-25** `gameoverload` [🔍╎texturing-and-shading] Well Diffuse is normally a combination of Albedo or color, and either a baked in map on top of that, or something in the alpha of it that pertains to light or shadow in some form, or something entirely different like color masking.
  A normal texture requires the use of a normal map node, with the normal texture connected to it and then the normal map connected to a normal input.
  The AO would require a MixRGB which multiplies the Diffuse by the AO.
  Metallic into the metallic input.
  Smoothness into an inverse node then into roughness, since smoothness is the inverse of roughness.
  Then the tint index sounds like a tinting mask of some kind.
  Also you should not be using a Halo 5 armor shader for this, it is not built for how Gears Tactics works, it is built for how Halo 5 works.
  You would need to build your own shader for it.
- **2022-08-27** `fiycsf` [💬╎general-help] Fiy rant's story time;
  ```
  For example, for a non halo (Vroid software made model) I had someone want to pay 30$ for a hair colour change. Took me like, 15 minutes to get the exact kinda fade they wanted. But normally, when doing on commission work, I ask for that much (30$) for the hour. So I jumped through the hoops to force the model into VRC in a desktop medium performance rank.
  
  Uploaded it on my account, let em clone and try it so they'd have something until they figured it out, and said "here's all the files, you may have to ask in various servers public help channels to get it sorted in unity for yourself, but you have it cloned from me for now, so you shouldn't have to rush."
  
  And I figured, that'd be the end of that. I did far more than was asked, on the same cost.
  No, no it was not that simple. I'd check my phone to a dozen some messages many times over the next few days asking me alone *in DM's* how to setup the model in unity that I did in under an hour, and ended up getting so fed up with the constant questioning and them not doing as explained to setup the model, coupled with phrases/behavior like I still owed them something, that I packed up the model gave it to them, taking another bit of time outside of the constant messaging, just to get them to leave me be.
  ```
  I know stories like these aren't uncommon, commission work or not, among the friends I have that have been willing to spend time teaching or setting stuff up for some. And honestly, it burns out a c ...[cut]
- **2022-09-19** `zangh` [📜╎ripping] Yee that would be related to the alpha channel thing. So in this case where you'd have the cc mask, you plug the alpha channel of the base color into that where you would the cc mask
- **2022-09-19** `tacohombre` [🔍╎texturing-and-shading] I changed to the 2.0 shader which seems to make it more consistent but the colour change doesn't work anymore
  [attachments] unknown.png
- **2022-09-25** `.kola` [🔍╎texturing-and-shading] I'm trying to make a custom spartin and I have figured out how to change the primary colors in the shaders, but is there any way to change the 2ndaries in the shader editor?
- **2022-09-28** `sswampp` [💬╎general-help] the red channel is primary color, the green channel is secondary color
- **2022-09-29** `orkscool` [💬╎general-help] Trying to render an ODST rn and the cloth around the neck won’t change colors
- **2022-10-31** `jadedqrow` [💬╎general-help] ~~Go into google and google how to use the shader Editor~~
  
  Just go into the shader editor click on the eyes and go to its material, drop a mix rgb node on the diffuse/base color set to multiply. Change factor to 1 and use the bottom white box to change color. If that doesn’t work there’s other ways
- **2022-11-04** `jackofchaos` [🔍╎texturing-and-shading] I forgot to expose the color change in the node group. A quick redownload will fix that
- **2022-11-06** `jadedqrow` [💬╎general-help] I’m blender put a plane under him, metallic to 1 and Roughness to 0 and change color. Also use cycles
- **2022-11-10** `willywonka8612` [💬╎general-help] I was able to change the other color modifier for the helmet, but that only messed up the secondary colors
- **2022-11-14** `hydranaught4444` [💬╎general-help] could someone explain to me exactly which infinite texture does what exactly cuz i'm seeing 4 colors 1 i can tell us the color change and normal map but what are these other two textures
- **2022-11-15** `hydranaught4444` [💬╎general-help] i'm porting to the halo engine this is a color change i made based off the halo wars 2 color change
  [attachments] B5D60AC2-D1DC-4628-BA21-732D42D48C60.jpg, 5A26B50E-9B5D-4C3C-A7ED-599088E532A3.jpg
- **2022-11-15** `hydranaught4444` [💬╎general-help] sentinels currently im making textures and a color change for the infinite sentinel but i can just bake a uv and use that to make a color change pretty easy
- **2022-11-15** `hydranaught4444` [💬╎general-help] i did for the diffuse and i didn't change the uv's i made a blank texture and set bake type to uv and it makes something i could easily work with to make a color change myself
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
- **2022-11-15** `hydranaught4444` [💬╎general-help] the halo wars 2 color changes work good enough which is what mine is based on i don't have any good pictures since i haven't had wifi in a while but like i've been saying i can just make the cc myself to fit what i need it to same with the diffuse
  [attachments] IMG_5971.jpg, IMG_5970.jpg
- **2022-11-15** `hydranaught4444` [💬╎general-help] i'm not trying to throw you off i'm saying doing something basic is good enough for me for now and i can just make my own color change for something basic making it fit in is currently not what i'm trying to do
- **2022-11-17** `tootall_76` [🔍╎texturing-and-shading] im trying to change the shading on the artaius helmet i can get it to change color, it seems ony the visor can
- **2022-11-20** `hydranaught4444` [💬╎general-help] some of the parts don't color change like it does so yeah probably different ones so ill just have to get close
- **2022-12-15** `wraith0912` [🦴╎animation] I approve of the color changes to the wraiths (though the glowing lights are a bit more purple than blue but that's not too bad)
- **2022-12-22** `evenmanatee887` [💬╎general-help] One knee is showing colours fine, the other one isn't showing colour changes
  [attachments] image.png
- **2022-12-25** `hydranaught4444` [💬╎general-help] looking at them in ce they definitely have a roughness but i don't get how they also have green lights but bright blue in game i guess it's a color change as well?
- **2022-12-25** `hydranaught4444` [💬╎general-help] so i just made a basic color change and made the whole thing gold i also found out the sentinel beam is kinda buggy
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
- **2022-12-25** `hydranaught4444` [💬╎general-help] oh no the sentinel in 3 does have a specific color change bitmap it doesn't use but it's not that good the one i made from scratch is better
- **2023-01-07** `silent1meow` [🔍╎texturing-and-shading] yknow how goblins shader has three color changes for each zone, they use the three channels from gradientmasks
- **2023-01-09** `kelcken` [💬╎general-help] Trying to bake some textures for the Infinite Banished vehicles In blender using the ones uploaded here, and the Wraith, Ghost, and Chopper have all worked fine, but the Banshee is getting a blue tint and not baking  the emissives at all.
  
  I've looked at the shader setup (identical) as well as the bake settings (also identical) and I can't figure out what the issue is. I've also tried out several different influence settings, but none of them come out right on the banshee
  
  If anyone has any ideas, please let me know! Thank you in advance!
  
  First image is of the wraith (working as intended) and the second is the banshee (not working)
  
  Bake results in the bottom left corner.
  [attachments] RenderWraith.PNG, RenderBanshee.PNG
- **2023-01-16** `e.nine` [📜╎ripping] The spartan changes color with "Color Change" but the elite does not. I'm not sure if this is something many people would have knowledge on, so it's fine if this is unknown.
  [attachments] image.png
- **2023-01-16** `e.nine` [📜╎ripping] Well, the Elite *does* change, but not in the way you'd expect. It mostly tints the entire area
- **2023-01-26** `blue.nasca` [🔍╎texturing-and-shading] Does anyone happen to know what the primary color hex is for the Noble Defender coating? I've been trying to set up the tools to grab it myself but it's taking a really long time and it's kind of a lot for just identifying one color lol
- **2023-01-26** `blue.nasca` [🔍╎texturing-and-shading] (by primary color i mean like the majority of the armor's color, its like an oliveish color)
- **2023-02-08** `r93_sniper` [🔍╎texturing-and-shading] color change does as the name implies, its a mask that tells the renderer what parts of the texture are gonna be able to get their colors changed
- **2023-04-06** `hydranaught4444` [💬╎general-help] also would you know how to make something like the emission color changeable?
- **2023-04-17** `brooen` [💬╎general-help] it might be like the tint colors for a vehicle with a color change map
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
- **2023-04-27** `gameoverload` [💬╎general-help] *Does it...? Cause it wouldn't cause Squad from my knowledge doesn't use color tint or whatever else Chivalry has in its master material files.*
- **2023-04-29** `vamparical` [💬╎general-help] anyone know how I can get this secondary colour area for the ODST Shoulders? I'm assuming it's referred to as a zone?
  [attachments] 13f0b026c5b6c0a26b84a36aa7d5386c2.jpg, 59400393d86485ced203586cb5b1fa342.jpg
- **2023-05-07** `hydranaught4444` [💬╎general-help] okay so i need some shader help so the white/light gray parts of the texture is what color changes except for the omega the color change parts is the red how would i go about setting that up
  [attachments] zeta_d.png, omegab01_d.png
- **2023-05-27** `wraith0912` [🔍╎texturing-and-shading] now, fair warning, the shader for infinite's stuff is really friggin complex, but all you need to do is select the part you want to change colors, go to the main shader node (it'll be front and center most of the time, can't miss it), and click on the different color zones to change their color
- **2023-06-12** `_gallium` [🔍╎texturing-and-shading] OHh it was also the primary contributor to the keyword exhaustion issue back when the vrc unity verison only had a limited amount of global shader keywords available.
- **2023-06-13** `martyrthesaint` [🔍╎texturing-and-shading] using the h4 spartan pack and im noticing that the primary colors are bleeding in to secondary color areas, does anyone know why that would be?
  [attachments] image.png
- **2023-06-27** `purplmunkii` [💬╎general-help] If youre goin for a dust storm look k wouldnt tint the volume at all. Instead just do some color correction in post
- **2023-08-16** `exitity` [💬╎general-help] I was wondering if there was an easy way to adjust what parts of the armor are colored by the Secondary Color, particularly for the Halo 3 Spartan rig? I was trying to see if I could adjust the coloring to look like the Freelancers from Red vs. Blue Seasons 9-10, where the armor is modeled after Halo 3 put the color patterns are more like Halo 2 (like the armor is Halo 3 armor but their secondary color is coloring those parts on the wrist and the parts on the side of their thighs rather than the shoulders, and also the small bit on the helmet rather than the long stripe). I only know the basics of Blender so if its complicated I probably won't bother, but if its easy that would be cool.
- **2023-10-08** `blackandfan` [🔍╎texturing-and-shading] I originally thought that it is the color change but mpmask makes more sense
- **2023-10-09** `caldoric` [🔍╎texturing-and-shading] @ayomide I'm just trying to get a color control ("cc") texture map fixed. The alpha channel is set to one and should be zero, but I'm unable to get at it in GIMP. The rest of the avatar (coloring, etc) is something I can deal with myself.
  
  Also, my profile clearly states "ask before DMing, especially if I don't know you". I don't even know how you sent me a DM request, since you're definitely not on my friends list.
- **2023-10-10** `caldoric` [🔍╎texturing-and-shading] Btw, when I say "broke", I mean that the texture turned to a dark grey checkerboard and the material/shader thing I plugged it into started behaving as if I didn't have anything in the cc slot, even though it also said the texture was slotted in.
- **2023-10-21** `tacohombre` [🔍╎texturing-and-shading] funnily enough the parameters for the skin colour are just white so it doesn't even use the colour change masks
- **2023-10-31** `charmingquark09` [🔍╎texturing-and-shading] if you are using blender, the way that I have duplicate textures linked is to go into the **"shading"** tab, and at the top of the grey workspace there is this little beachball-looking icon. Click on the beachball and a drop-down menu appears. Either make a new **material **or edit an existing one and name it something easy to type. Click on each object you want to have that texture **separately **(as far as i know, blender wont let you assign a texture to multiple objects at once... which sucks. please correct me if im wrong lol) and using the beachball drop-down menu, apply the texture. 
  
  The good thing about this method is, you can edit just one material and it applies to everything you've assigned to it. Good for having a primary/secondary color that you can change on the fly, or to test out color schemes. To get rid of un-used materials, I believe there is an extension in blender that lets you delete all unused materials, but i cant remember what its called.
  [attachments] image.png
- **2023-11-03** `r93_sniper` [🔍╎texturing-and-shading] ```
    A secondary benefit of defending all users' freedom is that improvements made in alternate versions of the program, if they receive widespread use, become available for other developers to incorporate.  Many developers of free software are heartened and
  encouraged by the resulting cooperation.  However, in the case of software used on network servers, this result may fail to come about.
  The GNU General Public License permits making a modified version and letting the public access it on a server without ever releasing its source code to the public.```
- **2023-11-17** `.boe` [💬╎general-help] Does anyone have some sort of archive of every armor permutation and color that was loaded on Bungie.net service record and put in this image format? Not sure where to ask for this.
  [attachments] image.png
- **2023-11-28** `vp_snipes` [🔍╎texturing-and-shading] The normal has the Gloss map stored in the Blue channel, and your "_r" texture is your reflection map that tints the color (IE specular color mapping), so treating that as a roughness map will NOT give you the proper results.
- **2023-12-26** `inversefrog7689` [🔍╎texturing-and-shading] Alright, so this is using the H2A Elite armor on HI Elites pack made by Diego and I was wondering how do I switch the colors on the armor? When I go to the shading tab the primary color says its white (despite clearly being blue) and then when I try to click and pick a different color, it stays white unless I decided to scroll down to black on the color wheel. Any ideas in how to change that?
  [attachments] Screenshot_2023-12-26_172909.png
- **2024-01-03** `thoro.blend` [💬╎general-help] Alright, so I've seen a bunch of people have a primary and secondary "shader?" to use for the spartans. 
  
  How can I set this up as I still haven't figured out how to yet, other than making an rgb node and plugging it into every slot on *each* piece.
- **2024-01-05** `pepper.ghost` [💬╎general-help] on the right is an older version of the project before the texture issue occurred. Again I didn't mess with the textures aside from simply adjusting the primary color values.
- **2024-01-25** `soulburnin` [🔍╎texturing-and-shading] The fresnel function in reach acts as the mix between specular tint - albedo blend and fresnel color
- **2024-01-25** `gameoverload` [🔍╎texturing-and-shading] I was seeing Albedo Tint Spec used, but not IOR, which is where I was confused if they just set a global Specular f0 across the board, or the IOR value was input elsewhere in the shader stack.
  
  ..but ye, probably best to test in IRTV if I am wanting to actually tell.
- **2024-01-25** `gameoverload` [🔍╎texturing-and-shading] Now, with the IOR, below 1 does not apply the specular tint, ala white as default, to the specular color, meaning everything above 1 is the f0 multiplied by the specular tint.
  
  Which means if Specular f0 is 1, you only use the specular tint as is, which for most metal materials is .04 by default.
- **2024-01-27** `.reaper64` [🔍╎texturing-and-shading] Excuse me I need some help with a armor texture for Reach, you see I am creating one but I don't know if I am doing it right, if I got the back spine, shoes, and shoulder colors for it to be a secondary.
- **2024-02-12** `cbunny20` [💬╎general-help] so i just finished adding all of the recoloring how do i export the spartan so i can use it in unity and have all the texture and armor color changes still intact. do i need to compile the color changes i made?? what do i do
- **2024-02-21** `gameoverload` [🔍╎texturing-and-shading] Oh ye, I figured that out a while back since I assumed the base color only affected not primary / secondary color portions.
- **2024-02-21** `gameoverload` [🔍╎texturing-and-shading] Basically Base Color is the A in a mix, the factor is the first primar mask, and B is the primary color, and then the next mix has the factor be the secondary mask, and then the B of that second mix is the secondary color.
- **2024-02-21** `gameoverload` [🔍╎texturing-and-shading] So basically:
  
  Mix 1:
  Factor - Primary Color Mask
  A - Base Color
  B - Primary Color
  
  Mix 2:
  Factor - Secondary Color Mask
  A - Result of Mix 1
  B - Secondary Color
- **2024-02-23** `pendragone01` [💬╎general-help] Hey all, does anyone know if it's possible to change the permutations available in Halo Reach's Foundation? I'd like to put a Spartan Helmet on a marine, but I'm wondering if that's possible without ripping it out and creating an entirely new tag.
- **2024-02-28** `lt.pelekito` [💬╎general-help] Hello, for the Captain's Lasky's 3d model I wanted to change the color of his uniform but whoever made it made the base texure transparent and the shader changes the color using the tint. The tint changes everything not only the base color but also the gold color on the color and arms.
  [attachments] image.png
- **2024-06-15** `manon` [🔍╎texturing-and-shading] for the halo 3 spartan blend files, does anyone know how to change the tertiary colour for leg/arm?
  
  all the materials have nothing white in them, so im confused where its coming from
  [attachments] image.png
- **2024-06-25** `cythecy_` [🔍╎texturing-and-shading] Just want to double-check, I'm working with Reach assets in Blender.
  Can I assume that all the environment mapping for a single level would be the same?
  So same cubes, tints, roughness?
  I'm using the shader graph found in the Falcon asset, credit says @Soul made it
- **2024-07-04** `jacc97` [💬╎general-help] HDRI's work as a universal light source, you can find them online very easily. Your models will pick up the refelctions, colours and light to make them feel like they're more closely in the environment of your choosing. For example, your scene looks like a desert, ideally you'd find a HDRI with an overly orange colour pallet and a nice clear sky with a sun. Then it's simply a matter or rotating it to best represent your primary focus (the characters) with highlights and a clear silhouette.
- **2024-07-11** `zadeinator` [🔍╎texturing-and-shading] I see i see
  
  You'll want to set the tint colour to roughly be orange/goldish
- **2024-07-14** `matta_klark` [🔍╎texturing-and-shading] @R93_Sniper Do you know if the ODST skin specular texture is packed in some way?
  The blueish tint on it makes me think it is, but seeing at the channels can't figure what kind of packing could be
- **2024-07-14** `chiefster_4014` [🔍╎texturing-and-shading] the tinting is probably done through the shader if not the texture itself
- **2024-07-14** `soulburnin` [🔍╎texturing-and-shading] It's not, it's the actual color of the specular texture. In shader it's Specular Texture * (Analytical Specular Coefficient * Specular Tint) * Final Tint
- **2024-08-16** `soulburnin` [🔍╎texturing-and-shading] Bottom value is specular tint, top is glancing (fresnel color). The fresnel color is mixed on top of the specular tint - albedo blend. The exponent value in the bottom left is the "steepness" of the curve
  [attachments] Foundation_CfOs5f9srs.png
- **2024-08-16** `cythecy_` [🔍╎texturing-and-shading] Okay I get what you mean about the glancing/fresnel and the specular tint. The exponent appears to have its own value in the node group, but I'm still unsure what colour I'm supposed to put into the specular colour values?
  [attachments] image.png
- **2024-09-18** `vnanon.` [💬╎general-help] Trying to figure out an intuitive way to add the stripe  tertiary color from H3 to the MK VI bicep and thigh
  [attachments] image.png, t990sivdhmla1.jpg
- **2024-09-24** `badblizard` [💬╎general-help] I did that and when I mess with it the whole thing changes to that color, not just the armor pieces (I want the armor pieces to change color not the tech suit)
- **2024-10-02** `urgedfutuer117` [🔍╎texturing-and-shading] well i tired follow a video tutorial and went into the shaders section nut the visor color changed but only in the animation section
- **2024-10-19** `gwenwyn` [🔍╎texturing-and-shading] hello! i need help finding a texture from the reach spartans. for some reason one of my cc textures is fully transparent for the grenadier chest piece (file name: spartan_chest_joshua_cc.png) instead of showing the proper cc file. was wondering if anybody had it or could help me find it, since for some reason its the only thing thats screwing up the colors.
- **2024-10-28** `the_titanfist` [💬╎general-help] yeah I have my mark V B there thanks. now ill have to learn how to change color on armor. 😄
- **2024-10-28** `_themadmonk` [💬╎general-help] Hello, I'm making a quest mod for fallout 4, and most of the levels willbe 1 to 1 recreations of halo3odst. Now the primary area is an overrun city. Would the odst Mombasa streets level feel big enough for a left4dead horde esque exploration map? Or should I try and go bigger?
- **2024-11-05** `thecupocoffe` [💬╎general-help] Is it possible in Spartan Pack 3.0 to color or change a variable for every piece of armor? Like the primary of the hand and the helmet at the same time?
- **2024-12-05** `atari8879` [💬╎general-help] is there a way to color the spartans from the spartan pack easier and faster or do i have to individually figure out which color changes which part
- **2025-01-01** `chiefster_4014` [💬╎general-help] for H4's spartans, the diffuse is in the green channel of the control map and the alpha channel contains the color change zones. White areas are for primary color and grey areas are for secondary color.
  If you don't want to bother with messing with the control map there's a H4 shader you can get for Unity here: https://gitlab.com/FiyCsf/open-halo-4-spartan-shader-for-unity
  [embed] Cameron "Fiy" F. / Open Halo 4 Spartan Shader for Unity · GitLab https://gitlab.com/FiyCsf/open-halo-4-spartan-shader-for-unity This should play nicely with all Halo 4 MP Spartan Textures etc, and is written using the amplify shader editor and is clearly commented should anyone wish to...
- **2025-02-22** `jagerd274` [🔍╎texturing-and-shading] I'm new to this whole texture thing. Does anyone know how I can get this secondary color area for ODST's shoulders?
  [attachments] Hombrera_.png
- **2025-03-29** `reisan` [🔍╎texturing-and-shading] the odst i downloaded have a TIF file but I want to change some stuff/color to make it more unique. but when I load it into photoshop its all transparent and i cant barely see it. anyone know what I could do to see the texture so I can color change some stuff?
- **2025-05-15** `spartanjoe193` [🔍╎texturing-and-shading] It got it by multiplying the banshee's specular texture alpha channel and the secondary color mask.
- **2025-06-12** `dustyoo10` [💬╎general-help] Hey so which program do I use to view shader details? Things like tints, detail tiling, etc.
- **2025-07-20** `x_crimsongalaxy_x` [🔍╎texturing-and-shading] if I change the diffuse colour to the armour's primary colour, the whole thing darkens (including the secondary colour and visor and allat)
- **2025-08-07** `marcopolo1223` [💬╎general-help] Can I get some assistance for color change? I have the marine buttbag from Halo Infinite Marines for a Spartan I am creating but its only shifting different shades of grey. I cannot adjust the color at all
- **2025-10-02** `chiefster_4014` [💬╎general-help] visor_cc is just for the color change. it's used for all visors.
  the normal map and diffuse visor textures are included with the helmet textures
- **2025-10-09** `hydranaught4444` [💬╎general-help] well it's intended to be used as a color change map so as long as you set up the color change in the materials/shader you should be good to go with any color you want
- **2025-10-09** `hydranaught4444` [💬╎general-help] of course if you don't want to do that just separate the green part in gimp then use hue saturation but it would be better to set up the color change properly
- **2025-10-23** `gameoverload` [💬╎general-help] Due to her hair being a black color natively, it likely is why the hair tint does nothing.
- **2025-10-30** `gyro.stormblessed` [💬╎general-help] Hey I hope this is the right place to ask this. I am trying to use Soul's halo reach SFM models and I am having trouble trying to change the color of the armor. So far when I change the color it changes it for both the armor and the tech suit underneath, lights too. Does anyone know what I need to be doing to change specifically the primary and secondary colors on the reach armor?
- **2025-11-10** `anubisx72` [🔍╎texturing-and-shading] Blender updated a bit, and has seem to broken some textures. I know there is other ODST packs out there, but I do enjoy the pack this is from too much, I've just been copying the colors from primary/secondary into the fresno area and turning emission to 0.00 but it's still noticable.
  [attachments] Screenshot_2025-11-10_214133.png
- **2025-11-21** `scavhyena` [💬╎general-help] I would do favorite color primary and something contrasting for the trim. 
  
  Maybe some white grey or black detailing to make things pop, whichever stands out.
- **2025-11-24** `zattlebone` [💬╎general-help] For manual colour changes, that is the fast way im afraid. You could go into the group and connect stuff inside there, but thats not recommended as its easy to break stuff.
  
  If you're just using the in game coatings though, ekur is your best bet, as you can just select the armour and choose the coating you want
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
- **2025-12-06** `tristangold` [🦴╎animation] Hello! Had an animation question that's also part ripping question so please feel free to let me know if it should go into that section instead. 
  
  So I have two characters (non halo humanoids) that I have in my Blender scene and I was hoping to see if I could perform a simple assassination with - I think a Spartan v Spartan multiplayer reach one would be perfect. 
  
  I'm following along with the Plastered_CrawDAD tutorial however this seems to only apply to taking the Halo models and then through Bonobo and the other tools applying the Halo assassinations on top of them. Since my models aren't Halo related with a simple Mixamo skeleton, is it possible for me to retarget the animations or would the difference in skeletons make this impossible? 
  
  How I'm envisioning the process is A) Get 2 Halo Spartan Models, B) Apply the Assassin and Assassinated animations onto both of them. C) Retarget Animations onto secondary models once in blender properly. 
  
  Wanted to see if anyone has tried this yet
- **2025-12-10** `un_nonimus` [📜╎ripping] I mean specifically exporting the lightmap UV data from the bsp itself. Then also using the lightmaps for the specific bsps to essentially have accurate baked lighting for levels. 
  
  I know in reach for instance there are some lightmap bitmaps in the tags. However, when exporting the geometry using reclaimer that secondary set of uv data isn’t written for the models. So the lightmap textures are useless.
- **2026-01-10** `gameoverload` [💬╎general-help] If you mean changing colors, I do blame them for not allowing at minimum color change in the game itself for the coatings, they had the capacity in the engine itself, but thought they couldn't make money from it....when Warframe does, and makes a bit load of money from that.
- **2026-01-12** `welxor` [💬╎general-help] Following the textures, the only mask there with a black and white pattern (like you demonstrated) modifies only the third color (which I recolored to white)
  However there is no texture mask in black and white for the secondary color, and primary color
  [attachments] Capture1288.png
- **2026-01-12** `welxor` [💬╎general-help] This texture seem to make the difference through the secondary and primary color tho.
  But it's not in black and white, is it also a mask?
  [attachments] Capture1289.png
- **2026-01-14** `chiefster_4014` [🔍╎texturing-and-shading] Plus since it's in the base color it shouldn't be used as a spec map. It could be just the base AO of a material and it gets tinted by another parameter.
- **2026-02-26** `muskrai` [🔍╎texturing-and-shading] basically, what the pic says, i just want to change this so i can vertex color the upper legs as my secondary. wanted to mess with some shaders n stuff cuz it's really fun (i am kitbashing, cuz i had some ideas)
- **2026-02-27** `muskrai` [🔍╎texturing-and-shading] would i be able to swap the selected vertices to the secondary shader though? though im probably gonna dupe the current shader and then color swap and reassign, but wanted to see if i could do it through materials, or if i have to change it through a certain texture map?
- **2026-02-27** `mesathepostalgen` [📜╎ripping] thanks for the shading help, though i figured i could sort of... hijack the reach shader setup in blender to effect the color change, i dont do renders so i dunno how to use it, but thank you again!
- **2026-03-01** `chiefster_4014` [🔍╎texturing-and-shading] the channel/name/equivalent sorting is kinda awkward to read
  h3 CC maps can use RGB but it shows it only using RG
  reach CC maps can use RG but it shows it only using R
  i think the order for the h3 layer1 map is wrong, the roughness and metallic should be swapped. also the specular, blend, environment, roughness layout next to it is only used for H3's original in-game cook torrance shader. the in-game PBR shader 343 made is just specular & roughness.
- **2026-06-04** `chiefster_4014` [🔍╎texturing-and-shading] Trying to get Halo mats to look good in stuff that can't be given custom shaders is a pain for me. This looks pretty close though.
  Giving the diffuse for the armor material a slight blue tint might help bring back the blueish specular color.
- **2026-06-19** `yesimviolet` [💬╎general-help] Now this isn't a 100 percent accurate shader in Blender but it's the same sort of green tint in here as well
  [attachments] image.png
- **2026-06-19** `yesimviolet` [💬╎general-help] Just with my shader it's a little bit more prominent but you can definitely tell it's the same tint
- **2026-06-19** `chiefster_4014` [💬╎general-help] the cubemap itself is never tinted, it's only added onto the diffuse either before or after the detail map is applied to the base map, depending on what's set in the tag.
- **2026-07-12** `chiefster_4014` [🔍╎texturing-and-shading] the only camo map that i can tell they used is baked into their change color map ``marine_cloth_cc``
- **2026-09-02** `sev_pa_ranger` [📜╎ripping] Hey does anyone know where the h2a elite cc textures are located at in h2a as I can't seem to find them. I'm using index to look through the files but all I can find is a few for the grunts, the ranger helmet and thats about all
- **2026-09-02** `chiefster_4014` [📜╎ripping] If I remember correctly, the mask for their color change is in the alpha channel of the Diffuse map
- **2026-09-17** `lieutenantfoxtrot` [💬╎general-help] Does anyone happen to know how to change the primary and secondary colours for the Halo 3 Multiplayer Spartan model?
- **2026-09-17** `gameoverload` [💬╎general-help] It should have a color slot on the shader to change it, one for primary, one for secondary.
- **2026-09-17** `gameoverload` [💬╎general-help] Is it not a nodegroup with two slots for primary and secondary? 
  ...or at least if you make sure to back out to the top most layer of the graph, that it has a nodegroup with two color slots.
- **2026-09-17** `gameoverload` [💬╎general-help] Right, but also considering these are Halo 3 parts, might have been good to condense all of that down into one nodegroup that basically sums it down to color change, just like the regular Halo 3 stuff would have.
  
  That way people can change stuff in basic the easiest.
