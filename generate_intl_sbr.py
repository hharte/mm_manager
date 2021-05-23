#!/usr/bin/env python3
# -*- coding: latin-1 -*-
# www.github.com/hharte/mm_manager
#
# (c) 2020, Howard M. Harte
#
"""

This script generates the International Set-based rating table for Nortel Millennium Payphone.

The table is generated based on a .csv file containing International
calling codes and their associated RATE table entry.

The International Set-based rating able is an array of 603 bytes.
The first three bytes define the default flags and rate entry for
international codes not found in the remainder of the table.
The remainder of the table is an array of 200 entries of three bytes
each.  Two bytes for calling code, and one byte for flags/rate table
entry.  Thanks to æstrid for figuring this table out.

usage: generate_intl_sbr.py
"""

import array
import csv

print("International Set-based Rating Table Generator for the Nortel Millennium Payphone")
print("(c) 2020, Howard M. Harte\n")

print("Generating International Set-based Rating table")

ICC_DICT = {}

with open("icc_dial_codes.csv", mode='r') as iccfile:
    READER = csv.reader(iccfile)
    next(READER, None)  # Skip CSV header
    for rows in READER:
        ICC_DICT[rows[2]] = rows[3]

FNAME = "mm_table_97.bin"

# Start International table with Flags, Default Rate, Spare.
A = array.array('B', [0x01, 0x05, 0x00])

INDEX = 0

for item in ICC_DICT.items():
    INDEX += 1

    ccode = int(item[0])
    rate_entry = int(item[1])

    if rate_entry >= 2:
        rate_entry -= 28

    A.append(ccode & 0xff)
    A.append(ccode >> 8)
    A.append(rate_entry)

for i in range(INDEX, 200):
    A.append(0)
    A.append(0)
    A.append(0)

# Write LCD table array to file.
F = open(FNAME, "wb")
A.tofile(F)

print("Generated " + FNAME + ".")
