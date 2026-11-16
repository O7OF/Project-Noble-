# Start ManagedBlam (HREK, tags only) from background Blender. Usage: from mbstart import B, mb_stop
import sys, os, glob
HREK = r"D:\Steam\steamapps\common\HREK"
try:
    import clr
except ImportError:
    for p in glob.glob(os.path.expandvars(r"%LOCALAPPDATA%\Packages\BlenderFoundation.Blender_*\LocalCache\Roaming\Blender Foundation\Blender\*\extensions\**\site-packages"), recursive=True) + \
             glob.glob(os.path.expandvars(r"%APPDATA%\Blender Foundation\Blender\*\extensions\**\site-packages"), recursive=True):
        sys.path.append(p)
    import clr
clr.AddReference(os.path.join(HREK, "bin", "managedblam"))
import Bungie as B
_cb = B.ManagedBlamCrashCallback(lambda info: None)
_sp = B.ManagedBlamStartupParameters()
_sp.InitializationLevel = B.InitializationType.TagsOnly
_sys = B.ManagedBlamSystem()
_sys.Start(HREK, _cb, _sp)
def mb_stop():
    try: _sys.Stop()
    except Exception: pass
