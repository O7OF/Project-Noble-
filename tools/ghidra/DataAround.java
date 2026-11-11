// Print pointers and strings around an address (env DATA_ADDR) to identify a class descriptor.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;

public class DataAround extends GhidraScript {
    @Override
    public void run() throws Exception {
        String env = System.getenv("DATA_ADDR");
        long base = Long.decode(env == null || env.isEmpty() ? "0x180b95fb0" : env);
        println("DATA around " + toAddr(base));
        for (int i = -6; i <= 10; i++) {
            Address a = toAddr(base + i * 8);
            long v = getLong(a);
            Function f = getFunctionAt(toAddr(v));
            String s = "";
            Address t = toAddr(v);
            if (getMemoryBlock(t) != null && currentProgram.getMemory().contains(t)) {
                try {
                    byte[] buf = new byte[48];
                    currentProgram.getMemory().getBytes(t, buf);
                    StringBuilder sb = new StringBuilder();
                    for (byte b : buf) { int c = b & 0xff; sb.append(c >= 0x20 && c < 0x7f ? (char)c : '.'); }
                    s = " "" + sb + """;
                } catch (Exception e) { }
            }
            println("  " + a + " -> " + t + (f == null ? "" : " " + f.getName()) + s);
        }
    }
}
