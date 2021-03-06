#
#  Makefile for Bus
#
#  ** This file was automatically generated by the command:
#  opp_makemake -f -N -b ../../ -c ../../fieldbusconfig -n -r -I../../Common -I../Common -I../Ethernet
#

# Name of target to be created (-o option)
TARGET = Bus

# User interface (uncomment one) (-u option)
# USERIF_LIBS=$(CMDENV_LIBS)
USERIF_LIBS=$(TKENV_LIBS)

# .ned or .h include paths with -I
INCLUDE_PATH= -I../../Common -I../Common -I../Ethernet

# misc additional object and library files to link
EXTRA_OBJS=

# object files from other directories to link with (wildcard needed to prevent "no such file *.o" errors)
EXT_DIR_OBJS=

# time stamps of other directories (used as dependency)
EXT_DIR_TSTAMPS= ../../Common/.tstamp ../Common/.tstamp ../Ethernet/.tstamp

# Additional libraries (-L option -l option)
LIBS=

#------------------------------------------------------------------------------
# Import generic settings from ../../fieldbusconfig
include ../../fieldbusconfig

#------------------------------------------------------------------------------

# subdirectories to recurse into
SUBDIRS= 

# object files in this directory
OBJS=  Bus.o BusLogging.o ControlNetBus.o DeviceNetBus.o EthernetBus.o LonTalkBus.o Utils.o

# header files generated (from msg files)
GENERATEDHEADERS= 

#------------------------------------------------------------------------------

$(TARGET): $(OBJS) subdirs Makefile .tstamp
	@# do nothing

.tstamp: $(OBJS)
	echo>.tstamp

$(OBJS) : $(GENERATEDHEADERS)


purify: $(OBJS) $(EXTRA_OBJS) $(EXT_DIR_TSTAMPS) subdirs Makefile
	purify $(CXX) $(LDFLAGS) $(OBJS) $(EXTRA_OBJS) $(EXT_DIR_OBJS) $(LIBS) -L$(OMNETPP_LIB_DIR) $(KERNEL_LIBS) $(USERIF_LIBS) $(SYS_LIBS_PURE) -o $(TARGET).pure

$(EXT_DIR_TSTAMPS):
	@echo
	@echo Error: $@ does not exist.
	@echo This means that at least the above dependency directory has not been built.
	@echo Maybe you need to do a top-level make?
	@echo
	@exit 1

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

Bus.o: Bus.cc
	$(CXX) -c $(COPTS) Bus.cc

BusLogging.o: BusLogging.cc
	$(CXX) -c $(COPTS) BusLogging.cc

ControlNetBus.o: ControlNetBus.cc
	$(CXX) -c $(COPTS) ControlNetBus.cc

DeviceNetBus.o: DeviceNetBus.cc
	$(CXX) -c $(COPTS) DeviceNetBus.cc

EthernetBus.o: EthernetBus.cc
	$(CXX) -c $(COPTS) EthernetBus.cc

LonTalkBus.o: LonTalkBus.cc
	$(CXX) -c $(COPTS) LonTalkBus.cc

Utils.o: Utils.cc
	$(CXX) -c $(COPTS) Utils.cc


#doc: neddoc doxy

#neddoc:
#	opp_neddoc -a

#doxy: doxy.cfg
#	doxygen doxy.cfg

generateheaders: $(GENERATEDHEADERS)
	for i in $(SUBDIRS); do (cd $$i && $(MAKE) generateheaders) || exit 1; done

clean:
	rm -f *.o *_n.cc *_n.h *_m.cc *_m.h .tstamp
	rm -f *.vec *.sca
	for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean); done

depend:
	$(MAKEDEPEND) $(INCLUDE_PATH) -- *.cc
	# $(MAKEDEPEND) $(INCLUDE_PATH) -fMakefile.in -- *.cc
	for i in $(SUBDIRS); do (cd $$i && $(MAKE) depend) || exit 1; done

makefiles:
	# recreate Makefile
	opp_makemake -f  -N -b ../../ -c ../../fieldbusconfig -n -r -I../../Common -I../Common -I../Ethernet 
	for i in $(SUBDIRS); do (cd $$i && $(MAKE) makefiles) || exit 1; done

makefile-ins:
	# recreate Makefile.in
	opp_makemake -f -m  -N -b ../../ -c ../../fieldbusconfig -n -r -I../../Common -I../Common -I../Ethernet 
	for i in $(SUBDIRS); do (cd $$i && $(MAKE) makefile-ins) || exit 1; done

# "re-makemake" and "re-makemake-m" are deprecated, historic names of the above two targets
re-makemake: makefiles
re-makemake-m: makefile-ins


