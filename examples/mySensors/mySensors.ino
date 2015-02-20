// uncomment to enable debuging using serial port @115200bps
//#define DEBUG_ENABLED

// durée entre 2 rafraichissements des données - en mili-secondes 
// j'ai un probleme, il met 3 fois plus de temps que ce que j'indique!
// testé sur un arduino pro mini 3.3v/8mhz
//#define SLEEP_TIME 20000UL
#define SLEEP_TIME 10000UL

// mysensors
#include <SPI.h>
#include <MySensor.h>

// teleinfo
#include <SoftwareSerial.h>
#include "teleInfo.h"
#define TI_RX 4
teleInfo TI( TI_RX );

// longueur max des données qu'on reçoit
#define BUFSIZE 15

MySensor gw;

// paramètres des données téléinfo
// - ne pas toucher :)
// pour mieux comprendre, allez plus bas j'ai ajouté les informations de la doc ERDF afin de s'y retrouver
///////////////////////////////////

// infos générales
///////////////////////////////////
#define CHILD_ID_ADCO 0
MyMessage msgVAR1( 0, V_VAR1 );

#define CHILD_ID_OPTARIF 1
MyMessage msgVAR2( 0, V_VAR2 );

#define CHILD_ID_ISOUSC 2
MyMessage msgCURRENT( 0, V_CURRENT );

#define CHILD_ID_PTEC 3
MyMessage msgVAR3( 0, V_VAR3 );

#define CHILD_ID_IINST 4
#define CHILD_ID_ADPS 5
#define CHILD_ID_IMAX 6
#define CHILD_ID_PAPP 7

MyMessage msgWATT( 0, V_WATT ); // pas vrai c'est des VA!

// infos tarif BASE
///////////////////////////////////
#define CHILD_ID_BASE 10
MyMessage msgKWH( 0, V_KWH ); // en fait c'est des WH

// infos tarif HC/HP
///////////////////////////////////
#define CHILD_ID_HC_HC 20
#define CHILD_ID_HC_HP 21

// infos EJP
#define CHILD_ID_EJP_HN 30
#define CHILD_ID_EJP_HPM 31
#define CHILD_ID_PEJP 32

// infos tarif BBR (tempo)
///////////////////////////////////
#define CHILD_ID_BBR_HC_JB 40
#define CHILD_ID_BBR_HP_JB 41
#define CHILD_ID_BBR_HC_JW 42
#define CHILD_ID_BBR_HP_JW 43
#define CHILD_ID_BBR_HC_JR 44
#define CHILD_ID_BBR_HP_JR 45
#define CHILD_ID_DEMAIN 46

MyMessage msgVAR5( 0, V_VAR5 );

// sert à EJP/BBR
///////////////////////////////////
#define CHILD_ID_HHPHC 50
MyMessage msgVAR4( 0, V_VAR4 );

#define TARIF_BASE	1
#define TARIF_HCHP	2
#define	TARIF_EJP	4
#define	TARIF_TEMPO	8

