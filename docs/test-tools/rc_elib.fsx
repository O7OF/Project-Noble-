// Dump the first bytes of the emblem_library tag ('elib') meta in Reach maps. Args: map [map...]
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Core.dll"
#r @"D:\RECLAIMER\Reclaimer-2.1.1645\CommApp\Gravemind2401\Reclaimer\Reclaimer.Blam.dll"
let args = fsi.CommandLineArgs |> Array.skip 1
for m in args do
    try
        let cache = Reclaimer.Blam.Common.CacheFactory.ReadCacheFile(m)
        for t in cache.TagIndex do
            if t.ClassCode = "mlib" then
                printfn "== %s : %s" (System.IO.Path.GetFileName m) t.TagName
                let ct = cache.GetType()
                let tr = ct.GetProperty("MetadataTranslator").GetValue(cache)
                let mi = ct.GetMethods() |> Array.find (fun mm -> mm.Name = "CreateReader" && mm.GetParameters().Length = 1)
                let r = mi.Invoke(cache, [| tr |]) :?> System.IO.BinaryReader
                let mp = t.GetType().GetProperty("MetaPointer").GetValue(t)
                let addr = mp.GetType().GetProperty("Address").GetValue(mp) |> System.Convert.ToInt64
                r.BaseStream.Position <- addr
                let b = r.ReadBytes(0x90)
                for i in 0 .. 4 .. 0x8C do
                    let v = System.BitConverter.ToInt32(b, i)
                    printf "%02X:%-11d " i v
                    if (i / 4) % 6 = 5 then printfn ""
                printfn ""
    with e -> printfn "skip %s: %s" (System.IO.Path.GetFileName m) e.Message

