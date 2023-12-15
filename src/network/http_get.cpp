#include "http_get.hpp"

#include <format>
#include <string>

#include "cert.h"
#include "log.h"
#include "mongoose.h"
#include "network/dns.hpp"
#include "utils/string.hpp"

struct CallbackData {
    bool done                = false;
    std::string url          = "";
    std::string redirect_url = "";
    std::string head         = "";
    std::string body         = "";
    int content_length       = 0;
    int received_length      = 0;
    std::string error        = "";
};

static size_t send_request_get(struct mg_connection *c, std::string url) {
    struct mg_str host = mg_url_host(url.c_str());

    int result = mg_printf(
        c,
        "GET %s HTTP/1.0\r\n"
        "Host: %.*s\r\n"
        "User-Agent: H8PracticeKitSimulator\r\n"
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
        // Write received data to file
        size_t *c_data = (size_t *)c->data;
        if (c_data[0]) {
            cb_data->body += utils::conv_mg_str(mg_str{(char *)c->recv.buf, c->recv.len});
            c->recv.len = 0;  // cleanup the receive buffer

        } else {
            struct mg_http_message hm;
            hm.body.len = 0;
            int n       = mg_http_parse((char *)c->recv.buf, c->recv.len, &hm);

            if (hm.uri.ptr) {  // Redirect
                int status              = mg_http_status(&hm);
                struct mg_str *location = mg_http_get_header(&hm, "Location");
                if ((status == 301 || status == 302) && location != NULL) {
                    cb_data->redirect_url = utils::conv_mg_str(*location);
                    return;
                } else if (status != 0 && status != 200) {
                    std::string err = std::format("Failed request. url: {}, status: {}", cb_data->url, status);
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
                cb_data->head = utils::conv_mg_str(hm.head);
                cb_data->body += utils::conv_mg_str(mg_str{(char *)c->recv.buf + n, c->recv.len - n});

                c_data[0]   = hm.body.len + n;
                c->recv.len = 0;  // Cleanup the receive buffer
            }
        }
    } else if (ev == MG_EV_MQTT_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        klog::debug(utils::conv_mg_str(hm->head));
        klog::debug(utils::conv_mg_str(hm->body));
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

HttpGetResult http_get(const std::string url) {
    CallbackData callback_data = CallbackData();
    callback_data.url          = url;

    // Download file
    {
        struct mg_mgr mgr;
        mg_mgr_init(&mgr);
        mgr.dns4.url = std::format("udp://{}", network::get_dns()).c_str();

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
        }
        mg_mgr_free(&mgr);
    }

    // Check error
    if (!callback_data.error.empty()) {
        klog::error(callback_data.error);
        return HttpGetResult("", "", callback_data.error);
    }

    return HttpGetResult(callback_data.head, callback_data.body, "");
}

}  // namespace network