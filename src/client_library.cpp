#include "client_library.h"

Library::Library()
{
	head = NULL;
}

server_list * Library::find_server(int id) 
{
	server_list *sl;
	block_list *bk;

	for(sl = head; sl != NULL; sl = sl->succ)
		for(bk = sl->block_head; bk != NULL; bk = bk->succ)
			if(bk->ID == id)
				return sl;
	return sl;
}

block_list * Library::find_block(int id, server_list *sl)
{
	block_list *bk;
	for(bk = sl->block_head; bk->ID != id; bk = bk->succ)
		if(!bk->succ)
			return bk->succ;
	return bk;
}

void Library::create_socket()
{
	server_list *sl;
	for(sl = head; sl != NULL; sl = sl->succ) {
		sl->cl_sk = socket(AF_INET, SOCK_STREAM, 0);
		if(sl->cl_sk == -1) {
			cerr << "Errore: funzione <socket> non eseguita correttamente sul client" << endl;
    			exit(1);
  		}
		sl->connected = 0;
	}
}	

int Library::client_connect(char *ip_add, int port, int cl_sk, server_list *sl) 
{
	int cl_ret;
	struct sockaddr_in sv_addr;	
	
	/* Inizializzazione struttura dati */
	bzero(&sv_addr, sizeof(struct sockaddr_in)); /* Azzera il contenuto della struttura */
	sv_addr.sin_family = AF_INET;
	sv_addr.sin_port = htons(port);
	inet_pton(AF_INET, ip_add, &sv_addr.sin_addr.s_addr); /* Converte l'indirizzo da stringa a valore numerico */

	cl_ret = connect(cl_sk, (struct sockaddr *)(&sv_addr), sizeof(struct sockaddr_in));
	if(cl_ret == -1)  		
		return -1;

	sl->connected = 1;

	return 1;
}

void Library::read_config_file(char *config_file)
{
	char line[MAX_LINE_LEN]; /* Stringa in cui appoggio il contenuto del file di configurazione */
	char *tmp;
	int tmp_port;
	int tmp_block;	
	fstream fc;

	fc.open(config_file, ios::in);
	if(!fc) {
		cerr << "Errore nell'apertura del file di configurazione" << endl;
    		exit(0);
  	}
	
	while (fc.getline(line, MAX_LINE_LEN)) { /* Legge riga per riga il contenuto del file di configurazione */
		server_list* elem = new server_list;
		block_list *app = NULL;
		tmp = strtok(line, ":");
		if(tmp == NULL) /* Se incontra una riga vuota passa al ciclo successivo */
			continue;
		strcpy(elem->ip_add, tmp);
		tmp = strtok(NULL, ":");
		if(tmp == NULL)
			continue;
		tmp_port = atoi(tmp);
		elem->port = tmp_port;
		while((tmp = strtok(NULL, ":"))) {
			if(tmp == NULL)
				continue;
			tmp_block = atoi(tmp);
			block_list *bk_elem = new block_list;
			bk_elem->ID = tmp_block;
			bk_elem->succ = app;
			bk_elem->associated = 0;
			app = bk_elem;
		}
		elem->block_head = app;
		elem->succ = head;
		head = elem;
  	}

  	fc.close();
}

void Library::dm_init(char *config_file)
{
	read_config_file(config_file);
	create_socket();
}

