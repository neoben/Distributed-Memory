#include <iostream>
#include <fstream>
#include <cstring> 
#include <stdlib.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <strings.h> 
#include <pthread.h>

/*! Numero massimo di richieste di connessione che si possono accodare. */
#define BACKLOG 5 

/*! Lunghezza massima di ogni riga del file di configurazione. */
#define MAX_LINE_LEN 1024 

/*! Dimensione del blocco di memoria in bytes. */
#define DIMBLOCK 128*sizeof(char) 

using namespace std;

/*! Struttura dati del blocco di memoria: block_list */
/*! 
La struttura dati contiene l'ID del blocco di memoria, 
il puntatore alla prima cella di memoria in cui è memorizzato il blocco, 
la variabile booleana per gestire l'associazione tra blocco di memoria e indirizzo di memoria locale, 
il puntatore al blocco successivo.
*/
struct block_list {
	/*! ID del blocco di memoria. */
	int ID;
	/*! Puntatore alla prima cella di memoria in cui è memorizzato il blocco. */
	void *pointer; 
	/*! Variabile per gestire l'associazione tra blocco di memoria e indirizzo di memoria locale. */
	bool associated;
	/*! Puntatore al blocco successivo. */
	block_list *succ;
};

/*! Struttura dati della lista di server: server_list */
/*! 
La struttura dati contiene l'indirizzo IP del server a cui si connette il client, 
la porta associata all'indirizzo IP, 
il descrittore del socket lato client, 
la variabile booleana per la gestione della connessione client-server, 
il puntatore alla testa della lista di blocchi associati ad ogni server, 
il puntatore all'elemento successivo della lista dei server a cui il client si è connesso.
*/
struct server_list {
	/*! Indirizzo IP del server a cui si connette il client. */
	char ip_add[15]; 
	/*! Porta associata all'indirizzo IP. */
	int port; 
	/*! Descrittore del socket lato client. */
	int cl_sk;
	/*! Variabile per la gestione della connessione client-server. */
	bool connected; 
	/*! Puntatore alla testa della lista di blocchi associati ad ogni server. */
	block_list *block_head;	
	/*! Puntatore all'elemento successivo della lista di server a cui il client è connesso. */
	server_list *succ;
};

/*! Classe Library */
/*!
La classe implementa le strutture dati e le funzioni di interfaccia richieste dalle specifiche. 
Utilizza, inoltre, diverse funzioni private per svolgere le operazioni necessarie ai metodi richiesti.
*/
class Library{
	/*! Puntatore alla testa della lista dei server. */
	server_list *head;
		
	/*! 
	server_list * find_server(int id): 
	funzione privata che restituisce il server contenente il blocco identificato da id. 
	*/
	/*! 
	La funzione scorre la lista di server, 
	contestualmente scorre la lista di blocchi appesi al server e 
	restiuisce il puntatore al server che contiene il blocco che ha id come identificativo.
	Restituisce NULL se il server non è presente in lista.
	*/
	server_list * find_server(int id);

	/*! 
	block_list * find_block(int id, server_list *sl): 
	funzione privata che restituisce il blocco identificato da id.
	*/
	/*! 
	La funzione scorre la lista di blocchi appesi al server puntato da sl e 
	restituisce il puntatore al blocco identificato da id.
	Restituisce NULL se il blocco non è presente in lista.
	*/
	block_list * find_block(int id, server_list *sl);

	/*! 
	void create_socket(): 
	funzione privata che crea i socket necessari per mettersi in connessione con i server. 
	*/
	/*! 
	La funzione scorre la lista di server e 
	crea un socket per ogni elemento presente in lista.
	*/
	void create_socket();	

	/*! 
	int client_connect(char *ip_add, int port, int cl_sk, server_list *sl): 
	funzione privata che effettua la connessione del client al server.
	*/
	/*! 
	La funzione inizializza le strutture dati necessarie ,
	effettua la connessione verso il server avente indirizo IP ip_add e porta port, 
	setta la variabile booleana connected a 1.
	La funzione ritorna 1 in caso di successo e -1 in caso di errore.	
	*/
	int client_connect(char *ip_add, int port, int cl_sk, server_list *sl);	

	/*! void read_config_file(char* config_file): 
	funzione privata che legge il file di configurazione.
	*/
	/*! 
	La funzione legge il contenuto del file di configurazione, 
	crea la lista di server in base al numero di righe presenti nel file  
	effettuando l'inserimento di ogni nuovo elemento in testa, 
	per ogni elemento della lista di server crea una lista di blocchi associati al server 
	in base ai blocchi presenti nel file di configurazione 
	effettuando l'inserimento di ogni nuovo elemento in testa, 
	inizializza le variabili della struttura relativa al blocco per ogni elemento della lista di blocchi.
	*/
	void read_config_file(char *config_file);
public:
	/*! 
	Library(): 
	costruttore della classe Library.
	*/
	/*! 
	Il costruttore inizializza il puntatore alla testa della lista dei server a NULL.
	*/		
	Library();
		
