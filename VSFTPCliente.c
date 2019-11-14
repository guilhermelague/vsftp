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

struct sockaddr_in client, server;

int s0, status, sock_client_size = sizeof(client);

#define PORT 64000
#define OK 0
#define UPLOAD 1
#define DOWNLOAD 2
#define DATA 3
#define ERROR 4
#define MAXBUFF 512

// prototipo das funções
void conn();
void transfer();
void disconnect();

//=============================================================================
// Função principal.
//=============================================================================
int main(int argc, char *argv[]){
	conn();
	transfer();
	disconnect();
	return 0;
}

//=============================================================================
// Esta função cria um ponto de comunicação socket.
//=============================================================================
void conn(){
	int yes = 1;
	memset(&client, 0, sock_client_size);
	
	s0 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	client.sin_family = AF_INET;
	client.sin_port = htons(PORT);
	client.sin_addr.s_addr = htonl(-1);
	
    //============================================================================
	if(setsockopt(s0, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int)) == -1){
		perror("Setsockopt Status");
        exit(1);
	}
	else{
		printf("Setsockopt Status = %d\n", status);	
	}
}

//=============================================================================
// Esta função é responsavel pela troca de dados com o servidor
//=============================================================================
void transfer(){
	char buffer[MAXBUFF], ip_address[16], file_name[20];
	int op, dest, src, cont, quadros = 0;

	printf("1 - UPLOAD\n");
	printf("2 - DOWNLOAD\n");
	printf("Opção: ");
	scanf("%d", &op);
	printf("Digite o IP da maquina: ");
	scanf("%s", &ip_address);
	printf("Digite o nome do arquivo: ");
	scanf("%s", &file_name);
	
	if(op == UPLOAD){
		sendto(s0, &op, sizeof(op), 0, (struct sockaddr *)&client, sock_client_size); //manda opção
		sendto(s0, ip_address, sizeof(ip_address), 0, (struct sockaddr *)&client, sock_client_size); //manda o ip
		sendto(s0, file_name, sizeof(file_name), 0, (struct sockaddr *)&client, sock_client_size); //manda o nome de arquivo de upload
		printf("Requisição de UPLOAD enviada.\n");

		src = open(file_name, O_RDONLY);
    
	    if(src == -1){
	        printf("Impossivel abrir o arquivo \"%s\", verifique se o o nome e o caminho estão corretos!\n", file_name);
	        exit (1); 
	    }
	    else{
	    	cont = recvfrom(s0, &op, sizeof(op), 0, (struct sockaddr *)&client, &sock_client_size); // flag ok ou erro
    		cont = recvfrom(s0, buffer, sizeof(buffer), 0, (struct sockaddr *)&client, &sock_client_size); //mensagem de ok ou erro

    		if(op == ERROR){
	    		printf("%s\n", buffer); // imprime mensagem de erro
	    		exit (1);
			}
			else{
				printf("%s\n", buffer);
				while((cont = read(src, &buffer, sizeof(buffer))) > 0 ){
	    			sendto(s0, buffer, cont, 0, (struct sockaddr *)&client, sock_client_size);
	    			quadros++;	
	    		}
	    		printf("Upload do arquivo \"%s\" foi realizado com exito!\n", file_name);
	    		printf("%d quadros foram enviados.\n", quadros);
	    	}				
	    }	
	}
	else if(op == DOWNLOAD){
		sendto(s0, &op, sizeof(op), 0, (struct sockaddr *)&client, sock_client_size);//manda opção
		sendto(s0, &ip_address, sizeof(ip_address), 0, (struct sockaddr *)&client, sock_client_size); //manda o ip
		sendto(s0, file_name, sizeof(file_name), 0, (struct sockaddr *)&client, sock_client_size); //manda o nome de arquivo de upload
		printf("Requisição de DOWNLOAD enviada.\n");

		cont = recvfrom(s0, &op, sizeof(op), 0, (struct sockaddr *)&client, &sock_client_size); // flag ok ou erro
    	cont = recvfrom(s0, buffer, sizeof(buffer), 0, (struct sockaddr *)&client, &sock_client_size); //mensagem de ok ou erro

    	if(op == ERROR){
    		printf("%s\n", buffer); // imprime mensagem de erro
    		exit (1);
    	}
    	else{
			dest = creat(file_name, 0666);
    		
    		if(dest == -1){
		        printf("Impossivel criar o arquivo \"%s\"\n", file_name);
		        exit (1);
	    	}
			else{
				printf("%s\n", buffer);
				while((cont = recvfrom(s0, buffer, sizeof(buffer), 0, (struct sockaddr *)&client, &sock_client_size)) > 0 ){
		        	write(dest, &buffer, cont);
		        	quadros++;
		        	if(cont < 512){
		        		printf("Download do arquivo \"%s\" foi realizado com exito!", file_name);
		        		printf("%d quadros foram recebidos.\n", quadros);
	    				break;
		        	}
    			}
			}
		}
	}
	else{
		printf("Opção invalida!.\n");	
	}
}

//=============================================================================
// Esta função fecha a comunicação via socket
//=============================================================================
void disconnect(){
	shutdown(s0, 2);
	close(s0);
}
