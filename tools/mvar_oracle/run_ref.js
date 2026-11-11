// Runs ReachVariantTool's reference map-variant parser (./mvar_ref.js, unmodified except for a trailing
// module.exports) against real MCC .mvar files, through a minimal stream shim.
//
//   node tools/mvar_oracle/run_ref.js <file.mvar> [more files...]
//   node tools/mvar_oracle/run_ref.js --scan <dir>
//   node tools/mvar_oracle/run_ref.js --trace <file.mvar>
//
// The trace prints where every region ends in bits, so a port can be checked region by region instead of by
// eyeballing decoded values.
"use strict";
const fs = require("fs");
const path = require("path");

const ENDIAN_LITTLE = "le", ENDIAN_BIG = "be", BIT_ORDER_UNKNOWN = 0;

class Stream {
   constructor(buf) { this.buf = buf; this.pos = 0; this.endianness = ENDIAN_LITTLE; }
   get length() { return this.buf.length * 8; }
   readString(n, cstring) {
      let s = "";
      for (let i = 0; i < n; i++) {
         const c = this.buf[this.pos >> 3] ?? 0;
         this.pos += 8;
         if (cstring && c === 0) break;
         s += String.fromCharCode(c);
      }
      return s;
   }
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
   // The variant is one continuous bit stream: a "byte" read is eight bit reads, whether or not the cursor
   // happens to be byte-aligned. Reading whole bytes from buf[pos >> 3] silently discards the bit offset and
   // shifts everything after the first unaligned field (the author's name starts at bit 757 in atom.mvar).
   readBitByte() { return this.readBits(8); }
   readByte() { return this.readBits(8); }
   _int(bytes) {
      let v = 0n;
      for (let i = 0; i < bytes; i++) {
         const b = BigInt(this.readBits(8));
         if (this.endianness === ENDIAN_LITTLE) v |= b << BigInt(8 * i); else v = (v << 8n) | b;
      }
      return Number(v & 0xFFFFFFFFFFFFn);
   }
   readUInt16() { return this._int(2); }
   readUInt32() { return this._int(4); }
   readUInt64() { return this._int(8); }
   readBool() { return this.readBits(1) !== 0; }
   skipBytes(n) { for (let i = 0; i < n; i++) this.readBits(8); }
   readBytes(n) { const out = []; for (let i = 0; i < n; i++) out.push(this.readBits(8)); return out; }
   readFloat() { const raw = this.readBits(32); const b = Buffer.alloc(4); b.writeUInt32BE(raw); return b.readFloatBE(0); }
   // The reference switches to ENDIAN_BIG before the title and reads 16-bit units; its bit reader is
   // big-endian, so a straight 16-bit MSB-first read is the same thing.
   readWidecharString(chars) { let s = ""; for (let i = 0; i < chars; i++) { const c = this.readBits(16); if (!c) break; s += String.fromCharCode(c); } return s; }
   prepToReportBitnumber() {}
   reportBitnumber(label) { if (process.env.MVAR_TRACE) console.log("  @" + label + " bit " + this.pos); }
}

global._byteswap_ushort = (x) => ((x & 0xFF) << 8) | ((x >> 8) & 0xFF);
global._byteswap_ulong = (x) => ((x & 0xFF) << 24) | ((x & 0xFF00) << 8) | ((x >> 8) & 0xFF00) | ((x >>> 24) & 0xFF);
global._byteswap_uint64 = (x) => x;
global.ENDIAN_LITTLE = ENDIAN_LITTLE; global.ENDIAN_BIG = ENDIAN_BIG; global.BIT_ORDER_UNKNOWN = BIT_ORDER_UNKNOWN;
global.VariantContentAuthor = require("./variant_content_author.js").VariantContentAuthor;

const { MapVariant } = require("./mvar_ref.js");
const CHUNK_OFFSET = 0x2F0;

function parseFile(file, trace) {
   const raw = fs.readFileSync(file);
   const body = raw.subarray(CHUNK_OFFSET);
   const stream = new Stream(body);
   // Instrument the object loop without touching the reference: LoadedForgeObject's calls are counted here.
   const before = [];
   const objectsStartProbe = () => stream.pos;
   const mv = new MapVariant(stream);
   const bits = stream.pos;
   const chunkBits = body.length * 8;
   const tailSlots = 0x100;
   const tailBits = Math.min(mv.unk2B2 ?? 0, tailSlots) * 24;
   return { raw, mv, stream, bits, chunkBits, tailBits, objectsStartProbe };
}

const args = process.argv.slice(2);
if (args[0] === "--scan") {
   const dir = args[1];
   const files = fs.readdirSync(dir).filter(f => f.toLowerCase().endsWith(".mvar")).sort();
   let ok = 0, bad = 0;
   for (const f of files) {
      const file = path.join(dir, f);
      try {
         const { mv, bits, chunkBits, tailBits } = parseFile(file);
         const used = bits + tailBits;
         const pad = chunkBits - used;
         console.log(`${f}\tbits_after_objects=${bits}\tunk2B2=${mv.unk2B2}\ttail=${tailBits}\tchunk=${chunkBits}\tpad=${pad}`);
         ok++;
      } catch (e) {
         console.log(`${f}\tERROR ${e.message}`);
         bad++;
      }
   }
   console.log(`-- ${ok} parsed, ${bad} failed`);
} else {
   const trace = args[0] === "--trace";
   for (const file of trace ? args.slice(1) : args) {
      const { mv, bits, chunkBits, tailBits } = parseFile(file, trace);
      console.log(`${path.basename(file)}: mapID=${mv.header.mapID} title=${JSON.stringify(mv.header.title)} author=${JSON.stringify(mv.header.createdBy.author)} unk02B0=0x${(mv.unk02B0 ?? -1).toString(16)} unk02B2=${mv.unk02B2} labels=${mv.forgeLabels.stringCount} objectsEndBit=${bits} chunkBits=229696 pad=${229696 - bits} objects=${mv.forgeObjects.length}`);
   }
}