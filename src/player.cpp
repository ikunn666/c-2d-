#include "player.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr float kHorizontalDamping = 0.82f;
constexpr float kAirHorizontalDamping = 0.96f;
constexpr float kSubstituteDistance = 70.0f;
constexpr float kHudMargin = 36.0f;
constexpr float kHudBarWidth = 520.0f;
constexpr float kHudBarHeight = 26.0f;
}

Player::Player(const FighterConfig &fighterConfig, float startX, float floorY)
{
    Reset(fighterConfig, startX, floorY);
}

void Player::Reset(const FighterConfig &fighterConfig, float startX, float floorY)
{
    config = fighterConfig;
    body = {startX, floorY - config.height, config.width, config.height};
    velocity = {0.0f, 0.0f};
    state = FighterState::Neutral;
    action = {};
    facing = 1;
    normalIndex = 0;
    beans = config.startingBeans;
    health = config.maxHealth;
    stateTimer = 0.0f;
    invulnTimer = 0.0f;
    substitutionCooldown = 0.0f;
    skill1Cooldown = 0.0f;
    skill2Cooldown = 0.0f;
    comboTimer = 0.0f;
    comboDamage = 0.0f;
    airborneTime = 0.0f;
    grabReleaseKnockbackX = 0.0f;
    grabReleaseKnockbackY = 0.0f;
}

void Player::Update(float dt, float stageLeft, float stageRight, float floorY, const InputState &input, const Player &opponent)
{
    if (input.leftDown != input.rightDown)
    {
        facing = input.leftDown ? -1 : 1;
    }

    UpdateTimers(dt);
    UpdateStateTimers(floorY);
    UpdateMovement(stageLeft, stageRight, floorY, input, opponent);
    ApplyGravity(dt, floorY);

    body.x += velocity.x * dt;
    body.y += velocity.y * dt;

    const bool touchedFloor = body.y + body.height >= floorY;
    if (touchedFloor)
    {
        body.y = floorY - body.height;
        if (velocity.y > 0.0f)
        {
            velocity.y = 0.0f;
        }

        if (state == FighterState::Jumping)
        {
            state = FighterState::Neutral;
        }
        else if (state == FighterState::Airborne)
        {
            if (comboDamage >= config.comboProtectionDamage || airborneTime >= config.hardLandingMinAirTime)
            {
                EnterKnockdown();
            }
            else
            {
                EnterNeutralState();
            }
        }
        airborneTime = 0.0f;
    }
    else if (state == FighterState::Airborne || state == FighterState::Jumping)
    {
        airborneTime += dt;
    }

    ClampToStage(stageLeft, stageRight);
}

void Player::Draw(bool debugHitbox) const
{
    Color tint = config.primaryColor;
    const char *statusText = nullptr;

    if (state == FighterState::Hitstun)
    {
        tint = RED;
        statusText = "HITSTUN";
    }
    else if (state == FighterState::Airborne)
    {
        tint = RED;
        statusText = comboDamage >= config.comboProtectionDamage ? "DROPPING!" : "AIRBORNE";
    }
    else if (state == FighterState::Grabbed)
    {
        tint = MAROON;
        statusText = "GRABBED";
    }
    else if (state == FighterState::KnockedDown)
    {
        tint = DARKGRAY;
        statusText = "DOWN";
    }
    else if (IsInvincible())
    {
        tint = Fade(config.accentColor, 0.72f);
        statusText = "INVINCIBLE";
    }
    else if (action.definition != nullptr)
    {
        if (action.definition->armor == ArmorType::SuperArmor)
        {
            tint = Fade(SKYBLUE, 0.82f);
            statusText = "SUPER ARMOR";
        }
        else if (action.definition->armor == ArmorType::HyperArmor)
        {
            tint = Fade(LIME, 0.82f);
            statusText = "HARD ARMOR";
        }
    }

    DrawRectangleRounded(body, 0.22f, 6, tint);
    DrawRectangleLinesEx(body, 2.0f, BLACK);

    if (statusText != nullptr)
    {
        const int fontSize = 20;
        DrawText(statusText, static_cast<int>(GetCenterX() - MeasureText(statusText, fontSize) * 0.5f), static_cast<int>(body.y - 24.0f), fontSize, tint);
    }

    const float eyeX = facing == 1 ? body.x + body.width * 0.68f : body.x + body.width * 0.32f;
    DrawCircle(static_cast<int>(eyeX), static_cast<int>(body.y + body.height * 0.30f), 4.0f, BLACK);

    if (debugHitbox)
    {
        const HitboxInstance hitbox = GetActiveHitbox();
        if (hitbox.rect.width > 0.0f)
        {
            DrawRectangleLinesEx(hitbox.rect, 2.0f, YELLOW);
        }
    }
}

