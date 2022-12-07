/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 
 edit by liwen.su at 2022/12/07
 ****************************************************************************/

#include "HelloWorldScene.h"

#include <time.h>
#include "ui/CocosGUI.h"

USING_NS_CC;

namespace {
    int ASTC_TAG = 1;
    unsigned long long int getNowMillisecond()
    {
        struct timeval t;
        gettimeofday(&t, NULL);
        return t.tv_sec * 1000ull + t.tv_usec / 1000ull;
    }
};

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

bool HelloWorld::init()
{
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    Vec2 pos1 = Vec2(origin + visibleSize / 2);
    createTappableLabel("4x4", pos1, [this, origin, visibleSize](){
        auto start = getNowMillisecond();
        createAstc("lenna1024_4x4.astc", Vec2(origin + visibleSize / 2));
        auto end = getNowMillisecond();
        CCLOG("elapsed: %lld", end - start);
    });
    Vec2 pos2 = pos1 - Vec2(0, 24);
    createTappableLabel("8x8", pos2, [this, origin, visibleSize](){
        auto start = getNowMillisecond();
        createAstc("lenna1024_4x4.astc", Vec2(origin + visibleSize / 2));
        auto end = getNowMillisecond();
        CCLOG("elapsed: %lld", end - start);
    });
    Vec2 pos3 = pos2 - Vec2(0, 24);
    createTappableLabel("clear", pos3, [this](){
        auto astc = getChildByTag(ASTC_TAG);
        if (astc) {
            astc->runAction(RemoveSelf::create());
        }
    });

    return true;
}

void HelloWorld::createTappableLabel(const std::string& text, const Vec2& position, std::function<void()> callback)
{
    auto button = ui::Button::create("Button_Normal.png", "Button_Press.png", "Button_Disable.png");
    button->setPosition(position);
    button->setScale(2);
    button->setTitleText(text);
    button->setTitleColor(Color3B::BLACK);
    button->setTitleFontSize(5);
    button->addTouchEventListener([callback](Ref* sender, ui::Widget::TouchEventType type){
        if (callback && type == ui::Widget::TouchEventType::ENDED) {
            callback();
        }
    });
    this->addChild(button, 1);
}

void HelloWorld::createAstc(const std::string& filepath, const Vec2& position)
{
    auto astc = getChildByTag(ASTC_TAG);
    if (astc) {
        astc->runAction(RemoveSelf::create());
    }

    Director::getInstance()->getTextureCache()->removeAllTextures();

    auto sprite = Sprite::create(filepath);
    if (sprite)
    {
        sprite->setPosition(position);
        addChild(sprite, 0, ASTC_TAG);
    }
}
