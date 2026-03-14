#include "WebDriver.h"
#include <sstream>
#include <algorithm>

using json = nlohmann::json;

// ===== WebDriver =====

WebDriver::WebDriver(const Options& opts)
    : client_(opts.driverHost, opts.driverPort) {

    client_.set_connection_timeout(10);
    client_.set_read_timeout(30);

    json caps;
    json chromeOpts;
    std::vector<std::string> args = opts.args;
    if (opts.headless) {
        args.push_back("--headless=new");
    }
    args.push_back("--no-sandbox");
    args.push_back("--disable-dev-shm-usage");
    chromeOpts["args"] = args;

    caps["capabilities"]["alwaysMatch"]["browserName"] = "chrome";
    caps["capabilities"]["alwaysMatch"]["goog:chromeOptions"] = chromeOpts;

    auto resp = post("/session", caps);
    sessionId_ = resp["value"]["sessionId"].get<std::string>();
}

WebDriver::~WebDriver() {
    try {
        del(sessionPath());
    } catch (...) {}
}

void WebDriver::navigateTo(const std::string& url) {
    post(sessionPath() + "/url", {{"url", url}});
}

std::string WebDriver::getCurrentUrl() {
    auto resp = get(sessionPath() + "/url");
    return resp["value"].get<std::string>();
}

std::string WebDriver::getTitle() {
    auto resp = get(sessionPath() + "/title");
    return resp["value"].get<std::string>();
}

WebElement WebDriver::findElement(const std::string& using_, const std::string& value) {
    auto resp = post(sessionPath() + "/element", {{"using", using_}, {"value", value}});
    checkError(resp, "findElement(" + using_ + ", " + value + ")");
    std::string elemId = resp["value"][ELEMENT_KEY].get<std::string>();
    return WebElement(*this, elemId);
}

WebElement WebDriver::findElementByCss(const std::string& selector) {
    return findElement("css selector", selector);
}

WebElement WebDriver::findElementById(const std::string& id) {
    return findElement("css selector", "#" + id);
}

std::vector<WebElement> WebDriver::findElementsByCss(const std::string& selector) {
    auto resp = post(sessionPath() + "/elements", {{"using", "css selector"}, {"value", selector}});
    std::vector<WebElement> elements;
    for (auto& elem : resp["value"]) {
        elements.emplace_back(*this, elem[ELEMENT_KEY].get<std::string>());
    }
    return elements;
}

json WebDriver::executeScript(const std::string& script, const json& args) {
    auto resp = post(sessionPath() + "/execute/sync", {{"script", script}, {"args", args}});
    return resp["value"];
}

void WebDriver::waitFor(std::function<bool()> condition, int timeoutMs, int pollIntervalMs) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (std::chrono::steady_clock::now() < deadline) {
        try {
            if (condition()) return;
        } catch (...) {}
        std::this_thread::sleep_for(std::chrono::milliseconds(pollIntervalMs));
    }
    throw WebDriverException("waitFor timed out after " + std::to_string(timeoutMs) + "ms");
}

void WebDriver::sleep(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

json WebDriver::post(const std::string& path, const json& body) {
    std::string bodyStr = body.empty() ? "{}" : body.dump();
    auto res = client_.Post(path, bodyStr, "application/json");
    if (!res) {
        throw WebDriverException("POST " + path + " failed: connection error");
    }
    return json::parse(res->body);
}

json WebDriver::get(const std::string& path) {
    auto res = client_.Get(path);
    if (!res) {
        throw WebDriverException("GET " + path + " failed: connection error");
    }
    return json::parse(res->body);
}

json WebDriver::del(const std::string& path) {
    auto res = client_.Delete(path);
    if (!res) {
        throw WebDriverException("DELETE " + path + " failed: connection error");
    }
    return json::parse(res->body);
}

std::string WebDriver::sessionPath() const {
    return "/session/" + sessionId_;
}

void WebDriver::checkError(const json& response, const std::string& context) {
    if (response.contains("value") && response["value"].is_object()
        && response["value"].contains("error")) {
        std::string error = response["value"]["error"].get<std::string>();
        std::string message = response["value"].value("message", "");
        throw WebDriverException(context + ": " + error + " - " + message);
    }
}

// ===== WebElement =====

WebElement::WebElement(WebDriver& driver, const std::string& elementId)
    : driver_(driver), elementId_(elementId) {}

void WebElement::click() {
    driver_.post(elementPath() + "/click", json::object());
}

std::string WebElement::getText() {
    auto resp = driver_.get(elementPath() + "/text");
    return resp["value"].get<std::string>();
}

std::string WebElement::getAttribute(const std::string& name) {
    auto resp = driver_.get(elementPath() + "/attribute/" + name);
    if (resp["value"].is_null()) return "";
    return resp["value"].get<std::string>();
}

std::string WebElement::getProperty(const std::string& name) {
    auto resp = driver_.get(elementPath() + "/property/" + name);
    if (resp["value"].is_null()) return "";
    if (resp["value"].is_boolean()) return resp["value"].get<bool>() ? "true" : "false";
    return resp["value"].get<std::string>();
}

std::string WebElement::getCssClass() {
    return getAttribute("class");
}

bool WebElement::hasClass(const std::string& className) {
    std::string classes = getCssClass();
    std::istringstream iss(classes);
    std::string token;
    while (iss >> token) {
        if (token == className) return true;
    }
    return false;
}

bool WebElement::isEnabled() {
    auto resp = driver_.get(elementPath() + "/enabled");
    return resp["value"].get<bool>();
}

std::string WebElement::elementPath() const {
    return driver_.sessionPath() + "/element/" + elementId_;
}
