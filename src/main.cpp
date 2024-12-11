#include <Geode/Geode.hpp>
#include <Geode/Modify.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <cocos2d.h>

using namespace geode::prelude;

static int maxFrames = 4;

auto chosenGameSprite = Mod::get()->getSettingValue<std::string>("selected-sprite");

$execute {
    listenForSettingChanges("selected-sprite", [](std::string value) {
        chosenGameSprite = value;
    });
}

class $modify(PlayerObject) {
    struct Fields {
        float m_animationTimer = 0.f;
        int m_maxFrames = 4; // Default max frames
        int m_currentFrame = 1; // Start from frame 1
        CCSprite* m_customSprite = nullptr;
        bool m_flippedX = false; // Track X flip state
        bool m_flippedY = false; // Track Y flip state
        cocos2d::CCNode* m_mainLayer = nullptr; // Main layer node
    };

    bool init(int p0, int p1, GJBaseGameLayer* p2, cocos2d::CCLayer* p3, bool p4) {
        if (!PlayerObject::init(p0, p1, p2, p3, p4)) return false;

        // Adjust the maximum frames based on the selected game sprite
        if (chosenGameSprite == "mania_" || chosenGameSprite == "advance_") {
            m_fields->m_maxFrames = 8; // Set max frames to 8 for mania
        } else {
            m_fields->m_maxFrames = 4; // Default to 4 frames otherwise
        }

        // Hide robot sprite, fire and particles
        m_robotBatchNode->setVisible(false);

        // Create the custom sprite
        std::string frameName = fmt::format("{}sonicRun_01.png"_spr, chosenGameSprite);
        m_fields->m_customSprite = CCSprite::createWithSpriteFrameName(frameName.c_str()); // Start with frame 1
        if (m_fields->m_customSprite) {
            m_fields->m_customSprite->setAnchorPoint({0.5f, 0.5f});
            m_fields->m_customSprite->setPosition(this->getPosition());
            m_fields->m_customSprite->setVisible(false);
            this->addChild(m_fields->m_customSprite, 10);
        }

        return true;
    }

    void update(float dt) {
        PlayerObject::update(dt);

        // Sync rotation w mainLayer
        if (m_fields->m_customSprite && m_mainLayer) {
            m_fields->m_customSprite->setRotation(m_mainLayer->getRotation());

            // y flip with this instead maybe...?
            bool mainLayerFlippedY = m_mainLayer->getScaleY() < 0;
            m_fields->m_customSprite->setFlipY(mainLayerFlippedY);
        }

        // Check if the current game mode is robot
        if (!m_isRobot || !m_fields->m_customSprite) {
            if (m_fields->m_customSprite) {
                m_fields->m_customSprite->setVisible(false);
            }
            return;
        }

        m_robotFire->setVisible(false);
        m_robotBurstParticles->setVisible(false);

        // Show and update custom sprite
        m_fields->m_customSprite->setVisible(true);

        // Check if the player is in platformer mode and has no X velocity
        if (m_isPlatformer && m_platformerXVelocity == 0 && m_isOnGround) {
            // Set the sprite to idle and stop the animation
            std::string frameName = fmt::format("{}sonicIdle_01.png"_spr, chosenGameSprite);
            m_fields->m_customSprite->setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(frameName.c_str()));
            m_fields->m_animationTimer = 0; // Reset animation timer to stop cycling frames
        } else {
            // Determine current animation (run or jump)
            std::string frameName;
            float frameDuration;

            if (this->m_isOnGround || this->m_hasGroundParticles) {
                frameName = fmt::format("{}sonicRun_0{}.png"_spr, chosenGameSprite, m_fields->m_currentFrame);
                frameDuration = 1.9f; // Use run animation frame duration
            } else {
                frameName = fmt::format("{}sonicJump_0{}.png"_spr, chosenGameSprite, m_fields->m_currentFrame);
                frameDuration = 0.6f; // Use jump animation frame duration
            }

            // Update animation frame
            m_fields->m_animationTimer += dt;

            if (m_fields->m_animationTimer >= frameDuration) {
                m_fields->m_animationTimer -= frameDuration;

                // Update current frame (cycle through 1-4)
                m_fields->m_currentFrame = (m_fields->m_currentFrame % m_fields->m_maxFrames) + 1;

                // Get the new sprite frame from the cache
                auto frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(frameName.c_str());

                // Only update the frame if it's a valid one
                if (frame) {
                    m_fields->m_customSprite->setDisplayFrame(frame);
                }
            }
        }

        // if i need to add anything else
        // do it here

    }

    virtual void setFlipX(bool p0) override {
        if (p0 != m_fields->m_flippedX) {
            m_fields->m_flippedX = p0;
            m_fields->m_customSprite->setFlipX(p0); 
        }

        PlayerObject::setFlipX(p0);
    }

    void doReversePlayer(bool p0) {
        if (p0 != m_fields->m_flippedX) {
            m_fields->m_flippedX = p0;
            m_fields->m_customSprite->setFlipX(p0); 
        }

        PlayerObject::doReversePlayer(p0);
    }

    void setVisible(bool visible) {
        PlayerObject::setVisible(visible);
        if (m_fields->m_customSprite) {
            m_fields->m_customSprite->setVisible(visible);
        }
    }

    void onExit() override {
        // Cleanup custom sprite
        if (m_fields->m_customSprite) {
            m_fields->m_customSprite->removeFromParent();
            m_fields->m_customSprite = nullptr;
        }
        PlayerObject::onExit();
    }
};