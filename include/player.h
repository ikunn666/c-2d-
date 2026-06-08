#pragma once

#include "combat_config.h"

class Player
{
public:
    Player() = default;
    Player(const FighterConfig &fighterConfig, float startX, float floorY);
    void Reset(const FighterConfig &fighterConfig, float startX, float floorY);

    void Update(float dt, float stageLeft, float stageRight, float floorY, const InputState &input, const Player &opponent);
    void Draw(bool debugHitbox) const;
    void DrawHud(int index, float screenWidth, float roundTimeRemaining) const;

    bool CanUseSubstitute() const;
    void UseSubstitute(const Player &opponent, float stageLeft, float stageRight, float floorY);
    void ApplyHit(const AttackData &attack, bool fromSkill, int attackerFacing);
    bool TryTriggerCounter(const HitboxInstance &incoming, const Player &attacker);
    bool IsCounterWindowActive() const;
    bool IsInvincible() const;
    bool CanBeHitBy(const AttackData &attack) const;
    bool IsFacingTarget(float targetCenterX, bool ignoreFacing) const;
    bool ConsumeProjectileSpawnRequest();
    bool ShouldGrantSkillBean(ActionId actionId) const;
    void MarkSkillBeanGranted(ActionId actionId);
    void GrantBean();
    void ConsumeHit(ActionId actionId);
    void ResetComboProtection();

    Rectangle GetBody() const;
    HitboxInstance GetActiveHitbox() const;
    const FighterConfig &GetConfig() const;
    float GetHealth() const;
    int GetFacing() const;
    float GetCenterX() const;
    bool IsDefeated() const;

private:
    struct ActionRuntime
    {
        const ActionDefinition *definition = nullptr;
        ActionId id = ActionId::None;
        float elapsed = 0.0f;
        bool hitConnected = false;
        bool projectileSpawned = false;
        bool beanGranted = false;
        bool normalBuffered = false;
    };

    void UpdateTimers(float dt);
    void UpdateStateTimers(float floorY);
    void UpdateMovement(float stageLeft, float stageRight, float floorY, const InputState &input, const Player &opponent);
    void StartAction(const ActionDefinition &actionDefinition);
    void StartNormalAction();
    void StartSkillAction(int skillIndex);
    void StartCounterAttack();
    void EndAction();
    void EnterKnockdown();
    void EnterNeutralState();
    void ClampToStage(float stageLeft, float stageRight);
    void ApplyGravity(float dt, float floorY);
    Rectangle BuildHitbox(const AttackData &attack) const;

    FighterConfig config = {};
    Rectangle body = {0, 0, 0, 0};
    Vector2 velocity = {0, 0};
    FighterState state = FighterState::Neutral;
    ActionRuntime action = {};

    int facing = 1;
    int normalIndex = 0;
    int beans = 0;

    float health = 0.0f;
    float stateTimer = 0.0f;
    float invulnTimer = 0.0f;
    float substitutionCooldown = 0.0f;
    float skill1Cooldown = 0.0f;
    float skill2Cooldown = 0.0f;
    float comboTimer = 0.0f;
    float comboDamage = 0.0f;
    float airborneTime = 0.0f;
    float grabReleaseKnockbackX = 0.0f;
    float grabReleaseKnockbackY = 0.0f;
};
