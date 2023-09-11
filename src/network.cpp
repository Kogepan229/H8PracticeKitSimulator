#include "network.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "cert.h"
#include "mongoose.h"

static const std::string s_url =
    "https://github.com/Kogepan229/Koge29_H8-3069F_Emulator/releases/latest/download/"
    "h8-3069f_emulator-x86_64-pc-windows-msvc-0.1.1.zip";
// static const std::string s_url = "http://info.cern.ch/";

static const std::string s_path = "./";

struct CallbackData {
    bool done                 = false;
    std::string url           = "";
    std::string redirect_url  = "";
    int content_length        = 0;
    int received_length       = 0;
    std::string file_dir_path = "";
    std::string filename      = "";
    std::ofstream file;
};

static std::unique_ptr<char[]> conv_mg_str(size_t len, const char *str) {
    auto s = std::make_unique<char[]>(len + 1);
    std::memcpy(s.get(), str, len);
    s[len] = '\0';
    return s;
}

static size_t send_request_head(struct mg_connection *c, std::string url) {
    struct mg_str host = mg_url_host(url.c_str());
    // Send request
    printf("send: %s\n", mg_url_uri(url.c_str()));
    int result = mg_printf(
        c,
        "HEAD %s HTTP/1.0\r\n"
        "Host: %.*s\r\n"
        "\r\n",
        mg_url_uri(url.c_str()), (int)host.len, host.ptr
    );
    // printf("send len: %d\n", result);
    return result;
}

static size_t send_request_get(struct mg_connection *c, std::string url) {
    struct mg_str host = mg_url_host(url.c_str());
    // Send request
    printf("send: %s\n", mg_url_uri(url.c_str()));
    int result = mg_printf(
        c,
        "GET %s HTTP/1.0\r\n"
        "Host: %.*s\r\n"
        "\r\n",
        mg_url_uri(url.c_str()), (int)host.len, host.ptr
    );
    // printf("send len: %d\n", result);
    return result;
}

static void callback_head(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    // if (ev != 2 && ev != 7 && ev != 11) {
    //     printf("ev: %d\n", ev);
    // }
    printf("ev: %d\n", ev);
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    if (ev == MG_EV_CONNECT) {
        send_request_head(c, ((CallbackData *)fn_data)->url);
        // ((CallbackData *)fn_data)->url.clear();
    }
    if (ev == MG_EV_HTTP_MSG) {
        int status              = mg_http_status(hm);
        struct mg_str *location = mg_http_get_header(hm, "Location");

        // redirect
        if ((status == 301 || status == 302) && location != NULL) {
            auto s = conv_mg_str((int)location->len, location->ptr);
            // printf("%s\n", s.get());
            // printf("%.*s", (int)hm->head.len, hm->head.ptr);

            ((CallbackData *)fn_data)->redirect_url = std::string(s.get());
            return;
            // send_request(c, s.get());
        }
        // read header
        else {
            printf("%.*s", (int)hm->message.len, hm->message.ptr);
            // printf("%.*s", (int)hm->head.len, hm->head.ptr);

            // filename
            struct mg_str *content_disposition = mg_http_get_header(hm, "Content-Disposition");
            if (content_disposition != NULL) {
                auto filename = mg_http_get_header_var(*content_disposition, mg_str("filename"));
                if (filename.len > 0) {
                    ((CallbackData *)fn_data)->filename = std::string(conv_mg_str(filename.len, filename.ptr).get());
                    printf("filename: %s\n", ((CallbackData *)fn_data)->filename.c_str());
                }
                // printf("filename: %.*s\n", (int)filename.len, filename.ptr);
            }

            // content length
            struct mg_str *content_length = mg_http_get_header(hm, "Content-Length");
            if (content_length != NULL) {
                ((CallbackData *)fn_data)->content_length =
                    std::stoi(std::string(conv_mg_str(content_length->len, content_length->ptr).get()));
            }
        }
        ((CallbackData *)fn_data)->done = true;
    }
}

static void callback_get(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    // if (ev != 2 && ev != 7 && ev != 11) {
    //     printf("ev: %d\n", ev);
    // }
    printf("ev: %d\n", ev);
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    if (ev == MG_EV_CONNECT) {
        send_request_get(c, ((CallbackData *)fn_data)->url);
    }
    if (ev == MG_EV_HTTP_CHUNK) {
        if (hm->chunk.len == 0) {
            if (((CallbackData *)fn_data)->file.is_open()) {
                ((CallbackData *)fn_data)->file.close();
            }
            return;
        }
        if (!((CallbackData *)fn_data)->file.is_open()) {
            auto millisec_since_epoch =
                duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            std::string filename =
                /*((CallbackData *)fn_data)->file_dir_path + */ std::to_string(millisec_since_epoch);
            ((CallbackData *)fn_data)->filename = filename;
            ((CallbackData *)fn_data)->file.open(filename, std::ios_base::out | std::ios_base::binary);
            if (((CallbackData *)fn_data)->file.fail()) {
                printf("Could not open file to download.\n");
                ((CallbackData *)fn_data)->file.close();
            }
        }
        ((CallbackData *)fn_data)->file.write(hm->chunk.ptr, hm->chunk.len);
        if (((CallbackData *)fn_data)->file.fail()) {
            printf("Could not write to file to download.\n");
            ((CallbackData *)fn_data)->file.close();
        }
        ((CallbackData *)fn_data)->received_length += hm->chunk.len;
        printf("%d/%d\n", ((CallbackData *)fn_data)->received_length, ((CallbackData *)fn_data)->content_length);
    }
    if (ev == MG_EV_HTTP_MSG) {
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
    CallbackData callback_data = CallbackData();
    callback_data.url          = s_url;

    // init
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    struct mg_tls_opts opts = {.client_ca = get_cert()};
    mg_tls_ctx_init(&mgr, &opts);

    mg_http_connect(&mgr, s_url.c_str(), callback_head, &callback_data);  // Create client connection
    while (!callback_data.done) {
        mg_mgr_poll(&mgr, 50);
        if (!callback_data.redirect_url.empty()) {
            printf("redirect: %s\n", callback_data.url.c_str());
            callback_data.url = callback_data.redirect_url;
            callback_data.redirect_url.clear();
            // mg_mgr_free(&mgr);
            // mgr = mg_mgr();
            // mg_mgr_init(&mgr);
            // mg_tls_ctx_init(&mgr, &opts);
            mg_http_connect(&mgr, callback_data.url.c_str(), callback_head, &callback_data);
            printf("redirect\n");
            // callback_data.redirect_url.clear();
        }
    }

    callback_data.done = false;

    // TODO: file exist check

    mg_http_connect(&mgr, callback_data.url.c_str(), callback_get, &callback_data);  // Create client connection
    while (!callback_data.done) {
        mg_mgr_poll(&mgr, 50);
    }

    mg_mgr_free(&mgr);  // Cleanup
    return 0;
}