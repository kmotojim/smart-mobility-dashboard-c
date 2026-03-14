#include "VehicleResource.h"

using json = nlohmann::json;

VehicleResource::VehicleResource(std::shared_ptr<VehicleService> service)
    : vehicleService_(service) {
}

void VehicleResource::setupRoutes(httplib::Server& server) {
    // 現在の車両状態を取得
    server.Get("/api/vehicle/state", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetState(req, res);
    });

    // 加速
    server.Post("/api/vehicle/accelerate", [this](const httplib::Request& req, httplib::Response& res) {
        handleAccelerate(req, res);
    });

    // 減速
    server.Post("/api/vehicle/decelerate", [this](const httplib::Request& req, httplib::Response& res) {
        handleDecelerate(req, res);
    });

    // ギア変更
    server.Post(R"(/api/vehicle/gear/(\w+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleChangeGear(req, res);
    });

    // シートベルト状態変更
    server.Post(R"(/api/vehicle/seatbelt/(true|false))", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetSeatbelt(req, res);
    });

    // エンジン異常発生/解除
    server.Post(R"(/api/vehicle/engine-error/(true|false))", [this](const httplib::Request& req, httplib::Response& res) {
        handleSetEngineError(req, res);
    });

    // 状態リセット
    server.Post("/api/vehicle/reset", [this](const httplib::Request& req, httplib::Response& res) {
        handleReset(req, res);
    });

}

json VehicleResource::vehicleStateToJson(const VehicleState& state) const {
    return json{
        {"speed", state.getSpeed()},
        {"gear", VehicleState::gearToString(state.getGear())},
        {"engineWarning", state.isEngineWarning()},
        {"seatbeltWarning", state.isSeatbeltWarning()},
        {"speedWarning", state.isSpeedWarning()}
    };
}

void VehicleResource::handleGetState(const httplib::Request& req, httplib::Response& res) {
    const auto& state = vehicleService_->getState();
    res.set_content(vehicleStateToJson(state).dump(), "application/json");
}

void VehicleResource::handleAccelerate(const httplib::Request& req, httplib::Response& res) {
    const auto& state = vehicleService_->accelerate();
    res.set_content(vehicleStateToJson(state).dump(), "application/json");
}

void VehicleResource::handleDecelerate(const httplib::Request& req, httplib::Response& res) {
    const auto& state = vehicleService_->decelerate();
    res.set_content(vehicleStateToJson(state).dump(), "application/json");
}

void VehicleResource::handleChangeGear(const httplib::Request& req, httplib::Response& res) {
    try {
        std::string gearStr = req.matches[1];
        // Convert to uppercase
        std::transform(gearStr.begin(), gearStr.end(), gearStr.begin(), ::toupper);

        auto gear = VehicleState::stringToGear(gearStr);
        const auto& state = vehicleService_->changeGear(gear);
        res.set_content(vehicleStateToJson(state).dump(), "application/json");
    } catch (const std::invalid_argument& e) {
        json error = {{"error", std::string("Invalid gear. Valid values: P, R, N, D")}};
        res.status = 400;
        res.set_content(error.dump(), "application/json");
    }
}

void VehicleResource::handleSetSeatbelt(const httplib::Request& req, httplib::Response& res) {
    bool fastened = req.matches[1] == "true";
    const auto& state = vehicleService_->setSeatbelt(fastened);
    res.set_content(vehicleStateToJson(state).dump(), "application/json");
}

void VehicleResource::handleSetEngineError(const httplib::Request& req, httplib::Response& res) {
    bool hasError = req.matches[1] == "true";
    const auto& state = vehicleService_->setEngineError(hasError);
    res.set_content(vehicleStateToJson(state).dump(), "application/json");
}

void VehicleResource::handleReset(const httplib::Request& req, httplib::Response& res) {
    const auto& state = vehicleService_->reset();
    res.set_content(vehicleStateToJson(state).dump(), "application/json");
}
