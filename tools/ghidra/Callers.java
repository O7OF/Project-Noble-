// List callers of the function containing a given RVA (env CALLERS_RVA).
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.symbol.*;
import ghidra.program.model.listing.*;

public class Callers extends GhidraScript {
    @Override
    public void run() throws Exception {
        String env = System.getenv("CALLERS_RVA");
        long rva = Long.decode(env == null || env.isEmpty() ? "0x10CCD8" : env);
        Address inside = toAddr(0x180000000L + rva);
        Function target = getFunctionContaining(inside);
        Address entry = target == null ? inside : target.getEntryPoint();
        println("CALLERS of " + (target == null ? "raw " + inside : target.getName() + "@" + entry));
        Reference[] refs = getReferencesTo(entry);
        println("refs " + refs.length);
        for (Reference r : refs) {
            Function cf = getFunctionContaining(r.getFromAddress());
            println("CALL " + r.getFromAddress() + " " + r.getReferenceType() + " from " +
                    (cf == null ? "?" : cf.getName() + "@" + cf.getEntryPoint()));
        }
    }
}
