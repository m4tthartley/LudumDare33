
#include "ld_lib.h"
#include <math.h>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

f32 RandomFloat ()
{
	f32 Result = (f32)(rand()%1000) / 1000.0f;
	return Result;
}

s32 RandomInt (s32 Max)
{
	s32 Result = rand()%Max;
	return Result;
}

f32 VLength (f32 X, f32 Y)
{
	f32 Result = sqrt((X*X)+(Y*Y));
	return Result;
}

f32 Radians (f32 Degrees)
{
	return (3.14f / 180.0f) * Degrees;
}

f32 VDistance (f32 AX, f32 AY, f32 BX, f32 BY)
{
	f32 XDis = BX - AX;
	f32 YDis = BY - AY;
	if (XDis < 0)
	{
		XDis *= -1;
	}
	if (YDis < 0)
	{
		YDis *= -1;
	}
	return VLength(XDis, YDis);
}

#define FOR(Count) for (u32 I = 0; I < Count; I++)
//#define PRINTTOBUFFER(Buffer) 

#define ASSET_PATH "../src/assets/"
ld_texture T_Player;
ld_sprite S_Player[8];
ld_sprite PlayerShadow;
ld_texture T_Tree;
ld_sprite S_Tree;
ld_texture T_Tiles;
ld_sprite S_Tiles[4];

#define CHAR_COUNT 26
char *Characters = "abcdefghijklmnopqrstuvwxyz0123456789'.-";
ld_sprite S_Font[64];

s32 GetCharIndex (char Char, char *String)
{
	u32 CharIndex = 0;
	while (String[CharIndex])
	{
		if (Char == String[CharIndex])
		{
			return CharIndex;
		}

		++CharIndex;
	}

	return -1;
}

void DrawFontColor (char *String, f32 XPos, f32 YPos, f32 Size, color Color);

void DrawFont (char *String, f32 XPos, f32 YPos, f32 Size)
{
	/*u32 CharIndex = 0;
	while (String[CharIndex])
	{
		s32 SpriteIndex = GetCharIndex(String[CharIndex], Characters);
		if (SpriteIndex != -1)
		{
			LD_RDrawSprite(S_Font[SpriteIndex], XPos + (CharIndex*(8*Size)), YPos, Size);
		}

		++CharIndex;
	}*/

	DrawFontColor(String, XPos, YPos, Size, (color){1.0f, 1.0f, 1.0f, 1.0f});
}

void DrawFontColor (char *String, f32 XPos, f32 YPos, f32 Size, color Color)
{
	u32 CharIndex = 0;
	u32 Row = 0;
	u32 Column = 0;
	while (String[CharIndex])
	{
		if (String[CharIndex] == '\n')
		{
			++Row;
			++CharIndex;
			Column = 0;
			continue;
		}

		s32 SpriteIndex = GetCharIndex(String[CharIndex], Characters);
		if (SpriteIndex != -1)
		{
			LD_RDrawSpriteWithColor(S_Font[SpriteIndex], XPos + (Column*(8*Size)), YPos+(Row*8*Size), Size, Color);
		}

		++CharIndex;
		++Column;
	}
}

typedef struct
{
	f32 XPos;
	f32 YPos;
	color Color;
	u32 SpriteIndex;
} tile;

#define WORLD_SIZE 32
#define TILE_SIZE 16*4
#define TREE_COUNT 64
#define LOG_COUNT 256
#define FIRE_PARTICLE_COUNT 256

tile World[WORLD_SIZE*WORLD_SIZE];

void NewSprite (ld_sprite *Sprite, ld_texture Texture, s32 XOffset, s32 YOffset, s32 Width, s32 Height)
{
	Sprite->Texture = Texture;
	Sprite->XOffset = XOffset;
	Sprite->YOffset = YOffset;
	Sprite->Width = Width;
	Sprite->Height = Height;
}

typedef struct
{
	b32 Active;
	f32 XPos;
	f32 YPos;
	s32 Life;
	s32 MessageIndex;
	s32 MessageTimer;
} tree;

typedef struct
{
	b32 Active;
	f32 XPos;
	f32 YPos;
	f32 AniCounter;
} tree_log;

typedef struct
{
	b32 Active;
	f32 XPos;
	f32 YPos;
	s32 SpriteIndex;
	f32 XSpeed;
	f32 YSpeed;
	s32 Life;
} leaf;

