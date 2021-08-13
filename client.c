#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

#define SERVER "34.118.48.238"
#define PORT 8080
#define P_TYPE "application/json"
#define INPUT_SIZE 60

// trimite un mesaj de tip GET cu parametrii dati
char *send_get_message(char *url, char **cookies, int cookies_count, char *auth) {
	char *message, *response;
	int sockfd;
	// stabilesc conexiunea
	sockfd = open_connection(SERVER, PORT, AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("Socket error.");
	}
	// trimit mesajul
	message = compute_get_request(SERVER, url, NULL, cookies, cookies_count, auth);
	send_to_server(sockfd, message);
	response = receive_from_server(sockfd);

	close_connection(sockfd);

	return response;
}

// trimite un mesaj de tip POST cu parametrii dati
char *send_post_message(char *url, char *content, char **cookies, int cookies_count, char *auth) {
	char *message, *response;
	int sockfd;
	sockfd = open_connection(SERVER, PORT, AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("Socket error.");
	}

	message = compute_post_request(SERVER, url, P_TYPE, content, cookies, cookies_count, auth);
	send_to_server(sockfd, message);
	response = receive_from_server(sockfd);

	close_connection(sockfd);

	return response;
}

// trimite un mesaj de tip DELETE cu parametrii dati
char *send_delete_message(char *url, char **cookies, int cookies_count, char *auth) {
	char *message, *response;
	int sockfd;
	sockfd = open_connection(SERVER, PORT, AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("Socket error.");
	}

	message = compute_delete_request(SERVER, url, NULL, cookies, cookies_count, auth);
	send_to_server(sockfd, message);
	response = receive_from_server(sockfd);

	close_connection(sockfd);

	return response;
}

// verifica daca user-ul este autentificat si daca are acces la biblioteca
int check_access(char **cookies, char *auth_token) {
	if (*cookies == NULL) {
		printf("Server Error: You are not logged in!\n");
		return 0;
	}
	if (auth_token == NULL) {
		printf("Server Error: Authorization header is missing!\n");
		return 0;
	}
	return 1;
}

// functie de inregistrare a unui nou user
void register_user() {
	char *response;
	char username[INPUT_SIZE], password[INPUT_SIZE];
	// creez obiectul de tip JSON
	JSON_Value *val = json_value_init_object();
	JSON_Object *obj = json_value_get_object(val);

	// citesc input-ul
	printf("username=");
	fgets(username, INPUT_SIZE, stdin);
	username[strlen(username) - 1] = '\0';
	json_object_set_string(obj, "username", username);

	printf("password=");
	fgets(password, INPUT_SIZE, stdin);
	password[strlen(password) - 1] = '\0';
	json_object_set_string(obj, "password", password);

	response = send_post_message("/api/v1/tema/auth/register", json_serialize_to_string(val), NULL, 0, NULL);

	val = json_parse_string(strstr(response, "{"));
	obj = json_object(val);
	const char *check = json_object_dotget_string(obj, "error");
	if (check != NULL) {
		printf("Server Error: %s\n", check);
	} else {
		printf("Congratulations %s, account successfully created.\n", username);
	}
}

// functie de login pentru user
void login(char **cookies, int *cookies_count) {
	if (*cookies_count > 0) {
		printf("Server Error: You are already logged in!\n");
		return;
	}
 	char *response;
	char username[INPUT_SIZE], password[INPUT_SIZE];
	JSON_Value *val = json_value_init_object();
	JSON_Object *obj = json_value_get_object(val);

	printf("username=");
	fgets(username, INPUT_SIZE, stdin);
	username[strlen(username) - 1] = '\0';
	json_object_set_string(obj, "username", username);

	printf("password=");
	fgets(password, INPUT_SIZE, stdin);
	password[strlen(password) - 1] = '\0';
	json_object_set_string(obj, "password", password);

	response = send_post_message("/api/v1/tema/auth/login", json_serialize_to_string(val), cookies, *cookies_count, NULL);
	
	val = json_parse_string(strstr(response, "{"));
	obj = json_object(val);
	const char *check = json_object_dotget_string(obj, "error");
	if (check != NULL) {
		printf("Server Error: %s\n", check);
		return;
	}

	printf("Login successful %s!\n", username);
	// retin cookie-ul intors de server
	response = strstr(response, "Set-Cookie");
	char *token = strtok(response, " ");
	token = strtok(NULL, ";");
	*cookies = malloc(strlen(token) * sizeof(char));
	strcpy(*cookies, token);
	*cookies_count += 1;
}

