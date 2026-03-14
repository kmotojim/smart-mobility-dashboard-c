#include <gtest/gtest.h>
#include "WebDriver.h"

/**
 * Smart Mobility Dashboard E2Eテスト
 *
 * Java版 dashboard.feature (Cucumber + Playwright) を
 * C++ (WebDriver REST API + Google Test) に移植したもの。
 *
 * 前提条件:
 *   1. ChromeDriver が起動していること (デフォルト: localhost:9515)
 *   2. アプリケーションが起動していること (デフォルト: localhost:8080)
 *
 * コマンドラインフラグ:
 *   --headed  - ブラウザを表示してテストを実行する (デモ向け)
 *
 * 環境変数:
 *   APP_URL          - アプリケーションURL (default: http://localhost:8080)
 *   CHROMEDRIVER_HOST - ChromeDriver ホスト (default: 127.0.0.1)
 *   CHROMEDRIVER_PORT - ChromeDriver ポート (default: 9515)
 *   HEADLESS          - ヘッドレスモード (default: true, --headed 指定時は無視)
 */

static bool g_headed = false;

static std::string getEnv(const char* name, const std::string& defaultVal) {
    const char* val = std::getenv(name);
    return val ? val : defaultVal;
}

class DashboardE2ETest : public ::testing::Test {
protected:
    static std::unique_ptr<WebDriver> driver;
    static std::string appUrl;

    static void SetUpTestSuite() {
        appUrl = getEnv("APP_URL", "http://localhost:8080");

        WebDriver::Options opts;
        opts.driverHost = getEnv("CHROMEDRIVER_HOST", "127.0.0.1");
        opts.driverPort = std::stoi(getEnv("CHROMEDRIVER_PORT", "9515"));
        opts.headless = g_headed ? false : (getEnv("HEADLESS", "true") == "true");

        driver = std::make_unique<WebDriver>(opts);
    }

    static void TearDownTestSuite() {
        driver.reset();
    }

    void SetUp() override {
        driver->navigateTo(appUrl);
        driver->waitFor([&] {
            try {
                driver->findElementById("speedometer");
                return true;
            } catch (...) { return false; }
        });
        resetState();
    }

    // --- ヘルパー: 状態リセット ---
    void resetState() {
        driver->findElementById("btn-reset").click();
        driver->sleep(500);
    }

    // --- ヘルパー: 速度取得 ---
    std::string getSpeed() {
        return driver->findElementById("speed-value").getText();
    }

    int getSpeedInt() {
        return std::stoi(getSpeed());
    }

    // --- ヘルパー: ギア変更 ---
    void changeGear(const std::string& gear) {
        driver->findElementByCss(".gear-btn[data-gear='" + gear + "']").click();
        driver->sleep(300);
    }

    // --- ヘルパー: アクティブギア取得 ---
    std::string getActiveGear() {
        auto elem = driver->findElementByCss(".gear-item.active");
        return elem.getAttribute("data-gear");
    }

    // --- ヘルパー: 加速 ---
    void accelerate(int times = 1) {
        for (int i = 0; i < times; i++) {
            driver->findElementById("btn-accelerate").click();
            driver->sleep(200);
        }
    }

    // --- ヘルパー: 減速 ---
    void decelerate(int times = 1) {
        for (int i = 0; i < times; i++) {
            driver->findElementById("btn-decelerate").click();
            driver->sleep(200);
        }
    }

    // --- ヘルパー: 速度を指定値まで加速 ---
    void setSpeedTo(int speed) {
        int times = speed / 10;
        accelerate(times);
    }

    // --- ヘルパー: 警告灯チェック ---
    bool isWarningActive(const std::string& warningId) {
        auto elem = driver->findElementById(warningId);
        return elem.hasClass("active");
    }

    // --- ヘルパー: シートベルト装着 ---
    void fastenSeatbelt() {
        auto checked = driver->executeScript(
            "return document.querySelector('#seatbelt-toggle').checked");
        if (!checked.get<bool>()) {
            driver->findElementByCss("label:has(#seatbelt-toggle)").click();
        }
        driver->sleep(300);
    }

    // --- ヘルパー: エンジン異常ON ---
    void triggerEngineError() {
        auto checked = driver->executeScript(
            "return document.querySelector('#engine-error-toggle').checked");
        if (!checked.get<bool>()) {
            driver->findElementByCss("label:has(#engine-error-toggle)").click();
        }
        driver->sleep(300);
    }
};

std::unique_ptr<WebDriver> DashboardE2ETest::driver;
std::string DashboardE2ETest::appUrl;

// =============================================
// スピードメーター関連
// =============================================

