#ifndef WEBDRIVER_H
#define WEBDRIVER_H

#include <string>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <httplib.h>
#include <nlohmann/json.hpp>

/**
 * W3C WebDriver プロトコルの軽量C++クライアント
 *
 * ChromeDriver / GeckoDriver が公開する REST API を httplib で呼び出し、
 * ブラウザの自動操作を行う。
 */

class WebElement;

struct WebDriverOptions {
    std::string driverHost = "127.0.0.1";
    int driverPort = 9515;
    bool headless = true;
    std::vector<std::string> args;
};

class WebDriver {
public:
    using Options = WebDriverOptions;

    explicit WebDriver(const Options& opts = Options{});
    ~WebDriver();

    WebDriver(const WebDriver&) = delete;
    WebDriver& operator=(const WebDriver&) = delete;

    void navigateTo(const std::string& url);
    std::string getCurrentUrl();
    std::string getTitle();

    WebElement findElement(const std::string& using_, const std::string& value);
    WebElement findElementByCss(const std::string& selector);
    WebElement findElementById(const std::string& id);
    std::vector<WebElement> findElementsByCss(const std::string& selector);

    nlohmann::json executeScript(const std::string& script,
                                 const nlohmann::json& args = nlohmann::json::array());

    void waitFor(std::function<bool()> condition,
                 int timeoutMs = 5000, int pollIntervalMs = 200);

    void sleep(int ms);

private:
    friend class WebElement;

    static constexpr const char* ELEMENT_KEY = "element-6066-11e4-a52e-4f735466cecf";

    httplib::Client client_;
    std::string sessionId_;

    nlohmann::json post(const std::string& path, const nlohmann::json& body = {});
    nlohmann::json get(const std::string& path);
    nlohmann::json del(const std::string& path);

    std::string sessionPath() const;
    void checkError(const nlohmann::json& response, const std::string& context);
};

class WebElement {
public:
    WebElement(WebDriver& driver, const std::string& elementId);

    void click();
    std::string getText();
    std::string getAttribute(const std::string& name);
    std::string getProperty(const std::string& name);
    std::string getCssClass();
    bool hasClass(const std::string& className);
    bool isEnabled();

private:
    WebDriver& driver_;
    std::string elementId_;

    std::string elementPath() const;
};

class WebDriverException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

#endif // WEBDRIVER_H
