# scripting ai pathfinding

Selected from 74 candidate messages (ranked by length, links, Halo relevance; capped at ~260 KB). Chronological. Channels: BMAX, Reclaimer, UGX Pipeline (Importer), UGX_x360_import, 💬╎general-help, 💾╎autodesk, 💾╎substance-suite, 📜╎ripping, 🔍╎texturing-and-shading, 🦴╎animation

- **2021-03-26** `gravemind2401` [📜╎ripping] if someone wants to set it up for the map shrine and show me i can look at scripting it, as long as it’s not super complex like some of the shader setups i’ve seen screenshots of in here
- **2021-04-27** `lobbster` [🔍╎texturing-and-shading] I encountered this issue before with the actual Jorge model not liking color changes so it may be a problem with the actual reach texturing
- **2021-05-17** `Deleted User` [🦴╎animation] hello all umm i need help ripping animation files from halo 3 odst.map files
  bonobo seems to extract it but when i go to the folder
  it no work
  i am ripping models and animations from odst so i can put it in halo ce i know i could just rip everything out and just port it over
  but i am trying to put the animations on halo ce models instead
  so i thought renaming the skeleton of ce marines to that of odst marines for the animations would work then run some scripts to import it but idk what do you guys think?
  kinda wish there was a tutorial on this
- **2021-05-20** `zad_47` [💬╎general-help] https://www.moddb.com/games/halo-2/downloads/bonobo-version-1003
  [embed] Bonobo [Version 1.0.0.3] Halo2/3/ODST/Reach Animation Extractor file https://www.moddb.com/games/halo-2/downloads/bonobo-version-1003 Bonobo [Version 1.0.0.3] is a Halo2/3/ODST/Reach Animation Extractor used to extract the animations from those 4 halo games, this can be found on halomaps.org, this c
- **2021-06-06** `pointdevice` [📜╎ripping] pulling in Halo3 Chief from 005_intro.map from MCC, using the latest release build of reclaimer from github, and the script posted above using the listed settings(should be the defaults)
  [attachments] unknown.png
- **2021-06-29** `magi7tank` [💬╎general-help] anyone know where i can get a shader or something to make a model a AI
- **2021-08-19** `lavadeeto` [💾╎autodesk] Also remeber halo and various scripts/functions for halo were made on max so 🤷‍♂️
- **2021-10-22** `itzhouze` [💬╎general-help] hello there. i need some help, getting a campaign to multiplayer map conversion done (Halo 3).
  
  my plan was to make a part of the first halo3 campaign map (010_jungle) playable as a multiplayer map.
  
  i changed the map id to the mp one, set the gametype to multiplayer, set the Zone Sets => "set_null" to the configuration of the right part ("set_path_c"), deleted all other unused zone sets, deleted the following sections: Globals, Scripting data, source files, scripts
  
  then i opened up the scenario in sapien and added a respawn point tag to the map.
  
  then i build a map file out of it via: "tool build-cache-file levels\solo\010_jungle\010_jungle"
  i noticed, that the console log said: "loaded script 'global_scripts'" and some other scripts... (maybe i forgot to remove some?)
  
  the build then succeeded, but when i tried to load it up in halo3, it throws a "Fatal Error!" and crashes.
  
  I hope somebody can help me out with this! 🙂
- **2021-10-29** `Deleted User` [💬╎general-help] maybe there was a script to change the color, try using color ramp on the diffuse and change the color by adding multiple colors
- **2021-12-03** `tpg` [🔍╎texturing-and-shading] nope but you can use the shader made by @R93_Sniper and Point Device https://discord.com/channels/819709630540021810/825398188743458826/911430857384935424 unless you feel like making your own using https://assetstore.unity.com/packages/tools/visual-scripting/amplify-shader-editor-68570
  [embed] Amplify Shader Editor | Visual Scripting | Unity Asset Store https://assetstore.unity.com/packages/tools/visual-scripting/amplify-shader-editor-68570 Get the Amplify Shader Editor package from Amplify Creations and speed up your game development process. Find this & other Visual Scripting options on
