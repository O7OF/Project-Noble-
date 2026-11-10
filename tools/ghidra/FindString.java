// Find strings containing a substring (env FIND_STRING) and print their references + containing functions.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.symbol.*;

public class FindString extends GhidraScript {
    @Override
    public void run() throws Exception {
        String needle = System.getenv("FIND_STRING");
        if (needle == null || needle.isEmpty()) needle = "game incompatible";
        println("FIND_STRING '" + needle + "'");
        Memory mem = currentProgram.getMemory();
        byte[] pattern = needle.getBytes("US-ASCII");
        Address found = mem.findBytes(mem.getMinAddress(), pattern, null, true, monitor);
        int hits = 0;
        while (found != null && hits < 8) {
            hits++;
            println("STR at " + found);
            try {
                if (getDataAt(found) == null) createData(found, new TerminatedStringDataType());
            } catch (Exception e) {
                println("  (could not define string: " + e.getMessage() + ")");
            }
            Reference[] refs = getReferencesTo(found);
            println("  refs " + refs.length);
            for (Reference r : refs) {
                Function f = getFunctionContaining(r.getFromAddress());
                println("  REF " + r.getFromAddress() + " " + r.getReferenceType() + " in " +
                        (f == null ? "?" : f.getName() + "@" + f.getEntryPoint()));
            }
            found = mem.findBytes(found.add(1), pattern, null, true, monitor);
        }
        println("hits " + hits);
    }
}
