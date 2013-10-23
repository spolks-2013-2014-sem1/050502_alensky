#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

int createSocket( const char * af_domen, const char * protocol);
void bindSocket(int _socket, struct sockaddr_in* addr,const char * af_domen, const char * port );
char* getClientInfo( struct hostent clientInfo );
void error(char *msg);

int main(int argc, char *argv[])
{
        int client, listener, countOfAccesibleClients = 0;
        int accepted_bytes;
		struct sockaddr_in addr;
        struct sockaddr_in clientaddr;
        struct hostent* clientInfo;
        char accepted_data[1024];
        char *clientInfoName;
        
        if (argc < 2)
        {
                error("ERROR, no parametres provided");
        }

        listener = createSocket( argv[1], argv[2]);
        
        bindSocket( listener, &addr, argv[1], argv[3] );
		printf("Starts at port %s", argv[3]);
		fflush(stdout);
        countOfAccesibleClients = atoi(argv[4]);
        listen(listener, countOfAccesibleClients);

        while(1)
		{
					clientaddr.sin_family = AF_INET;

					socklen_t client_addr_len = sizeof( clientaddr );
					
					client = accept(listener, (struct sockaddr*)& clientaddr, &client_addr_len );
					if( client < 0 )
					{
							continue;
					}
					clientInfo = gethostbyaddr( &clientaddr.sin_addr, sizeof ( struct in_addr ) , AF_INET);
					clientInfoName = getClientInfo(*clientInfo);
					send(client, clientInfoName, strlen( clientInfoName ), 0);
					while(1)
					{
						accepted_bytes = recv(client, accepted_data, 1024, 0);
						if( accepted_bytes <= 0) break;
						send(client, accepted_data, accepted_bytes, 0);
					}
			free(clientInfoName);
			close(client);
		}

        close(listener);
        return 1;
}

int createSocket( const char * af_domen, const char * protocol)
{
        int _socket;
        int sock_type;
        int _af_domen;
        
        if(af_domen == NULL )
        {
                error( "ERROR, impossible to create socket" );
        }
        
        if( !strcmp( af_domen, "INET" ) )
        {
                _af_domen = AF_INET;
                sock_type = SOCK_STREAM;        // protocol is TCP ( by default )
                
                if( protocol != NULL)
                {
                        if( !strcmp(protocol,"UDP") )
                        {
                                sock_type = SOCK_DGRAM;
                        }
                        
                        if( !strcmp(protocol,"IP") )
                        {
                                sock_type = SOCK_RAW;
                        }
                }
        }
        
        if( !strcmp( af_domen, "UNIX" ) )
        {
                _af_domen = AF_UNIX;
                sock_type = SOCK_STREAM;        // the only available value for UNIX adress family
        }
        
        _socket = socket( _af_domen, sock_type, 0 );
        
        if( _socket < 0 )
        {
                error( "ERROR, impossible to create socket" );
        }
        
        return _socket;
}

void bindSocket(int _socket, struct sockaddr_in* addr,const char * af_domen, const char * port )
{
        if( af_domen == NULL )
        {
                error( "ERROR, impossible to bind socket" );
        }
        
        if( !strcmp( af_domen, "INTERNET" ) )
        {
                addr->sin_family = AF_INET;
        }
        
        if( !strcmp( af_domen, "UNIX" ) )
        {
                addr->sin_family = AF_UNIX;
        }
        
        if( port == NULL )
        {
                error("Error, port is not pointed");
        }
        else
        {
                addr->sin_port = htons( atoi(port) );
        }
        
		addr->sin_addr.s_addr = htonl( INADDR_ANY );
        
        if( bind( _socket,(struct sockaddr *) addr, sizeof( *addr ) ) < 0 )
        {
                error( "ERROR, impossible to bind socket" );
        }
}

char* getClientInfo( struct hostent clientInfo )
{
        char *preInfo = "Your are ";
        char *postInfo = "\nType something (Echo server is working)\n";
        char* info = ( char* ) malloc( sizeof(char) * ( strlen( preInfo ) + strlen ( clientInfo.h_name ) + strlen( postInfo ) + 1 ) );
        strcpy( info, preInfo);
        strcat(info, clientInfo.h_name);
        strcat(info, postInfo);
        return info;
}

void error(char *msg)
{
  perror(msg);
  exit(1);
}