- **2021-12-15** `unordinal` [🔍╎texturing-and-shading] In your spartan file, go to File > Append. Navigate to the downloaded Halo5Shaders.blend file. When you find it, double-click it and navigate to NodeTree and then the Halo 5 shader. Select it and click append at the bottom right.
  
  In your material for the spartan, press Shift + A to add a new node. Go to 'Group' and select the Halo 5 shader group you appended. Connect the inputs and output appropriately.
- **2022-02-13** `fiycsf` [💬╎general-help] Just base colour alone.
  Cooked height values.
  A fundamental lack of understanding basic shading behavior.
  
  :Fiy_Shrug:
- **2022-05-16** `surasia` [🔍╎texturing-and-shading] Technically the shader fully supports importing with the coating script, but I didn't include it as I wanted to lean more towards user input
- **2022-06-14** `rafaleeater` [💬╎general-help] https://www.moddb.com/games/halo-3/downloads/bonobo-version-1003
  [embed] Bonobo [Version 1.0.0.3] Halo2/3/ODST/Reach Animation Extractor file https://www.moddb.com/games/halo-3/downloads/bonobo-version-1003 Bonobo [Version 1.0.0.3] is a Halo2/3/ODST/Reach Animation Extractor used to extract the animations from those 4 halo games, this can be found on halomaps.org, this c
  [attachments] unknown.png
- **2022-07-08** `gameoverload` [🔍╎texturing-and-shading] It should be in shader resources somewhere, or at least a link to his shader then you can navigate to his main page from there.
- **2022-08-21** `micahhalcyon` [💬╎general-help] I'm trying to get the bitmaps for the ODST magnum. I got the model but I'm encountering roadblocks every method I try to get the textures
- **2022-08-27** `fiycsf` [💬╎general-help] Fiy rant's story time;
  ```
  For example, for a non halo (Vroid software made model) I had someone want to pay 30$ for a hair colour change. Took me like, 15 minutes to get the exact kinda fade they wanted. But normally, when doing on commission work, I ask for that much (30$) for the hour. So I jumped through the hoops to force the model into VRC in a desktop medium performance rank.
  
  Uploaded it on my account, let em clone and try it so they'd have something until they figured it out, and said "here's all the files, you may have to ask in various servers public help channels to get it sorted in unity for yourself, but you have it cloned from me for now, so you shouldn't have to rush."
  
  And I figured, that'd be the end of that. I did far more than was asked, on the same cost.
  No, no it was not that simple. I'd check my phone to a dozen some messages many times over the next few days asking me alone *in DM's* how to setup the model in unity that I did in under an hour, and ended up getting so fed up with the constant questioning and them not doing as explained to setup the model, coupled with phrases/behavior like I still owed them something, that I packed up the model gave it to them, taking another bit of time outside of the constant messaging, just to get them to leave me be.
  ```
  I know stories like these aren't uncommon, commission work or not, among the friends I have that have been willing to spend time teaching or setting stuff up for some. And honestly, it burns out a c ...[cut]
