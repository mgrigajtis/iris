# Iris

[![CMake on multiple platforms](https://github.com/mgrigajtis/iris/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/mgrigajtis/iris/actions/workflows/cmake-multi-platform.yml)

Iris is a C++ application that integrates **MQTT messaging** with a **PostgreSQL database**. It listens to specified MQTT topics, processes messages, decrypts them if necessary, and stores them in a PostgreSQL database.

## Features
- **MQTT Integration**: Uses the **Mosquitto** library to subscribe to and receive MQTT messages.
- **PostgreSQL Storage**: Saves incoming messages in a **PostgreSQL database**.
- **AES Encryption Handling**: Supports **AES-CBC encrypted messages** and decrypts them using **OpenSSL**.
- **Configurable Topics**: Reads topics and encryption keys from a JSON configuration file.
- **JSON-Based Configuration**: Uses `jsoncpp` to load database and MQTT settings dynamically.

## Requirements
- C++ Compiler (e.g., `g++` with C++17+ support)
- [Mosquitto](https://mosquitto.org/) (MQTT client library)
- [libpqxx](https://github.com/jtv/libpqxx) (PostgreSQL C++ client library)
- [OpenSSL](https://www.openssl.org/) (For AES decryption)
- [jsoncpp](https://github.com/open-source-parsers/jsoncpp) (For JSON parsing)
- PostgreSQL Database Server

## Installation
### **1. Install Dependencies**
#### **Ubuntu/Debian**:
```bash
sudo apt update && sudo apt install -y libmosquitto-dev libpqxx-dev libssl-dev libjsoncpp-dev
```
#### **MacOS (Homebrew)**:
```bash
brew install mosquitto libpqxx openssl jsoncpp
```
#### **Windows (vcpkg)**:
```bash
vcpkg install mosquitto libpqxx openssl jsoncpp
```

### **2. Build the Project**
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Configuration
Create a `config.json` file in the root directory with the following structure:
```json
{
    "mqtthost": "localhost",
    "mqttport": 1883,
    "mqtttimeout": 60,
    "postgresqlhost": "127.0.0.1",
    "postgresqlport": 5432,
    "postgresqldbname": "mydatabase",
    "postgresqluser": "myuser",
    "postgresqlpassword": "mypassword",
    "topics": [
        { "name": "sports/football", "aeskey": "your_aes_key_here" },
        { "name": "news/weather" }
    ]
}
```

## Running the Application
```bash
./iris
```

### **Expected Output**
```
Connected to database: mydatabase
Subscribed to: sports/football
Subscribed to: news/weather
Receiving messages.
```

## Code Overview
### **Main Components**
- **`iris.cpp`**
  - Loads configuration from `config.json`.
  - Connects to PostgreSQL.
  - Initializes `IrisMQTTClient` and subscribes to topics.
- **`irismqttclient.h / irismqttclient.cpp`**
  - Handles MQTT message reception.
  - Decrypts AES-CBC messages (if encrypted).
  - Stores messages in PostgreSQL.
- **`topic.h`**
  - Represents an MQTT topic with an optional AES key.

## Database Schema
```sql
CREATE TABLE messages (
    id SERIAL PRIMARY KEY,
    topic VARCHAR(255) NOT NULL,
    message_payload TEXT NOT NULL,
    received_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

## Contributing
Feel free to submit **pull requests** or report **issues** if you find bugs or improvements.

## License
This project is licensed under the **MIT License**.

---
**Author**: Matthew Grigajtis  
**Project**: Iris

