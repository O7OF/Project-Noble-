// List render_model tags whose name contains a filter, with region/permutation names. Args: map-or-dir [nameFilter]
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Core.dll"
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Blam.dll"
let args = fsi.CommandLineArgs |> Array.skip 1
let filt = if args.Length > 1 then args.[1] else "elite"
let maps = if System.IO.File.Exists(args.[0]) then [| args.[0] |] else System.IO.Directory.GetFiles(args.[0], "*.map")
for m in maps do
    try
        let cache = Reclaimer.Blam.Common.CacheFactory.ReadCacheFile(m)
        for t in cache.TagIndex do
            if t.ClassCode = "mode" && t.TagName.Contains(filt) then
                printfn "== %s : %s" (System.IO.Path.GetFileName m) t.TagName
                try
                    let rm = t.ReadMetadata<Reclaimer.Blam.HaloReach.RenderModelTag>()
                    for r in rm.Regions do
                        printfn "   %s: %s" r.Name (System.String.Join(", ", r.Permutations |> Seq.map (fun p -> p.Name)))
                with e -> printfn "   (read failed: %s)" e.Message
    with e -> printfn "skip %s: %s" (System.IO.Path.GetFileName m) e.Message
