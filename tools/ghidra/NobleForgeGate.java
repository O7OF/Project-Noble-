// Labels the Forge/campaign join-queue gate in haloreach.dll so the next person can read it without re-deriving
// anything. Run against the imported program:
//
//   analyzeHeadless.bat <projdir> <projname> -process haloreach-copy.dll -noanalysis \
//       -postScript NobleForgeGate.java -scriptPath <repo>\tools\ghidra
//
// Addresses are Ghidra addresses (image base 0x180000000). See docs/FORGE-NETPLAY-OFFSETS.md for the analysis.
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.symbol.SourceType;

public class NobleForgeGate extends GhidraScript {

    private Address at(long imageAddress) {
        return toAddr(Long.toHexString(imageAddress));
    }

    private void label(long imageAddress, String name, String comment) {
        Address a = at(imageAddress);
        try {
            createLabel(a, name, true, SourceType.USER_DEFINED);
        } catch (Exception e) {
            println("label failed at " + Long.toHexString(imageAddress) + ": " + e.getMessage());
        }
        try {
            setPlateComment(a, comment);
        } catch (Exception e) {
            println("comment failed at " + Long.toHexString(imageAddress) + ": " + e.getMessage());
        }
        println("labelled " + Long.toHexString(imageAddress) + " as " + name);
    }

    @Override
    public void run() throws Exception {
        label(0x18039BEECL, "Noble_JoinQueue_SetMode",
                "Join-queue mode setter. arg = requested mode (0 idle, 1..4 active session slot).\n" +
                "Special case at 0x18039BF03: raising the mode to 4 while it is 3 is IGNORED (cmp eax,3 / je return).\n" +
                "Patch candidate if a guest's join is never queued: store 1 unconditionally at 0x18039BF0D,\n" +
                "or NOP the 4-after-3 check (same pattern as LobbyPatches' 'Join-any' gates).");

        label(0x18039A8D2L, "Noble_JoinQueue_Dispatch",
                "Session-update dispatcher for the join queue.\n" +
                "0x18039A8DA reads the mode; 0x18039A8E3 test eax,0FFFFFFFCh / jne skips the queue unless mode is 1..4.\n" +
                "Then a step machine on [+0x2A49074]: step 4 (0x18039A92D) calls the processor.");

        label(0x18039A92DL, "Noble_JoinQueue_ProcessCall",
                "The only call site of Noble_JoinQueue_Process. Reached only at step 4 of the queue state machine.");

        label(0x18039BDCCL, "Noble_JoinQueue_Process",
                "Moves a queued join into the session. Picks its session by mode:\n" +
                "  mode 1 -> [0x182A1FE08], mode 2 -> [0x182A1FE10], mode 3/4 -> [0x182A1FE18],\n" +
                "  used only when the byte at 0x182A1FD40 is set.\n" +
                "Requires that session's state field (+0x60EF8) to be exactly 6 (loaded).");

        label(0x182A49308L, "Noble_JoinQueueMode",
                "Join-queue mode: 0 = idle, 1..4 = active (which session slot the queue belongs to).\n" +
                "Read by the client: 'LobbyPatches: join queue after request: mode N count N ... state N'.");

        label(0x182A4930CL, "Noble_JoinQueueCount", "Number of joins waiting in the queue.");
        label(0x182A49318L, "Noble_JoinQueueEntries", "Queue entries, 0x588 bytes each.");
        label(0x182A49074L, "Noble_JoinQueueStep", "Queue step index; the processor runs at step 4 only.");
        label(0x182A1FD40L, "Noble_JoinQueueSlotValid", "Byte: the per-mode session pointers below are valid.");
        label(0x182A1FE08L, "Noble_JoinQueueSession1", "Session pointer used for mode 1 (uint64).");
        label(0x182A1FE10L, "Noble_JoinQueueSession2", "Session pointer used for mode 2 (uint64).");
        label(0x182A1FE18L, "Noble_JoinQueueSession34", "Session pointer used for mode 3/4 (uint64).");

        label(0x18039BEE9L, "Noble_JoinQueue_SetMode_end", "Padding after the mode setter (int3 block).");
        println("NobleForgeGate: done");
    }
}
