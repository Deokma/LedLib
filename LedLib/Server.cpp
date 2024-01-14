#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <iostream>
#include <thread>
#include "simpleble_wrapper.h"
#include <common/utils.hpp>

using namespace web;
using namespace http;
using namespace http::experimental::listener;

class Server {
    http_listener listener;
    SimpleBLE::Peripheral peripheral;
    bool isRunning;

public:
    Server() : listener(L"http://localhost:25334"), isRunning(true) {
        // Move the BLE device connection logic to a separate thread
        std::thread([this] { connectToBLEDevice(); }).detach();

        listener.support(methods::POST, std::bind(&Server::handle_post, this, std::placeholders::_1));
    }

    void handle_post(http_request request) {
        request.extract_json().then([=](pplx::task<json::value> task) {
            try {
                json::value data = task.get();
                if (data.has_string_field(U("data"))) {
                    std::wstring wdata = data[U("data")].as_string();
                    std::string str_data(wdata.begin(), wdata.end());

                    // Call the simple_write function with the received data
                    simple_write(peripheral, str_data);

                    request.reply(status_codes::OK, U("Data received and processed"));
                }
                else {
                    request.reply(status_codes::BadRequest, U("Missing 'data' field in JSON"));
                }
            }
            catch (const std::exception& e) {
                request.reply(status_codes::InternalError, U("Error processing JSON: ") + utility::conversions::to_string_t(e.what()));
            }
            });
    }

    void run() {
        listener.open().wait();
        std::wcout << L"Listening for requests at: " << listener.uri().to_string() << std::endl;

        while (isRunning) {
            std::this_thread::sleep_for(std::chrono::seconds(6));
            // Reconnect to BLE device every 20 seconds
            connectToBLEDevice();
        }
    }

    // Stop the server
    void stop() {
        isRunning = false;
        listener.close().wait();
    }

private:
    void connectToBLEDevice() {
        auto adapter_optional = Utils::getAdapter();

        if (!adapter_optional.has_value()) {
            std::cout << "Bluetooth adapter not found." << std::endl;
            return;
        }

        auto adapter = adapter_optional.value();

        // Scan for the device and connect
        adapter.set_callback_on_scan_found([&](SimpleBLE::Peripheral foundPeripheral) {
            if (foundPeripheral.address() == "be:59:50:01:b0:14") {
                std::cout << "Found device: " << foundPeripheral.identifier() << " [" << foundPeripheral.address() << "]" << std::endl;
                peripheral = foundPeripheral;
                std::cout << "Connecting to " << peripheral.identifier() << " [" << peripheral.address() << "]" << std::endl;
                peripheral.connect();  // Connect to the found peripheral
            }
            });

        adapter.set_callback_on_scan_start([]() { std::cout << "Scan started." << std::endl; });
        adapter.set_callback_on_scan_stop([]() { std::cout << "Scan stopped." << std::endl; });

        adapter.scan_for(5000);
        adapter.scan_stop();
    }
};

// Exported function to start the server
extern "C" {
    __declspec(dllexport) void startServer() {
        Server server;
        server.run();
    }

    // Exported function to stop the server
    __declspec(dllexport) void stopServer() {
        Server server;
        server.stop();
    }
}
