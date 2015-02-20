// teleinfo
#include <SoftwareSerial.h>
#include "teleInfo.h"
#define TI_RX 4
teleInfo TI( TI_RX );

// longueur max des données qu'on reçoit
#define BUFSIZE 15

void setup() {
	Serial.begin( 115200 );
}

// a shortcut
#define PRINT_TI( label, value ) { Serial.print( label ); Serial.print( F(" has a value of ") ); Serial.println( value ); }

void loop() {
	teleInfo_t currentTI;
	
	// read téléinfo
	currentTI = TI.get();

	// gestion mySensor, vérifier si la value à changée et envoyer le message à la gateway si c'est le cas
	print( F( "ADCO" ), currentTI.ADCO );
	PRINT_TI( F( "OPTARIF" ), currentTI.OPTARIF );
	PRINT_TI( F( "ISOUSC" ), currentTI.ISOUSC );
	PRINT_TI( F( "PTEC" ), currentTI.PTEC );

	PRINT_TI( F( "IINST" ), currentTI.IINST );
	PRINT_TI( F( "ADPS" ), currentTI.ADPS );
	PRINT_TI( F( "IMAX" ), currentTI.IMAX );
	PRINT_TI( F( "PAPP" ), currentTI.PAPP );

	PRINT_TI( F( "BASE" ), currentTI.BASE );

	PRINT_TI( F( "HC_HC" ), currentTI.HC_HC );
	PRINT_TI( F( "HC_HP" ), currentTI.HC_HP );

	PRINT_TI( F( "EJP_HN" ), currentTI.EJP_HN );
	PRINT_TI( F( "EJP_HPM" ), currentTI.EJP_HPM );
	PRINT_TI( F( "PEJP" ), currentTI.PEJP );

	PRINT_TI( F( "BBR_HC_JB" ), currentTI.BBR_HC_JB );
	PRINT_TI( F( "BBR_HP_JB" ), currentTI.BBR_HP_JB );
	PRINT_TI( F( "BBR_HC_JW" ), currentTI.BBR_HC_JW );
	PRINT_TI( F( "BBR_HP_JW" ), currentTI.BBR_HP_JW );
	PRINT_TI( F( "BBR_HC_JR" ), currentTI.BBR_HC_JR );
	PRINT_TI( F( "BBR_HP_JR" ), currentTI.BBR_HP_JR );
	PRINT_TI( F( "DEMAIN" ), currentTI.DEMAIN );

	PRINT_TI( F( "HHPHC" ), currentTI.HHPHC );

	delay( SLEEP_TIME );
}