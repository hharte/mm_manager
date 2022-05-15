#!/usr/bin/env python3
# www.github.com/hharte/mm_manager
#
# (c) 2020-2022, Howard M. Harte
#
# Utility functions to generate LCD tables for the Nortel Millennium
# Payphone.
#
# --------------------------------------------------------------------------
# MTR 1.20 / 2.x: Double-Compressed LCD Tables 136-155 (0x88-0x9b)
#
# The LCD Table is an array of 202 bytes.  The first two bytes contain
# the NPA followed by 'e', for example, NPA 408 would be represented as
# 0x40 0x8e.
#
# The remaining 200 bytes represent two bits for each NXX in the range
# of 200-999.
#
# These two bits encode a value as follows:
# 0 - Local Rate
# 1 - ?
# 2 - Intra-LATA
# 3 Invalid NPA/NXX
#
# --------------------------------------------------------------------------
# MTR 1.9: Compressed LCD Tables 101-107 (0x65-0x6b)
#
# The LCD Table is an array of 402 bytes.  The first two bytes contain
# the NPA followed by 'e', for example, NPA 408 would be represented as
# 0x40 0x8e.
#
# The remaining 400 bytes represent four bits for each NXX in the range
# of 200-999.
#
# These four bits encode a value as follows:
# 0 Local
# 1 LMS (future)
# 2 Intra-lata Toll
# 3 Invalid NPA/NXX
# 4 Inter-lata Toll
#
# --------------------------------------------------------------------------
# MTR 1.7: Uncompressed LCD Tables 74-81, 90, 91 (0x4a-0x51, 0x5a, 0x5b)
#
# The LCD Table is an array of 818 bytes.  The first three bytes contain
# the terminal's NPA-NXX.  The next two bytes contain
# the NPA followed by 'e', for example, NPA 408 would be represented as
# 0x40 0x8e.
#
# The remaining 800 bytes represent one byte for each NXX in the range
# of 200-999.
#
# These four bits encode a value as follows:
# 0 Local
# 1 LMS (future)
# 2 Intra-lata Toll
# 3 Invalid NPA/NXX
# 4 Inter-lata Toll
#

import array

def generate_lcd(table, i, npanxx_dict, my_npa, my_nxx):

    fname = "mm_table_" + hex(table)[2:4] + ".bin"

    npa_h = int(int(i) / 100)
    npa_t = int((int(i) - (npa_h * 100)) / 10)
    npa_o = int((int(i) - (npa_h * 100)) - (npa_t * 10))

    npa_h = npa_h << 4
    npa_h = npa_h | npa_t
    npa_o = npa_o << 4
    npa_o = npa_o | 0x0e

    # The LCD table starts with the first 3 digits of NPA followed by 0xe.
    a = array.array('B', [npa_h, npa_o])
    if (table < 115):
        print("       Compressed LCD table " + fname + " for NPA " + i + ".")
    else:
        print("Double-Compressed LCD table " + fname + " for NPA " + i + ".")

    # Uncompressed tables have 16 byges of padding between the NPA and the LCD data.
    if (table < 92):
        for index in range(0, 16):
            a.append(0)

    stflag = 0

    for index in range(200, 1000): #, row in allnpa.iterrows():
        cur_npanxx = str(i) + "-" + str(index)

        if cur_npanxx in npanxx_dict:
            flag = npanxx_dict.get(cur_npanxx)
        else:
            flag = 3    # No entry in NPA table, means invalid.

        if (table < 92):
            # Uncompressed table, each entry is one byte.
            a.append(flag)
        elif (table < 115):
            # Compressed table, two entries packed into a byte.
            # Pack into Compressed LCD byte
            stflag = stflag << 4
            stflag = stflag | flag

            # Every 2nd entry, add compressed LCD byte to table.
            if index % 2 == 1:
                a.append(stflag)
                stflag = 0
        else:
            # Double-compressed, four entries in one byte.
                # Pack into double-compressed LCD byte
                stflag = stflag << 2
                stflag = stflag | flag

                # Every 4th entry, add compressed LCD byte to table.
                if index % 4 == 3:
                    a.append(stflag)
                    stflag = 0

    # Write LCD table array to file.
    f=open(fname, "wb")
    a.tofile(f)

    return

def generate_lcd_tables(npanxx_dict, lcd_npas, my_npa, my_nxx):
    # Generate Double-Compressed LCD Tables
    # Start with table 136 (0x88)
    #
    # MTR 2.x supports only Double-Compressed LCD tables (16 maximum.)
    status = 0

    table = 136

    print("Generating MTR 1.20/2.x (Double-Compressed) tables:")
    # Loop through list of LCD NPAs for which we need to generate tables.
    for i in lcd_npas:
        if table > 155:
            print("Error: maximum of 16 LCD tables reached.")
            status = -1
            break

        generate_lcd(table, i, npanxx_dict, my_npa, my_nxx)

        # Proceed to next table
        table = table + 1

        if (table == 150):
            table = 154

    # Generate Compressed LCD Tables
    # Compressed tables start with 101 (0x65)
    #
    # MTR 1.9 supports 7 compressed tables for 7 NPAs maximum.
    table = 101

    print("Generating MTR 1.9 (Compressed) tables:")
    # Loop through list of LCD NPAs for which we need to generate tables.
    for i in lcd_npas:
        if table > 115:
            print("Error: maximum of 7 Compressed LCD tables reached.")
            status = -1
            break

        generate_lcd(table, i, npanxx_dict, my_npa, my_nxx)

        # Proceed to next table
        table = table + 1

    if table > 115:
        print("* * * WARNING: "  + str(my_npa) + "-" + str(my_nxx) + " has more than 15 NPAs and cannot be supported by MTR 1.9.")

    # Generate Uncompressed LCD Tables
    # Uncompressed tables start with table 74 (0x4a)
    #
    # MTR 1.7 only supports uncompressed tables (maximum of 10 NPAs.)
    table = 74

    print("Generating MTR 1.7 (Uncompressed) tables:")
    # Loop through list of LCD NPAs for which we need to generate tables.
    for i in lcd_npas:
        if table > 91:
            print("Error: maximum of 10 Uncompressed LCD tables reached.")
            status = -1
            break

        generate_lcd(table, i, npanxx_dict, my_npa, my_nxx)

        # Proceed to next table
        table = table + 1

        if (table == 82):
            table = 90

    if table > 92:
        print("* * * WARNING: "  + str(my_npa) + "-" + str(my_nxx) + " has more than 10 NPAs and cannot be supported by MTR 1.7.")

    return status
