//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2015-2021 Via Technology Ltd. All Rights Reserved.
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
#include "via/http/request_router.hpp"
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace via::http;

typedef rx_request<1024, 8, 100, 8190, 1024, 8, true> http_request;
typedef request_router<std::string, http_request> string_router;

namespace
{
  const std::string CUSTOMER ("/customer");
  const std::string ID       ("/:id");
  const std::string NAME     ("/name");
  const std::string MEMORY   ("/memory");
  const std::string ADDRESS  ("/:address");

  const std::string get_bad_request("GET /bad HTTP/1.1\r\nContent: text\r\n\r\n");

  const std::string get_name_request("GET /name HTTP/1.1\r\nContent: text\r\n\r\n");
  const std::string put_name_request("PUT /name HTTP/1.1\r\nContent: text\r\n\r\n");
  const std::string post_name_request("POST /name HTTP/1.1\r\nContent: text\r\n\r\n");

  const std::string get_customer_request("GET /customer HTTP/1.1\r\nContent: text\r\n\r\n");
  const std::string get_customer_name_request("GET /customer/JohnSmith HTTP/1.1\r\nContent: text\r\n\r\n");
  const std::string get_name_address_request
    ("GET /customer/JohnSmith/London HTTP/1.1\r\nContent: text\r\n\r\n");


  std::string output_parameters(Parameters const& parameters)
  {
    std::string output;
    for (auto const& elem : parameters)
    {
      output += " param: ";
      output += elem.first;
      output += " value: ";
      output += elem.second;
      output += " ; ";
    }

    return output;
  }

  tx_response test_route1(http_request const&, //request,
                          Parameters const& parameters,
                          std::string const& data,
                          std::string &response_body)
  {
    response_body += "test_route1:";
    response_body += output_parameters(parameters);
    response_body += data;
    response_body += "\n";

    return tx_response(response_status::code::OK);
  }

  tx_response test_route2(http_request const&, //request,
                          Parameters const& parameters,
                          std::string const& data,
                          std::string &response_body)
  {
    response_body += "test_route2:";
    response_body += output_parameters(parameters);
    response_body += data;
    response_body += "\n";

    return tx_response(response_status::code::OK);
  }

  tx_response test_route3(http_request const&, //request,
                          Parameters const& parameters,
                          std::string const& data,
                          std::string &response_body)
  {
    response_body += "test_route3:";
    response_body += output_parameters(parameters);
    response_body += data;
    response_body += "\n";

    return tx_response(response_status::code::NOT_IMPLEMENTED);
  }

  tx_response test_route4(http_request const&, //request,
                          Parameters const& parameters,
                          std::string const& data,
                          std::string &response_body)
  {
    response_body += "test_route3:";
    response_body += output_parameters(parameters);
    response_body += data;
    response_body += "\n";

    return tx_response(response_status::code::NO_CONTENT);
  }

  // A boost test fixture for this test suite.
  struct RequestRouterFixture
  {
    string_router request_router_;

    RequestRouterFixture()
      : request_router_()
    {
      request_router_.add_method(request_method::id::GET, NAME, &test_route1);
      request_router_.add_method(request_method::id::PUT, NAME, &test_route2);

      request_router_.add_method(request_method::id::GET, CUSTOMER, &test_route1);
      request_router_.add_method(request_method::id::GET, CUSTOMER + ID, &test_route3);
      request_router_.add_method(request_method::id::GET, CUSTOMER + ID + ADDRESS,
                                  &test_route4);
    }
  };
}

//////////////////////////////////////////////////////////////////////////////
BOOST_FIXTURE_TEST_SUITE(TestRequestRouter, RequestRouterFixture)

BOOST_AUTO_TEST_CASE(Test_get_route_parameters1)
{
  std::string path(CUSTOMER + ID);
  std::string uri_path("/customer/JohnSmith");
  Parameters params(get_route_parameters(uri_path, path));
  BOOST_CHECK_EQUAL(1U, params.size());
}

BOOST_AUTO_TEST_CASE(Test_get_route_parameters2)
{
  std::string path(CUSTOMER + ID + ADDRESS);
  std::string uri_path("/customer/JohnSmith/London");
  Parameters params(get_route_parameters(uri_path, path));
  BOOST_CHECK_EQUAL(2U, params.size());
}

