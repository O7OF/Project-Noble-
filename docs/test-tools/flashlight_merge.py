# 2026-11-13: merge PepperMan's flashlight (Nexus 2507) into OUR objects\characters\spartans\spartans.biped:
# + function  unit_open -> flashlight_new   (copy of our night_vision -> nightvision_activate function, renamed)
# + attachments flashlight_1p.light / flashlight_3p.light at marker "head", primary scale flashlight_new
#   (copies of our night_vision.light attachment, retargeted). Idempotent. Backup is made by the caller.
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def find_block(el, name, depth=0):
    for f in el.Fields:
        if str(f.FieldType) == 'Block' and str(f.FieldName) == name: return f
        if str(f.FieldType) == 'Struct' and depth < 3:
            for s in f.Elements:
                r = find_block(s, name, depth + 1)
                if r is not None: return r
    return None
def sval(e, name):
    try: return str(e.SelectField(name).Data)
    except Exception: return ''
try:
    from mbstart import B, mb_stop
    tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(r'objects\characters\spartans\spartans', 'biped'))
    funcs = find_block(tf, 'functions'); atts = find_block(tf, 'attachments')
    if any(sval(e, 'export name') == 'flashlight_new' for e in funcs.Elements):
        L('already merged')
    else:
        src = [i for i, e in enumerate(funcs.Elements) if sval(e, 'export name') == 'nightvision_activate'][0]
        funcs.DuplicateElement(src); fe = list(funcs.Elements)[-1]
        fe.SelectField('import name').Data = 'unit_open'; fe.SelectField('export name').Data = 'flashlight_new'
        L('function %d: %s -> %s' % (funcs.Elements.Count - 1, sval(fe, 'import name'), sval(fe, 'export name')))
        nv = [i for i, e in enumerate(atts.Elements)
              if 'night_vision.light' in str(e.SelectField('type').Path.RelativePathWithExtension if e.SelectField('type').Path else '')][0]
        for light in ('flashlight_1p', 'flashlight_3p'):
            atts.DuplicateElement(nv); ae = list(atts.Elements)[-1]
            ae.SelectField('type').Path = B.Tags.TagPath.FromPathAndExtension('objects\\characters\\spartans\\fx\\flashlight\\' + light, 'light')
            ae.SelectField('primary scale').Data = 'flashlight_new'
            L('attachment %d: %s at %s, scale %s' % (atts.Elements.Count - 1, ae.SelectField('type').Path.RelativePathWithExtension,
                                                     sval(ae, 'marker'), sval(ae, 'primary scale')))
        tf.Save(); L('SAVED')
    tf.Dispose(); mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc()[:1500])
open(os.path.join(T, os.environ.get('OUT', 'flashlight_merge.txt')), 'w').write('\n'.join(out))
