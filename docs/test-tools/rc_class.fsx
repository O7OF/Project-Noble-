// 2026-11-13: list tags of given class codes (e.g. pmcg mulg). Args: map class1 [class2 ...]
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Core.dll"
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Blam.dll"
let args = fsi.CommandLineArgs |> Array.skip 1
let cache = Reclaimer.Blam.Common.CacheFactory.ReadCacheFile(args.[0])
let classes = args.[1..]
for t in cache.TagIndex do
    if classes |> Array.contains t.ClassCode then printfn "[%s] %s (id %d)" t.ClassCode t.TagName t.Id
