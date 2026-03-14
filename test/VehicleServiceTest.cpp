#include <gtest/gtest.h>
#include "VehicleService.h"

/**
 * VehicleServiceの単体テスト
 */

class VehicleServiceTest : public ::testing::Test {
protected:
    VehicleService vehicleService;

    void SetUp() override {
        vehicleService.reset();
    }
};

// ===== 加速テスト =====

class AccelerateTest : public VehicleServiceTest {};

TEST_F(AccelerateTest, Dレンジで加速すると速度が10増加する) {
    vehicleService.changeGear(VehicleState::Gear::D);

    VehicleState& result = vehicleService.accelerate();

    EXPECT_EQ(10, result.getSpeed());
}

TEST_F(AccelerateTest, Dレンジ以外では加速しない) {
    VehicleState& result = vehicleService.accelerate();

    EXPECT_EQ(0, result.getSpeed());
}

TEST_F(AccelerateTest, Rレンジでは加速しない) {
    vehicleService.changeGear(VehicleState::Gear::R);

    VehicleState& result = vehicleService.accelerate();

    EXPECT_EQ(0, result.getSpeed());
}

TEST_F(AccelerateTest, Nレンジでは加速しない) {
    vehicleService.changeGear(VehicleState::Gear::N);

    VehicleState& result = vehicleService.accelerate();

    EXPECT_EQ(0, result.getSpeed());
}

TEST_F(AccelerateTest, 最大速度180を超えない) {
    vehicleService.changeGear(VehicleState::Gear::D);

    for (int i = 0; i < 20; i++) {
        vehicleService.accelerate();
    }

    EXPECT_EQ(180, vehicleService.getState().getSpeed());
}

TEST_F(AccelerateTest, 連続加速で速度が累積する) {
    vehicleService.changeGear(VehicleState::Gear::D);

    vehicleService.accelerate();
    vehicleService.accelerate();
    VehicleState& result = vehicleService.accelerate();

    EXPECT_EQ(30, result.getSpeed());
}

// ===== 減速テスト =====

class DecelerateTest : public VehicleServiceTest {};

TEST_F(DecelerateTest, 減速すると速度が10減少する) {
    vehicleService.changeGear(VehicleState::Gear::D);
    vehicleService.accelerate();
    vehicleService.accelerate();
    vehicleService.accelerate(); // 30km/h

    VehicleState& result = vehicleService.decelerate();

    EXPECT_EQ(20, result.getSpeed());
}

TEST_F(DecelerateTest, 速度は0未満にならない) {
    VehicleState& result = vehicleService.decelerate();

    EXPECT_EQ(0, result.getSpeed());
}

// ===== ギア変更テスト =====

class GearChangeTest : public VehicleServiceTest {};

TEST_F(GearChangeTest, 速度0のときギア変更できる) {
    VehicleState& result = vehicleService.changeGear(VehicleState::Gear::D);

    EXPECT_EQ(VehicleState::Gear::D, result.getGear());
}

TEST_F(GearChangeTest, 走行中はギア変更できない_安全インターロック) {
    vehicleService.changeGear(VehicleState::Gear::D);
    vehicleService.accelerate(); // 10km/h

    VehicleState& result = vehicleService.changeGear(VehicleState::Gear::P);

    EXPECT_EQ(VehicleState::Gear::D, result.getGear());
}

TEST_F(GearChangeTest, Rレンジに変更できる) {
    VehicleState& result = vehicleService.changeGear(VehicleState::Gear::R);

    EXPECT_EQ(VehicleState::Gear::R, result.getGear());
}

TEST_F(GearChangeTest, Nレンジに変更できる) {
    VehicleState& result = vehicleService.changeGear(VehicleState::Gear::N);

    EXPECT_EQ(VehicleState::Gear::N, result.getGear());
}

// ===== リセットテスト =====

class ResetTest : public VehicleServiceTest {};

TEST_F(ResetTest, リセットで速度が0になる) {
    vehicleService.changeGear(VehicleState::Gear::D);
    vehicleService.accelerate();

    VehicleState& result = vehicleService.reset();

    EXPECT_EQ(0, result.getSpeed());
}

TEST_F(ResetTest, リセットでギアがPになる) {
    vehicleService.changeGear(VehicleState::Gear::D);

    VehicleState& result = vehicleService.reset();

    EXPECT_EQ(VehicleState::Gear::P, result.getGear());
}

