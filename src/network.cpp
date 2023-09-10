#include "network.h"

#include <iostream>
#include <memory>
#include <string>

#include "cert.h"
#include "mongoose.h"

static const std::string s_url =
    "https://github.com/Kogepan229/Koge29_H8-3069F_Emulator/releases/latest/download/"
    "h8-3069f_emulator-x86_64-unknown-linux-musl-0.1.1.zip";
// static const std::string s_url = "http://info.cern.ch/";

struct CallbackData {
    bool done                = false;
    std::string url          = "";
    std::string redirect_url = "";
};

static std::unique_ptr<char[]> conv_mg_str(size_t len, const char *str) {
    auto s = std::make_unique<char[]>(len + 1);
    std::memcpy(s.get(), str, len);
    s[len] = '\0';
    return s;
}

static size_t send_request(struct mg_connection *c, std::string &url) {
    struct mg_str host = mg_url_host(url.c_str());
    // Send request
    printf("send: %s\n", mg_url_uri(url.c_str()));
    int result = mg_printf(
        c,
        "GET %s HTTP/1.1\r\n"
        "Host: %.*s\r\n"
        "\r\n",
        mg_url_uri(url.c_str()), (int)host.len, host.ptr
    );
    url.clear();
    return result;
}

static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    printf("ev: %d\n", ev);
    if (ev == MG_EV_CONNECT) {
        send_request(c, ((CallbackData *)fn_data)->url);
    }
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;

        int status              = mg_http_status(hm);
        struct mg_str *location = mg_http_get_header(hm, "Location");
        if ((status == 301 || status == 302) && location != NULL) {
            auto s = conv_mg_str((int)location->len, location->ptr);
            // printf("%s\n", s.get());
            // printf("%.*s", (int)hm->head.len, hm->head.ptr);

            ((CallbackData *)fn_data)->redirect_url = std::string(s.get());
            return;
            // send_request(c, s.get());
        } else {
            printf("%.*s", (int)hm->message.len, hm->message.ptr);
            // printf("%.*s", (int)hm->head.len, hm->head.ptr);
            struct mg_str *content_disposition = mg_http_get_header(hm, "Content-Disposition");
            if (content_disposition != NULL) {
                auto filename = mg_http_get_header_var(*content_disposition, mg_str("filename"));
                printf("filename: %.*s\n", (int)filename.len, filename.ptr);
            }
        }
        ((CallbackData *)fn_data)->done = true;
    }
}

int access() {
    struct mg_mgr mgr;
    CallbackData data = CallbackData();

    mg_mgr_init(&mgr);  // Init manager
    struct mg_tls_opts opts = {.client_ca = get_cert()};
    mg_tls_ctx_init(&mgr, &opts);
    data.url = s_url;
    mg_http_connect(&mgr, s_url.c_str(), fn, &data);  // Create client connection
    while (!data.done) {
        mg_mgr_poll(&mgr, 50);
        if (!data.redirect_url.empty()) {
            printf("redirect: %s\n", data.url.c_str());
            data.url = data.redirect_url;
            data.redirect_url.clear();
            // mg_mgr_free(&mgr);
            // mgr = mg_mgr();
            // mg_mgr_init(&mgr);
            // mg_tls_ctx_init(&mgr, &opts);
            mg_http_connect(&mgr, data.url.c_str(), fn, &data);
            printf("redirect\n");
            // data.redirect_url.clear();
        }
    }

    mg_mgr_free(&mgr);  // Cleanup
    return 0;
}