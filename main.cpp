
#include "server/server.h"
#include <boost/asio/signal_set.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/json/object.hpp>
#include <boost/json/value.hpp>
#include <boost/mysql/handshake_params.hpp>
#include <chrono>
#include <csignal>
#include <exception>
#include <sodium.h>

namespace boost {
namespace mysql {

// Overload tag_invoke to tell Boost.JSON how to convert this type
void tag_invoke(json::value_from_tag, json::value &v, const datetime &dt) {
  if (dt.valid()) {
    std::string s = std::format("{:%a-%d-%Y %I:%M:%S}", dt.as_time_point());
    auto k = s.rfind(".");
    if (k != std::string::npos) {
      auto j = s.substr(0, k);
      v = j;
      return;
    }
    v = s;
  } else {
    v = nullptr; // Represent invalid MySQL dates as null
  }
}

} // namespace mysql
} // namespace boost

int main(int, char **) {

  if (sodium_init() == -1) {
    return 1;
  }


  // auto token =
  //     jwt::create<jwt::traits::boost_json>(jwt::default_clock{})
  //         .set_payload_claim("userInfo",
  //                            boost::json::object{
  //                             {"role","user"},
  //                             {"iat", std::chrono::system_clock::now()},
  //                             {"expire_at", std::chrono::steady_clock::now() + std::chrono::seconds(30)}
  //                            });
      // auto path =
      //     boost::urls::parse_uri("http://localhost:5555/api/test/?user=johhn");
      // std::cout << path->has_query() << std::endl;
      // std::cout << path->path() << std::endl;
      // std::cout << path->query() << std::endl;

      // for(auto c : path->params()){
      //   std::cout << c.key << " = " << c.value;
      // }

      boost::beast::error_code ec;
  // Logger::Log(ec, "hello");
  Server server;
  boost::asio::io_context ioc;

  
  boost::mysql::pool_params poolParams;
  poolParams.server_address.emplace_host_and_port("localhost", 33061);
  poolParams.password = "1914";
  poolParams.username = "root";
  poolParams.database = "boostbeastblog";
  poolParams.thread_safe = true;
  poolParams.multi_queries = true;

  // Construct the pool.
  // ctx will be used to create the connections and other I/O objects
  boost::mysql::connection_pool pool(ioc, std::move(poolParams));

  boost::asio::signal_set signSet(ioc, SIGINT, SIGTERM);
  boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), 5333);
  boost::asio::co_spawn(boost::asio::make_strand(ioc),
                        server.startServer(ep, pool), [](std::exception_ptr e) {
                          if (e) {
                            std::cout << "from main" << std::endl;
                            try {
                              std::rethrow_exception(e);
                            } catch (std::exception &ec) {
                              std::cerr << "caught error from exeption"
                                        << std::endl;
                              std::cerr << ec.what() << std::endl;
                            }
                          }
                        });
  signSet.async_wait(
      [&](const boost::beast::error_code &ec, int) { ioc.stop(); });
  pool.async_run(boost::asio::detached);
  ioc.run();
  std::cout << "Hello, from tt!\n";
  return 0;
}
