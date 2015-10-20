/*******************************************************
Protocolos de Transporte
Grado en Ingenier�a Telem�tica
Dpto. Ingen�er�a de Telecomunicaci�n
Univerisdad de Ja�n

Fichero: servidor.c
Versi�n: 1.0
Fecha: 23/09/2012
Descripci�n:
	Servidor de eco sencillo TCP.

Autor: Juan Carlos Cuevas Mart�nez

*******************************************************/
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <Winsock2.h>
#include "protocol.h"
#include <stdlib.h>




main()
{

	//Declaraci�n de variables
	WORD wVersionRequested;
	WSADATA wsaData;
	SOCKET sockfd,nuevosockfd;
	struct sockaddr_in  local_addr,remote_addr;
	char buffer_out[1024],buffer_in[1024], cmd[10], usr[10], pas[10],NUM1[5],NUM2[5];
	int err,tamanio, intentos=0;
	int fin=0, fin_conexion=0;
	int recibidos=0,enviados=0;
	int estado=0;
	system("color 1F");


	/** INICIALIZACION DE BIBLIOTECA WINSOCK2 **
	 ** OJO!: SOLO WINDOWS                    **/
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0){
		return(-1);
	}
	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1){
		WSACleanup() ;
		return(-2);
	}
	/** FIN INICIALIZACION DE BIBLIOTECA WINSOCK2 **/


	sockfd=socket(AF_INET,SOCK_STREAM,0);	//Creaci�n del socket

	if(sockfd==INVALID_SOCKET)		//Control de errores
	{
		return(-3);
	}
	else {
		local_addr.sin_family		=AF_INET;					// Familia de protocolos de Internet
		local_addr.sin_port			=htons(TCP_SERVICE_PORT);	// Puerto del servidor
		local_addr.sin_addr.s_addr	=htonl(INADDR_ANY);			// Direccion IP del servidor Any cualquier disponible
																// Cambiar para que conincida con la del host
	}
	
	// Enlace el socket a la direccion local (IP y puerto)
	if(bind(sockfd,(struct sockaddr*)&local_addr,sizeof(local_addr))<0)
		return(-4);
	
	//Se prepara el socket para recibir conexiones y se establece el tama�o de cola de espera
	if(listen(sockfd,5)!=0)
		return (-6);
	
	tamanio=sizeof(remote_addr);

	do
	{
		printf ("SERVIDOR> ESPERANDO NUEVA CONEXION DE TRANSPORTE\r\n");
		
		//aceptacion de una conexi�n
		nuevosockfd=accept(sockfd,(struct sockaddr*)&remote_addr,&tamanio);	

		if(nuevosockfd==INVALID_SOCKET) {
			
			return(-5);
		}

		printf ("SERVIDOR> CLIENTE CONECTADO\r\nSERVIDOR [IP CLIENTE]> %s\r\nSERVIDOR [CLIENTE PUERTO TCP]>%d\r\n",
					inet_ntoa(remote_addr.sin_addr),ntohs(remote_addr.sin_port));

		//Mensaje de Bienvenida
		sprintf_s (buffer_out, sizeof(buffer_out), "%s Bienvenindo al servidor de ECO%s",OK,CRLF);
		enviados=send(nuevosockfd,buffer_out,(int)strlen(buffer_out),0);

		//TODO Comprobar error de env�o
		if (enviados==0){
				printf("SERVIDOR> Conexion liberada correctamente\r\n");
				return 0;
			}
			else if (enviados==SOCKET_ERROR){
				printf("SERVIDOR> ERROR EN EL ENVIO\r\n");
				continue;
			}

		//Se reestablece el estado inicial
		estado = S_USER;
		fin_conexion = 0;
		intentos = 0;

		printf ("SERVIDOR> Esperando conexion de aplicacion\r\n");
		do
		{
			//Se espera un comando del cliente
			recibidos = recv(nuevosockfd,buffer_in,1023,0);

			//TODO Comprobar posible error de recepci�n
			if (recibidos==0){
				printf("SERVIDOR> CONEXION LIBERADA CORRECTAMENTE\r\n");
				return 0;
			}
			else 
			
			if (recibidos==SOCKET_ERROR){
				printf("SERVIDOR> ERROR EN LA RECEPCION\r\n");
				break;
			}

			buffer_in[recibidos] = 0x00;
			printf ("SERVIDOR [bytes recibidos]> %d\r\nSERVIDOR [datos recibidos]>%s", recibidos, buffer_in);
			
			switch (estado)
			{

				case S_USER:    /*****************************************/
					strncpy_s ( cmd, sizeof(cmd),buffer_in, 4);
					cmd[4]=0x00; // en C los arrays finalizan con el byte 0000 0000

					if ( strcmp(cmd,SC)==0 ) // si recibido es solicitud de conexion de aplicacion
					{
						sscanf_s (buffer_in,"USER %s\r\n",usr,sizeof(usr));
						
						// envia OK acepta todos los usuarios hasta que tenga la clave
						sprintf_s (buffer_out, sizeof(buffer_out), "%s%s", OK,CRLF);
						
						estado = S_PASS;
						printf ("SERVIDOR> Esperando clave\r\n");
					} else
					if ( strcmp(cmd,SD)==0 )
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexi�n%s", OK,CRLF);
						fin_conexion=1;
					}
					else
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF);
					}
				break;

				case S_PASS: /******************************************************/

					
					strncpy_s ( cmd, sizeof(cmd), buffer_in, 4);
					cmd[4]=0x00; // en C los arrays finalizan con el byte 0000 0000
					
					if ( strcmp(cmd,PW)==0 ) // si comando recibido es password
					{
						sscanf_s (buffer_in,"PASS %s\r\n",pas,sizeof(pas));

						if ( (strcmp(usr,USER)==0) && (strcmp(pas,PASSWORD)==0) ) // si password recibido es correcto
						{
							// envia aceptacion de la conexion de aplicacion, nombre de usuario y
							// la direccion IP desde donde se ha conectado
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s IP(%s)%s", OK, usr, inet_ntoa(remote_addr.sin_addr),CRLF);
							estado = S_DATA;
							printf ("SERVIDOR> Esperando comando\r\n");
						}
						else{
							intentos++;
							estado = S_USER;
							sprintf_s (buffer_out, sizeof(buffer_out), "%s Autenticacion erronea intentos %i%s",ER,intentos,CRLF);	//Formato del envio de numero de errores

							if (intentos==3){		//fin de conexion a los tres intentos
								fin_conexion=1;
								}
						}
					} 
					else if ( strcmp(cmd,SD)==0 )
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexi�n%s", OK,CRLF); //formato de fin de conexi�n
							fin_conexion=1;
						}
					else
					{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF); //formato de fin de conxi�n
					}
				break;

				case S_DATA: /***********************************************************/
					
					buffer_in[recibidos] = 0x00;

					strncpy_s(cmd,sizeof(cmd), buffer_in, 4);
					cmd[4]=0x00;

					printf ("SERVIDOR [Comando]>%s\r\n",cmd);

					//Comando SUM
					if ( strncmp(cmd,SUM,3)==0){
						int NUMi1,NUMi2,SUMA;
						sscanf_s(buffer_in,"SUM %s %s\r\n",NUM1,sizeof(NUM1),NUM2,sizeof(NUM2));  //Escaneo de los dos numeros 
						NUMi1 = atoi(NUM1);	// string to int 
						NUMi2 = atoi(NUM2); // string to int
						SUMA = NUMi1+NUMi2;	// suma de los dos numeros
						sprintf_s(buffer_out, sizeof(buffer_out), "%s %i%s",OK,SUMA,CRLF);	//Formato de envio del resultado
					}

					//Comando HELP
					else if ( strcmp(cmd,HELP)==0 || strcmp(cmd,"help")==0){
						sprintf_s(buffer_out, sizeof(buffer_out), "%s\tDespliega el menu de ayuda\n%s\tSuma dos numeros de maximo 4 digitos\n%s\tFinaliza la conexion cliente-servidor\n%s\tFinaliza el servidor%s",HELP,SUM,SD,SD2,CRLF);
					}

					//Comando Quit
					else if ( strcmp(cmd,SD)==0 )
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexi�n%s", OK,CRLF);
						fin_conexion=1;
					}

					//Comando EXIT
					else if (strcmp(cmd,SD2)==0)
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Finalizando servidor%s", OK,CRLF);
						fin_conexion=1;
						fin=1;
					}

					//Resto de comandos
					else
					{
						sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF);
					}
					break;
				
				default:
					break;
					
			} // switch
			enviados=send(nuevosockfd,buffer_out,(int)strlen(buffer_out),0);

			//TODO (Correci�n de errores de envio)
			if (enviados==0){
				printf("SERVIDOR> CONEXION LIBERADA CORRECTAMENTE\r\n");
				return 0;
			}
			else if (enviados==SOCKET_ERROR){
				printf("SERVIDOR> ERROR EN EL ENVIO\r\n");
				break;
			}

		} while (!fin_conexion);

		//Cerrando conexi�n
		printf ("SERVIDOR> CERRANDO CONEXION DE TRANSPORTE\r\n");
		shutdown(nuevosockfd,SD_SEND);
		closesocket(nuevosockfd);

	}while(!fin);

	//Cierre del servidor
	printf ("SERVIDOR> CERRANDO SERVIDOR\r\n");

	return(0);
} 
