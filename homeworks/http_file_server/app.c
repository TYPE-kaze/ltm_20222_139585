// NOTEs:
//      the location of "/" depend on where you start the server
//      seem to unable to work with filename with space (due to sscanf)
//      RES_TEMPLATE and its friends is a little bit of effort try to not to have to type the header manually, but forgot about the content-length :(
//                                   ^ just relize that this English grammar is weird -> try to fix in a way that do not have to rewriten the sentence -> failed -> Conclusion: English is weird :v
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>

void *client_thread(void *);
void *dir_req_handler(DIR *dir, char *, int client);
void *file_req_handler(FILE *f, char *uri, int client);
int make_res_header(char *s, int status, char *status_desr, char *content_type);
int make_anchor_tag(char *s, char *uri, char em_tag, char *name);
char *get_filename(char *filename);
char *get_ext(char *filename);

const char *RES_TEMPLATE = "HTTP/1.1 %d %s\r\nContent-Type: %s\r\n\r\n";

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Lacking args or wrong syntax\nExitting...");
        exit(-1);
    }

    char const *HOSTADDR = "127.0.0.1";
    int const PORT = atoi(argv[1]);

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        exit(-1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(HOSTADDR);
    addr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        exit(-1);
    }

    if (listen(listener, 5))
    {
        perror(("listen() failed"));
        exit(-1);
    }
    printf("Listening on port %d\n", PORT);

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            exit(-1);
        }

        printf("New client connected: %d\n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    close(listener);

    return 0;
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[128];
    int ret = 0;

    char *header_buf = NULL;
    char *header_end_pos = NULL;
    int size = 0;

    // make sure to recv all the req
    while (1)
    {
        ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            printf("Client %d disconnected\n", client);
            break;
        }

        header_buf = realloc(header_buf, size + ret);
        memcpy(header_buf + size, buf, ret);
        size += ret;

        if ((header_end_pos = strstr(header_buf, "\r\n\r\n")) != NULL)
            break;
    }
    *header_end_pos = '\0';
    char http_verb[8];
    char uri[128];

    sscanf(header_buf, "%s%s", http_verb, uri);

    DIR *dir;
    if (dir = opendir(strlen(uri) == 1 ? "." : uri + 1))
    {
        // folder
        dir_req_handler(dir, uri, client);
        closedir(dir);
    }
    else
    {
        // file
        FILE *f;
        if (f = fopen(uri + 1, "rb"))
        {
            // file exsit
            file_req_handler(f, uri, client);
            fclose(f);
        }
        else
        {
            // not
            char header[64];
            ret = make_res_header(header, 404, "Not Found", "text/html");
            strcpy(buf, header);
            strcat(buf, "<html><h1>404 File Not Found</h1></html>");

            send(client, buf, strlen(buf), 0);
        }
    }
    close(client);
}

void *file_req_handler(FILE *f, char *uri, int client)
{
    char buf[256];
    int ret = 0;
    // get file name and ext
    char *ext = get_ext(uri);
    char *filename = get_filename(uri);
    char content_type[64] = "\0";

    // Build header base on above
    // this if is for a file without ext
    if (ext)
    {
        if (!strcmp(ext, "c") || !strcmp(ext, "cpp") || !strcmp(ext, "txt"))
        {
            strcpy(content_type, "text/plain");
        }
        else if (!strcmp(ext, "jpg"))
        {
            // sprintf(content_type, "image/%s", strcmp(ext, "jpg") ? "png" : "jpeg");
            strcpy(content_type, "image/jpeg");
        }
        else if (!strcmp(ext, "png"))
        {
            strcpy(content_type, "image/png");
        }
        else if (!strcmp(ext, "mp3"))
        {
            strcpy(content_type, "audio/mpeg");
        }
    }

    // whether file ext is supported or not
    if (*content_type == '\0')
    {
        make_res_header(buf, 415, "Unsupported Media Type", "text/html");
        strcat(buf, "<html><h1>415 Unsupported Media Type</h1></html>");
        send(client, buf, strlen(buf), 0);
    }
    else
    {
        // get file size
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        // TODO: Forgot to take Content-Length in to account, now have to manually create this thing ...
        ret = sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: %s\r\n\r\n", size, content_type);
        send(client, buf, ret, 0);
        while (1)
        {
            int len = fread(buf, 1, sizeof(buf), f);
            if (len <= 0)
                break;
            send(client, buf, len, 0);
        }
    }
};

void *dir_req_handler(DIR *dir, char *uri, int client)
{
    char *res = malloc(1);
    *res = '\0';
    char header[128];
    int ret = 0;
    struct dirent *ent;

    // TODO: maybe there is a better way
    // Make header
    ret = make_res_header(header, 200, "OK", "text/html");
    // add header and html open tag to res
    res = realloc(res, strlen(res) + ret + 6 + 2 + 1);
    strcat(res, header);
    strcat(res, "<html>\r\n");
    // add html open tag

    while (1)
    {
        if (!(ent = readdir(dir)))
            break;

        // make the anchor tag
        char anchor[256];
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
        {
            continue;
        }

        ret = make_anchor_tag(anchor, uri, ent->d_type == DT_REG ? 'i' : 'b', ent->d_name);
        res = realloc(res, strlen(res) + ret + 1);
        strcat(res, anchor);
    }

    // Add closing html tag
    res = realloc(res, strlen(res) + 7 + 1);
    strcat(res, "</html>");

    send(client, res, strlen(res), 0);
    free(res);
}

int make_res_header(char *s, int status, char *status_desr, char *content_type)
{
    return sprintf(s, RES_TEMPLATE, status, status_desr, content_type);
}

int make_anchor_tag(char *s, char *uri, char em_tag, char *name)
{
    char href[128];
    sprintf(href, "%s/%s", uri + 1, name);
    return sprintf(s, "<a href=\"%s\"><%c>%s</%c></a><br>\r\n", href, em_tag, name, em_tag);
};

char *get_part_of_text(char *text, char delimiter);
// This do not make a new copy
char *get_filename(char *link)
{
    return get_part_of_text(link, '/');
}

char *get_ext(char *filename)
{
    return get_part_of_text(filename, '.');
}

// use to get the last piece of a text divied by a limiter like '/' or '.' (Should have name this better)
char *get_part_of_text(char *text, char delimiter)
{
    char *cur = strchr(text, delimiter);
    // this if is for a file without ext
    if (!cur)
        return NULL;
    char *next = NULL;
    while (1)
    {
        next = strchr(cur + 1, delimiter);
        if (!next)
        {
            break;
        }
        else
        {
            cur = next;
        }
    }
    return cur + 1;
}