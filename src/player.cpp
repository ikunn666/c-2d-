#include "player.h"

// 构造函数
Player::Player(float x, Color c)
{
    rect = {x, 100, 40, 60};
    color = c;
    speed = {0, 0};
    health = 100.0f;
    isGrounded = false;
    direction = 1;
    isAttacking = false;
    attackTimer = 0.0f;
    attackBox = {0, 0, 0, 0};
}

// 物理逻辑
void Player::Update(float gravity, float floorY)
{
    // 重力逻辑
    if (!isGrounded)
    {
        speed.y += gravity;
    }

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

    // 攻击计时
    if (isAttacking)
    {
        attackTimer -= GetFrameTime();
        if (attackTimer <= 0)
        {
            isAttacking = false;
        }
    }
    // 计算攻击框
    if (direction == 1)
    {
        attackBox = {rect.x + rect.width, rect.y, 30, 20};
    }
    else
    {
        attackBox = {rect.x - 30, rect.y, 30, 20};
    }
}

// 绘制角色
void Player::Draw()
{
    DrawRectangleRec(rect, color);

    // 绘制攻击框（调试用）
    if (isAttacking)
    {
        DrawRectangleRec(attackBox, Fade(RED, 0.5f));
    }

    // 绘制血条
    DrawRectangle(rect.x, rect.y - 10, rect.width, 5, GRAY);                    // 背景血条
    DrawRectangle(rect.x, rect.y - 10, rect.width * (health / 100.0f), 5, RED); // 当前血量
}

// 输入处理
void Player::HandleInput(int keyUp, int keyLeft, int keyRight, int keyAttack)
{

    speed.x = 0;
    if (IsKeyDown(keyLeft))
    {
        speed.x = -5;
        direction = -1;
    }
    if (IsKeyDown(keyRight))
    {
        speed.x = 5;
        direction = 1;
    }

    // 跳跃必须在地板上
    if (IsKeyPressed(keyUp) && isGrounded)
    {
        speed.y = -12;
        isGrounded = false;
    }

    // 攻击
    if (IsKeyPressed(keyAttack) && !isAttacking)
    {
        isAttacking = true;
        attackTimer = 0.2f; // 攻击持续时间0.2秒
    }
}