int Library::dm_block_map(int ID, void *address)
{
	int ret;
	int len;
	char command;
	
	server_list *tmp_sl = find_server(ID);
	if(tmp_sl == NULL) {
		cerr << "Errore: server non trovato nella <dm_block_map>" << endl;
		return -1;	
	}

	block_list *tmp_bk = find_block(ID, tmp_sl);
	if(tmp_bk == NULL) {
		cerr << "Errore: blocco non trovato nella <dm_block_map>" << endl;
		return -1;	
	}

	if(tmp_bk->associated == 1) {
		cerr << "Errore: blocco richiesto già associato ad un indirizzo di memoria locale" << endl;
		return -1;
	}

	if(tmp_sl->connected == 0) {
		ret = client_connect(tmp_sl->ip_add, tmp_sl->port, tmp_sl->cl_sk, tmp_sl);
		if(ret == -1) {
			cerr << "Errore: funzione <client_connect> non eseguita correttamente" << endl;
			cerr << "Connessione con il server fallita" << endl;
			return -1;
		}
	}

	command = 'm';
	len = sizeof(command);
	ret = send(tmp_sl->cl_sk, &command, len, 0);
	if(ret == -1) {
		cerr << "Errore: funzione <send> sul comando non eseguita correttamente nella <dm_block_map>" << endl;
		return -1;
	}

	len = sizeof(ID);
	ret = send(tmp_sl->cl_sk, &ID, len, 0);
	if(ret == -1) {
		cerr << "Errore: invio ID al server fallito nella <dm_block_map>" << endl;
		return -1;	
	}

	tmp_bk->pointer = address;

	len = DIMBLOCK;
	ret = recv(tmp_sl->cl_sk, tmp_bk->pointer, len, MSG_WAITALL);
	if((ret == -1) || (ret < len)) {
		cerr << "Errore: ricezione blocco di memoria fallita nella <dm_block_map>" << endl;	
		return -1;	
	}

	tmp_bk->associated = 1;

	return 1;
}

void Library::dm_block_unmap(int ID)
{
	int ret;
	int len;
	char command;

	server_list *tmp_sl = find_server(ID);
	if(tmp_sl == NULL) {
		cerr << "Errore: server non trovato nella <dm_block_unmap>" << endl;
		return;	
	}

	block_list *tmp_bk = find_block(ID, tmp_sl);
	if(tmp_bk == NULL) {
		cerr << "Errore: blocco non trovato nella <dm_block_unmap>" << endl;
		return;	
	}

	if(tmp_bk->associated == 0) {
		cerr << "Errore: blocco richiesto non associato ad alcun indirizzo di memoria locale" << endl;
		return;
	}	

	command = 'u';
	len = sizeof(command);
	ret = send(tmp_sl->cl_sk, &command, len, 0);
	if(ret == -1) {
		cerr << "Errore: funzione <send> sul comando non eseguita correttamente nella <dm_block_unmap>" << endl;
		return;
	}

	len = sizeof(ID);
	ret = send(tmp_sl->cl_sk, &ID, len, 0);
	if(ret == -1) {
		cerr << "Errore: invio ID al server fallito nella <dm_block_unmap>" << endl;
		return;	
	}

	tmp_bk->associated = 0;	

}

int Library::dm_block_update(int ID)
{
	int ret;
	int len;
	char command;
	bool ok;

	server_list *tmp_sl = find_server(ID);
	if(tmp_sl == NULL) {
		cerr << "Errore: server non trovato nella <dm_block_update>" << endl;
		return -1;	
	}

	block_list *tmp_bk = find_block(ID, tmp_sl);
	if(tmp_bk == NULL) {
		cerr << "Errore: blocco non trovato nella <dm_block_update>" << endl;
		return -1;	
	}

	if(tmp_bk->associated == 0) {
		cerr << "Errore: blocco richiesto non associato ad alcun indirizzo di memoria locale" << endl;
		return -1;
	}

	command = 'p';
	len = sizeof(command);
	ret = send(tmp_sl->cl_sk, &command, len, 0);
	if(ret == -1) {
		cerr << "Errore: funzione <send> sul comando non eseguita correttamente nella <dm_block_update>" << endl;
		return -1;
	}

	len = sizeof(ID);
	ret = send(tmp_sl->cl_sk, &ID, len, 0);
	if(ret == -1) {
		cerr << "Errore: invio ID al server fallito nella <dm_block_update>" << endl;
		return -1;	
	}

	len = sizeof(ok);
	ret = recv(tmp_sl->cl_sk, &ok, len, MSG_WAITALL);
	if((ret == -1) || (ret < len)) {
		cerr << "Errore: ricezione controllo validita' fallita nella <dm_block_update>" << endl;
		return -1;	
	}	

	if(ok)
		return 1;

	len = DIMBLOCK;
	ret = recv(tmp_sl->cl_sk, tmp_bk->pointer, len, MSG_WAITALL);
	if((ret == -1) || (ret < len)) {
		cerr << "Errore: ricezione blocco di memoria fallita nella <dm_block_update>" << endl;
		return -1;	
	}

	return 1;
}

