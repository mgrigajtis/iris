#ifndef IRISMQTTCLIENT_H
#define IRISMQTTCLIENT_H

#include <vector>
#include <iostream>
#include <pqxx/pqxx>
#include <unordered_map>
#include <mosquittopp.h>
#include "headers/topic.h"

class IrisMQTTClient : public mosqpp::mosquittopp {
private:
    std::vector<Topic>* topics;  // Pointer to topics list
    pqxx::connection* conn;
    std::unordered_map<std::string, Topic> topic_map;

public:
    IrisMQTTClient(const char *id, const char *host, const int port, const int timeout, std::vector<Topic>* topics_list, pqxx::connection* conn);

    void on_message(const struct mosquitto_message *message) override;

};

#endif // IRISMQTTCLIENT_H