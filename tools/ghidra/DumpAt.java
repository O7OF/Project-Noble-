// Dump the function that contains a given RVA (set DUMP_RVA, default 0x10DB1B).
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;

public class DumpAt extends GhidraScript {
    @Override
    public void run() throws Exception {
        String env = System.getenv("DUMP_RVA");
        long rva = Long.decode(env == null || env.isEmpty() ? "0x10DB1B" : env);
        Address base = currentProgram.getImageBase();
        Address addr = base.add(rva);
        Function f = getFunctionContaining(addr);
        println("IMAGE_BASE " + base);
        println("TARGET " + addr + " FUNCTION=" + (f == null ? "none" : f.getName() + " ENTRY=" + f.getEntryPoint()));
        Listing listing = currentProgram.getListing();
        Address start = addr.subtract(0x50);
        InstructionIterator it = listing.getInstructions(start, true);
        int n = 0;
        while (it.hasNext() && n < 64) {
            Instruction ins = it.next();
            println(ins.getAddress() + "  " + ins.toString());
            n++;
        }
        if (f != null) {
            DecompInterface ifc = new DecompInterface();
            ifc.openProgram(currentProgram);
            DecompileResults res = ifc.decompileFunction(f, 180, monitor);
            if (res != null && res.decompileCompleted()) {
                println("=== DECOMPILED ===");
                println(res.getDecompiledFunction().getC());
            } else {
                println("=== DECOMPILE FAILED ===");
            }
        }
    }
}
