// uncomment to enable debuging using serial port @115200bps
//#define DEBUG_ENABLED

// durée entre 2 rafraichissements des données - en mili-secondes (bien laisser UL la fin)
// j'ai un probleme, il met 3 fois plus de temps que ce que j'indique!
// testé sur un arduino pro mini 3.3v/8mhz - est-ce lié????
#define SLEEP_TIME 20000UL

// mysensors
#include <SPI.h>
#include <MySensor.h>

// teleinfo
#include <SoftwareSerial.h>
#define TI_RX 4

// version du sketch
#define VERSION "0.4c"

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

// connexion série avec le compteur EDF
SoftwareSerial tiSerial( TI_RX, TI_RX ); // dans les faits, le TX ne sert pas

void setup() {
#ifdef DEBUG_ENABLED
	Serial.begin( 115200 );
#endif

	tiSerial.begin( 1200 );

	gw.begin();
	gw.sendSketchInfo( "Teleinfo Sensor", VERSION );

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

typedef struct TeleInfo TeleInfo;
struct TeleInfo {
	// DOC ERDF - http://www.magdiblog.fr/wp-content/uploads/2014/09/ERDF-NOI-CPT_02E.pdf
	char ADCO[BUFSIZE]; //Identifiant du compteur - m'en fous
	char OPTARIF[BUFSIZE]; //Option tarifaire (type d’abonnement) - m'en fous
	// juste pour memoire, les values
	// BASE : option base
	// HC.. : option heure creuse
	// EJP. : option EJP
	// BBRx : option tempo, x est un char qui indique kkchose (relooooooou les mecs)
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
	uint32_t BBR_HC_JR; // Index heures creuses jours rouges si option = tempo (en Wh)
	uint32_t BBR_HP_JR; // Index heures pleines jours rouges si option = tempo (en Wh)
	uint8_t PEJP; // : Préavis EJP si option = EJP 30mn avant période EJP, en minutes
	char PTEC[BUFSIZE]; // Période tarifaire en cours
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
	char DEMAIN[BUFSIZE]; // Couleur du lendemain si option = tempo
	// valeurs de DEMAIN
	// - ---- : couleur du lendemain non connue
	// - BLEU : le lendemain est jour BLEU.
	// - BLAN : le lendemain est jour BLANC.
	// - ROUG : le lendemain est jour ROUGE.
	uint8_t IINST; // Intensité instantanée (en ampères)
	uint8_t ADPS; // Avertissement de dépassement de puissance souscrite (en ampères)
	uint8_t IMAX; // Intensité maximale (en ampères)
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

bool compareTI( char *label, char *searchLabel, char *value, uint32_t &last, MyMessage &msg, int SENSOR_ID ) {
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
	gw.send( msg.setSensor( SENSOR_ID ).set( last ) );

	return true;
}
bool compareTI( char *label, char *searchLabel, char *value, uint8_t &last, MyMessage &msg, int SENSOR_ID ) {
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
	gw.send( msg.setSensor( SENSOR_ID ).set( last ) );

	return true;
}
bool compareTI( char *label, char *searchLabel, char &value, char &last, MyMessage &msg, int SENSOR_ID ) {
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
	gw.send( msg.setSensor( SENSOR_ID ).set( last ) );

	return true;
}
bool compareTI( char *label, char *searchLabel, char *value, char *last, MyMessage &msg, int SENSOR_ID ) {
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

	memset( last, 0, BUFSIZE ); // sembleraie que strcpy ne fasse pas bien son boulot...
	strcpy( last, value );

	gw.send( msg.setSensor( SENSOR_ID ).set( last ) );

	return true;
}

void getTI() {
	static TeleInfo last; // dernière lecture
	char c; // le char qu'on read
	byte nb = 0;

	// enable this softSerial to listening - only if more than one softSerial is in use
	tiSerial.listen();

	// clear the softSerial buffer
	// pas sur que ca serve vraiement en fait...
	if ( tiSerial.overflow() ) {
#ifdef DEBUG_ENABLED
		Serial.println( F( "Serial overflow, flushing datas" ) );
#endif
		// j'ai des doutes, comme le compteur bombarde en permanence, on terminera jamais cette boucle...
		while ( Serial.available() ) {
  			c = tiSerial.read();
#ifdef DEBUG_ENABLED
		Serial.print( c );
#endif
		}
#ifdef DEBUG_ENABLED
		Serial.println( F( "Serial overflow end" ) );
#endif
	}

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

	readline:

	// Cherche une fin de ligne pour etre sur de bien commencer au début d'une ligne ensuite
	while ( readTI() != '\n' );

	uint8_t i; // un compteur
	uint8_t myCS = 32, cs; // le checksum

	// commencer par detecter le label (search for ' ')
	i = 0;
	char label[ BUFSIZE ]; // etiquette
	memset( label, '\0', BUFSIZE );
	while ( true ) {
		c = readTI();
		myCS += (int)c;

		if ( c == ' ' ) break;

		label[ i++ ] = c;
		if ( i == BUFSIZE ) // prevent overflow, it will break the checksum, so silent exit
			break; 
	}

	// rapidement on regarde si c'etait la fin de trame et on skip la suite dans ce cas
	// the end ?
	if ( strcmp( label, "MOTDETAT" ) == 0 ) { // on verifie pas le checksum de cette ligne
#ifdef DEBUG_ENABLED
		Serial.println( F( "------------------------" ) );
		Serial.println( F( "GOT MOTDETAT -  bye " ) );
#endif
		return; // fin de trame
	}

	// la value (search for ' ')
	i = 0;
	char value[ BUFSIZE ];  // la value la plus longue ligne est ADCO / ~15
	memset( value, '\0', BUFSIZE );
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

#ifdef DEBUG_ENABLED
		Serial.println( F( "------------------------" ) );
		Serial.print( F("GOT LABEL=") );
		Serial.print( label );
		Serial.print( F(" VALUE=") );
		Serial.print( value );
		Serial.print( F(" CHECKSUM=") );
		Serial.println( cs, HEX );
#endif

	// check le checksum
	myCS = (myCS & 0x3F) + 0x20;
	if ( myCS != cs ) { // si c'est pas bon... ben c'est con!
#ifdef DEBUG_ENABLED
		Serial.print( F("CHECKSUM ERROR!!!, MY=") );
		Serial.print( myCS, HEX );
		Serial.print( F(" CHECKSUM=") );
		Serial.println( cs, HEX );
#endif

		goto readline;
	}

	// gestion mySensor, vérifier si la value à changée et envoyer le message à la gateway si c'est le cas
	if ( compareTI( label, "ADCO", value, last.ADCO, msgVAR1, CHILD_ID_ADCO ) )			goto readline;
	if ( compareTI( label, "OPTARIF", value, last.OPTARIF, msgVAR2, CHILD_ID_OPTARIF ) )	goto readline;
	if ( compareTI( label, "ISOUSC", value, last.ISOUSC, msgCURRENT, CHILD_ID_ISOUSC ) )		goto readline;
	if ( compareTI( label, "BASE", value, last.BASE, msgKWH, CHILD_ID_BASE ) )			goto readline;

	if ( compareTI( label, "HCHC", value, last.HC_HC, msgKWH, CHILD_ID_HC_HC ) )			goto readline;
	if ( compareTI( label, "HCHP", value, last.HC_HP, msgKWH, CHILD_ID_HC_HP ) )			goto readline;

	if ( compareTI( label, "EJPHN", value, last.EJP_HN, msgKWH, CHILD_ID_EJP_HN ) )		goto readline;
	if ( compareTI( label, "EJPHPM", value, last.EJP_HPM, msgKWH, CHILD_ID_EJP_HPM ) )	goto readline;

	if ( compareTI( label, "BBRHCJB", value, last.BBR_HC_JB, msgKWH, CHILD_ID_BBR_HC_JB ) ) goto readline;
	if ( compareTI( label, "BBRHPJB", value, last.BBR_HP_JB, msgKWH, CHILD_ID_BBR_HP_JB ) ) goto readline;
	if ( compareTI( label, "BBRHCJW", value, last.BBR_HC_JW, msgKWH, CHILD_ID_BBR_HC_JW ) ) goto readline;
	if ( compareTI( label, "BBRHPJW", value, last.BBR_HP_JW, msgKWH, CHILD_ID_BBR_HP_JW ) ) goto readline;
	if ( compareTI( label, "BBRHCJR", value, last.BBR_HC_JR, msgKWH, CHILD_ID_BBR_HC_JR ) ) goto readline;
	if ( compareTI( label, "BBRHPJR", value, last.BBR_HP_JR, msgKWH, CHILD_ID_BBR_HP_JR ) ) goto readline;

	if ( compareTI( label, "PEJP", value, last.PEJP, msgKWH, CHILD_ID_PEJP ) )			goto readline;
	if ( compareTI( label, "PTEC", value, last.PTEC, msgVAR3, CHILD_ID_PTEC ) )			goto readline;

	if ( compareTI( label, "DEMAIN", value, last.DEMAIN, msgVAR5, CHILD_ID_DEMAIN ) )		goto readline;

	if ( compareTI( label, "IINST", value, last.IINST, msgCURRENT, CHILD_ID_IINST ) )		goto readline;
	if ( compareTI( label, "ADPS", value, last.ADPS, msgCURRENT, CHILD_ID_ADPS ) )			goto readline;
	if ( compareTI( label, "IMAX", value, last.IMAX, msgCURRENT, CHILD_ID_IMAX ) )			goto readline;
	if ( compareTI( label, "PAPP", value, last.PAPP, msgWATT, CHILD_ID_PAPP ) )			goto readline;

	if ( compareTI( label, "HHPHC", value[0], last.HHPHC, msgVAR4, CHILD_ID_HHPHC ) )		goto readline;

#ifdef DEBUG_ENABLED
	Serial.print( F( "unkown LABEL=" ) );
	Serial.print( label );
	Serial.print( F( " VALUE=" ) );
	Serial.println( value );	
#endif

	// pour les cas non gérés
	goto readline;
}

void loop() {
	// Process incoming messages (like config from server)
	gw.process(); 
	
	getTI();

	delay( SLEEP_TIME );
	//gw.sleep( SLEEP_TIME );
}
