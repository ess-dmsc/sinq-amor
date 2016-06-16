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

iocInit

## Start any sequence programs
#seq sncxxx,"user=koenneckeHost"
