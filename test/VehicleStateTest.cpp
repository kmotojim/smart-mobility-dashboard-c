#include <gtest/gtest.h>
#include <climits>
#include "VehicleState.h"

/**
 * VehicleStateの単体テスト
 */

class VehicleStateTest : public ::testing::Test {
protected:
    VehicleState vehicleState;
};

// ===== 初期状態テスト =====

class InitialStateTest : public VehicleStateTest {};

TEST_F(InitialStateTest, 速度の初期値は0) {
    EXPECT_EQ(0, vehicleState.getSpeed());
}

TEST_F(InitialStateTest, ギアの初期値はP) {
    EXPECT_EQ(VehicleState::Gear::P, vehicleState.getGear());
}

TEST_F(InitialStateTest, エンジン警告の初期値はfalse) {
    EXPECT_FALSE(vehicleState.isEngineWarning());
}

TEST_F(InitialStateTest, シートベルト警告の初期値はtrue_未装着) {
    EXPECT_TRUE(vehicleState.isSeatbeltWarning());
}

TEST_F(InitialStateTest, 速度警告の初期値はfalse) {
    EXPECT_FALSE(vehicleState.isSpeedWarning());
}

// ===== 速度setter/getterテスト =====

class SpeedTest : public VehicleStateTest {};

TEST_F(SpeedTest, 通常の速度設定) {
    vehicleState.setSpeed(50);
    EXPECT_EQ(50, vehicleState.getSpeed());
}

TEST_F(SpeedTest, 速度0を設定) {
    vehicleState.setSpeed(0);
    EXPECT_EQ(0, vehicleState.getSpeed());
}

TEST_F(SpeedTest, 最大速度180を設定) {
    vehicleState.setSpeed(180);
    EXPECT_EQ(180, vehicleState.getSpeed());
}

TEST_F(SpeedTest, 負の速度は0に制限される) {
    vehicleState.setSpeed(-10);
    EXPECT_EQ(0, vehicleState.getSpeed());
}

TEST_F(SpeedTest, 速度180超は180に制限される) {
    vehicleState.setSpeed(200);
    EXPECT_EQ(180, vehicleState.getSpeed());
}

TEST_F(SpeedTest, 速度120では速度警告なし) {
    vehicleState.setSpeed(120);
    EXPECT_FALSE(vehicleState.isSpeedWarning());
}

TEST_F(SpeedTest, 速度121で速度警告発生) {
    vehicleState.setSpeed(121);
    EXPECT_TRUE(vehicleState.isSpeedWarning());
}

TEST_F(SpeedTest, 速度警告後に120以下で警告解除) {
    vehicleState.setSpeed(130);
    EXPECT_TRUE(vehicleState.isSpeedWarning());

    vehicleState.setSpeed(100);
    EXPECT_FALSE(vehicleState.isSpeedWarning());
}

// ===== ギアsetter/getterテスト =====

class GearTest : public VehicleStateTest {};

TEST_F(GearTest, Dレンジに変更) {
    vehicleState.setGear(VehicleState::Gear::D);
    EXPECT_EQ(VehicleState::Gear::D, vehicleState.getGear());
}

TEST_F(GearTest, Rレンジに変更) {
    vehicleState.setGear(VehicleState::Gear::R);
    EXPECT_EQ(VehicleState::Gear::R, vehicleState.getGear());
}

TEST_F(GearTest, Nレンジに変更) {
    vehicleState.setGear(VehicleState::Gear::N);
    EXPECT_EQ(VehicleState::Gear::N, vehicleState.getGear());
}

TEST_F(GearTest, Pレンジに変更) {
    vehicleState.setGear(VehicleState::Gear::D);
    vehicleState.setGear(VehicleState::Gear::P);
    EXPECT_EQ(VehicleState::Gear::P, vehicleState.getGear());
}

// ===== エンジン警告setter/getterテスト =====

class EngineWarningTest : public VehicleStateTest {};

TEST_F(EngineWarningTest, エンジン警告をtrueに設定) {
    vehicleState.setEngineWarning(true);
    EXPECT_TRUE(vehicleState.isEngineWarning());
}

