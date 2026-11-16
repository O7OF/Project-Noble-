#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Core.dll"
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Blam.dll"
for m in fsi.CommandLineArgs |> Array.skip 1 do
    let cache = Reclaimer.Blam.Common.CacheFactory.ReadCacheFile(m)
    for t in cache.TagIndex do
        if t.TagName.Contains("emblem") then printfn "%s  [%s] %s" (System.IO.Path.GetFileName m) t.ClassCode t.TagName
