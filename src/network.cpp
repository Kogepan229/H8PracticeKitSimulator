#include "network.h"

#include <iostream>

#include "cert.h"
#include "mongoose.h"

// First web page in history
static const char *s_url = "https://github.com/Kogepan229/H8PracticeKitSimulator";
// static const char *s_url = "http://info.cern.ch/";

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_CONNECT) {
        struct mg_str host = mg_url_host(s_url);
        // Send request
        mg_printf(
            c,
            "GET %s HTTP/1.0\r\n"
            "Host: %.*s\r\n"
            "\r\n",
            mg_url_uri(s_url), (int)host.len, host.ptr
        );
    }
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        printf("%.*s", (int)hm->message.len, hm->message.ptr);
    }
}

int access() {
    struct mg_mgr mgr;
    bool done = false;
    mg_mgr_init(&mgr);  // Init manager
    struct mg_tls_opts opts = {.client_ca = get_cert()};
    mg_tls_ctx_init(&mgr, &opts);
    mg_http_connect(&mgr, s_url, fn, &done);  // Create client connection
    while (!done) mg_mgr_poll(&mgr, 50);
    mg_mgr_free(&mgr);  // Cleanup
    return 0;
}