//
//  ItemDetailScene.h
//  yamiHikariGame
//
//  Created by slightair on 2013/08/01.
//
//

#ifndef __yamiHikariGame__ItemDetailScene__
#define __yamiHikariGame__ItemDetailScene__

#include "cocos2d.h"
#include "GradientLayer.h"
#include "Item.h"

using namespace std;
using namespace cocos2d;
using namespace hiberlite;

class ItemDetailScene : public GradientLayer
{
protected:
    Item _item;

    CCSprite *_itemImage;
    CCLabelTTF *_itemNameLabel;
    CCLabelTTF *_scoreLabel;
    CCLabelTTF *_descriptionLabel;
    CCMenu *_pageSelectorPrev;
    CCMenu *_pageSelectorNext;

    void updatePageSelector();
    void showPrevItem();
    void showNextItem();

public:
    void setItem(Item item);

    virtual bool init();

    static CCScene* sceneWithItem(Item item);
    CREATE_FUNC(ItemDetailScene);
};

#endif /* defined(__yamiHikariGame__ItemDetailScene__) */
