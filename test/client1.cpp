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
	int text_size; /* Lunghezza in bytes del file di test */
	int num_block; /* Numero di blocchi necessari per il file di test */
	size_t num_elem; /* Numero di elementi presenti nel file di test */
	char *tmp;
	Library lib;

	lib.dm_init(argv[1]);

	FILE *ft = fopen("text.txt", "r");
	if(ft == NULL) {
		cerr << "Errore nell'apertura del file di test" << endl;
		exit(0);
	}

	fseek(ft, 0, SEEK_END);
	text_size = ftell(ft); 
	rewind(ft);
	
	if(text_size % DIMBLOCK == 0) 
		num_block = text_size / DIMBLOCK;
	else
		num_block = (text_size / DIMBLOCK) + 1;

	char *buffer = (char *)malloc(num_block * DIMBLOCK);

	bzero(buffer, DIMBLOCK*num_block);

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
		cerr << "Associazione tra blocco di memoria e indirizzo di memoria locale fallita c1 words" << endl;

	ret = lib.dm_block_map(SPACES_ID, spaces);
	if(ret == -1)
		cerr << "Associazione tra blocco di memoria e indirizzo di memoria locale fallita c1 spaces" << endl;
	
	ret = lib.dm_block_map(LENGHT_ID, lenght);
	if(ret == -1)
		cerr << "Associazione tra blocco di memoria e indirizzo di memoria locale fallita c1 lenght" << endl;

	ret = lib.dm_block_map(SYNCR1_ID, syncr1);
	if(ret == -1)
		cerr << "Associazione tra blocco di memoria e indirizzo di memoria locale fallita c1 syncr1" << endl;

	ret = lib.dm_block_map(SYNCR2_ID, syncr2);
	if(ret == -1)
		cerr << "Associazione tra blocco di memoria e indirizzo di memoria locale fallita c1 syncr2" << endl;

	for(int i = 0; i < num_block; i++) {
		ret = lib.dm_block_map(i + 5, buffer + (i * DIMBLOCK));
		if(ret == -1)
			cerr << "Associazione tra blocco di memoria e indirizzo di memoria locale fallita c1 blocco "<<(i+5)<< endl;
	}

	*lenght = num_block;

	ret = lib.dm_block_write(LENGHT_ID);
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

	*syncr1 += 1;

	ret = lib.dm_block_write(SYNCR1_ID);
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
	

	num_elem = fread(buffer, sizeof(char), text_size, ft);

	for(int i = 0; i < num_block; i++) {
		ret = lib.dm_block_write((i + 5));
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

		*syncr1 += 1;

		ret = lib.dm_block_write(SYNCR1_ID);
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
			
	}

	*spaces = 0;
	
	tmp = strtok(buffer, " ");
	if(tmp != NULL) 		
		while((tmp = strtok(NULL, " ")))
			*spaces += 1;

	ret = lib.dm_block_write(SPACES_ID);
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
	
	*syncr1 += 1;

	ret = lib.dm_block_write(SYNCR1_ID);
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


	ret = lib.dm_block_update(SYNCR2_ID);
	if(ret == -1)
		cerr << "Aggiornamento del contenuto della copia locale del blocco fallito" << endl;
	
	if(*syncr2 < 1) {
		ret = lib.dm_block_wait(WORDS_ID);	
		if(ret == -1)
			cerr << "Controllo della validita' della copia locale del blocco fallito" << endl;
	}

	ret = lib.dm_block_update(WORDS_ID);
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


