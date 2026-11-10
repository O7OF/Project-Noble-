// Walk backwards from a vtable slot to the table start, then list references to it (env VTABLE_ADDR).
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.symbol.*;

public class Vtable extends GhidraScript {
    private boolean isCode(long v) {
        Address a = toAddr(v);
        return getMemoryBlock(a) != null && currentProgram.getMemory().contains(a) &&
               currentProgram.getMemory().getBlock(a).isExecute();
    }
    @Override
    public void run() throws Exception {
        String env = System.getenv("VTABLE_ADDR");
        long slot = Long.decode(env == null || env.isEmpty() ? "0x1809d9328" : env);
        println("VTABLE search from " + toAddr(slot));
        long start = slot;
        for (int i = 1; i < 0x80; i++) {
            long v = getLong(toAddr(slot - i * 8L));
            if (!isCode(v)) { break; }
            start = slot - i * 8L;
        }
        println("candidate table start " + toAddr(start) + " slotIndex=" + ((slot - start) / 8));
        for (int i = -2; i <= 10; i++) {
            long v = getLong(toAddr(start + i * 8L));
            Function f = getFunctionAt(toAddr(v));
            println("  [" + i + "] " + toAddr(start + i * 8L) + " -> " + toAddr(v) + " " + (f == null ? "" : f.getName()));
        }
        Reference[] refs = getReferencesTo(toAddr(start));
        println("refs to table start: " + refs.length);
        for (Reference r : refs) {
            Function f = getFunctionContaining(r.getFromAddress());
            println("  REF " + r.getFromAddress() + " " + r.getReferenceType() + " in " +
                    (f == null ? "?" : f.getName() + "@" + f.getEntryPoint()));
        }
    }
}
