#
# This is a "Manager" for the Nortel Millennium payhone.
#
# It can provision a Nortel Millennium payphone with Rev 1.0 or 1.3
# Control PCP.  CDRs, Alarms, and Maintenance Reports can also be
# retieved.
#
# (c) 2020, Howard M. Harte
#
# mm_manager - Millennium Manager
# mm_areacode - Dump Millennium Set Based Rating NPA Table 150.
# mm_lcd - Dump Millennium Local Call Determination (LCD) Tables (136-138).
#
IDIR =.
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj

DEPS = mm_manager.h

_MGR_OBJ = mm_manager.o mm_proto.o mm_util.o mm_modem.o
MGR_OBJ = $(patsubst %,$(ODIR)/%,$(_MGR_OBJ))

_ADMESS_OBJ = mm_admess.o
ADMESS_OBJ = $(patsubst %,$(ODIR)/%,$(_ADMESS_OBJ))

_NPA_OBJ = mm_areacode.o
NPA_OBJ = $(patsubst %,$(ODIR)/%,$(_NPA_OBJ))

_CALLSCRN_OBJ = mm_callscrn.o mm_util.o
CALLSCRN_OBJ = $(patsubst %,$(ODIR)/%,$(_CALLSCRN_OBJ))

_CARRIER_OBJ = mm_carrier.o
CARRIER_OBJ = $(patsubst %,$(ODIR)/%,$(_CARRIER_OBJ))

_FEATRU_OBJ = mm_featru.o mm_util.o
FEATRU_OBJ = $(patsubst %,$(ODIR)/%,$(_FEATRU_OBJ))

_LCD_OBJ = mm_lcd.o
LCD_OBJ = $(patsubst %,$(ODIR)/%,$(_LCD_OBJ))

_RATE_OBJ = mm_rate.o mm_util.o
RATE_OBJ = $(patsubst %,$(ODIR)/%,$(_RATE_OBJ))

_SMCARD_OBJ = mm_smcard.o
SMCARD_OBJ = $(patsubst %,$(ODIR)/%,$(_SMCARD_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: mm_manager mm_admess mm_areacode mm_callscrn mm_carrier mm_featru mm_lcd mm_rate mm_smcard

mm_manager: $(MGR_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_admess: $(ADMESS_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_areacode: $(NPA_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_callscrn: $(CALLSCRN_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_carrier: $(CARRIER_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_featru: $(FEATRU_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_lcd: $(LCD_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_rate: $(RATE_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_smcard: $(SMCARD_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
