#ifndef TELEINFO_H
#define TELEINFO_H

// to enable debug, uncomment this line
//#define DEBUG_TI

#define TI_VERSION "0.6b"

#if ARDUINO >= 100
	#include <Arduino.h> // Arduino 1.0
#else
	#include <WProgram.h> // Arduino 0022
#endif
#include <SoftwareSerial.h>

#define BUFSIZE 15

typedef struct teleInfo_t teleInfo_t;
struct teleInfo_t {
	// DOC ERDF - http://www.magdiblog.fr/wp-content/uploads/2014/09/ERDF-NOI-CPT_02E.pdf
	char ADCO[BUFSIZE]; //Identifiant du compteur - m'en fous
	char OPTARIF[BUFSIZE]; //Option tarifaire (type d’abonnement) - m'en fous
	// juste pour memoire, les values
	// BASE : option base
	// HC.. : option heure creuse
	// EJP. : option EJP
	// BBRx : option tempo, x est un char qui indique kkchose (relooooooou les mecs)
	uint8_t ISOUSC; // : Intensité souscrite 2chars en Ampères - m'en fous
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
	uint8_t IINST; // Intensité instantanée (en ampères)
	uint8_t ADPS; // Avertissement de dépassement de puissance souscrite (en ampères)
	uint8_t IMAX; // Intensité maximale (en ampères)
	uint32_t PAPP; // Puissance apparente (en Volt.ampères)

	uint32_t BASE; // Index si option = base (en Wh)

	uint32_t HC_HC; // Index heures creuses si option = heures creuses (en Wh)
	uint32_t HC_HP; // Index heures pleines si option = heures creuses (en Wh)

	uint32_t EJP_HN; // Index heures normales si option = EJP (en Wh)
	uint32_t EJP_HPM; // Index heures de pointe mobile si option = EJP (en Wh)
	uint8_t PEJP; // : Préavis EJP si option = EJP 30mn avant période EJP, en minutes

	uint32_t BBR_HC_JB; // Index heures creuses jours bleus si option = tempo (en Wh)
	uint32_t BBR_HP_JB; // Index heures pleines jours bleus si option = tempo (en Wh)
	uint32_t BBR_HC_JW; // Index heures creuses jours blancs si option = tempo (en Wh)
	uint32_t BBR_HP_JW; // Index heures pleines jours blancs si option = tempo (en Wh)
	uint32_t BBR_HC_JR; // Index heures creuses jours rouges si option = tempo (en Wh)
	uint32_t BBR_HP_JR; // Index heures pleines jours rouges si option = tempo (en Wh)
	char DEMAIN[BUFSIZE]; // Couleur du lendemain si option = tempo
	// valeurs de DEMAIN
	// - ---- : couleur du lendemain non connue
	// - BLEU : le lendemain est jour BLEU.
	// - BLAN : le lendemain est jour BLANC.
	// - ROUG : le lendemain est jour ROUGE.
	char HHPHC; // Groupe horaire si option = heures creuses ou tempo
	// je comprend pas ce que veulent dire les valeurs de ce truc ... :
	// L'horaire heures pleines/heures creuses (Groupe "HHPHC")
	// est codé par le caractère alphanumérique A, C, D, E ou Y correspondant à la programmation du compteur.

	// init members
	teleInfo_t() :
		ADCO( {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'} ),
		OPTARIF( {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'} ),
		ISOUSC( 0 ),
		PTEC( {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'} ),
		IINST( 0 ),
		ADPS( 0 ),
		IMAX( 0 ),
		PAPP( 0 ),

		BASE( 0 ),
		HC_HC( 0 ),	HC_HP( 0 ),
		EJP_HN( 0 ), EJP_HPM( 0 ), PEJP( 0 ),
		BBR_HC_JB( 0 ), BBR_HP_JB( 0 ), BBR_HC_JW( 0 ), BBR_HP_JW( 0 ), BBR_HC_JR( 0 ), BBR_HP_JR( 0 ),
		DEMAIN( {'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'} ),
		HHPHC( { '\0' } )
		{}
};

class teleInfo {
	public:
		teleInfo( uint8_t rxPin );
		~teleInfo();
		teleInfo_t get();
		const char *VERSION = TI_VERSION;

	private:
		SoftwareSerial* tiSerial;

		char read();

		bool save( char *label, char *searchLabel, char *value, uint32_t &dst );
		bool save( char *label, char *searchLabel, char *value, uint8_t &dst );
		bool save( char *label, char *searchLabel, char &value, char &dst );
		bool save( char *label, char *searchLabel, char *value, char *dst );
};

#endif
