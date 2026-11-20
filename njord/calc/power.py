"""Njord — Iris, Arke and Ratatoskr: the power budgets. A rough calculation, run as it
stands; every figure from a sheet is named, every guess is bracketed and marked GUESS.

Sources: Semtech SX1261/2 rev 1.2 (Table 3-5 receive 4,6 mA, sleep 0,6 uA; Table 3-6 +14 dBm 45 mA,
+22 dBm 118 mA); Seeed Wio-SX1262 V1.0 (sleep 1,62 uA, 22 dBm 125 mA); Nordic nPM1300 PS v1.1 (IQBAT
0,8 uA); ST DS14217 rev 5 (U535 Stop 3 with full SRAM 2,2 uA; run ~19 + 2,5 uA/MHz on its LDO and
MSI); Sensirion SHT4x V7.3 (SHT45 0,4 uA at 1 Hz, idle 80 nA); Bosch BMP585 DS003 (1,3 uA at 1 Hz);
TI TMP117 (3,5 uA at 1 Hz, as NIC-Heimdall cites it). LoRa time on air by Semtech's formula."""
import math

def airtime(pl, sf, bw=125e3, cr=1, preamble=8, crc=1):
    de = 1 if sf >= 11 else 0
    ts = 2**sf / bw
    n = 8 + max(math.ceil((8*pl - 4*sf + 28 + 16*crc) / (4*(sf - 2*de))) * (cr + 4), 0)
    return (preamble + 4.25) * ts + n * ts

WAKES = 144                     # a day, every ten minutes
UP, ACK = 27, 14                # B on the air — ../LINK.md §3, the 8 B tag included (Imber's tips: 29 B)
RX_MARGIN = 0.05                # s the receive window stays open beyond the acknowledgement — GUESS

def radio_mAs(sf, tx_mA):
    """One wake's radio: the packet sent, the acknowledgement's window."""
    return tx_mA * airtime(UP, sf) + 4.6 * (airtime(ACK, sf) + RX_MARGIN)

# ---------------- Ratatoskr: one sodium-ion cell, the nPM1300, the U535 asleep between wakes -------
SLEEP_uA = 2.2 + 0.8 + 1.62 + 0.55            # U535 Stop 3 · nPM1300 · Wio-SX1262 · BMP585 deep standby — the SHT45s' branches switched off
SENSE_uAs = 2 * 0.4 + 1.3                     # two SHT45 · BMP585 — a reading each, from the 1 Hz figures
MCU_mA, MCU_S = 16 * 21.5e-3 + 0.2, 0.3       # U535 at 16 MHz plus peripherals (GUESS 0,2 mA), awake 0,3 s a wake
CELL_mAh = 1300 * 0.7                         # the cell is open (../ratatoskr/HARDWARE.md §1): an 18650's 1 300 mAh, the lower bound, ~70 % of it between 2,5 and 3,6 V

print("RATATOSKR — a day")
for label, sf, tx in (("near, 5 mW (14 dBm's current as the ceiling)", 7, 45),
                      ("near, SF9", 9, 45),
                      ("far, 22 dBm SF9", 9, 125),
                      ("far, 22 dBm SF10", 10, 125)):
    sleep = SLEEP_uA * 24 / 1000
    wake = WAKES * (radio_mAs(sf, tx) + MCU_mA * MCU_S + SENSE_uAs / 1000) / 3600
    day = sleep + wake
    print(f"  {label:44} sleep {sleep:4.2f} + wakes {wake:4.2f} = {day:4.2f} mAh"
          f" ({day*3.2:5.1f} mWh); the cell alone {CELL_mAh/day:4.0f} days")
for pct in (2, 5):                            # % a month — GUESS until the cell's sheet: sodium-ion's own loss
    print(f"  the cell's self-discharge at {pct} % a month: {1300*pct/100/30:4.2f} mAh a day")
print(f"  time on air: {UP} B up {', '.join(f'SF{s} {airtime(UP, s)*1e3:.0f} ms' for s in (7, 9, 10))};"
      f" {ACK} B ack SF7 {airtime(ACK, 7)*1e3:.0f} ms")
# On a public LoRaWAN network (../LINK.md §7): a measurement every ten minutes, one unacknowledged uplink an hour,
# six samples in 77 B plus LoRaWAN's 13 B; RX1 and RX2 opened after each (~30 ms each at SF7 — GUESS for the
# preamble's detection), a downlink in a few of them a day, ignored here.
LW = 77 + 13
for label, sf, tx in (("LoRaWAN, 14 dBm SF7, hourly", 7, 45), ("LoRaWAN, 14 dBm SF9, hourly", 9, 45)):
    sleep = SLEEP_uA * 24 / 1000
    meas = WAKES * (MCU_mA * MCU_S + SENSE_uAs / 1000) / 3600
    up = 24 * (tx * airtime(LW, sf) + 4.6 * 0.06) / 3600
    day = sleep + meas + up
    print(f"  {label:44} sleep {sleep:4.2f} + wakes {meas+up:4.2f} = {day:4.2f} mAh"
          f" ({day*3.2:5.1f} mWh); {LW} B {airtime(LW, sf)*1e3:.0f} ms, {24*airtime(LW, sf):.1f} s a day")
# the panel: December near Prague ~1 peak-sun hour on a steep panel; the 5 V stage and the charger ~60 % together (GUESS)
need = 2.6 * 3.2 / 1000 * 1.5                 # Wh a day with half again for margin (the far band's worst day)
print(f"  panel for the worst line with 50 % margin: {need/0.6:.2f} W — any small glass panel of 0,5–1 W carries it")

# ---------------- Iris: at the station, on the arm's 12 V, the radio always listening ------------------
print("\nIRIS — on the Palatine arm's 12 V, through the LMR43610 at ~80 % (GUESS at this load)")
for label, mcu in (("H523 asleep between Palatine's polls", 0.27), ("H523 awake at a low clock", 8.0)):
    ma33 = 4.6 + mcu + 1.0          # receiving · the H523 · the THVD1450 listening (GUESS 1 mA)
    w = ma33 * 3.3 / 0.8 / 1000
    print(f"  {label:40} {ma33:4.1f} mA at 3,3 V = {w:4.2f} W, {w*24:4.1f} Wh a day")

# ---------------- Arke: 4S sodium-ion as LiFePO4, ~13 V, a 20–50 W panel ------------------------------
print("\nARKE — a day at ~13 V")
node_W = (0.27 * 3.3 / 0.8 + 24 * 0) / 1000            # the H523 asleep through the LMR43610
radio_W = WAKES * radio_mAs(9, 125) * 3.3 / 0.8 / 3600 / 1000 * 24 / 24   # as an average power
for label, probes_W, mppt_W, bms_W in (("probes switched, a quiet MPPT and BMS", 0.05, 0.10, 0.013),
                                        ("probes switched, a smart BMS at 20 mA", 0.05, 0.10, 0.26),
                                        ("probes always on (0,3–0,9 W), quiet", 0.6, 0.10, 0.013),
                                        ("probes always on, MPPT 0,3 W, BMS 20 mA", 0.9, 0.30, 0.26)):
    w = node_W + radio_W + probes_W + mppt_W + bms_W
    print(f"  {label:42} {w:4.2f} W = {w*24:5.1f} Wh; from 20 W {20*0.75:4.1f} Wh, 50 W {50*0.75:4.1f} Wh"
          f" in December; seven dark days {w*24*7/13:4.1f} Ah at 13 V")
print(f"  of it the node {node_W*24*1000:.0f} mWh and the radio {radio_W*24*1000:.0f} mWh a day —"
      " the MPPT and the BMS set the size, not the node")
