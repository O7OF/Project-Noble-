// Reclaimer scripting: list bitmap tags in an MCC Reach map whose name matches a filter. Args: map, filter
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Core.dll"
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Blam.dll"
open System
let args = fsi.CommandLineArgs |> Array.skip 1
let cache = Reclaimer.Blam.Common.CacheFactory.ReadCacheFile(args.[0])
printfn "cache %s %A" (cache.GetType().FullName) cache.CacheType
let mutable n = 0
for t in cache.TagIndex do
    if t.ClassCode = "bitm" && t.TagName.Contains(args.[1]) then
        printfn "%s" t.TagName; n <- n + 1
printfn "matches %d" n
