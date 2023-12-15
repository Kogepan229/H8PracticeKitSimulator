#include "dns.hpp"

#if defined(_WIN32) || defined(_WIN64)
//// Windows
// Must include before windows.h
#include <winsock2.h>
// Next
#include <iphlpapi.h>
#include <stdio.h>
#include <winerror.h>
#else
//// Linux
#include <arpa/inet.h>
#include <bits/types/res_state.h>
#include <netinet/in.h>

#include <cstdlib>
#include <format>

#include "resolv.h"
#endif

#include <string>

#include "log.h"

namespace network {

std::string get_dns() {
#if defined(_WIN32) || defined(_WIN64)
    FIXED_INFO *fixed_info = NULL;
    ULONG out_buf_len      = 0;

    if (GetNetworkParams(NULL, &out_buf_len) == ERROR_BUFFER_OVERFLOW) {
        fixed_info = (FIXED_INFO *)GlobalAlloc(GPTR, out_buf_len);
    } else {
        klog::error("Failed to get DNS address.");
        return "";
    }

    DWORD result = GetNetworkParams(fixed_info, &out_buf_len);
    if (result != ERROR_SUCCESS) {
        klog::error(std::string("Failed to get DNS address. ErrorCode: {}", result));

        return "";
    }

    return std::string(fixed_info->DnsServerList.IpAddress.String) + ":53";
#else
    res_state res = (res_state)malloc(sizeof(struct __res_state));
    res_ninit(res);
    return std::format("{}:{}", inet_ntoa(res->nsaddr_list[0].sin_addr), htons(res->nsaddr_list[0].sin_port));
#endif
}

}  // namespace network
