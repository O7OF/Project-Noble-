# Curated resource channels (The Halo Archive Public Archive)

Full text of the shader, control-map, tutorial, tool, lavadeeto and reclaimer-updates channels, oldest first per channel.


## 📑╎control-map-guide

- **2021-12-25** `r93_sniper` [📑╎control-map-guide] Halo is weird and has historically used these weird composite textures to tell the engine how to process shading. The names change from game to game, but this channel is designed to give everyone an understanding of what each of these textures do respective to their games.
- **2021-12-25** `r93_sniper` [📑╎control-map-guide] ```Halo Combat Evolved```
  Halo CE names their composite texture maps "Multipurpose" Maps. Generally speaking the functionality doesn't change too much throughout the series but the actual contents and ordering of each of the channels does
- **2021-12-25** `r93_sniper` [📑╎control-map-guide] Because of Gearbox's fiddling, there are two types of multipurpose maps that are used for CE, and there is no way to tell which one is which unless you open up each channel. The only difference is the order of the textures within each channel
- **2021-12-25** `r93_sniper` [📑╎control-map-guide] __For OG Xbox Formatted Multipurpose Maps: __
  Red Channel - Specular Reflection Mask
  Green Channel - Self Illumination Mask
  Blue Channel - Primary Color Change Mask
  Alpha Channel - Auxiliary Mask
  
  __For Gearbox Formatted Multipurpose Maps__
  Red Channel - Auxiliary Mask
  Green Channel - Self Illumination Mask
  Blue Channel - Specular Reflection Mask
  Alpha Channel - Primary Color Change Mask
  
  __According to the HEK documentation, this is what each map is__
  **Specular Reflection Mask**: Modulates Reflections
  - This is a dielectric specular map
  **Self Illumination Mask**: Adds to Diffuse Light
  - This is an Emissive mask (glowy bits)
  **Primary Color Change Mask**: Recolors Diffuse Map
  - If it changes colors, this is the mask for it. CE only supports 1 color to be changed on any given material
  
  The Auxiliary Mask is used for any arbitrary purpose
