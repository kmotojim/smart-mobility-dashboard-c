#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include <httplib.h>
#include "VehicleResource.h"

using json = nlohmann::json;

/**
 * VehicleResource REST APIの統合テスト
 *
 * テスト用にHTTPサーバーを起動し、HTTPクライアントで実際にリクエストを送信して検証する。
 */

class VehicleResourceTest : public ::testing::Test {
protected:
    static std::unique_ptr<httplib::Server> server;
    static std::shared_ptr<VehicleService> vehicleService;
    static std::unique_ptr<VehicleResource> vehicleResource;
    static std::thread serverThread;
    static int port;

    static void SetUpTestSuite() {
        vehicleService = std::make_shared<VehicleService>();
        vehicleResource = std::make_unique<VehicleResource>(vehicleService);
        server = std::make_unique<httplib::Server>();

        server->set_default_headers({
            {"Access-Control-Allow-Origin", "*"},
            {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
            {"Access-Control-Allow-Headers", "Content-Type"}
        });

        vehicleResource->setupRoutes(*server);

        port = server->bind_to_any_port("127.0.0.1");
        serverThread = std::thread([&]() {
            server->listen_after_bind();
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    static void TearDownTestSuite() {
        server->stop();
        if (serverThread.joinable()) {
            serverThread.join();
        }
    }

    void SetUp() override {
        httplib::Client cli("127.0.0.1", port);
        auto res = cli.Post("/api/vehicle/reset");
        ASSERT_TRUE(res);
        ASSERT_EQ(200, res->status);
    }

    httplib::Client createClient() {
        return httplib::Client("127.0.0.1", port);
    }
};

std::unique_ptr<httplib::Server> VehicleResourceTest::server;
std::shared_ptr<VehicleService> VehicleResourceTest::vehicleService;
std::unique_ptr<VehicleResource> VehicleResourceTest::vehicleResource;
std::thread VehicleResourceTest::serverThread;
int VehicleResourceTest::port = 0;

TEST_F(VehicleResourceTest, GET_state_初期状態を取得できる) {
    auto cli = createClient();
    auto res = cli.Get("/api/vehicle/state");

    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ(0, body["speed"].get<int>());
    EXPECT_EQ("P", body["gear"].get<std::string>());
    EXPECT_EQ(false, body["engineWarning"].get<bool>());
    EXPECT_EQ(true, body["seatbeltWarning"].get<bool>());
}

TEST_F(VehicleResourceTest, POST_accelerate_Dレンジで加速できる) {
    auto cli = createClient();

    cli.Post("/api/vehicle/gear/D");

    auto res = cli.Post("/api/vehicle/accelerate");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ(10, body["speed"].get<int>());
}

TEST_F(VehicleResourceTest, POST_decelerate_減速できる) {
    auto cli = createClient();

    cli.Post("/api/vehicle/gear/D");
    cli.Post("/api/vehicle/accelerate");
    cli.Post("/api/vehicle/accelerate");

    auto res = cli.Post("/api/vehicle/decelerate");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ(10, body["speed"].get<int>());
}

TEST_F(VehicleResourceTest, POST_gear_有効なギアに変更できる) {
    auto cli = createClient();

    auto res = cli.Post("/api/vehicle/gear/D");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ("D", body["gear"].get<std::string>());
}

TEST_F(VehicleResourceTest, POST_gear_無効なギアでエラーを返す) {
    auto cli = createClient();

    auto res = cli.Post("/api/vehicle/gear/X");
    ASSERT_TRUE(res);
    EXPECT_EQ(400, res->status);

    auto body = json::parse(res->body);
    EXPECT_TRUE(body["error"].get<std::string>().find("Invalid gear") != std::string::npos);
}

TEST_F(VehicleResourceTest, POST_gear_Rレンジに変更できる) {
    auto cli = createClient();

    auto res = cli.Post("/api/vehicle/gear/R");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ("R", body["gear"].get<std::string>());
}

TEST_F(VehicleResourceTest, POST_gear_Nレンジに変更できる) {
    auto cli = createClient();

    auto res = cli.Post("/api/vehicle/gear/N");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ("N", body["gear"].get<std::string>());
}

TEST_F(VehicleResourceTest, POST_gear_小文字でも変更できる) {
    auto cli = createClient();

    auto res = cli.Post("/api/vehicle/gear/d");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ("D", body["gear"].get<std::string>());
}

TEST_F(VehicleResourceTest, POST_seatbelt_シートベルト状態を変更できる) {
    auto cli = createClient();

    auto res = cli.Post("/api/vehicle/seatbelt/true");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ(false, body["seatbeltWarning"].get<bool>());
}

TEST_F(VehicleResourceTest, POST_engine_error_エンジン異常を発生できる) {
    auto cli = createClient();

    auto res = cli.Post("/api/vehicle/engine-error/true");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ(true, body["engineWarning"].get<bool>());
}

TEST_F(VehicleResourceTest, POST_engine_error_エンジン異常を解除できる) {
    auto cli = createClient();

    cli.Post("/api/vehicle/engine-error/true");

    auto res = cli.Post("/api/vehicle/engine-error/false");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ(false, body["engineWarning"].get<bool>());
}

TEST_F(VehicleResourceTest, POST_seatbelt_シートベルトを解除できる) {
    auto cli = createClient();

    cli.Post("/api/vehicle/seatbelt/true");

    auto res = cli.Post("/api/vehicle/seatbelt/false");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ(true, body["seatbeltWarning"].get<bool>());
}

TEST_F(VehicleResourceTest, POST_reset_状態をリセットできる) {
    auto cli = createClient();

    cli.Post("/api/vehicle/gear/D");
    cli.Post("/api/vehicle/accelerate");
    cli.Post("/api/vehicle/engine-error/true");

    auto res = cli.Post("/api/vehicle/reset");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ(0, body["speed"].get<int>());
    EXPECT_EQ("P", body["gear"].get<std::string>());
    EXPECT_EQ(false, body["engineWarning"].get<bool>());
}

TEST_F(VehicleResourceTest, 速度超過で警告が発生する) {
    auto cli = createClient();

    cli.Post("/api/vehicle/gear/D");

    for (int i = 0; i < 13; i++) {
        cli.Post("/api/vehicle/accelerate");
    }

    auto res = cli.Get("/api/vehicle/state");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ(130, body["speed"].get<int>());
    EXPECT_EQ(true, body["speedWarning"].get<bool>());
}

TEST_F(VehicleResourceTest, POST_accelerate_Pレンジでは加速しない) {
    auto cli = createClient();

    auto res = cli.Post("/api/vehicle/accelerate");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ(0, body["speed"].get<int>());
}

TEST_F(VehicleResourceTest, POST_decelerate_速度0から減速しても0のまま) {
    auto cli = createClient();

    auto res = cli.Post("/api/vehicle/decelerate");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ(0, body["speed"].get<int>());
}

TEST_F(VehicleResourceTest, 走行中はギア変更できない_安全インターロック) {
    auto cli = createClient();

    cli.Post("/api/vehicle/gear/D");
    cli.Post("/api/vehicle/accelerate");

    auto res = cli.Post("/api/vehicle/gear/P");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ("D", body["gear"].get<std::string>());
}

TEST_F(VehicleResourceTest, エンジン異常時に高速走行中は60に強制減速) {
    auto cli = createClient();

    cli.Post("/api/vehicle/gear/D");
    for (int i = 0; i < 10; i++) {
        cli.Post("/api/vehicle/accelerate");
    }

    auto res = cli.Post("/api/vehicle/engine-error/true");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ(60, body["speed"].get<int>());
    EXPECT_EQ(true, body["engineWarning"].get<bool>());
}

TEST_F(VehicleResourceTest, 最大速度180を超えない) {
    auto cli = createClient();

    cli.Post("/api/vehicle/gear/D");

    for (int i = 0; i < 20; i++) {
        cli.Post("/api/vehicle/accelerate");
    }

    auto res = cli.Get("/api/vehicle/state");
    ASSERT_TRUE(res);
    EXPECT_EQ(200, res->status);

    auto body = json::parse(res->body);
    EXPECT_EQ(180, body["speed"].get<int>());
}
