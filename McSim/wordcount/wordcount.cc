#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

int main(int argc, char *argv[])
{
	int sockfd, portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256] = {0};
	if (argc < 3) {
		std::cerr << "usage " << argv[0] << " hostname port\n";
		std::exit(1);
	}
	portno = std::atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		std::cerr << "ERROR opening socket";
		std::exit(1);
	}
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		std::cerr << "ERROR, no such host\n";
		std::exit(1);
	}
	std::memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
		server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		std::cerr << "ERROR connecting\n";
		std::exit(1);
	}

	std::cout << "Connected to sender, reading and counting words...\n";
	std::map<std::string, unsigned int> words;
	
	char c = '1';
	while (read(sockfd, buffer, 255) > 0) { // Read response
		++words[std::string(buffer)];
		std::cout << buffer << "\n";
		memset(buffer, 0, 256);
		write(sockfd, &c, 1); // Send request (ask for a new word)
	}
	
	std::cout << "==========================================\n";
	std::cout << "Terminated\n";
	std::cout << "Number of distinct words = " << words.size() << "\n";
	std::cout << "==========================================\n";
	for (std::map<std::string, unsigned int>::iterator it = words.begin(); it != words.end(); ++it) {
		std::cout << it->first << " : " << it->second << "\n";
	}

	close(sockfd);
	return 0;
}
