#ifndef REQUEST_ROUTER_HPP_VIA_HTTPLIB_
#define REQUEST_ROUTER_HPP_VIA_HTTPLIB_

#pragma once

//////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2015-2021 Ken Barker
// (ken dot barker at via-technology dot co dot uk)
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//////////////////////////////////////////////////////////////////////////////
/// @file request_router.hpp
/// @brief Classes to route HTTP requests.
//////////////////////////////////////////////////////////////////////////////
#include "via/http/request_handler.hpp"
#include "via/http/request_uri.hpp"
#include "via/http/authentication/authentication.hpp"
#include <boost/algorithm/string.hpp>
#include <map>

namespace via
{
  namespace http
  {
    /// A map of strings to hold route parameters for the request handlers.
    typedef std::map<std::string, std::string> Parameters;

    /// Get the route parameters from the uri_path given the route_path.
    /// @param uri_path the path received in the request_uri.
    /// @param route_path the path in the Route.
    /// @return the map of route parameter name:value pairs extracted from the
    /// paths, empty if none or if their was a problem reading the parameters.
    inline Parameters get_route_parameters(std::string uri_path, std::string route_path)
    {
      Parameters parameters;

      // Find the first ':' in the route_path
      auto param_start(route_path.find(':'));
      if (param_start != std::string::npos)
      {
        // erase both paths prior to the ':'
        route_path = route_path.substr(param_start);
        uri_path   = uri_path.substr(param_start);

        // get the strings between the '/'s
        std::vector<std::string> names;
        boost::split(names, route_path, boost::is_any_of("/"));

        std::vector<std::string> values;
        boost::split(values, uri_path, boost::is_any_of("/"));

        // get the route_parameter name value pairs
        if (names.size() == values.size())
        {
          for (auto i(0U); i < names.size(); ++i)
          {
            std::string name(names[i]);
            if ((name[0] != ':') && (name != values[i]))
              return Parameters();
            else
            {
              name.erase(0, 1); // ignore the ':'
              parameters.insert(Parameters::value_type(name, values[i]));
            }
          }
        }
      }

      return parameters;
    }

    /// Get the route parameter with the given name from the route parameters.
    /// @param params a map of route parameter name:value pairs.
    /// @param name a parameter name.
    /// @return the value corresponding to the parameter name, or an empty
    /// string if not found.
    inline std::string get_parameter(Parameters const& params,
                                     std::string const& name)
    {
      auto iter(params.find(name));
      return (iter != params.cend()) ? iter->second : std::string();
    }

    /// @class request_router
    /// The class contains the route paths to search in HTTP requests.
    /// Note: the routes are searched in the order that they are added.
    template <typename Container, typename R>
    class request_router : public request_handler<Container, R>
    {
    public:
      /// An HTTP request handler function.
      typedef std::function<tx_response (R const& request,
                                         Parameters const& parameters,
                                         Container const& data,
                                         Container& response_body)> Handler;

      /// A request handler with an (optional) authentication object pointer.
      struct AuthenticatedHandler
      {
        Handler handler;
        authentication::authentication const* auth_ptr;
      };

      /// A map of handlers
      typedef std::map<std::string, AuthenticatedHandler> MethodHandlers;

      /// The value_type stored in the map of handlers
      typedef typename MethodHandlers::value_type MethodHandlers_value_type;

      /// @class Route
      /// The data stored for each route associated with this type of request.
      struct Route
      {
        /// The search path including ':' parameters, if any.
        std::string    path;
        /// The search path upto the first ':' parameter, if any.
        std::string    search_path;
        /// The map of HTTP methods to request handlers.
        MethodHandlers method_handlers;

        /// Constructor
        Route(std::string const& path_str,
              MethodHandlers_value_type method_handler)
          : path(path_str)
          , search_path(path_str)
          , method_handlers{method_handler}
        {
          // Find the first ':' in the path
          auto param_start(search_path.find(':'));
          if (param_start != std::string::npos)
            search_path.erase(param_start); // delete it and everything after it
        }

        /// Whether the Route has parameters i.e. a ':'
        bool has_parameters() const
        { return path.size() != search_path.size(); }

        /// The string of methods allowed for a given url
        std::string allowed_methods() const
        {
          std::string text;
          auto iter(method_handlers.cbegin());
          if (iter != method_handlers.cend())
            text = iter->first;

          for (++iter; iter != method_handlers.cend(); ++iter)
            text += ", " + iter->first;

          return text;
        }

        /// Whether a given route matches a path
        friend bool operator==(Route const& lhs, std::string const& path)
        { return lhs.path == path; }
      };

      /// A collection of routes.
      typedef std::vector<Route> Routes;

