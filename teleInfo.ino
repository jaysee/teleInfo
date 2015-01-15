// mysensors
#include <SPI.h>
#include <MySensor.h>

// teleinfo
#include <SoftwareSerial.h>
#define TI_RX 4
#define TI_TX 5

// uncomment to enable debuging
#define DEBUG_ENABLED

// longueur max des données qu'on recoi
#define BUFSIZE 15

// durée entre 2 rafraichissements des données
#define SLEEP_TIME (10 * 1000)

// battery voltage calculator
// mmm la téléinfo emet un signal carré entre -25v et 25v a 1200bps, une diode, une capa, le regul du duino et hop non ?
#define BATTERY_VOLTAGE 9.0
#define BATTERY_SENSE_PIN A0
// pont diviseur de tension
#define VDIV_R1 100e3
#define VDIV_R2 10e3
// vdiv est la sensibilité de mon pont diviseur de tension
#define VDIV ( ( VDIV_R1 + VDIV_R2 ) / VDIV_R2 ) * 1.1 / 1023

MySensor gw;

// paramètres des données téléinfo
// - ne pas toucher :)
// pour mieux comprendre, allez plus bas j'ai ajouté les informations de la doc ERDF afin de s'y retrouver
///////////////////////////////////

// infos générales
///////////////////////////////////
#define CHILD_ID_ADCO 0
MyMessage msgADCO( CHILD_ID_ADCO, V_VAR1 );

#define CHILD_ID_OPTARIF 1
MyMessage msgOPTARIF( CHILD_ID_OPTARIF, V_VAR2 );

#define CHILD_ID_ISOUSC 2
MyMessage msgISOUSC( CHILD_ID_ISOUSC, V_CURRENT );

#define CHILD_ID_PTEC 3
MyMessage msgPTEC( CHILD_ID_PTEC, V_VAR3 );

#define CHILD_ID_IINST 4
MyMessage msgIINST( CHILD_ID_IINST, V_CURRENT );

#define CHILD_ID_ADPS 5
MyMessage msgADPS( CHILD_ID_ADPS, V_KWH );

#define CHILD_ID_IMAX 6
MyMessage msgIMAX( CHILD_ID_IMAX, V_CURRENT );

#define CHILD_ID_PAPP 7
MyMessage msgPAPP( CHILD_ID_PAPP, V_WATT );

// infos tarif BASE
///////////////////////////////////
#define CHILD_ID_BASE 10
MyMessage msgBASE( CHILD_ID_BASE, V_KWH );

// infos tarif HC/HP
///////////////////////////////////
#define CHILD_ID_HC_HC 20
MyMessage msgHC_HC( CHILD_ID_HC_HC, V_KWH );

#define CHILD_ID_HC_HP 21
MyMessage msgHC_HP( CHILD_ID_HC_HP, V_KWH) ;

// infos EJP
#define CHILD_ID_EJP_HN 30
MyMessage msgEJP_HN( CHILD_ID_EJP_HN, V_KWH );

#define CHILD_ID_EJP_HPM 31
MyMessage msgEJP_HPM( CHILD_ID_EJP_HPM, V_KWH );

#define CHILD_ID_PEJP 32
MyMessage msgPEJP( CHILD_ID_PEJP, V_KWH );

// infos tarif BBR (tempo)
///////////////////////////////////
#define CHILD_ID_BBR_HC_JB 40
MyMessage msgBBR_HC_JB( CHILD_ID_BBR_HC_JB, V_KWH );

#define CHILD_ID_BBR_HP_JB 41
MyMessage msgBBR_HP_JB( CHILD_ID_BBR_HP_JB, V_KWH );

#define CHILD_ID_BBR_HC_JW 42
MyMessage msgBBR_HC_JW( CHILD_ID_BBR_HC_JW, V_KWH );

#define CHILD_ID_BBR_HP_JW 43
MyMessage msgBBR_HP_JW( CHILD_ID_BBR_HP_JW, V_KWH );

#define CHILD_ID_BBR_HC_JR 44
MyMessage msgBBR_HC_JR( CHILD_ID_BBR_HC_JR, V_KWH );

#define CHILD_ID_BBR_HP_JR 45
MyMessage msgBBR_HP_JR( CHILD_ID_BBR_HP_JR, V_KWH );

#define CHILD_ID_DEMAIN 46
MyMessage msgDEMAIN( CHILD_ID_DEMAIN, V_KWH );

// sert à EJP/BBR
///////////////////////////////////
#define CHILD_ID_HHPHC 50
MyMessage msgHHPHC( CHILD_ID_HHPHC, V_VAR4 );

// connexion série avec le compteur EDF
SoftwareSerial tiSerial( TI_RX, TI_RX );

