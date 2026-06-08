#include "raylib.h"

#include <algorithm>

#include "combat_config.h"
#include "player.h"
#include "resource_dir.h"

namespace
{
struct MatchState
{
    MatchConfig config = {};
    Player players[2];
    ProjectileInstance projectile = {};
    float roundTimer = 60.0f;
    bool gameOver = false;
    int winner = -1;
    bool debugHitbox = false;
};

InputState ReadPlayer1Input()
{
    return {IsKeyDown(KEY_A), IsKeyDown(KEY_D), IsKeyPressed(KEY_K), IsKeyPressed(KEY_J), IsKeyPressed(KEY_U), IsKeyPressed(KEY_I), IsKeyPressed(KEY_L)};
}

InputState ReadPlayer2Input()
{
    return {IsKeyDown(KEY_LEFT), IsKeyDown(KEY_RIGHT), IsKeyPressed(KEY_KP_2), IsKeyPressed(KEY_KP_1), IsKeyPressed(KEY_KP_4), IsKeyPressed(KEY_KP_5), IsKeyPressed(KEY_KP_3)};
}

void ResetMatch(MatchState &match)
{
    match.players[0].Reset(MakeNarutoConfig(), 300.0f, match.config.floorY);
    match.players[1].Reset(MakeWhiteMaskConfig(), 980.0f, match.config.floorY);
    match.roundTimer = match.config.roundTime;
    match.projectile = {};
    match.gameOver = false;
    match.winner = -1;
}

Rectangle MakeProjectileRect(const ProjectileInstance &projectile)
{
    return {projectile.x, projectile.y, projectile.width, projectile.height};
}

bool CanResolveHit(const Player &attacker, const Player &defender, const HitboxInstance &hitbox)
{
    if (hitbox.rect.width <= 0.0f)
    {
        return false;
    }
    if (!CheckCollisionRecs(hitbox.rect, defender.GetBody()))
    {
        return false;
    }
    if (!attacker.IsFacingTarget(defender.GetCenterX(), hitbox.attack.ignoresFacing))
    {
        return false;
    }
    return defender.CanBeHitBy(hitbox.attack);
}

void TryResolveHit(Player &attacker, Player &defender, const HitboxInstance &hitbox)
{
    if (!CanResolveHit(attacker, defender, hitbox))
    {
        return;
    }

    if (defender.TryTriggerCounter(hitbox, attacker))
    {
        attacker.ConsumeHit(hitbox.actionId);
        return;
    }

    defender.ApplyHit(hitbox.attack, hitbox.fromSkill, attacker.GetFacing());
    attacker.ConsumeHit(hitbox.actionId);
    if (hitbox.fromSkill && attacker.ShouldGrantSkillBean(hitbox.actionId))
    {
        attacker.MarkSkillBeanGranted(hitbox.actionId);
    }
}

void SpawnProjectileIfNeeded(MatchState &match, int ownerIndex)
{
    Player &owner = match.players[ownerIndex];
    if (!owner.ConsumeProjectileSpawnRequest())
    {
        return;
    }

    const FighterConfig &config = owner.GetConfig();
    match.projectile.active = true;
    match.projectile.ownerIndex = ownerIndex;
    match.projectile.width = config.skill1.projectileWidth;
    match.projectile.height = config.skill1.projectileHeight;
    match.projectile.x = owner.GetCenterX() + owner.GetFacing() * config.skill1.projectileOffsetX;
    match.projectile.y = owner.GetBody().y + config.skill1.projectileOffsetY;
    match.projectile.speed = config.skill1.projectileSpeed;
    match.projectile.direction = static_cast<float>(owner.GetFacing());
    match.projectile.remainingLife = config.skill1.projectileLifetime;
    match.projectile.hitInterval = config.skill1.projectileHitInterval;
    match.projectile.hitCooldown = 0.0f;
    match.projectile.attack = config.skill1.attack;
    match.projectile.beanGranted = false;
}

void UpdateProjectile(MatchState &match, float dt)
{
    if (!match.projectile.active)
    {
        return;
    }

    ProjectileInstance &projectile = match.projectile;
    projectile.remainingLife -= dt;
    projectile.hitCooldown = std::max(0.0f, projectile.hitCooldown - dt);
    projectile.x += projectile.speed * projectile.direction * dt;

    if (projectile.remainingLife <= 0.0f || projectile.x + projectile.width < match.config.stageLeft || projectile.x > match.config.stageRight)
    {
        projectile.active = false;
        return;
    }

    const int targetIndex = projectile.ownerIndex == 0 ? 1 : 0;
    Player &owner = match.players[projectile.ownerIndex];
    Player &target = match.players[targetIndex];
    const Rectangle projectileRect = MakeProjectileRect(projectile);

    if (projectile.hitCooldown <= 0.0f && CheckCollisionRecs(projectileRect, target.GetBody()))
    {
        const HitboxInstance projectileHit = {ActionId::Skill1, projectile.attack, projectileRect, true};
        if (target.TryTriggerCounter(projectileHit, owner))
        {
            projectile.active = false;
            return;
        }

        if (target.CanBeHitBy(projectile.attack))
        {
            target.ApplyHit(projectile.attack, true, owner.GetFacing());
            projectile.hitCooldown = projectile.hitInterval;
            if (!projectile.beanGranted)
            {
                owner.GrantBean();
                projectile.beanGranted = true;
            }
        }
    }
}

void ResolveRound(MatchState &match)
{
    if (match.players[0].IsDefeated() || match.players[1].IsDefeated())
    {
        match.gameOver = true;
        match.winner = match.players[0].IsDefeated() ? 1 : 0;
        return;
    }

    if (match.roundTimer <= 0.0f)
    {
        match.gameOver = true;
        if (match.players[0].GetHealth() == match.players[1].GetHealth())
        {
            match.winner = -1;
        }
        else
        {
            match.winner = match.players[0].GetHealth() > match.players[1].GetHealth() ? 0 : 1;
        }
    }
}

void DrawStage(const MatchState &match)
{
    ClearBackground(Color{244, 236, 215, 255});
    DrawRectangle(0, static_cast<int>(match.config.floorY), match.config.screenWidth, match.config.screenHeight, Color{215, 197, 167, 255});
    DrawLineEx({0.0f, match.config.floorY}, {static_cast<float>(match.config.screenWidth), match.config.floorY}, 3.0f, DARKBROWN);
    DrawRectangleLinesEx({match.config.stageLeft, 0.0f, match.config.stageRight - match.config.stageLeft, match.config.floorY}, 3.0f, Fade(BLACK, 0.15f));
    DrawText("1P: A/D J K U I L", 24, match.config.screenHeight - 56, 22, BLACK);
    DrawText("2P: <- -> Num1 Num2 Num4 Num5 Num3", 900, match.config.screenHeight - 56, 22, BLACK);
    DrawText("F1: debug hitbox", 24, match.config.screenHeight - 84, 20, DARKGRAY);
}
}