- **2021-12-25** `r93_sniper` [📑╎control-map-guide] CE otherwise stores its transparency in the alpha of the diffuse map, wherein the RGB of the diffuse map is...well, diffuse information
- **2021-12-25** `r93_sniper` [📑╎control-map-guide] ```Halo 2```
  Halo 2 has some differences between how it handles its textures compared to CE. The first of which is that it supports _two_ colors to be changed per material rather than just one! What an upgrade right? Beyond that, Halo 2 supports Normal Maps for static and dynamic objects (CE only supported bump maps for static objects, nothing on dynamic ones). The specular map for everything is stored in the alpha of the diffuse (unless its transparent, in which case it'll be in the alpha instead)
- **2021-12-25** `r93_sniper` [📑╎control-map-guide] Unlike Halo CE, there is no multipurpose map, so the maps are as follows
  __Diffuse__
  RGB - Diffuse
  Alpha - Spec or Opacity
  
  __Normal__
  RGB - DirectX Formatted Normal (Flip Green channel in Blender/Unity)
  
  __CC__
  R - Primary Color Change Mask
  G - Secondary Color Change Mask
  B - Nothing
- **2021-12-25** `r93_sniper` [📑╎control-map-guide] ```Halo 3, ODST, and Reach```
  Halo 3, ODST, and Reach all are basically the same thing as Halo 2's set up. However there are some minor differences here and there and sometimes there are additional textures
- **2021-12-25** `r93_sniper` [📑╎control-map-guide] Here are the textures as they show up for these three games
  __Diffuse__
  RGB - Diffuse
  Alpha - Spec or Opacity
  
  __Normal__
  RGB - DirectX Formatted Normal (Flip Green channel in Blender/Unity)
  
  __CC__
  R - Primary Color Change Mask
  G - Secondary Color Change Mask
  B - Nothing
  
  __Illum__
  RGB - Self Illumination
  
  __Material__
  R - Specular Coefficient
  G - Albedo Blend
  B - Environment Contribution
  A - Roughness
  
  Not everything has an Illum texture, nor does everything have a material texture. Illum textures are not masks, they are simply the colors used fully for the self illumination (emissive) of a material. When the "Material" texture is used, the Diffuse will not have anything in the specular channel (usually)
- **2021-12-28** `r93_sniper` [📑╎control-map-guide] ```Halo 4```
  Halo 4 marks another complete shift in how the games processes their shading. This time there's less textures and a bit more consistency in how everything is used/named.
- **2021-12-28** `r93_sniper` [📑╎control-map-guide] There are two types of control maps for Halo 4
  
  __**TYPE 1** Control Map__
  R - Specular Coefficient
  G - Specular Exponent
  B - Reflection Mask
  
  __**TYPE 2** Control Map__
  R - Specular Coefficient
  G - Diffuse
  B - Specular Exponent
  A - Color Change
  
  __Diffuse Map__
  RGB - Diffuse
  A - Opacity / Illum (Depends on usage)
  
  __Normal__
  RGB - DirectX Formatted Normal
- **2021-12-28** `r93_sniper` [📑╎control-map-guide] The easy way to tell if something is a control Type 1 or Type 2 is by the additional maps with it
  
  __Type 1__: These are suffixed with "multi" and always have a separate diffuse map. They are also given static colors, ie weapons and unsc props
  __Type 2__: These do not have dedicated diffuse maps unlike the other types. All MP armor sets use type 2 control maps
- **2021-12-28** `r93_sniper` [📑╎control-map-guide] Additionally, Color change that is in Type 2 control maps are **two tone** rather than single. 
  __White__ - Primary Color
  __50% Grey__ - Secondary Color
  __Black__ - Uncolored
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] ```Halo 2 Anniversary Multiplayer```
  "Hey wait sniper, why are you calling out the multiplayer separately?" you may be wondering. Great question! The answer is that Certain Affinity made a remastered version of the Multiplayer based off of the Halo 4 engine. There's actually a GDC talk about what their tech team had to do (I actually highly recommend giving it a look if you want to get a bit of BTS for how this was made) that you can find here: <https://www.youtube.com/watch?v=072HkIJ_DMs> if you're curious. But their system was actually notably different from Halo 4's, the biggest of which being that they actually worked in PBR this time around. So instead of being Spec/Gloss this time around, it'll be Metalness and Roughness
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] So control maps, here's what they're like
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] __C1 (Control)__
  R - Metalness
  G - Ambient Occlusion
  B - Self Illumination Mask
  A - Smoothness
  
  __D (CC Map)__
  R - Primary Color
  G - Secondary Color
  B - Nothing
  A - Scratches / Wear
  
  __S__
  RGB - Dielectric Specular*
  
  __N (Normal Map)__
  RGB - DirectX Formatted Normal Map
  
  *note this is an assumption. I haven't fully nailed down what this is supposed to be. If I find out, i'll correct this guide later on (either delete the note or fix it)
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] ```Halo 5```
  Halo 5 is weird in that its clear that something happened down the pipeline, wherein 343 changed their pipeline. It probably had something to do with the release of H2A, since it seems that 5 uses a slightly more modified version of that engine branch. So what are the textures here?
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] There's 3 textures
  
  __Color__
  RGB - Albedo\**
  
  __Control for MULTIPLAYER SPARTANS__
  R - Primary Color*
  G - Secondary Color
  B - Metalness*
  A - Smoothness
  
  __Control for WEAPONS__
  R - Smoothness
  G -Ambient Occlusion
  B - Metalness
  
  __Normal__
  RGB - DirectX Formatted Normal
  
  *For helmets, Metalness for the visors' hex patterns is in the Red channel, not the blue
  **Albedo is similar to Diffuse, but there's no shadows/lighting information. It's just color. The alternate name is "Base Color"
  
  This applies specifically to the MP Spartans. I'll make adjustments later to specify how it works for weapons, vehicles, and SP spartans
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] ```Halo Wars 1```
  So I've only recently gotten my hands on Halo Wars 1 assets, so I'm going to say this as clearly as possible
  **This information may not be fully correct. Take this following section with a grain of salt**
  With that out of the way, let's see what we've got
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] Halo Wars 1 seems to have 4 or 5 textures, depending on if the model is a UNSC or Covenant model. Here's what I've found
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] __df (Diffuse Map)__
  RGB - Diffuse
  
  __sp (Specular)__
  R - Spec
  G - ???
  B - Gloss(?)
  
  __rm (Rim Map[?])__
  I'm not fully sure what this does. Looks to control Covenant shell iridecence?
  
  __nm (Normal Map)__
  RGB - DirectX Formatted Normals*
  
  
  __xf__
  RGB - (Inverted) Color Change Mask
  
  *In order to make the normal map work, you'll want to make blue channel white or properly normalize it.
  If anyone who's a bit more knowledgeable about this subject is willing to inform me about what's used for what, I'd like to get this as guide as accurate as possible
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] ```Halo Wars 2```
  One would expect that Halo Wars 2 would have a similar or even identical setup to its predecessor considering it's just an RTS game. However, that's not the case at all. Also unlike Halo Wars 1, I've actually had some experience on setting the models up to be used, so I actually know what i'm doing
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] The one thing that remains the same between the two games is that there's 5 textures
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] __[al]__
  RGB - Albedo
  
  __[em]__
  RGB - Emissive Color
  
  __[nm]__
  RGB - DirectX Formatted Normals
  
  __[sp]__
  RGB - Specular Map
  
  __[pm] (Packed Map)__
  R - Gloss
  G - Ambient Occlusion
  B - Color Change Mask
  
  This one is a lot more straightforward. Though much like Wars 1, you need to convert the blue channel of the normal map.
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] ```Halo Infinite```
  And now to the one thing that everyone and their mother is going to be asking about. But before I go and explain this out, I'd like to make something apparent real quick. Depending on what program you're using, there are pre-made infinite shaders for you to use and play with:
  __Blender__ - @Chunch made a shader that's available to use here: 
  https://discord.com/channels/819709630540021810/821051966058004501/948261628573532161
  __Unreal Engine__ -  @Sketch10121 has made a large UE4/5 Virtual Production project with infinite assets that you can grab here: 
  https://discord.com/channels/819709630540021810/821676515099279374/929395385863143464
  __Unity__ - @PointDevice and I have a free, open source, public shader pinned in the #🎮╎unity channel. Check the pins or click here:
  https://discord.com/channels/819709630540021810/825398188743458826/911430857384935424
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] Now, to the meat and potatoes of this section. The thing to be aware of is that Halo Infinite uses what are called Procedural Materials (ProcMats for short). This basically means that instead of just having a base texture to color, the entire texture is built up by defining at runtime what kind of material any given pixel is. This has been validated by actually pulling out the information about coatings.
  
  That out of the way, lets go
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] __ASG__
  R - Ambient Occlusion
  G - Scratches / Wear
  B - Grime / Dirt
  
  __Mask0 (CC1)__
  R - Zone 2
  G - Zone 3
  B - Zone 4
  
  __Mask1 (CC2)__
  R - Zone 5
  G - Zone 6
  B - Zone 7
  
  __Normal__
  RGB - DirectX Formatted Normal
  
  The big thing to note is that **Zone 1 is built procedurally**. You need to add up all the other zones together and invert it to build it.
  One thing I want to also note is that I don't define any specifics as to what each zone is. This is because this is variable on weapons and vehicles, whereas it's mostly standardized for spartans.
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] Thank you for listening to my ted talk
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] im going to go die now
- **2022-01-05** `r93_sniper` [📑╎control-map-guide] 

## 📗╎lavadeeto-resources

