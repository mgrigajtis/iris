#include <iostream>

class Topic {
public:
    // The name of the topic
    std::string name;

    // The contents of the message can be encrypted with an AES key
    // If this is blank, Iris will assume the message is unencrypted.
    // If it is not blank, Iris will attempt to decrypt the payload of 
    // the message with this key.
    std::string aes_key;

    Topic(const std::string& topic_name, const std::string& aeskey) : name(topic_name), aes_key(aeskey) {}
};