#include "dns.hpp"

// Must include before windows.h
#include <winsock2.h>
// Next
#include <iphlpapi.h>
#include <stdio.h>
#include <winerror.h>

#include <string>

#include "log.h"

namespace network {

std::string get_dns() {
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

    return std::string(fixed_info->DnsServerList.IpAddress.String);
}

}  // namespace network
