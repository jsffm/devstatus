/*
 * A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#include "i18n.h"

#if APIVERSNUM < 10507

const tI18nPhrase Phrases[] = {
  { "recording status monitor",
    "Anzeige der laufenden Aufnahmen je Ger�t",// Deutsch
    "",// TODO Slovenski
    "Mostra le registrazioni attuali su ogni scheda",// Italian
    "",// TODO Nederlands
    "",// TODO Portugues
    "Afficher les enregistrements pour chaque carte",//French
    "",// TODO Norsk
    "tallennusten tilan seuranta",//Finnish
    "",// TODO Polski
    "Ver grabaciones actuales de cada dispositivo",// Espa�ol
    "",// TODO �������� (Greek)
    "",// TODO Svenska
    "",// TODO Romaneste
    "",// TODO Magyar
    "",// TODO Catal�
    "",// TODO ������� (Russian)
    "",// TODO Hrvatski (Croatian)
#if VDRVERSNUM && VDRVERSNUM >= 10313
    "", // Eesti
#if VDRVERSNUM >= 10316
    "", // Dansk
#endif
#endif
  },
  { "This plugin has no info text!",
    "Dieses Plugin hat keinen Infotext!",
    "",// TODO
    "Questo plugin non ha ulteriori informazioni!",
    "",// TODO
    "",// TODO
    "Le plugin n'a pas de text d'info",
    "",// TODO
    "Laajennoksella ei ole infoteksti�!",
    "",// TODO
    "�Este plugin no tiene informaci�n!",
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
#if VDRVERSNUM && VDRVERSNUM >= 10313
    "", // Eesti
#if VDRVERSNUM >= 10316
    "", // Dansk
#endif
#endif
  },
  { "Recording status",
    "Aufnahmestatus",
    "",// TODO
    "Stato registrazioni",
    "",// TODO
    "",// TODO
    "Status d'enregistrement",
    "",// TODO
    "Tallennusten tila",
    "",// TODO
    "Estado de la grabaci�n",
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
#if VDRVERSNUM && VDRVERSNUM >= 10313
    "", // Eesti
#if VDRVERSNUM >= 10316
    "", // Dansk
#endif
#endif
  },
  { "Device",
    "Ger�t",
    "",// TODO
    "Scheda",
    "",// TODO
    "",// TODO
    "Carte",
    "",// TODO
    "Kortti",
    "",// TODO
    "Dispositivo",
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
#if VDRVERSNUM && VDRVERSNUM >= 10313
    "", // Eesti
#if VDRVERSNUM >= 10316
    "", // Dansk
#endif
#endif
  },
  { "no current recordings",
    "keine laufenden Aufnahmen",
    "",// TODO
    "nessuna registrazione al momento",
    "",// TODO
    "",// TODO
    "pas d'enregistrement actuellement",
    "",// TODO
    "ei meneill��n olevia tallennuksia",
    "",// TODO
    "actualmente no hay grabaciones",
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
#if VDRVERSNUM && VDRVERSNUM >= 10313
    "", // Eesti
#if VDRVERSNUM >= 10316
    "", // Dansk
#endif
#endif
  },
  { "device with decoder",
    "Ger�t hat Decoder",
    "",// TODO
    "scheda con decoder",
    "",// TODO
    "",// TODO
    "La carte a un d�codeur",
    "",// TODO
    "dekooderi",
    "",// TODO
    "Decodificador",
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
#if VDRVERSNUM && VDRVERSNUM >= 10313
    "", // Eesti
#if VDRVERSNUM >= 10316
    "", // Dansk
#endif
#endif
  },
  { "primary device",
    "Prim�rger�t",
    "",// TODO
    "scheda primaria",
    "",// TODO
    "",// TODO
    "Carte primaire",
    "",// TODO
    "ensisijainen",
    "",// TODO
    "Dispositivo principal",
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
    "",// TODO
#if VDRVERSNUM && VDRVERSNUM >= 10313
    "", // Eesti
#if VDRVERSNUM >= 10316
    "", // Dansk
#endif
#endif
  },
  { NULL }
  };

#endif
