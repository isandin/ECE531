// Compile using: make -f make_h2 (gcc)
// Compile using make -f make_arm (arm)
// Isandin
// HW2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <curl/curl.h>

#define OK_EXIT 0
#define ARG_ERROR 1
#define curlE 2

// Callback for printing response directly to stdout
static size_t response(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_bytes = size * nmemb;
    fwrite(contents, size, nmemb, stdout);
    return total_bytes;
}

// This is the help list or helpline.
static void display_help(const char *name) {
    printf("Usage: %s [OPTIONS] [MESSAGE_STRING]\n", name);
    printf("Options:\n");
    printf("  -u, --url       <url>      Specify the URL (This is required)\n");
    printf("  -o, --post                 Perform post\n");
    printf("  -g, --get                  Perform get\n");
    printf("  -p, --put                  Perform put\n");
    printf("  -d, --delete               Perform delete\n");
    printf("  -h, --help                 Display this help message\n");
    printf("Note: Python3 SimpleHTTPServer does not handle post/put. Use Flask for full testing.\n");
}

//Expected arguments
int main(int argc, char *argv[]) {
    const char *url = NULL;
    const char *method = NULL;
    bool require = false;

    static const struct option command_options[] = {
        {"url", required_argument, NULL, 'u'},
        {"post", no_argument, NULL, 'o'},
        {"get", no_argument, NULL, 'g'},
        {"put", no_argument, NULL, 'p'},
        {"delete", no_argument, NULL, 'd'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };

    // Parse command-line options
    int stor;
    while ((stor = getopt_long(argc, argv, "u:ogpdh", command_options, NULL)) != -1) {
        switch (stor) {
            case 'u':
                url = optarg;
                break;
            case 'o':
                if (method) { fprintf(stderr, "You can only use one method i.e ./hw2 -p --url ....\n"); return ARG_ERROR; }
                method = "post";
                require = true;
                break;
            case 'g':
                if (method) { fprintf(stderr, "You can only use one method i.e ./hw2 -p --url ....\n"); return ARG_ERROR; }
                method = "get";
                break;
            case 'p':
                if (method) { fprintf(stderr, "You can only use one method i.e ./hw2 -p --url ....\n"); return ARG_ERROR; }
                method = "put";
                require = true;
                break;
            case 'd':
                if (method) { fprintf(stderr, "You can only use one method i.e ./hw2 -p --url ....\n"); return ARG_ERROR; }
                method = "delete";
                break;
            case 'h':
                display_help(argv[0]);
                return OK_EXIT;
            default:
                display_help(argv[0]);
                return ARG_ERROR;
        }
    }

    if (!method || !url) {
        fprintf(stderr, "You must specify both a URL and an HTTP method.\n");
        return ARG_ERROR;
    }

    // Collect body if present (for POST/PUT)
    char *bod = NULL;
    size_t len = 0;
    if (optind < argc) {
        size_t size = 0;
        for (int i = optind; i < argc; i++) size += strlen(argv[i]) + 1;
        bod = malloc(size + 1);
        if (!bod) {
            perror("malloc");
            return curlE;
        }
        bod[0] = '\0';
        for (int i = optind; i < argc; i++) {
            strcat(bod, argv[i]);
            if (i < argc - 1) strcat(bod, " ");
        }
        len = strlen(bod);
    }

    if (require && len == 0) {
        fprintf(stderr, "This needs some text/body.\n");
        free(bod);
        return ARG_ERROR;
    }

    // Initialize libcurl and set options
    CURL *session = curl_easy_init();
    if (!session) {
        fprintf(stderr, "Curl failed\n");
        free(bod);
        return curlE;
    }

    curl_easy_setopt(session, CURLOPT_URL, url);
    curl_easy_setopt(session, CURLOPT_WRITEFUNCTION, response);
    curl_easy_setopt(session, CURLOPT_USERAGENT, "hw-client/1.0");

    if (strcmp(method, "get") == 0) {
        // GET is default
    } else if (strcmp(method, "post") == 0) {
        curl_easy_setopt(session, CURLOPT_POST, 1L);
        curl_easy_setopt(session, CURLOPT_POSTFIELDS, bod);
        curl_easy_setopt(session, CURLOPT_POSTFIELDSIZE, (long)len);
    } else {
        curl_easy_setopt(session, CURLOPT_CUSTOMREQUEST, method);
        if (len > 0) {
            curl_easy_setopt(session, CURLOPT_POSTFIELDS, bod);
            curl_easy_setopt(session, CURLOPT_POSTFIELDSIZE, (long)len);
        }
    }

    CURLcode result = curl_easy_perform(session);
    if (result != CURLE_OK) {
        fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(result));
        curl_easy_cleanup(session);
        free(bod);
        return curlE;
    }

    long response_code;
    curl_easy_getinfo(session, CURLINFO_RESPONSE_CODE, &response_code);
    printf("\n[HTTP %ld]\n", response_code);

    curl_easy_cleanup(session);
    free(bod);
    return OK_EXIT;
}

