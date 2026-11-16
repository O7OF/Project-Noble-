// 2026-11-14: every tag (class + name) of a map, one per line, sorted. Args: map outFile
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Core.dll"
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Blam.dll"
let args = fsi.CommandLineArgs |> Array.skip 1
let cache = Reclaimer.Blam.Common.CacheFactory.ReadCacheFile(args.[0])
let lines = [ for t in cache.TagIndex -> sprintf "[%s] %s" t.ClassCode t.TagName ] |> List.sort
System.IO.File.WriteAllLines(args.[1], lines)
printfn "%d tags" lines.Length
