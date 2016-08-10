#!/opt/amor/epics/sinqEPICS


cd /opt/amor/epics

< envPaths


## Register all support components
dbLoadDatabase "dbd/sinqEPICS.dbd"
dbLoadDatabase "dbd/sinq.dbd"
sinqEPICS_registerRecordDeviceDriver pdbbase


#---------- connect to controllers
drvAsynIPPortConfigure("serial1", "localhost:60001",0,0,0)
drvAsynIPPortConfigure("serial2", "localhost:60002",0,0,0)
drvAsynIPPortConfigure("serial3", "localhost:60003",0,0,0)
EL734CreateController("mota","serial1",12);
EL734CreateController("motb","serial2",12);
EL734CreateController("motc","serial3",12);

### Motors

dbLoadRecords("$(ASYN)/db/asynRecord.db","P=SQ:AMOR:,R=serial1,PORT=serial1,ADDR=0,OMAX=80,IMAX=80")
dbLoadRecords("$(ASYN)/db/asynRecord.db","P=SQ:AMOR:,R=serial2,PORT=serial1,ADDR=0,OMAX=80,IMAX=80")
dbLoadRecords("$(ASYN)/db/asynRecord.db","P=SQ:AMOR:,R=serial3,PORT=serial1,ADDR=0,OMAX=80,IMAX=80")

dbLoadTemplate "mota.substitutions"
dbLoadTemplate "motb.substitutions"
dbLoadTemplate "motc.substitutions"


#--------- load EL737 counter box
drvAsynIPPortConfigure("cter1","localhost:62000",0,0,0)
dbLoadRecords("$(ASYN)/db/asynRecord.db","P=SQ:AMOR:,R=cter1,PORT=cter1,ADDR=0,OMAX=80,IMAX=80")
dbLoadRecords("${TOP}/db/el737Record.db")

#asynSetTraceIOMask("cter1",0,2)

#----------- load Magnets
drvAsynIPPortConfigure("slsvme", "localhost:60066",0,0,0)
#drvAsynIPPortConfigure("slsvme", "localhost:8080",0,0,0)

dbLoadRecords("$(ASYN)/db/asynRecord.db","P=SQ:AMOR:,R=slsvme,PORT=slsvme,ADDR=0,OMAX=80,IMAX=80")

epicsEnvSet ("STREAM_PROTOCOL_PATH", "$(TOP)/db:.")

dbLoadRecords("$(TOP)/db/slsvme.db","PREFIX=SQ:AMOR:PBY:,NO=1")
dbLoadRecords("$(TOP)/db/slsvme.db","PREFIX=SQ:AMOR:FMA:,NO=2")
dbLoadRecords("$(TOP)/db/slsvme.db","PREFIX=SQ:AMOR:ABY:,NO=3")

#-------------- load SPS
drvAsynIPPortConfigure("sps1", "localhost:60077",0,0,0)
dbLoadRecords("$(ASYN)/db/asynRecord.db","P=SQ:AMOR:,R=spsdirect,PORT=sps1,ADDR=0,OMAX=80,IMAX=80")
dbLoadRecords("$(TOP)/db/spsamor.db","PREFIX=SQ:AMOR:SPS1:")

#------------- Load dimetix distance measurement device
drvAsynIPPortConfigure("dimetix", "localhost:60088",0,0,0)
dbLoadRecords("$(ASYN)/db/asynRecord.db","P=SQ:AMOR:,R=dimetixdirect,PORT=dimetix,ADDR=0,OMAX=80,IMAX=80")
dbLoadRecords("$(TOP)/db/dimetix.db","PREFIX=SQ:AMOR:DIMETIX:")



iocInit

## Start any sequence programs
#seq sncxxx,"user=koenneckeHost"
