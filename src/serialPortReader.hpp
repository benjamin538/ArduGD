#include <Geode/Geode.hpp>
#include "arduino.hpp"
#include <thread>

using namespace geode::prelude;

class SerialPortMonitor : public geode::Popup {
protected:
    std::atomic<bool> m_listening = true;

    bool init(std::string const& value) {
        auto winSize = CCDirector::get()->getWinSize();
        if (!Popup::init(winSize.width - 50.f, winSize.height - 50.f)) return false;
        this->setTitle("Serial Port Monitor");
        // clear
        auto clear = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_deleteBtn_001.png"), this, menu_selector(SerialPortMonitor::clearSerial));
        // send
        auto send = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("GJ_redoBtn_001.png"), this, menu_selector(SerialPortMonitor::write));
        // copy
        auto copy = CCMenuItemSpriteExtra::create(CCSprite::createWithSpriteFrameName("geode.loader/file.png"), this, menu_selector(SerialPortMonitor::copy));
        // input
        auto input = TextInput::create(m_mainLayer->getContentWidth() / 2, "Enter message here...", "bigFont.fnt");
        input->setID("input");
        // scroll
        auto scroll = ScrollLayer::create({winSize.width - 200, winSize.height - 150});
        scroll->setPosition({50, 25});
        scroll->setID("scroll");
        scroll->m_horizontalScrollbar->setVisible(true);
        // content
        auto content = CCLabelBMFont::create("", "chatFont.fnt");
        content->setID("content");
        content->setAnchorPoint({0.f, 1.f});
        content->setPositionY(scroll->m_contentLayer->getContentHeight());
        scroll->m_contentLayer->addChild(content);
        // adding
        m_mainLayer->addChild(scroll);
        m_mainLayer->addChildAtPosition(input, Anchor::TopLeft, {175, -50});
        m_buttonMenu->addChildAtPosition(clear, Anchor::TopRight, {-100, -50});
        m_buttonMenu->addChildAtPosition(send, Anchor::TopRight, {-150, -50});
        m_buttonMenu->addChildAtPosition(copy, Anchor::TopRight, {-50, -50});
        // logic
        if (!Arduino::isOpened()) return true;
        std::thread([this, content, scroll](){
            while (m_listening) {
                std::string data = Arduino::read();
                if (data.empty()) continue;
                Loader::get()->queueInMainThread([this, data, content, scroll](){
                    if (!m_listening) return;
                    std::string text = std::string(content->getString());
                    content->setString((text + data).c_str());
                    content->setPositionY(scroll->m_contentLayer->getContentHeight());
                    scroll->m_contentLayer->setContentHeight(content->getContentHeight());
                    scroll->m_contentLayer->setPositionY(0.f);
                });
            }
        }).detach();  
        return true;
    }

    void clearSerial(CCObject*) {
        auto scroll = typeinfo_cast<ScrollLayer*>(m_mainLayer->getChildByID("scroll"));
        auto label = typeinfo_cast<CCLabelBMFont*>(scroll->m_contentLayer->getChildByID("content"));
        label->setString("");
    }

    void onClose(CCObject* sender) {
        m_listening = false;
        geode::Popup::onClose(sender);
    }

    void write(CCObject*) {
        auto textType = typeinfo_cast<TextInput*>(m_mainLayer->getChildByID("input"));
        auto input = textType->getString();
        if (!input.empty()) std::thread([input](){Arduino::write(input.c_str());}).detach();
    }

    void copy(CCObject*) {
        auto scroll = typeinfo_cast<ScrollLayer*>(m_mainLayer->getChildByID("scroll"));
        auto label = typeinfo_cast<CCLabelBMFont*>(scroll->m_contentLayer->getChildByID("content"));
        clipboard::write(label->getString());
        Notification::create("Copied to clipboard", NotificationIcon::Success)->show();
    }
public:
    static SerialPortMonitor* create(std::string const& value) {
        auto ret = new SerialPortMonitor();
        if (ret->init(value)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }
};