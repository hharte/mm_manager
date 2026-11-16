#!/usr/bin/env python3
# Generate Numbering Plan (0x3e) and Call Screening (0x18) tables for
# non-NANP dialling on the Nortel Millennium Payphone.
#
# www.github.com/hharte/mm_manager
#
# usage: generate_numplan.py <table_3e.bin> <numplan.csv> <callscrn.csv> <out_dir>
#
# <table_3e.bin> is table 0x3e cut from the terminal's control ROM with
# mm_table_cutter, or an existing mm_table_3e.bin; only its rules are replaced.
# Patterns use 0-9 and X (any digit).  Tested on control ROM 06CAA17.
import csv
import sys

DIGIT = {"0": 0x0A, "X": 0x11, **{d: int(d) for d in "123456789"}}

# Only type 0x3_ rules copy the dialled digits into the outgoing number;
# 0x2_ and 0x4_ are carrier access codes.
RULE_FLAGS, RULE_TYPE, RULE_CALL_TYPE = 0x08, 0x30, 0x43
RULES_START, RULES_END = 32, 212

CS_FREE, CS_ASUP, CS_KPEN, CS_MUTE = 0x01, 0x02, 0x04, 0x10
CS_CALL_LOCAL, CS_CALL_DENIED = 0x03, 0x0B
CALLSCRN_ACTIONS = {
    "emergency": (CS_FREE | CS_ASUP, CS_CALL_LOCAL),
    "free": (CS_FREE | CS_ASUP | CS_KPEN, CS_CALL_LOCAL),
    "deny": (CS_ASUP | CS_MUTE, CS_CALL_DENIED),
}
CALLSCRN_ENTRIES, CALLSCRN_ENTRY_LEN = 60, 12


def numplan_rules(patterns):
    out = b""
    for i, p in enumerate(patterns):
        nxt = len(out) + 6 + len(p) if i + 1 < len(patterns) else 0
        out += bytes([nxt, 0, RULE_FLAGS, RULE_TYPE, RULE_CALL_TYPE, len(p)]) + bytes(DIGIT[c] for c in p)
    size = RULES_END - RULES_START
    if len(out) > size:
        sys.exit(f"numplan: rules need {len(out)} bytes, table has room for {size}")
    return out + bytes(size - len(out))


def callscrn_number(p):
    nib = [0x0A if c == "0" else 0x0F if c == "X" else int(c) for c in p] + [0]
    nib += [0] * (16 - len(nib))
    return bytes((nib[i] << 4) | nib[i + 1] for i in range(0, 16, 2))


def callscrn_table(rows):
    if len(rows) > CALLSCRN_ENTRIES:
        sys.exit(f"callscrn: {len(rows)} entries, table holds {CALLSCRN_ENTRIES}")
    out = b""
    for r in rows:
        fcf, call_type = CALLSCRN_ACTIONS[r["Action"]]
        # Byte 3 is the number of digits still to come after this number (0 = exact).
        out += bytes([fcf, call_type, 0xFF, 0]) + callscrn_number(r["Number"])
    return out + bytes(CALLSCRN_ENTRY_LEN * (CALLSCRN_ENTRIES - len(rows)))


def main():
    if len(sys.argv) != 5:
        sys.exit("usage: generate_numplan.py <table_3e.bin> <numplan.csv> <callscrn.csv> <out_dir>")
    base_path, numplan_csv, callscrn_csv, out_dir = sys.argv[1:]
    base = open(base_path, "rb").read()
    if len(base) == 353:
        base = base[1:]
    if len(base) != 352:
        sys.exit(f"{base_path}: expected 352 or 353 bytes, got {len(base)}")
    patterns = [r["Pattern"] for r in csv.DictReader(open(numplan_csv))]
    callscrn = list(csv.DictReader(open(callscrn_csv)))
    t3e = base[:RULES_START] + numplan_rules(patterns) + base[RULES_END:]
    open(f"{out_dir}/mm_table_3e.bin", "wb").write(t3e)
    open(f"{out_dir}/mm_table_18.bin", "wb").write(callscrn_table(callscrn))


main()
