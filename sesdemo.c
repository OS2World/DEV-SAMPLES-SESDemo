/***************************************************************************/
/*                                                                         */
/* Modul: SESDemo.C                                                        */
/* Autor: Michael Schmidt                                                  */
/* Datum: 26.02.1996                                                       */
/*                                                                         */
/***************************************************************************/

#define INCL_BASE
#define INCL_PM

#include <os2.h>                            /* OS/2-Deklarationen          */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <builtin.h>


#define INCL_SCSAPI
#define INCL_LSSAPI
#include <sesapi.h>

#include "sesdemo.h"                        /* Konstanten                  */


/*---- Deklarationen ------------------------------------------------------*/
typedef struct _MYSESLOGON                  /* fr Dialogparameter         */
{
   USHORT usStructSize;                     /* Gr”áe dieser Struktur       */
   PSESLOGON pSESLogon;                     /* SES Logon-Struktur          */
}
   MYSESLOGON, *PMYSESLOGON;


/*---- globale Variablen --------------------------------------------------*/

/*---- modulglobale Variablen ---------------------------------------------*/
static HWND hwndMainFrame;                  /* Handle fr Frame Window     */
static HWND hwndMain;                       /* Handle fr Main Window      */


/*---- Funktionsprototypen ------------------------------------------------*/
static MRESULT EXPENTRY MainWindowProc(HWND hwnd, USHORT msg, MPARAM mp1, 
   MPARAM mp2);
static VOID MainCommand(HWND hwnd, USHORT msg, MPARAM mp1, MPARAM mp2);
static VOID SESEventHandler(PVOID pvParm);
static VOID SESEventStarter(PVOID pvParm);
static MRESULT EXPENTRY LogonDlgProc(HWND hwnd, USHORT msg, MPARAM mp1,
   MPARAM mp2);
static VOID CenterDialog(HWND hwnd);



/***************************************************************************/
/*                                                                         */
/* Funktion: main                                                          */
/*                                                                         */
/* Der PM-Hauptthread ist in Hinblick auf SES weitgehend uninteressant.    */
/* Das einzige bemerkenswerte ist vielleicht, daá der Message-Dispatcher   */
/* nicht mit WM_QUIT abgebrochen werden kann, sondern in einer Endlos-     */
/* schleife l„uft. Dementsprechend ist der Aufruf von 'WinCancelShutdown'  */
/* erforderlich.                                                           */
/* Damit wird erreicht, daá das Programm auch den Shutdown berlebt.       */
/*                                                                         */
/***************************************************************************/

INT main 
(
   VOID
)

{
   ULONG ulCtlData;                         /* Dummy                       */

   HAB hab = WinInitialize(0);              /* PM initialisieren           */
   HMQ hmq = WinCreateMsgQueue(hab, 0);     /* Message Queue aufbauen      */

   QMSG qmsg;                               /* Message Struktur            */


   WinCancelShutdown(hmq, TRUE);            /* wir berleben Shutdown      */


   /*---- nun wird das Main Window erstellt -------------------------------*/
   ulCtlData = FCF_SIZEBORDER | FCF_MENU | FCF_ACCELTABLE | FCF_TASKLIST;

   WinRegisterClass(hab, "SESDemo",(PFNWP)MainWindowProc,
      CS_SIZEREDRAW | CS_CLIPCHILDREN, 0);

   hwndMainFrame = WinCreateStdWindow(HWND_DESKTOP, 0, &ulCtlData,
      "SESDemo", "SES-Steuerung", 0, NULLHANDLE, ID_MAIN, &hwndMain);

   WinSetWindowPos(hwndMainFrame, NULLHANDLE, 0, 10, 0, 10,
      SWP_ACTIVATE | SWP_SIZE);             /* Fenster positionieren       */


   /*- Message Dispatcher aufbauen ----------------------------------------*/
   for (;;)                                 /* Endlosschleife              */
   {
      WinGetMsg(hab, &qmsg, 0, 0, 0);
      WinDispatchMsg(hab, &qmsg);
   }


   /*---- die folgenden Funktionen sollten nie erreicht werden. -----------*/
   WinDestroyMsgQueue(hmq);                 /* PM abbauen                  */
   WinTerminate(hab);

   return 0;                                /* Compiler zufriedenstellen   */
} 


/***************************************************************************/
/*                                                                         */
/* Funktion: MainWindowProc                                                */
/*                                                                         */
/* Hierin wird der SES-Eventhandler-Thread gestartet und es werden die     */
/* WM_USER-Nachrichten an 'MainCommand' aussortiert.                       */
/*                                                                         */
/***************************************************************************/