	/*! 
	void dm_init(char* config_file): 
	funzione pubblica che inizializza la libreria.
	*/
	/*! 
	La funzione carica dal file config_file l'associazione tra ID dei blocchi e server
	richiamando la read_config_file, 
	apre una serie di socket per permettere al client di mettersi in connessione con i server
	richiamando la create_socket. 
	*/			
	void dm_init(char *config_file);

	/*! 
	int dm_block_map(int ID, void* address): 
	funzione pubblica che stabilisce l'associazione tra blocco di memoria e indirizzo di memoria locale.
	*/
	/*! 
	La funzione trova il server che contiene il blocco ID richiamando la find_server, 
	trova il blocco interessato richiamando la find_block, 
	effettua un controllo sulla variabile booleana connected e 
	se il client non risulta connesso al server, 
	effettua la connessione richiamando la client_connect.
	Successivamente invia il comando relativo all'operazione dal svolgere al server, 
	invia l'ID del blocco interessato al server, 
	richiede la copia del blocco a partire dall'indirizzo address, 
	segnala che il blocco è associato ad un indirizzo di memoria locale 
	settando la variabile booleana associated a 1.
	La funzione restituisce 1 in caso di successo e -1 in caso di errore 
	(non è possibile effettuare la connessione,  
	il server non risponde, 
	il blocco è gia associato ad un altro indirizzo di memoria locale).
	*/
	int dm_block_map(int ID, void *address);

	/*! 
	void dm_block_unmap(int ID): 
	funzione pubblica che segnala al server che il client non è più interessato al blocco ID.
	*/
	/*! 
	La funzione trova il server che contiene il blocco ID richiamando la find_server, 
	trova il blocco interessato richiamando la find_block, 
	invia il comando relativo all'operazione dal svolgere al server, 
	invia l'ID del blocco interessato al server, 
	segnala che il blocco non è più associato ad un indirizzo di memoria locale 
	resettando la variabile boolena associated a 0.
	La funzione fallisce se il blocco non era stato preventivamente associato 
	ad un indirizzo di memoria locale.
	*/
	void dm_block_unmap(int ID);

	/*! 
	int dm_block_update(int ID): 
	funzione pubblica che aggiorna il contenuto della copia locale con il contenuto globale sul server.
	*/
	/*! 
	La funzione trova il server che contiene il blocco ID richiamando la find_server, 
	trova il blocco interessato richiamando la find_block, 
	invia il comando relativo all'operazione dal svolgere al server, 
	invia l'ID del blocco interessato al server, 
	riceve il controllo sulla validità della copia locale del blocco dal server e 
	se la copia locale risulta ancora valida 
	non effettua alcuna copia, altrimenti, 
	aggiorna il contenuto della copia locale con quello della copia globale.
	La funzione restituisce 1 in caso di successo e -1 in caso di errore 
	(il server non risponde, 
	il blocco non era stato associato ad alcun indirizzo di memoria locale).
	*/
	int dm_block_update(int ID);

	/*! 
	int dm_block_write(int ID): 
	funzione pubblica che sincronizza il contenuto locale del blocco con il contenuto globale sul server.
	*/
	/*! 
	La funzione trova il server che contiene il blocco ID richiamando la find_server, 
	trova il blocco interessato richiamando la find_block, 
	invia il comando relativo all'operazione dal svolgere al server, 
	invia l'ID del blocco interessato al server, 
	riceve il controllo sulla validità della copia locale del blocco dal server e 
	se la copia locale risulta valida 
	sincronizza il contenuto locale del blocco con il contenuto globale sul server.
	La funzione ritorna con errore (con codici di errori diversi) nel caso in cui 
	il blocco non era stato associato ad alcun indirizzo di memoria locale, 
	la copia locale del blocco risulta invalida, 
	il server non risponde.
	La funzione restituisce 1 in caso di successo.
	*/
	int dm_block_write(int ID);

	/*! 
	int dm_block_wait(int ID): 
	funzione pubblica che controlla la validità della copia locale del blocco.
	*/
	/*! 
	La funzione trova il server che contiene il blocco ID richiamando la find_server, 
	trova il blocco interessato richiamando la find_block, 
	invia il comando relativo all'operazione dal svolgere al server, 
	invia l'ID del blocco interessato al server, 
	riceve il controllo sulla validità della copia locale del blocco dal server e 	
	se la copia locale risulta invalida ritorna immediatamente, altrimenti, 
	resta in attesa di ricevere dal server il segnale che notifica che 
	la copia locale è diventata invalida.
	La funzione restituisce 1 in caso di successo e -1 in caso di errore 
	(il server non risponde, 
	il blocco non era stato associato ad alcun indirizzo di memoria locale).
	*/
	int dm_block_wait(int ID);

};



