#include <iostream>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include "headers/irismqttclient.h"

// Function to decode Base64
std::vector<unsigned char> base64_decode(const std::string &b64_input) {
    BIO *bio, *b64;
    int decode_len = b64_input.size() * 3 / 4;
    std::vector<unsigned char> buffer(decode_len);

    bio = BIO_new_mem_buf(b64_input.data(), b64_input.size());
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);

    int len = BIO_read(bio, buffer.data(), buffer.size());
    buffer.resize(len); // Adjust size after decoding

    BIO_free_all(bio);
    return buffer;
}

// Function to remove PKCS7 padding
std::string unpad(const std::string &input) {
    if (input.empty()) return input;
    unsigned char pad_length = input.back();
    if (pad_length > AES_BLOCK_SIZE) return input; // Invalid padding
    return input.substr(0, input.size() - pad_length);
}

// AES-CBC Decryption Function
std::string decrypt_message(const std::string &encrypted_b64, const std::string &key) {
    // Decode Base64 to get IV + encrypted data
    std::vector<unsigned char> full_data = base64_decode(encrypted_b64);

    if (full_data.size() <= AES_BLOCK_SIZE) {
        return ""; // Invalid data, must contain at least IV + 1 byte of encrypted text
    }

    // Extract IV (first 16 bytes)
    std::vector<unsigned char> iv(full_data.begin(), full_data.begin() + AES_BLOCK_SIZE);

    // Extract encrypted data
    std::vector<unsigned char> encrypted_bytes(full_data.begin() + AES_BLOCK_SIZE, full_data.end());

    // Convert key to 32 bytes (padded or truncated)
    std::vector<unsigned char> key_bytes(32, 0);
    std::memcpy(key_bytes.data(), key.data(), std::min(key.size(), size_t(32)));

    // OpenSSL AES decryption setup
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key_bytes.data(), iv.data());

    std::vector<unsigned char> decrypted(encrypted_bytes.size() + AES_BLOCK_SIZE);
    int decrypted_len = 0, final_len = 0;

    EVP_DecryptUpdate(ctx, decrypted.data(), &decrypted_len, encrypted_bytes.data(), encrypted_bytes.size());
    EVP_DecryptFinal_ex(ctx, decrypted.data() + decrypted_len, &final_len);
    
    decrypted.resize(decrypted_len + final_len);
    EVP_CIPHER_CTX_free(ctx);

    // Convert decrypted bytes to string and remove padding
    std::string decrypted_text(reinterpret_cast<char *>(decrypted.data()), decrypted.size());
    return unpad(decrypted_text);
}

void insert_message(pqxx::connection* conn, const std::string& topic, const std::string& message_payload) {
    try {
        if (!conn->is_open()) {
            std::cerr << "Connection to database is not open." << std::endl;
            return;
        }

        // Start a transaction
        pqxx::work txn(*conn);

        // SQL Insert Query (Prepared Statement for security)
        txn.exec_params(
            "INSERT INTO messages (topic, message_payload) VALUES ($1, $2);",
            topic, message_payload
        );

        // Commit the transaction
        txn.commit();

        std::cout << "Message inserted successfully!" << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Database error: " << e.what() << std::endl;
    }
}


IrisMQTTClient::IrisMQTTClient(const char *id, const char *host, int port, int timeout, std::vector<Topic> *topics_list, pqxx::connection* dbconn) 
    : mosquittopp(id), topics(topics_list), conn(dbconn) {
    connect(host, port, timeout);

    std::cout << "Connected to queue" << std::endl;

    // conver the list of topics to a hash map for fast searching
    // Loop through vector and insert each topic into the hash map
    for (const auto& topic : *topics_list) {
        topic_map.emplace(topic.name, topic);
    }
}

void IrisMQTTClient::on_message(const struct mosquitto_message *message) {
    std::string message_topic = static_cast<char *>(message->topic);
    std::string message_payload = static_cast<char *>(message->payload);
    std::string aes_key = "";

    try {
        aes_key = topic_map.at(message_topic).aes_key;  // ✅ Throws if key is missing
    } catch (const std::out_of_range&) {
        std::cerr << "Error: topic not found in topic_map\n";
    }

    if (aes_key != "") {
        message_payload = decrypt_message(message_payload, aes_key);
    }

    std::cout << message_topic << ": " << message_payload << std::endl;

    insert_message(conn, message_topic, message_payload);
}