      /// A const_iterator to the collection of routes.
      typedef typename Routes::const_iterator Routes_const_iterator;

    private:

      /// The routes to search for an HTTP request.
      Routes routes_;

      /// Searches for the request in the routes collection.
      /// @param uri_path the http request uri path
      /// @retval parameters the route paramters (if any)
      /// @return a constant iterator to the route in routes_, cend if not found.
      Routes_const_iterator find_route(std::string const& uri_path,
                                       Parameters& parameters) const
      {
        Routes_const_iterator iter(routes_.cbegin());
        for (; iter != routes_.cend (); ++iter)
        {
          bool found_path(uri_path.find(iter->search_path) != std::string::npos);
          if (found_path)
          {
            if (iter->has_parameters())
            {
              parameters = get_route_parameters(uri_path, iter->path);
              if (!parameters.empty())
                break;
            }
            else if (uri_path.size() == iter->search_path.size())
              break;
          }
        }

        return iter;
      }

    public:

      /// Constructor
      request_router() = default;

      /// Destructor
      virtual ~request_router()
      {}

      /// Add a method and it's handler to the given path.
      /// Creates the path if it's not already got any handlers.
      /// @param method the method name (an uppercase string).
      /// @param path the uri path. Note: it may contain ':' characters to
      /// capture paramters from the uri path like Node.js.
      /// @param handler the request handler to be called.
      /// @param auth_ptr a shared pointer to an authentication, default nullptr.
      /// @return true if the path is new, false otherwise.
      bool add_method(std::string_view method, std::string_view path,
                      Handler handler,
                      authentication::authentication const* auth_ptr = nullptr)
      {
        // Serach for the path in the existing routes
        auto iter(std::find(routes_.begin(), routes_.end(), path.data()));
        bool is_new_path(iter == routes_.end());
        if (is_new_path)
          routes_.push_back(Route(std::string(path),
                    MethodHandlers_value_type(std::string(method), { handler, auth_ptr })));
        else
          iter->method_handlers.insert
              (MethodHandlers_value_type(std::string(method), { handler, auth_ptr }));

        return is_new_path;
      }

      /// Add a method and it's handler to the given path.
      /// Creates the path if it's not already got any handlers.
      /// @param method_id the method id, e.g. request_method::id::GET.
      /// @param path the uri path. Note: it may contain ':' characters to
      /// capture paramters from the uri path like Node.js.
      /// @param handler the request handler to be called.
      /// @param auth_ptr a shared pointer to an authentication, default nullptr.
      /// @return true if the path is new, false otherwise.
      bool add_method(request_method::id method_id, std::string_view path,
                      Handler handler,
                      authentication::authentication const* auth_ptr = nullptr)
      { return add_method(request_method::name(method_id), path, handler, auth_ptr); }

      /// The function handle HTTP requests.
      /// It validates the request and routes it to the
      /// @param request the HTTP request.
      /// @param request_body the body of the HTTP request.
      /// @retval response_body the body for the HTTP response.
      /// @return the response header from the handler or NOT_FOUND if it could
      /// not find a handler for the request.
      virtual tx_response handle_request(R const& request,
                                         Container const& request_body,
                                         Container& response_body) const
      {
        request_uri uri(request.uri());

        // Search for the path and any route parameters associated with it
        Parameters parameters;
        auto route_itr(find_route(uri.path(), parameters));
        if (route_itr == routes_.cend())
          return tx_response(response_status::code::NOT_FOUND);

        // Search for the method
        auto methods_iter(route_itr->method_handlers.find(request.method()));
        if (methods_iter == route_itr->method_handlers.cend())
        {
          // send a METHOD_NOT_ALLOWED response with an ALLOW header
          tx_response response(response_status::code::METHOD_NOT_ALLOWED);
          response.add_header(header_field::HEADER_ALLOW, route_itr->allowed_methods());
          return response;
        }
        else
        {
          // If this method has authentication
          if (methods_iter->second.auth_ptr)
          {
            // authenticate the request
            std::string challenge
                (methods_iter->second.auth_ptr->authenticate(request));
            if (!challenge.empty())
            {
              // authentication failed, send an UNAUTHORISED response
              tx_response response(response_status::code::UNAUTHORISED);
              response.add_header(header_field::HEADER_WWW_AUTHENTICATE, challenge);
              return response;
            }
          }

          // call the registered handler
          return methods_iter->second.handler(request, parameters,
                                              request_body, response_body);
        }
      }

      /// Accessor for the stored routes
      Routes const& routes() const
      { return routes_; }
    };
  }
}

#endif // REQUEST_ROUTER_HPP_VIA_HTTPLIB_