static MRESULT EXPENTRY MainWindowProc      /* Hauptfensterfunktion        */
(
   HWND hwnd,                               /* Fensterhandle               */
   USHORT msg,                              /* Message                     */
   MPARAM mp1,                              /* Messageparameter            */
   MPARAM mp2
)

{
   switch (msg)                             /* welche Message?             */
   {
      case WM_CREATE:                       /* Client Window erzeugen      */
         _beginthread(SESEventHandler, 0, 0x8000, NULL);
         break;

      case WM_COMMAND:                      /* Kommandofunktion ausl”sen   */
         MainCommand(hwnd, msg, mp1, mp2);
         break;

      default:
         return WinDefWindowProc(hwnd, msg, mp1, mp2);
   }

   return (MRESULT)0; 
} 


/***************************************************************************/
/*                                                                         */
/* Funktion: MainCommand                                                   */
/*                                                                         */
/* Hierin werden im wesentlichen die Benutzeranforderungen fr LOGOFF und  */
/* SHUTDOWN  abgefangen und zur Bearbeitung an einen Worker-Thread weiter- */
/* geleitet. Ein direkter Aufruf von 'SESStartEvent' wrde zu einer        */
/* Verklemmung zwischen SES und PM und damit zum Systemstillstand fhren!  */
/*                                                                         */
/***************************************************************************/

static VOID MainCommand                     /* Kommandofunktion bearbeiten */
(
   HWND hwnd,                               /* Fensterhandle               */
   USHORT msg,                              /* Message                     */
   MPARAM mp1,                              /* Messageparameter            */
   MPARAM mp2
)

{
   PSESSTARTEVENT pSES_StartEvent;          /* SES-Ereignisstruktur        */


   switch (SHORT1FROMMP(mp1))               /* welcher Menpunkt?          */
   {
      case ID_ABOUT:                        /* Copyright ausgeben          */
         WinMessageBox                      
         (
            HWND_DESKTOP,
            hwndMainFrame,
            "SESDemo\n\n'OS/2 Inside' SES-Demoprogramm\n\n"
            "Copyright (C) 1996 by 'OS/2 Inside' and Michael Schmidt",
            "",
            0,
            MB_MOVEABLE
         );

         break;

     case ID_HIDE:                          /* Dialog verstecken           */
         WinShowWindow(hwndMainFrame, FALSE);

         break;

     case ID_LOGOFF:                        /* SES-Logoff ausl”sen         */
         pSES_StartEvent = calloc(1, sizeof (SESSTARTEVENT));
         pSES_StartEvent->Event= SES_EVENT_LOGOFF;
         pSES_StartEvent->EventStatus = SES_STATUS_NO_ERROR;

         _beginthread(SESEventStarter, 0, 0x8000, pSES_StartEvent);

         break;

     case ID_SHUTDOWN:                      /* SES-Shutdown ausl”sen       */
         pSES_StartEvent = calloc(1, sizeof (SESSTARTEVENT));
         pSES_StartEvent->Event= SES_EVENT_SHUTDOWN;
         pSES_StartEvent->EventStatus = SES_STATUS_NO_ERROR;

         _beginthread(SESEventStarter, 0, 0x8000, pSES_StartEvent);

         break;

      default:                              /* sonstige Kommandos          */
         WinDefWindowProc(hwnd, msg, mp1, mp2);
   }
} 


/***************************************************************************/
/*                                                                         */
/* Funktion: SESEventHandler                                               */
/*                                                                         */
/* Dieser Thread ist fr die Bearbeitung aller SES-Events verantwortlich,  */
/* fr die er sich registriert hat. Es ist unbedingt erforderlich, daá     */
/* sich die SLA fr das SES_EVENT_PROCESS_CREATION registriert, und dieses */
/* OHNE weitere Verz”gerung (Blockieren) best„tigt. Jedes Blockieren       */
/* k”nnte zu einer Verklemmung und damit zum Systemstillstand fhren!      */
/*                                                                         */
/* Der Aufruf der Funktion 'SESWaitEvent' fhrt zum Blockieren des Threads */
/* im SES-Treiber, bis er durch das Auftreten eines SES-Events wieder      */
/* 'freigelassen' wird. Jedes SES-Event muá mit 'SESReturnEventStatus'     */
/* best„tigt werden.                                                       */
/*                                                                         */
/***************************************************************************/

static VOID SESEventHandler
(
   PVOID pvParm                             /* unbenutzt                   */
)

