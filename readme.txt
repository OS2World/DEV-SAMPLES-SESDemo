SESDemo - SES ('Security Enabling Services') Demo Programm Version 1.00

Dieses Programm demonstriert die in einem mit SES abgesicherten System typische
BenutzerfÅhrung mit Logon, Logoff... Das Programm besteht aus der Datei 
SESDemo.EXE.

Voraussetzung:

- OS/2 2.11 mit Fixpack Level >= 100 und SES installiert	oder
- OS/2 Warp mit Fixpack Level >= 16 und SES installiert


Installation:

- Installation von SES (inklusive entsprechendem Fixpack)

- Eintragen von "x:\OS2\SECURITY\SLA\SESDEMO.EXE /SCA='sla' /SLA /UIA /START" 
  in die Datei x:\OS2\SECURITY\SESDB\SECURE.SYS. DafÅr mu· der Eintrag von 
  PDFSLA.EXE entfernt werden. 

- ErgÑnzen der CONFIG.SYS um die folgenden EintrÑge:

  REM ==== SES ====
  BASEDEV=SESDD32.SYS
  CALL=x:\OS2\SECURITY\SES\SESSTART.EXE x:\OS2\SECURITY\SES\SESDMON.EXE
  PROTSHELL=x:\OS2\SECURITY\SES\SESSHELL.EXE
  SET RUNWORKPLACE=x:\OS2\SECURITY\SES\PSSDMON.EXE
  SET SESDBPATH=x:\OS2\SECURITY\SESDB
  SET AUTOGUEST=NO
  SET GUESTNAME=AUTOGUEST
  SET TRUSTEDPATH=NO
  SET USERSHELL=x:\OS2\PMSHELL.EXE
  SET RESTARTUSERSHELL=YES
  REM ==== SES Ende ====
  REM
  LIBPATH=C:\OS2\SECURITY\SES;...
  REM PROTSHELL=x:\OS2\PMSHELL.EXE
  REM SET RUNWORKPLACE=x:\OS2\PMSHELL.EXE



Fragen, Kritik und Anregungen bitte an:

Michael Schmidt
Nelkenstra·e 24
D-85774 Unterfîhring

CIS:	100637,1666
email:  100637.1666@compuserve.com


Bekannte Probleme:
Wird mittels des SES-MenÅs ein Systemabschlu· ausgelîst, bei dem Dialogboxen
mit der Frage nach Terminierung einzelner Programme erscheinen, und wird einer 
der Dialoge mit 'Nein' bestÑtigt (also der Systemabschlu· abgebrochen), so 
befindet sich das System in einem undefinierten Zustand. Ein Weiterarbeiten 
ist nicht mehr mîglich.


Versionsgeschichte:
11.03.1996      1.00    fÅr OS/2 Inside
