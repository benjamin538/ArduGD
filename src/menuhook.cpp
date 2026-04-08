#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "settingsPopup.hpp"
#include "arduino.hpp"

using namespace geode::prelude;

class $modify(menuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        auto sprite = CCSprite::createWithSpriteFrameName("greencircle.png"_spr);
        sprite->setScale(0.5f);
        auto arduButton = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(menuLayer::showPopup));
        arduButton->setScale(0.5f);
        arduButton->m_baseScale = 0.5f;
        auto menu = this->getChildByID("side-menu");
        menu->addChild(arduButton);
        arduButton->setID("ardu-button"_spr);
        menu->updateLayout();
        return true;
    }

    void showPopup(CCObject*) {
        SettingsPopup::create("settings")->show();
    }

    void onQuit(CCObject* sender) {
        if (Arduino::isOpened()) {
            geode::createQuickPopup("Wait!", "You have <cr>opened port</c>. Close it?", "Yes", "No",
            [this, sender](bool btn1, auto){
                if (btn1) {
                    // thread again
                    std::thread([this, sender](){
                        Arduino::close();
                        Loader::get()->queueInMainThread([this, sender](){
                            MenuLayer::onQuit(sender);
                        });
                    }).detach();
                }
            });
        } else {
            MenuLayer::onQuit(sender);
        }
    }
};