uint8_t tarif = 0;
teleInfo_t last; // dernière lecture téléinfo
void setup() {
#ifdef DEBUG_ENABLED
	Serial.begin( 115200 );
#endif

	gw.begin();
	gw.sendSketchInfo( "Teleinfo Sensor", TI.VERSION );

	// une première lecture va nous donner l'option tarifaire
	// on va s'en servir pour présenter uniquement les bons messages a la gateway
	teleInfo_t currentTI;
	currentTI = TI.get();

	// communs a tout le monde
	gw.present( CHILD_ID_ADCO, S_POWER );
	gw.present( CHILD_ID_OPTARIF, S_POWER );
	gw.present( CHILD_ID_ISOUSC, S_POWER );
	gw.present( CHILD_ID_PTEC, S_POWER );
	gw.present( CHILD_ID_IINST, S_POWER );
	gw.present( CHILD_ID_ADPS, S_POWER );
	gw.present( CHILD_ID_IMAX, S_POWER );
	gw.present( CHILD_ID_PAPP, S_POWER );

	if ( strcmp( currentTI.OPTARIF, "BASE" ) == 0 ) {
		gw.present( CHILD_ID_BASE, S_POWER );

		tarif = TARIF_BASE;
	} else if ( strcmp( currentTI.OPTARIF, "HC.." ) == 0 ) {
		gw.present( CHILD_ID_HC_HC, S_POWER );
		gw.present( CHILD_ID_HC_HP, S_POWER );

		tarif = TARIF_HCHP;
	} else if ( strcmp( currentTI.OPTARIF, "EJP." ) == 0 ) {
		gw.present( CHILD_ID_EJP_HN, S_POWER );
		gw.present( CHILD_ID_EJP_HPM, S_POWER );
		gw.present( CHILD_ID_PEJP, S_POWER );

		tarif = TARIF_EJP;
	} else if ( strstr( currentTI.OPTARIF, "BBR" ) != NULL ) { // ! c'est BBRx (x varie) d'ou un strstr et non strcmp
		gw.present( CHILD_ID_BBR_HC_JB, S_POWER );
		gw.present( CHILD_ID_BBR_HP_JB, S_POWER );
		gw.present( CHILD_ID_BBR_HC_JW, S_POWER );
		gw.present( CHILD_ID_BBR_HP_JW, S_POWER );
		gw.present( CHILD_ID_BBR_HC_JR, S_POWER );
		gw.present( CHILD_ID_BBR_HP_JR, S_POWER );
		gw.present( CHILD_ID_DEMAIN, S_POWER );

		tarif = TARIF_TEMPO;
	}

#ifdef DEBUG_ENABLED
	Serial.print( F( "TARIF IS " ) );
	if ( tarif == TARIF_BASE )
		Serial.println( F( "BASE" ) );
	else if ( tarif == TARIF_TEMPO )
		Serial.println( F( "TEMPO" ) );
	else if ( tarif == TARIF_EJP )
		Serial.println( F( "EJP" ) );
	else if ( tarif == TARIF_HCHP )
		Serial.println( F( "HCHP" ) );
	else
		Serial.print( F( "UNKNOWN" ) );
#endif

	// cas particulier, appartient a EJP et TEMPO
	if ( (tarif & TARIF_EJP) || (tarif & TARIF_TEMPO) ) {
		gw.present( CHILD_ID_HHPHC, S_POWER );
	}
}

// compare 2 valeurs, s'il y a eu un changement, le sauvegarde et l'envoi à la gateway
// NB label ne sert qu'au DEBUG
bool compareTI( const __FlashStringHelper *label, uint32_t value, uint32_t &last, MyMessage &msg, int SENSOR_ID ) {
	if ( last == value )
		return true;

#ifdef DEBUG_ENABLED
	Serial.print( label );
	Serial.print( F(" changed from ") );
	Serial.print( last );
	Serial.print( F(" to ") );
	Serial.println( value );
#endif

	last = value;
	gw.send( msg.setSensor( SENSOR_ID ).set( last ) );

	return true;
}
bool compareTI( const __FlashStringHelper *label, uint8_t value, uint8_t &last, MyMessage &msg, int SENSOR_ID ) {
	if ( last == value )
		return true;

#ifdef DEBUG_ENABLED
  Serial.print( label );
  Serial.print( F(" changed from ") );
  Serial.print( last );
  Serial.print( F(" to ") );
  Serial.println( value );
#endif

	last = value;
	gw.send( msg.setSensor( SENSOR_ID ).set( last ) );

	return true;
}
bool compareTI( const __FlashStringHelper *label, char value, char &last, MyMessage &msg, int SENSOR_ID ) {
	if ( last == value )
		return true;

#ifdef DEBUG_ENABLED
	Serial.print( label );
	Serial.print( F(" changed from ") );
	Serial.print( last );
	Serial.print( F(" to ") );
	Serial.println( value );
#endif

	last = value;
	gw.send( msg.setSensor( SENSOR_ID ).set( last ) );

	return true;
}
bool compareTI( const __FlashStringHelper *label, char *value, char *last, MyMessage &msg, int SENSOR_ID ) {
	if ( strcmp( last, value ) == 0 )
		return true;

#ifdef DEBUG_ENABLED
	Serial.print( label );
	Serial.print( F(" changed from ") );
	Serial.print( last );
	Serial.print( F(" to ") );
	Serial.println( value );
#endif

	memset( last, 0, BUFSIZE );
	strcpy( last, value );

	gw.send( msg.setSensor( SENSOR_ID ).set( last ) );

	return true;
}