- **2022-08-29** `nascarlaser1` [💬╎general-help] now under "delay" set it to 5 seconds (the max delay possible. Then in blender navigate to the "shader" or "output" menu of the search thing (where I had the screenshot). Once the 5 seconds is done, it'll switch to screenshot mode, which will freeze your screen and let you click and drag a box over the portion of screen you want to capture. Do that for the menu, then just copy and send the photo here 🙂
- **2022-09-15** `blakjewellio` [💾╎autodesk] I found a few elites on sketch fab but I haven’t seen if they’re very well rigged I know there’s one that cost money that specifically says it’s for Maya and it’s pretty cheap for a model if you find it just look at description
- **2022-09-22** `coarforge` [📜╎ripping] Infinite uses control maps. There is a description of how they work in #📑╎control-map-guide if you want to create your own shader, or you could probably use goblins shader as a starting point (or bacons shader). I haven't tried either of those shaders with the brute models, but they should probably work with some tweaking (the rat for example did work with bacons shader without any modifications, but it's probably not super accurate)
- **2022-11-07** `hausdev` [🔍╎texturing-and-shading] I'm a brainlet when it comes to most 3D stuff, and I've been focusing on CEA so my H2A knowledge is a bit rusty, but in the texture definitions json, i beleive there's a field called "preset" or something along those lines. It might be called something else idk
  
  But I think once you have that name, you can find that file under Shader Definitions in index, and that'll have the HLSL code. Iirc it's polymorphic, so theres a base shader that other shaders build on top of, so you'll probably need to navigate a few different files to find what you're looking for. There's also a lot of mathematical functions in the "common" shader file, if that helps
- **2022-11-07** `hausdev` [🔍╎texturing-and-shading] which means it's using their GLT shader. You can type in "glt" in the search bar and there's shader definitions/descriptors that define the parameters and stuff, and then there's the actual code for GLT in the shader code group
- **2022-12-08** `stumpyj` [🦴╎animation] Is anyone here familiar with both 3ds max scripting and blender scripting? I am converting a script from max to blender, and everything is great until I do animations. Both of these animations were extracted using the same .gr2 data (left: max - imported to blender as fbx, right: blender). Both the good animation and the bad work on either rig identically to eachother, telling me the rig is not the issue.
  [attachments] image.png
- **2023-02-26** `wake9090` [💬╎general-help] yeah it says this in the description of the vid:
      - Zone 7 (Damage) enables the 7th zone in models, which is usually used to add additional detail to models. This remains unused on models such as multiplayer armor.
- **2023-03-31** `classytm` [🔍╎texturing-and-shading] speaking of real ones, thanks to Archangel/bluephoenix my delta squad models have had some banging renders as well. Honestly in love with this one, makes me wonder what a republic commando game in the style of reach would be like
  [attachments] df6g40x-efeaabe3-6be0-498a-a441-72379735e8da.png
- **2023-04-27** `gameoverload` [💬╎general-help] *Does it...? Cause it wouldn't cause Squad from my knowledge doesn't use color tint or whatever else Chivalry has in its master material files.*
- **2023-05-19** `bertglassed` [💾╎autodesk] You probably need to make a script or plugin for maya for it to work correctly. Maya is great, but especially when it comes to sth old like halo where it was built around 3ds, you will run into issues. Voice versa when working with games that were built on maya, 3ds users will have a hard time
- **2023-05-24** `theidiot5651` [💬╎general-help] https://www.youtube.com/watch?v=Jm39-vYl1zo&t=86s
  
  https://www.youtube.com/watch?v=LdmoBY08LTM
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo&start=86 This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan p
- **2023-07-11** `fiycsf` [🔍╎texturing-and-shading] My biggest advice, is ignore any Arma shader made for substance, and public export presets. Everyone I've encountered is setup wrong.
  
  ***Like, super wrong.***
  (*Seriously who the hell uses greyscale base colour as spec?*)
- **2023-08-14** `gameoverload` [📜╎ripping] This is a capture of the System Shock Remake.
  I can pull Scrubbing of particular events, the entire events tree including descriptions and such, and the objects those events are linked to, event details, render target outputs, and API information like VS and PS information ( this includes shader resources and pixel shaders ). At least that is all the important bits.
  [attachments] image.png
- **2023-08-27** `gameoverload` [💬╎general-help] That is why I explained things for the script bit and piece, but he was going to the shader tab like we were already going to, to look at the textures to make sure any were blank, again, like I was already having him do.
  
  He wasn't finding his own solution there, that was with the script stuff he thought about, but directed him away from and explained since that wasn't the issue, while for the texture stuff, I was pointing him to that.
  
  So I was doing what you do also since I have to take that approach at work also, but the texture part was not on him, I pointed him to that instead.
- **2023-09-01** `momomainia` [💬╎general-help] Okay well here's the situation that I'm encountering. That model pack isn't compatible with Olympus (not even completely sure what that is yet) and I need to use this shader pack (https://discord.com/channels/819709630540021810/1072705765208506369/1072705765208506369) on the armor. But from my understanding you need to use Olympus to use the shaders. So my question is how do I get those shaders onto the models from the Spartan pack??
- **2023-09-06** `trisckit` [💬╎general-help] anyone know how to grab / set this value on materials with a bpy script? i basically want to just set all materials in my scene to have the same Emission "color" (being the same image file) as the base color image texture node
  [attachments] image.png, image.png
- **2023-09-20** `thefunman37` [💬╎general-help] Got it from this video if that helps (The Google Drive link in the description doesn't work anymore) https://www.youtube.com/watch?v=vVS9HJd73io
  [embed] Halo Reach Custom Spartan Creator https://www.youtube.com/watch?v=vVS9HJd73io Basic tutorial on how to replicate your custom Halo Reach Spartan in Blender. Great for cosplay references
  Credit goes to Chief and the Halo Archive for putting together this amazing resource
  https://drive.google.com/file/
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
- **2023-10-03** `felixvyra` [🔍╎texturing-and-shading] Hey Guys.
  
  I haven't been on this server for some time as I have been doing some other stuff, and although this isn't related to Halo I have a bit of a texturing issue and I don't know a better place to ask for help.
  
  So. The last few days I have been trying to rip some assets from a game called Rainbow Six Siege. It's a long and complex process so I'm not going to go into it right now but TLDR: the guns have different skins and some of the "Lower quality *universal*" skins use a simple image that get's applied to all the guns, instead of having a different base color map for each one.
  My problem seems to be with the UV mapping. In the Images provided below you will see how the gun looks completely fine using the "no skin" base color map but If I try to apply the "Peacock" skin only SOME parts of the gun look correct.
  
  This is super weird and not something I have ever encountered before.
  I tried to look for some additional data that I might have missed but there is nothing. No extra information in the alpha or anything in the ripped files.
  
  Any help would be appreciated.
- **2023-11-09** `itexturel` [💬╎general-help] Anyone know how to change any created model into a hologram/AI for blender and change the overall color while keeping the tones in tact?
- **2024-01-05** `tomhogenberg` [💬╎general-help] general help needed with the importing of the sabre from reach, using it as a placeholder/first iteration for an animation. maybe someone knows specifically what causes the texture glitching and behavior with reach assets. its not reliant on the texture(s) and materials applied. anyone has a clue, if this is halo (reach) asset related?
  [attachments] Screenshot_2024-01-05_135220.png
- **2024-03-07** `coarforge` [💬╎general-help] I did a test with a quick script and a mostly bogus shader earlier today (just a normal map and solid color or color map for most of the materials, no control maps or layers, and it sometimes picked one of the detail layers instead of the macro layers)
  [attachments] image.png
- **2024-03-08** `goajango` [💬╎general-help] Hello, i have download the spartan pack 3 of this video (https://www.youtube.com/watch?v=LdmoBY08LTM&t=69s), and I would like to export it to 3ds max. I tried to export it to fbx and obj but the textures are not implemented in 3ds max. Does anyone have a solution ?
  [embed] Adding Custom Camo to your Spartan (Halo Infinite Spartan Pack 2) https://www.youtube.com/watch?v=LdmoBY08LTM [Link to over 1,000 greyscale camo patterns in description]
  
  [NOTE] For new versions of the Spartan Pack you need to duplicate your camo plugin nodes. If you connect 1 camo shader node to al
- **2024-03-08** `frostbitecinematics` [🔍╎texturing-and-shading] I know the functionality is in there at the very least for game accuracy since I've seen references to the BotMidTop Color params in some of the material py scripts (in Olympus maybe?). Idk what it gets used for in-game though. It seems redundant because the gradientmap+curve-driven approach gives you the exact same capabilities right? Only difference being ofc that the curve approach lets you use "unlimited" colours, vs BotMidTop which only lets you blend between 3 colours. But if you just add 3x colours to a curve, is the output of that setup basically identical and therefore interchangeable?
- **2024-03-10** `coarforge` [📜╎ripping] I have a WIP script to load material parameters from the json exported along with the dae and set them up in blender, but without a proper shader and without knowing how some of the parameters work, it's not looking that great yet.
- **2024-03-10** `coarforge` [📜╎ripping] A quick script to set up materials from an infiniteExplorer export. You need to load the script in blender and run it in the editor after putting in the correct paths at the top. With my test shader, you also need to append the material `testmat_color` to the file first (that shader is everything but correct). I haven't written any documentation for the shaderdb file yet, but basically, it maps material parameters given in the exported json to parameters in the blender material (currently, only image textures, UV map indicies starting at 0, booleans indicating if UV3 (UV2 when starting at 0) should be used and float values are supported, I haven't added colors yet). 
  If there are multiple shaders listed, the script calculates a score for each shader by incrementing the score for each parameter the shader uses that is present in the material by one and decrementing it by 2 for each parameter the material doesn't have but the shader has if it's not marked as optional. If the score is 0 or lower, the shader isn't used. This is far from a great way to do this, but it was quick to do.
  There's also currently no material style info being exported by iE yet, but at least for maps, not too many materials seem to have it anyways from what I've seen
  [attachments] matsetup.py, shaderdb.json, mat.blend
- **2024-04-04** `dg5312` [💬╎general-help] Hi 
  @Gravemind2401
  Some later versions of 3ds Max (maybe 2020+) cant work with your script, because now use by default physical materials and cant be changed.
  Can be fixed, please?
  I tried to do it, but my experience in scripting is 0 basically and i get a misunderstanding with 3ds max docs because we have two parameters for create a shader but just make a physical material again qwq
  [attachments] image.png
- **2024-04-04** `dg5312` [💬╎general-help] the issue with the script is which use cshader for generate a shader, but this make a physical material.
  idk why but in 3ds max docs we have two parameters.
  i use standard, for make a standard legacy material but idk why is just make the physical material again.
  [attachments] image.png
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
- **2024-07-23** `rougegamer` [💬╎general-help] How do I get grunts to suicide in Halo 3/Odst in sapien? There value for kamikaze is setup. But however I never see them suiciding when there leader is dead or anything like that. All they do is run away. Is there a global script I need to do in order for them to function like you would see in the games? And if so, what do I do to make the enemys function like you'd see in the games.
- **2024-08-06** `r93_sniper` [Reclaimer] Tool created by @Gravemind2401 to extract models, textures, and sounds from various halo games. Read the github link for more information on what is and isnt supported, as well as #📰╎reclaimer-updates for any pertinent news.
  
  Program: https://github.com/Gravemind2401/Reclaimer
  Addon Downloads, including AMF importer script: https://www.mediafire.com/folder/dabvr2j5ofe73/Reclaimer_Downloads
  [embed] GitHub - Gravemind2401/Reclaimer: A Halo game file explorer https://github.com/Gravemind2401/Reclaimer A Halo game file explorer. Contribute to Gravemind2401/Reclaimer development by creating an account on GitHub. MediaFire https://www.mediafire.com/folder/dabvr2j5ofe73/Reclaimer_Downloads MediaFire
- **2024-08-06** `r93_sniper` [BMAX] Free connector app to transfer content between 3DS Max and Blender
  https://titus.gumroad.com/l/B2MAX
  [embed] Blender<->3DSMax Connector [BMAX] https://titus.gumroad.com/l/B2MAX Hello, Blender <> 3DSMAX connector(BMAX) is a two small scripts for these applications.Workflow is simple - just send and get models through FBX format.License:GNU General Public License.OS:Windows onlyInstallation:Extract "_BMAX_xx
- **2024-08-07** `blackandfan` [UGX_x360_import] Blender script used to import xbox 360 mesh data from .ugx files (Change the path to use the script). Script has been created by @stumpy
  [attachments] ugx_x360_import.py
- **2024-09-23** `r93_sniper` [💾╎substance-suite] this is a tutorial i like to send to newbies a lot: https://www.youtube.com/watch?v=RQ-hRk0WHJ8
  [embed] Introduction to Substance Painter - Ultimate Beginners Guide https://www.youtube.com/watch?v=RQ-hRk0WHJ8 This is an elaborate tutorial to help you get started with Substance Painter. Check out our full 4-hour premium series for more training!
  
  https://flippednormals.com/downloads/introduction-to-sub
- **2024-10-11** `thoro.blend` [🔍╎texturing-and-shading] the shader setup Im using for squad
  
  and the tops not red, so I dont think its getting handled properly
  [attachments] image.png, image.png, image.png, image.png
- **2024-11-03** `juggarnxught` [💬╎general-help] Yo I just read your bio let's squad up and spank some kids on reach or 3
- **2024-11-17** `spartans384` [💾╎substance-suite] So I'm gonna hope someone's encountered this issue before. I erased some clasps (normal mapped) on this Spartan forearm, but it's left this smooth area that sticks out among the untouched portions of the normal map, which have a bit of graininess to them. Is there any way to blend this together to make it pretty much disappear?
  [attachments] Normal_map_blending_2.PNG, normal_map_blending_setup.PNG, Normal_map_blending.PNG
- **2025-01-26** `gameoverload` [💬╎general-help] Use this:
  https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.0.1536
  [embed] Release v2.0.1536 · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.0.1536 RMF exports will now include a color channel for h4 and h5 meshes that use position W
  
  Affected models will need to be re-exported
  Version 1.0.1 of the import script is required in order to 
- **2025-02-27** `masterj2001` [🔍╎texturing-and-shading] https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/17
  https://projects.blender.org/blender/blender/issues/131605
  [embed] Cycles and surface normals (normalmap shading) https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/17 Somebody else had a very similar complaint/observation to this thread.  Various parts of Cycles will use the geometric normals (or smoothed geometric normals depending o
- **2025-03-03** `iloveagoodcrisp` [💬╎general-help] This is a general question for those who make animations / cinematics in Blender or other programs. I'm building out a system for exporting cinematics from blender to Halo Reach/Halo 4 and I'm unsure of how I should approach creating "cinematic events" (these are dialogue lines, particle effects, scripted events that play in game). 
  
  Should I try to bind these to timeline markers? Or have my own entirely separately menu with these events and let the user specify the frames they occur on? Use the video editing sequencer?
  
  Been thinking lots about the pros and cons of different approaches and struggling to decide what's best
- **2025-06-11** `r93_sniper` [💾╎substance-suite] https://www.youtube.com/watch?v=RQ-hRk0WHJ8
  [embed] Introduction to Substance Painter - Ultimate Beginners Guide https://www.youtube.com/watch?v=RQ-hRk0WHJ8 This is an elaborate tutorial to help you get started with Substance Painter. Check out our full 4-hour premium series for more training!
  
  https://flippednormals.com/downloads/introduction-to-sub
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
- **2025-08-27** `manusanram` [🔍╎texturing-and-shading] Hello to all you guys
  I was wondering something; I'm sure you guys have encountered issues with the light volume textures for Halo Reach on Reclaimer (How they are always clipped exactly in half)
  Is there a way to get the full texture? I was working on something and I want to know how to get the resources correctly
  [attachments] image.png
- **2025-10-22** `katbug` [🔍╎texturing-and-shading] https://www.youtube.com/watch?v=LdmoBY08LTM
  [embed] Adding Custom Camo to your Spartan (Halo Infinite Spartan Pack 2) https://www.youtube.com/watch?v=LdmoBY08LTM [Link to over 1,000 greyscale camo patterns in description]
  
  [NOTE] For new versions of the Spartan Pack you need to duplicate your camo plugin nodes. If you connect 1 camo shader node to al
- **2025-12-09** `tristangold` [📜╎ripping] quick ripping question - so I am trying to port some of the Halo Reach assassinations into blender via Foundry with the animation repository. First attempt to drag and drop Repo animation file into Blender failed and just want to understand exactly what I'm missing.
  
  1. For me to properly import the animation data - do I first need to extract it onto a spartan model first or can I rip the data and bones just directly into blender. 
  2. If I do need a model, are these findable in to Omaha directory or just easier to grab through the discord's already existing models. It's possible I've also been searching through the Model tools folder rather than Halo MCC itself, but navigating the files I've really just been coming across .hsc scripts.
  3. Is there a specific location I need to be dropping these animation files into on the Omaha folder? My drag and drop attempt spit back the error "Ensure your file is saved to your Omaha data directory"
  
  EDIT: poked around the discord some more so also gave a go at importing one of the animations through the .jms importer. Got an internal error noting the "action object has no attribute 'fcurves'
- **2026-01-10** `gameoverload` [💬╎general-help] Welcome to poor resource management, not even on AI or anything, its because of what xbox requires of them, and microsoft on top of that, and the fact they originally went for a hero shooter only to pivot to open world + multiplayer.
  
  Its Halo Studios, yes. The 343i folks basically got let go, but that doesn't help when all the hires will likely get let go eventually, and xbox makes them still rely heavily on contractors over full time employees.
  
  Oh, side note, if you want info on all of the customization in Halo Infinite: https://cylix.guide/
  [embed] Cylix Guide https://cylix.guide/ Halo Infinite API Explorer
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
- **2026-02-03** `jishlefish` [🔍╎texturing-and-shading] is there any form of effect/shader which i can layer over the top of a spartan to make them look like an ai? or is my best bet just to do it in compositing? im trying to make Church from RvB and am a bit stuck
- **2026-02-22** `echo_mation` [📜╎ripping] Anyone know if I can pull the classic halo 2 ODST model and rig with reclaimer? I've been trying for a while and can't figure it out. I can only find the bitmap, shaders and ai. (Im using the delta halo map)
- **2026-03-16** `r93_sniper` [💾╎substance-suite] https://www.youtube.com/watch?v=RQ-hRk0WHJ8
  https://www.youtube.com/watch?v=KMXpmsmZy38
  @Mr. Grieving
  [embed] Introduction to Substance Painter - Ultimate Beginners Guide https://www.youtube.com/watch?v=RQ-hRk0WHJ8 This is an elaborate tutorial to help you get started with Substance Painter. Check out our full 4-hour premium series for more training!
  
  https://flippednormals.com/downloads/introduction-to-sub
- **2026-06-03** `koax117` [💬╎general-help] I don't have the CE mods tools installed, so I can't vouch for how any of it works sadly.
  For when I'm working in H3, I load up Guerilla and will navigate to locate the object's render.model / relevant shader. if there's any textures missing that didn't rip with the model, that's where i'd go to look for it.
  [attachments] image.png, image.png
- **2026-06-14** `dustyoo10` [🔍╎texturing-and-shading] is there more resources I need besides the shield shader and the transparent generic script?
- **2026-06-19** `asrieldream` [🦴╎animation] someone at HS must be inspired by thishttps://youtu.be/_AxMlgPFxao?is=heS5UigLQhS-RLp1
  [embed] Halo (WIP)  - Opening Shot - Teaser Clip https://www.youtube.com/watch?v=_AxMlgPFxao #halo #fananimation #blender3d #halomcc #haloceanniversary #animation 
  
  The opening scene to Episode 1 of my Halo CE adaptation.
  
  The Pillar of Autumn emerges from slip-space after a blind jump, narrowly escaping th
- **2026-07-10** `alabamanwildman` [🦴╎animation] If you want to get into animation, there's plenty of excellent tutorials out there. I personally wouldn't jump straight into character animation, you're going to want to do basically the whole ass curriculum from animschool.com before you reach for the stars. As for cascadeur, while it does use ai to make things look a bit better, it's not a crutch and people will be able to tell if you used cascadeur for everything
- **2026-07-12** `fancyneer` [🦴╎animation] Cascadeur was an in-house animation program for the studio maintaining it until they made it public for others to use. The AI tag is a bit of a misnomer and most likely just there to drive up speculation since it's just running off a neural network which has been trained by the studio's own mocap data they've accrued over the past decade. Tech like this has been around for a while, albeit in very experimental states and not as publicly available.
  
  As a beginner, you're doing yourself a disservice for using it as it automates a lot of stuff an animator should be able to do by hand. What are you gonna do when you don't have access to its toolkit in your project due to technical constraints?
- **2026-08-03** `koax117` [💬╎general-help] if you'd like, you can share screenshots of -
   - Render Properties window: Color Management at the bottom
   - Output window. Color Management and Post Processing at the bottom
   - Sun light properties: strength, exposure, etc.
   - Compositing window: if you have any exposure nodes or anything built in. 
   - World Shader: what the HDRI texture is (by clicking on it) as well as any nodes affecting it.
  
  My guess is with you using that Physical Atmosphere addon it runs a script that changes some settings to help with the rendering, but it is probably set a default lighting configuration that doesn't match your scene. I've used TrueSky in the past and it fucks with the exposure settings in my scenes a lot.
