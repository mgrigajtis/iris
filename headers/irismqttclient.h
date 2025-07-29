#ifndef IRISMQTTCLIENT_H
#define IRISMQTTCLIENT_H

#include <vector>
#include <iostream>
#include <pqxx/pqxx>
#include <unordered_map>
#include <mosquittopp.h>
#include "headers/topic.h"

struct MessageRecord {
    std::string topic;
    std::string payload;
};

class IrisMQTTClient : public mosqpp::mosquittopp {
private:
    std::vector<Topic>* topics;  // Pointer to topics list
    pqxx::connection* conn;
    std::unordered_map<std::string, Topic> topic_map;
    int batch_size;
    std::vector<int> id_list;
    std::vector<MessageRecord> message_buffer;

public:
    IrisMQTTClient(const char *id, const char *host, const int port, const int timeout, std::vector<Topic>* topics_list, const int batchSize, pqxx::connection* conn);

    void on_message(const struct mosquitto_message *message) override;

};

#endif // IRISMQTTCLIENT_H