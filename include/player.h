#pragma once
#include "raylib.h"

class Player
{
public:
    Rectangle rect; // 碰撞框：x, y, width, height
    Color color;    // 颜色
    Vector2 speed;  // 速度：x, y(水平速度，垂直速度)
    float health;
    bool isGrounded;
    int direction;       // 面朝方向：-1左，1右
    bool isAttacking;    // 是否正在攻击
    float attackTimer;   // 攻击计时器
    Rectangle attackBox; // 攻击范围

    Player(float x, Color c);
    void Update(float gravity, float floorY);
    void Draw();
    void HandleInput(int keyUp, int keyLeft, int keyRight, int keyAttack);
};