- **2021-07-05** `lavadeeto` [📗╎lavadeeto-resources] K so I want to keep this as a resource or source for not super important notes or info, but stuff that actually can be helpful.
- **2021-07-05** `lavadeeto` [📗╎lavadeeto-resources] So first...
- **2021-07-05** `lavadeeto` [📗╎lavadeeto-resources] Notes: 
  Halo uses World Units (wu) as its sizing in all Halo Games (Halo CE - Halo 4). I used the vertex points on nodes as reference to see how big 1wu is in both 3ds and Blender. Also Blender is different as the AMF importer increases the size to what was consdired the standard at the time which MC was 7ft or 2.14m, but as shown Masterchief isn't this anymore:
  
  Masterchief Lore Height in armor: 
  ``` 7ft 2in / 86gu/in    | 2.18m / 218cm```
  
  ```1 wu (World Unit) = 
  3ds Max = 100gu/in   | Blender / Unity = 2.54m```
  
  Doing this I looked into the height of Masterchief as his perspective/sizing is what the each game is based around. I took the tallest vertex as this would be the height of Masterchief's model from each game.
  
  Heights (Represented by the tallest vertex on the model):
  
  World Units:
  
   ``` 0.70362wu    | Halo CE Masterchief Model
   0.70364wwu   | Halo 2 Masterchief Model
   0.7027779wu  | Halo 3 Masterchief Model
   0.71048wu    | Halo 4 Masterchief Model
  ```
  
  3ds Max:
  
   ``` 70.362gu/in  | 5ft 10in  | Halo CE Masterchief Model
   70.364gu/in  | 5ft 10in  | Halo 2 Masterchief Model
   70.278gu/in  | 5ft 10in  | Halo 3 Masterchief Model
   71.048gu/in  | 5ft 11in  | Halo 4 Masterchief Model
  ```
  
  Blender:
  
   ``` | 2.1446m  | Halo CE Masterchief Model
   | 2.1453m  | Halo 2 Masterchief Model
   | 2.1427m  | Halo 3 Masterchief Model
   | 2.1661m  | Halo 4 Masterchief Model
  ```
  
  Now after looking into it the difference between importing into either 3ds or Blender is different, but neither is lore accurate. So to help this I will have different ratios based on engine below:
  
  So I created a percentage ratio that is based on what he is ingame resized to lore accurate heights.
  
  Ratios %s (3ds Max):
  
  ``` 1.220%  | 70.362gu/in x 1.220% = 86in/gu / 7ft 2in  | Halo CE
   1.220%  | 70.364gu/in x 1.220% = 86in/gu / 7ft 2in   Halo 2
   1.221%  | 70.278gu/in x 1.221% = 86in/gu / 7ft 2in   Halo 3
   1.208%  | 71.048gu/in x 1.208% = 86in/gu / 7ft 2in   Halo 4```
  
  Ratios %s (Blender):
  
  ``` 1.0165%  | 2.1446m  x 1.0165% = 2.18m / 218cm   | Halo CE
   1.0162%  | 2.1453m x 1.0162% = 2.18m / 218cm    | Halo 2
   1.0174%  | 2.1427m x 1.0174% = 2.18m / 218cm    | Halo 3
   1.0064%  | 2.1661m x 1.0064% = 2.18m / 218cm    | Halo 4```
  
  Now you can use these if you want as accurate as you can be based on each game, but I have a proposed / universal one for all games.
  
  Proposed: 
  
  ```3ds Max: 1.22% | Blender: 1.0165%```
  
  Ratio Differences (3ds Max):
  
  ```
  1.220% (Halo CE)
  
   86.679gu/in  | 7ft 3in | Halo 4 (Tallest)
   85.739gu/in  | 7ft 2in | Halo 3 (Shortest)
  
  1.220% (Halo 2)
  
   86.679gu/in  | 7ft 3in | Halo 4 (Tallest)
   85.739gu/in  | 7ft 2in | Halo 3 (Shortest)
  
  1.221% (Halo 3)
  
   86.750gu/in  | 7ft 3in | Halo 4 (Tallest)
   85.810gu/in  | 7ft 2in | Halo 3 (Shortest)
  
  1.208% (Halo 4)
  
   85.826gu/in  | 7ft 2in | Halo 4 (Tallest)
   84.899gu/in  | 7ft 1in | Halo 3 (Shortest)
  
  Differences:
  
   0.940gu/in  | 6%    | Halo CE
   0.940gu/in  | 6%    | Halo 2
   0.940gu/in  | 6%    | Halo 3
   0.927gu/in  | 7.3%  | Halo 4 ...[cut]
- **2021-07-06** `lavadeeto` [📗╎lavadeeto-resources] I choose Halo CE's ratio because the established lore height 7ft 2in / 218cm / 2.18m was based on the Mark V in Halo CE and kept as the standard for his height for every game, but also because the differences with Halo 3 being the shortest height and Halo 4 bieng the tallest, is shown to be basically negligible as its 1,000ths of a percent for 3ds and 10,000ths of a percent for blender difference between the end results for each games ratio. To me that's a very negligible difference
  
  You can choose to use the proper conversions and the information from these math solutions as you wish, but the math shows the difference isn't as big as you can see.
- **2021-07-08** `lavadeeto` [📗╎lavadeeto-resources] ```Halo Colors```
  So Im going to be giving a hex code and color to match for people who want the actual color code used in the games, as not everyone knows the actual colors and makes it easier for referencing.
- **2021-07-08** `lavadeeto` [📗╎lavadeeto-resources] ```Halo: CE Colors```
  ```ffffff | white
  000000 | black
  fe0000 | red
  0201e3 | blue
  707e71 | gray
  ffff01 | yellow
  00ff01 | green
  ff56b9 | pink
  ab10f4 | purple
  01ffff | cyan
  6493ed | cobalt
  ff7f00 | orange
  1ecc91 | teal
  006401 | sage
  603814 | brown
  c69c6c | tan
  9d0b0e | maroon
  f5999e | salmon
  576725 | chief```
  [attachments] unknown.png
- **2021-07-08** `lavadeeto` [📗╎lavadeeto-resources] ```Halo 2 Colors```
  ```FDFEFF = White
  535353 = Steel
  BE2C2C = Red
  F57A1F = Orange
  F5D22C = Gold
  9FAC59 = Olive
  21922F = Green
  235644 = Sage
  16A0A0 = Cyan
  36747A = Teal
  416C8F = Cobalt
  28459B = Blue
  6A4EB6 = Violet
  75466D = Purple
  FB9BC9 = Pink
  981244 = Crimson
  664E3E = Brown
  B19256 = Tan
  
  Team Colors
  
  C13E3E = Red
  3639C9 = Blue
  C9BA36 = Yellow
  208A20 = Green
  B53C8A = Purple
  DF9735 = Orange
  744821 = Brown
  EB7EC5 = Pink
  D2D2D2 = White```
  [attachments] unknown.png

## 📰╎reclaimer-updates

- **2021-04-18** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer** release notes and newest version can always be found here:
  https://github.com/Gravemind2401/Reclaimer/releases
  
  **Architect** release notes and newest version can always be found here:
  https://github.com/Gravemind2401/Reclaimer.Architect/releases
  
  **Mirror** for latest Reclaimer related downloads (including import scripts) here:
  https://www.mediafire.com/folder/dabvr2j5ofe73/Reclaimer_Downloads
- **2021-04-21** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.6.249** is now available with the following changes;
  - fix crash on certain H4 models
  - improved material detection on Halo 4 to reduce instances of the control map being used instead of diffuse
    -- AMF exports on H4 models will now include normal maps and detail maps where possible
  - fix Halo 1 and 2 AMF exports not setting the material name if the material's bitmap could not be loaded
  - add support for exporting bitmaps as TGA
  - add support for U8V8 bitmap format
  - add support for Halo 2 Beta render_model tags
  - add proper bitmap, model and bsp support for most recent version of H4MCC and H2AMP
  
  **Architect v1.4.178** is also available to maintain compatibility with Reclaimer changes as well as add initial scenario editing support for H4 and H2AMP
- **2021-04-23** `gravemind2401` [📰╎reclaimer-updates] Just uploaded a hotfix for Reclaimer (1.6.250) to resolve a handful of H4/2A MCC models not working, and also one for Architect (1.4.183) to resolve some models loading with the camera out of bounds.
- **2021-05-08** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.6.274** is now available with the following changes;
  - update settings viewer to honor theme colours
  - setting names displayed in a more user friendly way
  - output viewer will pop up automatically to display certain errors
  - batch extract data folder no longer gets changed if you choose a different folder
    - an `Auto Update Data Folder` setting has been added to opt in to the previous behaviour
  - generated collada files will now contain vertex position W data as vertex colour R where applicable
  - the right-click menu in the tag list now includes a `Copy Path` option
  - Halo 2 bump maps now use the appropriate colour palette
  - support for custom meta viewer xml folders
    - profiles for each game can be configured via `settings > meta viewer > plugin profiles`
    - a game can be assigned to multiple profiles, and will search them in order from top to bottom to find the relevant xml file
    - the meta xml files that were included in the installer have been removed, the plugin directory can be set to use Assembly's plugins instead
    - by default there will be a number of profiles already configured to match Assembly's directory structure
  - batch extraction can now extract multiple tags concurrently
    - this behaviour is opt-in, by default it will be sequential to avoid high cpu usage
    - the concurrency can be configured in `settings > batch extractor > batch workers`
  - added ability to check for updates under the `tools` menu
    - updates can be checked automatically on startup by enabling `settings > reclaimer > auto updates check`
    - the auto check will not happen more than once in 12 hours
    - when an available update is detected the status bar will change colour and show a message
    - when an update is available the `check for updates` menu will be replaced with `view update details`
  
  **Architect v1.4.186** is also available to maintain compatibility with Reclaimer changes as well as displaying squad indexes in the hierarchy
- **2021-05-16** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.6.285** is now available with the following changes;
  - fix some Halo 5 bitmaps not exporting correctly
  - add global extract toolbar button for map, module and pak files
  - support batch extraction from pak files
  - fix obj not always exporting correctly after previous update
  - fix crash opening meta viewer when the xml file no longer exists
  - fix meta viewer duplicating items in the block selection dropdown
  
  **Architect v1.4.194** is also available to maintain compatibility with Reclaimer changes as well as adding the ability to edit string/stringID fields such as variant names
- **2021-05-21** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.6.294** is now available with the following changes;
  - change default Assimp Scale setting to match Halo scaling
  - add `[0]` to the end of bitmap filenames in collada and mtl files for multi-bitmaps
  - omit settings from the `settings.json` file when they have not been changed from the default
  - added a Max Recent Files setting
  - fix map-wide extract not working with multiple maps open
  - fix deswizzling of small A8Y8 and Y8 bitmaps
  - fix a number of Halo 5 models not loading, such as forge objects
  
  **Architect** will need to be updated after installing the Reclaimer update
- **2021-05-22** `gravemind2401` [📰╎reclaimer-updates] just uploaded a new version that supports the latest flight maps
- **2021-05-31** `plastered_crab` [📰╎reclaimer-updates] Wiki for Reclaimer
  https://github.com/Gravemind2401/Reclaimer/wiki
  [embed] Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/wiki A Halo IDE. Contribute to Gravemind2401/Reclaimer development by creating an account on GitHub.
- **2021-06-03** `surasia` [📰╎reclaimer-updates] **Reclaimer v1.6.297** is now available with the following changes;
  
  -   minor model viewer fixes
  -   fix batch extract folder prompt not being honored without AutoDataFolder enabled
- **2021-06-25** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.6.298** is now available to support the latest MCC update
  **Architect v1.4.199** is also available, adds block index prefix to area names and spawn location names
- **2021-07-18** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.7.305** is now available with the following changes;
  - add support for MCC Halo1 maps
    - does not currently include bsps
  - add support for Halo1 PC beta
  - add 'export selected bitmaps' option to model viewer
    - this exports all bitmaps that are used by the currently visible meshes
  - include mipmaps in extracted DDS files for Halo 1 and 2 (excludes cubemaps)
  - include all frames of array bitmaps in extracted DDS files for Halo 3+
    - mipmaps of each frame will also be included when extracted from MCC
  - fix a small number of Halo 5 models not loading
  - fix gbxmodel tags not loading on x360 CEA maps
  
  **Architect v1.4.200** is also available with the 'export selected bitmaps' option
- **2021-07-20** `gravemind2401` [📰╎reclaimer-updates] **Architect v1.4.200.1** has been uploaded to resolve a build issue in the first upload that prevented models from loading
  Update: currently in Architect the "export selected geometry" button doesn't seem to be working, but you can still use the "export all" button
- **2021-08-07** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.7.307** is now available with the following changes;
  - fix standard model export producing empty files
  - include markers when exporting only parts of a model
  - fix vertex weights on certain H1 models
  
  **Architect v1.4.201** is also available to fix partial model exports being inverted
- **2021-10-17** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.7.309** is now available with the following changes;
  - add support for season 8 maps
  - fix weighting on H2 rigid meshes
  - fix inside out H1 meshes
  - fix external bitmaps in H1 custom edition campaign maps
- **2021-11-04** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.7.310** is now available to resolve an issue with some Halo 2 UI bitmaps
- **2021-11-14** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.7.311** is now available to add support for the November MCC update
- **2022-04-30** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.7.794** is now available to add support for the latest MCC builds of Halo 3, ODST and Reach
- **2022-07-04** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.7.800** adds support for the latest MCC builds of Halo 3
- **2022-07-09** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.7.802** adds support for the latest MCC builds of H4/H2AMP
- **2022-08-13** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.7.806** is now available with the following changes;
  - Fix MCC H1 internalized bitmaps
  - Add missed MCC H3 builds for custom maps
  - Add missed MCC H3ODST builds from last title update
- **2022-09-04** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v1.7.808** is now available with the following changes;
  Limited forward compatibility for MCC map files
  - _MCC title updates will no longer require Reclaimer to be updated if there are no breaking changes in the map file structure_
  - _Compatibility is based on the MCC installation's folder structure and will not necessarily work for map files that have been moved or copied elsewhere_
- **2023-07-23** `plastered_crab` [📰╎reclaimer-updates] Reclaimer has been updated to work with the latest MCC maps thanks to @Gravemind2401
- **2024-08-17** `gravemind2401` [📰╎reclaimer-updates] https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.0.0-beta
  Beta release of Reclaimer v2.0, **read the upgrade notes before installing**.
  I'm looking for feedback on any potential issues with the RMF importer for both Blender and 3DS Max users.
  [embed] Release v2.0.0-beta · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.0.0-beta Important notes on upgrading from previous versions:
  
  This version requires .NET Desktop Runtime 7 to be installed which you can get from here under the ".NET Desktop Runtime 7.0" sectio
- **2024-11-01** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v2.0.1506** is now available with the following changes;
  - New icons
  - New themes
  - New bug fixes
  - Expanded CEA support
  - Expanded Halo 5 support
  - Added support for Halo Infinite
  - Scenario tag support
  - New model export format (RMF) with new Blender/Max importer addon
  
  The full change list is much longer; you can see the rest here: https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.0.1506
  [embed] Release v2.0.1506 · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.0.1506 Important notes on upgrading from versions earlier than 2.0:
  
  This version requires .NET Desktop Runtime 7 to be installed which you can get from here under the ".NET Desktop Runtime 7.0.x&q
- **2024-11-16** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v2.0.1536** is now available with the following changes;
  - RMF now exports an extra color channel for h4 and h5 models that use position W
  - Added `Export Metadata` option to right-click menu for exporting to JSON without needing to open the tag
  - Expanded decorator model support to work for Reach and h4
  - Added `Open Module Folder` option in the File menu
  - The 'add linked module' button in module tag list now has an option to add a folder
  - Improved bitmap support for infinite
  - Various fixes
  
  <https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.0.1536>
- **2025-02-09** `gravemind2401` [📰╎reclaimer-updates] **Reclaimer v2.1** is now available, with the main changes being support for Halo 2 Vista and Halo 2 MCC (finally) as well as a new map browser feature
  
  <https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1608>

## 🔗╎shaders-resources

- **2021-03-15** `plastered_crab` [🔗╎shaders-resources] Halo 5 Unity Shader
  
  Ask @FiyCsf#6054 for help
  
  https://gitlab.com/FiyCsf/open-halo-5-spartan-shader-for-unity
  [embed] Fiy Csf / Open Halo 5 Spartan Shader for Unity https://gitlab.com/FiyCsf/open-halo-5-spartan-shader-for-unity Created By PointDevice And FiyCsf For Use With Halo 5 Spartan Models in Unity. This should play nicely with all Halo 5 MP Spartan Textures etc,...
- **2021-03-16** `plastered_crab` [🔗╎shaders-resources] Halo Wars 2 shader! Thanks to @VelociQrow for making it 🙂
  [attachments] hw2_shader_Credit_to_WolfFire.blend
- **2021-03-16** `plastered_crab` [🔗╎shaders-resources] [attachments] 7d52af8cdc62455818f8cef8dc0153d4.png
- **2021-03-16** `surasia` [🔗╎shaders-resources] TheJudSub's Halo Reach shader :
  [attachments] Halo_Reach_Shader_Nodegroup.blend
- **2021-03-16** `surasia` [🔗╎shaders-resources] TheJudSub's Halo 2 Anniversary shader :
  [attachments] Halo_2_Anniversary_Shader_Nodegroup.blend
- **2021-03-23** `surasia` [🔗╎shaders-resources] Halo 5 Shader; Includes Weapons, vehicles, armors, campaign characters and more! Shader by Devarture
  https://drive.google.com/file/d/1KI_k7D64GgJc68HcEVhyv51BYdiqs4Qe/view?usp=sharing
- **2021-06-08** `surasia` [🔗╎shaders-resources] Halo Wars 2 Banished Shader. Credit to @VelociQrow for the original shader, @Deleted User for the color ramp fix and @Blackandfan for the RGB codes for the banished
  [attachments] HW2BanishedShader.blend
- **2021-06-09** `surasia` [🔗╎shaders-resources] Halo CE Unreal Shader by @TJ Sauce 
  
  https://drive.google.com/file/d/1XdcO1773T5l3irBo8PD3BZ-L13luyCuc/view?usp=sharing
  
  
  This archive contains the UE4 assets necessary to recreate Halo's cyborg shader (Master Chief) in Unreal Engine 4. The static meshes are an HD rework I did a while back. I've included both the stock textures and my upscaled textures, as well as parameters to change the appearance of the armor and visor...
  There's options to use the Xbox or Gearbox shader, as well as HDR compensation.
  [embed] Halo CE - cyborg HD UE4.zip https://drive.google.com/file/d/1XdcO1773T5l3irBo8PD3BZ-L13luyCuc/view?usp=sharing
- **2021-06-28** `fiycsf` [🔗╎shaders-resources] **Current Open Halo Unity Shaders (Unity):** 
  All of these use unity's standard lighting models, which mean these shaders behave like unity standard would if you were to convert to them externally. That's my general recommendation, but if you'd rather use a shader, here they are:
  
  *(Message current as of 28-02-2022):*
  
  **Multiplayer Spartan Shaders;**
  Open Halo 5 Spartan Shader for Unity  (V1.6):
  https://gitlab.com/FiyCsf/open-halo-5-spartan-shader-for-unity
  Open Halo 4 Spartan Shader for Unity (V0.5):
  https://gitlab.com/FiyCsf/open-halo-4-spartan-shader-for-unity
  Open Halo Reach Spartan Shader for Unity (V0.8):
  https://gitlab.com/FiyCsf/open-halo-reach-spartan-shader-for-unity
  
  **Object Shaders;**
  Open Halo 5 Weapon Shader for Unity (V0.5):
  https://gitlab.com/FiyCsf/open-halo-5-weapon-shader-for-unity/-/tree/master
  .
- **2021-07-04** `chiefster_4014` [🔗╎shaders-resources] A .blend file containing some Halo shaders I've made over time. The file will be updated as I fix/add things, so check back every now and then. There's one shader for objects, vehicles, etc and a work in progress one for terrain. 
  Shaders available for Halo 3, ODST, Reach and 4. 
  https://drive.google.com/file/d/1bJJnAItSgoIgPvphSMVdX5QBVkJ-YUNC/view?usp=sharing
  [embed] Chief's Halo Shaders.zip https://drive.google.com/file/d/1bJJnAItSgoIgPvphSMVdX5QBVkJ-YUNC/view?usp=sharing
- **2021-08-28** `lavadeeto` [🔗╎shaders-resources] ```Lavadeeto’s Halo 3 - Spartan Shader 1.0 for UNITY```
  ```Make sure to read documentation down below for specific material setup!```
  Documentation: https://docs.google.com/document/d/1kfCLkQWxZGDwP38FSXJa2svR3X_N5XYP-qcup4V7rXA/edit?usp=sharing
  
  **WARNING**
  
  This works only with Halo 3 Spartans assets. Will not work correctly with Elites (coming soon) or any other game assets. If you're having issues, reference the troubleshooting section below. 
  
  > *Hey guys just want to say first and foremost thanks for waiting patiantly for this. I'm sorry it's taken as long as it has but now the wait is over. In the mean time enjoy and create some cool halo 3 spartans!
  > 
  > Currently this is the most accurate 1:1 Halo 3 armor shader you can get as its utilizing the same shader functions and compilation as Halo 3! 
  > 
  > I would also like to thank @JackOfChaos  and @PointDevice for the help on making this shader possible. Without them it would of not gotten as far as it has.*
  
  **Features**
  
  > **All in 1 shader!** No need for multiple shaders for armor, metal, undersuit or visor materials now!
  > **Full control!** Able to choose armor AND visor colors!
  > **New MCC armor support?!** Works well with the newer armors in MCC, so you can show off with new or old sets!
  > **Visor Colors?!** You heard that right, you can choose between default gold, custom colors, or even cubemap colors! And for extra fun they can even be animated!
  > **Pulsing Lights!** You can make your shield lights blink with a speed of 0 to infintiy!
  > **Perfect for VRC avatars!** Their is support for fallback lighting and reflection cubemaps so no matter what world you'll look just as good in a normal world!*
  
  Download Below!
  [embed] Lavadeeto’s Halo 3 - Spartan Shader https://docs.google.com/document/d/1kfCLkQWxZGDwP38FSXJa2svR3X_N5XYP-qcup4V7rXA/edit?usp=sharing Lavadeeto’s Halo 3 - Spartan Shader 1.0 for UNITY  (REMEMBER: apply your lookup tables c78d78, cc0236, dd0236 or else it won’t look right.)  FUNCTIONS:  Shader Type - 
  [attachments] Lavadeetos_Halo_3_-_Spartan_Shader_1.0.zip
- **2021-08-28** `lavadeeto` [🔗╎shaders-resources] [attachments] unknown.png
- **2021-11-29** `plastered_crab` [🔗╎shaders-resources] Blender now supports a 3 point lighting setup addon that is easy to install and use. It can help make very simple renders of a single Spartan *pop*
- **2021-11-29** `plastered_crab` [🔗╎shaders-resources] [attachments] unknown-30.png
- **2021-11-29** `plastered_crab` [🔗╎shaders-resources] [attachments] unknown-32.png
- **2021-11-29** `plastered_crab` [🔗╎shaders-resources] [attachments] unknown-42.png
- **2021-11-29** `plastered_crab` [🔗╎shaders-resources] This has been a public announcement on behalf of our 3-point light fan guy Williaso
- **2022-03-01** `surasia` [🔗╎shaders-resources] https://github.com/Theathkill9512/Halo-Infinite-Modular-Shader/releases
  [embed] Releases · Theathkill9512/Halo-Infinite-Modular-Shader https://github.com/Theathkill9512/Halo-Infinite-Modular-Shader/releases A Collection of various Halo Infinite Shaders I have worked on (and am working on)  - Theathkill9512/Halo-Infinite-Modular-Shader
- **2022-03-01** `surasia` [🔗╎shaders-resources] https://github.com/R93Sniper/HaloInfiniteVRCShader
  [embed] GitHub - R93Sniper/HaloInfiniteVRCShader: An open source unity shad... https://github.com/R93Sniper/HaloInfiniteVRCShader An open source unity shader designed to emulate the look of Halo Infinite&#39;s shader system - GitHub - R93Sniper/HaloInfiniteVRCShader: An open source unity shader designed to 

## 🔗╎tutorials-resources

- **2021-03-11** `plastered_crab` [🔗╎tutorials-resources] Tutorial on using Reclaimer for ripping Halo models (includes the .map files needed)
  https://www.vg-resource.com/thread-38349.html
  
  Reclaimer program (made by Gravemind2401)
  https://www.mediafire.com/folder/dabvr2j5ofe73/Reclaimer_Downloads
  
  Halo .map files
  https://drive.google.com/folderview?id=1dEkvzYbbfsaiwVa473Xcl4o0KzJUbJ2u
  [embed] MediaFire https://www.mediafire.com/folder/dabvr2j5ofe73/Reclaimer_Downloads MediaFire is a simple to use free service that lets you put all your photos, documents, music, and video in a single place so you can access them anywhere and share them everywhere.
- **2021-03-14** `plastered_crab` [🔗╎tutorials-resources] Brief tutorial on ripping from Halo Online below
- **2021-03-14** `plastered_crab` [🔗╎tutorials-resources] [attachments] unknown-128.png
- **2021-03-14** `plastered_crab` [🔗╎tutorials-resources] https://github.com/Lord-Zedd/Assembly
  [embed] Lord-Zedd/Assembly https://github.com/Lord-Zedd/Assembly Multi-Generation Blam Engine Research Tool. Contribute to Lord-Zedd/Assembly development by creating an account on GitHub.
- **2021-03-14** `plastered_crab` [🔗╎tutorials-resources] https://github.com/TheGuardians/TagTool
  [embed] TheGuardians/TagTool https://github.com/TheGuardians/TagTool Contribute to TheGuardians/TagTool development by creating an account on GitHub.
- **2021-06-16** `plastered_crab` [🔗╎tutorials-resources] [Reclaimer Tutorial]
  Quick tutorial I made on how to get assets using Reclaimer. Everything you'll need will be in the description of the video. Prepare for a *lot* of ummmms lol
  https://youtu.be/LVmxTNp4pYs
  [embed] How to Extract Halo models using Reclaimer (quick rundown) https://www.youtube.com/watch?v=LVmxTNp4pYs I realize after the fact that my screen recording didn't show the menu with all the .map files lol. 
  
  Download Reclaimer here: https://www.mediafire.com/folder/dabvr2j5ofe73/Reclaimer_Downloads
  
  Ge
- **2021-06-23** `plastered_crab` [🔗╎tutorials-resources] Here is another crappy tutorial, this time for porting Halo 3 maps to Blender and setting them up. Obligatory "Um" warning as well as a default Windows 10 screen recorder warning (can't change windows at all and doesn't show menus, but this will do until I get my actual desktop out of storage 🤷🏻‍♂️)
  https://youtu.be/JwP_CgivSuY
  [embed] How to import Halo 3 maps to Blender (quick overview) https://www.youtube.com/watch?v=JwP_CgivSuY Due to me being a pleb and unable to get Shadowplay working I have to use the built in screen recording of Windows 10 which doesn't allow for recording multiple screens OR in-program menus so sorry if i
- **2021-07-06** `plastered_crab` [🔗╎tutorials-resources] [Halo Animation Ripping tutorial 1]
  Thanks to @PlasmaGiant#8981 for taking the time to provide a brief rundown of ripping animations with Bonobo and porting them to Blender!
  
  "So what i found first was this program called Bonobo, an animation extractor program. It essentially reads the .map files for the Bungie Era games for you to either transfer them between games, or export them out as a separate file. In my case, I needed a walk cycle from a Jackal, so I had all the .jma files from a Jackal. You actually get all animations from one character, so pick and choose what you want.
  
  http://forum.halomaps.org/index.cfm?page=topic&topicID=51196
  Heres the link to that
  
  Now after that, you need to get the .map files from the games, which you can get here
  https://haloce3.com/xmf/
  
  Lets use Halo Reach as an example, here I needed the level, any level will work as long as it has the object with the animation you need, so here is what I needed to grab:
- **2021-07-06** `plastered_crab` [🔗╎tutorials-resources] [attachments] Map_Tutorial_2.png
- **2021-07-06** `plastered_crab` [🔗╎tutorials-resources] Here is an example list, so I needed to get Winter Contingency, the first level where a Jackal appears. So you need to grab the map file for that level, towards the bottom, as well as 'Mainmenu', 'Campaign', and 'Shared'
  
  Be sure to keep all these files in the same folder. With that, you can use Bonobo to open the Winter contingency level, fine the tag with the object you need and extract the source files. Follow the link with the download for Bonobo, its very straight forward.
  
  Ok so youll have the .jma files, but you cant open them in any program natively. So you have two options. If you use 3DSMAX, you can use a maxscript that comes with it to import it into the program. I found this kinda difficult and cumbersome, so heres a better way:
  
  so in Blender, lovely people made a Halo Import/Export resource addon, you can get it here:
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
- **2021-07-06** `plastered_crab` [🔗╎tutorials-resources] So you have to manually install it to work, as I had trouble with automatic as well.
  To quote moderator Gallium:
  
  "Download the zip of the addon, extract it.
  Locate your blender addons folder, assuming windows it will be %USERPROFILE%\AppData\Roaming\Blender Foundation\Blender\
  In there there may be multiple directories with the same names as blender versions, go into the one that you use, then into scripts/addons
  Move the folder called io_scene_halo into the addons directory.
  In blender's preferences go to the addons section, click refresh if you did this while blender was open. Search for halo, enable the addon."
- **2021-07-06** `plastered_crab` [🔗╎tutorials-resources] Im using 2.93, so it works for the newest version
  
  From there you can import the .jma file as a armeture with keyframes, and then you can just copy and paste those into a fbx model from the ripper resource downloads, in my case a Jackal!
  
  From there, you can do whatever you want with it. I used "unarmed patrol walk front", and I slowed it down"
- **2021-07-06** `plastered_crab` [🔗╎tutorials-resources] https://cdn.discordapp.com/attachments/819714264922062868/862015402082304010/Jackal_Walk_WIP0001-0070.webm
  [embed] https://cdn.discordapp.com/attachments/819714264922062868/862015402082304010/Jackal_Walk_WIP0001-0070.webm
- **2021-07-07** `surasia` [🔗╎tutorials-resources] Halo 2 : Anniversary model extraction tutorial by @Jackal 
  
  
  1. Make sure your ninjaripper target is set to: Steam\steamapps\common\Halo The Master Chief\MCC\Binaries\Win64 and it will be whatever is called something x64. You'll know it when you see it.
  
  2. set the inject mode from Intruder Inject to D3D11
  
  3. Hit run. It will start MCC normally, but shortly after the splash screen loads, it will come up with an error. This is due to the Anti-Cheat blocking it (don't worry, perfectly fine). However, you've planted a little .dll in the folder (same folder as the .exe) that does the magic, so when you go to run MCC, make sure to launch the Anti-Cheat disabled version.
  
  4. Boot up the map you know what you want to rip is on.
  
  5. Get to said Item/Vehicle/Weapon/Character/Enemy
  
  6. Hit your Forced Rip button. The game should start running at about 2FPS for a bit. I tend to just give it a good 10 minutes before closing the game
  
  7. Boot up Noesis and head to your rip directory
  
  8. filter by size and just go through all the models and textures until you find what you're looking for.
  
  9. Export as OBJ
  ---------------------------------------
- **2021-07-07** `surasia` [🔗╎tutorials-resources] ---------------------------------------
  
  Things to note
  
  1. The game uses 4 maps. Diffuse, Specular, Normal and Emissive. 
  Diffuse looks how you would expect it to, same with emission. Specular maps looks like a different more intense version of the Diffuse (will post examples) and the normals are yellow. Everything should work how it's supposed to, the only thing you have to do is take the normal map into photoshop and invert the colours, (ctrl+i unless changed otherwise) and then it will look more familiar. Just save the texture and you're good to go.
  
  2. Very Important!; Make sure when you import the model, in blender, head to Object Geometry Data (The green triangle on the right), click on Geometry Data and make sure you hit the Clear custom set normals button for every mesh. If not, this will make your object have a really weird black shadow effect 
  
  3. There is a shader that is used for Blender made by Jud that you can just plug the corresponding textures into (make sure to turn the emission down when plugging it into the material output as it will make your model completely white((very noticeable)). This is can be found at #🔗╎shaders-resources
- **2021-07-09** `plastered_crab` [🔗╎tutorials-resources] [Complex overview tutorial of Model porting (Halo 3)]
  Here is a more in-depth video about setting up models and maps from Halo 3 in Blender. 
  
  It's long as fuck, but it goes into all the steps needed to get things setup with all their textures, tiling and other Blender quirks too. I added timestamps to help you navigate to the various chapters so you don't have to scrub through the 1.5 hour long video to find a certain section.
  
  It was all in one take so I tried my best to keep it somewhat structured 🤷🏻‍♂️
  https://youtu.be/aYnVWsOZHlE
  [embed] How to port Halo 3 models into Blender (Full in-depth process) https://www.youtube.com/watch?v=aYnVWsOZHlE Hey sorry this video is so long, I wanted to stuff in as many examples of the full process as possible to really give you all an idea of what exactly you need to do to get the models looking ri
- **2021-07-20** `plastered_crab` [🔗╎tutorials-resources] [Animation Ripping / Porting Tutorial 2]
  Thanks to @R93_Sniper for creating a quick rundown of the process and thanks to GeneralI01 for the blender importers and thank you to @LaikaGlove for making Bonobo! This opens a ton of doors and it works great!
  
  "So here's the basics
  1 - Grab map files from the game you want animations from (you'll need the same maps you need for ripping models, so shared/campaign/ui)
  2 - Grab both bonobo and reclaimer if you don't already
  3 - Blender Users: Install General_101's tools for blender (https://github.com/General-101/Halo-Asset-Blender-Development-Toolset) [Note: You need to unpack the zip, for some reason]
  Max Users: Bonobo's install comes with maxscripts to import files. Put them in the scripts directory
  4 - Grab the model you want to animate with reclaimer. If you're using Blender, export as AMF. If you're using Max you can use either JMS or AMF
  ---NEW AMF SCRIPT FOR BLENDER LINKED BELOW---
  5a - Open bonobo and open up any map you want to pull animations from. Navigate to the JMAD area, open the animation file you want, right click, "extract source files". 
  5b -Navigate to a folder where you can dump the relevant animations, and then hit ok
  
  3DS Max:
  1. Run the maxscript to import the AMF or JMS file. Import your model
  2. Run the Animation Importer maxscript
  3. Hit "Load File" and a window will pop up. Navigate to the folder you dumped the animations to
  4. Find an animation you want to import, and then hit ok
  5. Click Import Animation in the main maxscript window
  6. If the skeletons matched, the animation will import correctly
  
  Blender: 
  1. File -> Import -> AMF (new file linked below)
  2. Navigate to where your model is and import it. *Be sure to set your scale to 3DS max under import options on the upper right side of the window*
  3. File -> Import -> "Halo Jointed Model Animation"
  4. Navigate to the folder you dumped the animations to
  5. Find an Animation you want to import and hit Import JMA. Be sure to set your game version correctly
  6. The animation will now be loaded onto the model."
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2021-07-20** `plastered_crab` [🔗╎tutorials-resources] [attachments] Blender_AMF2.py
- **2021-07-20** `plastered_crab` [🔗╎tutorials-resources] https://media.discordapp.net/attachments/819714264922062868/867097279710363648/Tacti-moa_1.gif
  [embed] https://media.discordapp.net/attachments/819714264922062868/867097279710363648/Tacti-moa_1.gif
- **2021-07-23** `plastered_crab` [🔗╎tutorials-resources] [Halo Animation Porting Video Tutorial]
  Here is a quick tutorial I made going over the full process on how to port animations from Halo CE, Halo 2, Halo 3 / ODST, and Halo Reach to Blender! I hope this helps out those of you who got confused with the written tutorials and thanks to @R93_Sniper @kayhotic @PlasmaGiant and @swamp for helping out with the process of animation extraction and porting
  https://www.youtube.com/watch?v=oIIHsOifz60&ab_channel=PlasteredCrab
  [embed] Halo Animation Ripping / Porting [Blender] https://www.youtube.com/watch?v=oIIHsOifz60 This video goes in-depth with the process of extracting Halo animations and porting them into Blender. I provide all the useful tools you need in order to do this as well as a .blend file with my Tactical Moa for 
- **2021-08-31** `plastered_crab` [🔗╎tutorials-resources] [Halo 3 FP animation Blender porting]
  Credit to @SpacedZed
  [attachments] H3_FP_Anims_in_Blender_Tutorial.txt
- **2021-08-31** `plastered_crab` [🔗╎tutorials-resources] [attachments] H3_FP_Anims_in_Blender_Tutorial.mp4
- **2021-10-05** `plastered_crab` [🔗╎tutorials-resources] [temp placeholder for HW2 scripts]
- **2021-10-05** `plastered_crab` [🔗╎tutorials-resources] Halo Wars 2 model 3DS Max 2015-2017 script
  [attachments] halo_wars_2.ms
- **2021-10-05** `plastered_crab` [🔗╎tutorials-resources] Halo Wars 2 model 3DS Max 2020+ script
  [attachments] HaloWars2_ModelImporter-1_1.ms
- **2021-10-05** `plastered_crab` [🔗╎tutorials-resources] Halo Wars 2 animation (uax) script
  [attachments] halo_wars_2_animation.ms
- **2022-05-03** `plastered_crab` [🔗╎tutorials-resources] Pinned a message.
- **2022-07-01** `plastered_crab` [🔗╎tutorials-resources] [Halo Reach Forge World / Blender <- -> Forge mode importer/exporter]
  Import forge maps made in Forge World on Halo MCC into Blender and export maps made in Blender back into Forge World
  https://www.youtube.com/watch?v=Q_LMjZkp1jo
  [embed] BEGINNER Blender Forge Tutorial | Mjolnir Forge Editor (Halo MCC) https://www.youtube.com/watch?v=Q_LMjZkp1jo Yes you heard me right! You can use Blender to Forge in Halo Reach! This new tool gives the user the ability to manipulate Forge objects in Blender and then "export" them into your Halo The 
- **2022-07-09** `plastered_crab` [🔗╎tutorials-resources] https://www.405th.com/forums/threads/tutorial-how-to-make-view-export-your-own-halo-spartans-in-blender.53179/page-2#post-816811
  [embed] [Tutorial] How to make/view/export your own Halo Spartans in Blender https://www.405th.com/forums/threads/tutorial-how-to-make-view-export-your-own-halo-spartans-in-blender.53179/page-2 This is actually really awesome. I need to give this a try when I'm back on my PC. Been wanting to do this for a w
- **2022-07-09** `plastered_crab` [🔗╎tutorials-resources] Various guides on building your Spartans, posing them, and converting the pieces to STL format
- **2022-08-05** `plastered_crab` [🔗╎tutorials-resources] [Halo Wars 2 Model / Animation ripping and porting tutorial]
  https://www.youtube.com/watch?v=wYqUsnkrhTY
  [embed] How to extract Halo Wars 2 models / animations into Blender https://www.youtube.com/watch?v=wYqUsnkrhTY Hey everyone, I wanted to make a quick video going over the process on how to extract Halo Wars 2 models and animations out of the base game and get them into Blender. For this you will need:
  1. 3
- **2022-11-02** `plastered_crab` [🔗╎tutorials-resources] Tutorial I made really quick that goes over how to export Pokemon Masters characters along with their animations
  https://www.youtube.com/watch?v=M7giAPObM3k
  [embed] How to package up Pokemon Masters Characters https://www.youtube.com/watch?v=M7giAPObM3k Hey everyone, I'll be showing you how to package up Pokemon Masters characters to be prepped and ready to go for modding projects using the Pokemon Masters Blender Pack, an edited animation import script, and so
- **2023-01-13** `plastered_crab` [🔗╎tutorials-resources] [Tutorial] How to create ASG Maps for other Halo models. Thanks to @TacoHombre!
  https://www.youtube.com/watch?v=8Szp4FM4y64&ab_channel=TacoHombre
  [embed] Halo Infinite style textures basic guide for substance painter https://www.youtube.com/watch?v=8Szp4FM4y64 This is a quick guide I threw together for recreating the halo infinite texture sets for any model using substance painter, this assumes you already have a decent knowledge of the program so if