typedef struct
{
	b32 Active;
	f32 XPos;
	f32 YPos;
	f32 XSpeed;
	f32 YSpeed;
	f32 AniCounter;
	f32 Life;
} fire_particle;

void AddLog (tree_log *Logs, f32 XPos, f32 YPos)
{
	FOR(LOG_COUNT)
	{
		if (!Logs[I].Active)
		{
			Logs[I].Active = TRUE;
			Logs[I].XPos = XPos;
			Logs[I].YPos = YPos;
			break;
		}
	}
}

void AddLeaf (leaf *Leaves, f32 XPos, f32 YPos)
{
	FOR(LOG_COUNT)
	{
		if (!Leaves[I].Active)
		{
			Leaves[I].Active = TRUE;
			Leaves[I].XPos = XPos;
			Leaves[I].YPos = YPos;
			Leaves[I].SpriteIndex = RandomInt(2);
			Leaves[I].XSpeed = 0.0f;
			Leaves[I].YSpeed = 0.0f;
			Leaves[I].Life = 120;
			break;
		}
	}
}

void AddFire (fire_particle *Fire, f32 XPos, f32 YPos, f32 XSpeed, f32 YSpeed)
{
	FOR(FIRE_PARTICLE_COUNT)
	{
		if (!Fire[I].Active)
		{
			/*b32 Active;
			f32 XPos;
			f32 YPos;
			f32 XSpeed;
			f32 YSpeed;
			f32 AniCounter;
			f32 Life;*/

			Fire[I].Active = TRUE;
			Fire[I].XPos = XPos;
			Fire[I].YPos = YPos;
			Fire[I].XSpeed = XSpeed;
			Fire[I].YSpeed = YSpeed;
			Fire[I].AniCounter = 0.0f;
			Fire[I].Life = 60;
			break;
		}
	}
}

void AddLogs (tree_log *Logs, f32 XPos, f32 YPos, s32 Count)
{
	FOR(Count)
	{
		AddLog(Logs, XPos+(RandomFloat()*128.0f - 64.0f), YPos+(RandomFloat()*128.0f - 64.0f));
	}
}

