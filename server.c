#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <string.h>

#define die(e) do { fprintf(stderr, "%s\n", e); exit(EXIT_FAILURE); } while (0);

int PORT;

void replaceAll(char * str, char oldChar, char newChar);

void respond (int sock);

void sendall(int sock, char* msg) {
  int length = strlen(msg);
  int bytes;
  while(length > 0) {
    bytes = send(sock, msg, length, 0);
    length = length - bytes;
  }
}

void usage(){
  printf("usage : server -p <portno> -r.\n-p : specify port number, Default is : 3838\n-h : to display this help message.\n");
}

int main(int argc, char **argv) {
  // Bash execution argument parsing
  int opt;
  bool lan_restrict = true;
  int r_flag = 0;
  int p_flag = 0;
  int h_flag = 0;
  while ((opt = getopt(argc, argv, "hpr")) != -1) {
        switch (opt) {
            case 'p': p_flag++;break;
            case 'h': h_flag++;usage();exit(1);break;
            case 'r': r_flag++;lan_restrict = false;break;
            default:usage();exit(1);
        }
  }
  if (p_flag==0) PORT = 3838;
  if (p_flag==1 && h_flag==0 && r_flag==1) PORT = atoi(argv[3]);
  if (p_flag==1 && h_flag==0 && r_flag==0) PORT = atoi(argv[2]);
  if (p_flag==1 && h_flag==1) exit(1);
  if (PORT == 0) exit(1);
  if (r_flag==1) printf("Lan restriction deactivated\n");
  if (r_flag==0) printf("Lan restriction activated\n");
  int newsockfd[50];
  int sockfd, portno = PORT;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  clilen = sizeof(cli_addr);

  // First call to socket() function
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  // port reusable
  int tr = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
  }

  // Initialize socket structure
  bzero((char *) &serv_addr, sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  // Binding the host address using bind() call
  if ( bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1 ){
    perror("bind error");
    exit(1);
  }

  // listen on socket created

  if ( listen(sockfd, 20) == -1 ){
    perror("listen error");
    exit(1);
  }

  printf("Server is running on port %d\n", portno);
  int client_count = 0;
  while (1) {
    newsockfd[client_count] = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if ( newsockfd[client_count] == -1 ){
      perror("accept error");
      exit(1);
    }
  struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&cli_addr;
  struct in_addr ipAddr = pV4Addr->sin_addr;
  char addr[INET_ADDRSTRLEN];
  inet_ntop( AF_INET, &ipAddr, addr, INET_ADDRSTRLEN );
  // Accepting connections
  if (strcmp(addr, "127.0.0.1")==0) {
	respond(newsockfd[client_count]);
        client_count++;
  }
  else if (strcmp(addr, "127.0.0.1") != 0 && !lan_restrict)
  {
        respond(newsockfd[client_count]);
        client_count++;
  }
  else
  {
        printf("[*]An outsider client tried to connect! : %s\n[+]Connection terminated\n", addr);
  }
  }

  return 0;
}

void respond(int sock) {
	int offset, bytes;
	char buffer[9000];
	bzero(buffer,9000);
	char cmd[30];
	offset = 0;
	bytes = 0;
        // Parsing the client's request
	do {
	    // bytes < 0 : unexpected error
	    // bytes == 0 : client closed connection
	    bytes = recv(sock, buffer + offset, 1500, 0);
            offset += bytes;
	    char *result = strstr(buffer, "HTTP/1.1");
            int position = result - buffer;
            for (int i=5; i<position-1; i++)
            {
                cmd[i-5] = buffer[i];
            }
            if (strcmp(cmd, "favicon.ico") != 0){ printf("%s", cmd);}
            else {memset(cmd,0,strlen(cmd));}
	    if (strncmp(buffer + offset - 4, "\r\n\r\n", 4) == 0) break;
	} while (bytes > 0);
        replaceAll(cmd, '@', ' ');
        replaceAll(cmd, '#', '"');
        printf("%sx\n", cmd);
	// Executing command in OS command line

        char output[10000];
	FILE *pp;
	    pp = popen(cmd, "r");
	    if (pp != NULL) {
	        while (1) {
	           char *line;
	           char buf[1000];
	           line = fgets(buf, sizeof(buf), pp);
	           if (line == NULL) break;
	              strcat(output, line);
	           }
	       pclose(pp);
		}
        memset(cmd,0,strlen(cmd));
	
       if (bytes < 0) {
		printf("recv() error\n");
		return;
	} else if (bytes == 0) {
		printf("Client disconnected unexpectedly\n");
		return;
	}
	// $.get(document.getElementById('fd').value, function(data){document.getElementById('txtarea').innerHTML=data.slice(505);});
	buffer[offset] = 0;
	// Response to client GET request
	char message[10100] = "HTTP/1.1 200 OK\r\nContent-Type: text/html;\r\n\r\n<!DOCTYPE html><html><script src='https://ajax.googleapis.com/ajax/libs/jquery/3.4.0/jquery.min.js'></script><body><script>function send(){let cmd = document.getElementById('fd').value;cmd = cmd.replace(' ', '@');cmd = cmd.replace('\"', '#');$.get(cmd, function(data){document.getElementById('txtarea').innerHTML=data.slice(573).replace('%20', ' ');});}</script><input id='fd'><button type='button' onclick='send()'>Run command!</button><br><br><p id='txtarea' style='width:92%;padding:2%;height:150px;font:12px/16px cursive;border:10px double yellowgreen;' readonly='true'>";
    strcat(message, output);
    // Sleeping for 1 second
    sleep(1);
    printf("%s\n", output);
    // Sending the packet
	int length = strlen(message);
	while(length > 0) {
		printf("send bytes : %d\n", bytes);
		bytes = send(sock, message, length, 0);
		length = length - bytes;
	}
	// Clearing the placeholders and closing connection
	memset(output,0,strlen(output));
	memset(message,0,strlen(message));
	shutdown(sock, SHUT_RDWR);
	close(sock);
}

void replaceAll(char * str, char oldChar, char newChar)
{
    int i = 0;
    while(str[i] != '\0')
    {
        if(str[i] == oldChar)
        {
            str[i] = newChar;
        }

        i++;
    }
}

void exec_cmd(char *cmd){
  char str[12];
  char *args;

  strcpy(str, cmd);
  strtok_r(str, " ", &args);

  //for (int i=0; i<argc; i++) printf("%s\n", argv[i]);
  char path[] = "/usr/bin/";
  strcat(path, str);
  int link[2];
  pid_t pid;
  char foo[4096];

  if (pipe(link)==-1)
    die("pipe");

  if ((pid = fork()) == -1)
    die("fork");

  if(pid == 0) {

    dup2 (link[1], STDOUT_FILENO);
    close(link[0]);
    close(link[1]);
    execl(path, cmd, &args, (char *)0);
    die("execl");

  } else {

    close(link[1]);
    int nbytes = read(link[0], foo, sizeof(foo));
    printf("Output: (%.*s)\n", nbytes, foo);
    //wait(NULL);

  }
}
