// 2026-11-13: list tags (class + name) whose name matches any of the filters. Args: map filter1 [filter2 ...]
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Core.dll"
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Blam.dll"
let args = fsi.CommandLineArgs |> Array.skip 1
let cache = Reclaimer.Blam.Common.CacheFactory.ReadCacheFile(args.[0])
let filters = args.[1..]
for t in cache.TagIndex do
    if filters |> Array.exists (fun f -> t.TagName.Contains(f)) then printfn "[%s] %s" t.ClassCode t.TagName