# DO NOT DELETE THIS LINE -- make depend depends on it.
Bus.o: Bus.cc \
  Bus.h \
  Utils.h \
  ../../Common/Util.h \
  BusLogging.h \
  ../Ethernet/EthernetFrame_m.h \
  ../../Common/Logging.h \
  ../../Common/../ttl/var/variant.hpp \
  ../../Common/../ttl/var/../meta/typelist.hpp \
  ../../Common/../ttl/var/../meta/is_reference.hpp \
  ../../Common/../ttl/var/../meta/is_pointer.hpp \
  ../../Common/../ttl/var/../meta/is_same.hpp \
  ../../Common/../ttl/var/../meta/../config.hpp \
  ../../Common/../ttl/var/../meta/../data_holder.hpp \
  ../../Common/../ttl/var/../meta/../exception.hpp \
  ../../Common/../ttl/var/../meta/../macro_params.hpp \
  ../../Common/../ttl/var/../meta/../equivalent_types.hpp \
  ../../Common/../ttl/var/../meta/../selector.hpp \
  ../../Common/../ttl/var/../meta/../macro_repeat.hpp \
  ../../Common/../ttl/var/../meta/../macro_misc.hpp \
  ../../Common/../ttl/var/../meta/../detail/macro_counter.hpp \
  ../../Common/../ttl/var/../meta/../detail/../macro_misc.hpp \
  ../Common/Frame_m.h
BusLogging.o: BusLogging.cc \
  BusLogging.h \
  ../../Common/Logging.h \
  ../../Common/Util.h \
  ../../Common/../ttl/var/variant.hpp \
  ../../Common/../ttl/var/../meta/typelist.hpp \
  ../../Common/../ttl/var/../meta/is_reference.hpp \
  ../../Common/../ttl/var/../meta/is_pointer.hpp \
  ../../Common/../ttl/var/../meta/is_same.hpp \
  ../../Common/../ttl/var/../meta/../config.hpp \
  ../../Common/../ttl/var/../meta/../data_holder.hpp \
  ../../Common/../ttl/var/../meta/../exception.hpp \
  ../../Common/../ttl/var/../meta/../macro_params.hpp \
  ../../Common/../ttl/var/../meta/../equivalent_types.hpp \
  ../../Common/../ttl/var/../meta/../selector.hpp \
  ../../Common/../ttl/var/../meta/../macro_repeat.hpp \
  ../../Common/../ttl/var/../meta/../macro_misc.hpp \
  ../../Common/../ttl/var/../meta/../detail/macro_counter.hpp \
  ../../Common/../ttl/var/../meta/../detail/../macro_misc.hpp
ControlNetBus.o: ControlNetBus.cc \
  Bus.h \
  Utils.h \
  ../../Common/Util.h \
  BusLogging.h \
  ../Ethernet/EthernetFrame_m.h \
  ../../Common/Logging.h \
  ../../Common/../ttl/var/variant.hpp \
  ../../Common/../ttl/var/../meta/typelist.hpp \
  ../../Common/../ttl/var/../meta/is_reference.hpp \
  ../../Common/../ttl/var/../meta/is_pointer.hpp \
  ../../Common/../ttl/var/../meta/is_same.hpp \
  ../../Common/../ttl/var/../meta/../config.hpp \
  ../../Common/../ttl/var/../meta/../data_holder.hpp \
  ../../Common/../ttl/var/../meta/../exception.hpp \
  ../../Common/../ttl/var/../meta/../macro_params.hpp \
  ../../Common/../ttl/var/../meta/../equivalent_types.hpp \
  ../../Common/../ttl/var/../meta/../selector.hpp \
  ../../Common/../ttl/var/../meta/../macro_repeat.hpp \
  ../../Common/../ttl/var/../meta/../macro_misc.hpp \
  ../../Common/../ttl/var/../meta/../detail/macro_counter.hpp \
  ../../Common/../ttl/var/../meta/../detail/../macro_misc.hpp \
  ../Common/Frame_m.h
DeviceNetBus.o: DeviceNetBus.cc \
  Bus.h \
  Utils.h \
  ../../Common/Util.h \
  BusLogging.h \
  ../Ethernet/EthernetFrame_m.h \
  ../../Common/Logging.h \
  ../../Common/../ttl/var/variant.hpp \
  ../../Common/../ttl/var/../meta/typelist.hpp \
  ../../Common/../ttl/var/../meta/is_reference.hpp \
  ../../Common/../ttl/var/../meta/is_pointer.hpp \
  ../../Common/../ttl/var/../meta/is_same.hpp \
  ../../Common/../ttl/var/../meta/../config.hpp \
  ../../Common/../ttl/var/../meta/../data_holder.hpp \
  ../../Common/../ttl/var/../meta/../exception.hpp \
  ../../Common/../ttl/var/../meta/../macro_params.hpp \
  ../../Common/../ttl/var/../meta/../equivalent_types.hpp \
  ../../Common/../ttl/var/../meta/../selector.hpp \
  ../../Common/../ttl/var/../meta/../macro_repeat.hpp \
  ../../Common/../ttl/var/../meta/../macro_misc.hpp \
  ../../Common/../ttl/var/../meta/../detail/macro_counter.hpp \
  ../../Common/../ttl/var/../meta/../detail/../macro_misc.hpp \
  ../Common/Frame_m.h
