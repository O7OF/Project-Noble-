import renderdoc as rd, traceback
outp=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\rdoc_probe.txt"
f=open(outp,"w",encoding="utf-8")
def p(*a):
    print(*a); f.write(" ".join(str(x) for x in a)+"\n"); f.flush()
try:
    cap=r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\rdoc2\palette_frame863.rdc"
    pyrenderdoc.LoadCapture(cap, rd.ReplayOptions(), cap, False, False)
    def sample(controller):
        roots=controller.GetRootActions(); draws=[]; stack=list(roots)
        while stack:
            a=stack.pop()
            if a.numIndices>0: draws.append(a)
            else: stack.extend(a.children)
        p("draws",len(draws))
        texs={str(t.resourceId):t for t in controller.GetTextures()}
        hits=[]
        for d in draws:
            controller.SetFrameEvent(d.eventId,True)
            pipe=controller.GetPipelineState(); ps=pipe.GetShader(rd.ShaderStage.Pixel)
            refl=pipe.GetShaderReflection(rd.ShaderStage.Pixel)
            if not refl: continue
            names=[r.name for r in refl.readOnlyResources]
            if any('palette' in n.lower() for n in names):
                hits.append((d.eventId,str(ps),names,d.numIndices))
        p("PALETTE_HITS",len(hits))
        for eid,ps,names,nidx in hits:
            controller.SetFrameEvent(eid,True); pipe=controller.GetPipelineState()
            vs=pipe.GetShader(rd.ShaderStage.Vertex); refls=pipe.GetShaderReflection(rd.ShaderStage.Vertex)
            p("HIT",eid,"PS",ps,"VS",str(vs),"nidx",nidx,"PSRES",names,"VSRES",[r.name for r in refls.readOnlyResources] if refls else None,"VSCB",[c.name for c in refls.constantBlocks] if refls else None)
            for r in pipe.GetReadOnlyResources(rd.ShaderStage.Pixel,True):
                dsc=r.descriptor; rid=str(dsc.resource); t=texs.get(rid)
                p("  PS_TEX",r.access.index,rid,(t.width,t.height,t.format.Name()) if t else None)
    pyrenderdoc.Replay().BlockInvoke(sample)
except Exception:
    p("ERROR",traceback.format_exc())
finally: f.close()
