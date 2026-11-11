
const fs = require("fs");
const ENDIAN_LITTLE = "le", ENDIAN_BIG = "be", BIT_ORDER_UNKNOWN = 0;

class Stream {
  constructor(buf) { this.buf = buf; this.pos = 0; this.endianness = ENDIAN_LITTLE; }
  readString(n, cstring) {
    let s = "";
    for (let i = 0; i < n; i++) {
      const start = this.pos >> 3;
      const c = (this.buf[start] === undefined ? 0 : this.buf[start]);
      this.pos += 8;
      if (cstring && c === 0) break;
      s += String.fromCharCode(c);
    }
    return s;
  }
  readBitByte() { const start = this.pos >> 3; const c = this.buf[start] ?? 0; this.pos += 8; return c; }
  _int(bytes) {
    const start = this.pos >> 3;
    let v = 0n;
    for (let i = 0; i < bytes; i++) {
      const b = BigInt(this.buf[start + i] ?? 0);
      if (this.endianness === "le") v |= b << BigInt(8 * i); else v = (v << 8n) | b;
    }
    this.pos += bytes * 8;
    return Number(v & 0xFFFFFFFFFFFFn);
  }
  readByte() { return this._int(1); }
  readUInt16() { return this._int(2); }
  readUInt32() { return this._int(4); }
  readUInt64() { return this._int(8); }
  readBool() { return this._int(1) !== 0; }
  skipBytes(n) { this.pos += n * 8; }
  readBytes(n) { const out = []; for (let i = 0; i < n; i++) out.push(this.readBitByte()); return out; }
  readBits(count) {
    let v = 0n;
    for (let i = 0; i < count; i++) {
      const byte = this.buf[this.pos >> 3] ?? 0;
      const bit = (byte >> (7 - (this.pos & 7))) & 1;
      v = (v << 1n) | BigInt(bit);
      this.pos++;
    }
    return Number(v & 0xFFFFFFFFn);
  }
  readFloat() { const raw = this.readBits(32); const b = Buffer.alloc(4); b.writeUInt32BE(raw); return b.readFloatBE(0); }
  readWidecharString(chars) { let s = ""; for (let i = 0; i < chars; i++) { const c = this.readBits(16); if (!c) break; s += String.fromCharCode(c); } return s; }
  prepToReportBitnumber() {}
  reportBitnumber(label) { console.log("  @" + label + " bit " + this.pos); }
}

global._byteswap_ushort = (x) => ((x & 0xFF) << 8) | ((x >> 8) & 0xFF);
global._byteswap_ulong = (x) => ((x & 0xFF) << 24) | ((x & 0xFF00) << 8) | ((x >> 8) & 0xFF00) | ((x >>> 24) & 0xFF);
global._byteswap_uint64 = (x) => x;
global.ENDIAN_LITTLE = ENDIAN_LITTLE; global.ENDIAN_BIG = ENDIAN_BIG; global.BIT_ORDER_UNKNOWN = 0;

const { MapVariant } = require("C:/temp/hr/mvar_ref_mod.js");
const raw = fs.readFileSync(process.argv[2]);
const body = raw.subarray(0x2F0);
const s = new Stream(body);
try {
  const mv = new MapVariant(s);
  console.log("mapID", mv.header.mapID, "title", JSON.stringify(mv.header.title), "author", JSON.stringify(mv.header.createdBy && mv.header.createdBy.author));
  console.log("forgeLabels", mv.forgeLabels && mv.forgeLabels.stringCount);
  console.log("objects", mv.objects ? mv.objects.length : "n/a", "bits", s.pos);
} catch (e) {
  console.log("error:", e.message, "at bit", s.pos, "(byte", s.pos >> 3, ")");
}
