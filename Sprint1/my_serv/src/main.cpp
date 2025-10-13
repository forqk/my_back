#include <boost/asio.hpp>
#include <array>
#include <iostream>
#include <string>
#include <string_view>

namespace net = boost::asio;
// TCP больше не нужен, импортируем имя UDP
using net::ip::udp;

using namespace std::literals;

static const int port = 3333;
static const size_t max_buffer_size = 1024;

void Server(){
    try {
        boost::asio::io_context io_context;

        udp::socket socket(io_context, udp::endpoint(udp::v4(), port));

        // Запускаем сервер в цикле, чтобы можно было работать со многими клиентами
        for (;;) {
            // Создаём буфер достаточного размера, чтобы вместить датаграмму.
            std::array<char, max_buffer_size> recv_buf;
            udp::endpoint remote_endpoint;

            // Получаем не только данные, но и endpoint клиента
            auto size = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);

            std::cout << "Client said "sv << std::string_view(recv_buf.data(), size) << std::endl;

            // Отправляем ответ на полученный endpoint, игнорируя ошибку.
            // На этот раз не отправляем перевод строки: размер датаграммы будет получен автоматически.
            boost::system::error_code ignored_error;
            socket.send_to(boost::asio::buffer("Hello from UDP-server"sv), remote_endpoint, 0, ignored_error);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

bool Client(int argc, const char** argv){
    std::cout << argc << std::endl;
    if (argc != 2) {
        std::cout << "Usage: "sv << argv[0] << " <server IP>"sv << std::endl;
        return false;
    }

    try {
        net::io_context io_context;

        // Перед отправкой данных нужно открыть сокет.
        // При открытии указываем протокол (IPv4 или IPv6) вместо endpoint.
        udp::socket socket(io_context, udp::v4());

        boost::system::error_code ec;
        auto endpoint = udp::endpoint(net::ip::make_address(argv[1], ec), port);
        socket.send_to(net::buffer("Hello from UDP-client"sv), endpoint);

        // Получаем данные и endpoint.
        std::array<char, max_buffer_size> recv_buf;
        udp::endpoint sender_endpoint;
        size_t size = socket.receive_from(net::buffer(recv_buf), sender_endpoint);

        std::cout << "Server responded "sv << std::string_view(recv_buf.data(), size) << std::endl;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}


int main(int argc, const char** argv) {
    if(argv[1] == "Server"sv){
        std::cout << "Start server" << std::endl;
        Server();
    } else {
        std::cout << "Start client" << std::endl;
        if(!Client(argc, argv)){
            return 1;
        }
    }

    return 0;

}
