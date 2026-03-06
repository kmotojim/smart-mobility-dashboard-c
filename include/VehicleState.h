#ifndef VEHICLE_STATE_H
#define VEHICLE_STATE_H

#include <string>

/**
 * 車両状態を表すモデルクラス
 */
class VehicleState {
public:
    /**
     * ギア状態
     */
    enum class Gear {
        P,  // パーキング
        R,  // リバース
        N,  // ニュートラル
        D   // ドライブ
    };

    VehicleState();

    // Getters
    int getSpeed() const;
    Gear getGear() const;
    bool isEngineWarning() const;
    bool isSeatbeltWarning() const;
    bool isSpeedWarning() const;

    // Setters
    void setSpeed(int speed);
    void setGear(Gear gear);
    void setEngineWarning(bool warning);
    void setSeatbeltWarning(bool warning);
    void setSpeedWarning(bool warning);

    // Utility
    static std::string gearToString(Gear gear);
    static Gear stringToGear(const std::string& str);

private:
    int speed_;                 // 現在速度 (0-180 km/h)
    Gear gear_;                 // ギア状態
    bool engineWarning_;        // エンジン警告
    bool seatbeltWarning_;      // シートベルト警告
    bool speedWarning_;         // 速度超過警告
};

#endif // VEHICLE_STATE_H
