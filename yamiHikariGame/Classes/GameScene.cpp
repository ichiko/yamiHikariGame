//
//  GameScene.cpp
//  yamiHikariGame
//
//  Created by slightair on 2013/07/16.
//
//

#include "GameScene.h"
#include "SimpleAudioEngine.h"
#include "Constants.h"
#include "GameEngine.h"
#include "DropItem.h"

#define kDropItemInterval 0.2
#define kEmergedAreaHorizontalMarginRate 0.1
#define kCollisionAreaPadding 8

#define kObtainedEffectDuration 0.5

#define kObtainedScoreMoveY 48
#define kObtainedScoreAdjustX 0
#define kObtainedScoreAdjustY 12

#define kObtainedStaminaMoveY 32
#define kObtainedStaminaAdjustX 32
#define kObtainedStaminaAdjustY -12

#define kStaminaLabelColorIncrease (ccc3(0x66, 0xff, 0x66))
#define kStaminaLabelColorDecrease (ccc3(0xff, 0x66, 0x66))

CCScene* GameScene::scene()
{
    CCScene *scene = CCScene::create();

    GameScene *layer = GameScene::create();
    scene->addChild(layer);

    return scene;
}

void GameScene::update(float delta)
{
    _monster->followBrave(_brave->getPosition(), delta);

    CCSize braveSize = _brave->getContentSize();
    _braveRect = CCRectMake(_brave->getPosition().x - braveSize.width / 2 + kCollisionAreaPadding,
                            _brave->getPosition().y - braveSize.height / 2 + kCollisionAreaPadding,
                            braveSize.width - kCollisionAreaPadding * 2,
                            braveSize.height - kCollisionAreaPadding * 2);
    collisionCheck();
}

void GameScene::onEnter()
{
    CCLayer::onEnter();

    CCSize windowSize = CCDirector::sharedDirector()->getWinSize();

    _finished = false;

    _worldNode = CCNode::create();
    _groundNode = Ground::create();
    _itemsNode = CCSpriteBatchNode::create(SpriteSheetImageFileName);
    _scoreBoardNode = ScoreBoard::create();

    _worldNode->addChild(_groundNode);
    _worldNode->addChild(_itemsNode);

    _monster = Monster::createWithSpriteFrameName("monster.png");
    _monster->setPosition(ccp(windowSize.width / 2, windowSize.height - _monster->getContentSize().height));
    _worldNode->addChild(_monster);

    _brave = Brave::createWithSpriteFrameName("brave.png");
    _brave->setPosition(ccp(windowSize.width / 2, windowSize.height - _monster->getContentSize().height * 2 - _brave->getContentSize().height / 2));
    _worldNode->addChild(_brave);

    _darknessNode = Darkness::create(_brave->getPosition());

    this->addChild(_worldNode);
    this->addChild(_darknessNode);
    this->addChild(_scoreBoardNode);

    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, true);
    
    this->scheduleUpdate();
}

void GameScene::onEnterTransitionDidFinish()
{
    CCLayer::onEnterTransitionDidFinish();

    _monster->startAnimation();
    _brave->startAnimation();

    this->schedule(schedule_selector(GameScene::dropItem), kDropItemInterval);
}

void GameScene::onExit()
{
    CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);

    CCLayer::onExit();
}

bool GameScene::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent)
{
    if (_finished) {
        return false;
    }

    _touchedLocation = pTouch->getLocationInView();

    return true;
}

void GameScene::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent)
{
    if (_finished) {
        return;
    }

    CCPoint location = pTouch->getLocationInView();
    float distance = location.x - _touchedLocation.x;

    _brave->moveX(distance);
    _darknessNode->setLightPosition(_brave->getPosition());

    _touchedLocation = location;
}

void GameScene::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent)
{

}

void GameScene::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent)
{

}

