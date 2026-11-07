CC := arm-linux-gnueabi-gcc
RM := rm

M5HOME     := /home/cristinel/hw4/gem5
M5BIN      := $(M5HOME)/build/ARM/gem5.opt
FS_DISK    := $(M5HOME)/full_system_images_ARM/disks/linux-aarch32-ael.img
FS_KERNEL  := $(M5HOME)/full_system_images_ARM/binaries/vmlinux.aarch32.ll_20131205.0-gem5
FS_CFG     := ./fs_modified.py
SIM_SCRIPT := ./sim.sh
M5TOMCPAT  := python m5-mcpat-parse-se-ALPHA-ARM-Bus.py
MCPAT_BIN  := $(M5HOME)/mcpat/mcpat

NUM_CPU    := 4
#L1SIZE     := 32kB
#L2SIZE     := 512kB
SIM_OPT    := --caches --l2cache --cpu-type=timing --l1i_size=16kB --l1d_size=16kB --l1i_assoc=2 --l1d_assoc=2 --l2_size=512kB --l2_assoc=2 --cacheline_size=64 --mem-size=256MB 

SRCDIR :=.
OBJDIR :=.
LIBDIR :=
INCDIR :=-I.

SRC    := $(wildcard *.c)
HDR    := $(wildcard *.h)

OBJ    := $(SRC:%.c=%.o)
LIB    :=
BIN    := ffos
DEPEND := depend

INPUT_2_FFOS := input.pgm

OPT     := -O2
CFLAGS  := -std=c99 -Wall -fopenmp $(OPT) $(INCDIR)
LDFLAGS := -fopenmp $(OPT) -static

all: $(BIN)

run: all
	@echo Copying executable to disk image...
	@./scr_cp2dm $(BIN) $(FS_DISK)
	@./scr_cp2dm $(INPUT_2_FFOS) $(FS_DISK)
	@echo Running simulation, may take a while...
	@$(M5BIN) $(FS_CFG) --kernel=$(FS_KERNEL) --disk-image=$(FS_DISK) $(SIM_OPT) --script=$(SIM_SCRIPT) -n $(NUM_CPU)
	@echo Generating XML for McPAT
	@$(M5TOMCPAT)
	@echo Running McPAT
	@$(MCPAT_BIN) -infile power.xml > mcpat_out.txt

$(BIN): $(DEPEND) $(OBJ) m5op_arm.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BIN) $(OBJ) m5op_arm.o

m5op_arm.o:m5op.h m5op_arm.S
	$(CC) $(CFLAGS) -c -o m5op_arm.o m5op_arm.S

$(DEPEND): $(SRC) $(HDR)
	@echo 'checking dependencies'
	@$(SHELL) -ec '$(CC) -MM $(INCDIR) $(SRC)>$(DEPEND)'
	@echo


.PHONY: all clean cleanobj run

cleanobj:
	-$(RM) -f $(OBJDIR)/*.o
	-$(RM) -f $(DEPEND)

clean: cleanobj
	-$(RM) -f $(BIN)
	-$(RM) -f output.pgm binary.pgm erosion.pgm
	-$(RM) -rf m5out
	-$(RM) -f power.xml summary.xml mcpat_out.txt

-include $(DEPEND)