void setup() {
	// reduce clock speed (reduce power consumption)
	CLKPR = (1<<CLKPCE);
	CLKPR = B00000011; // should put the clock at 1Mhz

#ifdef DEBUG_ENABLED
	Serial.begin( 115200 );
#endif

	tiSerial.begin( 1200 );

	// use the 1.1 V internal reference
	analogReference( INTERNAL );

	gw.begin();
	gw.sendSketchInfo( "Teleinfo Sensor", "0.1" );

	gw.present( CHILD_ID_ADCO, S_POWER );
	gw.present( CHILD_ID_OPTARIF, S_POWER );
	gw.present( CHILD_ID_ISOUSC, S_POWER );
	gw.present( CHILD_ID_PTEC, S_POWER );
	gw.present( CHILD_ID_IINST, S_POWER );
	gw.present( CHILD_ID_ADPS, S_POWER );
	gw.present( CHILD_ID_IMAX, S_POWER );
	gw.present( CHILD_ID_PAPP, S_POWER );

	gw.present( CHILD_ID_BASE, S_POWER );

	gw.present( CHILD_ID_HC_HC, S_POWER );
	gw.present( CHILD_ID_HC_HP, S_POWER );

	gw.present( CHILD_ID_EJP_HN, S_POWER );
	gw.present( CHILD_ID_EJP_HPM, S_POWER );
	gw.present( CHILD_ID_PEJP, S_POWER );

	gw.present( CHILD_ID_BBR_HC_JB, S_POWER );
	gw.present( CHILD_ID_BBR_HP_JB, S_POWER );
	gw.present( CHILD_ID_BBR_HC_JW, S_POWER );
	gw.present( CHILD_ID_BBR_HP_JW, S_POWER );
	gw.present( CHILD_ID_BBR_HC_JR, S_POWER );
	gw.present( CHILD_ID_BBR_HP_JR, S_POWER );
	gw.present( CHILD_ID_DEMAIN, S_POWER );

	gw.present( CHILD_ID_HHPHC, S_POWER );
}

/*
void readBatteryLevel( MySensor &mygw ) {
	static int oldBatteryPcnt = 0;

	// get the battery Voltage
	int sensorValue = analogRead( BATTERY_SENSE_PIN );

#ifdef DEBUG_ENABLED
	Serial.print( F("Sensor Value: ") );
	Serial.println(sensorValue);
#endif

	float batteryV = sensorValue * VDIV;

#ifdef DEBUG_ENABLED
	Serial.print( F("Battery Voltage: ") );
	Serial.print( batteryV );
	Serial.println( F(" V") );
#endif

	int batteryPcnt = batteryV * 100 / BATTERY_VOLTAGE;
	if (batteryPcnt > 100)
		batteryPcnt = 100;

#ifdef DEBUG_ENABLED
	Serial.print( F("Battery percent: ") );
	Serial.print(batteryPcnt);
	Serial.println( F(" %") );
#endif

	if ( oldBatteryPcnt != batteryPcnt ) {
		// Power up radio after sleep
		mygw.sendBatteryLevel( batteryPcnt );
		oldBatteryPcnt = batteryPcnt;
	}
}
*/

typedef struct TeleInfo TeleInfo;
struct TeleInfo {
	// DOC ERDF - http://www.magdiblog.fr/wp-content/uploads/2014/09/ERDF-NOI-CPT_02E.pdf

	char ADCO[12]; //Identifiant du compteur - m'en fous
	char OPTARIF[4]; //Option tarifaire (type d’abonnement) - m'en fous
	//     juste pour memoire, les values
	//     BASE : option base
	//     HC.. : option heure creuse
	//     EJP. : option EJP
	//     BBRx : option tempo, x est un char qui indique kkchose (relooooooou les mecs)
	uint8_t ISOUSC; // : Intensité souscrite 2chars en Ampères - m'en fous
 	uint32_t BASE; // Index si option = base (en Wh)

	uint32_t HC_HC; // Index heures creuses si option = heures creuses (en Wh)
	uint32_t HC_HP; // Index heures pleines si option = heures creuses (en Wh)

	uint32_t EJP_HN; // Index heures normales si option = EJP (en Wh)
	uint32_t EJP_HPM; // Index heures de pointe mobile si option = EJP (en Wh)

	uint32_t BBR_HC_JB; // Index heures creuses jours bleus si option = tempo (en Wh)
	uint32_t BBR_HP_JB; // Index heures pleines jours bleus si option = tempo (en Wh)
	uint32_t BBR_HC_JW; // Index heures creuses jours blancs si option = tempo (en Wh)
	uint32_t BBR_HP_JW; // Index heures pleines jours blancs si option = tempo (en Wh)
	uint32_t BBR_HC_JR; // Index heures creuses jours rouges si option = tempo  (en Wh)
	uint32_t BBR_HP_JR; // Index heures pleines jours rouges si option = tempo (en Wh)

