# Load a tag and save it unchanged (does ManagedBlam save work for it?). TAG=path, EXT=model
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(os.environ['TAG'], os.environ['EXT']))
    tf.Save(); L('SAVED unchanged'); tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:600])
open(os.path.join(T, os.environ.get('OUT', 'nosave.txt')), 'w').write('\n'.join(out))
