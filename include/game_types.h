#pragma once

#include "raylib.h"

enum class FighterState
{
    Neutral,
    Moving,
    Jumping,
    Attacking,
    Hitstun,
    Airborne,
    KnockedDown,
    Grabbed,
    Dead
};

enum class ArmorType
{
    Normal,
    HyperArmor,
    SuperArmor,
    Invincible
};

enum class ActionId
{
    None,
    Normal1,
    Normal2,
    Normal3,
    Normal4,
    Skill1,
    Skill2,
    Skill2Counter
};

enum class AttackType
{
    Strike,
    Launch,
    Sweep,
    Grab
};

struct InputState
{
    bool leftDown = false;
    bool rightDown = false;
    bool jumpPressed = false;
    bool attackPressed = false;
    bool skill1Pressed = false;
    bool skill2Pressed = false;
    bool substitutePressed = false;
};

struct AttackData
{
    AttackType type = AttackType::Strike;
    float damage = 0.0f;
    float hitstun = 0.0f;
    float knockbackX = 0.0f;
    float knockbackY = 0.0f;
    float captureTime = 0.0f;

    float hitboxOffsetX = 0.0f;
    float hitboxOffsetY = 0.0f;
    float hitboxWidth = 0.0f;
    float hitboxHeight = 0.0f;

    bool hitsDowned = false;
    bool ignoresFacing = false;
};

struct ActionDefinition
{
    ActionId id = ActionId::None;

    float totalTime = 0.0f;
    float activeStart = 0.0f;
    float activeEnd = 0.0f;

    float forwardSpeed = 0.0f;
    float cooldown = 0.0f;
    bool canSkillCancel = false;
    bool isSkill = false;

    bool spawnsProjectile = false;
    float projectileLifetime = 0.0f;
    float projectileSpeed = 0.0f;
    float projectileHitInterval = 0.0f;
    float projectileWidth = 0.0f;
    float projectileHeight = 0.0f;
    float projectileOffsetX = 0.0f;
    float projectileOffsetY = 0.0f;

    ArmorType armor = ArmorType::Normal;
    float invincibleTime = 0.0f;
    AttackData attack;
};

struct FighterConfig
{
    const char *name = "";
    Color primaryColor = WHITE;
    Color accentColor = WHITE;
    float maxHealth = 1000.0f;
    float width = 58.0f;
    float height = 96.0f;
    float moveSpeed = 310.0f;
    float jumpVelocity = 720.0f;
    float gravity = 1800.0f;
    int maxBeans = 4;
    int startingBeans = 2;
    float substitutionCooldown = 15.0f;
    float substitutionInvuln = 0.5f;
    float substitutionRecovery = 0.12f;
    float wakeupTime = 1.1f;
    float wakeupInvuln = 0.45f;
    float comboResetTime = 1.0f;
    float comboProtectionDamage = 230.0f;
    float hardLandingMinAirTime = 0.2f;
    ActionDefinition normals[4];
    ActionDefinition skill1;
    ActionDefinition skill2;
};

struct HitboxInstance
{
    ActionId actionId = ActionId::None;
    AttackData attack;
    Rectangle rect = {0, 0, 0, 0};
    bool fromSkill = false;
};

struct ProjectileInstance
{
    bool active = false;
    int ownerIndex = -1;
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float speed = 0.0f;
    float direction = 1.0f;
    float remainingLife = 0.0f;
    float hitInterval = 0.0f;
    float hitCooldown = 0.0f;
    AttackData attack;
    bool beanGranted = false;
};