int main()
{
    SearchAndSetResourceDir("resources");

    MatchState match = {};
    InitWindow(match.config.screenWidth, match.config.screenHeight, "Naruto Duel MVP");
    SetTargetFPS(60);
    ResetMatch(match);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        if (dt > 0.1f)
            dt = 0.1f; // Cap dt for stability

        if (IsKeyPressed(KEY_F1))
        {
            match.debugHitbox = !match.debugHitbox;
        }

        if (!match.gameOver)
        {
            match.roundTimer -= dt;
            match.players[0].Update(dt, match.config.stageLeft, match.config.stageRight, match.config.floorY, ReadPlayer1Input(), match.players[1]);
            match.players[1].Update(dt, match.config.stageLeft, match.config.stageRight, match.config.floorY, ReadPlayer2Input(), match.players[0]);

            const HitboxInstance p1Hit = match.players[0].GetActiveHitbox();
            if (p1Hit.rect.width > 0.0f)
            {
                TryResolveHit(match.players[0], match.players[1], p1Hit);
            }

            const HitboxInstance p2Hit = match.players[1].GetActiveHitbox();
            if (p2Hit.rect.width > 0.0f)
            {
                TryResolveHit(match.players[1], match.players[0], p2Hit);
            }

            SpawnProjectileIfNeeded(match, 0);
            SpawnProjectileIfNeeded(match, 1);
            UpdateProjectile(match, dt);

            ResolveRound(match);
        }
        else
        {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_R))
            {
                ResetMatch(match);
            }
        }

        BeginDrawing();
        DrawStage(match);

        match.players[0].Draw(match.debugHitbox);
        match.players[1].Draw(match.debugHitbox);

        if (match.projectile.active)
        {
            const Color pColor = match.projectile.ownerIndex == 0 ? match.players[0].GetConfig().accentColor : match.players[1].GetConfig().accentColor;
            DrawRectangleRec(MakeProjectileRect(match.projectile), Fade(pColor, 0.6f));
        }

        match.players[0].DrawHud(0, static_cast<float>(match.config.screenWidth), match.roundTimer);
        match.players[1].DrawHud(1, static_cast<float>(match.config.screenWidth), match.roundTimer);

        if (match.gameOver)
        {
            const char *winText = match.winner == -1 ? "DRAW!" : (match.winner == 0 ? "1P NARUTO WINS!" : "2P WHITE MASK WINS!");
            const int winFontSize = 60;
            const int textX = match.config.screenWidth / 2 - MeasureText(winText, winFontSize) / 2;
            const int textY = match.config.screenHeight / 2 - winFontSize / 2;

            DrawRectangle(0, 0, match.config.screenWidth, match.config.screenHeight, Fade(BLACK, 0.5f));
            DrawText(winText, textX, textY, winFontSize, GOLD);
            DrawText("PRESS ENTER OR R TO RESTART", match.config.screenWidth / 2 - MeasureText("PRESS ENTER OR R TO RESTART", 20) / 2, textY + 80, 20, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
