"""The uplink session's messages, the Python reference, as `core/PROTOCOL.md` §6 fixes them:
HELLO, CURSOR, ARCHIVE with its chained tag, ACK, LIVE, WINDOW, BOARD, COMMAND and SET_K1; the head's cut of a file
for a cursor or a window, and the server's rebuilding of the file from the packets. The C
(`c/session.c`) writes the same bytes, and `tests/test_c.py` compares them."""

from __future__ import annotations

import hashlib
import hmac
import struct
from typing import Dict, List, Optional, Sequence, Tuple

import hmc

HELLO, CURSOR, ARCHIVE, ACK, LIVE, WINDOW, BOARD, COMMAND, SET_K1 = range(1, 10)
NONE_ENTRY = 0xFFFFFFFF


def tag(key: bytes, data: bytes) -> bytes:
    """HMAC-SHA-256 under K0‖K1, its first 8 B."""
    return hmac.new(key, data, hashlib.sha256).digest()[:8]


def message(raw: bytes) -> bytes:
    return hmc.framed(raw)


def unframe(framed_without_zero: bytes) -> Tuple[int, bytes]:
    raw = hmc.cobs_decode(framed_without_zero)
    if not raw:
        raise ValueError("session: an empty message")
    return raw[0], raw[1:]


def split(stream: bytes) -> List[Tuple[int, bytes]]:
    """A byte stream's whole messages; a damaged one is skipped to the next zero."""
    out = []
    for chunk in stream.split(b"\x00"):
        if chunk:
            try:
                out.append(unframe(chunk))
            except ValueError:
                pass
    return out


# ---------------------------------------------------------------- the messages

def hello(key: bytes, station: int, counter: int, nonce: bytes, board: bool) -> Tuple[bytes, bytes]:
    """(the message, its tag — the chain's start)."""
    raw = bytes([HELLO, 1]) + station.to_bytes(3, "little") + struct.pack("<I", counter) + nonce \
        + bytes([int(board)])
    t = tag(key, raw)
    return message(raw + t), t


def parse_hello(key: bytes, body: bytes) -> dict:
    if len(body) != 25 or body[0] != 1:
        raise ValueError("session: not a HELLO")
    if not hmac.compare_digest(tag(key, bytes([HELLO]) + body[:17]), body[17:]):
        raise ValueError("session: HELLO's tag fails")
    return {"station": int.from_bytes(body[1:4], "little"), "counter": struct.unpack_from("<I", body, 4)[0],
            "nonce": body[8:16], "board": bool(body[16]), "tag": body[17:]}


def marks(type_: int, held: Sequence[Tuple[int, int, int]]) -> bytes:
    """CURSOR or ACK: (address, sequence, entry) per file."""
    return message(bytes([type_]) + struct.pack("<H", len(held))
                   + b"".join(struct.pack("<BHI", a, s, e) for a, s, e in held))


def parse_marks(body: bytes) -> List[Tuple[int, int, int]]:
    if len(body) < 2 or len(body) != 2 + 7 * struct.unpack_from("<H", body)[0]:
        raise ValueError("session: a cursor cut short")
    return [struct.unpack_from("<BHI", body, 2 + 7 * i) for i in range(struct.unpack_from("<H", body)[0])]


class Chain:
    """The ARCHIVE packets' tags: each over the nonce, the previous tag and the packet."""

    def __init__(self, key: bytes, nonce: bytes, hello_tag: bytes):
        self.key, self.nonce, self.last, self.packet = key, nonce, hello_tag, 0

    def archive(self, address: int, sequence: int, header: bytes,
                objects: Sequence[Tuple[int, bytes, bytes]]) -> bytes:
        """objects: (index, entry 16 B, COBS bytes)."""
        self.packet += 1
        raw = bytes([ARCHIVE]) + struct.pack("<BHII", address, sequence, self.packet, len(header)) + header \
            + struct.pack("<H", len(objects)) \
            + b"".join(struct.pack("<I", i) + e + c for i, e, c in objects)
        self.last = tag(self.key, self.nonce + self.last + raw)
        return message(raw + self.last)

    def verify(self, body: bytes) -> dict:
        """The server's side: the packet parsed, its tag checked against the chain."""
        raw = bytes([ARCHIVE]) + body[:-8]
        want = tag(self.key, self.nonce + self.last + raw)
        if len(body) < 8 or not hmac.compare_digest(want, body[-8:]):
            raise ValueError("session: an ARCHIVE packet's tag fails")
        p = parse_archive(body)
        if p["packet"] != self.packet + 1:
            raise ValueError("session: an ARCHIVE packet out of order")
        self.packet, self.last = p["packet"], body[-8:]
        return p


