#ifndef VIA_SERVER_CRYPTO_HPP
#define VIA_SERVER_CRYPTO_HPP

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2017-2023 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file server_crypto.hpp
/// @brief Example server certificate and private key in memory.
//////////////////////////////////////////////////////////////////////////////
#ifdef ASIO_STANDALONE
  #include <asio.hpp>
  #define ASIO asio
#else
  #include <boost/asio.hpp>
  #define ASIO boost::asio
#endif
#include <cstring>

constexpr auto SERVER_CERTIFICATE_STRING = R"delim(
-----BEGIN CERTIFICATE-----
MIIEszCCA5sCFGUQBO8SGzi6i6E+6Fv/IhelAcehMA0GCSqGSIb3DQEBCwUAMIGZ
MQswCQYDVQQGEwJHQjEPMA0GA1UECAwGRG9yc2V0MQ4wDAYDVQQHDAVQb29sZTEa
MBgGA1UECgwRQWNtZSBXaWRnZXRzIEx0ZC4xCzAJBgNVBAsMAklUMRkwFwYDVQQD
DBBhY21lLXdpZGdldHMuY29tMSUwIwYJKoZIhvcNAQkBFhZhZG1pbkBhY21lLXdp
ZGdldHMuY29tMB4XDTIzMDQyOTE2NDgxNloXDTQ4MDQyNzE2NDgxNlowgZExCzAJ
BgNVBAYTAkdCMQ8wDQYDVQQIDAZEb3JzZXQxDjAMBgNVBAcMBVBvb2xlMRkwFwYD
VQQKDBBBY21lIFdpZGdldHMgTHRkMQswCQYDVQQLDAJJVDESMBAGA1UEAwwJbG9j
YWxob3N0MSUwIwYJKoZIhvcNAQkBFhZhZG1pbkBhY21lLXdpZGdldHMuY29tMIIC
IjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAp46ip+35xsiKU4JVJ333rJt3
77Wnj3ftL8mjsaYD2iOJ1C4zMS3ih8Mk6ulQRoySx0qrmn1v3eKKeC/RSmx7w2oo
RzjrFMK450F9C3eCxdkPv97mUVNtUlPL4pwhZSRylznSUq1D2dss81ZGiy6ZqQqE
b50N4IHIaXILICMBpMsF1sTgnyXvPFDw0OIp0qsWT+WLkB5n3iKkWRx1bsQLF+Ge
h040nMt8EeQV2KTM7WCDcnE4RvCcERptK7Bol8dtm1la/+3OoXnDFM7kMpyHyosx
vOEtMaXjILpfLaeJzct5BEe2RhH8av2lHACXFY4SyeP/DxsFsdl1/MM7G9vPnE2M
9XJWkfpZHX1RuhlANj2CHX13LpN4YCXc0zOMp1xTfG6henra1rT8jA2JwnvafFlv
pQwBIHPv2hqf8FDU5qGDFEL7ZbmZnZk0//TupBXdAqyhAFqT4P970nKy6HgD5fiR
d4PG62ClPImc6f2rGl90hbApPU9rrCvLkkcyTJtJ+7jpKjR1mHs4XQSymhotAwhB
TABdv+puWksNr6QkMguURcYpnIQAGfqjqF+7z2zQFK3fYzgZYTewNejAqiCHTRvn
1lHIBSnbruY/Z6sKfhLOGpD1qjKJNEX9BRg4MIMdSHP6c8b7NvTi9+UgR7pMWoXj
8HHIRhr2NRjjOri/ynkCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEAXzSPQu0KmPhn
x6iYTkKCJ7nxaHFYkoCgnW0vLH+0YN3msSbtczSqwM2JbnPOHg0Hwa+uCUfL4g1M
+42TmjtD1R+rinGDrIfcBHIuCbKJOXAunKu1nVqWhMDl3yoqM4SDZ22QMZOxe+IQ
+uk6cv4++P3ukk+SIkEfpQUTBaE5oVGIMRA3vCYl1KaQ3zQlDeW7DxZ6PuNOBn8r
G2VZfgplQfSi1iNATbBqUIUPzu6hlZS3Iz7ct5aUjw688zHQw3kf1KLObJScW71n
kDWZwQMNL3VH+x9Pfkuva/dM65IMg6wg4+x6IPpHM8XKGWGhiIxcVr8p1IxMaDt1
IhBZTDUzaQ==
-----END CERTIFICATE-----
)delim";

const ASIO::const_buffer SERVER_CERTIFICATE(SERVER_CERTIFICATE_STRING, std::strlen(SERVER_CERTIFICATE_STRING));

constexpr auto SERVER_KEY_STRING = R"delim(
-----BEGIN PRIVATE KEY-----
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
nrhYrYej1nnIslBp3TDr+8Tz2DWT
-----END PRIVATE KEY-----
)delim";

const ASIO::const_buffer SERVER_KEY(SERVER_KEY_STRING, std::strlen(SERVER_KEY_STRING));

constexpr auto SERVER_KEY_TYPE{ASIO::ssl::context::pem};

constexpr auto SERVER_KEY_PASSWORD{"test"};

#endif // VIA_SERVER_CRYPTO_HPP
