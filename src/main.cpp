#include "VehicleResource.h"
#include <iostream>
#include <memory>
#include <csignal>
#include <filesystem>
#include <chrono>

std::unique_ptr<httplib::Server> server;
auto startTime = std::chrono::steady_clock::now();

void signalHandler(int signum) {
    std::cout << "\nシャットダウン中..." << std::endl;
    if (server) {
        server->stop();
    }
    exit(signum);
}

int main() {
    // シグナルハンドラの設定
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // サービスとリソースの初期化
    auto vehicleService = std::make_shared<VehicleService>();
    auto vehicleResource = std::make_unique<VehicleResource>(vehicleService);

    // HTTPサーバーの設定
    server = std::make_unique<httplib::Server>();

    // CORSヘッダーの設定（開発用）
    server->set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type"}
    });

    // 静的ファイルの配信設定（実行ディレクトリからの相対パス）
    std::string publicDir = "./public";
    if (!server->set_mount_point("/", publicDir)) {
        std::cerr << "警告: 静的ファイルディレクトリが見つかりません: " << publicDir << std::endl;
    } else {
        std::cout << "静的ファイル配信: " << std::filesystem::absolute(publicDir).string() << std::endl;
    }

    // ルートの設定
    vehicleResource->setupRoutes(*server);

    // ヘルスチェックエンドポイント
    server->Get("/health", [](const httplib::Request&, httplib::Response& res) {
        auto now = std::chrono::steady_clock::now();
        auto uptimeSec = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
        nlohmann::json health = {
            {"status", "UP"},
            {"uptimeSeconds", uptimeSec}
        };
        res.set_content(health.dump(), "application/json");
    });

    // サーバー起動
    const char* host = "0.0.0.0";
    int port = 8080;

    std::cout << "Smart Mobility Dashboard Backend (C++) v1.1" << std::endl;
    std::cout << "サーバー起動: http://" << host << ":" << port << std::endl;
    std::cout << "終了するには Ctrl+C を押してください" << std::endl;

    if (!server->listen(host, port)) {
        std::cerr << "サーバーの起動に失敗しました" << std::endl;
        return 1;
    }

    return 0;
}