BOOST_AUTO_TEST_CASE(FailedRouteTest1)
{
  // A simple GET request to an unknow resource
  std::string request_data(get_bad_request);
  request_data += CRLF;
  std::string::iterator next(request_data.begin());
  rx_request<1024, 8, 100, 8190, 1024, 8, true> request;
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string data;
  std::string response_body;
  tx_response response(request_router_.handle_request(request, data, response_body));
  BOOST_CHECK_EQUAL(static_cast<int>(response_status::code::NOT_FOUND),
                    response.status());
//  std::cout << "SimpleRouteTest1: "<< response_body << std::endl;
}

BOOST_AUTO_TEST_CASE(FailedRouteTest2)
{
  // A simple POST request
  std::string request_data(post_name_request);
  request_data += CRLF;
  std::string::iterator next(request_data.begin());
  rx_request<1024, 8, 100, 8190, 1024, 8, true> request;
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string data;
  std::string response_body;
  tx_response response(request_router_.handle_request(request, data, response_body));
  BOOST_CHECK_EQUAL(static_cast<int>(response_status::code::METHOD_NOT_ALLOWED),
                    response.status());
//  std::cout << "FailedRouteTest2: "<< response.message() << std::endl;
}

BOOST_AUTO_TEST_CASE(SimpleRouteTest1)
{
  // A simple GET request
  std::string request_data(get_name_request);
  request_data += CRLF;
  std::string::iterator next(request_data.begin());
  rx_request<1024, 8, 100, 8190, 1024, 8, true> request;
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string data;
  std::string response_body;
  tx_response response(request_router_.handle_request(request, data, response_body));
  BOOST_CHECK_EQUAL(static_cast<int>(response_status::code::OK),
                    response.status());
//  std::cout << "SimpleRouteTest1: "<< response_body << std::endl;
}

BOOST_AUTO_TEST_CASE(SimpleRouteTest2)
{
  // A simple PUT request
  std::string request_data(put_name_request);
  request_data += CRLF;
  std::string::iterator next(request_data.begin());
  rx_request<1024, 8, 100, 8190, 1024, 8, true> request;
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string data;
  std::string response_body;
  tx_response response(request_router_.handle_request(request, data, response_body));
  BOOST_CHECK_EQUAL(static_cast<int>(response_status::code::OK),
                    response.status());
//  std::cout << "SimpleRouteTest2: "<< response_body << std::endl;
}

BOOST_AUTO_TEST_CASE(SimpleRouteTest3)
{
  // A simple GET request
  std::string request_data(get_customer_request);
  request_data += CRLF;
  std::string::iterator next(request_data.begin());
  rx_request<1024, 8, 100, 8190, 1024, 8, true> request;
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string data;
  std::string response_body;
  tx_response response(request_router_.handle_request(request, data, response_body));
  BOOST_CHECK_EQUAL(static_cast<int>(response_status::code::OK),
                    response.status());
//  std::cout << "SimpleRouteTest3: "<< response_body << std::endl;
}

BOOST_AUTO_TEST_CASE(ComplexRouteTest1)
{
  // A simple GET request
  std::string request_data(get_customer_name_request);
  request_data += CRLF;
  std::string::iterator next(request_data.begin());
  rx_request<1024, 8, 100, 8190, 1024, 8, true> request;
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string data;
  std::string response_body;
  tx_response response(request_router_.handle_request(request, data, response_body));
  BOOST_CHECK_EQUAL(static_cast<int>(response_status::code::NOT_IMPLEMENTED),
                    response.status());
//  std::cout << "ComplexRouteTest1: "<< response_body << std::endl;
}

BOOST_AUTO_TEST_CASE(ComplexRouteTest2)
{
  // A simple GET request
  std::string request_data(get_name_address_request);
  request_data += CRLF;
  std::string::iterator next(request_data.begin());
  rx_request<1024, 8, 100, 8190, 1024, 8, true> request;
  BOOST_CHECK(request.parse(next, request_data.end()));

  std::string data;
  std::string response_body;
  tx_response response(request_router_.handle_request(request, data, response_body));
  BOOST_CHECK_EQUAL(static_cast<int>(response_status::code::NO_CONTENT),
                    response.status());
//  std::cout << "ComplexRouteTest2: "<< response_body << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()
//////////////////////////////////////////////////////////////////////////////