TEST_F(ResetTest, リセットでエンジン警告が解除される) {
    vehicleService.setEngineError(true);

    VehicleState& result = vehicleService.reset();

    EXPECT_FALSE(result.isEngineWarning());
}

TEST_F(ResetTest, リセットでシートベルト警告が有効になる_未装着状態) {
    vehicleService.setSeatbelt(true);

    VehicleState& result = vehicleService.reset();

    EXPECT_TRUE(result.isSeatbeltWarning());
}

// ===== 状態取得テスト =====

class GetStateTest : public VehicleServiceTest {};

TEST_F(GetStateTest, getStateが現在の状態を返す) {
    VehicleState& state = vehicleService.getState();

    EXPECT_EQ(0, state.getSpeed());
    EXPECT_EQ(VehicleState::Gear::P, state.getGear());
}

TEST_F(GetStateTest, getStateは同じ参照を返す) {
    VehicleState& state1 = vehicleService.getState();
    VehicleState& state2 = vehicleService.getState();

    EXPECT_EQ(&state1, &state2);
}

TEST_F(GetStateTest, const版getStateが現在の状態を返す) {
    const VehicleService& constService = vehicleService;
    const VehicleState& state = constService.getState();

    EXPECT_EQ(0, state.getSpeed());
    EXPECT_EQ(VehicleState::Gear::P, state.getGear());
    EXPECT_FALSE(state.isEngineWarning());
    EXPECT_TRUE(state.isSeatbeltWarning());
    EXPECT_FALSE(state.isSpeedWarning());
}

TEST_F(GetStateTest, const版getStateは状態変更後も最新を返す) {
    vehicleService.changeGear(VehicleState::Gear::D);
    vehicleService.accelerate();

    const VehicleService& constService = vehicleService;
    const VehicleState& state = constService.getState();

    EXPECT_EQ(10, state.getSpeed());
    EXPECT_EQ(VehicleState::Gear::D, state.getGear());
}

// ===== 警告灯テスト =====

class WarningTest : public VehicleServiceTest {};

TEST_F(WarningTest, 速度120超過で速度警告が発生する) {
    vehicleService.changeGear(VehicleState::Gear::D);

    for (int i = 0; i < 13; i++) {
        vehicleService.accelerate(); // 130km/h
    }

    EXPECT_TRUE(vehicleService.getState().isSpeedWarning());
}

TEST_F(WarningTest, 速度120以下では速度警告は発生しない) {
    vehicleService.changeGear(VehicleState::Gear::D);

    for (int i = 0; i < 12; i++) {
        vehicleService.accelerate(); // 120km/h
    }

    EXPECT_FALSE(vehicleService.getState().isSpeedWarning());
}

TEST_F(WarningTest, エンジン異常時は60に強制減速する) {
    vehicleService.changeGear(VehicleState::Gear::D);
    for (int i = 0; i < 10; i++) {
        vehicleService.accelerate(); // 100km/h
    }

    VehicleState& result = vehicleService.setEngineError(true);

    EXPECT_EQ(60, result.getSpeed());
    EXPECT_TRUE(result.isEngineWarning());
}

TEST_F(WarningTest, エンジン異常時に60以下なら減速しない) {
    vehicleService.changeGear(VehicleState::Gear::D);
    for (int i = 0; i < 5; i++) {
        vehicleService.accelerate(); // 50km/h
    }

    VehicleState& result = vehicleService.setEngineError(true);

    EXPECT_EQ(50, result.getSpeed());
    EXPECT_TRUE(result.isEngineWarning());
}

TEST_F(WarningTest, エンジン異常を解除できる) {
    vehicleService.setEngineError(true);

    VehicleState& result = vehicleService.setEngineError(false);

    EXPECT_FALSE(result.isEngineWarning());
}

TEST_F(WarningTest, シートベルト装着で警告が解除される) {
    EXPECT_TRUE(vehicleService.getState().isSeatbeltWarning());

    VehicleState& result = vehicleService.setSeatbelt(true);

    EXPECT_FALSE(result.isSeatbeltWarning());
}

TEST_F(WarningTest, シートベルト解除で警告が発生する) {
    vehicleService.setSeatbelt(true);
    EXPECT_FALSE(vehicleService.getState().isSeatbeltWarning());

    VehicleState& result = vehicleService.setSeatbelt(false);

    EXPECT_TRUE(result.isSeatbeltWarning());
}

// ===== 減速度計算テスト =====

class DecelerationCalculationTest : public VehicleServiceTest {};

TEST_F(DecelerationCalculationTest, ブレーキ力1で減速度10を返す) {
    int deceleration = VehicleService::calculateDeceleration(50, 1.0);

    EXPECT_EQ(10, deceleration);
}