void Player::DrawHud(int index, float screenWidth, float roundTimeRemaining) const
{
    const bool isLeft = index == 0;
    const float barX = isLeft ? kHudMargin : screenWidth - kHudMargin - kHudBarWidth;
    const float barY = 32.0f;
    const float healthRatio = std::max(0.0f, health / config.maxHealth);

    DrawText(config.name, static_cast<int>(barX), 6, 24, BLACK);
    DrawRectangleRounded({barX, barY, kHudBarWidth, kHudBarHeight}, 0.3f, 8, Fade(BLACK, 0.18f));
    DrawRectangleRounded({barX + 4.0f, barY + 4.0f, (kHudBarWidth - 8.0f) * healthRatio, kHudBarHeight - 8.0f}, 0.3f, 8, config.primaryColor);

    for (int i = 0; i < config.maxBeans; ++i)
    {
        const float beanX = barX + i * 30.0f;
        const float beanY = barY + 42.0f;
        DrawRectangleRounded({beanX, beanY, 22.0f, 18.0f}, 0.35f, 4, i < beans ? GOLD : Fade(DARKGRAY, 0.35f));
    }

    const int subCooldown = static_cast<int>(std::ceil(std::max(substitutionCooldown, 0.0f)));
    DrawText(TextFormat("�� %ds", subCooldown), static_cast<int>(barX), static_cast<int>(barY + 70.0f), 20, MAROON);
    DrawText(TextFormat("��1 %.1f", std::max(skill1Cooldown, 0.0f)), static_cast<int>(barX + 110.0f), static_cast<int>(barY + 70.0f), 20, BLACK);
    DrawText(TextFormat("��2 %.1f", std::max(skill2Cooldown, 0.0f)), static_cast<int>(barX + 220.0f), static_cast<int>(barY + 70.0f), 20, BLACK);

    if (isLeft)
    {
        const char *timeText = TextFormat("%02d", static_cast<int>(std::ceil(roundTimeRemaining)));
        DrawText(timeText, static_cast<int>(screenWidth * 0.5f - MeasureText(timeText, 36) * 0.5f), 26, 36, BLACK);
    }
}

bool Player::CanUseSubstitute() const
{
    return beans > 0 && substitutionCooldown <= 0.0f && (state == FighterState::Hitstun || state == FighterState::Airborne || state == FighterState::KnockedDown);
}

void Player::UseSubstitute(const Player &opponent, float stageLeft, float stageRight, float floorY)
{
    if (!CanUseSubstitute())
    {
        return;
    }

    beans = std::max(0, beans - 1);
    substitutionCooldown = config.substitutionCooldown;
    invulnTimer = config.substitutionInvuln;
    action = {};
    state = FighterState::Attacking;
    stateTimer = config.substitutionRecovery;

    const float behindDirection = -static_cast<float>(opponent.GetFacing());
    body.x = opponent.GetCenterX() + behindDirection * kSubstituteDistance - body.width * 0.5f;
    body.y = floorY - body.height;
    velocity = {0.0f, 0.0f};
    ResetComboProtection();
    ClampToStage(stageLeft, stageRight);
}

