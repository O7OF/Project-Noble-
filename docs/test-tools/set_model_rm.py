import importlib, sys, traceback
LOG = r'D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11\set_model_rm_log.txt'
target = sys.argv[sys.argv.index('--') + 1] if '--' in sys.argv else ''
log = []
def L(s):
    log.append(str(s)); open(LOG, 'w').write('\n'.join(log))
try:
    mb = importlib.import_module('bl_ext.iloveagoodcrisp_github_io.io_scene_foundry.managed_blam')
    with mb.Tag(path='objects\\characters\\elite_noble\\elite_noble.model') as t:
        f = t.tag.SelectField('Reference:render model')
        L('before: ' + t.get_path_str(f.Path))
        f.Path = t._TagPath_from_string(target)
        L('after: ' + t.get_path_str(f.Path))
        t.tag_has_changes = True
except Exception:
    L('FATAL\n' + traceback.format_exc())
L('DONE')