TEST_F(DashboardE2ETest, 加速ボタンで速度が増加する) {
    changeGear("D");
    EXPECT_EQ("0", getSpeed());

    accelerate(1);

    EXPECT_EQ("10", getSpeed());
}

TEST_F(DashboardE2ETest, 連続加速で速度が累積する) {
    changeGear("D");

    accelerate(3);

    EXPECT_EQ("30", getSpeed());
}

TEST_F(DashboardE2ETest, 減速ボタンで速度が減少する) {
    changeGear("D");
    setSpeedTo(50);

    decelerate(2);

    EXPECT_EQ("30", getSpeed());
}

TEST_F(DashboardE2ETest, 最大速度を超えない) {
    changeGear("D");

    accelerate(20);

    EXPECT_EQ("180", getSpeed());
}

TEST_F(DashboardE2ETest, 速度0から減速しても0のまま) {
    EXPECT_EQ("0", getSpeed());

    decelerate(1);

    EXPECT_EQ("0", getSpeed());
}

// =============================================
// ギアインジケーター関連
// =============================================

TEST_F(DashboardE2ETest, ギア変更でインジケーターが更新される) {
    EXPECT_EQ("0", getSpeed());

    changeGear("D");

    EXPECT_EQ("D", getActiveGear());
}

TEST_F(DashboardE2ETest, 走行中はギア変更できない_安全インターロック) {
    changeGear("D");
    setSpeedTo(10);

    // 強制クリック (disabled ボタンでも実行)
    driver->executeScript(
        "document.querySelector(\".gear-btn[data-gear='P']\").click()");
    driver->sleep(300);

    EXPECT_EQ("D", getActiveGear());
}

TEST_F(DashboardE2ETest, ギアPに変更できる) {
    changeGear("D");
    changeGear("P");
    EXPECT_EQ("P", getActiveGear());
}

TEST_F(DashboardE2ETest, ギアRに変更できる) {
    changeGear("R");
    EXPECT_EQ("R", getActiveGear());
}

TEST_F(DashboardE2ETest, ギアNに変更できる) {
    changeGear("N");
    EXPECT_EQ("N", getActiveGear());
}

TEST_F(DashboardE2ETest, ギアDに変更できる) {
    changeGear("D");
    EXPECT_EQ("D", getActiveGear());
}

// =============================================
// 警告灯関連
// =============================================

TEST_F(DashboardE2ETest, 速度超過で警告灯が点灯する) {
    changeGear("D");

    accelerate(13); // 130 km/h

    EXPECT_TRUE(isWarningActive("speed-warning"));
}

TEST_F(DashboardE2ETest, 速度120以下では警告灯は消灯) {
    changeGear("D");

    accelerate(12); // 120 km/h

    EXPECT_FALSE(isWarningActive("speed-warning"));
}

TEST_F(DashboardE2ETest, エンジン異常で警告灯が点灯する) {
    triggerEngineError();

    EXPECT_TRUE(isWarningActive("engine-warning"));
}

TEST_F(DashboardE2ETest, エンジン異常時は強制減速する) {
    changeGear("D");
    setSpeedTo(100);

    triggerEngineError();

    EXPECT_EQ("60", getSpeed());
}

TEST_F(DashboardE2ETest, シートベルト装着で警告灯が消灯する) {
    EXPECT_TRUE(isWarningActive("seatbelt-warning"));

    fastenSeatbelt();

    EXPECT_FALSE(isWarningActive("seatbelt-warning"));
}

// =============================================
// 制御パネル関連
// =============================================

TEST_F(DashboardE2ETest, リセットボタンで初期状態に戻る) {
    changeGear("D");
    setSpeedTo(50);
    triggerEngineError();

    driver->findElementById("btn-reset").click();
    driver->sleep(500);

    EXPECT_EQ("0", getSpeed());
    EXPECT_EQ("P", getActiveGear());
    EXPECT_FALSE(isWarningActive("engine-warning"));
}

TEST_F(DashboardE2ETest, Pレンジでは加速できない) {
    // 初期状態はPレンジ
    EXPECT_EQ("P", getActiveGear());

    accelerate(3);

    EXPECT_EQ("0", getSpeed());
}

// =============================================
// カスタム main: --headed フラグ対応
// =============================================

int main(int argc, char** argv) {
    std::vector<char*> filteredArgs;
    for (int i = 0; i < argc; i++) {
        if (std::string(argv[i]) == "--headed") {
            g_headed = true;
        } else {
            filteredArgs.push_back(argv[i]);
        }
    }
    int filteredArgc = static_cast<int>(filteredArgs.size());
    ::testing::InitGoogleTest(&filteredArgc, filteredArgs.data());
    return RUN_ALL_TESTS();
}
