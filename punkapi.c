#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <jzon.h>

#ifdef VERSION
static const char *version = VERSION;
#else
static const char *version = "UNKNOWN";
#endif

/* operational flags */
struct flags {
    unsigned int random;
    unsigned int page;
    unsigned int items;
    unsigned int insecure;
};

struct flags flags = {
    .random = 0,
    .page = 1,
    .items = 25,
    .insecure = 0
};

/* dynamic buffer */
struct buffer {
    size_t size;
    char *data;
};

void help(void)
{
    const char *usage =
        "Usage: punkapi [options]\n\n"
        "OPTIONS:\n"
        "  -r        \tGet a random beer\n"
        "  -p <page> \tGet all beers at the given page\n"
        "  -i <items>\tSet the number of beers per page\n"
        "  -k        \tUse insecure connection\n";

    fprintf(stdout, "PunkAPI %s\n\n%s", version, usage);
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t realsize = size * nmemb;
    struct buffer *buffer = userdata;

    buffer->data = realloc(buffer->data, buffer->size + realsize);
    if (!buffer->data) {
        return 0;
    }

    memcpy(buffer->data + buffer->size, ptr, realsize);
    buffer->size += realsize;

    return realsize;
}

int perform_api_request(const char *endpoint, struct buffer *buffer,
    char *errbuf)
{
    /* initialize cURL */
    CURL *curl = curl_easy_init();
    if (!curl) {
        return -1;
    }

    /* set cURL options */
    curl_easy_setopt(curl, CURLOPT_URL, endpoint);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

    if (flags.insecure) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    /* perform cURL request */
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        return -1;
    }

    /* cleanup cURL */
    curl_easy_cleanup(curl);

    return buffer->size;
}

void print_beers(struct jzon *jzon)
{
    if (jzon_is_array(jzon, NULL)) {
        for (int i = 0; i < jzon_array_size(jzon, NULL); i++) {
            struct jzon *beer = jzon_array_get(jzon, i, NULL);
            fprintf(stdout, "%s\n", jzon_object_get(beer, "name", NULL)->string);
        }
    }
}

int main(int argc, char **argv)
{
    /* PunkAPI JSON API HTTP endpoint */
    const char *punkapi_endpoint = "api.punkapi.com/v2/beers";

    /* parse command line */
    int ch;
    while ((ch = getopt(argc, argv, "rp:i:hk")) != -1) {
        switch (ch) {
            case 'r':
                flags.random = 1;
                break;
            case 'p':
                flags.page = atoi(optarg);
                break;
            case 'i':
                flags.items = atoi(optarg);
                break;
            case 'h':
                help();
                return EXIT_SUCCESS;
            case 'k':
                flags.insecure = 1;
                break;
            default:
                help();
                return EXIT_FAILURE;
        }
    }

    /* receive buffer */
    struct buffer buffer;
    buffer.size = 0;
    buffer.data = NULL;

    /* prepare request URL */
    char *request_url = NULL;
    if (flags.random) {
        request_url =
            calloc(
                8 + strlen(punkapi_endpoint) + 7 + 1,
                sizeof(char)
            );
        sprintf(
            request_url,
            "https://%s/random",
            punkapi_endpoint
        );
    } else {
        request_url =
            calloc(
                8 + strlen(punkapi_endpoint) + 22 + 1,
                sizeof(char)
            );
        sprintf(
            request_url,
            "https://%s?page=%d&per_page=%d",
            punkapi_endpoint,
            flags.page,
            flags.items
        );
    }

    /* perform API request */
    char errbuf[CURL_ERROR_SIZE];
    errbuf[0] = '\0';
    if (perform_api_request(request_url, &buffer, errbuf) == -1) {
        fprintf(stderr, "ERROR: %s\n", errbuf);

        if (buffer.data) {
            free(buffer.data);
        }

        free(request_url);

        return EXIT_FAILURE;
    }

    /* cleanup request URL */
    free(request_url);

    /* make buffer data a string */
    buffer.data = realloc(buffer.data, buffer.size + 1);
    buffer.data[buffer.size] = '\0';
    buffer.size += 1;

    /* JZON parse API response */
    struct jzon *jzon = jzon_parse(buffer.data, NULL);
    if (!jzon) {
        fprintf(stderr, "ERROR: %s\n", "JSON data could not get parsed");
        free(buffer.data);

        return EXIT_FAILURE;
    }

    /* cleanup buffer */
    free(buffer.data);

    /* print beers */
    print_beers(jzon);

    /* cleanup JZON */
    jzon_free(jzon);

    return EXIT_SUCCESS;
}
