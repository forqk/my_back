#include "audio.h"

#include <boost/asio.hpp>
#include <array>
#include <iostream>
#include <string>
#include <string_view>

namespace net = boost::asio;
using net::ip::udp;

using namespace std::literals;

static const int port = 3333;
static const size_t max_buffer_size = 65500;

void StartServer(Player& player){
    std::cout << "Start server" << std::endl;
    try {
        boost::asio::io_context io_context;

        udp::socket socket(io_context, udp::endpoint(udp::v4(), port));

        for (;;) {
            std::array<char, max_buffer_size> recv_buf;
            udp::endpoint remote_endpoint;

            auto size = socket.receive_from(boost::asio::buffer(recv_buf), remote_endpoint);
            auto frames = size / player.GetFrameSize();

            player.PlayBuffer(recv_buf.data(), frames, 1.5s);
            std::cout << "Play Message" << std::endl;

        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}


Recorder::RecordingResult RecordAudio(Recorder& recorder){
    std::string str;

    std::cout << "Press Enter to record message..." << std::endl;

    std::getline(std::cin, str);

    auto rec_result = recorder.Record(65000, 1.5s);

    std::cout << "Recording done" << std::endl;

    return rec_result;
}

void StartClient(const char* addr, Recorder& recorder){
    std::cout << "Start client" << std::endl;

    try {
        net::io_context io_context;

        udp::socket socket(io_context, udp::v4());

        boost::system::error_code ec;

        auto endpoint = udp::endpoint(net::ip::make_address(addr, ec), port);

        while (true) {
            auto audio_line = RecordAudio(recorder);

            size_t bytesToWrite = audio_line.frames * recorder.GetFrameSize();

            socket.send_to(net::buffer(audio_line.data.data(), bytesToWrite), endpoint);
        }

    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}


int main(int argc, const char** argv) {
    Recorder recorder(ma_format_u8, 1);
    Player player(ma_format_u8, 1);

    if(argv[1] == "Server"sv){
        StartServer(player);
    } else {
        if (argc != 2) {
            std::cout << "Usage: "sv << argv[0] << " <server IP>"sv << std::endl;
            return 1;
        }

        StartClient(argv[1], recorder);
    }

    return 0;

}