void GameScene::dropItem()
{
    CCSize windowSize = CCDirector::sharedDirector()->getWinSize();
    float horizontalMargin = windowSize.width * kEmergedAreaHorizontalMarginRate;
    int emergedAreaWidth = windowSize.width - horizontalMargin * 2;

    GameEngine *engine = GameEngine::sharedEngine();

    vector<Item> *items = engine->getItems();
    vector<Item>::reverse_iterator iterator = items->rbegin();

    int itemCount = 0;
    while (iterator != items->rend()) {
        Item item = *iterator;
        float needle = (float)arc4random() / UINT32_MAX;

        if (engine->getScore() > item->score_threshold && item->drop_rate > needle) {
            float positionX = arc4random() % emergedAreaWidth + horizontalMargin;

            DropItem *dropItem = DropItem::create(item);
            dropItem->setPosition(ccp(positionX, -dropItem->getContentSize().height / 2));
            _itemsNode->addChild(dropItem);
            dropItem->drop();

            itemCount++;
            if (itemCount >= SameTimeDropItemKindMax) {
                break;
            }
        }

        iterator++;
    }
}

void GameScene::collisionCheck()
{
    GameEngine *engine = GameEngine::sharedEngine();
    DropItem *dropItem = NULL;
    CCObject *child = NULL;
    CCARRAY_FOREACH(_itemsNode->getChildren(), child) {
        dropItem = (DropItem *)child;
        if (_braveRect.intersectsRect(dropItem->boundingBox())) {
            CCParticleSystem *starParticle = CCParticleSystemQuad::create("stars.plist");
            starParticle->setPosition(dropItem->getPosition());
            starParticle->setAutoRemoveOnFinish(true);
            this->addChild(starParticle);

            int obtainedStamina = dropItem->getStamina();

            showObtainedScore(dropItem->getScore());
            if (obtainedStamina != 0) {
                showObtainedStamina(obtainedStamina);
            }

            if (obtainedStamina < 0) {
                _brave->damageEffect();
                CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(SEBadItemGet);
            }
            else {
                if (obtainedStamina != 0) {
                    _brave->recoverEffect();
                }
                CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(SEItemGet);
            }

            engine->foundItem(dropItem->getItemID());

            dropItem->stopAllActions();
            dropItem->removeFromParentAndCleanup(true);
        }
    }
}

void GameScene::showObtainedScore(int score)
{
    CCLabelTTF *scoreLabel = CCLabelTTF::create(CCString::createWithFormat("%d", score)->getCString(), DefaultFontName, FontSizeSmall);
    scoreLabel->setPosition(ccpAdd(_brave->getPosition(), ccp(kObtainedScoreAdjustX, kObtainedScoreAdjustY)));
    _scoreBoardNode->addChild(scoreLabel);

    CCSequence *action = CCSequence::create(CCSpawn::create(CCFadeOut::create(kObtainedEffectDuration),
                                                            CCMoveBy::create(kObtainedEffectDuration, ccp(0, kObtainedScoreMoveY)),
                                                            NULL),
                                            CCCallFuncND::create(scoreLabel, callfuncND_selector(CCNode::removeFromParentAndCleanup), (void *)true),
                                            NULL);
    scoreLabel->runAction(action);
}

void GameScene::showObtainedStamina(int stamina)
{
    CCLabelTTF *staminaLabel = CCLabelTTF::create(CCString::createWithFormat("%+d", stamina)->getCString(), DefaultFontName, FontSizeSmall);
    staminaLabel->setPosition(ccpAdd(_brave->getPosition(), ccp(kObtainedStaminaAdjustX, 0)));
    staminaLabel->setColor(stamina > 0 ? kStaminaLabelColorIncrease : kStaminaLabelColorDecrease);
    _scoreBoardNode->addChild(staminaLabel);

    float moveY = stamina > 0 ? kObtainedStaminaMoveY : -kObtainedStaminaMoveY;
    CCSequence *action = CCSequence::create(CCSpawn::create(CCFadeOut::create(kObtainedEffectDuration),
                                                            CCMoveBy::create(kObtainedEffectDuration, ccp(0, moveY)),
                                                            NULL),
                                            CCCallFuncND::create(staminaLabel, callfuncND_selector(CCNode::removeFromParentAndCleanup), (void *)true),
                                            NULL);
    staminaLabel->runAction(action);
}

void GameScene::finishAnimations()
{
    _groundNode->finishAnimations();
    _brave->finishAnimation();
    _monster->finishAnimation();

    this->unscheduleAllSelectors();

    CCObject *child = NULL;
    CCARRAY_FOREACH(_itemsNode->getChildren(), child) {
        ((DropItem *)child)->stopAllActions();
    }

    _finished = true;
}