typedef struct
{
	f32 XPos;
	f32 YPos;
	f32 AniCounter;
	b32 Flip;
	b32 Chopping;
	s32 ChopTimer;
	s32 Logs;
	f32 Life;
} player;

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow
)
{
	ld_window Window;
	LD_CreateWindow(&Window, WINDOW_WIDTH, WINDOW_HEIGHT, "LudumDare 33");

	// Load assets
	LD_LoadBitmap(&T_Player, ASSET_PATH"player.bmp");
	LD_LoadBitmap(&T_Tree, ASSET_PATH"trees.bmp");
	LD_LoadBitmap(&T_Tiles, ASSET_PATH"tiles.bmp");
	ld_texture T_Font;
	LD_LoadBitmap(&T_Font, ASSET_PATH"font.bmp");

	sound_asset SFX_Chop;
	sound_asset SFX_Pickup;
	LD_LoadWav(&SFX_Chop, ASSET_PATH"chop.wav");
	LD_LoadWav(&SFX_Pickup, ASSET_PATH"pickup.wav");

	u32 CharSpriteIndex = 0;
	while (Characters[CharSpriteIndex])
	{
		NewSprite(&S_Font[CharSpriteIndex], T_Font, (CharSpriteIndex%8)*8, (CharSpriteIndex/8)*8, 8, 8);
		++CharSpriteIndex;
	}

	ld_sprite S_TextBox;
	NewSprite(&S_TextBox, T_Font, 0, 56, 8*5, 8*3);

	NewSprite(&S_Tree, T_Tree, 96, 0, 96, 96);
	ld_sprite S_Log;
	NewSprite(&S_Log, T_Tree, 0, 96, 32, 32);
	ld_sprite S_Leaves[2];
	NewSprite(&S_Leaves[0], T_Tree, 32, 96, 32, 32);
	NewSprite(&S_Leaves[1], T_Tree, 64, 96, 32, 32);
	ld_sprite S_Fire[3];
	NewSprite(&S_Fire[0], T_Player, 32, 64, 32, 32);
	NewSprite(&S_Fire[1], T_Player, 64, 64, 32, 32);
	NewSprite(&S_Fire[2], T_Player, 96, 64, 32, 32);

	for (u32 I = 0;
		I < 8;
		I++)
	{
		NewSprite(&S_Player[I], T_Player, (I%4)*32, (I/4)*32, 32, 32);
	}
	NewSprite(&PlayerShadow, T_Player, 0, 64, 32, 32);

	for (u32 I = 0;
		I < 4;
		I++)
	{
		NewSprite(&S_Tiles[I], T_Tiles, I*16, 0, 16, 16);
	}

#define MESSAGE_COUNT 6
	char *TreeMessages[] = { "you\nmonster!", "please\ndon't\nkill us", "he's\nevil", "we did\nnothing\nto you", "we are\npeaceful",
   							 "i hope\nyou trip\nand fall\non your\naxe"};

	s32 Score = 0;

	// Entities
	player Player = {0};
	Player.XPos = 4*TILE_SIZE;
	Player.YPos = 4*TILE_SIZE;
	Player.Life = 100.0f;

	tree Trees[TREE_COUNT] = {0};
	tree_log Logs[LOG_COUNT] = {0};
	leaf Leaves[LOG_COUNT] = {0};

	for (u32 Index = 0;
		Index < WORLD_SIZE*WORLD_SIZE;
		Index++)
	{
		//World[Index].Color = (color){0.0f, 0.8f+(RandomFloat()*0.2f), 0.0f, 1.0f};
		World[Index].SpriteIndex = RandomInt(4);
	}

	FOR(TREE_COUNT)
	{
		Trees[I].XPos = (RandomInt(WORLD_SIZE)*TILE_SIZE)+(TILE_SIZE/2);
		Trees[I].YPos = (RandomInt(WORLD_SIZE)*TILE_SIZE)+(TILE_SIZE/2);
		Trees[I].Life = 3;
		Trees[I].Active = TRUE;
		Trees[I].MessageTimer = RandomInt(240*3);
	}

	fire_particle Fire[FIRE_PARTICLE_COUNT] = {0};

	f32 WorldOffsetX = 0.0f;
	f32 WorldOffsetY = 0.0f;

	color Purple = (color){1.0f, 0.0f, 1.0f, 1.0f};
	color Red = (color){1.0f, 0.0f, 0.0f, 1.0f};
	color Black = (color){0.0f, 0.0f, 0.0f, 1.0f};

	// Render loop
	while (Window.Alive)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		WorldOffsetX = -(Player.XPos) +(WINDOW_WIDTH/2);
		WorldOffsetY = -(Player.YPos) +(WINDOW_HEIGHT/2);
		if (WorldOffsetX > 0.0f)
		{
			WorldOffsetX = 0.0f;
		}
		if (WorldOffsetY > 0.0f)
		{
			WorldOffsetY = 0.0f;
		}
		if (WorldOffsetX < -(WORLD_SIZE*TILE_SIZE-WINDOW_WIDTH))
		{
			WorldOffsetX = -(WORLD_SIZE*TILE_SIZE-WINDOW_WIDTH);
		}
		if (WorldOffsetY < -(WORLD_SIZE*TILE_SIZE-WINDOW_HEIGHT))
		{
			WorldOffsetY = -(WORLD_SIZE*TILE_SIZE-WINDOW_HEIGHT);
		}

		//WorldOffsetX = (s32)WorldOffsetX;
		//WorldOffsetY = (s32)WorldOffsetY;

		--Player.ChopTimer;

		// Chopping
		Player.Chopping = FALSE;
		if (Player.ChopTimer < 1 && Keys & Key_X)
		{
			Player.Chopping = TRUE;
			Player.ChopTimer = 60;
		}

		// Flame thrower
		if (Keys & Key_X)
		{
			AddFire(Fire, Player.XPos, Player.YPos, 10.0f, RandomFloat()*2.0f - 1.0f);
		}

		for (u32 Y = 0;
			Y < WORLD_SIZE;
			Y++)
		{
			for (u32 X = 0;
				X < WORLD_SIZE;
				X++)
			{
				//LD_RDrawQuad(X*TILE_SIZE, Y*TILE_SIZE, TILE_SIZE, TILE_SIZE, World[Y*WORLD_SIZE+X].Color);
				LD_RDrawSprite(S_Tiles[World[Y*WORLD_SIZE+X].SpriteIndex], X*TILE_SIZE + WorldOffsetX, Y*TILE_SIZE + WorldOffsetY, 4);
			}
		}

		// Trees
		FOR(TREE_COUNT)
		{
			if (Trees[I].Active)
			{
				if (VDistance(Player.XPos, Player.YPos, Trees[I].XPos, Trees[I].YPos) < 512)
				{
					--Trees[I].MessageTimer;
					if (Trees[I].MessageTimer < 1)
					{
						Trees[I].MessageTimer = 240*4;
						Trees[I].MessageIndex = RandomInt(MESSAGE_COUNT);
					}
				}

				if (Score >= 5)
				{
					if (VDistance(Player.XPos, Player.YPos, Trees[I].XPos, Trees[I].YPos) < 256)
					{
						AddLeaf(Leaves, Trees[I].XPos+(RandomFloat()*128.0f - 64.0f), Trees[I].YPos+(RandomFloat()*128.0f - 64.0f));
					}
				}
				if (Score >= 6)
				{
					if (VDistance(Player.XPos, Player.YPos, Trees[I].XPos, Trees[I].YPos) < 1024)
					{
						f32 Angle = (atan2( Trees[I].XPos-Player.XPos, (-Trees[I].YPos)-(-Player.YPos) ));
						Angle *= (180.0f / 3.14f);
						f32 NormalX = sin(Radians(Angle));
						f32 NormalY = cos(Radians(Angle+180.0f));

						Trees[I].XPos -= NormalX;
						Trees[I].YPos -= NormalY;
					}
				}

				if (VDistance(Player.XPos, Player.YPos, Trees[I].XPos, Trees[I].YPos) < 128)
				{
					if (Player.Chopping)
					{
						LD_PlaySound(SFX_Chop);

						--Trees[I].Life;
						if (Trees[I].Life < 1)
						{
							++Score;
							AddLogs(Logs, Trees[I].XPos, Trees[I].YPos, 4);
							Trees[I].Active = FALSE;
						}
					}
				}

				f32 XOffset = -188;
				f32 YOffset = -360;
				f32 X = Trees[I].XPos + WorldOffsetX;
				f32 Y = Trees[I].YPos + WorldOffsetY;
				LD_RDrawSprite(S_Tree, X+XOffset, Y+YOffset, 4);
				LD_RDrawQuad((Trees[I].XPos-2) + WorldOffsetX, (Trees[I].YPos-2) + WorldOffsetY, 4, 4, Purple);

				if (Trees[I].MessageTimer > 240*3)
				{
					LD_RDrawSprite(S_TextBox, X, Y-96, 4);
					f32 TextX = X+32;
					f32 TextY = Y+4-96;
					//DrawFontColor("you\nmonster", TextX, TextY, 2, Black);
					DrawFontColor(TreeMessages[Trees[I].MessageIndex], TextX, TextY, 2, Black);
				}
			}
		}

		// Logs
		FOR(LOG_COUNT)
		{
			if (Logs[I].Active)
			{
				if (VDistance(Player.XPos, Player.YPos, Logs[I].XPos, Logs[I].YPos) < 32)
				{
					++Player.Logs;
					Player.Life += 5.0f;
					Logs[I].Active = FALSE;
					LD_PlaySound(SFX_Pickup);
				}

				Logs[I].AniCounter += 0.02f;
				if (Logs[I].AniCounter > 3.14f)
				{
					Logs[I].AniCounter -= 3.14f;
				}
				LD_RDrawSprite(S_Log, Logs[I].XPos + WorldOffsetX, Logs[I].YPos+(sinf(Logs[I].AniCounter)*10.0f) + WorldOffsetY, 4);
				LD_RDrawQuad((Logs[I].XPos-2) + WorldOffsetX, (Logs[I].YPos-2) + WorldOffsetY, 4, 4, Purple);
			}
		}

		// Leaves
		FOR(LOG_COUNT)
		{
			if (Leaves[I].Active)
			{
				--Leaves[I].Life;
				if (Leaves[I].Life < 1)
				{
					Leaves[I].Active = FALSE;
				}

				f32 Angle = (atan2( Leaves[I].XPos-Player.XPos, (-Leaves[I].YPos)-(-Player.YPos) ));
				Angle *= (180.0f / 3.14f);
				f32 NormalX = sin(Radians(Angle));
				f32 NormalY = cos(Radians(Angle+180.0f));

				if (VDistance(Player.XPos, Player.YPos, Leaves[I].XPos, Leaves[I].YPos) < 32)
				{
					Player.XPos -= NormalX * 0.5f;
					Player.YPos -= NormalY * 0.5f;
					Player.Life -= 0.05f;
					Leaves[I].Active = FALSE;
				}

				Leaves[I].XSpeed -= NormalX * 0.05f;
				Leaves[I].YSpeed -= NormalY * 0.05f;

				Leaves[I].XPos += Leaves[I].XSpeed;
				Leaves[I].YPos += Leaves[I].YSpeed;

				LD_RDrawSprite(S_Leaves[Leaves[I].SpriteIndex], Leaves[I].XPos + WorldOffsetX, Leaves[I].YPos + WorldOffsetY, 4);
			}
		}

		// Fire
		FOR(FIRE_PARTICLE_COUNT)
		{
			if (Fire[I].Active)
			{
				--Fire[I].Life;
				if (Fire[I].Life < 1)
				{
					Fire[I].Active = FALSE;
				}

				Fire[I].XSpeed *= 0.99f;
				Fire[I].YSpeed *= 0.99f;
				//Fire[I].XSpeed -= 0.1f;
				//Fire[I].YSpeed -= 0.1f;

				Fire[I].XPos += Fire[I].XSpeed;
				Fire[I].YPos += Fire[I].YSpeed;

				Fire[I].AniCounter += 0.1f;
				if (Fire[I].AniCounter >= 3.0f)
				{
					Fire[I].AniCounter = 0.0f;
				}
					
				LD_RDrawSprite(S_Fire[(s32)Fire[I].AniCounter], Fire[I].XPos, Fire[I].YPos, 4);
			}
		}

		// Player
		f32 MoveX = 0.0f;
		f32 MoveY = 0.0f;
		if (Keys & Key_Right)
		{
			MoveX += 1.0f;
			Player.Flip = FALSE;
		}
		if (Keys & Key_Left)
		{
			MoveX -= 1.0f;
			Player.Flip = TRUE;
		}
		if (Keys & Key_Down)
		{
			MoveY += 1.0f;
		}
		if (Keys & Key_Up)
		{
			MoveY -= 1.0f;
		}

		f32 LengthX = 0.0f;
		if (MoveX != 0)
		{
			LengthX = MoveX/sqrt((MoveX*MoveX)+(MoveY*MoveY));
		}
		f32 LengthY = 0.0f;
		if (MoveY != 0)
		{
			LengthY = MoveY/sqrt((MoveX*MoveX)+(MoveY*MoveY));
		}

		//f32 Length = sqrt((MoveX*MoveX)+(MoveY*MoveY));
		/*char String[64];
		sprintf(String, "Length: %f\n", LengthX);
		OutputDebugString(String);*/

		Player.XPos += (LengthX)*3.0f;
		Player.YPos += (LengthY)*3.0f;

		if (Player.XPos < 16.0f)
		{
			Player.XPos = 16.0f;
		}
		if (Player.YPos < 64.0f)
		{
			Player.YPos = 64.0f;
		}
		if (Player.XPos > WORLD_SIZE*TILE_SIZE - 16.0f)
		{
			Player.XPos = WORLD_SIZE*TILE_SIZE - 16.0f;
		}
		if (Player.YPos > WORLD_SIZE*TILE_SIZE)
		{
			Player.YPos = WORLD_SIZE*TILE_SIZE;
		}

		Player.AniCounter += 0.1f;
		if (Player.AniCounter >= 4.0f)
		{
			Player.AniCounter = 0.0f;
		}
		LD_RDrawQuad((Player.XPos-2) + WorldOffsetX, (Player.YPos-2) + WorldOffsetY, 4, 4, Purple);
		f32 XOffset = -18;
		f32 YOffset = -74;
		LD_RDrawSprite(PlayerShadow, Player.XPos+XOffset + WorldOffsetX, Player.YPos+YOffset + WorldOffsetY, 4);
		s32 SpriteOffset = 0;
		if(Player.Flip)
		{
			SpriteOffset = 4;
		}
		LD_RDrawSprite(S_Player[(s32)Player.AniCounter+SpriteOffset], Player.XPos+XOffset + WorldOffsetX, Player.YPos+YOffset + WorldOffsetY, 4);

		// Gui
		char GuiString[64];
		sprintf(GuiString, "logs %i\n"
						   "life %f\n"
						   "tree deaths %i", Player.Logs, Player.Life, Score);
		DrawFont(GuiString, 10, 10, 2);
		//DrawFontColor("hello world!", 10, 10+32, 4, Black);
		//DrawFont("the quick brown fox jumped over the lazy dog!", 10, 10+32+32, 1);
		DrawFont("you are the monster", 10, 10+32+32+32, 8);

		LD_UpdateWindow(&Window);
	}

	return 0;
}
