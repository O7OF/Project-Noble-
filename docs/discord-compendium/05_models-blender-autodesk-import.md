# models blender autodesk import

Selected from 1113 candidate messages (ranked by length, links, Halo relevance; capped at ~260 KB). Chronological. Channels: BMAX, Blender .max Importer, Blender DDS, Foundry, Halo 4 Light Importer, Halo 4 Material Importer, Halo 5 Animation Extractor, Halo 5 Data Importer, Halo Asset Blender Development Toolset, Halo Reach Light Importer, Index, Invader + SixShooter, Reclaimer, Source I/O, UGX Pipeline (Exporter), UGX Pipeline (Importer), Unreal Engine Blender Tools, Unreal Engine I/O, 💬╎general-help, 💾╎autodesk, 💾╎substance-suite, 📜╎ripping, 🔍╎texturing-and-shading, 🦴╎animation, 🦴╎rigging

- **2021-03-22** `err_0r` [📜╎ripping] Surasia, I haven't used the blender shader before, I do my shader work in unity, so if you could verify this for me: Is the reach shader block in the image that Brooen posted supposed to not be connected at all to the output? IT looks to me like the material isn't set up right
- **2021-03-22** `err_0r` [📜╎ripping] @swagster Chief, generally the way textures work is that textures that aren't color based, as in they are actually a bunch of black to white masks all crammed into one texture, which is normal maps, color mask maps, and often times things like smoothness and metallic, are all stored in linear. The only things still using sRGB are diffuse textures, and the color values of specular data when you're using a specular shader, which is important for us because halo games use a really odd type of specular/metallic hybrid
- **2021-04-17** `chromacore` [🔍╎texturing-and-shading] Made my own reach shader in blender. Also set it up so I can bake it out to pbr colored spec https://media.discordapp.net/attachments/668244844769968129/833153942346137600/Capture.PNG?width=526&height=701
  [embed] https://media.discordapp.net/attachments/668244844769968129/833153942346137600/Capture.PNG?width=526&height=701
- **2021-04-19** `fiycsf` [📜╎ripping] The mindset some have, of "I've been doing it longer" is very dangerous.
  
  There's many people who have loads of area's of expertise. But that doesn't mean things are being done correctly, or that important things cannot be overlooked.
  
  There was a community that for over a year really believed that the blue channel of the control was related to the coloration of Spartans for halo 5. They were not convinced otherwise, until they had a unity shader to play with that proved it.
- **2021-04-20** `sonofzeta` [💬╎general-help] Also, there is a model from a .blend collection of the Halo Reach Covenant weapons. I want to save the Gravity Hammer so I can put it in other .blend files. How do I export it so that it has the same level of detail?
- **2021-04-21** `krios24` [🔍╎texturing-and-shading] Found what I can safely guess is one of the only rips of the Halo 4 ODST armor via a gmod addon. The old XBox 360 textures are trash so I’m using it as a test dummy for reacquainting myself with Substance Painter. I remember liking the armor’s look when it came out but not the texture/color scheme they gave it. So I’m trying to re-envision it in the OG style
  [attachments] image0.png
- **2021-04-22** `err_0r` [🔍╎texturing-and-shading] It's good to see someone else who's trying to do right by the H4 ODST stuff. I reached the same conclusion you did about the striping/paintwork being the biggest issue. It doesn't *feel* odst to me, so I also brought it through substance to see if I could get something more acceptable out of it. Admittedly it's some of my older substance work so it's not quite as refined as some of my newer projects but none the less I'm relatively happy with the texturework I got out of it. 
  
  An interesting bit of the project that I've used in some of my newer stuff since is that this is what I call a hybrid texture, combining the original textures with my own. I imported the diffuse as well as grayscaled channels of the specularity/metallic/smoothness, and set it up so it looked as it did in game with shine and everything, and then used the original color code masks to set where my paintwork went. This way I got rid of the horrid stripe nonsense without having to worry about the thousand little details, keeping my layer count down and computer happy.
  [attachments] unknown.png
- **2021-05-06** `dashiv` [💬╎general-help] I am pretty inexperienced in blender so this is going to be interesting, but can someone tell me how to do this? in this video at around the 3:20 mark, he insets the ends of the reach thruster. I had a game rip file (cant remember if it was from the archive or not) and tried deleting every face so i could have the thruster by itself. then when i tried a few different methods, it always screwed with the model and deformed it or added protruding geometry. Is there an easy way to do it? also after the 3:05 mark, he rotates the thruster of the reach jetpack. this isn't crucial, i just thought the origin or rotation was neat and useful. https://www.youtube.com/watch?v=2b89MxacEl8
  [embed] Something about Sharkface https://www.youtube.com/watch?v=2b89MxacEl8 A pretty pointless video to wet the palette during a dry spell.
  
  My DeviantArt: https://www.deviantart.com/eproth
  
  #redvsblue
- **2021-05-06** `allindawoods` [💬╎general-help] "chief — 03/31/2021
  Halo: Reach Elites with all armors, pre-setup textures, rig and shading.
  https://drive.google.com/file/d/1lEKWe4WHANJD4DmsHLjIsyq9_XPRi6tC/view?usp=sharing"
  how do i export these elites with their materials??
  [embed] Reach Elites.zip https://drive.google.com/file/d/1lEKWe4WHANJD4DmsHLjIsyq9_XPRi6tC/view?usp=sharing%22
- **2021-05-09** `nobleofdeath16` [💬╎general-help] probably a dumb question; but are the models provided in the Archive already compatible for use with following Reject's model importing video tutorial? Wanted to work on getting that awesome shotgun Revenge did into Halo Reach
- **2021-05-14** `surasia` [📜╎ripping] @Gravemind2401 I've been trying to port some halo 3 maps to blender, but it seems like the .shader tag is broken for halo 3. It now crashes whenever I try to open it in the meta editor.
- **2021-05-15** `surdavow` [🦴╎animation] Aight, I'll share something here. This is an animation featuring worts and all from March. https://youtu.be/2CqCECbulsE
  [embed] Distant Targets https://www.youtube.com/watch?v=2CqCECbulsE A Sangheili spots some targets from a distance, but constant intervention prevents him from just shooting, or perhaps he is just unwilling to with that Carbine???
  
  This project has been something I've been working on since the end of Januar
- **2021-05-17** `Deleted User` [🦴╎animation] hello all umm i need help ripping animation files from halo 3 odst.map files
  bonobo seems to extract it but when i go to the folder
  it no work
  i am ripping models and animations from odst so i can put it in halo ce i know i could just rip everything out and just port it over
  but i am trying to put the animations on halo ce models instead
  so i thought renaming the skeleton of ce marines to that of odst marines for the animations would work then run some scripts to import it but idk what do you guys think?
  kinda wish there was a tutorial on this
- **2021-05-20** `zad_47` [💬╎general-help] https://www.moddb.com/games/halo-2/downloads/bonobo-version-1003
  [embed] Bonobo [Version 1.0.0.3] Halo2/3/ODST/Reach Animation Extractor file https://www.moddb.com/games/halo-2/downloads/bonobo-version-1003 Bonobo [Version 1.0.0.3] is a Halo2/3/ODST/Reach Animation Extractor used to extract the animations from those 4 halo games, this can be found on halomaps.org, this c