void loop() {
	teleInfo_t currentTI;

	// Process incoming messages (like config from server)
	gw.process(); 
	
	// read téléinfo
	currentTI = TI.get();

	// gestion mySensor, vérifier si la value à changée et envoyer le message à la gateway si c'est le cas
	compareTI( F( "ADCO" ), currentTI.ADCO, last.ADCO, msgVAR1, CHILD_ID_ADCO );
	compareTI( F( "OPTARIF" ), currentTI.OPTARIF, last.OPTARIF, msgVAR2, CHILD_ID_OPTARIF );
	compareTI( F( "ISOUSC" ), currentTI.ISOUSC, last.ISOUSC, msgCURRENT, CHILD_ID_ISOUSC );
	compareTI( F( "PTEC" ), currentTI.PTEC, last.PTEC, msgVAR3, CHILD_ID_PTEC );

	compareTI( F( "IINST" ), currentTI.IINST, last.IINST, msgCURRENT, CHILD_ID_IINST );
	compareTI( F( "ADPS" ), currentTI.ADPS, last.ADPS, msgCURRENT, CHILD_ID_ADPS );
	compareTI( F( "IMAX" ), currentTI.IMAX, last.IMAX, msgCURRENT, CHILD_ID_IMAX );
	compareTI( F( "PAPP" ), currentTI.PAPP, last.PAPP, msgWATT, CHILD_ID_PAPP );

	if ( tarif == TARIF_BASE ) {
		compareTI( F( "BASE" ), currentTI.BASE, last.BASE, msgKWH, CHILD_ID_BASE );
	} else if ( tarif == TARIF_HCHP ) {
		compareTI( F( "HC_HC" ), currentTI.HC_HC, last.HC_HC, msgKWH, CHILD_ID_HC_HC );
		compareTI( F( "HC_HP" ), currentTI.HC_HP, last.HC_HP, msgKWH, CHILD_ID_HC_HP );
	} else if ( tarif == TARIF_EJP ) {
		compareTI( F( "EJP_HN" ), currentTI.EJP_HN, last.EJP_HN, msgKWH, CHILD_ID_EJP_HN );
		compareTI( F( "EJP_HPM" ), currentTI.EJP_HPM, last.EJP_HPM, msgKWH, CHILD_ID_EJP_HPM );
		compareTI( F( "PEJP" ), currentTI.PEJP, last.PEJP, msgKWH, CHILD_ID_PEJP );
	} else if ( tarif == TARIF_TEMPO ) {
		compareTI( F( "BBR_HC_JB" ), currentTI.BBR_HC_JB, last.BBR_HC_JB, msgKWH, CHILD_ID_BBR_HC_JB );
		compareTI( F( "BBR_HP_JB" ), currentTI.BBR_HP_JB, last.BBR_HP_JB, msgKWH, CHILD_ID_BBR_HP_JB );
		compareTI( F( "BBR_HC_JW" ), currentTI.BBR_HC_JW, last.BBR_HC_JW, msgKWH, CHILD_ID_BBR_HC_JW );
		compareTI( F( "BBR_HP_JW" ), currentTI.BBR_HP_JW, last.BBR_HP_JW, msgKWH, CHILD_ID_BBR_HP_JW );
		compareTI( F( "BBR_HC_JR" ), currentTI.BBR_HC_JR, last.BBR_HC_JR, msgKWH, CHILD_ID_BBR_HC_JR );
		compareTI( F( "BBR_HP_JR" ), currentTI.BBR_HP_JR, last.BBR_HP_JR, msgKWH, CHILD_ID_BBR_HP_JR );
		compareTI( F( "DEMAIN" ), currentTI.DEMAIN, last.DEMAIN, msgVAR5, CHILD_ID_DEMAIN );
	}

	if ( (tarif & TARIF_EJP) || (tarif & TARIF_TEMPO) ) {
		compareTI( F( "HHPHC" ), currentTI.HHPHC, last.HHPHC, msgVAR4, CHILD_ID_HHPHC );
	}

	delay( SLEEP_TIME );
	//gw.sleep( SLEEP_TIME );
}