/*
	Instituição: UERGS
	Curso: Engenharia de computação
	Data: 10/11/2017
	Autor: Guilherme Lague
	E-mail: guilhermelaguebmx@gmail.com
	Descrição: Implementar VSFTP (Very Simple File Transfer Protocol)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

struct sockaddr_in server;

int s0, status, sock_server_size = sizeof(server);
char server_ip[16];

#define PORT 64000
#define OK 0
#define UPLOAD 1
#define DOWNLOAD 2
#define DATA 3
#define ERROR 4
#define MAXBUFF 512

// protótipo das funções
void conn();
void *transfer();
void disconnect();

//=============================================================================
// Função principal
//=============================================================================
int main(int argc, char *argv[]){
	pthread_t th1;

	conn();

	pthread_create(&th1, NULL, transfer, NULL);
    pthread_join(th1, NULL);

	disconnect();
	return 0;
}

//=============================================================================
// Esta função cria um ponto de comunicação socket.
//=============================================================================
void conn(){
	FILE *file;

	memset(&server, 0, sock_server_size);

	s0 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	//============================================================================
    if(bind(s0, (struct sockaddr *)&server, sock_server_size) == -1){
        perror("Bind");
        exit(1);
    }
    else{
        printf("Bind ok!\n");
    }

    //============================================================================
    if(getsockname(s0, (struct sockaddr *)&server, &sock_server_size) == -1){
    	perror("Get sock name");
    	exit(1);
    }
    else{
    	printf("Port: %d\n", htons(server.sin_port));	
    }

    //===========================================================================
    system("caminho=`pwd`\"/ip.txt\"; hostname -I | cut -d ' ' -f 1 > $caminho");
	
	file = fopen ("ip.txt", "r");
    while(!feof(file)){
		fscanf(file, "%s", &server_ip);
	}

	printf("IP: %s\n", server_ip);
}

//=============================================================================
// Esta função é responsavel pela troca de dados com o cliente
//=============================================================================
void *transfer(){
	char buffer[MAXBUFF], ip_address[16], file_name[20];
	int op, dest, src, cont;
	
	while(1){
		recvfrom(s0, &op, sizeof(op), 0, (struct sockaddr *)&server, &sock_server_size);
		recvfrom(s0, ip_address, sizeof(ip_address), 0, (struct sockaddr *)&server, &sock_server_size);
		recvfrom(s0, file_name, sizeof(file_name), 0, (struct sockaddr *)&server, &sock_server_size);

		if(strcmp(ip_address, server_ip) == 0){// aqui vai verificar se o ip enviado é igual ao do servidor, se sim, o servidor vai receber os dados da maquina que requisitou o upload.
			if(op == UPLOAD){
				printf("Cliente %s requisitou UPLOAD.\n", inet_ntoa(server.sin_addr));
				
				dest = creat(file_name, 0666);

				if(dest == -1){
					op = ERROR;
			        printf("Impossivel criar o arquivo \"%s\"\n", file_name);
			        sendto(s0, &op, sizeof(op), 0, (struct sockaddr *)&server, sock_server_size);	
			        sendto(s0, "Impossivel criar o arquivo no servidor!", 40, 0, (struct sockaddr *)&server, sock_server_size);	 
			        
			    }
			    else{
			    	op = OK;
			    	sendto(s0, &op, sizeof(op), 0, (struct sockaddr *)&server, sock_server_size);	
			        sendto(s0, "Iniciando o upload...", 40, 0, (struct sockaddr *)&server, sock_server_size);	 
			    	
			    	while((cont = recvfrom(s0, &buffer, sizeof(buffer), 0, (struct sockaddr *)&server, &sock_server_size)) > 0 ){
			        	write(dest, &buffer, cont);
			        	if(cont < 512){
			        		break;
			        	}
			    	}		
			    }
			}
			else{
				printf("Cliente %s requisitou DOWNLOAD.\n", inet_ntoa(server.sin_addr));

				src = open(file_name, O_RDONLY);
		    
			    if(src == -1){
			    	op = ERROR;
			        printf("Impossivel abrir o arquivo \"%s\"\n", file_name);
			        sendto(s0, &op, sizeof(op), 0, (struct sockaddr *)&server, sock_server_size);	
			        sendto(s0, "Impossivel acessar o arquivo! O nome pode estar errado ou talvez ele esteja indisponivel.", 90, 0, (struct sockaddr *)&server, sock_server_size);	
			    }
			    else{
			    	op = OK; 
			    	sendto(s0, &op, sizeof(op), 0, (struct sockaddr *)&server, sock_server_size);	
			        sendto(s0, "Iniciando download...", 22, 0, (struct sockaddr *)&server, sock_server_size);	
			    	while((cont = read(src, &buffer, sizeof(buffer))) > 0){
			        	sendto(s0, buffer, cont, 0, (struct sockaddr *)&server, sock_server_size);	
			    	}	
			    }
			}
		}
		else{
			printf("Uma requisição de upload ou download foi enviada para outra maquina.\n");
		}
	}
}

//=============================================================================
// Esta função fecha a comunicação via socket
//=============================================================================
void disconnect(){
	shutdown(s0, 2);
	close(s0);
}