{
   ULONG ulDaemonID, ulEventList;           /* SES-Parameter               */

   SESEVENT SESEvent;                       /* SES-Ereignisstruktur        */

   MYSESLOGON MySESLogon;                   /* brauchen wir fr Dialog     */

   APIRET RetCode;


   HAB hab = WinInitialize(0);              /* PM initialisieren           */
   HMQ hmq = WinCreateMsgQueue(hab, 0);     /* Message Queue aufbauen      */


   WinCancelShutdown(hmq, TRUE);            /* wir berleben Shutdown      */


   /*---- Registrierung fr Prozeáerzeugung, Logon. Logoff, Shutdown ------*/
   ulEventList = SES_EVENT_PROCESS_CREATION | 
                 SES_EVENT_LOGON            | 
                 SES_EVENT_LOGOFF           |
                 SES_EVENT_SHUTDOWN;

   RetCode = SESRegisterDaemon(&ulDaemonID, ulEventList);
   if (RetCode != NO_ERROR)
   {
      _interrupt(3);

      DosBeep(200, 100);
   }


   for (;;)                                 /* Endlosschleife              */
   {
      memset(&SESEvent, 0, sizeof (SESEvent));
                                            /* Initialisierung             */

      SESEvent.DaemonID = ulDaemonID;       /* Daemon-ID setzen            */

      /*---- wir kehren erst beim n„chsten SES-Event zurck ---------------*/
      RetCode = SESWaitEvent(&SESEvent, (ULONG)SES_INDEFINITE_WAIT);
      if (RetCode != NO_ERROR)
      {
         _interrupt(3);

         DosBeep(200, 100);
      }


      switch (SESEvent.Event)               /* welches Ereignis?           */
      {  
         case SES_EVENT_LOGON_UIA:          /* Logon-Authentisierung       */

            /*---- nun mssen wir den Logondialog starten -----------------*/

            MySESLogon.usStructSize = sizeof (MySESLogon);
            MySESLogon.pSESLogon = &SESEvent.EventData.Logon;

            WinDlgBox(HWND_DESKTOP, hwndMainFrame, (PFNWP)LogonDlgProc,  
               NULLHANDLE, ID_LOGON, &MySESLogon);
                  

            if (SESEvent.EventData.Logon.UserNameLen == 0)
            {
               /*---- jetzt sagen wir 'mal, Authentisierung ungltig ------*/
               DosBeep(500, 500);
               SESEvent.EventStatus = SES_STATUS_USER_UNAUTHENTICATED;
            }

            else if (SESEvent.EventData.Logon.UserTokenLen == 0)
            {
               /*---- jetzt sagen wir 'mal, Authentisierung ungltig ------*/
               DosBeep(500, 500);
               SESEvent.EventStatus = SES_STATUS_USER_UNAUTHENTICATED;
            }
            else
            {
               SESEvent.EventStatus = SES_STATUS_USER_AUTHENTICATED;
            }
         
            break;


         case SES_EVENT_LOGON_SLA:          /* WPS-Logon best„tigen        */
            if (SESEvent.EventStatus == SES_STATUS_USER_AUTHENTICATED)
            {
               WinShowWindow(hwndMainFrame, TRUE);
  
               SESEvent.EventStatus = SES_STATUS_NO_ERROR;
            } 
            else 
               SESEvent.EventStatus = SES_STATUS_EVENT_FAILURE;

            break;
 

         case SES_EVENT_LOGOFF_SLA:         /* WPS-Logoff best„tigen       */
         case SES_EVENT_SHUTDOWN_SLA:       /* WPS-Shutdown best„tigen     */

            WinShowWindow(hwndMainFrame, FALSE);

            SESEvent.EventStatus = SES_STATUS_NO_ERROR;

            break;
 

         case SES_EVENT_PROCESS_CREATION:   /* PROCESS_CREATION best„tigen */
         case SES_EVENT_LOGOFF_QUERY:       /* WPS-Abbau best„tigen        */
         case SES_EVENT_SHUTDOWN_QUERY:     /* WPS-Shutdown best„tigen     */
            SESEvent.EventStatus = SES_STATUS_NO_ERROR;

            break;


         default:                           /* dafr sind wir nicht reg.   */

            _interrupt(3);
            DosBeep(200, 100);

            break;
      }


      SESEvent.DaemonID = ulDaemonID;       /* Daemon-ID setzen            */

      RetCode = SESReturnEventStatus(&SESEvent);
      if ((RetCode != NO_ERROR) && (RetCode != SES_EVENT_FAILURE))
      {
         _interrupt(3);
         DosBeep(200, 100);
      }
   }                                        /* ...Endlosschleife           */


   WinDestroyMsgQueue(hmq);                 /* PM abbauen                  */
   WinTerminate(hab);
}


