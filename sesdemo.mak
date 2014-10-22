# Created by IBM WorkFrame/2 MakeMake at 23:36:59 on 02/22/96
#
# This makefile should be run in the following directory:
#   H:\Projekte\SESDemo V1.00
#
# The actions included in this makefile are:
#   BIND::Resource Bind
#   COMPILE::CLC C++
#   COMPILE::Resource Compile
#   INSPECT::Map Symbols
#   LINK::Link

.all: \
  .\SESDEMO.EXE \
  .\SESDemo.sym

.SUFFIXES:

.SUFFIXES: .MAP .C .RC

.RC.res:
      @echo WF::COMPILE::Resource Compile
      rc.exe -r %s %|fF.RES

.C.obj:
      @echo WF::COMPILE::CLC C++
      icc.exe /IH:\SECPACK\SECURITY\DEV\H /Ss /Q /Wclscmpcndcnscnvcpydcleffenuextgengnrgotinilanobsordparporppcpptprorearettrdtruundusevft /Ti /W2 /Gm /G4 /C %s

.MAP.sym:
      @echo WF::INSPECT::Map Symbols
      mapsym.exe %s

.\SESDEMO.EXE: \
    .\SESDemo.obj \
    .\SESDemo.res \
    {$(LIB)}SESDEMO.def \
    SESDEMO.MAK
      @echo WF::LINK::Link
      link386.exe @<<
         /PM:PM /DE /NOI /NOLOGO /M: +
        .\SESDemo.obj
        SESDEMO.EXE
        
        
        SESDEMO;
<<
      @echo WF::BIND::Resource Bind
      rc.exe .\SESDemo.res SESDEMO.EXE

.\SESDemo.res: \
    "H:\Projekte\SESDemo V1.00\SESDemo.RC" \
    {$(INCLUDE)}sesdemo.h \
    SESDEMO.MAK

.\SESDemo.obj: \
    "H:\Projekte\SESDemo V1.00\SESDemo.C" \
    {"H:\Projekte\SESDemo V1.00;H:\SECPACK\SECURITY\DEV\H;$(INCLUDE);"}sesdemo.h \
    SESDEMO.MAK

.\SESDemo.sym: \
    "H:\Projekte\SESDemo V1.00\SESDemo.MAP" \
    SESDEMO.MAK

