# Spartan Armory pieces not changing: root cause and fix (2026-11-12 05:55)
(Copy of claude.ai project doc.)

Symptom: in the Armory the Spartan Equipped label changes but the model does not. Only helmets whose item name equals a model permutation (EVA, Scout, Commando, EVA [C]) changed. Body armour, knees and arms affected too.

NOT the cause (ruled out): Elite camera work, the menu map (an older map behaved the same), string ids.

Root cause: `ReachArmory.cpp` resolves Spartan items through the pmcg table (item -> region/permutation list, e.g. vinh -> head player_vinh + attachments). Read from `data\armor\*.rrpmcg`; when none is there it falls back to the copy embedded from `src\res\spartan.rrpmcg`. The pack rebuild left no `.rrpmcg` in `build\bin\Release\data\armor`, so the embedded copy was used. That file started with a UTF-8 BOM, so its first line `L 20 90` was not recognised, all 90 helmets were stored under the key ":name", `PmcgItem("20","default")` was null, and the code took the raw-name fallback (no attachments).

Fix: BOM stripped in both pmcg parse loops, BOM removed from `src\res\spartan.rrpmcg`, one-time warning if list 20 has no `default` item.

Verified (hl/ch sheets): helmets CQC, Hazop, EOD, Operator each show a different model.
