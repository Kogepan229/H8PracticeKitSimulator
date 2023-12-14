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
#include "utils/string.hpp"

struct CallbackData {
    bool done                = false;
    std::string url          = "";
    std::string redirect_url = "";
    int content_length       = 0;
    int received_length      = 0;
    std::string filepath     = "";
    std::ofstream file;
    std::string error = "";
};

static std::unique_ptr<char[]> conv_mg_str(size_t len, const char *str) {
    auto s = std::make_unique<char[]>(len + 1);
    memcpy(s.get(), str, len);
    s[len] = '\0';
    return s;
}

static size_t send_request_get(struct mg_connection *c, std::string url) {
    struct mg_str host = mg_url_host(url.c_str());

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
    CallbackData *cb_data = static_cast<CallbackData *>(fn_data);

    if (ev == MG_EV_CONNECT) {
        if (mg_url_is_ssl(cb_data->url.c_str())) {
            struct mg_str host      = mg_url_host(cb_data->url.c_str());
            struct mg_tls_opts opts = {.ca = network::get_cert(), .name = host};
            mg_tls_init(c, &opts);
        }
        send_request_get(c, cb_data->url);
    } else if (ev == MG_EV_READ) {
        // Create and open file
        if (!cb_data->file.is_open()) {
            cb_data->file.open(cb_data->filepath, std::ios_base::out | std::ios_base::binary);
            if (cb_data->file.fail()) {
                cb_data->error = "Could not open file to download.";
                cb_data->done  = true;
                return;
            }
        }

        // Write received data to file
        size_t *c_data = (size_t *)c->data;
        if (c_data[0]) {
            c_data[1] += c->recv.len;
            cb_data->file.write((char *)c->recv.buf, c->recv.len);
            c->recv.len = 0;  // cleanup the receive buffer

        } else {
            struct mg_http_message hm;
            int n = mg_http_parse((char *)c->recv.buf, c->recv.len, &hm);

            if (hm.uri.ptr) {  // Redirect
                int status              = mg_http_status(&hm);
                struct mg_str *location = mg_http_get_header(&hm, "Location");
                if ((status == 301 || status == 302) && location != NULL) {
                    cb_data->redirect_url = conv_mg_str(location->len, location->ptr).get();
                    return;
                } else if (status != 0 && status != 200) {
                    std::string err = std::format("Failed download. url: {}, status: {}", cb_data->url, status);
                    cb_data->error  = err;
                    cb_data->done   = true;
                    return;
                }
            }

            if (n < 0) {
                cb_data->error = "Bad response";
                cb_data->done  = true;
                return;
            }
            if (n > 0) {
                cb_data->file.write((char *)c->recv.buf + n, c->recv.len - n);
                c_data[0] = n + hm.body.len;
                c_data[1] += c->recv.len;
                c->recv.len = 0;  // Cleanup the receive buffer
            }
        }

        // Update progress
        cb_data->content_length  = c_data[0];
        cb_data->received_length = c_data[1];

        // End of receive
        if (c_data[0] != 0 && c_data[1] >= c_data[0]) {
            cb_data->done = true;
            return;
        }

        if (cb_data->file.fail()) {
            cb_data->error = "Could not write to file.";
            cb_data->done  = true;
            return;
        }
    } else if (ev == MG_EV_ERROR) {
        cb_data->error = std::string((char *)ev_data);
        cb_data->done  = true;
        return;
    } else if (ev == MG_EV_CLOSE) {
        cb_data->done = true;
        return;
    }
}

namespace network {

DownloadFileResult download_file(
    const std::string url, const std::string desc_dir_path, int *const content_length, int *const received_length
) {
    CallbackData callback_data = CallbackData();
    callback_data.url          = url;

    {
        auto strs            = utils::split_str(url, "/");
        std::string filename = strs[strs.size() - 1];
        if (desc_dir_path.ends_with("/")) {
            callback_data.filepath = desc_dir_path + filename;
        } else {
            callback_data.filepath = desc_dir_path + "/" + filename;
        }
    }

    // Check exist file
    if (std::filesystem::is_regular_file(callback_data.filepath)) {
        std::string exist_file_error = "The file tried to download is already exist.";
        klog::warn(exist_file_error);
        return DownloadFileResult(callback_data.filepath, exist_file_error);
    }

    // Create directory
    try {
        std::filesystem::create_directories(desc_dir_path);
    } catch (const std::filesystem::filesystem_error &e) {
        klog::error(e.what());
        callback_data.error = e.what();
        return DownloadFileResult("", callback_data.error);
    }

    // Download file
    {
        struct mg_mgr mgr;
        mg_mgr_init(&mgr);

        mg_http_connect(&mgr, callback_data.url.c_str(), callback_get, &callback_data);
        while (!callback_data.done) {
            mg_mgr_poll(&mgr, 50);

            // Redirect
            if (!callback_data.redirect_url.empty()) {
                while (!callback_data.done) {
                    mg_mgr_poll(&mgr, 50);
                }
                callback_data.done = false;
                callback_data.url  = callback_data.redirect_url;
                callback_data.redirect_url.clear();
                mg_http_connect(&mgr, callback_data.url.c_str(), callback_get, &callback_data);
            }

            *content_length  = callback_data.content_length;
            *received_length = callback_data.received_length;
        }
        mg_mgr_free(&mgr);
    }

    // Close file
    if (callback_data.file.is_open()) {
        callback_data.file.close();
    }

    // Check error
    if (!callback_data.error.empty()) {
        klog::error(callback_data.error);
        return DownloadFileResult("", callback_data.error);
    }

    klog::debug("Complete download.");
    return DownloadFileResult(callback_data.filepath, "");
}

}  // namespace network