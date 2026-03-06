#ifndef VEHICLE_SERVICE_H
#define VEHICLE_SERVICE_H

#include "VehicleState.h"
#include <memory>

/**
 * 車両状態を管理するビジネスロジッククラス
 */
class VehicleService {
public:
    VehicleService();

    /**
     * 現在の車両状態を取得
     */
    VehicleState& getState();
    const VehicleState& getState() const;

    /**
     * 加速処理
     * @return 加速後の状態
     */
    VehicleState& accelerate();

    /**
     * 減速処理
     * @return 減速後の状態
     */
    VehicleState& decelerate();

    /**
     * ギア変更
     * @param gear 新しいギア状態
     * @return 変更後の状態
     */
    VehicleState& changeGear(VehicleState::Gear gear);

    /**
     * シートベルト装着/解除
     * @param fastened シートベルト装着状態
     * @return 変更後の状態
     */
    VehicleState& setSeatbelt(bool fastened);

    /**
     * エンジン異常を発生/解除（テスト用）
     * @param hasError エラー状態
     * @return 変更後の状態
     */
    VehicleState& setEngineError(bool hasError);

    /**
     * 状態をリセット
     * @return リセット後の状態
     */
    VehicleState& reset();

    /**
     * 速度から減速度を計算（ブレーキロジック）
     * @param currentSpeed 現在速度
     * @param brakeForce ブレーキ力 (0.0-1.0)
     * @return 減速度 (km/h/s)
     */
    static int calculateDeceleration(int currentSpeed, double brakeForce);

private:
    static constexpr int SPEED_INCREMENT = 10;       // 加速時の速度増分
    static constexpr int SPEED_DECREMENT = 10;       // 減速時の速度減分
    static constexpr int MAX_SPEED = 180;            // 最大速度
    static constexpr int MIN_SPEED = 0;              // 最小速度
    static constexpr int SPEED_WARNING_THRESHOLD = 120; // 速度警告閾値

    VehicleState state_;
};

#endif // VEHICLE_SERVICE_H