void Player::ApplyHit(const AttackData &attack, bool fromSkill, int attackerFacing)
{
    if (!CanBeHitBy(attack))
    {
        return;
    }

    const bool isGrab = attack.type == AttackType::Grab;
    const bool isNormalHit = !fromSkill && !isGrab;
    const ArmorType currentArmor = (action.definition != nullptr) ? action.definition->armor : ArmorType::Normal;

    // Check armor protection
    bool ignoreHitstun = false;
    if (currentArmor == ArmorType::Invincible)
    {
        return; // Double check, though CanBeHitBy already checks invuln
    }
    else if (currentArmor == ArmorType::SuperArmor && !isGrab)
    {
        ignoreHitstun = true; // SuperArmor (霸体) only taken by Grabs
    }
    else if (currentArmor == ArmorType::HyperArmor && isNormalHit)
    {
        ignoreHitstun = true; // HyperArmor (硬体) only taken by Skills/Grabs
    }

    health = std::max(0.0f, health - attack.damage);
    comboTimer = config.comboResetTime;
    comboDamage += attack.damage;

    if (ignoreHitstun)
    {
        return; // Damage taken but no state change
    }

    if (isGrab)
    {
        state = FighterState::Grabbed;
        stateTimer = attack.captureTime;
        velocity = {0.0f, 0.0f};
        grabReleaseKnockbackX = attack.knockbackX * static_cast<float>(attackerFacing);
        grabReleaseKnockbackY = attack.knockbackY;
        action = {}; // Cancel current action when grabbed
        if (health <= 0.0f)
        {
            state = FighterState::Dead;
        }
        return;
    }

    if (state == FighterState::KnockedDown && !attack.hitsDowned)
    {
        return;
    }

    if (attack.type == AttackType::Launch || attack.type == AttackType::Sweep)
    {
        state = FighterState::Airborne;
        stateTimer = attack.hitstun;
        velocity.x = attack.knockbackX * static_cast<float>(attackerFacing);
        velocity.y = -attack.knockbackY;
        airborneTime = 0.0f;
    }
    else
    {
        state = FighterState::Hitstun;
        stateTimer = attack.hitstun;
        velocity.x = attack.knockbackX * static_cast<float>(attackerFacing);
        if (fromSkill && std::fabs(velocity.x) < 10.0f)
        {
            velocity.x = 35.0f * static_cast<float>(attackerFacing);
        }
    }

    if (health <= 0.0f)
    {
        state = FighterState::Dead;
    }
}

bool Player::TryTriggerCounter(const HitboxInstance &incoming, const Player &attacker)
{
    if (!IsCounterWindowActive() || incoming.attack.type == AttackType::Grab)
    {
        return false;
    }

    body.x = attacker.GetCenterX() - attacker.GetFacing() * kSubstituteDistance - body.width * 0.5f;
    velocity = {0.0f, 0.0f};
    StartCounterAttack();
    return true;
}

bool Player::IsCounterWindowActive() const
{
    if (action.id != ActionId::Skill2 || action.definition == nullptr)
    {
        return false;
    }
    return action.elapsed >= action.definition->activeStart && action.elapsed <= action.definition->activeEnd;
}

bool Player::IsInvincible() const
{
    return invulnTimer > 0.0f || (action.definition != nullptr && action.definition->armor == ArmorType::Invincible && action.elapsed <= action.definition->invincibleTime);
}

bool Player::CanBeHitBy(const AttackData &attack) const
{
    if (state == FighterState::Dead || IsInvincible())
    {
        return false;
    }
    if (state == FighterState::KnockedDown && !attack.hitsDowned)
    {
        return false;
    }
    return true;
}

bool Player::IsFacingTarget(float targetCenterX, bool ignoreFacing) const
{
    if (ignoreFacing)
    {
        return true;
    }
    return facing == 1 ? targetCenterX >= GetCenterX() : targetCenterX <= GetCenterX();
}

bool Player::ConsumeProjectileSpawnRequest()
{
    if (action.definition == nullptr || !action.definition->spawnsProjectile || action.projectileSpawned)
    {
        return false;
    }
    if (action.elapsed < action.definition->activeStart)
    {
        return false;
    }
    action.projectileSpawned = true;
    return true;
}

bool Player::ShouldGrantSkillBean(ActionId actionId) const
{
    return action.id == actionId && action.definition != nullptr && action.definition->isSkill && !action.beanGranted;
}

void Player::MarkSkillBeanGranted(ActionId actionId)
{
    if (action.id == actionId)
    {
        action.beanGranted = true;
    }
    GrantBean();
}

void Player::GrantBean()
{
    beans = std::min(config.maxBeans, beans + 1);
}

void Player::ConsumeHit(ActionId actionId)
{
    if (action.id == actionId)
    {
        action.hitConnected = true;
    }
}

void Player::ResetComboProtection()
{
    comboTimer = 0.0f;
    comboDamage = 0.0f;
}

Rectangle Player::GetBody() const
{
    return body;
}