- **2021-06-01** `surasia` [📜╎ripping] https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] General-101/Halo-Asset-Blender-Development-Toolset https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2021-06-06** `dyland64` [🔍╎texturing-and-shading] how would one go about properly setting up the halo wars 2 banished phantom in blender? I am not familiar complex lighting set ups, but I've already got something going for the render I want to achieve and I need the banished phantom to have proper color etc
- **2021-06-11** `thelifeweaver` [📜╎ripping] The aforementioned demo BTW ||(https://youtu.be/MMkLdWBn5Ew) ||
  [embed] Ninja Ripper 2 beta Quick Guide https://www.youtube.com/watch?v=MMkLdWBn5Ew Ninja Ripper 2 is an experimental utility for extracting geometry from 3D game levels and exploring them in a 3D editor such as Blender, 3D Max, etc. Use cases could be, for example, exploring hard-to-reach places in a game 
- **2021-06-18** `vinwarrior` [📜╎ripping] Currently trying to pull the Spade truck from Halo Reach and its normals seem to be messed up. Attempted both the Winter Contingency version and the Forge World version. Both rip with incorrect normals. I have 'import normals' checked on, and other vehicles from Reach haven't had the same issues. Everything seems to import fine except for the Spade.
  [attachments] unknown.png
- **2021-06-19** `neckzilla` [📜╎ripping] https://youtu.be/iHxHPWdQr7k?t=2
  [embed] Parametric modifiers Performance:  3ds Max vs Blender vs Houdini https://www.youtube.com/watch?v=iHxHPWdQr7k&start=2 A comparission with a parametric modifier over a 3d scan in Houdini, blender and 3ds max. 
  The scan has 2M polys. 
  Plays fluid in 3dsmax (15 fps)
  Houdini around 5fps.
  Blender doesnt r
- **2021-06-24** `surasia` [💬╎general-help] For Reach Spartans, there's a blender file @swagster made. It basically allows you to create your own spartan in blender. Just open the file, and unhide/hide what you want in your spartan.
- **2021-06-26** `bifflive` [💬╎general-help] Hey guys quick question. So I’m completely new to blender and making 3D art etc. so if I want to change the color of a Spartan model or an elite would I have to change the color of the texture in photoshop or just add color to the model itself in blender? Also any tips would be very appreciated! Thank you!
- **2021-07-02** `deityeternal` [💬╎general-help] yeah I can't seem to figure this out, I downloaded the zip for Halo 3 ODST MCC and they come up as 3D Manufacturing Format, or 3D Object, I'm trying to import them in and I can't seem to figure it out
- **2021-07-16** `Average_Username` [💬╎general-help] so i've been trying to get bonobo working using the tutorial 
  but bonobo just refuses to open for me and i get this
  [embed] Thanks to @PlasmaGiant for taking the time to provide a brief rundown of ripping animations with Bonobo and porting them to Blender!
  
  "So what i found first was this program called Bonobo, an animation extractor program. It essentially reads the .map files for the Bungie Era games for you to either 
- **2021-07-18** `flipdark95` [📜╎ripping] Hey guys, has anyone had any luck in exporting the assets of the intro cinematic for Halo Reach? It's entirely a in-engine scene, but so far I haven't been able to find the command outpost that Noble has their first briefing in.
- **2021-07-18** `surasia` [🔍╎texturing-and-shading] Texture importing to reach is super easy, you can use forerunners tool to convert DDS into .tagc and just import it into assembly with your custom name
  
  Also that's pretty interesting
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
- **2021-08-28** `the_heavynator` [💬╎general-help] so im trying to import 2 separate textures into reach but for some reason importing through assembly will only import the first one, if i try to import another one it just duplicates the first one
- **2021-08-30** `rylietw` [💬╎general-help] I'm trying to import two animations from reach, specifically the scene in which Jorge sacrifices himself. Jorges animation is imported and as it should be, but whenever I try to import Noble Six's animations they always go onto jorge. is there a way to stop this from happening?
- **2021-08-31** `benji3892` [💬╎general-help] Hello everyone, I’m really new to blender and I’m just wanting to use it for screenshots of my halo reach character. I have the file to open where you can select what pieces of armor you want on your character. So I got him the way I wanted and then deleted all of the other pieces and export him as a bpx file. When I try to import him on any other map that I have made he comes up textureless. Like I said I am ridiculously new, how I do go about either keeping the textures on him or applying the textures after importing? Or is that not possible and I need to just use the file where the creator is every time?
- **2021-09-30** `sswampp` [💬╎general-help] https://docs.blender.org/manual/en/latest/render/shader_nodes/shader/emission.html
- **2021-10-05** `arithen` [🔍╎texturing-and-shading] Maybe I'm just understanding how they function. Yes, they are in there (as I just discovered), but the one I'm working on right now in blender is titled "Spartan_Visor_Kurt" and I don't see that specific material. 
  
  Nevertheless, the one you just linked is working, despite the name change.
  
  Would I need to use a color ramp node to decide the visor color then?
- **2021-10-19** `sswampp` [💬╎general-help] https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2021-10-20** `itzhouze` [💬╎general-help] Hey there. I need some help. I exported the halo 3 multiplayer map: "warehouse" structure_bsp file to an AMF file using Reclaimer. Then i modified the geometry in blender. Now i exported the modified file as an ASS file. Whats the correct steps to get an structure_bsp file? i tried using tool=>structure, but after 5 minutes it just throws many errors, missing shaders and triangle errors and wont build. Hope you can help me out 🙂
- **2021-10-21** `orangejambo` [💬╎general-help] I'm seeing a lot of fun errors here.
  Just a quick surface level question, is it overly difficult to import custom assets into H3 w/ the newly released toolset? Lets say... a textured cube?
- **2021-10-24** `extreme_narwhal` [💬╎general-help] Hey there, fairly new to blender and trying my hand at porting maps. I was able to find an fbx file w/ textures and bitmaps of a certain map i'm trying to pull into halo 3. I was able to create a .ass off the files then used the H3EK tool fast to get a proper scenario but i'm getting far too many errors for duplicate triangles and collision edges. I had to scale up the map a bit and im wondering if scaling would cause those issues.
- **2021-10-24** `sierra060822` [🦴╎animation] yeeee all ripped and reused stuff from halo reach and ported to blender! i may do a little shader fuckery for the armor before i finish the final render but im actually planning on rendering it tonight while i sleep, now that i've finally bought more storage for projects cause i had like none and had to stop making stuff  😅
- **2021-11-03** `itzhouze` [🔍╎texturing-and-shading] hey there. is here a blender pro? i need some help getting my blender materials exported as a texture (to generate a shader for halo3)
  i have many materials in blender and need to get them exported. (every .dat has its own material)
- **2021-11-04** `itzhouze` [🔍╎texturing-and-shading] I need some help in Blender. Getting all the materials from an imported .ldraw file exported as .tif. I want to create a Shader in Guerilla from this materials. Any help would be appreciated 🙂
- **2021-11-07** `itzhouze` [💬╎general-help] Hey there! i have a long list of materials in blender and want to create a h3 shader for every material. Is there a good automated way to create a shader for every material? The only difference in the .shader will be the ALBEDO base_map filepath.
  
  I tried opening the .shader file and manipulate the filepath it was looking for. But after modifying it, the shader file wont open again in guerilla (i think its corrupted).
- **2021-11-07** `surasia` [💬╎general-help] https://github.com/arkryl/Blader
  [embed] GitHub - arkryl/Blader: Generate (as of now, empty) Halo .shader fi... https://github.com/arkryl/Blader Generate (as of now, empty) Halo .shader files from FBX files - GitHub - arkryl/Blader: Generate (as of now, empty) Halo .shader files from FBX files
- **2021-11-25** `sswampp` [📜╎ripping] https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-2-1
  [embed] fbx-sdk-2020-2-1 https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-2-1
- **2021-12-01** `xsoulstacex` [💬╎general-help] Hey all! I'm trying to use Reclaimer and Architect to extract the multiplayer spartan from Halo 3. I plan on unchecking all the armor pieces except for my spartans armor set so I can pose/animate him in blender for videos or thumbnails. My first hurdle atm is Reclaimer crashing every single time I try to save anything anywhere. I've been troubleshooting for hours and would appreciate any help 🙂
  
  I am trying to save as an obj extracting only the selected geometry, I installed Architect properly and followed the install guide to a T
- **2021-12-01** `xsoulstacex` [💬╎general-help] Yeah blender doesnt like amf's although it looks like I can export those just fine! Where could I find that blender file with all the h3 armor? The set I want is exclusively new season 343 added stuff, some from the latest season so I hope its not out of date
- **2021-12-01** `xsoulstacex` [💬╎general-help] Thanks for pointing me the right way R93! I got my model in blender working fine. Do you know how I would now begin ripping the textures and applying those? I was told I'll have to make my own visor shader especially if I want a new age 343 animated visor
- **2021-12-01** `elitecaden` [🦴╎rigging] > How does one IK Rig a Reach Elite?
  @Haken Studios  I’ve been following this tutorial: https://youtu.be/Pt3-mHBCoQk. I’ve only done the legs so far
  [embed] Blender 2.8 Inverse Kinematics In 2 Minutes! https://www.youtube.com/watch?v=Pt3-mHBCoQk Hey everyone! Today I'll be showing you how to create inverse kinematics for your character in Blender 2.8 in 2 minutes!
  
  If you enjoyed this video, please don't forget to like and subscribe for more content!
  Yo
- **2021-12-01** `xsoulstacex` [💬╎general-help] Anyone have any leads for me on ripping / recreating the animated visors from halo 3? I was thinking that making a custom gradient texture (or whatever the visor looks like) and writing a shader to scroll through the texture's y coordinates would be doable, that's all that 343 are doing in engine anyways. Thing is I don't know how to do that in blender. Any visor texture would be a start, Reclaimer didn't give me one. I think I can ignore the back piece for now
- **2021-12-02** `bios_0307` [📜╎ripping] This is the Destiny 2 NPC shader for Blender, you can cannibalize the Z reconstruct from it: https://drive.google.com/file/d/1JhvfpJNIpZgXqUm1jSnMhlfZm9LymE7F/view?usp=sharing
  [embed] D2 NPC_Enemy Shader 1.4.5.blend https://drive.google.com/file/d/1JhvfpJNIpZgXqUm1jSnMhlfZm9LymE7F/view?usp=sharing
- **2021-12-02** `xsoulstacex` [💬╎general-help] Color me depressed and in denial because I can't believe Blender is limited that way. You very well could be correct about that though. If that is the case then my only options would be to make everything uniform to the chest runes, or perhaps try and put some carefully placed planes over them and parent them to the same bone?
  
  I'm not ready to throw in the towel just yet on him
- **2021-12-02** `ubald007` [💾╎autodesk] Thanks, I had to separate the rubber or (Tech suit) from the armor on the sub mesh level. It was pretty simple, I still don't know how blender dose it with just textures but oh well.. Nobody wanted to explain so I had to make my own solution. Ill dm you a pic when I have a chance. I just got some great news from a H3 stand point so Infinite will have to go on the back burner for now
- **2021-12-03** `xsoulstacex` [💬╎general-help] Hey I'm trying to export my spartan as an fbx but every time I do he loses all of his textures! I think it has something to do with replacing the principled BSDF with the bungie shader on most objects. Some like the visor don't use the bungie halo shader though and still come out gray. Followed all the fbx export tutorials I could youtube up and still my textures dont stick.
  
  Help?
- **2021-12-03** `xsoulstacex` [💬╎general-help] Is it not possible to get my Blender shaders into Unity? Default diffuse maps for the armor don't give it any color I had to do a lot of work for that in Blender using the Bungie Halo Shader. I don't know how I'm going  to get my Spartan colors or emissions in unity :what:
- **2021-12-03** `xsoulstacex` [🔍╎texturing-and-shading] Is it possible to take my Blender shaders into Unity? New to texturing and did all my good texture work for a multiplayer spartan using nodes in blender. Used the Bungie Halo Shader in place of Principled BSDF so I could set the appropriate colors onto the armor. Because the diffuse maps came without color, I added it in blender. I just need to know how to get him looking the same in unity as he does in blender
- **2021-12-17** `thejudsub` [🔍╎texturing-and-shading] Yep. Thats exactly the thing im referencing with the blender parallax build.
  https://devtalk.blender.org/t/parallax-occlusion-mapping/15774
  [embed] Parallax Occlusion Mapping https://devtalk.blender.org/t/parallax-occlusion-mapping/15774 There has been interest in supporting parallax occlusion mapping (POM) in Blender for some time. See for example https://developer.blender.org/T68477, https://blender.community/c/rightclickselect/t9bbbc/ and Pa
- **2021-12-25** `el_perro_de_mario_bros` [🦴╎animation] Hi !
  I am starting modding again and searching videos/tuto to learn how to import animation to 3
  In particular I need reach elites models and animations. I also need reach 1st person animation
  Does anyone have that ?
- **2021-12-26** `young_gusano` [🔍╎texturing-and-shading] @Deskclaw Hey bro, I logged onto blender and tried doing the instance thing. It’s a near complete success! I’m fixing a lot of the broken textures and so far my only issue was with the Elite face but even then, I had just made my own texture for the face. Thanks a million man 😎
- **2022-01-01** `coarforge` [🔍╎texturing-and-shading] https://github.com/Coreforge/blender-halo-infinite/tree/dev Make sure to download the dev branch (master is a bit out of date). It works with bacons shader (and should work with others too), but you need to rename the image texture nodes for it to work, as they are just called "Image Texture" by default.
  [embed] GitHub - Coreforge/blender-halo-infinite at dev https://github.com/Coreforge/blender-halo-infinite/tree/dev Contribute to Coreforge/blender-halo-infinite development by creating an account on GitHub.
- **2022-01-01** `coarforge` [🔍╎texturing-and-shading] I can quickly, but I'm off to bed after that. You go onto the github page and select the dev branch (the link I sent already has that branch selected). You then download the code as a .zip file, which you can then install as an addon in the user preferences in blender. To be able to import textures, you need to install the module texture2ddecoder into blenders python environment by running a few commands in the python console in blender (the commands are in the readme.md). You can then go to import -> Halo infinite rendermodel and import a rendermodel. If you want to import more then just the 3D model (so textures), you have to set the root path property to the root path of where you extracted the modules to (the filechooser doesn't work, so you have to copy in the path). Same for the shader .blend if you want it to automatically apply that shader.
- **2022-01-08** `opr_23b` [💬╎general-help] I wanted to ask a question regarding Halo 2 assets, texture stuff, and whatnot.
  
  So, I've extracted the model of the Elite using reclaimer, and I've imported it into blender with rigging, textures, and whatnot. (I personally preferred to rip the model myself rather than download it from a previously existing source.
  And to cut to the question itself without bloating with too much information, is it at all possible to use some sort of node setup that uses the RGB textures from the game, in order to easily retexture the Elite?
  Basically, you set a specific color in some sort of option, and it retextures the elite using the RGB mask as a "base"
  [attachments] arms_color_change.png
- **2022-01-12** `sunksi` [🔍╎texturing-and-shading] Yeah you can use decals, the default way just kinda sucks ass
  I use Decal machine 
  But if you dont wanna shell out 50 bucks (I dont blame you) ive heard this works
  https://amanbairwal.gumroad.com/l/ImportAsDecal
  [embed] Import As Decal - Free Blender Addon https://amanbairwal.gumroad.com/l/ImportAsDecal My All-In-One RanTools (and P-Cutter) Addon is at flat 20% OFF using the code 'newyear' . You can check it out here:https://amanbairwal.gumroad.com/l/rantoolsv2.1 Out Now!Import As Decal is a Free Addon for Blender 
- **2022-01-12** `activelydrowsed` [💬╎general-help] what do I need to input in the materials box for textures to appear. Do I need the file location their, like ``C:\Users\Desktop\elite\bitmap`` ? I have them extracted but can't get blender to see them.
  [attachments] Capasdasdture.PNG
- **2022-01-13** `ubald007` [💾╎autodesk] https://github.com/TitusLVR/BMAX_Connector/tree/5998c9eb54f5e90bad343e08c1c43cea102d2f8a
  [embed] GitHub - TitusLVR/BMAX_Connector at 5998c9eb54f5e90bad343e08c1c43ce... https://github.com/TitusLVR/BMAX_Connector/tree/5998c9eb54f5e90bad343e08c1c43cea102d2f8a Blender <> 3DSMAX FBX connector. Contribute to TitusLVR/BMAX_Connector development by creating an account on GitHub.
- **2022-01-19** `sunksi` [💬╎general-help] Can't walk you through it cause I'm about to sleep but download this and look up tutorials for it
  https://amanbairwal.gumroad.com/l/ImportAsDecal
  [embed] Import As Decal - Free Blender Addon https://amanbairwal.gumroad.com/l/ImportAsDecal My All-In-One RanTools (and P-Cutter) Addon is at flat 20% OFF using the code 'newyear' . You can check it out here:https://amanbairwal.gumroad.com/l/rantoolsv2.1 Out Now!Import As Decal is a Free Addon for Blender 
- **2022-01-26** `gingerjesus.` [💬╎general-help] How do you save your scenes in Blender without making whole stinking copies of your .blend files?
  I wouldn't wanna have to have multiple copies of the Infinite Elite for every pose I make when that thing's .blend file is a gigabyte big
- **2022-01-29** `pajamaboy` [🔍╎texturing-and-shading] So I opened the ripped spartan (Mk VII) in blender 3.1 last night but the shader appears to be broken? I tried readding the images starting with the normals but I've had no luck so far.
- **2022-01-31** `piratepenguin.` [🦴╎rigging] Has anyone got the female reach marines rigged? I've been trying for a while & they just dead ass look odd, I've attempted multiple times so idk if it's an issue on  my end or if that's just how it is in game
- **2022-02-07** `kalawaki` [🦴╎rigging] Hello yes, Blender newbie here seeking what is probably super basic and simple advice.
  
  I'm trying to figure out how to make the Elite model in #archive-infinite-packs have eyelids that you can close to give it a wider range of facial expressions.
  
  Anyone know a good tutorial that would work for it?
  [attachments] unknown.png
- **2022-02-13** `sledgehammer_` [💬╎general-help] So I know this may be a stupid question, but is it possible to load maps into blender directly from a halo game? Like say for instance I wanted to make a blender file of one of infinite's multiplayer maps?
- **2022-02-27** `plastered_crab` [📜╎ripping] You've already ported it as in you loaded it up into Blender? Or did you go so far as to spend like 2 months in Blender / Assembly and hand tweaking all the textures and tillable bitmaps for each and every surface on the map? 😅 the palm trees alone for that map have like 12 different textures they use as well as a huge terrain shader solely for them. Not to mention the skybox and whatnot.
- **2022-03-05** `defultimate12k` [💬╎general-help] question
  when i was watching the video about exporting Halo render models from Reclaimer i followed all the right steps but for some reason it doesnt keep the color of the rendered model heres an example
  The middle one is the video from Plastered Crab and his came out exactly like that but mine didnt
  are there are certain settings i need to change or configure that i missed?
  @Plastered_CrawDAD
  [attachments] unknown.png, unknown.png
- **2022-03-18** `get2dachopra` [💬╎general-help] Hey there - so I’m trying to recreate my Infinite Spartan with the Scorpion Punch armor coating (does anyone know the color code btw?); how do I just recolor just the right arm, and how do I move the helmet attachment to his left side? Thanks! 
  
  (I’m also new to Blender so apologies if this is a noob question)
  [attachments] InfiniteSpartans.png
- **2022-03-18** `jwgjjman` [💬╎general-help] How to get Halo 2 textures to load properly in blender? I downloaded all the Arbiter and elite . amf and . tif files from the Google drive. Imported to blender with the amf importer tool, but the textures aren't there. I tried it again with Halo 3 arbiter and that worked fine. Is there a different process for Halo?
- **2022-03-19** `lankyb` [💬╎general-help] Hey, was wondering if someone could help me with a few things.
  
  I have a Blender version of my Halo Reach Spartan, however I'm looking at exporting it to Unity so I can do some VR things with it.
  
  I'm aware that the shaders from Blender won't transfer over to Unity, so could someone help me with the model on the Unity side with the shaders? Like I said, the model looks great on Blender, just don't want an insane quality drop when moving it to Unity
- **2022-03-21** `williaso` [🔍╎texturing-and-shading] https://blender.stackexchange.com/questions/214925/how-to-use-a-red-green-color-map-in-blender *Maybe* this'll help? @AverageUsername
  [embed] How to use a Red/Green color map in Blender? https://blender.stackexchange.com/questions/214925/how-to-use-a-red-green-color-map-in-blender (I have modified my initial question after finding out more about the normal maps.)
  I am playing around with some ripped models and I am trying to figure out ho
- **2022-03-26** `snapchipper` [🔍╎texturing-and-shading] would anyone be willing to add the H3 color palette hex values to #📗╎lavadeeto-resources ? It'd be useful for testing CC maps in Blender before exporting my model into MCC proper
- **2022-03-27** `xferginatorx` [🔍╎texturing-and-shading] Not sure where that is but i think i figured it out, i was trying to follow this video but i think that was where i messed up. https://www.youtube.com/watch?v=vVS9HJd73io Was this what you meant?
  [embed] Halo Reach Custom Spartan Creator https://www.youtube.com/watch?v=vVS9HJd73io Basic tutorial on how to replicate your custom Halo Reach Spartan in Blender. Great for cosplay references
  Credit goes to Chief and the Halo Archive for putting together this amazing resource
  https://drive.google.com/file/
  [attachments] 2022-03-27_15_25_03-Window.png
- **2022-03-28** `get2dachopra` [💬╎general-help] Ok so Im still having problems with the Blender file - this specific hip attachment and all of the Reach torso pieces and attachments are all like this. I’ve tried moving everything into a new folder but same issue. Tried uninstalling and reinstalling, same thing. Any ideas on how to fix it?
  [attachments] InfiniteSpartans2.png
- **2022-04-14** `juxtajynx` [💬╎general-help] So I downloaded this pack, https://drive.google.com/file/d/1K2Tjl2_WC4EW0TQ--VRsAhJv1kHy9OKO/view?usp=sharing 
  
  Yet when I go to open it in Blender after downloading it, it says that the file type is not supported? Does anyone know what might be the problem?
  [embed] Infinite ODST Pack.blend https://drive.google.com/file/d/1K2Tjl2_WC4EW0TQ--VRsAhJv1kHy9OKO/view?usp=sharing
- **2022-04-20** `squishypon3` [💬╎general-help] Took a bit of a break working on an H3 map and coming back trying to use reclaimer to get some .tiff files for blender. Anyone know why Reclaimer would have issues opening up h3 .map files on steam h3 mcc?
  [attachments] unknown.png
- **2022-04-30** `awsus` [💬╎general-help] I cant seem to get textures to appear on the Reach MCC props, I import them using the .amf addon and they appear black/glossy. I watched Crab's tutorial that's linked above, and I tried to adjust the texture formats from tif to png and dds and no luck. Any idea what might be wrong?
- **2022-05-03** `Deleted User` [📜╎ripping] https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2020-2-1
- **2022-05-04** `inocturne` [🦴╎animation] https://youtu.be/VMbJIFvNrT8 decided to release the  Halo 3 Art / Diorama / Story Board esqe short
  [embed] The Standoff   -  Halo 3 Blender Enviroment Short   ( WIP  Might Fi... https://www.youtube.com/watch?v=VMbJIFvNrT8 Been working on this blender piece for a while i am not sure if i will finish it but if i do i hope you enjoy its a work in progress render of 5 days work so might render out the last f
- **2022-05-04** `09williamsad` [📜╎ripping] I am redoing my Halo Infinite rip for season 2, how did people manage to export the elite model?
  It gets vert errors when I try, I thought to try catch the erroring section of code but that did not get any usable model.
- **2022-05-04** `fiycsf` [💬╎general-help] If your asking about the infinite pelican, for Vrchat, my personal opinion is likely TLDR'd as "don't". 
  
  (It's not a good thing to cut one's teeth on, start simpler, unless you're a bit of a masochist)
  
  The infinite pelican is like, 30MB of mesh alone last I checked, and would need to be broken up well as to not render all at once,  (Occlusion culling) or else the framerate would be pretty hard hit. This is for mesh alone.
  
  For materials, the pelican as Kayhotic says, also uses a lot of shader work for it's materials, so if you don't have something like amplify shader editor, or shader coding knowledge, it'll be hard pressed to look even close to what you see in infinite/blender.
  
  It also makes use of some clever height tricks for fake depth in panel lines and bolts and dips in the hull, that aren't easily replicated in unity.
- **2022-05-08** `xandion` [💬╎general-help] Or, to simplify those steps: https://blendermarket.com/products/import-as-decal
  [embed] Import As Decal - With Advanced Decal Shader https://blendermarket.com/products/import-as-decal Import As Decal is a Free Addon for Blender that allows you to directly import images as Decals/Stickers and comes with an advanced Shader.
- **2022-05-11** `silent1meow` [💾╎substance-suite] Finally got around to finishing the whole thing
  https://twitter.com/Silent1Meow/status/1524470624457961472
  [embed] https://twitter.com/Silent1Meow/status/1524470624457961472 Finally finished retexturing the Infinite MKV (B) core and also used the Halo 5 Operator helmet. Overall I'm pretty happy with how it turned out.
  #Halo #SubstancePainter #blender 
  
  -Rendered in Blender 3.0 Cycles & Edited in Photoshop-
- **2022-05-18** `thebigdaddy1518` [🦴╎rigging] Weight issues like that honestly shouldn't be too hard to fix, from what I can see they only need a small amount of adjustment. Like Williaso said, I think you should look up a weight painting tutorial. Its all universal so it sould translate well onto the elite, and the armor actually should make it easier. But theres waayyyy too much weight coming from his bones into the others. And I think alternatively you could try Blender's Auto Rig system, Its not 100% but its not terrible
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
- **2022-06-01** `soulburnin` [🔍╎texturing-and-shading] I'm not really familiar with the mod tools, but in Blender I would set the texture to linear/non-color, then adjust the channels individually with a math node set to multiply to reduce the visibility of those lines. Not sure if that's something you can do with the H3 tools
- **2022-06-02** `r93_sniper` [📜╎ripping] For a Comprehensive Halo 5 pack, i'd like it set up as such:
  1. Have all Armors set up with Names, Materials, Rigging, and Skin Weights
  2. Have it set up in 3DS Max with Layers
  3. Have it set up in Blender with Collections
  4. Have a Full FBX with everything exported in a monolithic stack
  5. Have Each individual Armor Set exported as a Seperate FBX
  6. Have all the textures for each individual armor set named correctly and located in their own relevant folder
  7. [Optional] Have a comprehensive blender shader set up
- **2022-06-04** `randomando2902` [💾╎autodesk] @Sketch10121 What is the best way to get these models into maya? I have them open in blender (blend file was included ofc) just wondering the best way to go about building a spartan in maya
- **2022-06-09** `gameoverload` [💬╎general-help] For the pack and or Goblin's shader, the pose has an action pose and rest pose, to put it back into the base pose, switch to the rest pose, and when you want to actually pose, use the action pose.
  Also the Spartan armor is just saved by saving your Blender project, like any other object in Blender, and color saves the same way since it just from changing the shader of the armor piece your customizing.
- **2022-06-13** `rufflestukov` [💾╎autodesk] Hello, I've been trying to use the Halo 2 Anniversary Chief model in Maya for a render and I really don't know how to use the textures, I mean, I only know where to put the normals and the color texture, but in the render it looks bad, any ideas?
- **2022-06-13** `evactouch` [💬╎general-help] (Surasia S2 Infinite Pack, Blender 3.2)
  How can I add a camo onto armor while retaining the color controls?
  My first thought was to hook up a image texture and apply them to the Zone Top/Bot/Mid, and while it does apply the camo texture, it removes color control. And I am attempting to do this with a mostly greyscale and mostly transparent digital camo texture so I can easily change its color.
- **2022-06-14** `rafaleeater` [💬╎general-help] https://www.moddb.com/games/halo-3/downloads/bonobo-version-1003
  [embed] Bonobo [Version 1.0.0.3] Halo2/3/ODST/Reach Animation Extractor file https://www.moddb.com/games/halo-3/downloads/bonobo-version-1003 Bonobo [Version 1.0.0.3] is a Halo2/3/ODST/Reach Animation Extractor used to extract the animations from those 4 halo games, this can be found on halomaps.org, this c
  [attachments] unknown.png
- **2022-06-15** `gameoverload` [💬╎general-help] Through the Blender Spartan 2.0 Pack, via it's Bake outputs on the right of the Shader node where the BSDF connects to the material output. Although you will want to have an image texture node somewhere on each armor you wish to bake out textures for that has a new 1k, 2k, or 4k image ready for the baking of textures.
- **2022-06-15** `butter1734` [💬╎general-help] so i've been tweaking with the reach models from the reach spartan pack, trying to make Jorge, but i've noticed that some things are just scaled up in game for Jorge like the Pads, and the shin of his armour is differently sized and shaped from the regular base armour in game. I don't wanna be lazy and ask for someone to rip it for me, but if someone has his shit or the full FBX on hand, i would gladly appreciate it, if not i'll just go and rip from the game myself
- **2022-06-17** `frostbitecinematics` [📜╎ripping] It's kind of like how you set up the clone markings. Basically each channel of the _W map contains a black and white mask, one for each weather effect. E.g red might be sand, green could be snow, blue could be wet mud when you walk into rivers, etc. I don't use Blender much so I'm going off of what I know from doing it in Unreal Engine, but you essentially use the mask to LERP between the base texture and the weather colour and normal map texture. In Blender, I believe the MixRGB node is the same as UE's LERP. So the mask goes into the LERP Alpha (MixRGB's Value?), the A input of the MixRGB is your base colour and the B of the Mix RGB is a mud texture. Then set the same thing up to blend between the model's base normal map and the weather effect normal map, e.g the mud normals.
- **2022-07-01** `swagdaddy69` [💬╎general-help] Can anyone run me through adding the toon shader in the community uploads channel and make it usable with the infinite spartan pack? Complete blender newbie, hope that made sense lol
- **2022-07-10** `nemesiswmd` [🔍╎texturing-and-shading] hi im new to the server and just downloaded the infinite spartans pack i exported masterchief as obj to put it in element 3d does anyone know if there is a way to export with textures when i try to retexture it in element everything is the wrong color or i might be putting it in the wrong place but im brand new to 3d so any advice will help thankyou
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
- **2022-07-15** `incuriouslog` [🔍╎texturing-and-shading] I just did something I'm very proud of, yet am fairly certain I did in a completely dumbass way, and was wondering if anyone would care to translate my stupidity into correctness.
  I wanted to add a PNG with transparency to the ODST chest attachment for the Mk. Vb core, but I also wanted it to match the distressed look of the metal.
  So I opened the symbol in Photoshop, then added the asg bitmap for the chestpiece to another layer. I lined it up with where the symbol was going to be, created a layer mask on the symbol layer, then applied one of the channels (red, I think) inverted as the layer mask. This matched the distress perfectly with the chestpiece. I saved it as a PNG with transparency.
  I then imported the PNG as a plane into Blender. I added a subdivision modifier and then a shrinkwrap modifier, set to project onto the surface of the chestpiece with a .0005 offset. I sized and positioned the plane, then for good measure made sure it cast no shadow, and paired it to the chestbone of the rig so it moves when I pose it.
  It works exactly how I need it to.
  How should I have done this properly?
  [attachments] blender_22.07.15_21.44.33.920.jpg
- **2022-07-18** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-07-21** `fable_law_458` [💬╎general-help] All good man, we still are improving our skills doing cool things into most of the old bungie halo games, I can truly attest to that with my complex modeling into halo 1 with 3ds max and blender. I'm still learning how to figure out how halo 3/odst and halo 2 works with the lighting and their different quirks which is not the same as halo 1.
- **2022-07-26** `recl8015` [📜╎ripping] Using the new Halo 2 Anniversary mod tool to extract the skyboxes for various sentinelhq BSPs. it gets me the render model but does not pull the textures attached to it. I can extract the render data of og H2 via Tool just fine. Am porting Containment to H3 and trying to get different skybox variants for it, including H2A.
- **2022-07-30** `sunksi` [🦴╎rigging] https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-07-30** `gameoverload` [🔍╎texturing-and-shading] If you read the outputs of the shader, not quite, a shader built in Blender will not work in another graphics engine, without replicating it within that engine's graphical structure.
  In the case of Unity' that is Unity's visual shader code system, or Amplify for a plugin of similar design.
  However Unity comes with a standard shader, which if you read the texture inputs of, will tell you what texture maps you need. Why does this matter?
  If you read the outputs of the shader in Blender, it will tell you what it can 'emit' on the mesh, and guess what Blender's graphics engine, Cycles, can do.
  It can bake out the emitting visuals of a selected mesh.
- **2022-08-01** `guhgunk` [📜╎ripping] hoping for some help with basic fp animation importing into blender. i've imported H3 chief fp arms and assault rifle into blender and am trying to import the ready anim. i get an error on anim import that "Node 'camera_control' not found in an existing armature". is there another model i need to bring into the scene from the map files? thanks!
- **2022-08-01** `baconator650` [💾╎autodesk] Sorry should have clarified, that this is working with animations that were extracted from Halo and then imported. I’ve used animation save/load and find it too limited when working with multiple assets in a large scene. Animation layers work best (for my needs). Experimenting with Blender’s animation importer I noticed it automatically puts animation into a layer.
- **2022-08-05** `soundflare` [💬╎general-help] I'll look it up, was also wondering if Reach models could look decently with a infinite shader if anyone has ever done that in blender, cause I'm probably gonna see about trying that if possible.
- **2022-08-05** `soundflare` [💬╎general-help] Any current Reach shaders at the moment? Am just trying to properly learn blender as best as i can, only just need to understand the different shaders for this is all, along with potentially how to make the Corvette bridges glass see through for a proper visible space backdrop when able. Cause I'm basically wanting to recreate this from the campaign honestly, either with the Halo Reach backdrop or a black void of space backdrop potentially with what i'm currently doing in Blender. Any advice is appreciated ^^
  [attachments] 8-5-2022_4-49-46_PM-2bx1khlc.png
- **2022-08-08** `2nafish117` [📜╎ripping] https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2022-08-08** `heavensc` [🦴╎rigging] https://cdn.discordapp.com/attachments/869803909953245246/1006241357343502427/unknown.png Trying to move an ODST model I downloaded here into FBX to upload to mixamo, but can't seem to do it. Textures won't bake.
  [embed] https://cdn.discordapp.com/attachments/869803909953245246/1006241357343502427/unknown.png
- **2022-08-13** `vat_` [💬╎general-help] If you using unity, you either need to bake out new textures in blender, as the ones in the blend file are procedural, or use this shader set up by point/r93sniper https://github.com/R93Sniper/HaloInfiniteVRCShader. Even tho its for vrc, im pretty sure ill work in general for unity stuff.
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
- **2022-08-29** `nascarlaser1` [💬╎general-help] ok so there are two ways of doing it. Both are easy, ones just a bit quicker if you plan to do this often/have specific needs. 
  
  The default/no addon way: go to a website like polyhaven (my preffered site):https://polyhaven.com/hdris or 3d assets.https://3dassets.one/#type=hdri&order=latest
  
  From there, find an HDRI you like and download the HDR or EXR (depending on website) file.  1K is good if you mainly need lighting, 2-4k is good if you want a background too. 8-16 is just overkill imo and will make your file far larger than it has to be, but if you feel like you need it you can download it too, size doesn't change the steps.
  
  After downloading it, swap your node texture window from "object" to "World". Grab an "Environment texture" node, and then just op your HDR/EXR file into that, and connect it to the color input of the already-made "background" node. 🙂
  [embed] HDRIs • Poly Haven https://polyhaven.com/hdris The Public 3D Asset Library
- **2022-08-29** `nascarlaser1` [💬╎general-help] now under "delay" set it to 5 seconds (the max delay possible. Then in blender navigate to the "shader" or "output" menu of the search thing (where I had the screenshot). Once the 5 seconds is done, it'll switch to screenshot mode, which will freeze your screen and let you click and drag a box over the portion of screen you want to capture. Do that for the menu, then just copy and send the photo here 🙂
- **2022-09-04** `gameoverload` [💬╎general-help] Your probably gonna wanna learn Blender anyway just to know the general process, and it isn't so much making models as presumably working with the existing models from Halo 3, which have all been pulled out at this point one way or the other, or can be pulled out.
  The shader portion would be the harder part, but I know a couple people here have worked on Blender shaders for 3 specifically, unless your doing this in a different engine.
- **2022-09-04** `gameoverload` [💬╎general-help] By tools, I simply mean any specified tools, not assets, that you wish for someone helping you to use.
  Other than that, things like do you want the models used to be accurate to Halo 3/ODST?
  In that case the person helping you will need a Halo 3/ODST shader to work with, which only two people here have done the research to recreate in Blender's node graph based shader system.
- **2022-09-08** `blakjewellio` [💾╎autodesk] on its defense I like the rigs I can find made in maya im lokin at this chat log and asia said 343 uses maya wonder if I can find some of those How did he rip infinites spartans
- **2022-09-11** `Deleted User` [💬╎general-help] Hello, I am looking for three models for blender: Halo Reach-> Assault rifle, Magnum, Mounted Turret. idk how this server works or who i need to talk to, but i would like some help
- **2022-09-12** `piratepenguin.` [💾╎autodesk] 93 pm me, I want to talk to you about Max08 and reaching out the the Unification team. We might have something to work with for porting this stuff over into current 3dsmax builds
- **2022-09-12** `chunch` [🔍╎texturing-and-shading] This is the best one tho @The Dog https://youtu.be/r_5suSLUucU
  [embed] How to Port Coatings to Blender from Halo Infinite [100% Accurate] https://www.youtube.com/watch?v=r_5suSLUucU Swatches: https://mega.nz/folder/yWoCxTrZ#0emVAwxOqA85fg9fePy3tA/folder/7XYBVCxT
  
  make sure all of these are in a /materials folder.
  
  CoatingTool: https://mega.nz/file/oqwx0LRJ#cL4vRS7QPj08
- **2022-09-13** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-09-15** `blakjewellio` [💾╎autodesk] I found a few elites on sketch fab but I haven’t seen if they’re very well rigged I know there’s one that cost money that specifically says it’s for Maya and it’s pretty cheap for a model if you find it just look at description
- **2022-09-15** `blakjewellio` [💾╎autodesk] https://sketchfab.com/3d-models/halo-elite-minor-ca4504cbd0f548d8aaefa31f41db97a2
  [embed] Halo Elite Minor - 3D model by Jawley3D https://sketchfab.com/3d-models/halo-elite-minor-ca4504cbd0f548d8aaefa31f41db97a2 Halo Elite Minor made using the following:
  
  
  Maya - Rigging and Rendering
  Substance Painter - Texturing and Baking
  ZBrush - Sculpting
  
  This model is now available on my Gumroad!
  
- **2022-09-16** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
- **2022-09-22** `spartanj104` [📜╎ripping] Hello, 
  I saw that there was no "Infinite Brute Pack" so I stick to the task...even though I've never done that in my life and I am struggling xD.
  I've already ripped the game, output the raw models, sorted through Blender and now I come to the part I hate the most: the textures/shader/UV.
  I try to watch tutorials on this but it doesn't help me understand how you managed the textures like on "Infinite Spartans Pack" I need someone to show me the light 😅 🥲 
  Thank you in advance ✋  🙂
- **2022-09-27** `somethinspartan` [💬╎general-help] Hey so I finally imported the map condemned from reach into blender, and couldn't figure out how to add my character into it without appending it, and I think it might be causing a problem for me to not have my character save it in place when I close out of blender, so the real question is, is appending a model or object the wrong way of adding a character into another project, or is it something else? (sorry for a dump text)
- **2022-09-28** `krisiejamie` [🔍╎texturing-and-shading] Hey guys. im working with this blender halo infinite asset thats popular i think. and the air assault helmet does not have any sections to change visor colour. any help?
  Image
- **2022-10-03** `trystt` [💾╎autodesk] Okay looks like this is a bit of an FAQ but I'm just gonna ask to put it in my own words, to be absolutely clear.
  The assets from https://discord.com/channels/819709630540021810/821145601416364063/821146113062731776 (Or anywhere in the #🔗╎all-halo-asset-resources section. I am looking to just put together parts for my own Spartan in Maya for the purposes of posing/modeling for artwork.
  Is Maya capable of using the files? Or is Blender basically a necessity?
- **2022-10-04** `elitecaden` [🦴╎animation] Gif of an elite running. Couldn't for the life of me figure out how to speed up the gif to the original framerate. Couldn't find how to export it as a gif in Blender and the websites that provided mp4's to gif's don't let me change the speed or changing the speed is very finicky
  [attachments] Animation.gif
- **2022-10-06** `jaeger1648` [🔍╎texturing-and-shading] Hello, I was told my message might fit better over here
  
  I'm very new to blender and I'm using the reach spartan pack. I couldn't figure out how to export an fbx with any materials/textures at all. Actually the decals exported with it but that's it. Now I kinda know it's because I need to texture bake?
  I found a video on how to texture bake, but ever time I try to smart UV project it messes up the textures, shifts them around on the model a bit. It seems it's because it's not unwrapping right, but I have no clue how to fix it.
  
  I'm about to go to bed, but any help would be greatly appreciated!
- **2022-10-15** `plastered_crab` [📜╎ripping] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). I have included a modified version of th
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
- **2022-10-23** `sierra060822` [💬╎general-help] so I'm at a loss rn with trying to export H3 ODST's Dare, someone mentioned the one I posed had a messed up rig and it did so i went to fix it but I've just finishing trying so many different import and export methods and it still persists! If anyone else wouldn't mind trying to export it lmk if its broken for you as well!
- **2022-10-25** `exporush` [💬╎general-help] How do I get my laptop open the template.py file using notepad like in this video? 
  
  https://www.youtube.com/watch?v=r_5suSLUucU
  [embed] How to Port Coatings to Blender from Halo Infinite [100% Accurate] https://www.youtube.com/watch?v=r_5suSLUucU Swatches: https://mega.nz/folder/yWoCxTrZ#0emVAwxOqA85fg9fePy3tA/folder/7XYBVCxT
  
  make sure all of these are in a /materials folder.
  
  CoatingTool: https://mega.nz/file/oqwx0LRJ#cL4vRS7QPj08
- **2022-10-30** `neapaw` [💬╎general-help] i just finally got the elite files open and im trying to change the color of the eyes themselves. I am new to blender and i  know its not the bitmap that needs the modifications. assistance please?
- **2022-11-08** `gloorby` [💬╎general-help] Okay so I want your guys opinions on something. I'm attempting to make a Halo animation but I'm unsure of which software to use. I'm tossing up between Blender or UE5 but I'm just not sure. I'm far more experienced with UE5 but I can't import many of the assets I want to use due to the Halo Infinite shader. On the other hand Blender is much easier to animate in plus the assets are all there but I find it harder to make scenes and the render times are insanely long even for a 30 second clip. What do you guys think I should do??
- **2022-11-11** `hydranaught4444` [🦴╎animation] well for importing into halo 3 im gonna do it individual blend files but importing into reach with the plug in it. an all go in at once so it would be better to just have them in the file
- **2022-11-11** `waketheshark` [💬╎general-help] Is there a way to access the areas that cutscenes take place in? I'm specifically wanting to access one of Reach's where Noble Six is overlooking New Alexandria. Alternatively is there a way to extract that level into blender somehow
- **2022-11-12** `waketheshark` [🔍╎texturing-and-shading] Not sure if this is the right place for this but I'm trying to use Reclaimer for exporting Reach maps. I followed what I can find and downloaded the shared reach maps as well as the level I want to open. I have them all in one folder extracted from their zip folders. I can open the level I want in the 'Map Viewer' window with all of its folders. But I'm getting an error: "System.IO.InvalidDataException: Found invalid data while decoding" when trying to open one of the render_model files like in the youtube tutorial and I can't wrap my head around how to fix it
- **2022-11-13** `waketheshark` [📜╎ripping] I'm following the tutorial to export from reclaimer and to import into blender. I've exported the .amf for m50_000, and within the same folder, exported all bitmaps of m50_000. But when I import into blender with the AMF importer (making sure that it's looking for tiff files), nothing is textured. Is there something extra I need to do for Structures? I have all of the shared reach maps: campaign, mainmenu, and shared.
  [attachments] image.png
- **2022-11-14** `waketheshark` [📜╎ripping] Naw trying to import the map from the intro scene from Exodus in Reach for Blender rendering. Having an blender artist replicate the scene and we need the level in blender to do it well
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
- **2022-11-17** `gameoverload` [🔍╎texturing-and-shading] Either bake the textures out in Blender to standard ones that are static ones, or make your own shader to work with the base ones..or use an existing shader like the Unity project Halo Infinite shader.
- **2022-11-17** `thechipsmotherfuckendubbo` [💬╎general-help] Heyo. I got a question for blender stuff. I wanted to use a Halo reach helmet on a halo 4 techsuit, but I cant find just the helmet blend file. Is there a way to tak it from the Reach pack or is there a set of helmets I can download?
- **2022-11-18** `grandbacon` [🔍╎texturing-and-shading] *I've* only ever used my old infinite shader on MP armor as that's what it was made to do, I don't know how it works on the elite and it definetly won't work in unity since it's a blender shader do if you want it to work in unity you'll have to bake some textures
- **2022-11-22** `thechipsmotherfuckendubbo` [💬╎general-help] Heyo, I wanna grab the Halo Reach Pelican for blender. We had one, but the googledrive is down. 
  
  https://discord.com/channels/819709630540021810/821676515099279374/824506529604173845
  
  Anyone able to help? Gimme a ping, would be legendary
- **2022-11-30** `gameoverload` [💬╎general-help] OBJ basically holds your model, and one UV set, and that is about it on the important parts.
  
  FBX on the other hand can hold your model, armature, multiple UV sets, texture data ( that is properly assigned so even 3D can read it ), some standardized shader data, and other bits that are dependent on what you are working with.
  
  Basically FBX is best to use in every way.
  
  GLTF is similar, but just simply open source, but not as robust as FBX given FBX has been around as long as Autodesk has used it as a format.
- **2022-12-01** `gameoverload` [💬╎general-help] Sounds like it either couldn't find certain files for something while finding others, and that cascaded into not being able to find a bunch of stuff which crashed it, or it couldn't find anything properly and crashed, or it found everything it could, was too much, and crashed.
  
  In which case, you may have to reassign textures manually in shader so not to crash Blender trying to do it all at once.
  
  Also a pink texture issue, at least for the Spartan Pack, can also be due to a missing HDRI.
- **2022-12-08** `stumpyj` [🦴╎animation] Is anyone here familiar with both 3ds max scripting and blender scripting? I am converting a script from max to blender, and everything is great until I do animations. Both of these animations were extracted using the same .gr2 data (left: max - imported to blender as fbx, right: blender). Both the good animation and the bad work on either rig identically to eachother, telling me the rig is not the issue.
  [attachments] image.png
- **2022-12-08** `exporush` [💬╎general-help] I always feel stupid when watching this video cause I can never get it right. So what am I supposed to change when when he says to change the extension?
  https://youtu.be/r_5suSLUucU?t=35
  [embed] How to Port Coatings to Blender from Halo Infinite [100% Accurate] https://www.youtube.com/watch?v=r_5suSLUucU&start=35 Swatches: https://mega.nz/folder/yWoCxTrZ#0emVAwxOqA85fg9fePy3tA/folder/7XYBVCxT
  
  make sure all of these are in a /materials folder.
  
  CoatingTool: https://mega.nz/file/oqwx0LRJ#cL4
- **2022-12-12** `gameoverload` [💬╎general-help] Now you would create a map in say Blender if your using the Halo Blender tool set, then use all the other tools to handle the materials and such, but otherwise, you use the mod tools to look at most of the data for a level, while the source shader code would be read in something like C++.
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
- **2023-01-11** `k1d650` [🦴╎rigging] If anyone could help it would be much appreciated.  I'm trying to pose my Reach spartan while following the only tutorial I could find.  The video seems to be specific to Infinite Spartans, as the rig for the Reach spartans looks different, and the commands they use don't seem to line up.  I'm brand new to Blender and was hoping someone could give me a hand.  Thanks!
- **2023-01-12** `dioisdead` [💬╎general-help] im having a issue with getting the reach model i made in blender over to unity its kind of my first spartan so i dont necessarily know what im doing and the main issue in the mats aren't saving with the model
- **2023-01-16** `.moonzen.` [📜╎ripping] is there a newer way of ripping reach animations? i used the video tutorial and it just keeps saying "no valid armature" and stuff like that when i import the jma. i think bonobos scuffed or something
- **2023-01-17** `r93_sniper` [📜╎ripping] https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2023-01-20** `rubyqueen0` [💬╎general-help] Im guessing this question has been asked a few times (or something in this vain).  Im looking to get the halo reach/infinite Spartans tho blender to unity to vrc. I have a "good" understanding of the unity side of making avatars but am having a lot of trouble getting it over from blender to unity. missing texters, bones braking, what's the profred shader and so on. If there's anyone that can run me tho the best way to go tho this proses that would be wonderful. (can dm) Sorry if this is the wrong place to ask
- **2023-02-05** `spencerbutler` [🔍╎texturing-and-shading] okay so I'm an absolute blender noob, I threw myself into the deep end this afternoon trying to get my Reach Spartan put together from Chief's spartan pack by following this tutorial https://youtu.be/vVS9HJd73io
  [embed] Halo Reach Custom Spartan Creator https://www.youtube.com/watch?v=vVS9HJd73io Basic tutorial on how to replicate your custom Halo Reach Spartan in Blender. Great for cosplay references
  Credit goes to Chief and the Halo Archive for putting together this amazing resource
  https://drive.google.com/file/
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
- **2023-02-08** `faminemc` [🔍╎texturing-and-shading] hey so its probably just my pc but I was using the halo reach spartans pack to make my own spartan and it was fine but when I started trying to make my own halo reach spartan and I turn on textures my blender freezes
- **2023-02-22** `planetalexanderprojects` [🔍╎texturing-and-shading] Download can be found here:
  https://www.405th.com/forums/threads/procedural-armour-texture-setup-for-blender.53457/#:~:text=Cycles%20render%20engine.-,22/02/23,-%3A%20Version%202
  [embed] Procedural Armour Texture Setup for Blender https://www.405th.com/forums/threads/procedural-armour-texture-setup-for-blender.53457/ 22/02/23: Version 2 is available, download at the end of the post!
  
  I recently made a Belos Horan helmet from MCC that was to be used in a render of a spartan. To make 
- **2023-02-26** `wake9090` [💬╎general-help] So if I wanted to import the coating redacted records from the reach core. Do I use `M:\__chore\gen__\objects\characters\spartan_armor\coatings\reach\reach_spartan_style{ct}.materialstyles` and select it in the style or is there a different place I have to find it? And if this is correct, would the material path then be `M:\objects\characters\spartan_armor\coatings\reach\`?
- **2023-03-15** `ProBot ✨` [💬╎general-help] [Completely Ready 4 Blender (CR4B) Tool]
  
  CR4B Tool is a Blender plugin that aims to eliminate most of the tedious manual setup of Halo 3/ODST/Reach levels and assets with just a single button click!
  
  Download Link: https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool
  
  Tutorial showing setup and use: https://youtu.be/749xwvg-GK4
  
  Requirements: Halo 3 / ODST Mod Tools + Files from GitHub
  [embed] GitHub - PlasteredCrab/Halo-3-ODST-CR4B-Tool: One-click game-accura... https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool One-click game-accuracy tool for Halo 3/ODST/Reach assets in Blender - GitHub - PlasteredCrab/Halo-3-ODST-CR4B-Tool: One-click game-accuracy tool for Halo 3/ODST/Reach assets
- **2023-03-16** `Deleted User` [🔍╎texturing-and-shading] im trying to port some textures from blender and they show up like this, what am i meant to do?(its a sangheili ultra helmet from halo infinite)
  https://i.gyazo.com/ebdb483e1769d654bf85530562a03b0b.png
  [embed] https://i.gyazo.com/ebdb483e1769d654bf85530562a03b0b.png
- **2023-03-26** `plastered_crab` [📜╎ripping] in that case you will likely need a few things
  1: the .map files of the level !maps
  2: the mod tools for Reach from Steam so you can see what the values are for each material
  3: to use Reclaimer to rip the level geo and get it into Blender
- **2023-03-27** `plastered_crab` [📜╎ripping] https://youtu.be/749xwvg-GK4
  [embed] CR4B Tool Setup Guide (Game Accurate H3/ODST/Reach Importer) https://www.youtube.com/watch?v=749xwvg-GK4 GitHub Link: https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool
  
  Halo Archive Discord: https://discord.gg/haloarchive
  
  Completely Ready 4 Blender (CR4B) Tool
  This is a tool I have spent the l
- **2023-03-28** `wulfrick_productions` [💬╎general-help] Hey everyone getting better at blender and trying to get some of my OC's renders made up.
  
  Been a few years, but just looking for the Gungnir helmet from Reach and the Maverick from Halo 5. Last I was looking around they were still getting ported into packs.
- **2023-03-29** `thechipsmotherfuckendubbo` [🦴╎rigging] I hate to sound Like a pain, But I want to try and IK the Halo Reach spartans, but the Tutorial I am using confuses me. Anyone know what to do? 
  https://www.youtube.com/watch?v=Pt3-mHBCoQk
  [embed] Blender 2.8 Inverse Kinematics In 2 Minutes! https://www.youtube.com/watch?v=Pt3-mHBCoQk Hey everyone! Today I'll be showing you how to create inverse kinematics for your character in Blender 2.8 in 2 minutes!
  
  If you enjoyed this video, please don't forget to like and subscribe for more content!
  Yo
- **2023-03-30** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2023-04-01** `onemadwomble` [🔍╎texturing-and-shading] Need help. Trying to use textures from Halo Infinite ODST Pack. Don't know what the channels in mask_0_control or mask_1_control are for. I believe the asg_control is red=specular, green=roughness. Is there any documentation that might shed light on how I might setup an unreal shader? Would the best approach be to reverse engineer blender shader?
- **2023-04-04** `haloboxstudios` [💬╎general-help] Hey, uhhhh- really stupid question from a really stupid person here. I downloaded the newest spartan pack today and extracted the pack and textures into a new folder on my laptop, but when I open up the Blender program all my assets are locked in a solid gray color. Am I missing something here?
- **2023-04-10** `orkscool` [💬╎general-help] Anyone know how to add shield effects to my reach models? such as breaking, and regening? My knowledge of blender Is pretty minimal so If anyone knows a simple solution to making the shield effects that’d be great.
- **2023-04-19** `existential_selkath` [🦴╎animation] sorry for just jumping in here. Anyone know how to actually import halo 4 animation files into blender or anything? The JMA files are in h4-packs but I cant figure out how to do anything with them. All the tools seem to be built for anything older than H3
- **2023-04-21** `gameoverload` [🔍╎texturing-and-shading] You'd need to use a specific compression scheme and a standard shader setup in UE if your baking things down, which doesn't use the original normal map anyway, but a baked down one from the shader in Blender.
  
  Otherwise you'd need to make a shader to use all of the original textures as it does in Blender via the Halo Infinite Material Shader, presumably based on the HI Material Shader since that is the most accurate one at the minute.
- **2023-04-25** `gameoverload` [💬╎general-help] At work, so no, and I'd need to get into the dev tools first so you'd have all the data for it, but I made a Blender specific and accurate shader for WOTC expansion armor / weapon assets alongside the shaders needed for skin, eyes, teeth, and hair, which would go with any head / hair assets ( for both the base game and expansion content ) and any expansion specific armor / weapon assets.
- **2023-05-01** `hobbes240` [📜╎ripping] Hi, I'm new to this community and well applications like blender and all, however I'm understanding quickly as there are a lot of tutorials out there. One issue I am having that I need help with is ripping the animation for assassinations with Bonobo for Halo Reach then transferring it to Blender. I have seen the tutorial from PlasteredCrab, however one version of Bonobo is telling me that the Reach Files are unsupported, while the other is saying it can not find the file. Bonobo is working for Halo ODST, but isn't working for Halo 3 either. I'm uncertain if maybe the application is no longer working or what. I used this link for .map files https://haloce3.com/xmf/, downloaded all of the shared files for Reach, ODST, and 3. As for the .net downloads, this one works https://www.microsoft.com/en-us/download/details.aspx?id=48130, but this one doesnt https://www.microsoft.com/en-us/download/details.aspx?id=53344. Does Anyone know what the issue could be and a solution for it?
- **2023-05-04** `cameraguy` [📜╎ripping] For some reason with Reclaimer when I Rip from 3, ODST, Reach, maps half of the props I see in the Reclaimers preview are missing in the exported .amf
  I'm using the Halo xmf website .map files
  
  Any ideas?
- **2023-05-15** `harambesfistvictim` [🔍╎texturing-and-shading] Not sure what happened here. But all I did was copy the visor set up to another helmet after changing the color of the visor pattern, saved. Went to change the color back and it's all messed up. Almost like something in the textures is magnified or rotated ( Which I did nothing of) So I deleted my visor pattern set up. Still fucked. Imported the helmet from another older file to copy the shader set up. Fucked still. what happen & how do I fix?
  [attachments] Screenshot_2023-05-15_001438.jpg
- **2023-05-16** `dgenpoly` [💾╎autodesk] I've been trying to get Maya to fit easier into a Halo Mapping workflow, one of the many issues I have ran across is material names and prefixes. Coming from 3dsMax I was expecting a straight forward transition, for the most part it has. Today I ran into a material issue while assigning my sky material, when I go to name it "+sky" Maya removes the "+" automatically. Has anyone else ran into this and have a fix or a workaround?
  [attachments] image.png
- **2023-05-19** `bertglassed` [💾╎autodesk] You probably need to make a script or plugin for maya for it to work correctly. Maya is great, but especially when it comes to sth old like halo where it was built around 3ds, you will run into issues. Voice versa when working with games that were built on maya, 3ds users will have a hard time
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
- **2023-05-24** `theidiot5651` [💬╎general-help] https://www.youtube.com/watch?v=Jm39-vYl1zo&t=86s
  
  https://www.youtube.com/watch?v=LdmoBY08LTM
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo&start=86 This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan p
- **2023-05-31** `metamoq0395` [🔍╎texturing-and-shading] This seems like a better place to ask than #💬╎general-help, i got a mirage IIC im trying to export out, and iv gotten a way to bake the textures onto a png so i can take them to unity. However, I need this camo as well. Is there someway i can find the texture file used for it and plug it into the shader editor in blender? Im a total noob when it comes to the shader editor btw, i just need it for this one use.
  [attachments] Halo_Infinite_Screenshot_2023.05.28_-_08.56.10.92.png
- **2023-05-31** `el1tejim` [💬╎general-help] hey, was just wondering if anyone could spot me some advice. amateur blender artist here: not a single clue what I'm doing
  
  I wanted to add a sky for the world, and once I looked it up, I applied the dynamic sky addon. in the video tutorials, everything seems to work instantly - but when I tried this, and when I've tried rendering with this, there's 0 colour whatsoever. 
  
  I'd even settle for making the sky a solid colour and add an overlay afterwards; this is stressing me out lol
  [attachments] image.png
- **2023-06-03** `Deleted User` [🔍╎texturing-and-shading] I’m using this on blender https://drive.google.com/file/d/1aRVcA0V5R5gTeSkuzGjYH5z0hQ0kTspZ/view?usp=sharing and I cannot get the textures to come up at all I’m just trying to get the textures up for this
  [embed] Reach Spartans.zip https://drive.google.com/file/d/1aRVcA0V5R5gTeSkuzGjYH5z0hQ0kTspZ/view?usp=sharing
  [attachments] IMG_6683.jpg
- **2023-06-09** `___swan` [🔍╎texturing-and-shading] My bad if this is the wrong channel. I'm new to extracting Halo models, is there a way to automatically assign correct textures to Reach models in blender or do I just have to guess by name?
- **2023-06-12** `_ashrowan` [🔍╎texturing-and-shading] Fellow shader wizards I need your help setting up shaders for a Halo 5 Sparton for friend in blender. My goal is to bake the textures so I can optimize it for for VRChat. Both Recluse Male and MK V Alpha have the R and G channels inversed in the Open Halo 5 unity shader for a "Fake Zone 3" Not really sure how to apply that in blender.
  [attachments] image.png, image.png
- **2023-06-14** `gingerjesus.` [🦴╎animation] I'm really late, but I strongly recommend Blender. Modern tools & technology, you can use/develop assets that aren't restrained by the likes of the Source engine. Source engine itself(for HL2, Gmod, SFM, such forth) has various hardcoded limitations for assets that effectively make cloth-sim impossible for example. It does not support modern shader methods such as PBR, forcing experts to use very roundabout methods to emulate it. Lastly, SFM itself has memory leak problems that can be *very* tedious to deal with when making a scene with a massive variety of assets or even a number of high-quality assets.
- **2023-06-27** `olipog` [💬╎general-help] I'm having some trouble opening the Reach scenery pack, as I can't figure out how to opem .amf files. The Blender addon featured in The_Plastered_Crab's tutorial doesn't seem to work anymore.
- **2023-07-05** `nosensemade` [💬╎general-help] Hey I am new to this and looking to work on a few spartan renders for a myself and a few mates. Does anyone know where to find a reach style operator helmet? Btw I am using the halo infinite blender assets and for the sake of ease I am looking for the helmet with infinite style textures.
- **2023-07-05** `r93_sniper` [📜╎ripping] https://www.youtube.com/watch?v=aYnVWsOZHlE&t
  same idea applies to reach
  [embed] How to port Halo 3 models into Blender (Full in-depth process) https://www.youtube.com/watch?v=aYnVWsOZHlE&start=0 Hey sorry this video is so long, I wanted to stuff in as many examples of the full process as possible to really give you all an idea of what exactly you need to do to get the models lo
- **2023-07-07** `spacemonqee` [💬╎general-help] is there a way in blender to bake the final look of a Infinite model into one texture for each channel (diffuse, roughness etc.) even though it has multiple materials and UV sets? for example the scorpion cannon consists of decals, grilles and several other geo, I want to bake it down so I have one shader for the whole cannon with all the decals and greebles and stuff baked down to one texture
- **2023-07-08** `commanderprime` [💬╎general-help] I have just downloaded the Halo Infintie Elite file and I followed the steps to get it from IK to FK (or whatever the opposite of the default is) but when I export it and reimport it to a new Blender file the fingers dont move with the rest of the body.
- **2023-07-09** `deltahalocontrolroomcore` [💬╎general-help] Got a quick question-
  
  Doing a render for a couple people that won a championship. One of them has a knife on their wrist, which means I have to unparent it from the original armature and apply it to the new one with new vertex groups and all. Should be the easiest thing in the world. However, the instant I tab into edit mode, the shader *instantly* breaks. I know it's not an issue with Blender, or something I'm doing out of the ordinary (because the Mirage pack can tab in and out of edit mode just fine), which means this issue is present only with assets in Spartan Pack 3.5. Does anyone know a fix or workaround to this issue?
  [attachments] image.png, image.png
- **2023-07-10** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2023-07-11** `pmarc0011` [🦴╎rigging] ok so.... I bet you get this question often here but looking through I didn't find a solution that worked so: 
  I'm kitbashing some odst pieces on the Mirage core made by Dan, I appended all the pieces and once I deleted the rigs of the appended pieces they oriented themselves almost perfectly to the SPI model and its rig. Buuut if I parent them no weight is present; automatic weights kinda work but always leave stuff behind. What do you think is the best course? Just automatic w. and then paint what is missing? Did I forget something? (I'm a noob in Blender 😅 )
- **2023-07-15** `sswampp` [💬╎general-help] to do it properly would take a lot of explanation, but if you have to do it manually and not with the CR4B tool, you want to get the shader info (for each material) from the game either with assembly or with the editing kit (guerilla) and copy it over to a blender shader/nodegroup that's set up for game accuracy
- **2023-07-22** `ProBot ✨` [💬╎general-help] [Completely Ready 4 Blender (CR4B) Tool]
  
  CR4B Tool is a Blender plugin that aims to eliminate most of the tedious manual setup of Halo 3/ODST/Reach levels and assets with just a single button click!
  
  Download Link: https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool
  
  Tutorial showing setup and use: https://youtu.be/749xwvg-GK4
  
  Requirements: Halo 3 / ODST Mod Tools + Files from GitHub
  [embed] GitHub - PlasteredCrab/Halo-3-ODST-CR4B-Tool: One-click game-accura... https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool One-click game-accuracy tool for Halo 3/ODST/Reach assets in Blender - GitHub - PlasteredCrab/Halo-3-ODST-CR4B-Tool: One-click game-accuracy tool for Halo 3/ODST/Reach assets
- **2023-07-24** `wildbucko` [🔍╎texturing-and-shading] Hey could anyone help me with Baking textures for export?  I am using Halo Reach armour and I wanted to bake the materials into one for export, but following a tutorial, I smart UV project and it messes up the textures
- **2023-07-29** `ProBot ✨` [📜╎ripping] [Completely Ready 4 Blender (CR4B) Tool]
  
  CR4B Tool is a Blender plugin that aims to eliminate most of the tedious manual setup of Halo 3/ODST/Reach levels and assets with just a single button click!
  
  Download Link: https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool
  
  Tutorial showing setup and use: https://youtu.be/749xwvg-GK4
  
  Requirements: Halo 3 / ODST Mod Tools + Files from GitHub
  [embed] GitHub - PlasteredCrab/Halo-3-ODST-CR4B-Tool: One-click game-accura... https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool One-click game-accuracy tool for Halo 3/ODST/Reach assets in Blender - GitHub - PlasteredCrab/Halo-3-ODST-CR4B-Tool: One-click game-accuracy tool for Halo 3/ODST/Reach assets
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
- **2023-08-16** `exitity` [💬╎general-help] I was wondering if there was an easy way to adjust what parts of the armor are colored by the Secondary Color, particularly for the Halo 3 Spartan rig? I was trying to see if I could adjust the coloring to look like the Freelancers from Red vs. Blue Seasons 9-10, where the armor is modeled after Halo 3 put the color patterns are more like Halo 2 (like the armor is Halo 3 armor but their secondary color is coloring those parts on the wrist and the parts on the side of their thighs rather than the shoulders, and also the small bit on the helmet rather than the long stripe). I only know the basics of Blender so if its complicated I probably won't bother, but if its easy that would be cool.
- **2023-08-28** `koi191` [🦴╎animation] Anybody know if any of the Halo Spartan packs have functioning IK rigs? I was playing around the with Reach pack and it seems to be only FK (which is fine in itself) but the rig is kinda scuffed so that you can't paste a pose X-flipped. Orrr maybe I'm just too much of a Blender noob to use it correctly
- **2023-09-02** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo?si=7V7QIHIQTpvSh9IS
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2023-09-04** `plastered_crab` [📜╎ripping] https://github.com/77Mynameislol77/HaloAnimationRepository
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository A Repository of ripped Halo Animations for use in MCC modding. - GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of ripped Halo Animations for use in MCC m
- **2023-09-08** `ProBot ✨` [📜╎ripping] #Halo Infinite Model and Texture Extraction
  Goes over everything you need to get it working step by step
  
  I made sure to start from absolute scratch as if nothing was installed too so you can see the full process (only takes around 5 mins of actual installing and setup. I just spent a bit of extra time to show it in action and give some extra tips on finding certain models)
  
  Find the video here: https://youtu.be/AOStYxiDLis
  
  Get CR4B Tool here: https://github.com/PlasteredCrab/Halo-CR4B-Tool
  [embed] How to rip Halo Infinite Models and Textures with CR4B Tool and HIRT https://www.youtube.com/watch?v=AOStYxiDLis This video goes over how to extract Halo Infinite models and textures (even batch extracting) using HIRT CLI and CR4B Tool.
  
  It is a step by step guide which I start from complete scratch
- **2023-09-15** `r93_sniper` [🦴╎animation] https://github.com/77Mynameislol77/HaloAnimationRepository
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository A Repository of ripped Halo Animations for use in MCC modding. - GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of ripped Halo Animations for use in MCC m
- **2023-09-19** `tacoco53` [💬╎general-help] well ok some context is an order i wanna use the models from this addon for halo 2 i found but dont know how to unpack a .map, if its a .lvl its easy but this is for unity so i wont need shader data
- **2023-09-20** `thefunman37` [💬╎general-help] Got it from this video if that helps (The Google Drive link in the description doesn't work anymore) https://www.youtube.com/watch?v=vVS9HJd73io
  [embed] Halo Reach Custom Spartan Creator https://www.youtube.com/watch?v=vVS9HJd73io Basic tutorial on how to replicate your custom Halo Reach Spartan in Blender. Great for cosplay references
  Credit goes to Chief and the Halo Archive for putting together this amazing resource
  https://drive.google.com/file/
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
- **2023-09-29** `plastered_crab` [💬╎general-help] https://youtu.be/Jm39-vYl1zo?si=cKIiV8tuybx8RE_G
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2023-10-01** `chiefster_4014` [📜╎ripping] cr4b tool's github https://github.com/PlasteredCrab/Halo-CR4B-Tool
  [embed] GitHub - PlasteredCrab/Halo-CR4B-Tool: One-click game-accuracy tool... https://github.com/PlasteredCrab/Halo-CR4B-Tool One-click game-accuracy tool for Halo 3/ODST/Reach assets in Blender that also exports assets from every Halo game - GitHub - PlasteredCrab/Halo-CR4B-Tool: One-click game-accuracy t
- **2023-10-01** `plastered_crab` [📜╎ripping] https://github.com/PlasteredCrab/Halo-CR4B-Tool
  [embed] GitHub - PlasteredCrab/Halo-CR4B-Tool: One-click game-accuracy tool... https://github.com/PlasteredCrab/Halo-CR4B-Tool One-click game-accuracy tool for Halo 3/ODST/Reach assets in Blender that also exports assets from every Halo game - GitHub - PlasteredCrab/Halo-CR4B-Tool: One-click game-accuracy t
- **2023-10-01** `plastered_crab` [📜╎ripping] https://youtu.be/xhRq8ChDz3s?si=1Iiahv19MhxdeMUi
  [embed] Halo CR4B Tool v2 5 Install Guide https://www.youtube.com/watch?v=xhRq8ChDz3s Greetings Beta testers! It is time for a new install video for the recent changes I am making so I am making this to briefly go over what has changed and how to install it.
  
  Get CR4B Tool beta here: https://github.com/Plas
- **2023-10-03** `ProBot ✨` [💬╎general-help] [Completely Ready 4 Blender (CR4B) Tool]
  
  CR4B Tool is a Blender plugin that aims to eliminate most of the tedious manual setup of Halo 3/ODST/Reach levels and assets with just a single button click!
  
  Download Link: https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool
  
  Tutorial showing setup and use: https://youtu.be/749xwvg-GK4
  
  Requirements: Halo 3 / ODST Mod Tools + Files from GitHub
  [embed] GitHub - PlasteredCrab/Halo-CR4B-Tool: One-click game-accuracy tool... https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool One-click game-accuracy tool for Halo 3/ODST/Reach assets in Blender that also exports assets from every Halo game - GitHub - PlasteredCrab/Halo-CR4B-Tool: One-click game-acc
- **2023-10-13** `gunk337` [💬╎general-help] This is more of asking for a favor than for help, but can someone give me the model for the map Lone Wolf from reach for blender? I know there's a lengthy process which can make it look good and like how it does in reach but I'm not trying to do all that right now and I know someone with experience could do it 80x faster than someone new to blender and this whole process.
- **2023-10-16** `gameoverload` [💬╎general-help] You don't need to do any of the location, rotation, nor scale stuff in UE since as you can see, none of the default values for that were changed at all in Blender.
  
  You also do not need to invert the normal map, unless it was baked out as OpenGL ( You can change this in Blender's Cycles bake settings. ), otherwise, it should be DirectX by default.
  
  You don't need any of the strengths unless you have them in Blender.
  
  Also make sure your non-color maps have sRGB turned off in UE.
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
- **2023-10-29** `evactouch` [💬╎general-help] trying to figure out importing reach stuff with cr4b. ive got the models and levels showing up, but blender just freezes when i hit import with some models or levels selected for a minute, before unfreezing with nothing added. no new collections, and no new models.
  [attachments] image.png, image.png, image.png
- **2023-10-31** `ProBot ✨` [💬╎general-help] [Completely Ready 4 Blender (CR4B) Tool]
  
  CR4B Tool is a Blender plugin that aims to eliminate most of the tedious manual setup of Halo 3/ODST/Reach levels and assets with just a single button click!
  
  Download Link: https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool
  
  Tutorial showing setup and use: https://youtu.be/749xwvg-GK4
  
  Requirements: Halo 3 / ODST Mod Tools + Files from GitHub
  [embed] GitHub - PlasteredCrab/Halo-CR4B-Tool: One-click game-accuracy tool... https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool One-click game-accuracy tool for Halo 3/ODST/Reach assets in Blender that also exports assets from every Halo game - GitHub - PlasteredCrab/Halo-CR4B-Tool: One-click game-acc
- **2023-10-31** `charmingquark09` [🔍╎texturing-and-shading] if you are using blender, the way that I have duplicate textures linked is to go into the **"shading"** tab, and at the top of the grey workspace there is this little beachball-looking icon. Click on the beachball and a drop-down menu appears. Either make a new **material **or edit an existing one and name it something easy to type. Click on each object you want to have that texture **separately **(as far as i know, blender wont let you assign a texture to multiple objects at once... which sucks. please correct me if im wrong lol) and using the beachball drop-down menu, apply the texture. 
  
  The good thing about this method is, you can edit just one material and it applies to everything you've assigned to it. Good for having a primary/secondary color that you can change on the fly, or to test out color schemes. To get rid of un-used materials, I believe there is an extension in blender that lets you delete all unused materials, but i cant remember what its called.
  [attachments] image.png
- **2023-11-06** `caldoric` [💾╎autodesk] i've got a number of .max files from a couple old games (unrelated to the Halo franchise) that i ned to get into blender, and for whatever reason, trying to use AD's free trial of 3ds max isn't working. more specifically, the program has repeatedly refused to let me log in for the few days i've actually had it installed, from the day i installed it til now. otherwise, i'd be doing this shit myself.
- **2023-11-06** `caldoric` [💾╎autodesk] There's a tool or two for trying to import the specific model filetype that that engine used into Blender, but it's rather hit-or-miss at times, and said tool definitely wasn't designed for dealing with avatars/humanoid models, so those fail to import almost universally.
- **2023-11-11** `ProBot ✨` [📜╎ripping] #Halo Infinite Model and Texture Extraction
  Goes over everything you need to get it working step by step
  
  I made sure to start from absolute scratch as if nothing was installed too so you can see the full process (only takes around 5 mins of actual installing and setup. I just spent a bit of extra time to show it in action and give some extra tips on finding certain models)
  
  Find the video here: https://youtu.be/AOStYxiDLis
  
  Get CR4B Tool here: https://github.com/PlasteredCrab/Halo-CR4B-Tool
  [embed] How to rip Halo Infinite Models and Textures with CR4B Tool and HIRT https://www.youtube.com/watch?v=AOStYxiDLis This video goes over how to extract Halo Infinite models and textures (even batch extracting) using HIRT CLI and CR4B Tool.
  
  It is a step by step guide which I start from complete scratch
- **2023-11-12** `ProBot ✨` [💬╎general-help] [Completely Ready 4 Blender (CR4B) Tool]
  
  CR4B Tool is a Blender plugin that aims to eliminate most of the tedious manual setup of Halo 3/ODST/Reach levels and assets with just a single button click!
  
  Download Link: https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool
  
  Tutorial showing setup and use: https://youtu.be/749xwvg-GK4
  
  Requirements: Halo 3 / ODST Mod Tools + Files from GitHub
  [embed] GitHub - PlasteredCrab/Halo-CR4B-Tool: One-click game-accuracy tool... https://github.com/PlasteredCrab/Halo-3-ODST-CR4B-Tool One-click game-accuracy tool for Halo 3/ODST/Reach assets in Blender that also exports assets from every Halo game - GitHub - PlasteredCrab/Halo-CR4B-Tool: One-click game-acc
- **2023-11-16** `mozria` [💬╎general-help] I'm new here. Where would I find a guide on how to properly install + configure Reclaimer, Architect and C4AB Tool to extract and work on Reach map bsps in Blender?
  I tried following the video from Plastered Crab for how to get his tool working, but I've run into a lot of problems (largely in that I flat-out cannot get Reclaimer to funtion).
  I also don't have an "Append Nodes" button after setting up the tool in Blender :/
- **2023-11-16** `mozria` [💬╎general-help] I have a mod called Halo: Reach - Sky on Fire. It's a Firefight map based on Beachhead that has about a half-dozen fortifications made out of sandbags that are important for the gameplay. However, because they are composed of hundreds of individual sandbag units they are a big drain on map resources.
- **2023-11-16** `plastered_crab` [💬╎general-help] https://youtu.be/Q_LMjZkp1jo?si=HLXDWdE7qbLv0sLz
  [embed] BEGINNER Blender Forge Tutorial | Mjolnir Forge Editor (Halo MCC) https://www.youtube.com/watch?v=Q_LMjZkp1jo Yes you heard me right! You can use Blender to Forge in Halo Reach! This new tool gives the user the ability to manipulate Forge objects in Blender and then "export" them into your Halo The 
- **2023-11-19** `gameoverload` [💬╎general-help] In Blender terms, its a Mix ( Color, Vector, Float ).
  In UE terms, its the same, a Lerp. ( If I remember right. )
  In Unity terms, it depends on whether your using a third party shader tool, or Unity's own native shader tool.
- **2023-11-27** `ianstl` [🔍╎texturing-and-shading] Hey all! I'm seeing posts about the broken shaders on blender 4.0, so I'm wanting to port some older assets from this server to 4.0 using the newer inifinite shaders that are compatible. I know the basics of blender/3dmodeling, but I get a little lost in the sauce with the shader nodes. Could anyone help me out with this or point me in the right direction for a tutorial? Thanks!
- **2023-12-01** `Deleted User` [🔍╎texturing-and-shading] Hey everyone, I'm trying to bake out the textures on this ODST in Blender here for the use in Maya's Arnold Renderer. It's for a school project. I've noticed that when the UV isn't in the 1 to 1 square I can't bake out maps properly. (They come out all black). And if I try to unwrap the UV again it messed up the texture influence. (Wrinkles on clothing will be horizontal instead of verticle, ect.) So here's the major question. How do I bake out a texture when the UV it's self isn't set in the 1 to 1 position on the UV editor. Sorry for the long post.
  [attachments] image.png
- **2023-12-03** `purplmunkii` [🦴╎rigging] https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2023-12-09** `gameoverload` [💬╎general-help] Well the major change was how specularity is handled now, particularly in the input change of now the Specular Level and Specular Color are two seperate inputs, and Blender has now the habit, with old enough BSDFs, to tack on a mix multiply node which sets the B input to the original Specular Color value they used, because originally it was just the Specular input, which was internally the Specular Level * Specular Color, which is what the mix multiply node is doing that they tack on.
  
  That is the issue, in full context.
- **2023-12-11** `weavelcorrosion` [💬╎general-help] I need help right now. (Before you ask, I found the Halo 3 armors. And the blend file is fucking AWESOME!) But I'd like to ask one simple thing, how do I color the spartan, and how do I export only the shown objects in blender? (Blender Newbie here.)
- **2023-12-13** `blue.nasca` [💬╎general-help] Silly blender question - if I wanted multiple Infinite Multiplayer Spartans in one scene how would I go about doing that and having their rigs and stuff self contained? Normally when I copy a spartan from one scene and paste it into another with an already active spartan it seems to mess with the rigs
- **2023-12-17** `nihren` [🔍╎texturing-and-shading] Figure I'll try asking in here. Need some help with textures. I'm trying to upscale textures, but if I upscale the Color & Control textures as is it doesn't always work. Worked fine with the HI Grunt, however it's completely breaking with the H5 Grunt. Tried extracting the color channels, upscaling them individually, and then recombining them via nodes in Blender, and it gets close, but not quite there.
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
- **2023-12-22** `coarforge` [💬╎general-help] I don't know what aftereffects needs, but you typically can't just export a model with textures, as each application generally uses different shader systems. Some formats (like fbx and dae) can keep some texture and shader information, but both applications have to support it with the same names, and you have to use a shader that can be exported properly (not sure how well blender can usually do that)
- **2023-12-25** `amsierra` [🦴╎animation] Speaking of! I just re started this project finally, using HR animations. Just remastering the final cutscene. https://www.youtube.com/watch?v=bshivViVWbA
  [embed] Halo Reach: Lone Wolf Remastered in Blender WIP https://www.youtube.com/watch?v=bshivViVWbA This is a remaster of the final halo reach cutscene with noble six in their final moments, recreated in blender and in 4k. Still a work in progress! I plan to add audio effects, post effects (fog, dust, etc),
- **2024-01-05** `tomhogenberg` [💬╎general-help] general help needed with the importing of the sabre from reach, using it as a placeholder/first iteration for an animation. maybe someone knows specifically what causes the texture glitching and behavior with reach assets. its not reliant on the texture(s) and materials applied. anyone has a clue, if this is halo (reach) asset related?
  [attachments] Screenshot_2024-01-05_135220.png
- **2024-01-10** `nikolaas4430` [💬╎general-help] I followed Plastered_Crab's guide on importing Halo 3 maps to Blender, but after editing the map a bit, are there any guides on how to export that map back to H3 / Sapien? (.ass to .scenario)
- **2024-01-12** `.black_wick` [💬╎general-help] I'm trying to port my Reach Spartan to UE5 using the official blender plugin but I keep seeing these 2 errors for every part. I'm new to this so I have no idea what to do to solve this. Thanks!
  [attachments] image.png
- **2024-01-17** `dagnammit00` [📜╎ripping] Does anyone else have a problem with the cr4b tool when trying to import Halo 3 multiplayer spartan textures? I keep getting a message that says "bpy_prop_collection[key]:key "shader" not found"
- **2024-01-18** `_tobyr` [🔍╎texturing-and-shading] Hi all, I recently downloaded CanadianDragon's Noble Team models to mess around with and I'm having some trouble with the materials. Most of the model is showing up as white, both in material preview and rendering. For some reason the shins on Jorge's model are colored in fine, but the rest of the model is white.
  It looks like the materials are set up so that the textures provide detailing to the armor, and the color comes from elsewhere. I say this because the diffuse textures have no color, just detailing, with dark highlights along armor ridges and transparent where there is no detailing. These detailing highlights show up on the entire model, but the army green color only shows up on Jorge's shins -- the rest of the model is colored white. This is puzzling because the shins are part of the same model that composes the whole torso & lower body -- the materials and textures cover those anatomy parts. But only Jorge's shins are colored.
  Oh, and side note -- all the stuff that works uses a .tiff textures in the materials. Some of the stuff uses .png textures in their materials and none of that stuff works.
  I have enough experience in Blender to poke around and make general deductions about what's going on, but not experienced enough to do more than that. I also don't fully understand how to work with materials pulled from game files, with the ways stuff gets bundled into different channels of a single texture to be more efficient. If anyone has personal experience and can hel ...[cut]
- **2024-01-19** `frostbitecinematics` [💬╎general-help] yeah. Basically the texture image files are all fair game to use because they are owned by 343, but the actual node logic in Blender used to combine all the properties together (like colour, scratch and dirt intensity, etc) is technically copyright-able by whoever setup the node graphs.
- **2024-02-05** `.williamfaucher` [💬╎general-help] Hey folks, so I'm a bit of a blender noob here. I've got the amazing Infinite Pelican model here: https://discord.com/channels/819709630540021810/958797365085630494/983603593892487239
  But I was wondering if anyone knows how I can export that into Unreal, while preserving the texture paths? I am fully aware I need to rebuild the shader in Unreal, but if at least the textures were still linked it would be a solid starting point.
- **2024-02-08** `snipermann02` [💬╎general-help] hey so im like super new to trying blender shit and I downloaded this ODST file thingy to just mess around with it to see if I could try and learn the program
  
  im not understanding why the helmet is completely white when I've changed every non visor/grime color to purple... even if I zoom in real close there's no purple at all anywhere... Im only using purple as a placeholder to try and spot easier where the colors are
  
  sorry if this is kinda a dumb question
  [attachments] image.png, Screenshot_2024-02-08_033640.png
- **2024-02-09** `xen` [🦴╎rigging] Blender noob here trying to export (as an FBX into UE) - I was able to get the reach core in working just fine, trying to get the same for chief but this is happening. Any ideas on how to fix this? Thanks!
  [attachments] armor-binding.mp4
- **2024-02-12** `cbunny20` [💬╎general-help] so i just finished adding all of the recoloring how do i export the spartan so i can use it in unity and have all the texture and armor color changes still intact. do i need to compile the color changes i made?? what do i do
- **2024-02-14** `chiefster_4014` [💬╎general-help] There's no way to export the color than to do it with baking, but there is a shader for Unity that should work for what you need.
  https://gitlab.com/FiyCsf/open-halo-reach-spartan-shader-for-unity
  [embed] Cameron "Fiy" F. / Open Halo Reach Spartan Shader for Unity  · GitLab https://gitlab.com/FiyCsf/open-halo-reach-spartan-shader-for-unity This should play nicely with all Halo Reach MP Spartan Textures etc, and is written using the amplify shader editor and is clearly commented should anyone wish to.
- **2024-02-17** `atlasguitar` [💬╎general-help] Hey there, I'm just starting out tinkering around with MCC a bit, and am hoping I can get some help or pointers here. I found the H2A Chief in H3 tags on Nexus and was able to get that all sorted in the H3 campaign, and would like to learn how to go about actually making those H3 tags. I was able to find Index and export the Arbiter, but I'm not really sure where to go from there - it looks like when importing to Blender bones and meshes aren't parented (is that the right phrase? lol) but I basically don't know what I'm doing, so if anyone can help me out at all that'd be great :)
- **2024-02-19** `chiefster_4014` [💬╎general-help] Make sure both materials have the image texture you're baking onto, then use the outputs labeled Diffuse, Metallic, etc. on the shader to bake. You'll have to set bake as Emit for everything except the normal, which for that you would just use the BSDF output for rather than the Normal output, and set bake type to Normal.
  Also when you bake the normal, if you're on Blender 3.6+ you'll need to set the model to shade smooth rather than shade auto smooth. You can set it back to shade auto smooth after baking the normal.
- **2024-02-20** `trisckit` [💬╎general-help] map porting process via cr4b is good for halo reach/3/odst right
  
  is there an equivalent tool/setup/guide for doing the same with older game maps like halo ce and 2? I mostly just wanna get the old CE version of Hang Em High into blender
- **2024-02-21** `gameoverload` [🔍╎texturing-and-shading] It goes Zone 0 ( Black below all zones ) then Zone 1 - 6, then scratch is calculated per Zone via the scratch mask as the mask for where the scratch effect occurs, which in game is 1 value for scratch intensity, but is divided into a per zone value in the Blender shader, and then grime is on top of that via the grime mask in the ASG.
  
  The actual names of the base textures in the actual shader are as follows:
  Base Occlusion ( ASG )
  Material ID 0 ( Mask 0 ) ( Zone 1 to 3 )
  Material ID 1 ( Mask 1 ) ( Zone 4 to 6 )
  Base Normal ( Normal Map )
- **2024-02-22** `atlasguitar` [💬╎general-help] Can anyone help/point me in the right direction to properly understand basically how to port a model into H3? 
  
  I'm very new, so I'm finding it a bit difficult to find resources that relate to exactly what I want to do. Lots of MP based stuff, but I mostly want to mess with Campaign, so those kinds of things are harder to find than your run of the mill "here's how to make a weapon do X" kinds of tutorials.
  
  Specifically, I'm trying to wrap my head around porting H2A models to H3 (and presumably, the process for actually porting things into H3 in a workable state will be similar for different sources?). 
  
  I've extracted files using Index, everything's good in Blender, but I'm iffy after that. For example, I was able to create a new weapon for H3 that's the H2A Battle Rifle, and the model ports and loads in Sapien, but I can't get first person to work (telling me a matching node can't be found, which sounds to me like I need to have different naming conventions in the skeleton (if that's what it's called for a weapon) to match H3's BR model naming conventions?) and I'm having a particularly difficult time finding resources regarding bitmaps and shaders, so I can't really get the model textured. 
  
  Another thing I was trying was with the H2A Chief model, and the problem I run into there is a "no animation graph" error and I can't seem to find much info about how to port new models to use existing animations.
- **2024-02-22** `chiefster_4014` [💬╎general-help] If you use export the .render_model of the asset you want to replace from the mod tools, you can use its rig and markers, which should be easier than trying to use the ones that come with H2A. You'll just have to make sure the bone names match before replacing. If they don't, you can rename the bone names and they should work.
  You'll also have to make sure your asset matches the scale of H3.
- **2024-02-22** `chiefster_4014` [💬╎general-help] .render_model is the tag used to store model data in Halo, and for H3 you can export that data with Tool with the ``tool extract-import-info`` command. There's more info about it on the C20 site. For most things it'll extract a .ass file, which you can then import into Blender or whatever you're using.
- **2024-02-22** `pendragone01` [💬╎general-help] Hello everyone! I stumble upon this discord while looking for some specific answers about modding Halo, and I'm stoked to be here! Right now I'm trying, in Blender, to change the color of certain shaders on the marine I've ripped from Halo Reach, and I could really use some help. If y'all could point me towards any specific tutorials or documentation on it, I'd really appreciate it.
- **2024-02-22** `ProBot ✨` [📜╎ripping] #Halo Infinite Model and Texture Extraction
  Goes over everything you need to get it working step by step
  
  I made sure to start from absolute scratch as if nothing was installed too so you can see the full process (only takes around 5 mins of actual installing and setup. I just spent a bit of extra time to show it in action and give some extra tips on finding certain models)
  
  Find the video here: https://youtu.be/AOStYxiDLis
  
  Get CR4B Tool here: https://github.com/PlasteredCrab/Halo-CR4B-Tool
  [embed] How to rip Halo Infinite Models and Textures with CR4B Tool and HIRT https://www.youtube.com/watch?v=AOStYxiDLis This video goes over how to extract Halo Infinite models and textures (even batch extracting) using HIRT CLI and CR4B Tool.
  
  It is a step by step guide which I start from complete scratch
- **2024-02-23** `ricky764` [💬╎general-help] I know this group is more to talk about models and Blender, but I have a problem with Halo Infinite, it turns out that they put in the store a discounted armor color and I decided to buy it with some credits I got in the battle passes, the issue is that I bought it but when I select it, it tells me that I have not bought it and when I try to equip it from my store it sends me an error, does anyone know what I can do? :ChiefCry:
- **2024-02-26** `coarforge` [💬╎general-help] If you have everything set up, select the Image Texture node you want to bake to (just click on it in the shader editor) and then in the Properties Panel click the bake button (the panel where you set up the settings under `BLENDER PREFERENCES`). Depending on your blender version, the settings might be under the Render Properties tab instead of the Scene Settings. Also make sure you have cycles selected as the render engine instead of eevee
- **2024-02-27** `blue.nasca` [💬╎general-help] So I'm not sure if this would be classified as a blender issue or not but - I'm running into an issue where I set up an image in blender and render it, and then save it to my computer. When I open the render in my image editing software (I use GIMP) the image looks totally fine and correct - but if I try previewing the image in something like the Windows Photo app, the colors are washed out. Likewise, if I proceed to then upload the image to something like imgur or Discord, the colors wash out there, too. What the heck's going on, anyone know? I thought maybe it was a color management issue but from what I've researched on it, if that were the case, the colors would be busted inside the photo editor too ... but they aren't.
- **2024-03-06** `atlasguitar` [💬╎general-help] Hey so I've been using Index to rip H2A assets and port them to H3, but I get this trying to open the sniper rifle to export it. Anyone able to help - either get it working, or with another H2A sniper source?
  [attachments] image.png
- **2024-03-07** `coarforge` [💬╎general-help] I also have model export in infiniteExplorer, which exports some material parameters too (though HIRT I think also does rigs iirc, so it might be better to use that for now). If you open a mode (or sbsp) and go to the mesh viewer tab, you can click on `add all Meshes to Exporter` (or go through individual meshes if you want to). Then, under `Tools -> Export current scene`, you can export those models, which will also export some material parameters in a json file in the same directory you save the dae in (some info is currently still missing regarding material styles, but all parameters from the material and shader variant should be there).
  Using `Tools -> Export Bitmaps for current Scene`, you can also export all bitmaps used by materials used by currently loaded models. 
  I know it's unfortunately not very intuitive to use, I'm just really not a UI designer
- **2024-03-08** `goajango` [💬╎general-help] Hello, i have download the spartan pack 3 of this video (https://www.youtube.com/watch?v=LdmoBY08LTM&t=69s), and I would like to export it to 3ds max. I tried to export it to fbx and obj but the textures are not implemented in 3ds max. Does anyone have a solution ?
  [embed] Adding Custom Camo to your Spartan (Halo Infinite Spartan Pack 2) https://www.youtube.com/watch?v=LdmoBY08LTM [Link to over 1,000 greyscale camo patterns in description]
  
  [NOTE] For new versions of the Spartan Pack you need to duplicate your camo plugin nodes. If you connect 1 camo shader node to al
- **2024-03-10** `coarforge` [📜╎ripping] A quick script to set up materials from an infiniteExplorer export. You need to load the script in blender and run it in the editor after putting in the correct paths at the top. With my test shader, you also need to append the material `testmat_color` to the file first (that shader is everything but correct). I haven't written any documentation for the shaderdb file yet, but basically, it maps material parameters given in the exported json to parameters in the blender material (currently, only image textures, UV map indicies starting at 0, booleans indicating if UV3 (UV2 when starting at 0) should be used and float values are supported, I haven't added colors yet). 
  If there are multiple shaders listed, the script calculates a score for each shader by incrementing the score for each parameter the shader uses that is present in the material by one and decrementing it by 2 for each parameter the material doesn't have but the shader has if it's not marked as optional. If the score is 0 or lower, the shader isn't used. This is far from a great way to do this, but it was quick to do.
  There's also currently no material style info being exported by iE yet, but at least for maps, not too many materials seem to have it anyways from what I've seen
  [attachments] matsetup.py, shaderdb.json, mat.blend
- **2024-03-19** `ljzman10` [💬╎general-help] Hello, does anyone know the best way to export the halo reach Spartans pack into Unreal Engine with all the textures and the meshes separated from each other not combined into one object.
- **2024-03-25** `coarforge` [💬╎general-help] Hi, please use english in this server. You're also much more likely to get an answer that way.
  For your question (I think, I only know very little french), infinite for a lot of things only uses control maps for the materials, so you need a shader that uses those. Someone probably has made one for unreal you could use, or you can bake the textures in blender so that you get the usual textures
- **2024-03-26** `ubald007` [💾╎autodesk] Though I am unsure as to the facts, I really don't think the designers of Blender actually are deliberately messing with the FBX to provide there users with a messed up FBX? certainly not out of disliking Autodesk. Maybe like Spartan CDR.Casey said about lic though if that was the case and they rewritten the FBX I think they would have called it something else all together I mean if there was a lic issue. I do know the straight out of Blender FBX in the newer versions, import fine into Max now. It's much harder to build anything to be cross platform then a lot of ppl think, I think in this case this was just a growing pain and nothing more 🙂
- **2024-03-28** `unscfighter` [📜╎ripping] Hey im trying to export these buildings from reach and im seperating each one by material so its easier for me to edit them in another program and when i do that all other similar objects seem to disapear?
  [attachments] image.png, image.png, image.png
- **2024-03-29** `hydranaught4444` [💬╎general-help] doing shader stuff in blender is always fun ik how to do it in 3 but then i feel like a caveman in blender https://discord.com/channels/819709630540021810/832145913177702470/1028805622008909915😂
  [embed] Discord - A New Way to Chat with Friends & Communities https://discord.com/channels/819709630540021810/832145913177702470/1028805622008909915%F0%9F%98%82 Discord is the easiest way to communicate over voice, video, and text.  Chat, hang out, and stay close with your friends and communities.
- **2024-03-31** `chiefster_4014` [💬╎general-help] When you export as FBX from Blender, make sure to tick the Limit To Visible Objects box so only the armor you want exports.
  Also, textures will usually never import on their own, especially since the spartans have a custom nodegroup for their textures to work. You'll need to find a shader for Unity made for the textures.
  [attachments] image.png
- **2024-03-31** `machinesnerve` [💬╎general-help] so ive been moving from sfm to blender and am just so confused, im havinf alot of trouble moving my viewport around and with moving my halo reach models into poses lol, kinda feel like ive made a mistake.
- **2024-04-01** `ubald007` [💾╎autodesk] I don't know much about Maya but in Max if you really need to rerig your already riged character skinwrap is a handy tool, you may need to google and see if Maya has a similar modifier. As R93_Sniper said if exporting from blender save as FBX or Dae either will keep the rig in tact and you should be able to just do your animation then export to Ue..
- **2024-04-03** `finjodon` [💬╎general-help] Hello my Halo Brothers!
  
  Was curious to check in here after rendering my old Reach look :fingerguns: 
  
  Are there possibly blender files that the community's shared where the model is already posed in an immersive environment (perhaps with weapons), where one could essentially trade out the armor & color selections for their own look?
  
  I assume I'd need to ensure that I have both the models (as I currently do) and the Reach weapons downloaded in order to open such a blender file?
  
  Basically just trying to get a cool shot of my old spartan for a pfp!
  [attachments] image.png
- **2024-04-04** `dg5312` [💬╎general-help] Hi 
  @Gravemind2401
  Some later versions of 3ds Max (maybe 2020+) cant work with your script, because now use by default physical materials and cant be changed.
  Can be fixed, please?
  I tried to do it, but my experience in scripting is 0 basically and i get a misunderstanding with 3ds max docs because we have two parameters for create a shader but just make a physical material again qwq
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
- **2024-04-16** `shady.pl` [🔍╎texturing-and-shading] Ok basically to people who havent read #💾╎blender recently, I cannot for the life of me figure out how to fix the textures for the DMR from the reach UNSC Weapons by chiefster, basically there is a really bad seam and something up with the thing and thanks to ALG we're basically at this point which the issue seems to be the material/textures in the files for the DMR
  [attachments] DMRHELP.blend
- **2024-05-05** `setht1204` [🔍╎texturing-and-shading] https://www.youtube.com/watch?v=y_lRfuCg1NA Seems to be the resource we're looking for. I don't believe there's a way to just toggle different coatings on and off - it needs to be done through the Shader Editor. Little tedious but it's easy enough.
  [embed] Blender basics for making your own Spartan https://www.youtube.com/watch?v=y_lRfuCg1NA This video goes over the very bare bones knowledge needed to have fun playing with cross core ideas and making fun coatings for your dream Spartan in Blender. This will be a very beginner friendly tutorial as well
- **2024-05-05** `unbatedspy8` [💬╎general-help] I was customizing a Halo Reach Spartan model and when I saved it didn't keep the paint scheme I put on the armor chest piece. I switched to the shading tab, used object mode to select the chest, and then switched to texture paint to color the chest but it did not save the paint scheme but saved the armor attachments I had put on the model. Am I painting the model wrong? I'm completely new to Blender so I have no clue.
- **2024-05-20** `spartanatic` [💬╎general-help] ```Make your Halo Infinite Spartan in Blender```
  Tutorial by Plastered Crab
  https://www.youtube.com/watch?v=y_lRfuCg1NA
  
  Posing Tutorial by Plastered Crab
  https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] Blender basics for making your own Spartan https://www.youtube.com/watch?v=y_lRfuCg1NA This video goes over the very bare bones knowledge needed to have fun playing with cross core ideas and making fun coatings for your dream Spartan in Blender. This will be a very beginner friendly tutorial as well
- **2024-05-21** `inversefrog7689` [💬╎general-help] Anyone willing to help me getting this to install to blender? I have the .zip file, but I assume it doesn't install like a normal addon since it did nothing when I went that route.
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2024-05-24** `gameoverload` [💬╎general-help] Easiest thing to do is set up via CR3B tool, and then unpack the textures from there, and use the texture paths in the shader as reference to the texture files themselves, but you can also view the textures in the image viewer in Blender as well.
- **2024-05-29** `matt161` [💬╎general-help] Hey everyone
  So I'm sorry to bother you with this but it's driving me nuts
  I have an error everytime I'm trying to render a scene with any ODST kitbash here (either marine or mirage based)
  I'm always getting this error:
  Invalid handle in cuModuleGetGlobal_v2(&mem, &bytes, cuModule, "kernel_params") (C:\Users\blender\git\blender-v410\blender.git\intern\cycles\device\cuda\device_impl.cpp:671)
  
  When I remove the model it renders fine
  When I switch to CPU it works too but not in GPU
  
  Help please
- **2024-06-02** `zadeinator` [💬╎general-help] https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2024-06-05** `chiefster_4014` [🔍╎texturing-and-shading] Infinite's spartan materials are procedural so there's only masks. There's no albedo, roughness, etc. until the masks are used & combined inside a shader. to get what you need you'll have to bake out the albedo, roughness & such from the shader in Blender.
- **2024-06-08** `dyland64` [💬╎general-help] Looking for an H3 Blender Shader Setup. Something to accurately represent objects, levels, etc 
  I recall there being a blend file with some to pick from but I've had a hard drive failure since downloading it and can't seem to find it anymore.
- **2024-06-13** `rook` [💬╎general-help] Hey folks; doing some work for a friend of mine getting his Spartan into Unity/VRChat; it seems to use a lot of shaders/nodes inside of Blender (Named Halo Infinite Shader 3.0) rather than textures (I.E the ones I unpacked). Is there a way to get these packed into reglar PNG files to pull them into Unity? I've tried Baking but it just says that  "No valid selected objects"
- **2024-06-16** `cythecy_` [🔍╎texturing-and-shading] Just got here after the server was suggested by NotPipi.
  
  I've been successfully pulling models through Reclaimer and it brings along the TIFs for the different image maps. I'm trying to port them into Blender using the AMF importer, successful, but I'm having issues figuring out how to properly set up the shader graphs for various models. CE seems relatively straight forward but later games have different detail maps and such and I just can't figure out how to get it done correctly. I did Boneyard from Reach and the ground looked completely wrong just mixing the different maps together with the correct scaling on each.
- **2024-06-25** `cythecy_` [🔍╎texturing-and-shading] Just want to double-check, I'm working with Reach assets in Blender.
  Can I assume that all the environment mapping for a single level would be the same?
  So same cubes, tints, roughness?
  I'm using the shader graph found in the Falcon asset, credit says @Soul made it
- **2024-06-28** `lt_crunch236` [🔍╎texturing-and-shading] I hope this is the right place for a question like this, but does anyone know anything about H3EK not letting you change shaders for existing render models? I am duplicating the MP Chief biped to have two different color schemes. But I can’t have the different color schemes because they share the same shader which is locked to the render model. Thanks again.
- **2024-07-02** `o.v.e.rh.a.u.l` [💾╎substance-suite] Hello good night ppl. I'm trying to upload some guns from the Assets Packs called *HALO 5 Guardians - Weapons* But when i open on Blender it looks normal and 500% nice. But when I export to Substance to make new textures. The guns looks like this \/
  [attachments] image.png
- **2024-07-06** `dieofyes` [💬╎general-help] hi, i just got here, where would i go to download the halo: reach armor files to recreate multiplayer spartans in blender? if that even is a thing, i just got blender and im still figuring out how to use it. thanks
- **2024-07-09** `juliociraptor` [💬╎general-help] Hello everyone! I'm trying to import animations from the Jackal in Halo Reach into Blender, but I can't. Everything works fine with the Jackal from Halo 3, and I followed the tutorial here on the Discord server and in the YouTube video: 'https://www.youtube.com/watch?v=oIIHsOifz60&t=151s'."
  [embed] Halo Animation Ripping / Porting [Blender] https://www.youtube.com/watch?v=oIIHsOifz60 This video goes in-depth with the process of extracting Halo animations and porting them into Blender. I provide all the useful tools you need in order to do this as well as a .blend file with my Tactical Moa for 
- **2024-07-10** `juliociraptor` [💬╎general-help] "Well, actually I'm creating a player model for GMod, with custom animations from the Halo games. I've managed to import animations into Blender following the tutorial step-by-step. The steps I followed were as follows: I downloaded the Reclaimer and Bonobo programs, and downloaded the ".map" map files to extract models and animations. So far, so good. After obtaining the "amf" model of the jackal and the animations in "JMA," I use a Blender addon to import and export files from Halo games. Here's where my problem with the model and animations from Halo Reach arises. While following these steps, I successfully imported the model and animations of the Halo 3 Jackal. However, when I try to import the model and animations of the Jackal from Halo Reach, Blender shows an error message: "Internal error: <class 'ValueError'>(could not convert string to float: '0,9999999')."
- **2024-07-16** `cythecy_` [💬╎general-help] (realised I posted in the wrong chat)
  Bit of a rookie with Blender. Does anyone know how to deal with these black patches?
  Models are ripped straight from Halo 3/ODST, shaders are copies of what's available in the asset packs but I'm trying to move everything over to a single Blender file.
  [attachments] image.png, image.png, image.png
- **2024-07-27** `elitecaden` [🦴╎animation] If you mean like use the stuff you built in Forge and import it into Blender you can do that using this Forge Editor tool but I think it only works for Halo Reach and only a small pool of maps
- **2024-07-27** `elitecaden` [🦴╎animation] https://github.com/Waffle1434/Mjolnir-Forge-Editor
  [embed] GitHub - Waffle1434/Mjolnir-Forge-Editor: A Blender tool for manipu... https://github.com/Waffle1434/Mjolnir-Forge-Editor A Blender tool for manipulating in game objects in Halo Reach Forge in The Master Chief Collection. - Waffle1434/Mjolnir-Forge-Editor
- **2024-08-04** `xenomorph_xx121` [💬╎general-help] I'm trying to import .JMA animations onto the H2A elite model in Blender (4.2) but the transform data keeps stretching out the model (as seen below). Does anyone know how to prevent the stretching?
  [attachments] image.png
- **2024-08-06** `beeboponthetop` [💬╎general-help] does anyone have halo reach FP arms that have been cleanly ported to the latest version of blender? I've been using blender 3 for years and none of the models here are compatible, but when I tried moving to blender 4, it did this to my arms.
- **2024-08-06** `r93_sniper` [Invader + SixShooter] Replacement editing kit for Halo 1 PC, Halo 1 Xbox, Halo Trial, Halo Custom Edition, and Halo 1 MCC. Invader is CLI only, SixShooter is a GUI wrapper for some of the more important functions.
  
  https://github.com/SnowyMouse/invader
  [embed] GitHub - SnowyMouse/invader: Free toolkit for Halo: Combat Evolved ... https://github.com/SnowyMouse/invader Free toolkit for Halo: Combat Evolved for creating maps and assets - SnowyMouse/invader
- **2024-08-06** `r93_sniper` [Index] Tool created by @Haus to extract content from Halo CEA and H2A Campaign.
  
  V1 (Only H2A Support): https://github.com/Wildenhaus/Index
  V2 (CEA and H2A): https://github.com/Wildenhaus/IndexV2
  [embed] GitHub - Wildenhaus/Index: A tool for browsing and extracting asset... https://github.com/Wildenhaus/Index A tool for browsing and extracting assets from Halo 2 Anniversary. - Wildenhaus/Index GitHub - Wildenhaus/IndexV2: A tool for browsing and extracting ass... https://github.com/Wildenhaus/IndexV
- **2024-08-06** `r93_sniper` [Reclaimer] Tool created by @Gravemind2401 to extract models, textures, and sounds from various halo games. Read the github link for more information on what is and isnt supported, as well as #📰╎reclaimer-updates for any pertinent news.
  
  Program: https://github.com/Gravemind2401/Reclaimer
  Addon Downloads, including AMF importer script: https://www.mediafire.com/folder/dabvr2j5ofe73/Reclaimer_Downloads
  [embed] GitHub - Gravemind2401/Reclaimer: A Halo game file explorer https://github.com/Gravemind2401/Reclaimer A Halo game file explorer. Contribute to Gravemind2401/Reclaimer development by creating an account on GitHub. MediaFire https://www.mediafire.com/folder/dabvr2j5ofe73/Reclaimer_Downloads MediaFire
- **2024-08-06** `r93_sniper` [Halo Asset Blender Development Toolset] Tool created by General_101 and various others for a full suite of tools related to modding halo. More information in github link
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/ CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2024-08-06** `r93_sniper` [Foundry] Blender toolset for Reach+ modding made by @Crisp 
  https://github.com/ILoveAGoodCrisp/Foundry
  [embed] GitHub - ILoveAGoodCrisp/Foundry: A Blender addon supporting asset ... https://github.com/ILoveAGoodCrisp/Foundry A Blender addon supporting asset creation for Halo Reach, Halo 4, and Halo 2 Anniversary Multiplayer - ILoveAGoodCrisp/Foundry
- **2024-08-06** `r93_sniper` [BMAX] Free connector app to transfer content between 3DS Max and Blender
  https://titus.gumroad.com/l/B2MAX
  [embed] Blender<->3DSMax Connector [BMAX] https://titus.gumroad.com/l/B2MAX Hello, Blender <> 3DSMAX connector(BMAX) is a two small scripts for these applications.Workflow is simple - just send and get models through FBX format.License:GNU General Public License.OS:Windows onlyInstallation:Extract "_BMAX_xx
- **2024-08-06** `r93_sniper` [Blender .max Importer] Tool to import .max files directly into blender
  https://extensions.blender.org/add-ons/io-scene-max/
  [embed] Import Autodesk MAX (.max) https://extensions.blender.org/add-ons/io-scene-max/ Import 3DSMAX meshes & materials
- **2024-08-06** `r93_sniper` [Source I/O] Blender tool to work with files from source engine directly in blender
  https://github.com/REDxEYE/SourceIO
  [embed] GitHub - REDxEYE/SourceIO: SourceIO is an Blender(3.4+) addon for i... https://github.com/REDxEYE/SourceIO SourceIO is an Blender(3.4+) addon for importing source engine textures/models/maps - REDxEYE/SourceIO
- **2024-08-06** `gameoverload` [Unreal Engine I/O] `Blender`
  PSK/PSKX/PSA Support
  https://github.com/DarklightGames/io_scene_psk_psa
  [embed] GitHub - DarklightGames/io_scene_psk_psa: A Blender extension for i... https://github.com/DarklightGames/io_scene_psk_psa A Blender extension for importing and exporting Unreal PSK and PSA files - DarklightGames/io_scene_psk_psa
- **2024-08-06** `gameoverload` [Blender DDS] `Blender`
  Multiple DXGI format support.
  Multiple texture type support.
  https://github.com/matyalatte/Blender-DDS-Addon
  [embed] GitHub - matyalatte/Blender-DDS-Addon: Blender addon to import and ... https://github.com/matyalatte/Blender-DDS-Addon Blender addon to import and export dds textures. Contribute to matyalatte/Blender-DDS-Addon development by creating an account on GitHub.
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
- **2024-08-07** `blackandfan` [UGX Pipeline (Exporter)] Io_scene_ugx is a Blender addon that can export HWDE .ugx and HWDE/HW2 .uax files. The Addon has been tested in Blender 3.1 and 4.0 but should work in more recent releases. Tool has been created by @stumpy.
  [attachments] io_scene_ugx.rar
- **2024-08-07** `reallocal` [💾╎substance-suite] hey everyone, how should i go about setting up halo infinite textures in substance painter? i'd like to port some infinite weapons to a unity game but i'm unable to use the VRChat unity shader as the game requires a specific shader be used for lighting reasons, so setting them up in SP then exporting from there with the export profile intended for that shader works out best for me. i'm just stuck on setting up textures
- **2024-08-07** `reallocal` [🔍╎texturing-and-shading] hey everyone, how should i go about setting up halo infinite textures in substance painter? i'd like to port some infinite weapons to a unity game but i'm unable to use the VRChat unity shader as the game requires a specific shader be used for lighting reasons, so setting them up in SP then exporting from there with the export profile intended for that shader works out best for me. i'm just stuck on setting up textures
- **2024-08-14** `gravemind2401` [Reclaimer] https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.0.0-beta
  Beta release of Reclaimer v2, **read the upgrade notes before installing**.
  I'm looking for feedback on any potential issues with the RMF importer for both Blender and 3DS Max users.
  [embed] Release v2.0.0-beta · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.0.0-beta Important notes on upgrading from previous versions:
  
  This version requires .NET Desktop Runtime 7 to be installed which you can get from here under the ".NET Desktop Runtime 7.0" sectio
- **2024-08-14** `alexvalkryie` [💾╎autodesk] Ok here is a shot in the dark and I do realize this is the Halo ARchives Discord, but this is more about Starfield and how to use AutoDesk 3DS Max 2021 to create collision for model. Anyone familiar with how that could work? You may ask why would you need AutoDesk for collision in Starfield? Well with the release of Creation Kit they included a set of tools, one of which was a Havok collision plugin for 3DS Max. Any help would be appreciated
- **2024-08-18** `moistsaltine` [💬╎general-help] i know nothing of blender im trying to optimise for vrc  im using the spartan pack 4.0 and im currently trying to reduce my mesh numbers when ever i join the meshes the rig no longer works or atleast properly i can get it to controle the under armor peices of the reach models but thats it it doesnt controll the helm or any of the armor is there a step im missing or is there somthing i can do to make it work
- **2024-08-21** `brooen` [Halo 5 Data Importer] This is a Blender addon that reads halo 5 tags and imports them
  https://github.com/Brooen/Halo-5-Data-Importer
  
  **Current Tag Support**
  .material
  .structure_lights
  [embed] GitHub - Brooen/Halo-5-Data-Importer: Tool that reads halo 5 .mater... https://github.com/Brooen/Halo-5-Data-Importer Tool that reads halo 5 .material files and imports them to Blender - Brooen/Halo-5-Data-Importer
  [attachments] image.png
- **2024-08-21** `alteredtech` [💾╎substance-suite] Started messing around with substance painter yesterday. I was curious what the process was called with these types of textures? This is the grenade from the halo infinite weapon armory assets. I messed around with them in blender and know that these masks allow you to adjust the base color without having an actual color texture. I am not sure what to search to learn how to replicate these.
  [attachments] olympus_grenade_frag_asg_control.png, olympus_grenade_frag_mask_0_control.png, olympus_grenade_frag_mask_1_control.png, olympus_grenade_frag_normal.png
- **2024-08-22** `brooen` [Halo 5 Data Importer] update v1.1.0, fixed some stuff
  https://github.com/Brooen/Halo-5-Data-Importer/releases/tag/v1.1.0
- **2024-08-26** `masterj2001` [🔍╎texturing-and-shading] I am insanely biased with this recommendation, but if you're looking to build your own textures from the ground up for the spartan armour, I have a 2 part Quixel Mixer tutorial that explains how to make textures for spartan/Halo armour specifically: https://www.youtube.com/watch?v=eYQpmpadpPs
  [embed] Mixer - Master Chief Helmet Texturing Tutorial | Episode 5 | Textur... https://www.youtube.com/watch?v=eYQpmpadpPs In this episode, we import our Master Chief Helmet model from Blender into Quixel Mixer and set it up with our previously baked normal, ambient occlusion, and material ID bakes.
  
  We'll 
- **2024-09-02** `gameoverload` [🔍╎texturing-and-shading] @Masterj2001 So I had a thought about making just a custom node instead of trying to make an entire custom build of Blender, especially as I could more easily add to that, and also add some features as well honestly.
  
  Either just a custom normal map node, or a custom principled BSDF that integrates the normal map stuff into it directly.
  
  It won't fix the cycles rendering stuff, but it at least makes sure anything normal map wise in the shader is dealt with, aside from the bump map correction which is a settings toggle rather than anything in shader.
- **2024-09-17** `brooen` [Halo Reach Light Importer] Blender addon that imports light files from the halo reach tags.
  https://github.com/Brooen/Halo-Reach-Light-Importer
  [embed] GitHub - Brooen/Halo-Reach-Light-Importer: A tool that reads .scena... https://github.com/Brooen/Halo-Reach-Light-Importer A tool that reads .scenario_structure_lighting_info and imports them into Blender. - Brooen/Halo-Reach-Light-Importer
- **2024-09-17** `brooen` [Halo 5 Data Importer] update v1.2.0, fixed some stuff
  https://github.com/Brooen/Halo-5-Data-Importer/releases/tag/v1.2.0
- **2024-09-19** `amlammy` [💬╎general-help] very very new to blender here. is there any way i can separate the reach marines into their own blender files? thanks
  
  https://discord.com/channels/819709630540021810/958797302229766234/990063648116445245
- **2024-09-19** `cwallen` [💾╎substance-suite] any ideas as to why substance might be exporting a blank specular for only one of my texture sets on a model? they all come out fine and then there's just the one that's black. it still has the alpha though? its definitely working fine on the model in substance. just not keeping any info on the export. wondering if it's something related to the halo 3 shader? @JackOfChaos
  [attachments] image.png, image.png, image.png, image.png, image.png, image.png
- **2024-09-24** `r93_sniper` [💾╎autodesk] https://github.com/carpikes/MayaCrasher
  [embed] GitHub - carpikes/MayaCrasher: Maya is frozen and you want to save ... https://github.com/carpikes/MayaCrasher Maya is frozen and you want to save the project? This tool is for you. - carpikes/MayaCrasher
- **2024-10-01** `frankr2994` [📜╎ripping] Briefly looking I'm surprised at the amount of information and tools available for shader disassembly . 
  I really do need to stop going down rabbit holes though. I'm supposed to be setting guns up in unreal for a game mod that I already have ported in 😅. 
  
  At some point I will be more curious of the processes used in the various model ripping programs that we have for halo. I am noticing that with h2a and h4 any material using multiple textures or having a texture coordinate map will not pull over into blender. It would be truly awesome to get something sorted out to automatically pull some of the more advanced materials in at atleast a basic layout to aid in transfer to unreal for me. My material building knowledge is low so having a reference in blender is basically a requirement.
- **2024-10-04** `scrubbanubnub` [💬╎general-help] trying to help a friend get the h3 prowler with textures into blender, but i ran into an issue trying to test and see if https://github.com/General-101/Halo-Asset-Blender-Development-Toolset is what he needs
  is there another tool for that now or am i just doing something wrong
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2024-10-09** `bongcloset` [💬╎general-help] heya, just starting out with H3 animations and am trying the device Machine elevator guide on C20. In blender, the animation looks good and the physics model, parented bone relative to the bone animates with the render. Imports over no problem to guerilla and I set up the device machine tag. Loading it in sapien has the render and physics model start in the correct positions, but running the eleveator animation leaves the physics model at the start position. Any common errors that could be the reason?
- **2024-10-19** `.crackonos` [💬╎general-help] Can anyone tell me why the bones look like this? Im using the marine models from the halo reach asset pack, but every time I import it, it looks like this. Help pls? (I hid the model of the marine to see the bones better)
  [attachments] Screenshot_1.png
- **2024-10-28** `purplmunkii` [💬╎general-help] https://www.youtube.com/watch?v=Jm39-vYl1zo
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2024-10-29** `chiefster_4014` [💬╎general-help] You wouldn't import the .model, there should be a data folder with a .jms file of the Jackrabbit in it. You can import the .jms with this addon:
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account
- **2024-10-29** `zachafinackus` [🔍╎texturing-and-shading] Hello! I'm new to using Blender and was inspiried by bdoesstuff and some other Halo artists to give it a try. I've downloaded Surasia's Spartan Pack and thanks to Crab's videos I've done quite well in getting the armor set up and recolored to my preferences. 
  
  One question I did have for anyone familiar with the model, is how do I remove/lessen the dirt and grime texture on the armor that comes default? I can see where to change it's color, but I haven't been able to figure out how to tone it down a little. I think it does look good on the upper half of the model, but it's a bit much on the legs where they're almost entirely brown due to the dirt.
  
  Thank you for your time, and feel free to ping me if you can answer!
- **2024-10-30** `chiefster_4014` [🔍╎texturing-and-shading] Halo Infinite uses procmats for armor, weapons and other things, so you'd have to bake out the color, roughness, metallic, etc. using the baking outputs of the Infinite shader in Blender to get them.
- **2024-11-01** `duhxroach1` [💬╎general-help] Does anybody know what I’m doing wrong?
  
  I’m new to all this blender and modding stuff. But what I wanted to do is I guess extract the halo reach marine render model into jms file to import into blender.
  [attachments] IMG_5362.jpg
- **2024-11-01** `chiefster_4014` [💬╎general-help] .JMS files can't be extracted from Reach, H4, or H2AMP as the import pipeline changed. If you want the models you can either use Reclaimer with compiled .map files or the Foundry addon for Blender if you're working with tags.
- **2024-11-04** `chiefster_4014` [🔍╎texturing-and-shading] Yes, the spartan armor in H4 has the base color, or diffuse in the green channel of the _control textures, presumably to save on resources.
  Instead of using Blender to get base color out, it would probably be easier to separate the green channel out of the control using something like gimp or photoshop.
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
- **2024-12-05** `chiefster_4014` [💬╎general-help] you can place a decal onto a mesh with its textures using this method:
  https://www.youtube.com/watch?v=MBy62M7TcgY
  this way it'll be affected by dirt, scratches, roughness, etc. newer infinite shaders should also have an input for the decals near the bottom of the shader nodegroup, so once you do the UV setup you can just plug it in and it'll work.
  [embed] How to Overlay an Icon within a Material (Blender Tutorial) https://www.youtube.com/watch?v=MBy62M7TcgY In this Blender tutorial I will show you how to overlay an icon or image within a material, using Blender's shader nodes.
  ● Metal Texture: https://www.cgbookcase.com/textures/old-iron-01/
  ● Electr
- **2024-12-07** `mr_striker` [💬╎general-help] So I’m gonna be THAT guy and ask probably a stupid question but I’m trying my best to learn everything as I’m finally able to get my hands on all this stuff to render.
  
  Dumb question: How do i import the halo reach Spartans zip file into blender 😭
- **2024-12-08** `thoro.blend` [💬╎general-help] https://youtu.be/6qXfCM1z4HA?si=1mulOC5tT4W-zecP
  
  wondering if there was a way to do this in the composter?
  
  so instead of rendering an object with the shader I can just apply the effect onto the image itself?
  [embed] Realistic RGB Screen Shader (Blender Tutorial) https://www.youtube.com/watch?v=6qXfCM1z4HA TV Model:
  https://www.blendswap.com/blend/22217
  
  RBG Texture:
  https://drive.google.com/file/d/1rFZHxuYm67flzAWoM_mCknYfKLyiGJPO/view?usp=drivesdk
  
  #blender #blender3d #blendermodeling #blendertutorial
- **2024-12-13** `cheezballjones` [💬╎general-help] Hey all - Fairly new here, just trying to figure out some Blender stuff, using Halo Models. I am currently just working on changing the poses and such and now I'm trying to colour the model. I've done so in Blender, using the Sculpt mode and just painting each piece, individually. Now my model is looking pretty cool but I can't figure out how to bake the single image texture of the colors that are on the model. Can someone help me by chance?
- **2024-12-16** `colt556` [💬╎general-help] Dunno if this is the right channel, but I downloaded a couple 'SPI' blender files but none of them work with any of the animations I have for the games, I'm not sure if they simply don't have a game rig or if maybe they require animations from Infinite (I tried H3, Reach, and 4 animations and they didn't work). So I guess my question is if there's a repository for Infinite animations to throw at the rigs to see if they work or not
- **2024-12-22** `paradigm_.` [💬╎general-help] If a model is imported from a video game and it uses udims, but blender stacks all the uvs on one tile when imported, making the color maps chaotic, is there a way to fix this? @ᴡɪʟᴅ ᴛᴀʟᴇꜱ  is having the issue if anyone knows a solution for him. Imports a nomad hazmat from rust and it clearly uses udims but when imported into blender all the UVs are on one tile messing up the entire texture set
- **2024-12-31** `papyrus95` [💬╎general-help] Hello, I haven't been using Blender for long, but I've been trying to learn using some of the models from this server. I have mainly been using it for renders, but recently I've been trying to use the texture painting feature that it has.
  
  I've only had a small amount of practice with it, but enough for it to work with some projects I've been working on, but I am stuck here. I am using the Halo Infinite Skirmisher that was uploaded by Palpy, and I am trying to make the blue regions on the legs match the techsuit/undersuit seen on the shoulders.
  
  The models UV seems to be unwrapped, and wasn't inside the image square, but dragging the mesh down didn't seem to implement the change I was looking for, despite the mask 0 seeming to have correct color zones.
  [attachments] image.png, image.png
- **2025-01-01** `digital33` [💬╎general-help] very new to Blender and i've been modifying an odst to a Metahuman skeleton piece by piece and was wondering if there was any way to speed up my workflow.... 
  I get object from ODST rig, clear vertex groups, add modifier data transfer vertex groups from metahuman skel to object, add modifier armature to object as metahuman skel root, drop object on metahuman skel root (re-adjust proportions) transfer weights to objects and the export.... attempting to take what I learn from this to make a modular Halo Metahuman skel... probably pointless due to the new retarget system in ue5.5.1 but i dunno...
  [attachments] image.png, image.png, image.png
- **2025-01-06** `bojackdaggah` [💬╎general-help] Okay so it seems that the Architect plugin is outdated, and after watching some of Plasterd Crab's "How to port Halo 3 models into Blender (Full in-depth process)" video, I am able to view models, but only for Halo 3 inside of the "render_model" folders. However, for Halo 2, Halo Reach, and Halo 1, I cannot find viewable models, or any types of "render_model" folders for those games. Would anyone be able to help?
- **2025-01-06** `haloismylife650` [🔍╎texturing-and-shading] Just for clarification, I do create procedural textures in Blender and hand-edit some details onto them after baking alongside reordering colour channels for compatibility with H3.
- **2025-01-09** `gameoverload` [💬╎general-help] So older model, assuming thats from Halo 3 ODST directly, so it might not have as many bones, but I don't quite remember.
  
  However that should not matter, assuming you have set up your bones correctly in the humanoid avatar configuration, ala the one where you can set the bones so that the bones match up with the regular skeleton that VRC uses internally.
  
  However, I would check the skeleton orientation in Blender, making sure its rotated in the correct axis direction, and is applied as well.
- **2025-01-12** `brooen` [Halo 4 Light Importer] Blender addon that imports light files from the halo 4 tags.
  https://github.com/Brooen/Halo-4-Light-Importer
  [embed] GitHub - Brooen/Halo-4-Light-Importer: A tool that reads .scenario_... https://github.com/Brooen/Halo-4-Light-Importer A tool that reads .scenario_structure_lighting_info and imports them into Blender. - Brooen/Halo-4-Light-Importer
- **2025-01-22** `mrfelixvrc` [💬╎general-help] I need help with this, cause the pack im using is halo 5 multiplayer pack by taco, and for some reason it shows this in unity even though in blender it shows its rigged, with both game and spartan rig.
  [attachments] Capture.PNG
- **2025-01-26** `muskah.mp4` [💬╎general-help] Can anyone help me with getting H3 Master Chief models into Blender?
  I have Blender 4.3 and the Halo MCC Tool kit from Steam. I just don't know how to get started from there. My only experience with Blender so far is making 3D spinning logos, so I'm VERY nooby.
  I'm just looking to make small animations of Spartans to eventually use in Sony Vegas but I have no idea where to begin. Any tips or up-to-date tutorials?
- **2025-02-12** `killerboy3699` [💬╎general-help] Can someone explain to me what i am doing wrong? ive managed to bake other textures out of blender (only learnt how to 3 days ago) and now im getting this issue with the mk VI helmet from the halo reach spartan pack. im not sure if my setting are to cause this or not
  [attachments] image.png, image.png, image.png, image.png
- **2025-02-17** `momosenpai` [💬╎general-help] Okay so I've decided on the armor I want to use for the spartan now what step comes next after working on the multiplayer spartan's armor?
  
  Do you export the model as an fbx to load it into a new blender file for a new scene?
  
  Trying to get an idea of the pipeline
  [attachments] image.png
- **2025-02-23** `neocranium` [💬╎general-help] Anyone have a good idea where I could get the command bridge for the Forward Unto Dawn Halo 3 ship? I was thinking H2A assets or maybe a weird cinematic render model name in H3, but I don’t know.
- **2025-02-27** `purplmunkii` [🦴╎rigging] A. You need to scale the armature not the mesh
  B. Select the armature go to pose mode
  C. Id recommend watching this: https://www.youtube.com/watch?v=Jm39-vYl1zo. Its old but still mostly relative
  [embed] How to pose your Infinite Spartan (quick overview) https://www.youtube.com/watch?v=Jm39-vYl1zo This video goes very quickly over how to pose your Spartan in the Halo Infinite Spartan pack for Blender made by Surasia (shader made by Average Goblin Enthusiast). 
  
  [new version of the Spartan pack]
  http
- **2025-02-27** `masterj2001` [🔍╎texturing-and-shading] https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/17
  https://projects.blender.org/blender/blender/issues/131605
  [embed] Cycles and surface normals (normalmap shading) https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/17 Somebody else had a very similar complaint/observation to this thread.  Various parts of Cycles will use the geometric normals (or smoothed geometric normals depending o
- **2025-02-27** `gameoverload` [🔍╎texturing-and-shading] Genuinely, this just sounds like a bad idea, because then your wasting the use of the shader normals.
  ```
  Yes, this is a known limitation. When sampling lights we include how the light relates to the position and more importantly the normals of the mesh to help decide whether or not to sample the light.
  
  The normals we use are the normals prior to any normal mapping. Which means in your cube scene, we use the "smooth normals".
  
  This causes issue for your scene as the normals on the top of the cube "curve" around to the side of the cube, causing parts of the light to be considered "behind the surface" (because they're behind the plane formed from the smooth normals) and thus won't be sampled. This then leads to the directions and amount of light being sampled changing across the "flat surface" on the top of the cube and ultimately leads to the issues you observse.
  ```
- **2025-02-28** `masterj2001` [🔍╎texturing-and-shading] I just went and contributed my findings to the thread, so hopefully that can reignite some discussion about it: https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/32
  [embed] Cycles and surface normals (normalmap shading) https://devtalk.blender.org/t/cycles-and-surface-normals-normalmap-shading/38176/32 Apologies if any of my terminology is off. I’m not a technical user, but I can see the impact of this issue on my work. I create assets using a pseudo-game modelling wor
- **2025-03-03** `iloveagoodcrisp` [💬╎general-help] This is a general question for those who make animations / cinematics in Blender or other programs. I'm building out a system for exporting cinematics from blender to Halo Reach/Halo 4 and I'm unsure of how I should approach creating "cinematic events" (these are dialogue lines, particle effects, scripted events that play in game). 
  
  Should I try to bind these to timeline markers? Or have my own entirely separately menu with these events and let the user specify the frames they occur on? Use the video editing sequencer?
  
  Been thinking lots about the pros and cons of different approaches and struggling to decide what's best
- **2025-03-03** `masterj2001` [🔍╎texturing-and-shading] @Banned .Casey Alright, so I got Gleb Alexandrov and Aidy Burrows to notice this and both Curtis Holt and Andrew Price suggested that I make a post on Right Click Select, which I have now done (yes, I have spread this thing around like wild fire, you'd never suspect that I work in marketing 😂 ) https://blender.community/c/rightclickselect/l8yj/
  
  So I think by this stage, I have done everything in my power to get this heard and seen by the relevant audiences. I'm also waiting to hear back about the MK VI GEN3 model scene being tested within this branch: https://projects.blender.org/blender/blender/pulls/135380
  [embed] Toggleable Shader Normals in Blender https://blender.community/c/rightclickselect/l8yj/ In Cycles, the handling of normal maps currently relies on a combination of mes… WIP: Replace bump correction algorithm to better respect normal maps https://projects.blender.org/blender/blender/pulls/135380 **Pr
- **2025-03-10** `un_nonimus` [📜╎ripping] Im trying to use foundry to import a halo reach map, but I am unsure if Im not following a step or something but foundry basically keeps saying "Import completed in 33 milliseconds" and theres nothing in my scene
- **2025-03-12** `chiefster_4014` [🔍╎texturing-and-shading] if it's a floating decal then you'll have to UV its texture onto the mesh, then use Mix nodes to combine it into the roughness, metallic, etc. this video should show the process fairly well
  https://www.youtube.com/watch?v=MBy62M7TcgY
  [embed] How to Overlay an Icon within a Material (Blender Tutorial) https://www.youtube.com/watch?v=MBy62M7TcgY In this Blender tutorial I will show you how to overlay an icon or image within a material, using Blender's shader nodes.
  ● Metal Texture: https://www.cgbookcase.com/textures/old-iron-01/
  ● Electr
- **2025-03-18** `walpha` [💬╎general-help] Hey, fellas. I've got a rather... interesting issue on my end. So, I made a custom coating using an Ekur export as a base. I've been trying to transfer the coating over to the Spartans in MrGruntington's pack. But... when I try to transfer the exact coating shader to the other armor, it's missing some details. I've posted some screenshots to show what I'm talking about. The first one is what it looks like on the new armor, and the second is what it's supposed to look like. It almost seems like there's some layer of detail missing, but I have no idea what could be causing it, considering I just copied the exact shader over to the other armor. Do you guys have any ideas on how to fix this?
  [attachments] image.png, image.png
- **2025-03-29** `sosa758` [💬╎general-help] Hello, does anybody knows how hallo reach handles map collision? are there collision meshes? Cant find any help online and also trough the game files, can only find vehicle and props collision models. Thanks
- **2025-03-31** `chiefster_4014` [🔍╎texturing-and-shading] Giving the decals their own UV should let you keep the resolution high for the decals. This vid explains it well, just ignore what he does for normals unless your decals have their own. https://www.youtube.com/watch?v=MBy62M7TcgY
  [embed] How to Overlay an Icon within a Material (Blender Tutorial) https://www.youtube.com/watch?v=MBy62M7TcgY In this Blender tutorial I will show you how to overlay an icon or image within a material, using Blender's shader nodes.
  ● Metal Texture: https://www.cgbookcase.com/textures/old-iron-01/
  ● Electr
- **2025-04-01** `chiefster_4014` [📜╎ripping] for CE, you can use [General's Halo Toolset](https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master) to import the .gbxmodel tags from the mod tools directly into Blender
  For H2-ODST there are commands you can use with Tool to extract .jms files to import into Blender with the same addon. H2 has separate commands for render models, physics, collision and structures, while H3 & ODST you can just use ``extract-import-info``.
  For Reach, H4, and H2AMP, you can use [Crisp's Foundry addon](https://github.com/ILoveAGoodCrisp/Foundry) to import most tags directly into Blender, similarly to CE.
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account
- **2025-04-05** `hunh0w` [💬╎general-help] I'm new to blender, anyone know a (close enough) video guide on how to make the blue solid rings? I have tried finding yt guides but they only show solid colour mesh glow guides which is nowhere similar to the halo in the img
  [attachments] image.png
- **2025-04-27** `spacedog209` [🔍╎texturing-and-shading] Can anyone explain how Trisckit got the Spartan models to be shiny like that? Also, how did he get the visors to be more of a solid gold(?) color?
  https://www.youtube.com/watch?v=dlYXkIXtgXU
  [embed] I MUST DEFEND WOMEN ONLINE https://www.youtube.com/watch?v=dlYXkIXtgXU OFF THE RACK.
  
  video title, creation process explained in my animator journals - https://youtu.be/X5mpWPiOauE
  
  models from Halo Infinite, set up for Blender by The Halo Archive community
  
  Hang Em High map from gmod - https://stea
- **2025-05-11** `x_skel07` [📜╎ripping] I imported the Halo 2 Anniversary multiplayer model AMF generated from Reclaimer, and the male bodysuits have distorted deforms.
  Is there any way to fix this, or another way to extract it?
  [attachments] image.png
- **2025-05-15** `crystals_den` [💬╎general-help] Bit of a random question; what's the easiest way to import coatings onto a Spartan model in Blender? (I do have Ekur installed and stuff, I'm just a bit clueless on how to use most of it)
  Just curious for a render I wanna do for a friend, but he can't get on Infinite rn, so he can't change his armor to the one I wanna do a render of (for direct importing the Spartan with Ekur), but I just didn't know if I had to manually figure out color zoning or not for the coating
- **2025-05-24** `mr.boooze` [💬╎general-help] heya im kind of a blender noob and i was trying to use the halo infinite elite pack for an avatar im working on, but im really stuck on trying to figure this out since the bones are in layers which ive never seen before
- **2025-06-05** `brooen` [Halo 4 Material Importer] Imports Halo 4 .material files into Blender
  
  https://github.com/Brooen/Halo-4-Material-Importer
  [embed] GitHub - Brooen/Halo-4-Material-Importer: Imports Halo 4 .material ... https://github.com/Brooen/Halo-4-Material-Importer Imports Halo 4 .material files into Blender. Contribute to Brooen/Halo-4-Material-Importer development by creating an account on GitHub.
- **2025-06-09** `the1updude` [💬╎general-help] Hello all. I'm new to 3d modelling. Is there a tutorial or README to assist in the conversion of 3D models in Blender, to tags in Halo: Reach?
  I'd like to make a custom vehicle.
  
  I don't underestimate the process, I imagine there's a lot to learn.
  
  Thank you.
- **2025-06-18** `ohwhen6442` [💬╎general-help] anyone else got a sound issue where they can't rebuild the fmod sound banks. I did it ince before and it worked, but I've added more sounds and now when I build the bank it doesn't appear anywhere. I'm using this command from the reclaimer modding site "tool export-fmod-banks "reports\reports_00\sounds_report_sizes.csv" pc sfx -bank:h3" It worked once but now doesnt build the sfx.sfb file. When you build this file shouldn't it go to the fmod folder?
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
- **2025-07-26** `x_crimsongalaxy_x` [🔍╎texturing-and-shading] if there's anyone who could point me to / knows of any tutorials / how-tos for weathering a Reach Spartan rig, pls @ me & LMK (most of the tuts I could find for tweaking Spartans in Blender was for Infinite rigs. not exactly helpful to me)
  by weathering, I mean: making the armour look scratched, scorched, dented, and generally dirty. taking the overall unpolished look of Reach's armour and amplifying it
- **2025-07-26** `chiefster_4014` [💬╎general-help] AMF doesn't work on the latest versions of Blender. it's also outdated. use the RMF plugin for Blender and export as RMF from Reclaimer instead.
  also, since you're trying to get Digsite stuff I'd suggest just grabbing it from their Github instead of the .map files: https://github.com/digsite
  And then use this addon to import them: https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master
  [embed] Digsite Crew https://github.com/digsite Digsite Crew has 6 repositories available. Follow their code on GitHub. GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master CE/H2/H3/ODST JMS/JMA/ASS exporter 
- **2025-08-01** `lomnk` [📜╎ripping] Hello i was opening Mombasa Streets (h100.map) in Reclaimer to extract into blender but some of the top parts of the map are untextured as well as most of the billboards and signs. The skybox is also like this Even though I should already have the required .map files for ODST in the same directory (shared.map, mainmenu.map, and campaign.map) (the files i got from https://haloce3.com/xmf/ if that matters). 
  After exporting it anyway and opening it blender these yellow parts are just pure white. 
  Am i missing a file or should i just somehow manually find the textures for these parts myself?
  [attachments] image.png
- **2025-08-05** `krystalize` [📜╎ripping] ok, 2 questions: 
  1. Are you the person who made a Halo 3 shader file for blender? I saw a video from Plastered Crab and he linked a shader file but the google drive link is dead, but he named someone call Chief
  2. the pre-placed obj's does importing the files into blender automatically put them into their place or is this something that is needed to be done manually?
- **2025-08-06** `marcopolo1223` [💬╎general-help] Hello all, kinda new to this but basically I am recreating my halo reach spartan with inf assets, gotten pretty far though I am trying to use the buttbag from the marine pack as a commando satchel  replacement since the base game (infinite) doesn't include it. Downloaded the file but it doesn't have any textures? Just grey. Idk if i need to find the respective texture in the folder then add it but this is where my understanding of the basics end. (using blender 4.5 which the version calls for 3.0 which may be my problem.) Any help would be much appreciated, thanks!
- **2025-08-18** `r93_sniper` [🦴╎animation] looks pretty good ngl. I'd probably only really suggest two things:
  1. See if you can implement the plasma wipe effect so that instead of it just being an on/off switch, it does the effect like Reach+ where it forms from the hilt. I know this is possible in blender as I've seen people do it
  2. Maybe have the dot particles much denser. Have more of them spawn, but keeping closer to the sword. Make it look like it's vaporizing the air around it
- **2025-08-27** `chiefster_4014` [🔍╎texturing-and-shading] the latest version of Reclaimer is 2.1.1608. you are almost 3 years behind
  https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1608
  [embed] Release v2.1.1608 · Gravemind2401/Reclaimer https://github.com/Gravemind2401/Reclaimer/releases/tag/v2.1.1608 1608 Patch
  
  Fixed some Reach workshop maps not working correctly
  Increased maximum batch worker count to 30
  Fixed H2V multiplayer maps not able to open any models
  Updated RMF importer to wor
- **2025-08-28** `memeus_boi` [🔍╎texturing-and-shading] I'm trying to export the halo 3 master chief model into a GLB file (since their easier than fbx to me) but the visor is the only thing that doesn't export, i'm tryna attach it to the master chief shading colour but it goes to the entire png for some reason
- **2025-08-29** `conspicuouscrow` [🔍╎texturing-and-shading] Hi very new to blender, im using the halo infinite spartan pack 5, is there a way to copy the color pallet from one limb and paste it on the other rather than going from one leg, copying the color, then going to the other leg and pasting the color over and over
- **2025-08-31** `djxsam23` [🔍╎texturing-and-shading] Hello! I've been playing around in Blender and in the FrostyEditor for ripping, "trying" rigging and texturing some Battlefield 4 asset and I was wondering what are the MM and D textures are for. I know that AO, CM and N are important for the 3d model but how usefull are the other 2? 
  
  Also I might need help with the texturing. At the moment I'm using the SWBF2_Materials_Version1.1 node graph in the Shader Editor in Blender to apply textures and I'd say it's okay. It does the job but I wonder if it would be better to do a custom one for Battlefield 4 instead?
  
  Thanks if someone can help me!
  [attachments] AO.png, CM.png, N.png, MM.png, D.png
- **2025-09-05** `42manako` [🔍╎texturing-and-shading] does anyone here have expierience modding ArmA 3?? I need help with something
  I found a beautiful model of a KrAZ Shrek-1 MPV from an ArmA 3 mod I downloaded, but the texture wont apply properly.
  I used PBOManager to extract the .p3d and .paa files from the pbo, and using the Blender addon ArmA Toolbox for Blender I imported the .p3d model. I converted the .paa textures to .png successfully, but when I try to apply the texture to the 3d model, it appears as a solid color only. Any ideas?
  [attachments] Screenshot_247.png
- **2025-09-14** `chiefster_4014` [🔍╎texturing-and-shading] this is just due to Halo 3's lighting and shaders, and also the possibility of the Halo 3 Blender pack not shading things correctly since it's pretty old. Though to be honest, you can only get so close in Blender/Helldivers since they use completely different shaders and lighting to Halo 3.
  You may have to just edit the colors to get an approximation.
  Here are some screenshots that show Halo 3 with albedo color only vs full lighting.
  [attachments] halo3_sapien_ELXTWog8jv.png, P3eNLwacfR.png, halo3_sapien_46P70fogcg.png, POWnrCN5h0.png
- **2025-09-14** `autarchkatherlyn` [📜╎ripping] you can import the .jms file it gives you using the [Halo Modding Toolset](https://github.com/General-101/Halo-Asset-Blender-Development-Toolset)
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2025-09-16** `aliencraftman` [💬╎general-help] im either blind, havent looked hard enough, or it really just doesnt exist, but do we have the reach armor models from mcc mythic season 8 for blender? ive been scouring around the server trying to see if its anywhere to download for blender but ive had no luck
- **2025-09-16** `midnightzer0` [💬╎general-help] An old reliable method of deleting faces in Blender no longer works correctly on `Reach Spartans.blend`, is this a problem related to the version of Blender or something else?
  
  (ALT Text Inlcuded)
  [attachments] image.png, image.png
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
- **2025-10-10** `tim_irl` [💬╎general-help] Not sure where to post this- is anyone in need of a modified version of Reclaimer that can export .RMF files with the original lightmap UV channel intact? It allows you to keep the original baked lighting from Halo 2 when you pair it with proper shader in blender or other 3d software.
- **2025-10-10** `chiefster_4014` [💬╎general-help] Also iirc General-101's Halo Blender toolset can write lightmap UVs and .bitmap tags for the Halo CE and 2 editing kits for MCC. This could be of some use for extracting the bitmaps from the editing kits.
  GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
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
- **2025-10-19** `chiefster_4014` [📜╎ripping] We have access to most if not all of the assets thanks to Digsite porting them to Halo MCC's CE mod tools. You can get them from their official Github page here: https://github.com/digsite/h1
  
  Some of the assets you may need General-101's Halo Asset Blender Development Toolset to import, which you can also get here: https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  
  As for anything else that wasn't ported by them we aren't allowed to use or upload. We also aren't able to rip from those builds.
  [embed] GitHub - digsite/h1 https://github.com/digsite/h1 Contribute to digsite/h1 development by creating an account on GitHub. GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for
- **2025-10-21** `_themadmonk` [📜╎ripping] https://youtu.be/TCib8aBLLPE?si=V5lF2nFVllFDFRG6
  
  Hey does anyone know where I could find a corner piece or a T version of the covenant hallway in this animation? Looks like reach assets
  [embed] Halo: Truth and Reconciliation https://www.youtube.com/watch?v=TCib8aBLLPE I'd like to present Halo: Truth and Reconciliation
  A passion project where Masterchief has boarded a covenant ship, just like the mission in Halo Combat Evolved!
  
  This animation is a showing of love for one of my favourite vi
- **2025-10-24** `gigawatts` [💬╎general-help] So I got a question for folks in here that do Blender renders. Specifically a question relating to the Marines people use. What style is your favorite and why. I’m kinda thinking of using the Halo 3 style Marines in the future or something similar. I know the Halo Reach marines are a favorite for many cause the Spartans they use for the same scene. So the art styles mesh well. But I’m just curious
- **2025-10-29** `chiefster_4014` [📜╎ripping] For the animations you can use this Blender addon to import them:  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2025-10-31** `chiefster_4014` [💬╎general-help] Not really. You'll need to use the shader code from the mod tools to recreate their shaders.
  If you use the Foundry addon it'll set up all the shaders and colors automatically, though you'll need to install the Halo 4 mod tools
- **2025-11-04** `chiefster_4014` [🔍╎texturing-and-shading] If you already have it as an image then you can give it it's own UV and apply it in the material. This vid explains it well, just ignore what he does for normals unless your decals have their own. https://www.youtube.com/watch?v=MBy62M7TcgY
  If you don't have it as an image then I can't really help since I'm not good at drawing.
  [embed] How to Overlay an Icon within a Material (Blender Tutorial) https://www.youtube.com/watch?v=MBy62M7TcgY In this Blender tutorial I will show you how to overlay an icon or image within a material, using Blender's shader nodes.
  ● Metal Texture: https://www.cgbookcase.com/textures/old-iron-01/
  ● Electr
- **2025-11-05** `chiefster_4014` [📜╎ripping] If you have the Reach, H4 or H2AMP mod tools installed and extracted, you can simply drag & drop a sky's .scenery tag into Blender and Foundry will automatically set up all of the shaders for you. If you're reimporting back into the tools make sure to use the ``Setup as Asset`` option when importing.
- **2025-11-06** `chiefster_4014` [💬╎general-help] If I remember correctly you can't with Tool, as the Tool command to export models didn't exist until Halo 2.
  However, you can use General-101's Halo Asset Blender Development Toolset to import the .gbxmodel directly into Blender:
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2025-11-08** `chiefster_4014` [📜╎ripping] You can extract the models either from the official Halo CE Mod Tools with [this addon](https://github.com/General-101/Halo-Asset-Blender-Development-Toolset) or extract the models from MCC or another release of CE with Reclaimer. If you rip it from Reclaimer then you have to scale up the model's rig by a value of 32.8084
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2025-11-16** `thealmightyloafer` [🔍╎texturing-and-shading] To anyone that might be able to help. I’m trying to get my spartan I’m making from blender to unity. Everything is going to plan except one thing. The baked textures don’t line up for anything except the CORE. The core is fine but the attachments helmet, and gloves are all giving me grief. The ORM, colour and normal maps bake out fine but when applied to the model in unity they just appear black on the model. I’m following OneWeirdGuy’s tutorial and he glosses over the attachment side of things for the uv maps. If anyone can help it would be greatly appreciated
- **2025-11-16** `thealmightyloafer` [💬╎general-help] To anyone that might be able to help. I’m trying to get my spartan I’m making from blender to unity. Everything is going to plan except one thing. The baked textures don’t line up for anything except the CORE. The core is fine but the attachments helmet, and gloves are all giving me grief. The ORM, colour and normal maps bake out fine but when applied to the model in unity they just appear black on the model. I’m following OneWeirdGuy’s tutorial and he glosses over the attachment side of things for the uv maps. If anyone can help it would be greatly appreciated
- **2025-11-19** `pix3lpirat3` [📜╎ripping] I’m learning this whole system as a whole, I don’t have much prior Halo modding experience.
  I’ve got reading mvar files down to a point based off a few thousand Halo 3 .mvar files (Including Compressed Xbox360, as well as reach (based off a few thousand .mvar files), reach is missing some extract logic for items and maps since people make everything in forge world.
  
  Now I’ve gotta figure out how to rebuild the map and import their objects to recreate a maps environment, this is the system I’m stuck on atm
- **2025-11-19** `masterj2001` [🔍╎texturing-and-shading] Given that this is the first time that iridescence for anodised metals has been supported in Blender, I’d assume as such, yeah. 😁
  
  I’d only known to do iridescence in the past by using a colour ramp set to HSV, two red handles, and set to the farthest distance and feeding that into a fresnel (probably other ways of doing it too). But I’ve been eagerly awaiting this specific functionality for years! 😁
- **2025-11-20** `purplmunkii` [🔍╎texturing-and-shading] In blender theres a converter chunch made for halo 5 to infinite, somewhere in the upload folder. If you want in painter youd need to replicate the masks for the infinite shader
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
- **2025-11-24** `cyber_zen` [💬╎general-help] so im a blender noob trying to make my spartan oc from the finfinte spartan pack, and spent the last bit changing the color of each peice as i go, is there a faster way then clicking on the peice scrolling to the node that controls the color i wanna change for each peice? like is there a full control node im missing? pretty new at this so hopfully i explained that well
  [attachments] image.png
- **2025-11-24** `zattlebone` [💬╎general-help] Ekur is an addon made by surasia here, im not where the help forums are located so have a look round the server, searching for ekur (might be in #📰╎announcements )
  
  Once installed, you need yhe game installed too, then the options should be there all named, if its just strings of numbers, cylix is a great website with all halo infinite parts and the ids of each piece and colour coating to override
- **2025-11-25** `lytatroan` [📜╎ripping] I can't find the Halo 3 Gravemind tentacles model, how would I go with porting it to Blender?
  I was thinking of using the Reclaimer tool, I think I used it once to rip CEA's BSP's for a small render so I think it would be a good tool for ripping H3 models too.
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
- **2025-12-06** `r93_sniper` [🦴╎animation] you could definitely do it but you could also do it much easier using Foundry for blender than the old tutorial from crab. Given, that assumes you have MCC on steam and Reach's modkit installed, but if you do then it's a significantly easier task
- **2025-12-06** `r93_sniper` [🦴╎animation] link to plugin for ref: https://github.com/ILoveAGoodCrisp/Foundry
  [embed] GitHub - ILoveAGoodCrisp/Foundry: A Blender addon supporting asset ... https://github.com/ILoveAGoodCrisp/Foundry A Blender addon supporting asset creation for Halo Reach, Halo 4, and Halo 2 Anniversary Multiplayer - ILoveAGoodCrisp/Foundry
- **2025-12-09** `tristangold` [📜╎ripping] quick ripping question - so I am trying to port some of the Halo Reach assassinations into blender via Foundry with the animation repository. First attempt to drag and drop Repo animation file into Blender failed and just want to understand exactly what I'm missing.
  
  1. For me to properly import the animation data - do I first need to extract it onto a spartan model first or can I rip the data and bones just directly into blender. 
  2. If I do need a model, are these findable in to Omaha directory or just easier to grab through the discord's already existing models. It's possible I've also been searching through the Model tools folder rather than Halo MCC itself, but navigating the files I've really just been coming across .hsc scripts.
  3. Is there a specific location I need to be dropping these animation files into on the Omaha folder? My drag and drop attempt spit back the error "Ensure your file is saved to your Omaha data directory"
  
  EDIT: poked around the discord some more so also gave a go at importing one of the animations through the .jms importer. Got an internal error noting the "action object has no attribute 'fcurves'
- **2025-12-10** `un_nonimus` [📜╎ripping] I mean specifically exporting the lightmap UV data from the bsp itself. Then also using the lightmaps for the specific bsps to essentially have accurate baked lighting for levels. 
  
  I know in reach for instance there are some lightmap bitmaps in the tags. However, when exporting the geometry using reclaimer that secondary set of uv data isn’t written for the models. So the lightmap textures are useless.
- **2025-12-11** `bojackdaggah` [🔍╎texturing-and-shading] Hello! I hope its okay to ask this here, but I'm using the Halo Archive Google Drive to import an elite onto Blender, and 'm having trouble figuring out how to add the cubemaps as well as the purple "multi" textures onto the model. I imagine its fairly simple hopefully, but I'm just not very knowledgable on material stuff. I just really wanna be able to use that cubemap shine those elites have. The way the shader is set up is just how it was automatically upon importing save for selecting the texture to use
  [attachments] image.png, image.png
- **2025-12-13** `artificialangel22` [💬╎general-help] https://bsky.app/profile/artificialangel22.bsky.social/post/3m76pla6gmc2p
  [embed] https://bsky.app/profile/artificialangel22.bsky.social/post/3m76pla6gmc2p references of Rtas, Tem, Stolt and a new slightly color tweaked version of Vul\. I am finally done with the more important characters and now I can move on with making background and episodic ones
  [#halotsoicomic](https://bsky
- **2025-12-17** `cal.co` [💬╎general-help] in the spartan pack, is there a way to import a specific coating or do we have to color the spartans by hand? if by hand how? there was a tutorial but its a lil outdated by 3 years
- **2025-12-18** `chiefster_4014` [💬╎general-help] There's a repository of animations from CE to Halo 4 here:
  https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main
  I don't know of any addons for 3DS to import these, but for Blender all of the animations can be imported with the Halo Asset Blender Development Toolset addon:
  https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main A Repository of ripped Halo Animations for use in MCC modding. - 77Mynameislol77/HaloAnimationRepository GitHub - General-101/Halo-Asset-Blender-Development-Too
- **2025-12-26** `exodst.` [🦴╎rigging] Is this elite exported from reclaimer? I believe when i was importing the halo 2 grunt from reclaimer to blender, alot of vertices were missing from the vertex groups. I have been getting the models from the google drive ever since, i believe someone else fixed them.
- **2026-01-01** `dinoman455` [💬╎general-help] hi there, um idk how to do this so i'll just explain what i'm asking for help with haha. I'm a small Halo Machinima Creator who would like to get into attempted animation of the sorts, i've never really touched blender and the only animation experiance i have is with Mine Imator and that was self taught and i'm only just getting better at it after 4 years of self teaching. Back to the halo stuff, i don't know how to import maps or custom skins for my characters or anything really, i'm kinda starting from scratch and would really appreciate it if i could maybe have a call with someone to maybe teach me the basics as my puny little brain only understands stuff thats been shown to me or explained as i'm doing said work. if anyone could help me out please dm me or reach out or whatever i'd really appreciate the help, also happy new year haha
- **2026-01-02** `chiefster_4014` [💬╎general-help] Extracting it with Reclaimer isn't good for reimporting back into the game since the geometry won't be the same as what was originally imported. You should extract it from the H3EK itself instead since you get the original geometry that way.
  I don't remember if CR4B tool can do that but it might be better to just go it manually
- **2026-01-03** `koax117` [💬╎general-help] the process itself isn't that difficult in my opinion.
  I'd say you have to be crafty/knowledgeable enough to know how to look at the text and understand their naming conventions to find things. there's search parameters too
  This is for the H3 mission the Ark, for example. can see the file on the left to give you an idea. and then just have to export them out
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
- **2026-01-10** `gameoverload` [💬╎general-help] Just figured I'd mention, although if you want Reach stuff in Blender, that is easier to change stuff on:
  https://discord.com/channels/819709630540021810/1249181000425148417
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
- **2026-01-12** `welxor` [💬╎general-help] Great idea, I wish I could do so but in the two packs I am using I don't see any base color where I could layer one
  I don't know if they are bake or not since I am a texture rookie :ChiefCry: 
  if they aren't: I have found a tool to bake them into unity but not one for blender to dumbify them so I can edit them more easily
  [attachments] Capture1280.png, Capture1281.png, Capture1282.png, Capture1283.png
- **2026-01-19** `_spoopyuwu_` [🦴╎rigging] so im using a helmet from an odst pack on the spartan pack, and its not connected to the rig, how do i do that? the rig from the odst pack also imported when i imported the helmet
- **2026-01-25** `gameoverload` [💬╎general-help] So, those are just XY normals with the Z channel filled in as 1.
  You just need to take the XY normals and recalculate them if your making a shader in say unreal or unity, or if your using Blender, assuming this is meant for a spartan, then you can just load up the spartan pack that has this normal used in it for that particular piece.
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
- **2026-02-02** `bunny8912` [💬╎general-help] Hey guys, I just wanted to pop in and ask if there is an air assault helmet for reach marine bodies, the Spartan one is (obviously) disproportionate to the body.
  
  (I'm still trying to learn blender, it's my first day)
- **2026-02-06** `trxshwrld` [💬╎general-help] can someone help im new to uploading and i got a spartan put together in blender and i need to import it into unity but when i did it added everything i hid in blender and stripped the color
  [attachments] image.png, image.png, image.png, image.png
- **2026-02-27** `mesathepostalgen` [📜╎ripping] thanks for the shading help, though i figured i could sort of... hijack the reach shader setup in blender to effect the color change, i dont do renders so i dunno how to use it, but thank you again!
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
- **2026-03-13** `korrag` [💬╎general-help] ~~I'm having some trouble getting a model setup for use in tabletop simulator.
  
  I'm using the Reach Spartan pack and the Reach UNSC Weapon pack, trying to pose my Spartan with a shotgun and export it as a file to use as a Miniature on the board. I attempted to export as an obj and found out pretty quickly that it doesn't keep the textures and I tried opening the .blend up in unity to the same effect, I tried using the Open Halo Reach Shader for Unity by Cameron "Fly" F but I couldn't get that to actually do anything when I set the normals, diffuse and control map from the Reach Spartans pack. I read somewhere that baking the textures in blender might be a solution but I'm still figuring that out.
  At this point I'm at a loss, any help would be appreciated.~~
  
  Figured it out my own damn self, if anyone needs help porting to TTS via Blender->Unity I will help.
  
  Guide incoming sometime maybe
- **2026-03-21** `koax117` [💬╎general-help] if you want to use those kneepads but not have the snowflake, the way I *personally* would go about it is taking the textures in the shader that have that image of the snowflake (some only have it in certain color channels) and manually painting it out in a program like photoshop to match the surrounding area.
  If you're new to blender and 3D, I recommend looking up some tutorials on texturing and materials.
  [attachments] image.png, image.png, image.png
- **2026-04-04** `chiefster_4014` [📜╎ripping] yes, and they have been already. you can get them from this repo: https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main
  and import them into Blender with this addon: https://github.com/General-101/Halo-Asset-Blender-Development-Toolset/tree/master
  [embed] GitHub - 77Mynameislol77/HaloAnimationRepository: A Repository of r... https://github.com/77Mynameislol77/HaloAnimationRepository/tree/main A Repository of ripped Halo Animations for use in MCC modding. - 77Mynameislol77/HaloAnimationRepository GitHub - General-101/Halo-Asset-Blender-Development-Too
- **2026-05-01** `papyrus95` [🔍╎texturing-and-shading] Hey there, I was wondering if anyone could help me figure out something regarding baking in Blender. I have retextured the Helldivers 2 MA5C using the shader converter found on @squeak_grenade (#1 SPI Fan) (Sorry for the ping) Halo Infinite Reach ODST Kitbash. I have already baked color, normal, roughness, and emission, but I was wondering how to bake a metallic map with this shader
- **2026-05-06** `koax117` [🔍╎texturing-and-shading] I heavily recommend this addon for setting up color pallets 
  https://superhivemarket.com/products/alt-tab-colorful
- **2026-05-07** `haloismylife650` [📜╎ripping] If you mean H2ASP then you could use the cursed method of exporting the bitmap using tool (e.g., `tool export-bitmap-dds "ui\hud\bitmaps\new_hud\backgrounds\plasma_pistol_bkd" "C:\test"\`) and yoink the blue channel in the image editor of your choice. If you mean H2AMP, you're going to have to use Lord Zedd's FontPackager tool to open "font_package_icon.bin" found in "maps\fonts". When in FontPackager, you can double-click the "fixedsys-ui" entry to find the weapon icons, then right-clicking on the desired schematic will allow you to export it. I believe you're going to have to repeat the same steps for the x2 and x3 versions of the font package so you can import them into their respective font packages for Reach and H4.
- **2026-06-03** `koax117` [💬╎general-help] this was utilizing Foundry's Halo Reach halogram shader. (note that its multiple materials as well, so, that is something to check for the CE version)
  was a long arduous process of locating the tags (in my instance from H3) and copying them into the Reach tags to import via foundry into blender. 
  chiefster's got an updated for H3 that i am working on for my stuff though.
  i don't know how the CE halogram shaders are handled, if any differently. the only other thing I could suggest is either 
  A- install the CE mod tools if you haven't and check the tags for that shader
  B- see if there are any other CE accurate holograms floating around that are set up right to use as a base
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
- **2026-06-05** `chiefster_4014` [💬╎general-help] The addon isn't malicious. General101 simply named one of the variables in his code to ``this_is_a_horrible_hack``, probably because he knew it would stop working at some point, and it did.
  I'm not sure how to fix this error, but you might find more help in the dedicated Halo modding discord servers. You should be able to find them in #🤝╎partner-servers  with the names Halo Mods and Halo Modding Reclaimers
- **2026-06-07** `cherrybomb172` [💬╎general-help] good evening/day to you guys! I'm brand new into making things like VRC avatars in general I understand the basics and picked up on some things from blender but I'm still heavily struggling from getting my ODST from blender to unity. I'm wondering if there's any tutorials for the 2.2 Spartan pack or if anyone could help! Any help would be appreciated and honestly I wouldn't mind paying to just get the avatar made and into unity lol.
- **2026-06-13** `roman117` [🔍╎texturing-and-shading] Okay, making some progress at least. I used this tutorial I randomly stumbled across, IDK how it didn't show up through all my previous searching cuz its very good.
  https://www.youtube.com/watch?v=n7FkOaI0vPI
  
  I still have the problem of the texture sticking out like a sore thumb, have to figure out how to make it use the existing textures underneath. But there's another very annoying problem;
  
  There seems to be white lines or artifacts or something all over my model now that get worse the farther I zoom out. They change shape if I move the UV around. I have no idea what's causing this or how to get rid of them, so I'd appreciate some help...
  [embed] The Secret to Perfect Decals in Blender https://www.youtube.com/watch?v=n7FkOaI0vPI You’ve been doing stickers the hard way. Shrink wrap modifiers, floating geometry, projection nightmares—it’s all kind of a mess. So quit that. In this tutorial, I’ll show you a smarter way to apply decals in Blender
  [attachments] image.png, image.png, image.png, image.png, image.png
- **2026-06-14** `gameoverload` [💬╎general-help] I would go to the #📂╎asset-upload-download-forum and search for ODST, and find the file you want to use.
  I am working on a blender file for all of the MCC menu armor options, but it will be a minute before that is finished.
  So for now, I would suggest using one of the existing ODST uploads that we have.
- **2026-06-15** `carpxdiem` [💬╎general-help] Hey fellas, I was watching a video showing blender basics and how to change the spartan's color in the app once you've kitted out your spartan but since the video is 4 years old the menus are a little different and im stuck here with the default color :,) any quick pointers?
  [attachments] image.png
- **2026-06-16** `.unrelenting.` [💬╎general-help] Random question and this is my first chat here, I'm trying to figure out how to basically convert the terrain/landscape from games like Reach into Height Maps to remodify in World Machine (I know the levels are released for Blender) to give some of the landscapes we had from the original game a bit more detail and updated textures. Unfortunately I'm a complete Novice in Blender.
- **2026-06-19** `asrieldream` [🦴╎animation] someone at HS must be inspired by thishttps://youtu.be/_AxMlgPFxao?is=heS5UigLQhS-RLp1
  [embed] Halo (WIP)  - Opening Shot - Teaser Clip https://www.youtube.com/watch?v=_AxMlgPFxao #halo #fananimation #blender3d #halomcc #haloceanniversary #animation 
  
  The opening scene to Episode 1 of my Halo CE adaptation.
  
  The Pillar of Autumn emerges from slip-space after a blind jump, narrowly escaping th
- **2026-07-01** `kreh` [🦴╎animation] Hi guys, brand new to blender and animation
  
  In progress of making a jump cycle for an elite and something’s telling me I’ve been using the controls incorrectly 
  
  I’ve been moving the pole controls to position the elbows, is that the correct way of doing things?
  
  Plus using IK is a little foreign to me, are there times where I’ll have to use FK? And how would I use FK on these rigs? I’m using the Halo Infinite Elite Pack 
  
  I’d appreciate the help or a point to the right direction :)
  [attachments] Recording_2026-06-30_235330.mov
- **2026-07-07** `xander_1993` [🦴╎rigging] Is there any updated rigs that I could drag and drop into my Spartan models imported via the Ekur tool or H3 Spartan Pack?  I've seen rigs out there that just look way more simple and eaiser to control.
- **2026-07-10** `alabamanwildman` [🦴╎animation] If you want to get into animation, there's plenty of excellent tutorials out there. I personally wouldn't jump straight into character animation, you're going to want to do basically the whole ass curriculum from animschool.com before you reach for the stars. As for cascadeur, while it does use ai to make things look a bit better, it's not a crutch and people will be able to tell if you used cascadeur for everything
- **2026-07-21** `koax117` [🦴╎animation] yeah, you can use this -> https://github.com/General-101/Halo-Asset-Blender-Development-Toolset
  too import those in JMM files in and either re-assign or retarget the armatures
  [embed] GitHub - General-101/Halo-Asset-Blender-Development-Toolset: CE/H2/... https://github.com/General-101/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to General-101/Halo-Asset-Blender-Development-Toolset development by creating an account on GitHub.
- **2026-07-23** `koax117` [🔍╎texturing-and-shading] not in this instance. You just imported an .obj, not the textures, nor is that a .blend with all the shaders already set up. 
  you would have to manually set it up yourself
  there is a Reach Grunt pack with them already set up.
  https://discord.com/channels/819709630540021810/1271662648655876096
- **2026-07-27** `felixvyra` [💬╎general-help] Hi.
  
  Does anyone know anything about the original Halo combat evolved campaign shader pipeline and if anyone has been able to make a decent approximation of it in Blender?
  Along with some accurate Halo Combat Evolved rips.
  
  I mean the original look of the covenant surfaces specifically for the weapons and vehicles and armors is so unique and I would really love to inspect them more closely in Blender.
- **2026-07-29** `thoro.blend` [💬╎general-help] https://www.reddit.com/r/halo/comments/elb2xv/heres_a_near_11_recreation_of_new_mombasa_from/
  
  Has anyone gotten this into blender?
  [embed] Here's a near 1:1 recreation of New Mombasa, from Halo 2 to ODST an... https://www.reddit.com/r/halo/comments/elb2xv/heres_a_near_11_recreation_of_new_mombasa_from/
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
- **2026-08-04** `._voodoo.` [💬╎general-help] Has anyone released any VRChat ready files (spartans, ODST)? I'd like to make my own but blender is not my specialty and all the files I've looked at are miles different from the ones shown in guide videos.
- **2026-08-05** `zoephie` [📜╎ripping] https://github.com/camden-smallwood/Halo-Asset-Blender-Development-Toolset
  [embed] GitHub - camden-smallwood/Halo-Asset-Blender-Development-Toolset: C... https://github.com/camden-smallwood/Halo-Asset-Blender-Development-Toolset CE/H2/H3/ODST JMS/JMA/ASS exporter for Blender. Contribute to camden-smallwood/Halo-Asset-Blender-Development-Toolset development by creating an account o
- **2026-08-10** `chiefster_4014` [💬╎general-help] I'm more familiar with porting stuff to Halo 4 MCC, but I assume with both games the first thing to do would be to adjust and reposition infinite's model to have similar proportions and the same default pose as CER/H3, then swap the weights to use the weight names from either game. H3 has way less bones in its rig so you'll have to merge some of the weights or redo them. Then you'd bake the textures of the Infinite model into whatever format they need. Both can use PBR textures.
  Them you'd port each model into their respective game. Idk about CER but H3 should be mostly straightforward, you just need an addon to export to the correct format. Them you'd import the textures into the engines, setup your materials, and then that should be it if I'm not forgetting anything.
- **2026-08-15** `winnower` [💬╎general-help] Does anyone know a way to "bake" IK into an animation? I'm exporting the reach elite animations but their feet move a lot while idle so I put the bones of the feet attached to a cube via IK and it fixes it and I would like it to be like that when I export the animations
- **2026-08-16** `hunteriskindaindifferent` [🔍╎texturing-and-shading] Hello, Ive got all the reach stuff ready to import. But how do I change the colorable parts of the helmets? They all seem to reference the same diffuse texture. Im assuming I need to link it up so a color can be applied over certain parts with an alpha but I have no idea how to do that in blender
  [attachments] ACA597B8-FCD9-4C2A-9545-AD9D002D711B.png
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
- **2026-08-27** `chiefster_4014` [🔍╎texturing-and-shading] are you trying to bake assets from the Halo Infinite spartan pack in #📂╎asset-packs? if so you should try just baking them manually, as regular baking addons tend to screw up with Infinite assets due to the complex shader used. you could also try using Ekur, but I'm not sure if it supports Blender 4.5
- **2026-09-01** `doorsday` [💬╎general-help] hello, tried picking back up this hobby a bit and need some assistance
  can anyone explain to me why in the render preview on the left, the helmet changes colors and some attachments and stuff get removed? along with the background not showing up like it does on the right?
  im using blender and the "Reach Spartans" pack i found in a yt tutorial that i can link if necessary
  any help would be greatly appreciated
  [attachments] image.png, image.png
- **2026-09-08** `gameoverload` [💬╎general-help] So, Blender currently does not give you light and render pass information as part of the shader editor, unlike say a game engine in comparison.
  So, Bungie era games that do a bunch of their own custom render functions have to be approximated in various cases.
  My hope is with Blender adding light and render pass information in 5.3 or 5.4 that we can be a little more accurate with shading approximations.
  
  Halo Infinite becomes easier only because its using modern rendering techniques which Blender has more built in to work with.
- **2026-09-13** `scalemaster117` [💬╎general-help] I installed multiplayer first as per Ekur's own instructions, and then did a re-dump of the campaign level objects afterwards. From Ekur:
  
  "Are campaign assets supported?
  Yes! However, you do need to set up its own data folder and tick the "Is Campaign" option in the addon preferences, and forge import options will become unavailable." - https://github.com/TheHaloArchive/ekur
  [embed] GitHub - TheHaloArchive/ekur: A multi-purpose blender importer for ... https://github.com/TheHaloArchive/ekur A multi-purpose blender importer for Halo Infinite. - TheHaloArchive/ekur
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
- **2026-10-12** `memeus_boi` [💬╎general-help] Hello, I was trying to export this as an individual model but I can't seem to do so, and when I was able to the model became broken and lost all colour, can someone here help me please (This is the Halo 3 spartan asset pack)
  [attachments] image.png
- **2026-10-18** `koax117` [💬╎general-help] I don't know about Halo 3, but I know for Reach their is this blender-addon that I used once, and got it to rip a map live
  https://github.com/Waffle1434/Mjolnir-Forge-Editor
  No idea if the Foundry addon has forge support or not, but works for porting the regular maps well. https://github.com/ILoveAGoodCrisp/Foundry
  [embed] GitHub - Waffle1434/Mjolnir-Forge-Editor: A Blender tool for manipu... https://github.com/Waffle1434/Mjolnir-Forge-Editor A Blender tool for manipulating in game objects in Halo Reach Forge in The Master Chief Collection. - Waffle1434/Mjolnir-Forge-Editor GitHub - ILoveAGoodCrisp/Foundry: A Blender 
- **2026-11-01** `finny.1663.` [🔍╎texturing-and-shading] Trying to follow some tutorials on how to bake textures so me and a friend can convert the https://discord.com/channels/819709630540021810/1371126749009547336/1440493886416883723 into a usable FBX for unity. we can get it to bake properly don't know how, the shader thats used in blender doesn't follow the same tutorials that we've watched and we're also new to this.
