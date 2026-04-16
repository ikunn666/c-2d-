/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"

#include "resource_dir.h" // utility header for SearchAndSetResourceDir

class Player
{
public:
	Rectangle rect; // 碰撞框：x, y, width, height
	Color color;	// 颜色
	Vector2 speed;	// 速度：x, y(水平速度，垂直速度)
	float health;
	bool isGrounded;
	int direction;		 // 面朝方向：-1左，1右
	bool isAttacking;	 // 是否正在攻击
	float attackTimer;	 // 攻击计时器
	Rectangle attackBox; // 攻击范围

	// 初始化玩家属性
	Player(float x, Color c)
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
	void Update(float gravity, float floorY)
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
	void Draw()
	{
		DrawRectangleRec(rect, color);

		// 绘制攻击框（调试用）
		if (isAttacking)
		{
			DrawRectangleRec(attackBox, Fade(RED, 0.5f));
		}

		// 绘制血条
		DrawRectangle(rect.x, rect.y - 10, rect.width, 5, GRAY);					// 背景血条
		DrawRectangle(rect.x, rect.y - 10, rect.width * (health / 100.0f), 5, RED); // 当前血量
	}

	// 处理输入
	void HandleInput(int keyUp, int keyLeft, int keyRight, int keyAttack)
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
};

int main()
{
	// 初始化窗口
	const int screenWidth = 1500;
	const int screenHeight = 800;
	InitWindow(screenWidth, screenHeight, "Naruto MVP1");

	SetTargetFPS(60); // 设置帧率

	// 物理参数
	float gravity = 0.5f;
	float floorY = 350.0f;

	// 创建两个玩家
	Player p1(200.0f, RED);
	Player p2(600.f, BLUE);

	bool gameOver = false;
	const char *winnerText = "";

	while (IsWindowReady())
	{

		// 游戏逻辑
		if (!gameOver)
		{
			// 移动
			p1.HandleInput(KEY_K, KEY_A, KEY_D, KEY_J);
			p2.HandleInput(KEY_KP_2, KEY_LEFT, KEY_RIGHT, KEY_KP_1);

			// 更新物理状态
			p1.Update(gravity, floorY);
			p2.Update(gravity, floorY);

			// 碰撞检测
			if (p1.isAttacking && CheckCollisionRecs(p1.attackBox, p2.rect))
			{
				p2.health -= 0.5f; // 每帧扣除0.5血(0.2秒*60帧=12血)
			}
			if (p2.isAttacking && CheckCollisionRecs(p2.attackBox, p1.rect))
			{
				p1.health -= 0.5f;
			}

			// 检查游戏结束
			if (!gameOver)
			{
				if (p1.health <= 0)
				{
					gameOver = true;
					winnerText = "Player 2 Wins!";
				}
				else if (p2.health <= 0)
				{
					gameOver = true;
					winnerText = "Player 1 Wins!";
				}
			}
		}
		else
		{
			if (IsKeyPressed(KEY_R))
			{
				// 重置游戏
				p1 = Player(200.0f, RED);
				p2 = Player(600.f, BLUE);
				gameOver = false;
				winnerText = "";
			}
		}

		// 2.绘制阶段
		BeginDrawing();
		ClearBackground(RAYWHITE);

		// 绘制地面
		DrawLine(0, (int)floorY, screenWidth, (int)floorY, BLACK);
		DrawText("Naruto MVP1,重力和地面", 10, 10, 20, DARKGRAY);

		p1.Draw();
		p2.Draw();

		if (gameOver)
		{
			// 屏幕中心画出胜利文本
			int textWidth = MeasureText(winnerText, 40);
			DrawText(winnerText, (screenWidth - textWidth) / 2, screenHeight / 2 - 20, 40, DARKGREEN);
			DrawText("Press R to Restart", (screenWidth - MeasureText("Press R to Restart", 20)) / 2, screenHeight / 2 + 30, 20, DARKGREEN);
		}
		EndDrawing();
	}

	CloseWindow();

	return 0;
}
