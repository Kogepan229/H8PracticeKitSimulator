#include "download_file.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "cert.h"
#include "log.h"
#include "mongoose.h"

struct CallbackData {
    bool done                 = false;
    std::string url           = "";
    std::string redirect_url  = "";
    int content_length        = 0;
    int received_length       = 0;
    std::string desc_dir_path = "";
    std::string filename      = "";
    std::ofstream file;
    std::string error = "";
};

static std::unique_ptr<char[]> conv_mg_str(size_t len, const char *str) {
    auto s = std::make_unique<char[]>(len + 1);
    memcpy(s.get(), str, len);
    s[len] = '\0';
    return s;
}

static size_t send_request_head(struct mg_connection *c, std::string url) {
    struct mg_str host = mg_url_host(url.c_str());
    // Send request
    // printf("send head: %s\n", mg_url_uri(url.c_str()));
    int result = mg_printf(
        c,
        "HEAD %s HTTP/1.0\r\n"
        "Host: %.*s\r\n"
        "\r\n",
        mg_url_uri(url.c_str()), (int)host.len, host.ptr
    );
    return result;
}

static size_t send_request_get(struct mg_connection *c, std::string url) {
    struct mg_str host = mg_url_host(url.c_str());
    // Send request
    // printf("send get: %s\n", mg_url_uri(url.c_str()));
    int result = mg_printf(
        c,
        "GET %s HTTP/1.0\r\n"
        "Host: %.*s\r\n"
        "\r\n",
        mg_url_uri(url.c_str()), (int)host.len, host.ptr
    );
    return result;
}

static void callback_head(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    if (ev == MG_EV_CONNECT) {
        send_request_head(c, ((CallbackData *)fn_data)->url);
    }
    if (ev == MG_EV_HTTP_MSG) {
        int status              = mg_http_status(hm);
        struct mg_str *location = mg_http_get_header(hm, "Location");

        // Redirect
        if ((status == 301 || status == 302) && location != NULL) {
            auto s                                  = conv_mg_str((int)location->len, location->ptr);
            ((CallbackData *)fn_data)->redirect_url = std::string(s.get());
            return;
        }
        // Read header
        else {
            // Read filename
            struct mg_str *content_disposition = mg_http_get_header(hm, "Content-Disposition");
            if (content_disposition != NULL) {
                auto filename = mg_http_get_header_var(*content_disposition, mg_str("filename"));
                if (filename.len > 0) {
                    ((CallbackData *)fn_data)->filename = std::string(conv_mg_str(filename.len, filename.ptr).get());
                } else {
                    log::error("Could not get filename from header.");
                }
            } else {
                log::error("Could not get filename from header.");
            }

            // Read content length
            struct mg_str *content_length = mg_http_get_header(hm, "Content-Length");
            if (content_length != NULL) {
                ((CallbackData *)fn_data)->content_length =
                    std::stoi(std::string(conv_mg_str(content_length->len, content_length->ptr).get()));
            }
        }
        ((CallbackData *)fn_data)->done = true;
    }
    if ((ev == MG_EV_ERROR) || (ev == MG_EV_CLOSE)) {
        if (ev == MG_EV_ERROR) {
            ((CallbackData *)fn_data)->error = std::string((char *)ev_data);
            log::error(((CallbackData *)fn_data)->error);
            ((CallbackData *)fn_data)->done = true;
        }
        return;
    }
}

