import sys, os, traceback, runpy
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
script = os.environ['WRAP_SCRIPT']; outname = os.environ['WRAP_OUT']
sys.path.insert(0, os.path.join(T, 'tools'))
try:
    runpy.run_path(os.path.join(T, 'tools', script), run_name='__main__')
except Exception:
    open(os.path.join(T, outname), 'w').write("ERROR\n" + traceback.format_exc())
