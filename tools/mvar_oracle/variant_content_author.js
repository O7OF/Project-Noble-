// The reference bundle (ReachVariantTool's html/src/raw/map_variant_mvar.js) assumes VariantContentAuthor,
// a 64-bit timestamp, a 64-bit xuid, an ASCII name of at most 16 bytes (null terminated) and a 1-bit online flag.
// It lives in a different file in the reference project, so it is reproduced here to make this oracle runnable
// from a clean checkout.
class VariantContentAuthor {
   constructor() { this.xuid = 0; this.author = ""; this.isOnlineID = false; }
   parseBits(stream) {
      stream.readUInt64();                 // timestamp
      this.xuid = stream.readUInt64();
      let s = "";
      for (let i = 0; i < 16; i++) { const c = stream.readBitByte(); if (!c) break; s += String.fromCharCode(c); }
      this.author = s;
      this.isOnlineID = stream.readBits(1) !== 0;
   }
}

module.exports = { VariantContentAuthor };
