#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Core.dll"
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Blam.dll"
let m = fsi.CommandLineArgs.[1]
let cache = Reclaimer.Blam.Common.CacheFactory.ReadCacheFile(m)
printfn "cache type %s" (cache.GetType().FullName)
for p in cache.GetType().GetProperties() do printfn " P %s : %s" p.Name p.PropertyType.Name
for mm in cache.GetType().GetMethods() do if mm.Name.Contains("Reader") then printfn " M %s(%s)" mm.Name (System.String.Join(",", mm.GetParameters() |> Array.map (fun p -> p.ParameterType.Name)))
let t = cache.TagIndex |> Seq.find (fun t -> t.ClassCode = "mlib")
printfn "tag type %s" (t.GetType().FullName)
for p in t.GetType().GetProperties() do printfn " T %s : %s" p.Name p.PropertyType.Name