	uint8_t  PEJP; // : Préavis EJP si option = EJP 30mn avant période EJP, en minutes

	char PTEC[4]; // Période tarifaire en cours
	// les valeurs de PTEC :
	// - TH.. => Toutes les Heures.
	// - HC.. => Heures Creuses.
	// - HP.. => Heures Pleines.
	// - HN.. => Heures Normales.
	// - PM.. => Heures de Pointe Mobile.
	// - HCJB => Heures Creuses Jours Bleus.
	// - HCJW => Heures Creuses Jours Blancs (White).
	// - HCJR => Heures Creuses Jours Rouges.
	// - HPJB => Heures Pleines Jours Bleus.
	// - HPJW => Heures Pleines Jours Blancs (White).
	// - HPJR => Heures Pleines Jours Rouges

	char DEMAIN[4]; // Couleur du lendemain si option = tempo
	// valeurs de DEMAIN
	// - ---- : couleur du lendemain non connue
	// - BLEU : le lendemain est jour BLEU.
	// - BLAN : le lendemain est jour BLANC.
	// - ROUG : le lendemain est jour ROUGE.

	uint8_t IINST; // Intensité instantanée (en ampères)
	uint8_t ADPS;  // Avertissement de dépassement de puissance souscrite (en ampères)
	uint8_t IMAX;  // Intensité maximale (en ampères)
	uint32_t PAPP; // Puissance apparente (en Volt.ampères)

	char HHPHC; // Groupe horaire si option = heures creuses ou tempo
	// je comprend pas ce que veulent dire les valeurs de ce truc ... :
	// L'horaire heures pleines/heures creuses (Groupe "HHPHC")
	// est codé par le caractère alphanumérique A, C, D, E ou Y correspondant à la programmation du compteur.
};

// lecture teleinfo
char readTI() {
	while ( !tiSerial.available() );

	return tiSerial.read() & 0x7F;
}

bool atolTI( char *label, char *searchLabel, char *value, uint32_t &last, MyMessage &msg ) {
	uint32_t tmp;

	if ( strcmp( label, searchLabel ) != 0 )
		return false;

	tmp = atol( value );
	if ( last == tmp )
		return true;

#ifdef DEBUG_ENABLED
	Serial.print( label );
	Serial.print( F(" changed from ") );
	Serial.print( last );
	Serial.print( F(" to ") );
	Serial.println( tmp );
#endif

	last = tmp;
	gw.send( msg.set( last ) );

	return true;
}
bool atoiTI( char *label, char *searchLabel, char *value, uint8_t &last, MyMessage &msg ) {
	uint8_t tmp;

	if ( strcmp( label, searchLabel ) != 0 )
		return false;

	tmp = atoi( value );
	if ( last == tmp )
		return true;

#ifdef DEBUG_ENABLED
  Serial.print( label );
  Serial.print( F(" changed from ") );
  Serial.print( last );
  Serial.print( F(" to ") );
  Serial.println( tmp );
#endif

	last = tmp;
	gw.send( msg.set( last ) );

	return true;
}
bool charTI( char *label, char *searchLabel, char &value, char &last, MyMessage &msg ) {
	if ( strcmp( label, searchLabel ) != 0 )
		return false;

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
	gw.send( msg.set( last ) );

	return true;
}
bool strTI( char *label, char *searchLabel, char *value, char *last, MyMessage &msg ) {
	if ( strcmp( label, searchLabel ) != 0 )
		return false;

	if ( strcmp( last, value ) == 0 )
		return true;

#ifdef DEBUG_ENABLED
  Serial.print( label );
  Serial.print( F(" changed from ") );
  Serial.print( last );
  Serial.print( F(" to ") );
  Serial.println( value );
#endif

	strcpy(last, value);
	gw.send( msg.set( last ) );

	return true;
}

