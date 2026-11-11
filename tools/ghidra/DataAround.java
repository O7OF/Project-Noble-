// Print pointers and strings around an address (env DATA_ADDR) to identify a class descriptor table.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;

public class DataAround extends GhidraScript {
    @Override
    public void run() throws Exception {
        String env = System.getenv("DATA_ADDR");
        long base = Long.decode(env == null || env.isEmpty() ? "0x180b95f98" : env);
        println("DATA around " + toAddr(base));
        for (int i = -4; i <= 16; i++) {
            Address a = toAddr(base + i * 8);
            long v = getLong(a);
            Address t = toAddr(v);
            Function f = getFunctionAt(t);
            String s = "";
            try {
                byte[] buf = new byte[40];
                currentProgram.getMemory().getBytes(t, buf);
                StringBuilder sb = new StringBuilder();
                for (byte b : buf) { int c = b & 0xff; sb.append(c >= 0x20 && c < 0x7f ? (char)c : '.'); }
                s = " [" + sb.toString() + "]";
            } catch (Exception e) { }
            println("  " + a + " -> " + t + (f == null ? "" : " " + f.getName()) + s);
        }
    }
}
