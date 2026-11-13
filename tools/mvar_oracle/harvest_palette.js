const fs=require("fs");
class Bits{constructor(b,p=0){this.b=b;this.pos=p;} bit(){const v=(this.b[this.pos>>3]>>(7-(this.pos&7)))&1;this.pos++;return v;} read(n){let v=0;for(let i=0;i<n;i++)v=v*2+this.bit();return v;} wide(ch){let s="";for(let i=0;i<ch;i++){const c=this.read(16);if(!c)break;s+=String.fromCharCode(c);}return s;} fbe(){const r=this.read(32);const b=Buffer.alloc(4);b.writeUInt32BE(r);return b.readFloatBE(0);} auth(){this.read(64);this.read(64);for(let i=0;i<16;i++){if(!this.read(8))break;}this.read(1);}}
function decode(file){
  const raw=fs.readFileSync(file); const off=raw.indexOf(Buffer.from("mvar")); const b=raw.subarray(off);
  const s=new Bits(b);
  s.read(32);s.read(32);s.read(16);s.read(16);s.read(160);s.read(32);
  const ct=s.read(4)-1, fl=s.read(32); for(let i=0;i<4;i++)s.read(64);
  const act=s.read(3)-1, gm=s.read(3), eng=s.read(3); const mapId=s.read(32); s.read(8);
  s.auth();s.auth(); s.wide(128); s.wide(128);
  if(act===2)s.read(16);
  if(gm===1){s.read(8);s.read(2);s.read(2);s.read(8);s.read(32);} else if(gm===2){s.read(2);s.read(32);}
  s.read(8);s.read(32);s.read(32); s.read(9); s.read(32); s.read(1); s.read(1);
  const bounds=[]; for(let i=0;i<6;i++)bounds.push(s.fbe());
  s.read(32);s.read(32); const labels=s.read(9);
  for(let i=0;i<labels;i++){ if(!s.read(1))continue; s.read(12);} 
  if(labels>0){const dl=s.read(13),c=s.read(1); if(!c)for(let i=0;i<dl;i++)s.read(8); else {const n=s.read(13);for(let i=0;i<n;i++)s.read(8);}}
  function axisBits(bitCount,dim){const k=1/120;const me=bitCount<=16?k*(1<<(16-bitCount)):k/(1<<(bitCount-16));const mv=Math.min(Math.ceil(dim/(me*2)),0x800000);return Math.min(Math.ceil(Math.log2(mv)),26);}
  const ab=[axisBits(21,bounds[1]-bounds[0]),axisBits(21,bounds[3]-bounds[2]),axisBits(21,bounds[5]-bounds[4])];
  const out=[];
  for(let i=0;i<651;i++){
    if(!s.read(1))continue;
    s.read(2);
    let qi=-1, vi=-1;
    if(!s.read(1))qi=s.read(8);
    if(!s.read(1))vi=s.read(5);
    s.read(1); const posRaw=[s.read(ab[0]),s.read(ab[1]),s.read(ab[2])];
    if(!s.read(1))s.read(20); s.read(14); s.read(10);
    const shape=s.read(2); const n=[0,1,3,4][shape]||0; for(let k=0;k<n;k++)s.read(11);
    s.read(8);s.read(8); const ct2=s.read(5); if(!s.read(1))s.read(8); s.read(8); s.read(4); if(!s.read(1))s.read(3);
    if(ct2===1)s.read(8); else if(ct2===12||ct2===13||ct2===14){s.read(5);s.read(5);} else if(ct2===19){if(!s.read(1))s.read(8);}
    const pos=[]; for(let k=0;k<3;k++){const rng=bounds[k*2+1]-bounds[k*2];pos.push((0.5+posRaw[k])*(rng/(1<<ab[k]))+bounds[k*2]);}
    out.push({slot:i,vi,qi,pos});
  }
  return {mapId, objects: out};
}
const v=decode("C:/Program Files (x86)/Steam/steamapps/common/Halo The Master Chief Collection/haloreach/map_variants/hr_forgeWorld_asylum.mvar");
console.log("map", v.mapId, "objects", v.objects.length);
const log=fs.readFileSync("C:/Users/Chey1/Desktop/Project Noble Closed Source/build/bin/Release/noble-dev.log","latin1");
const lines=log.split("\n").filter(l=>/17:0[4-9]|17:[1-5][0-9]/.test(l) && l.includes("ForgeCapture: #"));
const caps=[];
for(const l of lines){
  const m=l.match(/ForgeCapture: #(\d+) object_new tag=(-?\d+) pos=\(([-0-9.]+),([-0-9.]+),([-0-9.]+)\)/);
  if(m) caps.push({tag:+m[2],pos:[+m[3],+m[4],+m[5]]});
}
console.log("captures this session", caps.length);
const palette=new Map(); let matched=0;
for(const c of caps){
  let best=null,bd=1e9;
  for(const o of v.objects){ const d=(o.pos[0]-c.pos[0])**2+(o.pos[1]-c.pos[1])**2+(o.pos[2]-c.pos[2])**2; if(d<bd){bd=d;best=o;} }
  if(best && bd<0.02){ matched++; const k=c.tag>>>0; const val=best.vi+","+best.qi; if(!palette.has(k)) palette.set(k,new Set()); palette.get(k).add(val); }
}
console.log("matched", matched, "distinct tags", palette.size);
const out={mapId:v.mapId, tags:{}};
let conflicts=0;
for(const [k,set] of palette){ if(set.size>1) conflicts++; out.tags[(k>>>0).toString(16)]=[...set][0]; }
console.log("conflicts", conflicts);
fs.writeFileSync("C:\\temp\\palette_209.json", JSON.stringify(out,null,1));
console.log("wrote C:\\temp\\palette_209.json");
for(const [k,set] of [...palette].slice(0,10)) console.log("0x"+(k>>>0).toString(16), [...set].join(" | "));