// functie de acces in biblioteca
void enter_library(char **cookies, int cookies_count, char **auth_token) {
	if (*auth_token != NULL) {
		printf("Server Error: You already have access to library!\n");
		return;
	}
	char *response;
	response = send_get_message("/api/v1/tema/library/access", cookies, cookies_count, *auth_token);
	
	JSON_Value *val = json_parse_string(strstr(response, "{"));
	JSON_Object *obj = json_object(val);
	const char *check = json_object_dotget_string(obj, "error");
	if (check != NULL) {
		printf("Server Error: %s\n", check);
		return;
	}

	// retin token-ul intors de server
	obj = json_value_get_object(val);
	char *token = (char*)json_object_get_string(obj, "token");
	*auth_token = malloc(strlen(token) * sizeof(char));
	strcpy(*auth_token, token);
	printf("Library access granted.\n");
}

// functie de intoarce cartile din biblioteca
void get_books(char **cookies, int cookies_count, char *auth_token) {
	char *response;
	response = send_get_message("/api/v1/tema/library/books", cookies, cookies_count, auth_token);
	
	JSON_Value *val = json_parse_string(strstr(response, "["));
	JSON_Array *books = json_value_get_array(val);
	int nr_books = json_array_get_count(books);
	if (!check_access(cookies, auth_token)) {
		return;
	}
	if (nr_books == 0) {
		printf("No books available in library.\n");
	} else {
		// sunt carti in bilbioteca deci de afisez
		printf("Books available - %d:\n", nr_books);
		for (int i = 0; i < nr_books; i++) {
			JSON_Object *obj = json_array_get_object(books, i);
			printf("Id: %d - Title: %s\n", (int)json_object_dotget_number(obj, "id"),
				json_object_dotget_string(obj, "title"));
		}
	}
}

// functie ce intoarce o anumita carte din biblioteca
void get_book(char **cookies, int cookies_count, char *auth_token) {
 	char *response;
	char id[INPUT_SIZE];
	printf("id=");
	fgets(id, INPUT_SIZE, stdin);
	id[strlen(id) - 1] = '\0';

	// adaug id-ul la url
	char url[INPUT_SIZE];
	strcpy(url, "/api/v1/tema/library/books/");
	strcat(url, id);

	response = send_get_message(url, cookies, cookies_count, auth_token);
	// verific daca a intors o eroare
	JSON_Value *val = json_parse_string(strstr(response, "{"));
	JSON_Object *obj = json_object(val);
	const char *check = json_object_dotget_string(obj, "error");
	if (check != NULL) {
		printf("Server Error: %s\n", check);
		return;
	}
	// afisez detaliile cartii cautate
	printf("Found the book in library:\n");
	printf("Title: %s\n", json_object_dotget_string(obj, "title"));
	printf("Author: %s\n", json_object_dotget_string(obj, "author"));
	printf("Publisher: %s\n", json_object_dotget_string(obj, "publisher"));
	printf("Genre: %s\n", json_object_dotget_string(obj, "genre"));
	printf("Page-count: %d\n", (int)json_object_dotget_number(obj, "page_count"));	
}

