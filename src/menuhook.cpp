#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include "settingsPopup.hpp"
#include "arduino.hpp"

using namespace geode::prelude;

class $modify(menuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        auto sprite = CircleButtonSprite::createWithSpriteFrameName("arduinobutton.png"_spr);
        auto arduButton = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(menuLayer::showPopup));
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