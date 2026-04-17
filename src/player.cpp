#include "player.h"
#include <math.h>

// 构造函数
Player::Player(float x, Color c)
{
    rect = {x, 100, 40, 60};
    color = c;
    speed = {0, 0};
    health = 100.0f;
    isGrounded = false;
    direction = 1;
    currentState = IDLE;
    attackBox = {0, 0, 0, 0};
    stateTimer = 0.0f;     // 状态计时器
    hitRegistered = false; // 攻击伤害注册
    canCancel = true;      // 是否可以取消当前状态
}

// 物理逻辑
void Player::Update(float gravity, float floorY)
{
    // 状态计时器更新
    if (stateTimer > 0)
    {
        stateTimer -= GetFrameTime();
    }

    // 状态转换逻辑
    switch (currentState)
    {
    case ATTACK:
        if (stateTimer <= 0)
            currentState = IDLE;
        break;
    case HURT:
        if (stateTimer <= 0)
            currentState = IDLE;
        break;
    case KONCKEDDOWN:
        if (stateTimer <= 0)
            currentState = IDLE;
        break;
    case KONCKEDUP:
        if (stateTimer <= 0)
            currentState = IDLE;
        break;
    default:
        break;
    }
    // 物理系统
    float friction = isGrounded ? 0.6f : 0.98f; // 地面摩擦力较大，空中摩擦力较小
    speed.x *= friction;
    if (fabs(speed.x) < 0.1f)
        speed.x = 0; // 小速度归零

    // 重力逻辑
    if (!isGrounded)
        speed.y += gravity;

    // 位置更新
    rect.x += speed.x;
    rect.y += speed.y;

    // 地面碰撞检测
    if (rect.y + rect.height >= floorY)
    {
        rect.y = floorY - rect.height;
        speed.y = 0;
        isGrounded = true;
    }
    else
    {
        isGrounded = false;
    }

    // 边界检测，防止玩家离开屏幕
    if (rect.x < 0)
    {
        rect.x = 0;
    }
    if (rect.x + rect.width > GetScreenWidth())
    {
        rect.x = GetScreenWidth() - rect.width;
    }

    // 计算攻击框
    if (currentState == ATTACK && stateTimer > 0.05f && stateTimer < 0.15f)
    {
        float attackBoxWidth = 30;
        attackBox = {(direction == 1) ? rect.x + rect.width : rect.x - attackBoxWidth, rect.y + 10, attackBoxWidth, 30};
    }
    else
    {
        attackBox = {0, 0, 0, 0};
    }
}

//
void Player::TakeDamage(float damage, float kX, float kY)
{
    health -= damage;
    currentState = HURT;
    stateTimer = 0.5f; // 受伤状态持续0.5秒
    speed.x = kX;
    speed.y = kY;
}

// 绘制角色
void Player::Draw()
{
    Color displayColor = (currentState == HURT) ? RED : color;
    DrawRectangleRec(rect, displayColor);

    // 绘制攻击框（调试用）
    if (attackBox.width > 0)
    {
        DrawRectangleRec(attackBox, Fade(RED, 0.6f));
    }

    // 绘制血条
    DrawRectangle(rect.x, rect.y - 10, rect.width, 5, GRAY);                    // 背景血条
    DrawRectangle(rect.x, rect.y - 10, rect.width * (health / 100.0f), 5, RED); // 当前血量
}

// 输入处理
void Player::HandleInput(int keyUp, int keyLeft, int keyRight, int keyAttack)
{
    // 优先级1：受击、击倒、被击飞、死亡时无法控制
    if (currentState == HURT || currentState == KONCKEDDOWN || currentState == KONCKEDUP || currentState == DEAD)
    {
        return;
    }
    // 优先级2：攻击状态不能移动
    if (currentState == ATTACK)
        return;

    // 移动
    if (IsKeyDown(keyLeft))
    {
        speed.x = -6;
        direction = -1;
        currentState = MOVE;
    }
    else if (IsKeyDown(keyRight))
    {
        speed.x = 6;
        direction = 1;
        currentState = MOVE;
    }
    else if (isGrounded)
    {
        currentState = IDLE;
    }

    // 跳跃
    if (IsKeyPressed(keyUp) && isGrounded)
    {
        speed.y = -14;
        isGrounded = false;
        currentState = JUMP;
    }

    // 攻击
    if (IsKeyPressed(keyAttack))
    {
        currentState = ATTACK;
        stateTimer = 0.3f;     // 攻击状态持续0.3秒
        hitRegistered = false; // 重置攻击注册
    }
}
