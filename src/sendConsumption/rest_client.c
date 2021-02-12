#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <regex.h>
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */

void error(const char *msg) { perror(msg); exit(0); }
const char* get_vz_json_from_timestamp(long);
void getLastToupleFromSource(char * source, char * result);
char * getTuples(long);

char * getTuples(long timestamp_end) {
    const char* json = get_vz_json_from_timestamp(timestamp_end);
    static char result[10];
    getLastToupleFromSource(json, &result);
    printf("Result rest_client:\n%s\n",result);

    return result;
}

const char* get_vz_json_from_timestamp(long long_timestamp_end) {
    long long_unix_ten_seconds = 10;
    long long_timestamp_begin = long_timestamp_end - long_unix_ten_seconds;

    char *http_begin = "GET /data.json?from=";
    char *http_mid = "000&to=";
    char *http_pre_uuid = "000&uuid%5B%5D=";
    char *http_uuid = "33f74400-51bb-11eb-871c-932af3a8379b";
    char *http_end =  "HTTP/1.0\r\n\r\n";

    char message_fmt[115] = "";
    sprintf(message_fmt,"%s%ld%s%ld%s", http_begin,long_timestamp_begin,http_mid,long_timestamp_end,http_pre_uuid,http_uuid,http_end);

    /* first what are we going to send and where are we going to send it? */
    int portno =        8080;
    char *host =        "localhost";

    printf("Result message_fmt:\n%s\n",message_fmt);

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;
    char response[4096];

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) error("ERROR, no such host");

    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    /* send the request */
    total = strlen(message_fmt);
    sent = 0;
    do {
        bytes = write(sockfd,message_fmt+sent,total-sent);
        if (bytes < 0)
            error("ERROR writing message to socket");
        if (bytes == 0)
            break;
        sent+=bytes;
    } while (sent < total);

    /* receive the response */
    memset(response,0,sizeof(response));
    total = sizeof(response)-1;
    received = 0;
    do {
        bytes = read(sockfd,response+received,total-received);
        if (bytes < 0)
            error("ERROR reading response from socket");
        if (bytes == 0)
            break;
        received+=bytes;
    } while (received < total);

    if (received == total)
        error("ERROR storing complete response from socket");

    /* close the socket */
    close(sockfd);

    char *result = response;

    printf("http request result:\n%s\n", result);
    return result;
}

void getLastToupleFromSource(char * source, char * result) {
    char *regexString = "([0-9\\.]+),1\\]\\]";
    size_t maxMatches = 10;
    size_t maxGroups = 2;

    regex_t regexCompiled;
    regmatch_t groupArray[maxGroups];
    unsigned int m;
    char *cursor;

    if (regcomp(&regexCompiled, regexString, REG_EXTENDED)) {
        printf("Could not compile regular expression.\n");
    };

    cursor = source;
    for (m = 0; m < maxMatches; m++) {
        if (regexec(&regexCompiled, cursor, maxGroups, groupArray, 0))
            break;  // No more matches

        unsigned int g;
        unsigned int offset = 0;
        for (g = 0; g < maxGroups; g++) {
            if (groupArray[g].rm_so == (size_t) -1)
                break;  // No more groups

            if (g == 0)
                offset = groupArray[g].rm_eo;

            char cursorCopy[strlen(cursor) + 1];
            strcpy(cursorCopy, cursor);
            cursorCopy[groupArray[g].rm_eo] = 0;
            sprintf(result, "%s", cursorCopy + groupArray[g].rm_so);
        }
        cursor += offset;
    }
    printf("regex result:\n%s\n", result);

    regfree(&regexCompiled);
}

