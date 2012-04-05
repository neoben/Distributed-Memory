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

/*! Indirizzo IP del server. */
char *SERVER_IP_ADD;
	
/*! Porta associata all'indirizzo IP del server. */
int SERVER_PORT;

/*! Path del file di configurazione. */
const char *config_file_path;

/*! Stringa di appoggio per il contenuto del file di configurazione. */
char line[MAX_LINE_LEN];

/*! Semaforo di mutua esclusione. */
pthread_mutex_t m_acc;

/*! Struttura dati della lista di client associati al blocco: client_list */
/*! 
La struttura dati contiene l'identificativo del client, 
la variabile booleana per la gestione della validità del blocco associato al client, 
la variabile booleana per la gestione del client in attesa che la copia locale diventi valida,  
il puntatore al client successivo connesso al server.
*/
struct client_list {
	/*! Identificativo del client. */
	int cl_ID;
	/*! Variabile per la gestione della validità del blocco associato al client. */
	bool valid;
	/*! Variabile per la gestione del client in attesa che la copia locale diventi valida. */
	bool lock;
	/*! Puntatore al client successivo connesso al server. */
	client_list *succ;
};

/*! Struttura dati del blocco di memoria: block */
/*! 
La struttura dati contiene l'ID del blocco allocato in memoria, 
il puntatore alla prima cella di memoria in cui è memorizzato il blocco, 
il semaforo per la gestione della mutua esclusione sul blocco, 
il puntatore alla testa della lista dei client connessi al server, 
il puntatore al blocco successivo.
*/
struct block {
	/*! ID del blocco allocato in memoria. */
	int ID;
	/*! Puntatore alla prima cella di memoria in cui è memorizzato il blocco. */
	void *pointer;
	/*! Semaforo per la gestione della mutua esclusione sul blocco. */
	pthread_mutex_t b_acc;
	/*! Puntatore alla testa della lista dei client connessi al server. */
	client_list *cl_ptr;
	/*! Puntatore al blocco successivo. */
	block *succ;
};

/*! Classe Server */
/*!
La classe server implementa le strutture dati e i metodi richiesti dalle specifiche lato server. 
Utilizza, inoltre, diverse funzioni private per svolgere le operazioni necessarie ai metodi richiesti.
*/
class Server {
	/*! Puntatore alla testa della lista di blocchi. */
	block *head;

	/*! 
	void add_block(int id_block): 
	funzione privata che crea e inserisce un nuovo blocco in lista. 
	*/
	/*! 
	La funzione crea la lista di blocchi effetuando l'inserimento di un nuovo elemento in testa alla lista, 
	inizializza i parametri della struttura dati block e alloca lo spazio di memoria necessario.
	*/		
	void add_block(int id_block);

	/*! 
	block * find_block(int id): 
	funzione privata che restituisce il blocco identificato da id. 
	*/
	/*! 
	La funzione scorre la lista di blocchi e restituisce il puntatore al blocco che ha id come identificativo. 
	Restituisce NULL se il blocco identificato da id non è presente in lista.
	*/ 
	block * find_block(int id);

	/*! 
	client_list * find_client(int client_id, block* bk_elem): 
	funzione privata che restituisce il client identificato da client_id. 
	*/
	/*! 
	La funzione scorre la lista di client appesa al blocco puntato da bk_elem e 
	restituisce il puntatore al client che ha client_id come identificativo.
	Restituisce NULL se il client identificato da client_id non è presente in lista.
	*/ 
	client_list * find_client(int client_id, block *bk_elem);

	/*! 
	void add_client(int client_id, block *bk_elem): 
	funzione privata che crea e inserisce un nuovo client in lista.
	*/
	/*! 
	La funzione crea la lista di client effettuando l'inserimento di un nuovo elemento in testa alla lista e 
	inizializza i parametri della struttura dati client_list.
	*/		
	void add_client(int client_id, block *bk_elem);

	/*! 
	void delete_client(int client_id, block *bk_elem): 
	funzione privata che elimina un client dalla lista di client. 
	*/
	/*! 
	La funzione scorre la lista di client appesa al blocco puntato da bk_elem, 
	trova il client identificato da client_id e lo elimina dalla lista.
	*/
	void delete_client(int client_id, block *bk_elem);

	/*! 
	void change_valid(int client_id, block *bk_elem): 
	funzione privata che modifica la validità della copia locale del blocco.
	*/
	/*! 
	La funzione scorre la lista di client appesa al blocco puntato da bk_elem, 
	trova il client identificato da client_id e 
	pone la variabile booleana valid relativa al client pari a 0 
	(copia locale invalida).
	*/
	void change_valid(int client_id, block *bk_elem);