TEST_F(DecelerationCalculationTest, ブレーキ力0_5で減速度5を返す) {
    int deceleration = VehicleService::calculateDeceleration(50, 0.5);

    EXPECT_EQ(5, deceleration);
}

TEST_F(DecelerationCalculationTest, ブレーキ力0で減速度0を返す) {
    int deceleration = VehicleService::calculateDeceleration(50, 0.0);

    EXPECT_EQ(0, deceleration);
}

TEST_F(DecelerationCalculationTest, 速度100超では減速効率が下がる) {
    int normalDeceleration = VehicleService::calculateDeceleration(50, 1.0);
    int highSpeedDeceleration = VehicleService::calculateDeceleration(120, 1.0);

    EXPECT_LT(highSpeedDeceleration, normalDeceleration);
}

TEST_F(DecelerationCalculationTest, 速度100ちょうどでは減速効率が下がらない) {
    int deceleration = VehicleService::calculateDeceleration(100, 1.0);

    EXPECT_EQ(10, deceleration);
}

TEST_F(DecelerationCalculationTest, 速度101では減速効率が下がる) {
    int deceleration = VehicleService::calculateDeceleration(101, 1.0);

    EXPECT_EQ(8, deceleration);
}

TEST_F(DecelerationCalculationTest, 速度0でも計算できる) {
    int deceleration = VehicleService::calculateDeceleration(0, 1.0);

    EXPECT_EQ(10, deceleration);
}

TEST_F(DecelerationCalculationTest, ブレーキ力が範囲外の場合は例外をスローする) {
    EXPECT_THROW(VehicleService::calculateDeceleration(50, 1.5), std::invalid_argument);
    EXPECT_THROW(VehicleService::calculateDeceleration(50, -0.1), std::invalid_argument);
}

TEST_F(DecelerationCalculationTest, 負の速度は例外をスローする) {
    EXPECT_THROW(VehicleService::calculateDeceleration(-10, 0.5), std::invalid_argument);
}

// ===== 境界値テスト =====

class ServiceBoundaryTest : public VehicleServiceTest {};

TEST_F(ServiceBoundaryTest, 速度0から減速しても0のまま) {
    VehicleState& result = vehicleService.decelerate();

    EXPECT_EQ(0, result.getSpeed());
}

TEST_F(ServiceBoundaryTest, 速度180から加速しても180のまま) {
    vehicleService.changeGear(VehicleState::Gear::D);
    for (int i = 0; i < 18; i++) {
        vehicleService.accelerate();
    }

    VehicleState& result = vehicleService.accelerate();

    EXPECT_EQ(180, result.getSpeed());
}

TEST_F(ServiceBoundaryTest, エンジン異常時にちょうど60なら減速しない) {
    vehicleService.changeGear(VehicleState::Gear::D);
    for (int i = 0; i < 6; i++) {
        vehicleService.accelerate(); // 60km/h
    }

    VehicleState& result = vehicleService.setEngineError(true);

    EXPECT_EQ(60, result.getSpeed());
}

TEST_F(ServiceBoundaryTest, ブレーキ力ちょうど0は有効) {
    EXPECT_NO_THROW(VehicleService::calculateDeceleration(50, 0.0));
}

TEST_F(ServiceBoundaryTest, ブレーキ力ちょうど1は有効) {
    EXPECT_NO_THROW(VehicleService::calculateDeceleration(50, 1.0));
}

TEST_F(ServiceBoundaryTest, 速度ちょうど0は有効) {
    EXPECT_NO_THROW(VehicleService::calculateDeceleration(0, 0.5));
}

TEST_F(ServiceBoundaryTest, ブレーキ力1_01は例外) {
    EXPECT_THROW(VehicleService::calculateDeceleration(50, 1.01), std::invalid_argument);
}

TEST_F(ServiceBoundaryTest, ブレーキ力マイナス0_01は例外) {
    EXPECT_THROW(VehicleService::calculateDeceleration(50, -0.01), std::invalid_argument);
}

TEST_F(ServiceBoundaryTest, 速度マイナス1は例外) {
    EXPECT_THROW(VehicleService::calculateDeceleration(-1, 0.5), std::invalid_argument);
}

TEST_F(ServiceBoundaryTest, 高速時の中間ブレーキ力) {
    int deceleration = VehicleService::calculateDeceleration(150, 0.5);
    EXPECT_EQ(4, deceleration);
}
