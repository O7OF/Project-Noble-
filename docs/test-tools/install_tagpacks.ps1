# Installs Tyler's downloaded tag packs into HREK. Additive only: refuses to overwrite.
# Writes a manifest of every file it created so they can be removed again.
param([switch]$Remove)
$s='C:\Users\tyler\Desktop\so many tags'
$H='D:\Steam\steamapps\common\HREK\tags'
$manifest='D:\Steam\steamapps\common\HREK\noble_mods\tagpacks_2026-11-13_manifest.txt'
if ($Remove) {
  Get-Content $manifest | % { if (Test-Path -LiteralPath $_) { Remove-Item -LiteralPath $_ } }
  "removed"; return
}
$map=@(
 @("$s\targetting laser tags\custom","$H\custom"),
 @("$s\[HR] Boathog\tags\stannerreach","$H\stannerreach"),
 @("$s\H2A Mix with HW Marines and scorpion\H2A_marine_tags\h2a_marines","$H\objects\characters\h2a_marines"),
 @("$s\H2A Mix with HW Marines and scorpion\H2A_marine_tags\frag_grenade_h3a","$H\objects\weapons\grenade\frag_grenade_h3a"),
 @("$s\working flashlight\tags\objects\characters\spartans\fx","$H\objects\characters\spartans\fx"),
 @("$s\working flashlight\tags\objects\characters\spartans\bitmaps","$H\objects\characters\spartans\bitmaps")
)
New-Item -ItemType Directory -Force (Split-Path $manifest) | Out-Null
$created=@()
foreach($m in $map){
  Get-ChildItem -LiteralPath $m[0] -Recurse -File | % {
    $d=$m[1]+$_.FullName.Substring($m[0].Length)
    if (Test-Path -LiteralPath $d) { "SKIP (exists) $d"; return }
    New-Item -ItemType Directory -Force (Split-Path $d) | Out-Null
    Copy-Item -LiteralPath $_.FullName -Destination $d
    $created += $d
  }
}
# flashlight biped: kept aside for a field merge, never dropped over ours
$fb='D:\Steam\steamapps\common\HREK\noble_mods\flashlight_src\spartans.biped'
New-Item -ItemType Directory -Force (Split-Path $fb) | Out-Null
Copy-Item -LiteralPath "$s\working flashlight\tags\objects\characters\spartans\spartans.biped" $fb -Force
Add-Content $manifest $created
"installed $($created.Count) files"
