/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"

#include "resource_dir.h" // utility header for SearchAndSetResourceDir

#include "player.h"

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

	while (!WindowShouldClose()) // 游戏主循环
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
		DrawText("Naruto MVP", 10, 10, 20, DARKGRAY);

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