void getTI() {
	static TeleInfo last; // dernière lecture
	char c; // le char qu'on read

	// tout d'abord on cherche une fin de ligne
	while ( readTI() != '\n' );
	// maintenant on cherche le label MOTDETAT (cad fin de trame)
	// TIP: c'est le seul qui commence par un M!
	bool sol = true; // start of line
	while ( true ) {
		c = readTI();
		if ( sol && (c == 'M') )
			break;
		sol = ( c == '\n' ); // fin de ligne trouvée, le prochain char sera en debut de ligne donc!
	}
	// eat the line
	while ( readTI() != '\n' );

	// ca y est, on peu bosser!
	readLine:

	uint8_t i; // un compteur
	uint8_t myCS = 32, cs; // le checksum

	// commencer par detecter le label (search for 0x20)
	i = 0;
	char label[ BUFSIZE ]; // etiquette
	memset( label, 0, BUFSIZE );
	while ( true ) {
		c = readTI();
		myCS += (int)c;

		if ( c == ' ' ) break;

		label[ i++ ] = c;
		if ( i == BUFSIZE ) // prevent overflow, it will break the checksum, so silent exit
			break; 
	}

	// la value (search for 0x20)
	i = 0;
	char value[ BUFSIZE ];  // la value la plus longue ligne est ADCO / 15
	memset( value, 0, BUFSIZE );
	while ( true ) {
		c = readTI();
		myCS += (int)c;

		if ( c == ' ' ) break;

		value[ i++ ] = c;
		if ( i == BUFSIZE ) // prevent overflow, it will break the checksum, so silent exit
			break; 
	}

	// le checksum
	cs = readTI();

	// drop end of line char
	readTI(); readTI(); // \r\n

#ifdef DEBUG_ENABLED
		Serial.print( F("GOT LABEL=") );
		Serial.print( label );
		Serial.print( F(" VALUE=") );
		Serial.print( value );
		Serial.print( F(" CHECKSUM=") );
		Serial.println( cs, HEX );
#endif

	// now on regarde ce que cette ligne voulait bien nous dire
	// the end ?
	if ( strcmp( label, "MOTDETAT" ) == 0 ) // on verifie pas le checksum de cette ligne
		return;

	// check le checksum
	myCS = (myCS & 0x3F) + 0x20;
	if ( myCS != cs ) { // si c'est pas bon... ben c'est con!
#ifdef DEBUG_ENABLED
		Serial.print( F("CHECKSUM ERROR!!!, MY=") );
		Serial.print( myCS, HEX );
		Serial.print( F(" CHECKSUM=") );
		Serial.println( cs, HEX );
#endif
		goto readLine;
	}

	if ( strTI( label, "ADCO", value, last.ADCO, msgADCO ) )			goto readLine;
	if ( strTI( label, "OPTARIF", value, last.OPTARIF, msgOPTARIF ) )	goto readLine;
	if ( atoiTI( label, "ISOUSC", value, last.ISOUSC, msgISOUSC ) )		goto readLine;
	if ( atolTI( label, "BASE", value, last.BASE, msgBASE ) )			goto readLine;

	if ( atolTI( label, "HCHC", value, last.HC_HC, msgHC_HC ) )			goto readLine;
	if ( atolTI( label, "HCHP", value, last.HC_HP, msgHC_HP ) )			goto readLine;

	if ( atolTI( label, "EJPHN", value, last.EJP_HN, msgEJP_HN ) )		goto readLine;
	if ( atolTI( label, "EJPHPM", value, last.EJP_HPM, msgEJP_HPM ) )	goto readLine;

	if ( atolTI( label, "BBRHCJB", value, last.BBR_HC_JB, msgBBR_HC_JB ) ) goto readLine;
	if ( atolTI( label, "BBRHPJB", value, last.BBR_HP_JB, msgBBR_HP_JB ) ) goto readLine;
	if ( atolTI( label, "BBRHCJW", value, last.BBR_HC_JW, msgBBR_HC_JW ) ) goto readLine;
	if ( atolTI( label, "BBRHPJW", value, last.BBR_HP_JW, msgBBR_HP_JW ) ) goto readLine;
	if ( atolTI( label, "BBRHCJR", value, last.BBR_HC_JR, msgBBR_HC_JR ) ) goto readLine;
	if ( atolTI( label, "BBRHPJR", value, last.BBR_HP_JR, msgBBR_HP_JR ) ) goto readLine;

	if ( atoiTI( label, "PEJP", value, last.PEJP, msgPEJP ) )			goto readLine;
	if ( strTI( label, "PTEC", value, last.PTEC, msgPTEC ) )			goto readLine;

	if ( strTI( label, "DEMAIN", value, last.DEMAIN, msgDEMAIN ) )		goto readLine;

	if ( atoiTI( label, "IINST", value, last.IINST, msgIINST ) )		goto readLine;
	if ( atoiTI( label, "ADPS", value, last.ADPS, msgADPS ) )			goto readLine;
	if ( atoiTI( label, "IMAX", value, last.IMAX, msgIMAX ) )			goto readLine;
	if ( atolTI( label, "PAPP", value, last.PAPP, msgPAPP ) )			goto readLine;

	if ( charTI( label, "HHPHC", value[0], last.HHPHC, msgHHPHC ) )
		goto readLine;

#ifdef DEBUG_ENABLED
	Serial.print( F( "unkown LABEL=" ) );
	Serial.print( label );
	Serial.print( F( " VALUE=" ) );
	Serial.println( value );	
#endif

	// pour les cas non gérés
	goto readLine;
}

void loop() {
	//readBatteryLevel( gw );
	getTI();

	// attendre avant prochaine lecture
	gw.sleep( SLEEP_TIME );
}