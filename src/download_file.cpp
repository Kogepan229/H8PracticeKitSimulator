#include "download_file.h"

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "cert.h"
#include "log.h"
#include "mongoose.h"
#include "utils.h"

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

static void callback_get(struct mg_connection *c, int ev, void *ev_data, void *fn_data) {
    if (ev == MG_EV_CONNECT) {
        if (mg_url_is_ssl(((CallbackData *)fn_data)->url.c_str())) {
            struct mg_str host      = mg_url_host(((CallbackData *)fn_data)->url.c_str());
            struct mg_tls_opts opts = {.ca = get_cert(), .name = host};
            mg_tls_init(c, &opts);
        }
        send_request_get(c, ((CallbackData *)fn_data)->url);
    }
    // End of receive
    if (ev == MG_EV_CLOSE) {
        if (((CallbackData *)fn_data)->file.is_open()) {
            ((CallbackData *)fn_data)->file.close();
        }
        ((CallbackData *)fn_data)->done = true;
        return;
    }
    if (ev == MG_EV_READ) {
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

        // Write received data to file
        size_t *data = (size_t *)c->data;
        if (data[0]) {
            data[1] += c->recv.len;
            ((CallbackData *)fn_data)->file.write((char *)c->recv.buf, c->recv.len);
            c->recv.len = 0;  // And cleanup the receive buffer. Streming!
            if (data[1] >= data[0]) {
                if (((CallbackData *)fn_data)->file.is_open()) {
                    ((CallbackData *)fn_data)->file.close();
                }
                ((CallbackData *)fn_data)->done = true;
                return;
            }
        } else {
            struct mg_http_message hm;
            int n = mg_http_parse((char *)c->recv.buf, c->recv.len, &hm);
            if (n < 0)
                mg_error(c, "Bad response");
            if (n > 0) {
                ((CallbackData *)fn_data)->file.write((char *)c->recv.buf + n, c->recv.len - n);
                data[0] = n + hm.body.len;
                data[1] += c->recv.len;
                c->recv.len = 0;  // Cleanup receive buffer

                // End of receive
                if (data[1] >= data[0]) {
                    if (((CallbackData *)fn_data)->file.is_open()) {
                        ((CallbackData *)fn_data)->file.close();
                    }
                    ((CallbackData *)fn_data)->done = true;
                    return;
                }
            }
        }

        // Update progress
        ((CallbackData *)fn_data)->content_length  = data[0];
        ((CallbackData *)fn_data)->received_length = data[1];

        if (((CallbackData *)fn_data)->file.fail()) {
            printf("Could not write to file to download.\n");
            ((CallbackData *)fn_data)->file.close();
        }
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

    {
        auto strs              = utils::split_str(url, "/");
        callback_data.filename = strs[strs.size() - 1];
    }

    // Check exist file
    if (std::filesystem::is_regular_file(callback_data.desc_dir_path + callback_data.filename)) {
        std::string exist_file_error = "The file tried to download is already exist.";
        log::warn(exist_file_error);
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
        return DownloadFileResult("", callback_data.error);
    }

    // Download file
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

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