AM_CPPFLAGS = -I$(top_srcdir)/src
LDADD = ../src/libuldaq.la

noinst_PROGRAMS = AIn AInScan AInScanWithEvents AInScanWithQueue AInScanWithTrigger AInScanIepe\
AOut AOutScan\
CIn CInScan CInScanWithEncoder\
DaqInScan DaqInScanWithTrigger\
DaqOutScan\
DIn DBitIn DInScan DOut DBitOut DOutScan\
TmrPulseOut\
TIn\
RemoteNetDiscovery

AIn_SOURCES = AIn.c utility.h
AInScan_SOURCES = AInScan.c
AInScanWithEvents_SOURCES = AInScanWithEvents.c
AInScanWithQueue_SOURCES = AInScanWithQueue.c
AInScanWithTrigger_SOURCES = AInScanWithTrigger.c
AInScanIepe_SOURCES = AInScanIepe.c
AOut_SOURCES = AOut.c
AOutScan_LDADD = $(LDADD) -lm
AOutScan_SOURCES = AOutScan.c
CIn_SOURCES = CIn.c
CInScan_SOURCES = CInScan.c
CInScanWithEncoder_SOURCES = CInScanWithEncoder.c
DaqInScan_SOURCES = DaqInScan.c
DaqInScanWithTrigger_SOURCES = DaqInScanWithTrigger.c
DaqOutScan_LDADD = $(LDADD) -lm
DaqOutScan_SOURCES = DaqOutScan.c
DIn_SOURCES = DIn.c
DBitIn_SOURCES = DBitIn.c
DInScan_SOURCES = DInScan.c
DOut_LDADD = $(LDADD) -lm
DOut_SOURCES = DOut.c
DBitOut_LDADD = $(LDADD) -lm
DBitOut_SOURCES = DBitOut.c
DOutScan_LDADD = $(LDADD) -lm
DOutScan_SOURCES = DOutScan.c
TmrPulseOut_SOURCES = TmrPulseOut.c
TIn_SOURCES = TIn.c
RemoteNetDiscovery_SOURCES = RemoteNetDiscovery.c