int Library::dm_block_write(int ID)
{
	int ret;
	int len;
	int err_code;
	char command;
	bool ok;
	
	server_list *tmp_sl = find_server(ID);
	if(tmp_sl == NULL) {
		cerr << "Errore: server non trovato nella <dm_block_write>" << endl;
		return -1;	
	}

	block_list *tmp_bk = find_block(ID, tmp_sl);
	if(tmp_bk == NULL) {
		cerr << "Errore: blocco non trovato nella <dm_block_write>" << endl;
		err_code = 2; /* Blocco non presente nella lista di blocchi */
		return err_code;
	}

	if(tmp_bk->associated == 0) {
		err_code = 2; /* Blocco non associato ad alcun indirizzo di memoria locale */
		return err_code;
	}

	command = 'w';
	len = sizeof(command);
	ret = send(tmp_sl->cl_sk, &command, len, 0);
	if(ret == -1) {
		cerr << "Errore: funzione <send> sul comando non eseguita correttamente nella <dm_block_write>" << endl;
		return -1;
	}

	len = sizeof(ID);
	ret = send(tmp_sl->cl_sk, &ID, len, 0);
	if(ret == -1) {
		cerr << "Errore: invio ID al server fallito nella <dm_block_write>" << endl;
		return -1;	
	}

	len = sizeof(ok);
	ret = recv(tmp_sl->cl_sk, &ok, len, MSG_WAITALL);
	if((ret == -1) || (ret < len)) {
		cerr << "Errore: ricezione controllo validita' fallita nella <dm_block_write>" << endl;
		return -1;	
	}	

	if(!ok) {
		err_code = 3; /* Copia locale invalida */
		return err_code;
	}

	ret = send(tmp_sl->cl_sk, tmp_bk->pointer, DIMBLOCK, 0);
	if(ret == -1) {
		cerr << "Errore: invio blocco di memoria fallito nella <dm_block_write>" << endl;
		return -1;	
	}
	
	return 1;
}

int Library::dm_block_wait(int ID)
{
	int len;
	int ret;
	char command;
	bool ok;
	bool unlock;

	server_list *tmp_sl = find_server(ID);
	if(tmp_sl == NULL) {
		cerr << "Errore: server non trovato nella <dm_block_wait>" << endl;
		return -1;	
	}

	block_list *tmp_bk = find_block(ID, tmp_sl);
	if(tmp_bk == NULL) {
		cerr << "Errore: blocco non trovato nella <dm_block_wait>" << endl;
		return -1;	
	}

	if(tmp_bk->associated == 0) {
		cerr << "Errore: blocco richiesto non associato ad alcun indirizzo di memoria locale" << endl;
		return -1;
	}	

	command = 's';
	len = sizeof(command);
	ret = send(tmp_sl->cl_sk, &command, len, 0);
	if(ret == -1) {
		cerr << "Errore: funzione <send> sul comando non eseguita correttamente nella <dm_block_wait>" << endl;
		return -1;
	}

	len = sizeof(ID);
	ret = send(tmp_sl->cl_sk, &ID, len, 0);
	if(ret == -1) {
		cerr << "Errore: invio ID al server fallito nella <dm_block_wait>" << endl;
		return -1;	
	}

	len = sizeof(ok);
	ret = recv(tmp_sl->cl_sk, &ok, len, MSG_WAITALL);
	if((ret == -1) || (ret < len)) {
		cerr << "Errore: ricezione controllo validita' fallita nella <dm_block_wait>" << endl;
		return -1;	
	}	
	
	if(!ok)
		return 1;

	len = sizeof(unlock);
	ret = recv(tmp_sl->cl_sk, &unlock, len, MSG_WAITALL);
	if((ret == -1) || (ret < len)) {
		cerr << "Errore: ricezione controllo validita' fallita nella <dm_block_wait>" << endl;
		return -1;	
	}	

	return 1;
}




