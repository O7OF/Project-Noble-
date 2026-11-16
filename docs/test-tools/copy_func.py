# Copy the raw animation-function data (and input/range names) of a parameter from SRC shader to DST shaders.
# SRC=path, DSTS="a|b", PARAM=self_illum_color
import os, traceback
T = r"D:\Project-Noble-Closed-Source\build\bin\Release\data\userdata\claude-tests-2026-11-11"
out = []; L = out.append
def find(tf, pname):
    for e in tf.SelectField('Struct:render_method[0]/Block:parameters').Elements:
        if str(e.SelectField('parameter name').Data) == pname: return e
def datafield(sub):
    fn = list(sub.SelectField('Struct:function').Elements)[0]
    for f in fn.Fields:
        if str(f.FieldType) == 'Data': return f
try:
    from mbstart import B, mb_stop
    pname = os.environ.get('PARAM', 'self_illum_color')
    src = None
    if os.environ.get('HEX'):
        sdata = bytes.fromhex(os.environ['HEX'].replace(' ', ''))
    else:
        src = B.Tags.TagFile(); src.Load(B.Tags.TagPath.FromPathAndExtension(os.environ['SRC'], 'shader'))
        ssub = list(find(src, pname).SelectField('animated parameters').Elements)[0]
        sdata = bytes(datafield(ssub).GetData())
    L('src data %d bytes: %s' % (len(sdata), sdata.hex(' ')))
    for dp in os.environ['DSTS'].split('|'):
        tf = B.Tags.TagFile(); tf.Load(B.Tags.TagPath.FromPathAndExtension(dp, 'shader'))
        dsub = list(find(tf, pname).SelectField('animated parameters').Elements)[0]
        df = datafield(dsub)
        L('%s before: %s' % (dp.split('\\')[-1], bytes(df.GetData()).hex(' ')))
        df.SetData(sdata)
        L('  after : %s' % bytes(df.GetData()).hex(' '))
        tf.Save(); L('  SAVED'); tf.Dispose()
    if src: src.Dispose()
    mb_stop()
except Exception:
    L('ERROR\n' + traceback.format_exc())
open(os.path.join(T, os.environ.get('OUT', 'copy_func.txt')), 'w').write('\n'.join(out))
