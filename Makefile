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

_NPA_OBJ = mm_areacode.o
NPA_OBJ = $(patsubst %,$(ODIR)/%,$(_NPA_OBJ))

_LCD_OBJ = mm_lcd.o
LCD_OBJ = $(patsubst %,$(ODIR)/%,$(_LCD_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: mm_manager mm_areacode mm_lcd

mm_manager: $(MGR_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_areacode: $(NPA_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

mm_lcd: $(LCD_OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