/***************************************************************************/
/*                                                                         */
/* Funktion: SESEventStarter                                               */
/*                                                                         */
/* Diese Funktion l„uft als eigenst„ndiger Thread und startet ein          */
/* SES-Event. Die Funktion 'SESStartEvent' kehrt dabei erst zum Aufrufer   */
/* zurck, nachdem ALLE registrierten Event-Handler das Event vollst„ndig  */
/* bearbeitet haben.                                                       */
/* Die Funktion sollte nicht reentrant aufgerufen werden, da der zweite    */
/* (und jeder weitere) reentrante Aufruf von 'SESStartEvent' i. A.         */
/* fehlschl„gt.                                                            */
/*                                                                         */
/***************************************************************************/

static VOID SESEventStarter
(
   PVOID pvParm                             /* Zeiger auf SESSTARTEVENT    */
)

{
   PSESSTARTEVENT pSES_StartEvent = (PSESSTARTEVENT)pvParm;
                                            /* Zeiger auf Event holen      */

   APIRET RetCode = SESStartEvent(pSES_StartEvent);
   if ((RetCode != NO_ERROR) && (RetCode != SES_EVENT_INVALID))
   {
      _interrupt(3);

      DosBeep(200, 100);
   }


   free(pSES_StartEvent);
}


/***************************************************************************/
/*                                                                         */
/* Funktion: LogonDlgProc                                                  */
/*                                                                         */
/* Dies ist der Anmeldedialog. Er nimmt den Benutzernamen und das Paáwort  */
/* entgegen.                                                               */
/*                                                                         */
/***************************************************************************/

static MRESULT EXPENTRY LogonDlgProc        /* Anmeldedialog               */
(
   HWND hwnd,                               /* Fensterhandle               */
   USHORT msg,                              /* Message                     */
   MPARAM mp1,                              /* Messageparameter            */
   MPARAM mp2
)

{
   HWND hwndUserID, hwndPassword;           /* Fensterhdl. Eingabefelder   */

   static PSESLOGON pSESLogon;              /* Zeiger auf Logon-Struktur   */


   switch (msg)                             /* welche Message?             */
   {
      case WM_INITDLG:                      /* Dialoginitialisierung       */
         CenterDialog(hwnd);                /* Dialog zentrieren           */

         pSESLogon = ((PMYSESLOGON)mp2)->pSESLogon;
                                            /* Zeiger setzen               */

         break;

      case WM_COMMAND:                      /* Dialogkommando              */
         switch (SHORT1FROMMP(mp1))         
         {
            case ID_OK:                     /* OK-Taste gedrckt           */
               hwndUserID = WinWindowFromID(hwnd, ID_USERENTRY);
               hwndPassword = WinWindowFromID(hwnd, ID_PASSWORDENTRY);
                                            /* Fensterhandles setzen       */

               /*---- Benutzerkennung und Paáwort auslesen ----------------*/
               pSESLogon->UserNameLen = 
                  (ULONG)WinQueryWindowText(hwndUserID, MAX_USER_NAME, 
                  pSESLogon->UserName);
               pSESLogon->UserTokenLen = 
                  (ULONG)WinQueryWindowText(hwndPassword, 
                  MAX_TOKEN, pSESLogon->UserToken);


               WinDismissDlg(hwnd, TRUE);   /* Dialogbox abbauen           */
               break;

            default:
               break;
         }
      
         break;

      case WM_CONTROL:                      /* Dialogkommando              */
      
         break;

      default:
         return WinDefDlgProc(hwnd, msg, mp1, mp2);
   }

   return (MRESULT)0; 
} 


/***************************************************************************/
/*                                                                         */
/* Funktion: CenterDialog                                                  */
/*                                                                         */
/***************************************************************************/

static VOID CenterDialog
(
   HWND hwnd                                /* Fensterhandle               */
)

{
   RECTL rclScreen,rclDialog;
   LONG  sWidth,sHeight,sBLCx,sBLCy;
 
   WinQueryWindowRect(HWND_DESKTOP, &rclScreen);
   WinQueryWindowRect(hwnd, &rclDialog);
 
   sWidth = (LONG)(rclDialog.xRight - rclDialog.xLeft);
   sHeight = (LONG)(rclDialog.yTop - rclDialog.yBottom);
 
   sBLCx = ((LONG)rclScreen.xRight - sWidth) / 2;
   sBLCy = ((LONG)rclScreen.yTop - sHeight) / 2;
 
   WinSetWindowPos(hwnd, HWND_TOP, sBLCx, sBLCy, 0, 0, SWP_MOVE);
}