def parse_archive(body: bytes) -> dict:
    try:
        address, sequence, packet, hlen = struct.unpack_from("<BHII", body)
        i = 11
        header = body[i:i + hlen]; i += hlen
        count = struct.unpack_from("<H", body, i)[0]; i += 2
        objects = []
        for _ in range(count):
            index = struct.unpack_from("<I", body, i)[0]
            entry = body[i + 4:i + 20]
            e = hmc.parse_entry(entry)
            if e is None or i + 20 + e[2] > len(body) - 8:
                raise ValueError
            objects.append((index, entry, body[i + 20:i + 20 + e[2]]))
            i += 20 + e[2]
        if i != len(body) - 8 or len(header) != hlen:
            raise ValueError
    except (struct.error, ValueError):
        raise ValueError("session: an ARCHIVE packet cut short")
    return {"address": address, "sequence": sequence, "packet": packet, "header": header,
            "objects": objects, "tag": body[-8:]}


def live(second: int, segments: Sequence[Tuple[int, bytes]]) -> bytes:
    return message(bytes([LIVE]) + live_body(second, segments))


def live_body(second: int, segments: Sequence[Tuple[int, bytes]]) -> bytes:
    return struct.pack("<IB", second, len(segments)) + \
        b"".join(struct.pack("<BI", a, len(s)) + s for a, s in segments)


def parse_live(body: bytes) -> Tuple[int, List[Tuple[int, bytes]]]:
    try:
        second, count = struct.unpack_from("<IB", body)
        i, out = 5, []
        for _ in range(count):
            a, n = struct.unpack_from("<BI", body, i)
            if i + 5 + n > len(body):
                raise ValueError
            out.append((a, body[i + 5:i + 5 + n])); i += 5 + n
        if i != len(body):
            raise ValueError
    except (struct.error, ValueError):
        raise ValueError("session: a LIVE packet cut short")
    return second, out


def window(key: bytes, station: int, counter: int, address: int, second: int, span: int) -> bytes:
    raw = bytes([WINDOW]) + struct.pack("<IBII", counter, address, second, span)
    return message(raw + tag(key, station.to_bytes(3, "little") + raw))


def parse_window(key: bytes, station: int, stored_counter: int, body: bytes) -> dict:
    if len(body) != 21:
        raise ValueError("session: not a WINDOW")
    raw = bytes([WINDOW]) + body[:13]
    if not hmac.compare_digest(tag(key, station.to_bytes(3, "little") + raw), body[13:]):
        raise ValueError("session: WINDOW's tag fails")
    counter, address, second, span = struct.unpack_from("<IBII", body)
    if counter <= stored_counter:
        raise ValueError("session: WINDOW's counter is not above the stored one")
    return {"counter": counter, "address": address, "second": second, "window": span}


def command(key: bytes, station: int, counter: int, text: str) -> bytes:
    """The handset's JSON command, or the server's, in its counter and tag. Counter 0 is a read:
    a `refresh`, taken unchecked, moving nothing; its tag is zeros."""
    raw = bytes([COMMAND]) + struct.pack("<I", counter) + text.encode("utf-8")
    t = bytes(8) if counter == 0 else tag(key, station.to_bytes(3, "little") + raw)
    return message(raw + t)


def parse_command(key: bytes, station: int, stored_counter: int, body: bytes) -> dict:
    if len(body) < 12:
        raise ValueError("session: not a COMMAND")
    counter = struct.unpack_from("<I", body)[0]
    text = body[4:-8].decode("utf-8")
    if counter == 0:
        return {"counter": 0, "command": text}
    if counter <= stored_counter:
        raise ValueError("session: COMMAND's counter is not above the stored one")
    raw = bytes([COMMAND]) + body[:-8]
    if not hmac.compare_digest(tag(key, station.to_bytes(3, "little") + raw), body[-8:]):
        raise ValueError("session: COMMAND's tag fails")
    return {"counter": counter, "command": text}