	/*! 
	int client_unblock(int client_id, block *bk_elem): 
	funzione privata che sblocca la wait.
	*/
	/*! 
	La funzione scorre la lista di client appesa al blocco puntato da bk_elem, 
	trova il client identificato da client_id e 
	se questo risulta essere bloccato sulla wait 
	(ovvero se ha la variabile booleana lock pari a 1), 
	lo sblocca e resetta il booleano lock relativo al client a 0.
	*/
	int client_unblock(int client_id, block *bk_elem);

public:	
	/*! 
	Server(): 
	costruttore della classe Server. 
	*/
	/*!
	Il costruttore inizializza il puntatore alla testa della lista di blocchi a NULL.
	*/		
	Server();

	/*! 
	int port_control(int port): 
	funzione pubblica che effettua il controllo sulla porta associata al server.
	*/
	/*!
	La funzione effettua il controllo sulla porta associata al server 
	per impedire l'utilizzo di porte destinate ad altri scopi.
	*/
	int port_control(int port);

	/*! 
	read_config_file(const char *config_file): 
	funzione pubblica che legge il file di configurazione. 
	*/
	/*!
	La funzione legge il contenuto del file di configurazione, ne memorizza le righe, 
	controlla indirizzo IP e porta, 
	e in caso di corrispondenza richiama la add_block.
	*/
	void read_config_file(const char *config_file);

	/*! 
	int block_map(int sk): 
	funzione pubblica che stabilisce l'associazione tra client e blocco. 
	*/
	/*!
	La funzione funzione riceve l'id del blocco interessato dal client identificato da sk, 
	trova il blocco richiamando la find_block, 
	stabilisce l'associazione blocco-client richiamando la add_client, 
	invia il contenuto del blocco al client.
	Tutte le operazioni sul blocco sono effettuate in mutua esclusione. 	
	La funzione ritorna 1 in caso di successo e -1 in caso di errore. 
	*/
	int block_map(int sk);

	/*! 
	int block_unmap(int sk): 
	funzione pubblica che cancella l'associazione tra client e blocco.
	*/
	/*!
	La funzione riceve l'id del blocco interessato dal client identificato da sk, 
	trova il blocco richiamando la find_block, 
	elimina l'associazione client-blocco richiamando la delete_client.
	Tutte le operazioni sul blocco sono effettuate in mutua esclusione.
	La funzione ritorna 1 in caso di successo e -1 in caso di errore. 
	*/
	int block_unmap(int sk);

	/*! 
	int block_update(int sk): 
	funzione pubblica che invia il contenuto del blocco al client.
	*/
	/*!
	La funzione riceve l'id del blocco interessato dal client identificato da sk, 
	trova il blocco richiamando la find_block, 
	trova il client assocciato al blocco richiamando la find_client, 
	effettua un controllo sulla validità della copia locale e 
	manda il valore del booleano valid al client. 
	Se la copia locale risulta ancora valida termina le operazioni, 
	altrimenti manda il contenuto del blocco al client e 
	segnala la copia locale come valida 
	settando il valore della variabile booleana valid a 1.
	Tutte le operazioni sul blocco sono effettuate in mutua esclusione.
	La funzione ritorna 1 in caso di successo e -1 in caso di errore. 
	*/
	int block_update(int sk);

	/*! 
	int block_write(int sk): 
	funzione pubblica che aggiorna il contenuto della copia globale del blocco con quello della copia locale. 
	*/
	/*!
	La funzione riceve l'id del blocco interessato dal client identificato da sk, 
	trova il blocco richiamando la find_block, 
	trova il client associato al blocco richiamando la find_client, 
	effettua un controllo sulla validità della copia locale e 
	manda il valore della variabile booleana valid al client.
	Se la copia locale risulta invalida termina le operazioni, 
	altrimenti aggiorna il contenuto della copia globale del blocco 
	con quello della copia locale inviatagli dal client, 
	rende la copia locale invalida richiamando la change_valid, 
	invia al client il segnale di sbloccarsi sulla wait richiamando la client_unblock.
	Tutte le operazioni sul blocco sono effettuate in mutua esclusione.
	La funzione ritorna 1 in caso di successo e -1 in caso di errore.	
	*/
	int block_write(int sk);

	/*! 
	int block_wait(int sk): 
	funzione pubblica che controlla la validità della copia locale del blocco.
	*/
	/*!
	La funzione riceve l'id del blocco interessato dal client identificato da sk, 
	trova il blocco richiamando la find_block, 
	trova il client associato al blocco richiamando la find_client, 
	manda al client il valore della variabile booleana valid e 
	se quest ultimo risulta pari a 1,  
	setta la variabile booleana lock a 1 segnalando che il client è bloccato sulla wait.
	*/
	int block_wait(int sk);
};




