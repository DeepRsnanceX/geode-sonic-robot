#include <Geode/Geode.hpp>
#include <Geode/Modify.hpp>
#include <Geode/utils/cocos.hpp>
#include <cocos2d.h>

using namespace geode::prelude;

class $modify(PlayerObject) {
    struct Fields {
        float m_animationTimer = 0.f;
        int m_currentFrame = 1; // Start from frame 1
        CCSprite* m_customSprite = nullptr;
        GJRobotSprite* m_robotSprite = nullptr; // Define m_robotSprite here
        bool m_flippedX = false; // Track X flip state
        bool m_flippedY = false; // Track Y flip state
    };

    bool init(int p0, int p1, GJBaseGameLayer* p2, cocos2d::CCLayer* p3, bool p4) {
        if (!PlayerObject::init(p0, p1, p2, p3, p4)) return false;

        // Hide robot sprite, fire and particles
        m_robotBatchNode->setVisible(false);

        // Create the custom sprite
        m_fields->m_customSprite = CCSprite::createWithSpriteFrameName("sonicRun_01.png"_spr); // Start with frame 1
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
            m_fields->m_customSprite->setDisplayFrame(CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName("sonicIdle_01.png"_spr));
            m_fields->m_animationTimer = 0; // Reset animation timer to stop cycling frames
        } else {
            // Determine current animation (run or jump)
            std::string frameName;


            if (this->m_isOnGround || this->m_hasGroundParticles) {
                frameName = fmt::format("sonicRun_0{}.png"_spr, m_fields->m_currentFrame);
            } else {
                frameName = fmt::format("sonicJump_0{}.png"_spr, m_fields->m_currentFrame);
            }

            // Update animation frame
            m_fields->m_animationTimer += dt;

            constexpr float frameDuration = 0.8f; // 2.5 FPS
            if (m_fields->m_animationTimer >= frameDuration) {
                m_fields->m_animationTimer -= frameDuration;

                // Update current frame (cycle through 1-4)
                m_fields->m_currentFrame = (m_fields->m_currentFrame % 4) + 1;

                // Get the new sprite frame from the cache
                auto frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(frameName.c_str());

                // Only update the frame if it's a valid one
                if (frame) {
                    m_fields->m_customSprite->setDisplayFrame(frame);
                }
            }
        }

        // Flip sprite based on gravity and facing direction
        flipGravity(m_fields->m_flippedY, m_fields->m_flippedX);
    }

    // Flip the sprite on Y
    void flipGravity(bool p0, bool p1) {
        // Flip Y based on the gravity
        if (p0 != m_fields->m_flippedY) {
            m_fields->m_flippedY = p0;
            m_fields->m_customSprite->setFlipY(p0); // Flip in Y direction
        }

        // Call the original flipGravity function (preserve its functionality)
        PlayerObject::flipGravity(p0, p1);
    }

    // Flip the sprite on X
    virtual void setFlipX(bool p0) override {
        // Flip X based on the player's direction (velocity or facing)
        if (p0 != m_fields->m_flippedX) {
            m_fields->m_flippedX = p0;
            m_fields->m_customSprite->setFlipX(p0); // Flip in X direction
        }

        // Call the original setFlipX function (preserve its functionality)
        PlayerObject::setFlipX(p0);
    }

    // Reverse the player's sprite (called separately from setFlipX)
    void doReversePlayer(bool p0) {
        if (p0 != m_fields->m_flippedX) {
            m_fields->m_flippedX = p0;
            m_fields->m_customSprite->setFlipX(p0); // Flip in X direction
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