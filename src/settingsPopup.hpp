#include <Geode/Geode.hpp>
#include "arduino.hpp"
#include "serialPortReader.hpp"

using namespace geode::prelude;

class SettingsPopup : public geode::Popup {
protected:
    bool init(std::string const& value) {
        if (!Popup::init(240.f, 180.f)) return false;
        this->setTitle("ArduGD Settings");
        // com label
        auto comLabel = CCLabelBMFont::create("COM Port", "bigFont.fnt");
        comLabel->setScale(0.5f);
        comLabel->setAnchorPoint({0.f, 1.f});
        // baud rate input
        auto baudRateLabel = CCLabelBMFont::create("Baud Rate", "bigFont.fnt");
        baudRateLabel->setScale(0.5f);
        baudRateLabel->setAnchorPoint({0.f, 1.f});
        // input
        auto comInput = TextInput::create(100.f, "");
        comInput->setID("com-number");
        auto port = Mod::get()->getSavedValue<int>("port");
        if (port && port != 0) 
            comInput->setString(std::to_string(port));
        comInput->setFilter("01234567890");
        // baud rate
        auto baudRateInput = TextInput::create(100.f, "9600");
        baudRateInput->setID("baud-number");
        auto baudRate = Mod::get()->getSavedValue<int>("baud-rate");
        if (baudRate && baudRate != 0)
            baudRateInput->setString(std::to_string(baudRate));
        baudRateInput->setFilter("01234567890");
        // status label
        auto comStatus = CCLabelBMFont::create("Port not available", "chatFont.fnt");
        comStatus->setID("status");
        comStatus->setColor({255, 0, 0});
        comStatus->setScale(0.75f);
        comStatus->setVisible(false);
        // button
        auto spr = ButtonSprite::create("Open");
        auto openButton = CCMenuItemSpriteExtra::create(spr, this, menu_selector(SettingsPopup::tryComm));
        openButton->setID("open-button");
        // close button
        auto closeSpr = ButtonSprite::create("Close");
        auto closeButton = CCMenuItemSpriteExtra::create(closeSpr, this, menu_selector(SettingsPopup::close));
        closeButton->setID("close-button");
        // serial button
        auto terminalSprite = CircleButtonSprite::createWithSpriteFrameName("serialconsole.png"_spr);
        auto serialButton = CCMenuItemSpriteExtra::create(terminalSprite, this, menu_selector(SettingsPopup::openSerialMonitor));
        serialButton->setID("serial-button");
        //  adding
        m_mainLayer->addChildAtPosition(comLabel, Anchor::Left, {16.f, 29.f});
        m_mainLayer->addChildAtPosition(baudRateLabel, Anchor::Left, {16.f, -10.f});
        m_mainLayer->addChildAtPosition(comInput, Anchor::Right, {-64.f, 20.f});
        m_mainLayer->addChildAtPosition(baudRateInput, Anchor::Right, {-64.f, -20.f});
        m_mainLayer->addChildAtPosition(comStatus, Anchor::Bottom, {0.f, 50.f});
        m_buttonMenu->addChildAtPosition(openButton, Anchor::Bottom, {50.f, 25.f});
        m_buttonMenu->addChildAtPosition(closeButton, Anchor::Bottom, {-50.f, 25.f});
        m_buttonMenu->addChildAtPosition(serialButton, Anchor::TopRight);
        return true;
    }
public:
    static SettingsPopup* create(std::string const& value) {
        auto ret = new SettingsPopup();
        if (ret->init(value)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    void tryComm(CCObject*) {
        auto label = typeinfo_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("status"));
        auto portNum = numFromString<int>(typeinfo_cast<TextInput*>(m_mainLayer->getChildByID("com-number"))->getString()).unwrapOr(0);
        auto baudRate = numFromString<int>(typeinfo_cast<TextInput*>(m_mainLayer->getChildByID("baud-number"))->getString()).unwrapOr(0);
        if (!portNum || !baudRate) {
            label->setString("Not all fields filled");
            label->setColor({ 255, 0, 0 });
            label->setVisible(true);
            return;
        }
        std::thread([this, label, portNum, baudRate](){
            Arduino::close();
            if (Arduino::open(portNum, baudRate)) {
                Loader::get()->queueInMainThread([this, label, portNum, baudRate]() {
                    label->setString("Port opened");
                    label->setColor({0, 255, 0});
                    Mod::get()->setSavedValue<int>("port", portNum);
                    Mod::get()->setSavedValue<int>("baud-rate", baudRate);
                });
            } else {
                Loader::get()->queueInMainThread([this, label](){
                    label->setString("Port not available");
                    label->setColor({255, 0, 0});
                    Mod::get()->setSavedValue<int>("port", 0);
                    Mod::get()->setSavedValue<int>("baud-rate", 0);
                });
            }
        }).detach();
        label->setVisible(true);
        label->updateLabel();
    }

    void openSerialMonitor(CCObject*)  {
        auto portNum = typeinfo_cast<TextInput*>(m_mainLayer->getChildByID("com-number"))->getString();
        SerialPortMonitor::create(portNum.c_str())->show();
    }

    void close(CCObject*) {
        std::thread([this](){
            auto status = Arduino::close();
            Loader::get()->queueInMainThread([this, status](){
                auto label = typeinfo_cast<CCLabelBMFont*>(m_mainLayer->getChildByID("status"));
                if (status) {
                    label->setColor({0, 255, 0});
                    label->setString("Port closed");
                } else {
                    label->setColor({255, 0, 0});
                    label->setString("Port already closed");
                }
                label->setVisible(true);
            });
        }).detach();
    }
};