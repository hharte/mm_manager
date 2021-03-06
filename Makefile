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
GIT_VERSION := "$(shell git describe --dirty --always --tags)"
CFLAGS=-I$(IDIR) -DVERSION=\"$(GIT_VERSION)\"

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

_CARD_OBJ = mm_card.o mm_util.o
CARD_OBJ = $(patsubst %,$(ODIR)/%,$(_CARD_OBJ))

_CARRIER_OBJ = mm_carrier.o mm_util.o
CARRIER_OBJ = $(patsubst %,$(ODIR)/%,$(_CARRIER_OBJ))

_COINVL_OBJ = mm_coinvl.o mm_util.o
COINVL_OBJ = $(patsubst %,$(ODIR)/%,$(_COINVL_OBJ))

_FEATRU_OBJ = mm_featru.o mm_util.o
FEATRU_OBJ = $(patsubst %,$(ODIR)/%,$(_FEATRU_OBJ))

_INSTSV_OBJ = mm_instsv.o mm_util.o
INSTSV_OBJ = $(patsubst %,$(ODIR)/%,$(_INSTSV_OBJ))

_INTL_RATE_OBJ = mm_rateint.o mm_util.o
INTL_RATE_OBJ = $(patsubst %,$(ODIR)/%,$(_INTL_RATE_OBJ))

_LCD_OBJ = mm_lcd.o
LCD_OBJ = $(patsubst %,$(ODIR)/%,$(_LCD_OBJ))

_RATE_OBJ = mm_rate.o mm_util.o
RATE_OBJ = $(patsubst %,$(ODIR)/%,$(_RATE_OBJ))

_RDLIST_OBJ = mm_rdlist.o mm_util.o
RDLIST_OBJ = $(patsubst %,$(ODIR)/%,$(_RDLIST_OBJ))

_SMCARD_OBJ = mm_smcard.o
SMCARD_OBJ = $(patsubst %,$(ODIR)/%,$(_SMCARD_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

EXE = mm_manager mm_admess mm_areacode mm_callscrn mm_carrier mm_coinvl mm_card mm_featru mm_instsv mm_lcd mm_rate mm_rateint mm_rdlist mm_smcard

all: $(EXE)

mm_manager: $(MGR_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_admess: $(ADMESS_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_areacode: $(NPA_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_callscrn: $(CALLSCRN_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_card: $(CARD_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_carrier: $(CARRIER_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_coinvl: $(COINVL_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_featru: $(FEATRU_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_instsv: $(INSTSV_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_lcd: $(LCD_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_rate: $(RATE_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_rateint: $(INTL_RATE_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_rdlist: $(RDLIST_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_smcard: $(SMCARD_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean clobber

clean:
	rm -f $(ODIR)/*.o

clobber: clean
	rm -f $(EXE)
