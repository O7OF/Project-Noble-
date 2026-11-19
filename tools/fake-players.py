"""Dev-only load test for Noble's own network layer: N fake players that talk to a real Noble host.

Each fake player is a UDP socket with its own machine id. It speaks Noble's transport, not the game engine:
  - PING every second (the host answers PONG; we measure round-trip time),
  - an "appearance_extra" control message at start and every 15 s (same pattern as the real client),
  - optional chat lines (each tagged, so we can measure how many the host relayed to everyone and how fast),
  - optional voice-sized frames (off by default).
The host registers any machine that sends it packets, relays control/chat to all peers, and so does real work for
every fake player. That is the load this tool measures. It cannot test the engine's own player/membership messages.

Usage:
  python -I tools\\fake-players.py --host 127.0.0.1 --port 11774 --count 60 --seconds 120 [--chat-every 20]
"""
import argparse, json, random, select, socket, statistics, struct, time

HOST_DEFAULT_ID = 0x484F535400000001


def make_id(rng):
    while True:
        v = (rng.getrandbits(48) << 8) | 0x01
        if v != HOST_DEFAULT_ID:
            return v


class FakePlayer:
    def __init__(self, index, args, rng):
        self.index = index
        self.mid = make_id(rng)
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(("0.0.0.0", 0))
        self.sock.setblocking(False)
        self.dest = (args.host, args.port)
        self.flame = rng.randint(-1, 29)
        self.next_ping = time.time() + rng.random()
        self.next_appearance = time.time() + rng.random() * 2
        self.next_chat = time.time() + (rng.random() * args.chat_every if args.chat_every > 0 else 1e18)
        self.next_voice = time.time() + rng.random() * 0.02
        self.ping_sent = {}
        self.rtts = []
        self.recv_packets = 0
        self.recv_bytes = 0
        self.recv_by_tag = {}
        self.chat_seq = 0
        self.sent_packets = 0
        self.sent_bytes = 0

    def send(self, tag, payload=b""):
        pkt = struct.pack("<Q", self.mid) + tag + payload
        try:
            self.sock.sendto(pkt, self.dest)
            self.sent_packets += 1
            self.sent_bytes += len(pkt)
        except OSError:
            pass

    def send_json(self, obj):
        self.send(b"NBCJ", json.dumps(obj, separators=(",", ":")).encode())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=11774)
    ap.add_argument("--count", type=int, default=60)
    ap.add_argument("--seconds", type=int, default=120)
    ap.add_argument("--chat-every", type=float, default=0, help="seconds between chat lines per fake player (0 = off)")
    ap.add_argument("--voice", type=int, default=0, help="how many fake players send voice-sized frames (50/s)")
    ap.add_argument("--seed", type=int, default=1)
    args = ap.parse_args()

    rng = random.Random(args.seed)
    players = [FakePlayer(i, args, rng) for i in range(args.count)]
    by_sock = {p.sock: p for p in players}
    chats_sent = {}      # tag -> send time
    chat_deliveries = {} # tag -> list of latencies (one per receiving fake player)
    start = time.time()
    end = start + args.seconds
    print(f"{args.count} fake players -> {args.host}:{args.port} for {args.seconds}s", flush=True)

    while time.time() < end:
        now = time.time()
        for p in players:
            if now >= p.next_ping:
                p.next_ping = now + 1.0
                p.ping_sent[round(now, 3)] = now
                p.send(b"PING")
            if now >= p.next_appearance:
                p.next_appearance = now + 15.0
                p.send_json({"type": "appearance_extra",
                             "players": [{"xuid": "%016X" % (0x0009000000000000 | (p.mid & 0xFFFFFFFFFFFF)),
                                          "flame": p.flame}]})
            if now >= p.next_chat:
                p.next_chat = now + args.chat_every
                tag = f"fp{p.index}-{p.chat_seq}"
                p.chat_seq += 1
                chats_sent[tag] = now
                p.send_json({"type": "chat", "channel": "all", "text": tag, "from": f"fake{p.index}"})
            if p.index < args.voice and now >= p.next_voice:
                p.next_voice = now + 0.02
                p.send(b"NVOX", b"NVOX" + bytes(rng.getrandbits(8) for _ in range(60)))

        readable, _, _ = select.select(list(by_sock), [], [], 0.005)
        for s in readable:
            p = by_sock[s]
            while True:
                try:
                    data, _ = s.recvfrom(65536)
                except (BlockingIOError, OSError):
                    break
                t = time.time()
                p.recv_packets += 1
                p.recv_bytes += len(data)
                tag = data[8:12] if len(data) >= 12 else b"????"
                key = tag.decode("latin-1") if tag in (b"PING", b"PONG", b"NBCJ", b"NVOX") else "engine/other"
                p.recv_by_tag[key] = p.recv_by_tag.get(key, 0) + 1
                if tag == b"PONG" and p.ping_sent:
                    # The host echoes PONG three times; use the oldest outstanding ping once.
                    k = min(p.ping_sent)
                    p.rtts.append((t - p.ping_sent.pop(k)) * 1000.0)
                    for old in [x for x in p.ping_sent if t - p.ping_sent[x] > 3.0]:
                        p.ping_sent.pop(old)
                elif tag == b"PING":
                    p.send(b"PONG")
                elif tag == b"NBCJ":
                    try:
                        j = json.loads(data[12:].decode("utf-8", "replace"))
                    except ValueError:
                        continue
                    if j.get("type") == "chat":
                        ctag = j.get("text", "")
                        if ctag in chats_sent and j.get("from") != f"fake{p.index}":
                            chat_deliveries.setdefault(ctag, []).append((t - chats_sent[ctag]) * 1000.0)

    for p in players:
        p.send(b"DISC")   # leave cleanly so the host drops them from its peer table
    elapsed = time.time() - start
    rtts = [r for p in players for r in p.rtts]
    answered = sum(1 for p in players if p.rtts)
    print(f"\nRESULT  fake players answered by host: {answered}/{len(players)}")
    if rtts:
        rtts.sort()
        p95 = rtts[int(len(rtts) * 0.95) - 1]
        print(f"RESULT  ping round trip: median {statistics.median(rtts):.1f} ms, 95% {p95:.1f} ms, max {rtts[-1]:.1f} ms"
              f" ({len(rtts)} samples)")
    sent = sum(p.sent_packets for p in players)
    recv = sum(p.recv_packets for p in players)
    rbytes = sum(p.recv_bytes for p in players)
    print(f"RESULT  sent {sent} packets ({sent/elapsed:.0f}/s); received {recv} packets ({recv/elapsed:.0f}/s, "
          f"{rbytes*8/elapsed/1000:.0f} kbit/s total, {rbytes*8/elapsed/1000/max(1,len(players)):.1f} kbit/s per player)")
    tags = {}
    for p in players:
        for k, v in p.recv_by_tag.items():
            tags[k] = tags.get(k, 0) + v
    print("RESULT  received by kind: " + ", ".join(f"{k} {v}" for k, v in sorted(tags.items())))
    if chats_sent:
        expected = len(chats_sent) * (len(players) - 1)
        got = sum(len(v) for v in chat_deliveries.values())
        lat = sorted(l for v in chat_deliveries.values() for l in v)
        line = f"RESULT  chat lines relayed to other fake players: {got}/{expected}"
        if lat:
            line += f" (median {statistics.median(lat):.1f} ms, 95% {lat[int(len(lat)*0.95)-1]:.1f} ms)"
        print(line)


if __name__ == "__main__":
    main()
