// List references to an absolute address (env XREF_ADDR) with the containing function.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.symbol.*;
import ghidra.program.model.listing.*;

public class Xrefs extends GhidraScript {
    @Override
    public void run() throws Exception {
        String env = System.getenv("XREF_ADDR");
        long target = Long.decode(env == null || env.isEmpty() ? "0x184e30388" : env);
        Address addr = toAddr(target);
        println("XREF target " + addr);
        Reference[] refs = getReferencesTo(addr);
        println("refs " + refs.length);
        for (Reference r : refs) {
            Function f = getFunctionContaining(r.getFromAddress());
            println("REF " + r.getFromAddress() + "  " + r.getReferenceType() + "  " +
                    (f == null ? "?" : f.getName() + "@" + f.getEntryPoint()));
        }
    }
}
