#pragma once
#include "raylib.h"

//
enum PlayerState
{
    IDLE,
    MOVE,
    JUMP,
    ATTACK,
    HURT,
    KONCKEDDOWN,
    KONCKEDUP,
    DEAD
};

class Player
{
public:
    // 基础属性
    Rectangle rect; // 碰撞框：x, y, width, height
    Color color;    // 颜色
    Vector2 speed;  // 速度：x, y(水平速度，垂直速度)
    float health;
    int direction; // 面朝方向：-1左，1右

    // 状态属性
    bool isGrounded;
    PlayerState currentState; // 当前状态
    float stateTimer;         // 状态计时器
    bool canCancel;           // 是否可以取消当前状态

    // 攻击属性
    bool hitRegistered;  // 是否已经对当前攻击注册过伤害
    Rectangle attackBox; // 攻击范围

    Player(float x, Color c);
    void Update(float gravity, float floorY);
    void Draw();
    void HandleInput(int keyUp, int keyLeft, int keyRight, int keyAttack);
    void TakeDamage(float damage, float konckbackX, float knockbackY);
};
