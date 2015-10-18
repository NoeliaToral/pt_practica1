/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: cliente.c
Versión: 1.1
Fecha: 17/10/2015
Descripción:
	Cliente de eco sencillo TCP mejorado.

Autor: Juan Carlos Cuevas Martínez, Emilio Sánchez Catalán, Noelia Toral Jimenez

*******************************************************/
#include <stdio.h>
#include <winsock.h>
#include <time.h>
#include <conio.h>

#include "protocol.h"





int main(int *argc, char *argv[])
{
	//Declaracion de variables
	SOCKET sockfd;
	struct sockaddr_in server_in;
	char buffer_in[1024], buffer_out[1024],input[1024],NUM1[5],NUM2[5],SOL[6];
	int recibidos=0,enviados=0;
	char intentos[2];
	int fallo_len=0;
	int estado=S_HELO;
	char option;

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

    char ipdest[16];
	char default_ip[16]="127.0.0.1";
	system("color 0F");
	//Inicialización Windows sockets
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1)
	{
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	do{
		system("cls");
		
		sockfd=socket(AF_INET,SOCK_STREAM,0);	//Creación del socket

		if(sockfd==INVALID_SOCKET)//Comprobación de posible fallo				
		{
			printf("CLIENTE> ERROR AL CREAR SOCKET\r\n");
			exit(-1);
		}
		else
		{
			printf("CLIENTE> SOCKET CREADO CORRECTAMENTE\r\n");

			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets(ipdest);

			if(strcmp(ipdest,"")==0)	//Caso por defecto
				strcpy(ipdest,default_ip);


			server_in.sin_family=AF_INET;	//Familia IP
			server_in.sin_port=htons(TCP_SERVICE_PORT);		//Puerto Que vamos a usar
			server_in.sin_addr.s_addr=inet_addr(ipdest);	//IP del servidor
			
			estado=S_HELO;	//Estado inicial de saludo
		
			// establece la conexion de transporte
			if(connect(sockfd,(struct sockaddr*)&server_in,sizeof(server_in))==0)
			{
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			
		
				//Inicio de la máquina de estados
				do{
					switch(estado)
					{
					case S_HELO:
						// Se recibe el mensaje de bienvenida
						break;
					case S_USER:
						// establece la conexion de aplicacion 
						printf("CLIENTE> Introduzca el usuario (enter para salir): ");
						gets(input);
						if(strlen(input)==0)	//Enter para salir
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF); //Formato para Cerrar la Conexión
							estado=S_QUIT;
						}
						else sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",SC,input,CRLF);  //Formato de envio de usuario
						break;
					case S_PASS:
						//Contraseña
						printf("CLIENTE> Introduzca la clave (enter para salir): ");
						gets(input);
						if(strlen(input)==0)	//Enter para salir
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF); //Formato para Cerrar la Conexion
							estado=S_QUIT;
						}
						else
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",PW,input,CRLF);	//Formato de envio de contraseña
						break;
					case S_DATA:
						//Funcionalidad del servidor
						printf("CLIENTE> Introduzca datos (enter o QUIT para salir): ");
						gets(input);
						if(strlen(input)==0)	//Enter para salir
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);		//Formato para Cerrar la Cnexion
							estado=S_QUIT;
						}
						else{
							if (strncmp(input,SUM,3)==0){		//Comando Sum

								//Numero 1
								do{
									fallo_len=0;
									printf("CLIENTE> Introduce el primer numero: ");
									gets(NUM1);
									if(strlen(NUM1)>4){		//Comprobación del tamaño
										printf("CLIENTE> %s MAXIMO 4 DIGITOS%s",ER,CRLF);
										fallo_len=1;
									}
								}while(fallo_len==1);

								//Numero 2
								do{
									fallo_len=0;
									printf("CLIENTE> Introduce el segundo numero: ");
									gets(NUM2);
									if(strlen(NUM2)>4){		//Comprobacion del tamaño
										printf("CLIENTE> %s MAXIMO 4 DIGITOS%s",ER,CRLF);
										fallo_len=1;
									}
								}while(fallo_len==1);

								//Formato de envio
								sprintf_s(buffer_out,sizeof(buffer_out),"SUM %s %s%s",NUM1,NUM2,CRLF);
							}
							
							//Comando EXIT
							else if(strncmp(input,SD2,4)==0){	//Correcion del Fallo del comando EXIT
								sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF);	//Formato de envio del EXIT
								estado = S_QUIT;
							}

							//Comando QUIT
							else if(strncmp(input,SD,4)==0){  //Correcion del Fallo del comando QUIT
								sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF);	//Formato de envio de QUIT
								estado = S_QUIT;
							}

							//Resto de comandos
							else sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input,CRLF); //Formato del resto de comandos
						}
						break;
				
					}

					//Envio
					if(estado!=S_HELO)
					// Ejercicio: Comprobar el estado de envio
						
						enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0);

					//Recibo
					recibidos=recv(sockfd,buffer_in,512,0);

					if(recibidos<=0){
						DWORD error=GetLastError();
						if(recibidos<0 && strncmp(input,SD2,4)!=0 && strncmp(input,SD,4)!=0)
						{
							printf("CLIENTE> Error %d en la recepción de datos\r\n",error);
							estado=S_QUIT;
						}
						else
						{
							printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							estado=S_QUIT;
						
					
						}
					}

					else
					{
						buffer_in[recibidos]=0x00;
						
						//Presentacion de la Solucion de la suma
						if (strncmp(input,SUM,3)==0){
							sscanf_s(buffer_in,"OK %s",SOL,sizeof(SOL));
							printf("CLIENTE> Resultado = %s%s",SOL,CRLF);
						}

						//Presentacion del resto de datos recibidos
						else printf(buffer_in);

						//Solución a la autentificacion erronea
						sscanf_s(buffer_in,"ERROR Autenticacion erronea intentos %s\r\n",intentos,sizeof(intentos));	//formato de recepción de intentos
						if (strncmp(buffer_in,ER,5)==0 && estado==S_PASS){	//En caso de reccibir un error despues de enviar la contraseña
							estado = S_USER;								//volvemos al estado usuario
							if (strncmp(intentos,"3",1)==0){				//Al tercer intento
								estado = S_QUIT;							//Se Cierra la conexión
							}	
						}

						//Avance de la maquina de estados
						else if(estado!=S_DATA && strncmp(buffer_in,OK,2)==0){
							estado++;
						}
						
					}

				}while(estado!=S_QUIT);
			}
			else	//Error al Conectar
			{
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			}		
			// fin de la conexion de transporte
			closesocket(sockfd);
			
		}	
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option=_getche();	//realizar otra conexión

	}while(option!='n' && option!='N');

	
	
	return(0);

}
