"""The uplink session on synthetic files: a file sent forward from the server's cursor rebuilds to
the card's bytes; a packet forged, dropped, reordered or replayed fails its tag; a window carries
the segments that touch it with the configs they need; every message parses back."""
import os, random, sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, os.path.dirname(__file__))
import hmc, session
from session import Chain, Server, NONE_ENTRY
import test_hmc

KEY = bytes(range(16))
STATION = 0x123456


def send(image, held, nonce, hello_tag):
    head = Chain(KEY, nonce, hello_tag)
    header, _ = session.file_objects(image)
    hb = bytes(image[:header.length])
    out = []
    for k, objs in enumerate(session.forward(image, held)):
        out.append(head.archive(header.address, header.sequence, hb if k == 0 and held == NONE_ENTRY else b"", objs))
    return out


def main():
    # RFC 4231, test case 2
    import hmac, hashlib
    assert hmac.new(b"Jefe", b"what do ya want for nothing?", hashlib.sha256).hexdigest().startswith("5bdcc146bf60754e")
    rng = random.Random(1)
    for name, header, ops in test_hmc.scenarios():
        w, _ = test_hmc.write(header, ops)
        image = bytes(w.image)
        nonce = bytes(rng.randrange(256) for _ in range(8))
        msg, htag = session.hello(KEY, STATION, 41, nonce, True)
        t, body = session.unframe(msg[:-1])
        hl = session.parse_hello(KEY, body)
        assert t == session.HELLO and hl["counter"] == 41 and hl["nonce"] == nonce
        # the whole file forward from no cursor, then a resume from the middle
        packets = send(image, NONE_ENTRY, nonce, htag)
        srv, chain = Server(), Chain(KEY, nonce, hl["tag"])
        for p in packets:
            t, body = session.unframe(p[:-1])
            srv.take(chain.verify(body))
        assert bytes(srv.image) == image, name
        mid = srv.held // 2
        nonce2 = bytes(rng.randrange(256) for _ in range(8))
        msg2, htag2 = session.hello(KEY, STATION, 42, nonce2, False)
        cur = session.parse_marks(session.unframe(session.marks(session.CURSOR, [(header.address, header.sequence, mid)])[:-1])[1])
        assert cur == [(header.address, header.sequence, mid)]
        part = Server()
        part.image = bytearray(image)
        for i in range(mid + 1, srv.held + 1):          # what the server lost after the cursor
            at = len(image) - 16 * (i + 1)
            part.image[at:at + 16] = bytes(16)
        resumed = send(image, mid, nonce2, htag2)
        chain2 = Chain(KEY, nonce2, htag2)
        for p in resumed:
            part.take(chain2.verify(session.unframe(p[:-1])[1]))
        assert bytes(part.image) == image, name
        # a packet forged, dropped, reordered or replayed into another session fails
        if len(packets) >= 2:
            for bad in (lambda ps: [ps[0][:20] + bytes([ps[0][20] ^ 1]) + ps[0][21:]] + ps[1:],
                        lambda ps: ps[1:], lambda ps: [ps[1], ps[0]] + ps[2:]):
                c = Chain(KEY, nonce, htag)
                try:
                    for p in bad(packets):
                        c.verify(session.unframe(p[:-1])[1])
                    raise AssertionError(f"{name}: a bad packet passed")
                except ValueError:
                    pass
            c = Chain(KEY, nonce2, htag2)
            try:
                c.verify(session.unframe(packets[0][:-1])[1])
                raise AssertionError(f"{name}: a replayed packet passed")
            except ValueError:
                pass
        # a window: the segments that touch it, with the configs before them
        r = hmc.Reader(image)
        segs = [(i, o) for i, (t, o, _) in enumerate(r.objects()) if t == hmc.SEG]
        mid_sec = segs[len(segs) // 2][1]["first"] + 3
        cut = session.window_cut(image, mid_sec, 5)
        got = [i for i, _, _ in cut]
        want = [i for i, o in segs if o["first"] <= mid_sec + 5 and o["last"] >= mid_sec - 5]
        assert [i for i in got if r.entries[i][3] == hmc.SEG] == want, name
        assert all(r.entries[i][3] == hmc.SEG or i < want[-1] for i in got)
        # live, window, board and the stream
        lv = session.live(test_hmc.T0, [(0x52, hmc.segment_bytes(ops[0][1][:50], hmc.hcc_layout(header.sections)))
                                        if ops[0][0] == "seg" else (0, hmc.event_segment(ops[0][1][:5]))])
        sec, segs_ = session.parse_live(session.unframe(lv[:-1])[1])
        assert sec == test_hmc.T0 and len(segs_) == 1
        wm = session.window(KEY, STATION, 43, 0xFF, mid_sec, 5)
        wd = session.parse_window(KEY, STATION, 42, session.unframe(wm[:-1])[1])
        assert wd == {"counter": 43, "address": 0xFF, "second": mid_sec, "window": 5}
        for stored, key in ((43, KEY), (42, bytes(16))):
            try:
                session.parse_window(key, STATION, stored, session.unframe(wm[:-1])[1])
                raise AssertionError("a stale or forged WINDOW passed")
            except ValueError:
                pass
        # a command, a read under counter 0, and the re-key wrapped under K0
        cm = session.command(KEY, STATION, 44, '{"cmd":"recommission"}')
        cd = session.parse_command(KEY, STATION, 43, session.unframe(cm[:-1])[1])
        assert cd == {"counter": 44, "command": '{"cmd":"recommission"}'}
        rf = session.command(KEY, STATION, 0, '{"cmd":"refresh"}')
        assert session.parse_command(bytes(16), STATION, 44, session.unframe(rf[:-1])[1])["counter"] == 0
        new_k1 = bytes(range(0xA1, 0xA9))
        sk = session.set_k1(KEY[:8], STATION, 45, new_k1)
        assert new_k1 not in sk
        sd = session.parse_set_k1(KEY[:8], STATION, 44, session.unframe(sk[:-1])[1])
        assert sd == {"counter": 45, "k1": new_k1}
        for stored, k0 in ((45, KEY[:8]), (44, bytes(8))):
            try:
                session.parse_set_k1(k0, STATION, stored, session.unframe(sk[:-1])[1])
                raise AssertionError("a stale or forged SET_K1 passed")
            except ValueError:
                pass
        bd = session.board(0x654321, session.unframe(lv[:-1])[1])
        t, body = session.unframe(bd[:-1])
        assert t == session.BOARD and session.parse_live(body[3:]) == (sec, segs_)
        stream = msg + packets[0][:7] + b"\x00" + lv + wm
        got = session.split(stream)                     # the cut packet unframes, but does not parse
        assert [t for t, _ in got if t != session.ARCHIVE] == [session.HELLO, session.LIVE, session.WINDOW], name
        for t, body in got:
            if t == session.ARCHIVE:
                try:
                    session.parse_archive(body)
                    raise AssertionError(f"{name}: a cut ARCHIVE parsed")
                except ValueError:
                    pass
        print(f"{name:20s} {len(packets):3d} packets rebuild the file; resumed from entry {mid}: {len(resumed)}")
    print("every session rebuilds its files")


if __name__ == "__main__":
    main()