static void callback_get(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    struct mg_http_message *hm = (struct mg_http_message *)ev_data;
    if (ev == MG_EV_CONNECT) {
        send_request_get(c, ((CallbackData *)fn_data)->url);
    }
    if (ev == MG_EV_HTTP_CHUNK) {
        // End of receive
        if (hm->chunk.len == 0) {
            if (((CallbackData *)fn_data)->file.is_open()) {
                ((CallbackData *)fn_data)->file.close();
            }
            ((CallbackData *)fn_data)->done = true;
            return;
        }
        // Create and open file
        if (!((CallbackData *)fn_data)->file.is_open()) {
            std::string filename = ((CallbackData *)fn_data)->desc_dir_path + ((CallbackData *)fn_data)->filename;
            ((CallbackData *)fn_data)->filename = filename;
            ((CallbackData *)fn_data)->file.open(filename, std::ios_base::out | std::ios_base::binary);
            if (((CallbackData *)fn_data)->file.fail()) {
                printf("Could not open file to download.\n");
                ((CallbackData *)fn_data)->file.close();
            }
        }
        // Write to file
        ((CallbackData *)fn_data)->file.write(hm->chunk.ptr, hm->chunk.len);
        mg_http_delete_chunk(c, hm);
        if (((CallbackData *)fn_data)->file.fail()) {
            printf("Could not write to file to download.\n");
            ((CallbackData *)fn_data)->file.close();
        }

        // Update progress
        ((CallbackData *)fn_data)->received_length += hm->chunk.len;
        // printf("%d/%d\n", ((CallbackData *)fn_data)->received_length, ((CallbackData *)fn_data)->content_length);
    }
    if (ev == MG_EV_HTTP_MSG) {
        ((CallbackData *)fn_data)->done = true;
    }
    if ((ev == MG_EV_ERROR) || (ev == MG_EV_CLOSE)) {
        if (ev == MG_EV_ERROR) {
            ((CallbackData *)fn_data)->error = std::string((char *)ev_data);
            log::error(((CallbackData *)fn_data)->error);
        }
        if (((CallbackData *)fn_data)->file.is_open()) {
            ((CallbackData *)fn_data)->file.close();
        }
        ((CallbackData *)fn_data)->done = true;
        return;
    }
}

namespace network {

DownloadFileResult download_file(
    std::string url, std::string desc_dir_path, int *const content_length, int *const received_length
) {
    CallbackData callback_data  = CallbackData();
    callback_data.url           = url;
    callback_data.desc_dir_path = desc_dir_path;

    // Init
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    struct mg_tls_opts opts = {.client_ca = get_cert()};
    mg_tls_ctx_init(&mgr, &opts);

    // Get Header to get content-length and filename
    mg_http_connect(&mgr, callback_data.url.c_str(), callback_head, &callback_data);
    while (!callback_data.done) {
        mg_mgr_poll(&mgr, 50);
        if (!callback_data.redirect_url.empty()) {
            log::debug("redirect to: " + callback_data.redirect_url);
            callback_data.url = callback_data.redirect_url;
            callback_data.redirect_url.clear();
            mg_http_connect(&mgr, callback_data.url.c_str(), callback_head, &callback_data);
        }
    }

    // Check error
    if (!callback_data.error.empty()) {
        log::error(callback_data.error);
        mg_mgr_free(&mgr);
        return DownloadFileResult("", callback_data.error);
    }

    // Check exist file
    if (std::filesystem::is_regular_file(callback_data.desc_dir_path + callback_data.filename)) {
        std::string exist_file_error = "The file tried to download is already exist.";
        log::warn(exist_file_error);
        mg_mgr_free(&mgr);
        return DownloadFileResult(callback_data.desc_dir_path + callback_data.filename, exist_file_error);
    }

    // Create directory
    if (!callback_data.desc_dir_path.ends_with("/")) {
        callback_data.desc_dir_path += "/";
    }
    try {
        std::filesystem::create_directories(callback_data.desc_dir_path);
    } catch (std::filesystem::filesystem_error e) {
        log::error(e.what());
        callback_data.error = e.what();
        mg_mgr_free(&mgr);
        return DownloadFileResult("", callback_data.error);
    }

    // Download file
    callback_data.done = false;
    mg_http_connect(&mgr, callback_data.url.c_str(), callback_get, &callback_data);
    while (!callback_data.done) {
        mg_mgr_poll(&mgr, 50);
        *content_length  = callback_data.content_length;
        *received_length = callback_data.received_length;
    }

    // Check error
    if (!callback_data.error.empty()) {
        log::error(callback_data.error);
    }

    mg_mgr_free(&mgr);
    log::debug("Complete download.");
    return DownloadFileResult(callback_data.desc_dir_path + callback_data.filename, "");
}

}  // namespace network