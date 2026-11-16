// 2026-11-13: for hlmt tags matching a filter, print the render model / collision / animation references and variants.
// Args: map filter
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Core.dll"
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Blam.dll"
#nowarn "3391"
let args = fsi.CommandLineArgs |> Array.skip 1
let cache = Reclaimer.Blam.Common.CacheFactory.ReadCacheFile(args.[0])
let asm = typeof<Reclaimer.Blam.HaloReach.RenderModelTag>.Assembly
for ty in asm.GetTypes() do
    if ty.Namespace = "Reclaimer.Blam.HaloReach" && ty.Name.Contains("Model") then printfn "type %s" ty.Name
for t in cache.TagIndex do
    if t.ClassCode = "hlmt" && t.TagName.Contains(args.[1]) then
        printfn "== %s" t.TagName
        try
            let m = t.ReadMetadata<Reclaimer.Blam.HaloReach.ModelTag>()
            for p in m.GetType().GetProperties() do
                try printfn "   %s = %O" p.Name (p.GetValue(m)) with _ -> ()
        with e -> printfn "   read failed: %s" e.Message
