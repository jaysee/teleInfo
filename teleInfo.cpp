#include "teleInfo.h"

// constructor
teleInfo::teleInfo( uint8_t rxPin ) {
	// pas sur que ce soit nécessaire, mais la doc de SoftwareSerial l'indique
	pinMode( rxPin, INPUT );

	tiSerial = new SoftwareSerial( rxPin, rxPin ); // dans les faits, le TX ne sert pas, je feinte en mettant 2 fois RX
	tiSerial->begin( 1200 );
}

// destructor
teleInfo::~teleInfo() {
	if ( NULL != tiSerial ) {
         delete tiSerial;
     }
}

// lecture
char teleInfo::read() {
	while ( !tiSerial->available() );

	return tiSerial->read() & 0x7F;
}

// save TI values in the (good) struct member
bool teleInfo::save( char *label, char *searchLabel, char *value, uint32_t &dst ) {
	if ( strcmp( label, searchLabel ) != 0 )
		return false;

	dst = atol( value );

	return true;
}
bool teleInfo::save( char *label, char *searchLabel, char *value, uint8_t &dst ) {
	if ( strcmp( label, searchLabel ) != 0 )
		return false;

	dst = atoi( value );

	return true;
}
bool teleInfo::save( char *label, char *searchLabel, char &value, char &dst ) {
	if ( strcmp( label, searchLabel ) != 0 )
		return false;

	dst = value;

	return true;
}
bool teleInfo::save( char *label, char *searchLabel, char *value, char *dst ) {
	if ( strcmp( label, searchLabel ) != 0 )
		return false;

	memset( dst, 0, BUFSIZE );
	strcpy( dst, value );

	return true;
}

teleInfo_t teleInfo::get() {
	teleInfo_t res;

	char c; // le char qu'on read
	byte nb = 0;

	// enable this softSerial to listening - only if more than one softSerial is in use
	tiSerial->listen();

	// clear the softSerial buffer - sinon on a du bazar qui traine et ca fout le dawa!
	if ( tiSerial->overflow() ) {
		// j'ai des doutes, comme le compteur bombarde en permanence, on terminera jamais cette boucle...
		while ( tiSerial->available() )
  			tiSerial->read();
	}

	// tout d'abord on cherche une fin de ligne
	while ( read() != '\n' );

	// maintenant on cherche le label MOTDETAT (cad fin de trame)
	// TIP: c'est le seul qui commence par un M!
	bool sol = true; // start of line
	while ( true ) {
		c = read();
		if ( sol && (c == 'M') )
			break;
		sol = ( c == '\n' ); // fin de ligne trouvée, le prochain char sera en debut de ligne donc!
	}

	readline:

	// Cherche une fin de ligne pour etre sur de bien commencer au début d'une ligne ensuite
	while ( read() != '\n' );

	uint8_t i; // un compteur
	uint8_t myCS = 32, cs; // le checksum

	// commencer par detecter le label (search for ' ')
	i = 0;
	char label[ BUFSIZE ]; // etiquette
	memset( label, '\0', BUFSIZE );
	while ( true ) {
		c = read();
		myCS += (int)c;

		if ( c == ' ' ) break;

		label[ i++ ] = c;
		if ( i == BUFSIZE ) // prevent overflow, it will break the checksum, so silent exit
			break; 
	}

	// rapidement on regarde si c'etait la fin de trame et on skip la suite dans ce cas
	// the end ?
	if ( strcmp( label, "MOTDETAT" ) == 0 ) { // on verifie pas le checksum de cette ligne
#ifdef DEBUG_TI
		Serial.println( F( "------------------------" ) );
		Serial.println( F( "GOT MOTDETAT -  bye " ) );
		Serial.println( F( "------------------------" ) );
#endif
		return res; // fin de trame
	}

	// la value (search for ' ')
	i = 0;
	char value[ BUFSIZE ];  // la value la plus longue ligne est ADCO / ~15
	memset( value, '\0', BUFSIZE );
	while ( true ) {
		c = read();
		myCS += (int)c;

		if ( c == ' ' ) break;

		value[ i++ ] = c;
		if ( i == BUFSIZE ) // prevent overflow, it will break the checksum, so silent exit
			break; 
	}

	// le checksum
	cs = read();

#ifdef DEBUG_TI
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
#ifdef DEBUG_TI
		Serial.print( F("CHECKSUM ERROR!!!, MY=") );
		Serial.print( myCS, HEX );
		Serial.print( F(" CHECKSUM=") );
		Serial.println( cs, HEX );
#endif

		goto readline;
	}

	// sauvegarde la value dans le bon element de la structure
	if ( save( label, "ADCO", value, res.ADCO ) )		goto readline;
	if ( save( label, "OPTARIF", value, res.OPTARIF ) )	goto readline;
	if ( save( label, "ISOUSC", value, res.ISOUSC ) )	goto readline;
	if ( save( label, "BASE", value, res.BASE ) )		goto readline;

	if ( save( label, "HCHC", value, res.HC_HC ) )		goto readline;
	if ( save( label, "HCHP", value, res.HC_HP ) )		goto readline;

	if ( save( label, "EJPHN", value, res.EJP_HN ) )	goto readline;
	if ( save( label, "EJPHPM", value, res.EJP_HPM ) )	goto readline;

	if ( save( label, "BBRHCJB", value, res.BBR_HC_JB ) ) goto readline;
	if ( save( label, "BBRHPJB", value, res.BBR_HP_JB ) ) goto readline;
	if ( save( label, "BBRHCJW", value, res.BBR_HC_JW ) ) goto readline;
	if ( save( label, "BBRHPJW", value, res.BBR_HP_JW ) ) goto readline;
	if ( save( label, "BBRHCJR", value, res.BBR_HC_JR ) ) goto readline;
	if ( save( label, "BBRHPJR", value, res.BBR_HP_JR ) ) goto readline;

	if ( save( label, "PEJP", value, res.PEJP ) )		goto readline;
	if ( save( label, "PTEC", value, res.PTEC ) )		goto readline;

	if ( save( label, "DEMAIN", value, res.DEMAIN ) )	goto readline;

	if ( save( label, "IINST", value, res.IINST ) )		goto readline;
	if ( save( label, "IINST1", value, res.IINST1 ) )   goto readline;
	if ( save( label, "IINST2", value, res.IINST2 ) )   goto readline;
	if ( save( label, "IINST3", value, res.IINST3 ) )   goto readline;
	if ( save( label, "ADPS", value, res.ADPS ) )		goto readline;
	if ( save( label, "IMAX", value, res.IMAX ) )		goto readline;
	if ( save( label, "IMAX1", value, res.IMAX1 ) )   	goto readline;
	if ( save( label, "IMAX2", value, res.IMAX2 ) )   	goto readline;
	if ( save( label, "IMAX3", value, res.IMAX3 ) )   	goto readline;
	if ( save( label, "PAPP", value, res.PAPP ) )		goto readline;
	if ( save( label, "PMAX", value, res.PMAX ) )   goto readline;

	if ( save( label, "HHPHC", value[0], res.HHPHC ) )	goto readline;

#ifdef DEBUG_TI
	Serial.print( F( "unkown LABEL=" ) );
	Serial.print( label );
	Serial.print( F( " VALUE=" ) );
	Serial.println( value );	
#endif

	// pour les cas non gérés (sick!)
	goto readline;
}