EthernetBus.o: EthernetBus.cc \
  Bus.h \
  Utils.h \
  ../../Common/Util.h \
  BusLogging.h \
  ../Ethernet/EthernetFrame_m.h \
  ../../Common/Logging.h \
  ../../Common/../ttl/var/variant.hpp \
  ../../Common/../ttl/var/../meta/typelist.hpp \
  ../../Common/../ttl/var/../meta/is_reference.hpp \
  ../../Common/../ttl/var/../meta/is_pointer.hpp \
  ../../Common/../ttl/var/../meta/is_same.hpp \
  ../../Common/../ttl/var/../meta/../config.hpp \
  ../../Common/../ttl/var/../meta/../data_holder.hpp \
  ../../Common/../ttl/var/../meta/../exception.hpp \
  ../../Common/../ttl/var/../meta/../macro_params.hpp \
  ../../Common/../ttl/var/../meta/../equivalent_types.hpp \
  ../../Common/../ttl/var/../meta/../selector.hpp \
  ../../Common/../ttl/var/../meta/../macro_repeat.hpp \
  ../../Common/../ttl/var/../meta/../macro_misc.hpp \
  ../../Common/../ttl/var/../meta/../detail/macro_counter.hpp \
  ../../Common/../ttl/var/../meta/../detail/../macro_misc.hpp \
  ../Common/Frame_m.h
LonTalkBus.o: LonTalkBus.cc \
  Bus.h \
  Utils.h \
  ../../Common/Util.h \
  BusLogging.h \
  ../Ethernet/EthernetFrame_m.h \
  ../../Common/Logging.h \
  ../../Common/../ttl/var/variant.hpp \
  ../../Common/../ttl/var/../meta/typelist.hpp \
  ../../Common/../ttl/var/../meta/is_reference.hpp \
  ../../Common/../ttl/var/../meta/is_pointer.hpp \
  ../../Common/../ttl/var/../meta/is_same.hpp \
  ../../Common/../ttl/var/../meta/../config.hpp \
  ../../Common/../ttl/var/../meta/../data_holder.hpp \
  ../../Common/../ttl/var/../meta/../exception.hpp \
  ../../Common/../ttl/var/../meta/../macro_params.hpp \
  ../../Common/../ttl/var/../meta/../equivalent_types.hpp \
  ../../Common/../ttl/var/../meta/../selector.hpp \
  ../../Common/../ttl/var/../meta/../macro_repeat.hpp \
  ../../Common/../ttl/var/../meta/../macro_misc.hpp \
  ../../Common/../ttl/var/../meta/../detail/macro_counter.hpp \
  ../../Common/../ttl/var/../meta/../detail/../macro_misc.hpp \
  ../Common/Frame_m.h
Utils.o: Utils.cc \
  Utils.h \
  Bus.h \
  ../../Common/Util.h \
  BusLogging.h \
  ../Ethernet/EthernetFrame_m.h \
  ../../Common/Logging.h \
  ../../Common/../ttl/var/variant.hpp \
  ../../Common/../ttl/var/../meta/typelist.hpp \
  ../../Common/../ttl/var/../meta/is_reference.hpp \
  ../../Common/../ttl/var/../meta/is_pointer.hpp \
  ../../Common/../ttl/var/../meta/is_same.hpp \
  ../../Common/../ttl/var/../meta/../config.hpp \
  ../../Common/../ttl/var/../meta/../data_holder.hpp \
  ../../Common/../ttl/var/../meta/../exception.hpp \
  ../../Common/../ttl/var/../meta/../macro_params.hpp \
  ../../Common/../ttl/var/../meta/../equivalent_types.hpp \
  ../../Common/../ttl/var/../meta/../selector.hpp \
  ../../Common/../ttl/var/../meta/../macro_repeat.hpp \
  ../../Common/../ttl/var/../meta/../macro_misc.hpp \
  ../../Common/../ttl/var/../meta/../detail/macro_counter.hpp \
  ../../Common/../ttl/var/../meta/../detail/../macro_misc.hpp \
  ../Common/Frame_m.h
