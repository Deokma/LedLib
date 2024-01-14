#include <iomanip>
#include <iostream>
#include <vector>
#include <cstring>

#include "simpleble/SimpleBLE.h"
#include "common/utils.hpp"
#include "simpleble_wrapper.h"
#include "cpprest/http_listener.h"
#include "cpprest/json.h"

using namespace web;
using namespace http;
using namespace http::experimental::listener;
using namespace SimpleBLE;

int simple_write(SimpleBLE::Peripheral& peripheral, const std::string& data) {
    // Store all service and characteristic uuids in a vector.
    std::vector<std::pair<SimpleBLE::BluetoothUUID, SimpleBLE::BluetoothUUID>> uuids;
    for (auto service : peripheral.services()) {
        for (auto characteristic : service.characteristics()) {
            uuids.push_back(std::make_pair(service.uuid(), characteristic.uuid()));
        }
    }

    // Find the index of the desired service and characteristic UUIDs
    size_t desired_index = 1;  // Assume the first characteristic is the desired one

    // Check if the desired characteristic was found
    if (desired_index >= uuids.size()) {
        std::cout << "Desired characteristic not found." << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Using the following characteristic:" << std::endl;
    std::cout << uuids[desired_index].first << " " << uuids[desired_index].second << std::endl;

    // Convert the received data to byte array
    ByteArray byteArray;
    for (std::size_t i = 0; i < data.length(); i += 2) {
        std::string byteString = data.substr(i, 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), NULL, 16);
        byteArray.push_back(byte);
    }

    // Write the converted data to the characteristic
    peripheral.write_command(uuids[desired_index].first, uuids[desired_index].second, byteArray);

    return EXIT_SUCCESS;
}