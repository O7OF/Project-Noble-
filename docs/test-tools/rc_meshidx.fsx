// 2026-11-13: for render_model tags matching a filter, print each region/permutation's section (mesh) index and the
// raw string id values of region and permutation names (to compare with Noble's ReadMapStringIds). Args: map filter
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Core.dll"
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Blam.dll"
#nowarn "3391"
let args = fsi.CommandLineArgs |> Array.skip 1
let cache = Reclaimer.Blam.Common.CacheFactory.ReadCacheFile(args.[0])
for t in cache.TagIndex do
    if t.ClassCode = "mode" && t.TagName.EndsWith(args.[1]) then
        printfn "== %s" t.TagName
        let rm = t.ReadMetadata<Reclaimer.Blam.HaloReach.RenderModelTag>()
        for r in rm.Regions do
            let parts =
                r.Permutations |> Seq.map (fun p -> sprintf "%s=0x%X(s%d)" (string p.Name) p.Name.Id (int p.SectionIndex))
            printfn "   %s=0x%X: %s" (string r.Name) r.Name.Id (System.String.Join(", ", parts))