HitboxInstance Player::GetActiveHitbox() const
{
    HitboxInstance hitbox = {};

    if (action.definition == nullptr)
    {
        return hitbox;
    }
    if (action.elapsed < action.definition->activeStart || action.elapsed > action.definition->activeEnd)
    {
        return hitbox;
    }
    if (action.definition->attack.hitboxWidth <= 0.0f || action.hitConnected)
    {
        return hitbox;
    }

    hitbox.actionId = action.id;
    hitbox.attack = action.definition->attack;
    hitbox.fromSkill = action.definition->isSkill;
    hitbox.rect = BuildHitbox(action.definition->attack);
    return hitbox;
}

const FighterConfig &Player::GetConfig() const
{
    return config;
}

float Player::GetHealth() const
{
    return health;
}

int Player::GetFacing() const
{
    return facing;
}

float Player::GetCenterX() const
{
    return body.x + body.width * 0.5f;
}

bool Player::IsDefeated() const
{
    return state == FighterState::Dead || health <= 0.0f;
}

void Player::UpdateTimers(float dt)
{
    stateTimer = std::max(0.0f, stateTimer - dt);
    invulnTimer = std::max(0.0f, invulnTimer - dt);
    substitutionCooldown = std::max(0.0f, substitutionCooldown - dt);
    skill1Cooldown = std::max(0.0f, skill1Cooldown - dt);
    skill2Cooldown = std::max(0.0f, skill2Cooldown - dt);

    if (comboTimer > 0.0f)
    {
        comboTimer = std::max(0.0f, comboTimer - dt);
        if (comboTimer <= 0.0f)
        {
            comboDamage = 0.0f;
        }
    }

    if (action.definition != nullptr)
    {
        action.elapsed += dt;
        if (action.elapsed >= action.definition->totalTime)
        {
            EndAction();
        }
    }
}

void Player::UpdateStateTimers(float floorY)
{
    if (state == FighterState::Grabbed && stateTimer <= 0.0f)
    {
        state = FighterState::Airborne;
        velocity.x = grabReleaseKnockbackX;
        velocity.y = -grabReleaseKnockbackY;
    }
    else if (state == FighterState::Hitstun && stateTimer <= 0.0f)
    {
        EnterNeutralState();
    }
    else if (state == FighterState::KnockedDown && stateTimer <= 0.0f)
    {
        body.y = floorY - body.height;
        EnterNeutralState();
        invulnTimer = std::max(invulnTimer, config.wakeupInvuln);
    }
    else if (state == FighterState::Attacking && action.definition == nullptr && stateTimer <= 0.0f)
    {
        EnterNeutralState();
    }
}

void Player::UpdateMovement(float stageLeft, float stageRight, float floorY, const InputState &input, const Player &opponent)
{
    const bool substituteRecovery = state == FighterState::Attacking && action.definition == nullptr && stateTimer > 0.0f;
    const bool locked = substituteRecovery || state == FighterState::Hitstun || state == FighterState::Airborne || state == FighterState::Grabbed || state == FighterState::KnockedDown || state == FighterState::Dead;
    if (locked)
    {
        // For Hitstun (grounded knockback), keep velocity constant to match attacker's dash
        if (state == FighterState::Hitstun)
        {
            // No damping during hitstun to maintain distance
        }
        else
        {
            velocity.x *= state == FighterState::Airborne ? kAirHorizontalDamping : kHorizontalDamping;
        }

        if (input.substitutePressed && CanUseSubstitute())
        {
            UseSubstitute(opponent, stageLeft, stageRight, floorY);
        }
        return;
    }

    if (action.definition != nullptr)
    {
        velocity.x = action.definition->forwardSpeed * static_cast<float>(facing);
        if (action.id == ActionId::Skill2)
        {
            velocity.x = 0.0f;
        }

        if (input.attackPressed && action.id != ActionId::Skill1 && action.id != ActionId::Skill2 && action.id != ActionId::Skill2Counter)
        {
            action.normalBuffered = true;
        }
        if (action.definition->canSkillCancel)
        {
            if (input.skill1Pressed && skill1Cooldown <= 0.0f)
            {
                StartSkillAction(0);
                return;
            }
            if (input.skill2Pressed && skill2Cooldown <= 0.0f)
            {
                StartSkillAction(1);
                return;
            }
        }
        return;
    }

    float horizontal = 0.0f;
    if (input.leftDown)
    {
        horizontal -= 1.0f;
    }
    if (input.rightDown)
    {
        horizontal += 1.0f;
    }

    velocity.x = horizontal * config.moveSpeed;
    state = horizontal != 0.0f ? FighterState::Moving : FighterState::Neutral;

    const bool grounded = std::abs(body.y + body.height - floorY) < 1.0f;
    if (input.jumpPressed && grounded)
    {
        velocity.y = -config.jumpVelocity;
        state = FighterState::Jumping;
    }

    if (input.attackPressed)
    {
        StartNormalAction();
        return;
    }
    if (input.skill1Pressed && skill1Cooldown <= 0.0f)
    {
        StartSkillAction(0);
        return;
    }
    if (input.skill2Pressed && skill2Cooldown <= 0.0f)
    {
        StartSkillAction(1);
    }
}

