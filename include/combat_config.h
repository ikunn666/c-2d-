#pragma once

#include "game_types.h"

struct MatchConfig
{
    int screenWidth = 1600;
    int screenHeight = 900;
    float stageLeft = 58.0f;
    float stageRight = 1542.0f;
    float floorY = 650.0f;
    float roundTime = 60.0f;
};

inline ActionDefinition MakeAction(ActionId id, float totalTime, float activeStart, float activeEnd, float forwardSpeed, float cooldown, bool canSkillCancel, bool isSkill, bool spawnsProjectile, ArmorType armor, float invincibleTime, const AttackData &attack)
{
    ActionDefinition action = {};
    action.id = id;
    action.totalTime = totalTime;
    action.activeStart = activeStart;
    action.activeEnd = activeEnd;
    action.forwardSpeed = forwardSpeed;
    action.cooldown = cooldown;
    action.canSkillCancel = canSkillCancel;
    action.isSkill = isSkill;
    action.spawnsProjectile = spawnsProjectile;
    action.armor = armor;
    action.invincibleTime = invincibleTime;
    action.attack = attack;
    return action;
}

inline FighterConfig MakeNarutoConfig()
{
    FighterConfig config = {};
    config.name = "Naruto";
    config.primaryColor = ORANGE;
    config.accentColor = GOLD;
    config.moveSpeed = 335.0f;
    config.jumpVelocity = 760.0f;

    // Adjusted Normals: forwardSpeed == knockbackX, large hitboxes covering body
    config.normals[0] = MakeAction(ActionId::Normal1, 0.23f, 0.03f, 0.12f, 160.0f, 0.0f, true, false, false, ArmorType::Normal, 0.0f, AttackData{AttackType::Strike, 28.0f, 0.26f, 160.0f, 0.0f, 0.0f, -40.0f, 16.0f, 120.0f, 40.0f, false, false});
    config.normals[1] = MakeAction(ActionId::Normal2, 0.25f, 0.04f, 0.14f, 170.0f, 0.0f, true, false, false, ArmorType::Normal, 0.0f, AttackData{AttackType::Strike, 30.0f, 0.28f, 170.0f, 0.0f, 0.0f, -40.0f, 16.0f, 125.0f, 45.0f, false, false});
    config.normals[2] = MakeAction(ActionId::Normal3, 0.27f, 0.06f, 0.16f, 180.0f, 0.0f, true, false, false, ArmorType::Normal, 0.0f, AttackData{AttackType::Strike, 34.0f, 0.30f, 180.0f, 0.0f, 0.0f, -40.0f, 14.0f, 130.0f, 50.0f, false, false});
    config.normals[3] = MakeAction(ActionId::Normal4, 0.36f, 0.08f, 0.22f, 190.0f, 0.0f, false, false, false, ArmorType::Normal, 0.0f, AttackData{AttackType::Launch, 42.0f, 0.36f, 240.0f, 720.0f, 0.0f, -40.0f, 10.0f, 140.0f, 60.0f, false, false});

    // Skills
    config.skill1 = MakeAction(ActionId::Skill1, 0.56f, 0.06f, 0.30f, 530.0f, 7.0f, false, true, false, ArmorType::HyperArmor, 0.0f, AttackData{AttackType::Grab, 85.0f, 0.40f, 180.0f, 620.0f, 0.20f, -30.0f, 10.0f, 120.0f, 60.0f, false, false});
    config.skill2 = MakeAction(ActionId::Skill2, 0.48f, 0.01f, 0.22f, 750.0f, 9.0f, false, true, false, ArmorType::Invincible, 0.22f, AttackData{AttackType::Launch, 92.0f, 0.42f, 280.0f, 900.0f, 0.0f, -30.0f, 8.0f, 130.0f, 60.0f, false, false});
    return config;
}

inline FighterConfig MakeWhiteMaskConfig()
{
    FighterConfig config = {};
    config.name = "White Mask";
    config.primaryColor = DARKBLUE;
    config.accentColor = SKYBLUE;
    config.moveSpeed = 305.0f;
    config.jumpVelocity = 730.0f;

    // Adjusted Normals for White Mask: forwardSpeed == knockbackX
    config.normals[0] = MakeAction(ActionId::Normal1, 0.26f, 0.05f, 0.16f, 130.0f, 0.0f, true, false, false, ArmorType::Normal, 0.0f, AttackData{AttackType::Strike, 34.0f, 0.30f, 130.0f, 0.0f, 0.0f, -40.0f, 18.0f, 135.0f, 40.0f, false, false});
    config.normals[1] = MakeAction(ActionId::Normal2, 0.28f, 0.07f, 0.18f, 140.0f, 0.0f, true, false, false, ArmorType::Normal, 0.0f, AttackData{AttackType::Strike, 38.0f, 0.32f, 140.0f, 0.0f, 0.0f, -40.0f, 12.0f, 145.0f, 45.0f, false, false});
    config.normals[2] = MakeAction(ActionId::Normal3, 0.38f, 0.10f, 0.25f, 150.0f, 0.0f, false, false, false, ArmorType::Normal, 0.0f, AttackData{AttackType::Launch, 50.0f, 0.38f, 220.0f, 720.0f, 0.0f, -40.0f, 8.0f, 155.0f, 60.0f, false, false});
    config.normals[3] = config.normals[2];

    config.skill1 = MakeAction(ActionId::Skill1, 0.56f, 0.10f, 0.26f, 0.0f, 8.0f, false, true, true, ArmorType::HyperArmor, 0.0f, AttackData{AttackType::Sweep, 24.0f, 0.24f, 70.0f, 620.0f, 0.0f, 0.0f, 0.0f, 130.0f, 110.0f, true, true});
    config.skill1.projectileLifetime = 1.75f;
    config.skill1.projectileSpeed = 380.0f;
    config.skill1.projectileHitInterval = 0.22f;
    config.skill1.projectileWidth = 120.0f;
    config.skill1.projectileHeight = 120.0f;
    config.skill1.projectileOffsetX = 40.0f;
    config.skill1.projectileOffsetY = -18.0f;

    config.skill2 = MakeAction(ActionId::Skill2, 0.52f, 0.03f, 0.28f, 0.0f, 10.0f, false, true, false, ArmorType::SuperArmor, 0.0f, AttackData{});
    return config;
}
