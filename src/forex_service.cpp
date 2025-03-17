#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <cstdlib>
#include <map>

#include <grpcpp/grpcpp.h>
#include <curl/curl.h>
#include "forex.grpc.pb.h"
#include "forex.pb.h"
#include "nlohmann/json.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using forex::ForexRequest;
using forex::ForexResponse;
using forex::ForexService;
using json = nlohmann::json;

class ForexServiceImpl final : public ForexService::Service {
    Status GetForexRate(ServerContext* context, const ForexRequest* request, ForexResponse* response) override {
        std::cout << "Received request: from_currency=" << request->from_currency() << ", to_currency=" << request->to_currency() << std::endl;

        double rate;
        std::string timestamp;
        if (!fetchForexRate(request->from_currency(), request->to_currency(), rate, timestamp)) {
            std::cerr << "Failed to fetch forex rate" << std::endl;
            return Status(grpc::StatusCode::INTERNAL, "Failed to fetch forex rate");
        }

        response->set_rate(rate);
        response->set_timestamp(timestamp);

        std::cout << "Sending response: rate=" << response->rate() << ", timestamp=" << response->timestamp() << std::endl;

        return Status::OK;
    }

private:
    bool fetchForexRate(const std::string& from, const std::string& to, double& rate, std::string& timestamp) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "Failed to initialize CURL" << std::endl;
            return false;
        }

        std::string apiKey = std::getenv("FOREX_API_KEY");
        if (apiKey.empty()) {
             std::cerr << "FOREX_API_KEY environment variable not set" << std::endl;
            return false;
        }

        std::string url = "https://openexchangerates.org/api/latest.json?app_id=" + apiKey + "&base=" + from;

        std::string responseData;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            return false;
        }

        curl_easy_cleanup(curl);

        try {
            json jsonData = json::parse(responseData);
            if (jsonData.find("rates") == jsonData.end() || !jsonData["rates"].is_object()) {
                std::cerr << "Invalid API response: rates not found or not an object" << std::endl;
                return false;
            }

            if (jsonData["rates"].find(to) == jsonData["rates"].end() || !jsonData["rates"][to].is_number()) {
                std::cerr << "Currency not found: " << to << std::endl;
                return false;
            }

            rate = jsonData["rates"][to].get<double>();
            time_t timestamp_t = jsonData["timestamp"].get<time_t>();

            std::stringstream ss;
            ss << std::put_time(std::localtime(&timestamp_t), "%Y-%m-%dT%H:%M:%SZ");
            timestamp = ss.str();

        } catch (const json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
            return false;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return false;
        }

        return true;
    }

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
        size_t totalSize = size * nmemb;
        output->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    ForexServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

int main(int argc, char** argv) {
    RunServer();
    return 0;
}