TEST_F(EngineWarningTest, エンジン警告をfalseに設定) {
    vehicleState.setEngineWarning(true);
    vehicleState.setEngineWarning(false);
    EXPECT_FALSE(vehicleState.isEngineWarning());
}

// ===== シートベルト警告setter/getterテスト =====

class SeatbeltWarningTest : public VehicleStateTest {};

TEST_F(SeatbeltWarningTest, シートベルト警告をtrueに設定) {
    vehicleState.setSeatbeltWarning(false);
    vehicleState.setSeatbeltWarning(true);
    EXPECT_TRUE(vehicleState.isSeatbeltWarning());
}

TEST_F(SeatbeltWarningTest, シートベルト警告をfalseに設定_装着状態) {
    vehicleState.setSeatbeltWarning(false);
    EXPECT_FALSE(vehicleState.isSeatbeltWarning());
}

// ===== 速度警告setter/getterテスト =====

class SpeedWarningTest : public VehicleStateTest {};

TEST_F(SpeedWarningTest, 速度警告を直接trueに設定) {
    vehicleState.setSpeedWarning(true);
    EXPECT_TRUE(vehicleState.isSpeedWarning());
}

TEST_F(SpeedWarningTest, 速度警告を直接falseに設定) {
    vehicleState.setSpeedWarning(true);
    vehicleState.setSpeedWarning(false);
    EXPECT_FALSE(vehicleState.isSpeedWarning());
}

// ===== Gear列挙型テスト =====

TEST(GearEnumTest, gearToStringで全ギア値を文字列に変換) {
    EXPECT_EQ("P", VehicleState::gearToString(VehicleState::Gear::P));
    EXPECT_EQ("R", VehicleState::gearToString(VehicleState::Gear::R));
    EXPECT_EQ("N", VehicleState::gearToString(VehicleState::Gear::N));
    EXPECT_EQ("D", VehicleState::gearToString(VehicleState::Gear::D));
}

TEST(GearEnumTest, gearToStringで範囲外の値はUNKNOWNを返す) {
    auto invalidGear = static_cast<VehicleState::Gear>(99);
    EXPECT_EQ("UNKNOWN", VehicleState::gearToString(invalidGear));
}

TEST(GearEnumTest, stringToGearで文字列からギアを取得) {
    EXPECT_EQ(VehicleState::Gear::P, VehicleState::stringToGear("P"));
    EXPECT_EQ(VehicleState::Gear::R, VehicleState::stringToGear("R"));
    EXPECT_EQ(VehicleState::Gear::N, VehicleState::stringToGear("N"));
    EXPECT_EQ(VehicleState::Gear::D, VehicleState::stringToGear("D"));
}

TEST(GearEnumTest, 無効なギア文字列で例外発生) {
    EXPECT_THROW(VehicleState::stringToGear("X"), std::invalid_argument);
}

TEST(GearEnumTest, 空文字列で例外発生) {
    EXPECT_THROW(VehicleState::stringToGear(""), std::invalid_argument);
}

TEST(GearEnumTest, 小文字ギア文字列で例外発生) {
    EXPECT_THROW(VehicleState::stringToGear("d"), std::invalid_argument);
}

// ===== 境界値テスト =====

class BoundaryTest : public VehicleStateTest {};

TEST_F(BoundaryTest, 速度マイナス1は0に制限) {
    vehicleState.setSpeed(-1);
    EXPECT_EQ(0, vehicleState.getSpeed());
}

TEST_F(BoundaryTest, 速度181は180に制限) {
    vehicleState.setSpeed(181);
    EXPECT_EQ(180, vehicleState.getSpeed());
}

TEST_F(BoundaryTest, INT_MAXは180に制限) {
    vehicleState.setSpeed(INT_MAX);
    EXPECT_EQ(180, vehicleState.getSpeed());
}

TEST_F(BoundaryTest, INT_MINは0に制限) {
    vehicleState.setSpeed(INT_MIN);
    EXPECT_EQ(0, vehicleState.getSpeed());
}
