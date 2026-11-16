// For each MCC Reach map in a folder, report which of the wanted bitmap tags it contains. Args: mapsDir, name1,name2,...
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Core.dll"
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Blam.dll"
let args = fsi.CommandLineArgs |> Array.skip 1
let want = args.[1].Split(',')
let found = System.Collections.Generic.Dictionary<string, string>()
for m in System.IO.Directory.GetFiles(args.[0], "*.map") do
    try
        let cache = Reclaimer.Blam.Common.CacheFactory.ReadCacheFile(m)
        for t in cache.TagIndex do
            if t.ClassCode = "bitm" then
                for w in want do
                    if t.TagName.EndsWith(w) && not (found.ContainsKey w) then found.[w] <- System.IO.Path.GetFileName m
    with e -> printfn "skip %s: %s" (System.IO.Path.GetFileName m) e.Message
for w in want do
    printfn "%-40s %s" w (if found.ContainsKey w then found.[w] else "NOT FOUND")
