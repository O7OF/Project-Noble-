// 2026-11-14 Trooper mode: per map, the tag index of the marine / female marine / player Spartan bipeds.
// Args: outCsv map1 [map2 ...]. CSV: map,marine,marine_female,spartans (tag index, -1 = not in the map)
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Core.dll"
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Blam.dll"
let args = fsi.CommandLineArgs |> Array.skip 1
let lines = System.Collections.Generic.List<string>()
lines.Add("map,marine,marine_female,spartans")
for path in args.[1..] do
    try
        let cache = Reclaimer.Blam.Common.CacheFactory.ReadCacheFile(path)
        let find (name: string) =
            cache.TagIndex |> Seq.tryFind (fun t -> t.ClassCode = "bipd" && t.TagName = name) |> Option.map (fun t -> t.Id) |> Option.defaultValue -1
        let m = System.IO.Path.GetFileNameWithoutExtension(path)
        let line = sprintf "%s,%d,%d,%d" m (find @"objects\characters\marine\marine") (find @"objects\characters\marine_female\marine_female") (find @"objects\characters\spartans\spartans")
        lines.Add(line); printfn "%s" line
    with e -> printfn "%s failed: %s" path e.Message
System.IO.File.WriteAllLines(args.[0], lines)