void Player::StartAction(const ActionDefinition &actionDefinition)
{
    action = {};
    action.definition = &actionDefinition;
    action.id = actionDefinition.id;
    state = FighterState::Attacking;
    velocity.x = actionDefinition.forwardSpeed * static_cast<float>(facing);
}

void Player::StartNormalAction()
{
    if (normalIndex < 0 || normalIndex >= 3)
    {
        normalIndex = 0;
    }
    else
    {
        ++normalIndex;
    }
    StartAction(config.normals[normalIndex]);
}

void Player::StartSkillAction(int skillIndex)
{
    if (skillIndex == 0)
    {
        StartAction(config.skill1);
        skill1Cooldown = config.skill1.cooldown;
    }
    else
    {
        StartAction(config.skill2);
        skill2Cooldown = config.skill2.cooldown;
    }
    normalIndex = -1;
}

void Player::StartCounterAttack()
{
    static const ActionDefinition counterAttack = MakeAction(ActionId::Skill2Counter, 0.42f, 0.02f, 0.16f, 0.0f, 0.0f, false, true, false, ArmorType::SuperArmor, 0.0f, AttackData{AttackType::Launch, 78.0f, 0.34f, 220.0f, 760.0f, 0.0f, 36.0f, 10.0f, 72.0f, 46.0f, false, false});
    StartAction(counterAttack);
}

void Player::EndAction()
{
    const bool chainNormal = action.normalBuffered && action.id != ActionId::Skill1 && action.id != ActionId::Skill2 && action.id != ActionId::Skill2Counter;
    action = {};
    if (chainNormal)
    {
        StartNormalAction();
        return;
    }
    EnterNeutralState();
}

void Player::EnterKnockdown()
{
    state = FighterState::KnockedDown;
    stateTimer = config.wakeupTime;
    velocity = {0.0f, 0.0f};
}

void Player::EnterNeutralState()
{
    if (health <= 0.0f)
    {
        state = FighterState::Dead;
        return;
    }
    state = FighterState::Neutral;
    velocity.x *= 0.2f;
}

void Player::ClampToStage(float stageLeft, float stageRight)
{
    if (body.x < stageLeft)
    {
        body.x = stageLeft;
        velocity.x = 0.0f;
    }
    if (body.x + body.width > stageRight)
    {
        body.x = stageRight - body.width;
        velocity.x = 0.0f;
    }
}

void Player::ApplyGravity(float dt, float floorY)
{
    if (body.y + body.height < floorY)
    {
        float gravityMult = 1.0f;
        if (state == FighterState::Airborne && comboDamage > 0.0f)
        {
            // Simple linear scaling of gravity based on combo damage
            // Once it hits comboProtectionDamage, gravity is significantly increased
            gravityMult = 1.0f + (std::min(comboDamage, config.comboProtectionDamage * 2.0f) / config.comboProtectionDamage) * 1.5f;
        }
        velocity.y += config.gravity * gravityMult * dt;
    }
}

Rectangle Player::BuildHitbox(const AttackData &attack) const
{
    const float x = facing == 1 ? body.x + body.width + attack.hitboxOffsetX : body.x - attack.hitboxWidth - attack.hitboxOffsetX;
    return {x, body.y + attack.hitboxOffsetY, attack.hitboxWidth, attack.hitboxHeight};
}