def k1_pad(k0: bytes, station: int, counter: int) -> bytes:
    """The wrap: the first 8 B of HMAC-SHA-256 under K0 over the station and the counter."""
    return tag(k0, station.to_bytes(3, "little") + struct.pack("<I", counter))


def set_k1(k0: bytes, station: int, counter: int, k1: bytes) -> bytes:
    """The re-key: the new K1 XOR'd with the wrap, the tag under K0 alone."""
    wrapped = bytes(a ^ b for a, b in zip(k1, k1_pad(k0, station, counter)))
    raw = bytes([SET_K1]) + struct.pack("<I", counter) + wrapped
    return message(raw + tag(k0, station.to_bytes(3, "little") + raw))


def parse_set_k1(k0: bytes, station: int, stored_counter: int, body: bytes) -> dict:
    if len(body) != 20:
        raise ValueError("session: not a SET_K1")
    counter = struct.unpack_from("<I", body)[0]
    if counter <= stored_counter:
        raise ValueError("session: SET_K1's counter is not above the stored one")
    raw = bytes([SET_K1]) + body[:12]
    if not hmac.compare_digest(tag(k0, station.to_bytes(3, "little") + raw), body[12:]):
        raise ValueError("session: SET_K1's tag fails")
    k1 = bytes(a ^ b for a, b in zip(body[4:12], k1_pad(k0, station, counter)))
    return {"counter": counter, "k1": k1}


def board(station: int, live_body_: bytes) -> bytes:
    return message(bytes([BOARD]) + station.to_bytes(3, "little") + live_body_)


# ---------------------------------------------------------------- the files

def file_objects(image: bytes) -> Tuple[hmc.Header, List[Tuple[int, bytes, bytes, Tuple]]]:
    """A file's objects as an ARCHIVE packet carries them: (index, entry, COBS bytes, entry)."""
    r = hmc.Reader(image)
    out = []
    for i, e in enumerate(r.entries):
        if e is None:
            continue
        at = len(image) - 16 * (i + 1)
        out.append((i, bytes(image[at:at + 16]), bytes(image[e[1]:e[1] + e[2]]), e))
    return r.header, out


def forward(image: bytes, held: int, budget: int = 16384) -> List[List[Tuple[int, bytes, bytes]]]:
    """The head's packets from the entry after `held` on, each up to `budget` bytes of objects."""
    _, objs = file_objects(image)
    packets, cur, size = [], [], 0
    for i, e, c, _ in objs:
        if held != NONE_ENTRY and i <= held:
            continue
        if cur and size + 20 + len(c) > budget:
            packets.append(cur); cur, size = [], 0
        cur.append((i, e, c)); size += 20 + len(c)
    if cur:
        packets.append(cur)
    return packets


def window_cut(image: bytes, second: int, span: int) -> List[Tuple[int, bytes, bytes]]:
    """The segments whose first to last second touches [second − span, second + span], and the
    configs before the last of them."""
    _, objs = file_objects(image)
    lo, hi = second - span, second + span
    segs = []
    for i, e, c, ent in objs:
        if ent[3] != hmc.SEG:
            continue
        try:
            t, o = hmc.parse_object(hmc.cobs_decode(c))
        except ValueError:
            continue
        if t == hmc.SEG and o["first"] <= hi and o["last"] >= lo:
            segs.append(i)
    if not segs:
        return []
    return [(i, e, c) for i, e, c, ent in objs
            if i in segs or (ent[3] == hmc.CFG and i < segs[-1])]


class Server:
    """The server's file, rebuilt from ARCHIVE packets: the header first, then each object at its
    entry's offset and the entry in its place."""

    def __init__(self):
        self.image: Optional[bytearray] = None
        self.held = NONE_ENTRY

    def take(self, p: dict) -> None:
        if p["header"]:
            h = hmc.parse_header(p["header"])
            if self.image is None:
                self.image = bytearray(hmc.MB << h.size)
            self.image[:len(p["header"])] = p["header"]
        for index, entry, cobs in p["objects"]:
            e = hmc.parse_entry(entry)
            self.image[e[1]:e[1] + e[2]] = cobs
            self.image[e[1] + e[2]] = 0
            at = len(self.image) - 16 * (index + 1)
            self.image[at:at + 16] = entry
            self.held = index if self.held == NONE_ENTRY else max(self.held, index)