// functie ce adauga o noua carte in biblioteca
void add_book(char **cookies, int cookies_count, char *auth_token) {
	char *response;
	char *line = malloc(INPUT_SIZE * sizeof(char));
	JSON_Value *val = json_value_init_object();
	JSON_Object *obj = json_value_get_object(val);

	// citim campurile din stdin si construim obiectul JSON
	printf("title=");
	fgets(line, INPUT_SIZE, stdin);
	line[strlen(line) - 1] = '\0';
	json_object_set_string(obj, "title", line);

	printf("author=");
	fgets(line, INPUT_SIZE, stdin);
	line[strlen(line) - 1] = '\0';
	json_object_set_string(obj, "author", line);

	printf("genre=");
	fgets(line, INPUT_SIZE, stdin);
	line[strlen(line) - 1] = '\0';
	json_object_set_string(obj, "genre", line);

	printf("publisher=");
	fgets(line, INPUT_SIZE, stdin);
	line[strlen(line) - 1] = '\0';
	json_object_set_string(obj, "publisher", line);

	printf("page_count=");
	fgets(line, INPUT_SIZE, stdin);
	line[strlen(line) - 1] = '\0';
	// verific sa fie valid
	if (atoi(line) <= 0) {
		printf("Server Error: Invalid input.\n");
		return;
	}
	json_object_set_number(obj, "page_count", (double)atoi(line));

	if (!check_access(cookies, auth_token)) {
		return;
	}
	response = send_post_message("/api/v1/tema/library/books", json_serialize_to_string(val),
								 cookies, cookies_count, auth_token);

	val = json_parse_string(strstr(response, "{"));
	obj = json_object(val);
	// verific daca a intors o eroare
	const char *check = json_object_dotget_string(obj, "error");
	if (check != NULL) {
		printf("Server Error: %s\n", check);
		return;
	}
	printf("Book successfully added to library.\n");

	free(line);
}

// functie ce sterge o carte din biblioteca
void delete_book(char **cookies, int cookies_count, char *auth_token) {
 	char *response;
	char id[INPUT_SIZE];
	printf("id=");
	fgets(id, INPUT_SIZE, stdin);
	id[strlen(id) - 1] = '\0';

	// adaug id-ul la url
	char url[INPUT_SIZE];
	strcpy(url, "/api/v1/tema/library/books/");
	strcat(url, id);

	response = send_delete_message(url, cookies, cookies_count, auth_token);

	JSON_Value *val = json_parse_string(strstr(response, "{"));
	JSON_Object *obj = json_object(val);
	const char *check = json_object_dotget_string(obj, "error");
	if (check != NULL) {
		// cartea cu id-ul specificat nu exista in biblioteca
		if (strstr(check, "Bad")) {
			printf("Server Error: id is not int!\n");
		} else {
			printf("Server Error: %s\n", check);
		}
		return;
	}

	printf("Book successfully deleted.\n");
}

// functie de logout
void logout(char **cookies, int *cookies_count, char **auth_token) {
	char *response;
	response = send_get_message("/api/v1/tema/auth/logout", cookies, *cookies_count, *auth_token);

	JSON_Value *val = json_parse_string(strstr(response, "{"));
	JSON_Object *obj = json_object(val);
	const char *check = json_object_dotget_string(obj, "error");
	if (check != NULL) {
		printf("Server Error: %s\n", check);
		return;
	}
	*cookies = NULL;
	*cookies_count = 0;
	*auth_token = NULL;
	printf("You have been successfully logged out.\n");
}

int main(int argc, char *argv[])
{
	char message[INPUT_SIZE];
	char *cookies = NULL;
	int cookies_count = 0;
	char *auth_token = NULL;

	while (1) {
		fgets(message, INPUT_SIZE, stdin);
		if (strcmp(message, "register\n") == 0) {
			register_user();

		} else if (strcmp(message, "login\n") == 0) {
			login(&cookies, &cookies_count);

		} else if (strcmp(message, "enter_library\n") == 0) {
			enter_library(&cookies, cookies_count, &auth_token);

		} else if (strcmp(message, "get_books\n") == 0) {
			get_books(&cookies, cookies_count, auth_token);

		} else if (strcmp(message, "get_book\n") == 0) {
			get_book(&cookies, cookies_count, auth_token);

		} else if (strcmp(message, "add_book\n") == 0) {
			add_book(&cookies, cookies_count, auth_token);

		} else if (strcmp(message, "delete_book\n") == 0) {
			delete_book(&cookies, cookies_count, auth_token);

		} else if (strcmp(message, "logout\n") == 0) {
			logout(&cookies, &cookies_count, &auth_token);

		} else if (strcmp(message, "exit\n") == 0) {
			printf("Exiting the program...\n");
			break;

		} else {
			printf("Invalid command.\n");
		}
	}

	// free the allocated data at the end!
	free(cookies);
	free(auth_token);

	return 0;
}