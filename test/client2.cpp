#include "client_library.h"

int main(int argc, char *argv[])
{
	cout << endl;	

	if(argc != 2) {
		cerr << "Errore: sintassi del comando errata" << endl;
		cout << "Lanciare il client con il seguente comando:" << endl;
		cout << "./client <absolute_path_config_file>" << endl;	
		exit(1);
	}

	int ret;
	int num_block; /* Numero di blocchi necessari per il file di test */
	char *tmp;
	Library lib;

	lib.dm_init(argv[1]);

	int WORDS_ID = 0;
	int SPACES_ID = 1;
	int LENGHT_ID = 2;
	int SYNCR1_ID = 3;
	int SYNCR2_ID = 4;

	int *words = (int *)malloc(DIMBLOCK);
	int *spaces = (int *)malloc(DIMBLOCK);
	int *lenght = (int *)malloc(DIMBLOCK);
	int *syncr1 = (int *)malloc(DIMBLOCK);
	int *syncr2 = (int *)malloc(DIMBLOCK);
	
	bzero(words, DIMBLOCK);
	bzero(spaces, DIMBLOCK);
	bzero(lenght, DIMBLOCK);
	bzero(syncr1, DIMBLOCK);
	bzero(syncr2, DIMBLOCK);

	*spaces = 0;
	*words = 0;
	*lenght = 0;
	*syncr1 = 0;
	*syncr2 = 0;

	ret = lib.dm_block_map(WORDS_ID, words);
	if(ret == -1)
		cerr << "Associazione tra blocco di memoria e indirizzo di memoria locale fallita c2 words" << endl;

	ret = lib.dm_block_map(SPACES_ID, spaces);
	if(ret == -1)
		cerr << "Associazione tra blocco di memoria e indirizzo di memoria locale fallita c2 spaces" << endl;

	ret = lib.dm_block_map(LENGHT_ID, lenght);
	if(ret == -1)
		cerr << "Associazione tra blocco di memoria e indirizzo di memoria locale fallita c2 lenght" << endl;

	ret = lib.dm_block_map(SYNCR1_ID, syncr1);
	if(ret == -1)
		cerr << "Associazione tra blocco di memoria e indirizzo di memoria locale fallita c2 syncr1" << endl;

	ret = lib.dm_block_map(SYNCR2_ID, syncr2);
	if(ret == -1)
		cerr << "Associazione tra blocco di memoria e indirizzo di memoria locale fallita c2 syncr2" << endl;


	ret = lib.dm_block_update(SYNCR1_ID);
	if(ret == -1)
		cerr << "Aggiornamento del contenuto della copia locale del blocco fallito" << endl;
	
	if(*syncr1 < 1) {
		ret = lib.dm_block_wait(LENGHT_ID);	
		if(ret == -1)
			cerr << "Controllo della validita' della copia locale del blocco fallito" << endl;
	}

	ret = lib.dm_block_update(LENGHT_ID);
	if(ret == -1)
		cerr << "Aggiornamento del contenuto della copia locale del blocco fallito" << endl;

	num_block = *lenght;

	char *buffer = (char *)malloc(num_block * DIMBLOCK);

	bzero(buffer, DIMBLOCK*num_block);

	for(int i = 0; i < num_block; i++) {
		ret = lib.dm_block_map(i + 5, buffer + (i * DIMBLOCK));
		if(ret == -1)
			cerr << "Associazione tra blocco di memoria e indirizzo di memoria locale fallita c2 blocco "<<(i+5)<< endl;
	}
	
	for(int i = 0; i < num_block; i++) {

		ret = lib.dm_block_update(SYNCR1_ID);
		if(ret == -1)
			cerr << "Aggiornamento del contenuto della copia locale del blocco fallito" << endl;
	
		if(*syncr1 < (2 + i)) {
			ret = lib.dm_block_wait(i + 5);
			if(ret == -1)
				cerr << "Controllo della validita' della copia locale del blocco fallito" << endl;
		}
	}

	for(int i = 0; i < num_block; i++) {
		lib.dm_block_update(i + 5);
		if(ret == -1)
			cerr << "Aggiornamento del contenuto della copia locale del blocco fallito" << endl;
	}

	*words = 0;
	tmp = strtok(buffer, " ");
	*words += 1;
	if(tmp != NULL) 		
		while((tmp = strtok(NULL, " ")))
			*words += 1;

	ret = lib.dm_block_write(WORDS_ID);
	if(ret == -1) 
		cerr << "Sincronizzazione del contenuto locale del blocco con il contenuto globale fallita" << endl;	
	if(ret == 2) {
		cerr << "Errore: blocco non associato ad alcun indirizzo di memoria locale" << endl;	
		cerr << "Sincronizzazione del contenuto locale del blocco con il contenuto globale fallita" << endl;	
	}
	if(ret == 3) {
		cerr << "Errore: copia locale del blocco invalida" << endl;	
		cerr << "Sincronizzazione del contenuto locale del blocco con il contenuto globale fallita" << endl;	
	}

	*syncr2 += 1;

	ret = lib.dm_block_write(SYNCR2_ID);
	if(ret == -1) 
		cerr << "Sincronizzazione del contenuto locale del blocco con il contenuto globale fallita" << endl;	
	if(ret == 2) {
		cerr << "Errore: blocco non associato ad alcun indirizzo di memoria locale" << endl;	
		cerr << "Sincronizzazione del contenuto locale del blocco con il contenuto globale fallita" << endl;	
	}
	if(ret == 3) {
		cerr << "Errore: copia locale del blocco invalida" << endl;	
		cerr << "Sincronizzazione del contenuto locale del blocco con il contenuto globale fallita" << endl;	
	}


	ret = lib.dm_block_update(SYNCR1_ID);
	if(ret == -1)
		cerr << "Aggiornamento del contenuto della copia locale del blocco fallito" << endl;
	
	if(*syncr1 < 4) {
		ret = lib.dm_block_wait(SPACES_ID);	
		if(ret == -1)
			cerr << "Controllo della validita' della copia locale del blocco fallito" << endl;
	}

	ret = lib.dm_block_update(SPACES_ID);
	if(ret == -1)
		cerr << "Aggiornamento del contenuto della copia locale del blocco fallito" << endl;
	
	
	lib.dm_block_unmap(WORDS_ID);
	lib.dm_block_unmap(SPACES_ID);
	lib.dm_block_unmap(LENGHT_ID);
	lib.dm_block_unmap(SYNCR1_ID);
	lib.dm_block_unmap(SYNCR2_ID);
	for(int i = 0; i < num_block; i++)
		lib.dm_block_unmap(i + 5);

	cout << "Number of SPACES --> " << *spaces << endl;
	cout << "Number of WORDS  --> " << *words << endl;

	return 1;
}


