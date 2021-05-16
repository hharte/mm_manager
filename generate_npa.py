#!/usr/bin/env python3
# Script to generate NPA tables for the Nortel Millennium Payphone
# Tables 150 (0x96)
#
# www.github.com/hharte/mm_manager
#
# (c) 2020, Howard M. Harte
#
# Table 0x96 is an array of 400 bytes, where each nibble corresponds
# to one of the area codes from 200-999.
#
# Each nibble contains a value representing the type of area code
# as follows:
#
# 0 - Invalid (for example, 211, 311, 800, 900)
# 2 - Unassigned at this time.
# 4 - US / Canada Area Code.
# 6 - International Area Code (for example, the Bahamas.)
#
# Details: https://nationalnanpa.com/reports/reports_npa.html
#
# All NPAs in the North American Numbering Plan https://nationalnanpa.com/nanp1/npa_report.csv
#
# usage: generate_npa.py [-h] [--debug DEBUG] [--datafile DATAFILE]
#                        [--countries COUNTRIES [COUNTRIES ...]]
#
# optional arguments:
#   -h, --help            show this help message and exit
#   --debug DEBUG         display debug output
#   --datafile DATAFILE   NPA source data file
#   --countries COUNTRIES [COUNTRIES ...]
#                         List of domestic countries#
#
#  For example, to generate an NPA table where US and Canada are domestic calls:
# ./generate_npa.py --countries US CANADA

import argparse
import array
import pandas

npa_cols = [ "NPA_ID", "type_of_code", "ASSIGNABLE", "EXPLANATION", "RESERVED", "ASSIGNED", "ASSIGNMENT_DT",
             "USE", "LOCATION", "COUNTRY", "IN_SERVICE", "IN_SERVICE_DT", "STATUS", "PLANNING_LETTERS",
             "NOTES", "OVERLAY", "OVERLAY_COMPLEX", "PARENT_NPA_ID", "SERVICE", "TIME_ZONE", "AREA_SERVED",
             "MAP", "IN_JEOPARDY", "RELIEF_PLANNING_IN_PROGRESS", "HOME_NPA_LOCAL_CALLS", "HOME_NPA_TOLL_CALLS",
             "FOREIGN_NPA_LOCAL_CALLS", "FOREIGN_NPA_TOLL_CALLS", "PERM_HNPA_LOCAL_CALLS", "PERM_HNPA_TOLL_CALLS",
             "PERM_HNPA_FOREIGN_LOCAL_CALLS", "DIALING_PLAN_NOTES", "Extra" ]

source_filename = 'npa_report.csv'

parser = argparse.ArgumentParser()
parser.add_argument("--debug", help="display debug output")
parser.add_argument("--datafile", help="NPA source data file")
parser.add_argument("--countries", nargs='+', help='List of domestic countries', default='US')

args = parser.parse_args()

print("NPA Table Generator for the Nortel Millennium Payphone")
print("(c) 2020, Howard M. Harte\n")

if args.datafile: source_filename = args.datafile

print("Reading data from " + source_filename)
try:
    df = pandas.read_csv(source_filename, dtype={'NPA_ID': object, 'COUNTRY': object}, skiprows=1, names=npa_cols, engine='python')
except OSError:
    print("Error: Cannot read NPA data file.")
    print("Please download https://nationalnanpa.com/nanp1/npa_report.csv into the current directory.")
    exit()

df["NPA_ID"].fillna("0", inplace = True)
df["COUNTRY"].fillna("", inplace = True)

if args.debug: print(df[['NPA_ID', 'COUNTRY', 'ASSIGNED']])


# Start with table 150 (0x96)
table = 150

fname = "mm_table_" + hex(table)[2:4] + ".bin"
print("Generating NPA table " + fname + ".")

# Start with empty array.
a = array.array('B', [])

stflag = 0

for index in range(200, 1000): #, row in allnpa.iterrows():
    use = df.loc[df['NPA_ID'] == str(index)]
    flag = 0

    if use.empty:   # No entry in NPA table, means invalid.
        flag = 0
    else:
        if (str(use['IN_SERVICE'].values[0]) == "Y"):   # NPA is in service
            # Either local or LD based on rate center.
            if use['COUNTRY'].values[0] in args.countries:
                flag = 4 # Matches a country in list
            else:
                flag = 6 # International

            if use['COUNTRY'].values[0] == "":
                flag = 2    # Non-Geographic Services

            if use['SERVICE'].values[0] == "Premium Services":
                flag = 0    # Premium service

            if use['SERVICE'].values[0] == "Toll-Free":
                flag = 2    # Toll-Free

        else:   # Not in service, check if NPA is assignable
            if (str(use['ASSIGNABLE'].values[0]) == "Yes") | \
               (str(use['EXPLANATION'].values[0]) != "N11 Code"):
                flag = 2 # Unassigned at this time.
            else:
                flag = 0 # Not assignable
        if index == 800:
            flag = 0    # 1-800 is "Not available"

    # Pack into double-compressed LCD byte
    stflag = stflag << 4
    stflag = stflag | flag

    # Every 4th entry, add compressed LCD byte to table.
    if index % 2 == 1:
        a.append(stflag)
        stflag = 0

# Display table data
if args.debug: print(a)

# Write LCD table array to file.
f=open(fname, "wb")
a.tofile(f)
f.close

print("Successfully generated the NPA table.")
