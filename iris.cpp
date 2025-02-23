#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <jsoncpp/json/json.h>
#include "headers/irismqttclient.h"

// Define AES block size
#define AES_BLOCK_SIZE 16

Json::Value load_json_from_file(const std::string &filename) {
    std::ifstream file(filename);
    Json::Value root;
    Json::CharReaderBuilder reader;
    std::string errs;

    if (!file) {
        std::cerr << "Error: Could not open " << filename << std::endl;
        return Json::Value();  // Return empty JSON object
    }

    if (!Json::parseFromStream(reader, file, &root, &errs)) {
        std::cerr << "Error parsing JSON: " << errs << std::endl;
        return Json::Value();
    }

    return root;
}

int load_int_value_from_config(const std::string &filename, const std::string property) {
    Json::Value root = load_json_from_file(filename);

    if (root.isNull() || !root.isMember(property) || !root[property].isInt()) {
        std::cerr << "Error: " << property << " is missing or not an integer." << std::endl;
        return -1;
    }

    int int_value = root[property].asInt();

    if (int_value <= 0) {
        std::cerr << "Error: "<< property <<" is null" << std::endl;
    }

    return int_value;
}

std::string load_string_value_from_config(const std::string &filename, const std::string property) {
    Json::Value root = load_json_from_file(filename);

    if (root.isNull() || !root.isMember(property) || !root[property].isString()) {
        std::cerr << "Error: " << property << " is missing or not a string." << std::endl;
        return "";
    }

    std::string host = root[property].asString();

    if (host == "") {
        std::cerr << "Error: " << property << " is blank" << std::endl;
    }

    return root[property].asString();
}

// Function to read topics from config.json
std::vector<Topic> load_topics_from_config(const std::string &filename) {
    Json::Value root = load_json_from_file(filename);
    std::vector<Topic> topics;

    if (root.isNull() || !root.isMember("topics") || !root["topics"].isArray()) {
        std::cerr << "Error: Invalid JSON format (missing 'topics' array)" << std::endl;
        return topics;
    }

    std::string topic_name = "";
    std::string aes_key;

    if (root.isMember("topics") && root["topics"].isArray()) {
        for (const auto& topic : root["topics"]) {
            aes_key = "";

            if (topic.isMember("name") && topic["name"].isString()) {
                topic_name = topic["name"].asString();                
            } else {
                continue;
            }

            if (topic.isMember("aeskey") && topic["aeskey"].isString()) {
                aes_key = topic["aeskey"].asString(); 
            }

            topics.emplace_back(topic_name, aes_key);
        }
    } else {
        std::cerr << "Error: 'topics' field is missing or not an array." << std::endl;
    }

    return topics;
}

int main() {
    // Load MQTT config values from config.json
    std::vector<Topic> topics = load_topics_from_config("./config.json");
    std::string mqtthost = load_string_value_from_config("./config.json", "mqtthost");
    int mqttport = load_int_value_from_config("./config.json", "mqttport");
    int mqtttimeout = load_int_value_from_config("./config.json", "mqtttimeout");

    // Load Postgresql config values from config.json
    std::string postgresqlhost = load_string_value_from_config("./config.json", "postgresqlhost");
    int postgresqlport = load_int_value_from_config("./config.json", "postgresqlport");
    std::string postgresqldbname = load_string_value_from_config("./config.json", "postgresqldbname");
    std::string postgresqluser = load_string_value_from_config("./config.json", "postgresqluser");
    std::string postgresqlpassword = load_string_value_from_config("./config.json", "postgresqlpassword");

    // The MQTT settings are required
    if (mqtthost == "" || mqttport == 0 || mqtttimeout == 0) {
        return 1;
    }

    // Right now the Postgresql settings are required
    // In the future there will be support for other databases
    if (postgresqlhost == "" || postgresqlport == 0 || postgresqldbname == "" || postgresqluser == "" || postgresqlpassword == "") {
        return 1;
    }

    // Exit if there are no Topics to subscribe to
    if (topics.empty()) {
        std::cerr << "No topics found. Exiting." << std::endl;
        return 1;
    }

    pqxx::connection* conn = nullptr; 

    try {
        // Construct connection string dynamically
        std::string conn_str = "dbname=" + postgresqldbname +
                               " user=" + postgresqluser +
                               " password=" + postgresqlpassword +
                               " hostaddr=" + postgresqlhost +
                               " port=" + std::to_string(postgresqlport);

        // Connect to PostgreSQL
        conn = new pqxx::connection(conn_str);

        if (conn->is_open()) {
            std::cout << "Connected to database: " << conn->dbname() << std::endl;
        } else {
            std::cerr << "Connection failed!" << std::endl;
            return 1;
        }
    } catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    // Initialize MQTT
    mosqpp::lib_init();
    IrisMQTTClient client("Iris", mqtthost.c_str(), mqttport, mqtttimeout, &topics, conn);

    // Subscribe to each topic from the config file
    for (const auto &topic : topics) {
        client.subscribe(nullptr, topic.name.c_str());
        std::cout << "Subscribed to: " << topic.name << std::endl;
    }

    std::cout << "Receiving messages." << std::endl;
    client.loop_forever();

    mosqpp::lib_cleanup();
    return 0;
}
