// 2026-11-14: names of tags by id (decimal or 0x hex, low 16 bits of a datum). Args: map id1 [id2 ...]
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Core.dll"
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Blam.dll"
let args = fsi.CommandLineArgs |> Array.skip 1
let cache = Reclaimer.Blam.Common.CacheFactory.ReadCacheFile(args.[0])
for a in args.[1..] do
    let id = if a.StartsWith("0x") then System.Convert.ToInt32(a.Substring(2), 16) else int a
    let id = id &&& 0xFFFF
    let t = cache.TagIndex.[id]
    printfn "%s -> [%s] %s" a t.ClassCode t.TagName
