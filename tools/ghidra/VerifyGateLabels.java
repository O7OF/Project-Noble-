import ghidra.app.script.GhidraScript;
public class VerifyGateLabels extends GhidraScript {
    public void run() throws Exception {
        long[] addrs = {0x18039BEECL, 0x18039A8D2L, 0x18039A92DL, 0x18039BDCCL,
                        0x182A49308L, 0x182A4930CL, 0x182A49318L, 0x182A49074L,
                        0x182A1FD40L, 0x182A1FE08L};
        for (long a : addrs) {
            var addr = toAddr(Long.toHexString(a));
            var sym = getSymbolAt(addr);
            String name = sym == null ? "<none>" : sym.getName();
            String comment = getPlateComment(addr);
            println("VERIFY " + Long.toHexString(a) + " name=" + name + " comment=" + (comment == null ? "no" : "yes"));
        }
    }
}
