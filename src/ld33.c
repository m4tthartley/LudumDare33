
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

#if BUILD_RELEASE
#define ASSET_PATH "assets/"
#else
#define ASSET_PATH "../src/assets/"
#endif
ld_texture T_Player;
ld_sprite S_Player[8];
ld_sprite PlayerShadow;
ld_texture T_Tree;
ld_sprite S_Tree[2];
ld_texture T_Tiles;
ld_sprite S_Tiles[4];

//#define CHAR_COUNT 26
char *Characters = "abcdefghijklmnopqrstuvwxyz0123456789'.-!";
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

void DrawFontColorWithBackground (char *String, f32 XPos, f32 YPos, f32 Size, color Color)
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

		LD_RDrawQuad(XPos + (Column*(8*Size)), YPos+(Row*8*Size), 18, 18, (color){0.0f, 0.0f, 0.0f, 1.0f});

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

#define WORLD_SIZE 64
#define TILE_SIZE 16*4
#define TREE_COUNT 40
#define LOG_COUNT 256
#define FIRE_PARTICLE_COUNT 256
#define Y_TO_Z 1000

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
	f32 XSpeed;
	f32 YSpeed;
	f32 AniCounter;
	f32 Life;
} fire_particle;

typedef struct
{
	b32 Active;
	f32 XPos;
	f32 YPos;
	f32 Life;
	s32 SpriteIndex;
	s32 MessageIndex;
	s32 MessageTimer;
	b32 OnFire;
	fire_particle Fires[32];
	s32 FireParticleIndex;
	s32 FireResistance;
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
			Leaves[I].SpriteIndex = RandomInt(6);
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
	b32 ActuallyChop;
	s32 ChopTimer;
	s32 Logs;
	f32 Life;
	f32 AxeAni;
	b32 FlameThrower;
	s32 HitSoundTimer;
	b32 HitFlash;
} player;

#define RENDER_DEBUG_DOTS 0

#define TREES_ATTACK_SCORE 3
#define TREES_MOVE_SCORE 10

// int CALLBACK WinMain(
// 	HINSTANCE hInstance,
// 	HINSTANCE hPrevInstance,
// 	LPSTR     lpCmdLine,
// 	int       nCmdShow
// )
ProgramEntryPoint()
{
	srand(33);

	ld_window Window;
	LD_CreateWindow(&Window, WINDOW_WIDTH, WINDOW_HEIGHT, "Lumberjack monster");

	// Load assets
	LD_LoadBitmap(&T_Player, ASSET_PATH"player.bmp");
	LD_LoadBitmap(&T_Tree, ASSET_PATH"trees.bmp");
	LD_LoadBitmap(&T_Tiles, ASSET_PATH"tiles.bmp");
	ld_texture T_Font;
	LD_LoadBitmap(&T_Font, ASSET_PATH"font.bmp");
	ld_texture T_Sprites;
	LD_LoadBitmap(&T_Sprites, ASSET_PATH"sprites.bmp");

	sound_asset SFX_Chop;
	sound_asset SFX_Pickup;
	sound_asset SFX_Fire;
	sound_asset SFX_Hit;
	sound_asset SFX_Powerup;
	LD_LoadWav(&SFX_Chop, ASSET_PATH"chop.wav");
	LD_LoadWav(&SFX_Pickup, ASSET_PATH"pickup.wav");
	LD_LoadWav(&SFX_Fire, ASSET_PATH"fire.wav");
	LD_LoadWav(&SFX_Hit, ASSET_PATH"hit.wav");
	LD_LoadWav(&SFX_Powerup, ASSET_PATH"powerup.wav");

	u32 CharSpriteIndex = 0;
	while (Characters[CharSpriteIndex])
	{
		NewSprite(&S_Font[CharSpriteIndex], T_Font, (CharSpriteIndex%8)*8, (CharSpriteIndex/8)*8, 8, 8);
		++CharSpriteIndex;
	}

	ld_sprite S_TextBox;
	NewSprite(&S_TextBox, T_Font, 0, 56, 8*5, 8*3);

	NewSprite(&S_Tree[0], T_Tree, 0, 0, 96, 96);
	NewSprite(&S_Tree[1], T_Tree, 96, 0, 96, 96);

	ld_sprite S_Log;
	NewSprite(&S_Log, T_Sprites, 0, 0, 16, 16);

	ld_sprite S_Leaves[6];
	NewSprite(&S_Leaves[0], T_Sprites, 0, 32, 16, 16);
	NewSprite(&S_Leaves[1], T_Sprites, 16, 32, 16, 16);
	NewSprite(&S_Leaves[2], T_Sprites, 32, 32, 16, 16);
	NewSprite(&S_Leaves[3], T_Sprites, 48, 32, 16, 16);
	NewSprite(&S_Leaves[4], T_Sprites, 0, 48, 16, 16);
	NewSprite(&S_Leaves[5], T_Sprites, 16, 48, 16, 16);

	ld_sprite S_Fire[4];
	NewSprite(&S_Fire[0], T_Sprites, 0, 16, 16, 16);
	NewSprite(&S_Fire[1], T_Sprites, 16, 16, 16, 16);
	NewSprite(&S_Fire[2], T_Sprites, 32, 16, 16, 16);
	NewSprite(&S_Fire[3], T_Sprites, 48, 16, 16, 16);

	ld_sprite S_PlayerChop[6];

	for (u32 I = 0;
		I < 8;
		I++)
	{
		NewSprite(&S_Player[I], T_Player, (I%4)*32, (I/4)*32, 32, 32);
	}
	NewSprite(&PlayerShadow, T_Player, 0, 64, 32, 32);
	NewSprite(&S_PlayerChop[0], T_Player, 0, 96, 32, 32);
	NewSprite(&S_PlayerChop[1], T_Player, 32, 96, 32, 32);
	NewSprite(&S_PlayerChop[2], T_Player, 64, 96, 32, 32);
	NewSprite(&S_PlayerChop[3], T_Player, 0, 128, 32, 32);
	NewSprite(&S_PlayerChop[4], T_Player, 32, 128, 32, 32);
	NewSprite(&S_PlayerChop[5], T_Player, 64, 128, 32, 32);

	ld_sprite S_FlameThrower[2];
	NewSprite(&S_FlameThrower[0], T_Player, 32, 64, 32, 32);
	NewSprite(&S_FlameThrower[1], T_Player, 64, 64, 32, 32);

	ld_sprite S_FlameThrowerItem;
	NewSprite(&S_FlameThrowerItem, T_Sprites, 16, 0, 16, 16);

	ld_sprite TreeShadow;
	NewSprite(&TreeShadow, T_Tree, 288, 0, 96, 96);

	ld_sprite S_TreeFaces[2];
	NewSprite(&S_TreeFaces[0], T_Sprites, 32, 48, 16, 16);
	NewSprite(&S_TreeFaces[1], T_Sprites, 48, 48, 16, 16);

	for (u32 I = 0;
		I < 4;
		I++)
	{
		NewSprite(&S_Tiles[I], T_Tiles, I*(16), 0, 16, 16);
	}

#define MESSAGE_COUNT 8
	char *TreeMessages[] = { "you monster!", "trees are people\ntoo you know", "he's evil", "i'll leaf you\nyou jerk", "we are innocent\ntrees",
   							 "i hope you trip and\nfall on your axe", "die!", "you make me sick!"};

	s32 Score = 0;
	b32 Dead = FALSE;
	b32 Win = FALSE;
	b32 FightingBackMessage = FALSE;
	s32 FightingBackMessageTimer = 240;

	// Entities
	player Player = {0};
	Player.XPos = (WORLD_SIZE/2)*TILE_SIZE;
	Player.YPos = (WORLD_SIZE/2)*TILE_SIZE;
	Player.Life = 100.0f;
	Player.FlameThrower = FALSE;

	b32 FlameThrowerItem = FALSE;
	f32 FlameThrowerItemXPos = Player.XPos - 100;
	f32 FlameThrowerItemYPos = Player.YPos;
	f32 FlameThrowerItemAniCounter = 0.0f;

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
		Trees[I].SpriteIndex = RandomInt(2);
		//Trees[I].OnFire = TRUE;
		Trees[I].FireResistance = 100;
	}

	fire_particle Fire[FIRE_PARTICLE_COUNT] = {0};

	f32 WorldOffsetX = 0.0f;
	f32 WorldOffsetY = 0.0f;

	color Purple = (color){1.0f, 0.0f, 1.0f, 1.0f};
	color Red = (color){1.0f, 0.0f, 0.0f, 1.0f};
	color Black = (color){0.0f, 0.0f, 0.0f, 1.0f};
	color White = (color){1.0f, 1.0f, 1.0f, 1.0f};

	s32 FireSFXTimerMax = 20;
	s32 FireSFXTimer = FireSFXTimerMax;

	//Score = 10;

	// Render loop
	b32 OnMenu = TRUE;
	b32 MenuButtonFlash = TRUE;
	s32 MenuButtonFlashTimer = 15;
	while (Window.Alive && OnMenu)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		DrawFont("lumberjack monster", 10, 10, 8);

		DrawFont("use the arrow keys to move", 10, 200, 2);
		DrawFont("press x to attack", 10, 200+24, 2);

		DrawFont("murder all the trees", 10, 200+24+48, 2);

		DrawFont("made by matt hartley", 10, 200+24+48+48, 2);
		DrawFont("for ludumdare 33", 10, 200+24+48+48+24, 2);

		--MenuButtonFlashTimer;
		if (MenuButtonFlashTimer <= 0)
		{
			MenuButtonFlashTimer = 15;
			if (MenuButtonFlash)
			{
				MenuButtonFlash = FALSE;
			}
			else
			{
				MenuButtonFlash = TRUE;
			}
		}
		if (MenuButtonFlash)
		{
			DrawFont("press x to start", WINDOW_WIDTH/2 - (8*16), 500, 2);
		}

		if (Keys & Key_X)
		{
			OnMenu = FALSE;
		}

		LD_UpdateWindow(&Window);
	}

	while (Window.Alive)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

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

		if (Player.ChopTimer > 0)
		{
			--Player.ChopTimer;
		}
		if (Player.HitSoundTimer > 0)
		{
			--Player.HitSoundTimer;
		}
		Player.HitFlash = FALSE;

		// Chopping
		//Player.Chopping = FALSE;
		if (Player.ChopTimer < 1 && !Player.Chopping && Keys & Key_X)
		{
			Player.Chopping = TRUE;
			Player.ChopTimer = 60;
		}

		// Flame thrower
		if (Player.FlameThrower && Keys & Key_X)
		{
			f32 XSpawnPos = 48;
			f32 XSpeed = 10.0f;
			if (Player.Flip)
			{
				XSpawnPos = -48;
				XSpeed = -10.0f;
			}
			AddFire(Fire, Player.XPos+4+XSpawnPos, Player.YPos-24, XSpeed, RandomFloat()*2.0f - 1.0f);
			--FireSFXTimer;
			if (FireSFXTimer < 1)
			{
				//LD_PlaySound(SFX_Fire);
				FireSFXTimer = FireSFXTimerMax;
			}
		}
		else
		{
			FireSFXTimer = 0;
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
				SetZIndex(0.0f);
				LD_RDrawSprite(S_Tiles[World[Y*WORLD_SIZE+X].SpriteIndex], (f32)(X*TILE_SIZE) + WorldOffsetX, (f32)(Y*TILE_SIZE) + WorldOffsetY, 4);
			}
		}

		// Trees
		FOR(TREE_COUNT)
		{
			if (Trees[I].Active)
			{
				if (Trees[I].Life < 1)
				{
					++Score;
					AddLogs(Logs, Trees[I].XPos, Trees[I].YPos, 4);
					Trees[I].Active = FALSE;
				}

				if (VDistance(Player.XPos, Player.YPos, Trees[I].XPos, Trees[I].YPos) < 512)
				{
					--Trees[I].MessageTimer;
					if (Trees[I].MessageTimer < 1)
					{
						Trees[I].MessageTimer = 240*4;
						Trees[I].MessageIndex = RandomInt(MESSAGE_COUNT);
					}
				}

				if (!Dead)
				{
					if (Score >= TREES_ATTACK_SCORE)
					{
						if (VDistance(Player.XPos, Player.YPos, Trees[I].XPos, Trees[I].YPos) < 384)
						{
							AddLeaf(Leaves, Trees[I].XPos+(RandomFloat()*128.0f - 64.0f), (Trees[I].YPos-128.0f)+(RandomFloat()*128.0f - 64.0f));
						}
					}
					if (Score >= TREES_MOVE_SCORE)
					{
						//if (VDistance(Player.XPos, Player.YPos, Trees[I].XPos, Trees[I].YPos) < 1024)
						{
							f32 Angle = (atan2( Trees[I].XPos-Player.XPos, (-Trees[I].YPos)-(-Player.YPos) ));
							Angle *= (180.0f / 3.14f);
							f32 NormalX = sin(Radians(Angle));
							f32 NormalY = cos(Radians(Angle+180.0f));

							Trees[I].XPos -= NormalX;
							Trees[I].YPos -= NormalY;
						}
					}
				}

				if (!Player.FlameThrower)
				{
					if (VDistance(Player.XPos, Player.YPos, Trees[I].XPos, Trees[I].YPos) < 128)
					{
						if (Player.ActuallyChop)
						{
							LD_PlaySound(SFX_Chop);

							--Trees[I].Life;
						}
					}
				}

				f32 X = Trees[I].XPos + WorldOffsetX;
				f32 Y = Trees[I].YPos + WorldOffsetY;
				SetZIndex((Trees[I].YPos / Y_TO_Z) + 0.01f);

				if (!Trees[I].OnFire)
				{
					for (s32 Index = 0;
						Index < FIRE_PARTICLE_COUNT;
						Index++)
					{
						if (Fire[Index].Active)
						{
							if (Fire[Index].XPos > Trees[I].XPos - 48.0f &&
								Fire[Index].XPos < Trees[I].XPos + 48.0f &&
								Fire[Index].YPos > (Trees[I].YPos-128.0f) - 128.0f &&
								Fire[Index].YPos < (Trees[I].YPos-128.0f) + 128.0f
									/*VDistance(Fire[Index].XPos, Fire[Index].YPos, Trees[I].XPos, Trees[I].YPos - 128.0f) < 64*/)
							{
								--Trees[I].FireResistance;
								if (Trees[I].FireResistance < 1)
								{
									Trees[I].OnFire = TRUE;
								}

								Fire[Index].Active = FALSE;
							}
						}
					}
				}

				if (Trees[I].OnFire)
				{
					++Trees[I].FireParticleIndex;
					if (Trees[I].FireParticleIndex >= 32)
					{
						Trees[I].FireParticleIndex = 0;
						Trees[I].Life -= 0.1f;
					}
					Trees[I].Fires[Trees[I].FireParticleIndex].XPos = (RandomFloat()*128.0f)-64.0f;
					Trees[I].Fires[Trees[I].FireParticleIndex].YPos = -(RandomFloat()*256.0f);
					Trees[I].Fires[Trees[I].FireParticleIndex].AniCounter = 0.0f;
					
					//AddFire(Fire, Trees[I].XPos, Trees[I].YPos, 0.0f, 0.0f);
					for (s32 Index = 0;
						Index < 32;
						Index++)
					{
						Trees[I].Fires[Index].AniCounter += 0.1f;
						/*if (Trees[I].Fires[Index].AniCounter >= 3.0f)
						{
							Trees[I].Fires[Index].AniCounter = 0.0f;
						}*/

						//f32 AniFrame = ((60.0f - (f32)Trees[I].Fires[Index].Life) / 60.0f) * 3.9f;

						if (Trees[I].Fires[Index].AniCounter < 3.0f)
						{
							LD_RDrawSprite(S_Fire[(s32)Trees[I].Fires[Index].AniCounter], X+Trees[I].Fires[Index].XPos, Y+Trees[I].Fires[Index].YPos, 4);
						}
					}
				}

				for (s32 Index = 0;
					Index < TREE_COUNT;
					Index++)
				{
					tree *Tree = &Trees[I];
					tree *OtherTree = &Trees[Index];

					if (I != Index && VDistance(Tree->XPos, Tree->YPos, OtherTree->XPos, OtherTree->YPos) < 128)
					{
						f32 Angle = (atan2( OtherTree->XPos-Tree->XPos, (-OtherTree->YPos)-(-Tree->YPos) ));
						Angle *= (180.0f / 3.14f);
						f32 XNormal = sin(Radians(Angle));
						f32 YNormal = cos(Radians(Angle+180.0f));

						OtherTree->XPos += (XNormal*1.0f);
						OtherTree->YPos += (YNormal*1.0f);
					}
				}

				f32 XOffset = -188;
				f32 YOffset = -360;
				SetZIndex(Trees[I].YPos / Y_TO_Z);
				s32 FaceIndex = 0;
				if (Score >= TREES_ATTACK_SCORE)
				{
					if (Score >= TREES_MOVE_SCORE)
					{
						FaceIndex = 1;
					}
					LD_RDrawSprite(S_TreeFaces[FaceIndex], X-12, Y-64, 4);
				}
				LD_RDrawSprite(S_Tree[Trees[I].SpriteIndex], X+XOffset, Y+YOffset, 4);
				SetZIndex((Trees[I].YPos / Y_TO_Z) - 1.0f);
				LD_RDrawSprite(TreeShadow, (X+XOffset)+4, (Y+YOffset)+40, 4);
#if RENDER_DEBUG_DOTS
				SetZIndex(10);
				LD_RDrawQuad((Trees[I].XPos-2) + WorldOffsetX, (Trees[I].YPos-2) + WorldOffsetY, 4, 4, Purple);
#endif
				/*char String[64];
				sprintf(String, "res %i", Trees[I].FireResistance);
				DrawFont(String, X+XOffset, Y+YOffset, 2);*/

				if (Score >= TREES_ATTACK_SCORE && Trees[I].MessageTimer > 240*3)
				{
					SetZIndex((Trees[I].YPos / Y_TO_Z)+0.01f);
					//LD_RDrawSprite(S_TextBox, X, Y-96, 4);
					f32 TextX = X+32;
					f32 TextY = Y+4-64;
					//DrawFontColor("you\nmonster", TextX, TextY, 2, Black);
					DrawFontColorWithBackground(TreeMessages[Trees[I].MessageIndex], TextX, TextY, 2, White);
				}
			}
		}

		// Logs
		FOR(LOG_COUNT)
		{
			if (Logs[I].Active)
			{
				if (Player.Life < 100 && VDistance(Player.XPos, Player.YPos, Logs[I].XPos, Logs[I].YPos) < 32)
				{
					++Player.Logs;
					Player.Life += 1.0f;
					Logs[I].Active = FALSE;
					LD_PlaySound(SFX_Pickup);
				}

				Logs[I].AniCounter += 0.02f;
				if (Logs[I].AniCounter > 3.14f)
				{
					Logs[I].AniCounter -= 3.14f;
				}
				SetZIndex(Logs[I].YPos / Y_TO_Z);
				LD_RDrawSprite(S_Log, (Logs[I].XPos-20) + WorldOffsetX, (Logs[I].YPos-20)+(sinf(Logs[I].AniCounter)*10.0f) + WorldOffsetY, 4);
#if RENDER_DEBUG_DOTS
				SetZIndex(10);
				LD_RDrawQuad((Logs[I].XPos-2) + WorldOffsetX, (Logs[I].YPos-2) + WorldOffsetY, 4, 4, Purple);
#endif
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

				if (!Dead && VDistance(Player.XPos, Player.YPos, Leaves[I].XPos, Leaves[I].YPos) < 16)
				{
					Player.XPos -= NormalX * 2.0f;
					Player.YPos -= NormalY * 2.0f;
					Player.Life -= 1.0f;
					if (Player.HitSoundTimer < 1)
					{
						Player.HitSoundTimer = 30;
						LD_PlaySound(SFX_Hit);
					}
					if (Player.HitSoundTimer > 10)
					{
						Player.HitFlash = TRUE;
					}
					Leaves[I].Active = FALSE;
				}

				Leaves[I].XSpeed -= NormalX * 0.05f;
				Leaves[I].YSpeed -= NormalY * 0.05f;

				Leaves[I].XPos += Leaves[I].XSpeed;
				Leaves[I].YPos += Leaves[I].YSpeed;

				SetZIndex(Leaves[I].YPos / Y_TO_Z);
				LD_RDrawSprite(S_Leaves[Leaves[I].SpriteIndex], (Leaves[I].XPos-12) + WorldOffsetX, (Leaves[I].YPos-12) + WorldOffsetY, 4);

#if RENDER_DEBUG_DOTS
				SetZIndex(10);
				LD_RDrawQuad((Leaves[I].XPos-2) + WorldOffsetX, (Leaves[I].YPos-2) + WorldOffsetY, 4, 4, Purple);
#endif
			}
		}

		// Fire
		FOR(FIRE_PARTICLE_COUNT)
		{
			if (Fire[I].Active)
			{
				Fire[I].Life -= 2;
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

				f32 AniFrame = ((60.0f - (f32)Fire[I].Life) / 60.0f) * 3.9f;
					
				SetZIndex(Fire[I].YPos / Y_TO_Z);
				LD_RDrawSprite(S_Fire[(s32)AniFrame], (Fire[I].XPos-16) + WorldOffsetX, (Fire[I].YPos-20) + WorldOffsetY, 4);

#if RENDER_DEBUG_DOTS
				SetZIndex(10);
				LD_RDrawQuad((Fire[I].XPos-2) + WorldOffsetX, (Fire[I].YPos-2) + WorldOffsetY, 4, 4, Purple);
#endif
			}
		}

		// Flame thrower item
		if (FlameThrowerItem)
		{
			if (VDistance(Player.XPos, Player.YPos, FlameThrowerItemXPos, FlameThrowerItemYPos) < 32)
			{
				FlameThrowerItem = FALSE;
				Player.FlameThrower = TRUE;
				LD_PlaySound(SFX_Pickup);
			}

			FlameThrowerItemAniCounter += 0.02f;
			if (FlameThrowerItemAniCounter > 3.14f)
			{
				FlameThrowerItemAniCounter -= 3.14f;
			}
			SetZIndex(FlameThrowerItemYPos / Y_TO_Z);
			LD_RDrawSprite(S_FlameThrowerItem, (FlameThrowerItemXPos-20) + WorldOffsetX, (FlameThrowerItemYPos-20)+(sinf(FlameThrowerItemAniCounter)*10.0f) + WorldOffsetY, 4);
			
			SetZIndex(10);
			DrawFontColor("flame thrower", (FlameThrowerItemXPos-96) + WorldOffsetX, (FlameThrowerItemYPos-40)+(sinf(FlameThrowerItemAniCounter)*10.0f) + WorldOffsetY, 2, Black);
#if RENDER_DEBUG_DOTS
			LD_RDrawQuad((FlameThrowerItemXPos-2) + WorldOffsetX, (FlameThrowerItemYPos-2) + WorldOffsetY, 4, 4, Purple);
#endif
		}

		// Player
		if (!Dead)
		{
			if (Score >= TREE_COUNT)
			{
				Win = TRUE;
			}

			if (Player.Life <= 0.0f)
			{
				Dead = TRUE;
			}

			if (Score >= /*1*/TREES_MOVE_SCORE && !Player.FlameThrower && !FlameThrowerItem)
			{
				FlameThrowerItem = TRUE;
				LD_PlaySound(SFX_Powerup);
				s32 XDir = 1;
				s32 YDir = 1;
				if (RandomInt(2) == 0)
				{
					XDir = -1;
				}
				if (RandomInt(2) == 0)
				{
					YDir = -1;
				}
				FlameThrowerItemXPos = Player.XPos + ((RandomFloat()*128.0f + 64.0f) * XDir);
				FlameThrowerItemYPos = Player.YPos + ((RandomFloat()*128.0f + 64.0f) * YDir);

				if (FlameThrowerItemXPos < 0)
				{
					FlameThrowerItemXPos = 64;
				}
				if (FlameThrowerItemXPos > WORLD_SIZE*TILE_SIZE)
				{
					FlameThrowerItemXPos = WORLD_SIZE*TILE_SIZE-64;
				}
				if (FlameThrowerItemYPos < 0)
				{
					FlameThrowerItemYPos = 64;
				}
				if (FlameThrowerItemYPos > WORLD_SIZE*TILE_SIZE)
				{
					FlameThrowerItemYPos = WORLD_SIZE*TILE_SIZE-64;
				}
			}

			f32 MoveX = 0.0f;
			f32 MoveY = 0.0f;
			if (Keys & Key_Right)
			{
				MoveX += 1.0f;
				if (!Player.FlameThrower || !(Keys & Key_X))
				{
					Player.Flip = FALSE;
				}
			}
			if (Keys & Key_Left)
			{
				MoveX -= 1.0f;
				if (!Player.FlameThrower || !(Keys & Key_X))
				{
					Player.Flip = TRUE;
				}
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

			if (MoveX || MoveY)
			{
				Player.AniCounter += 0.1f;
			}
			else
			{
				Player.AniCounter = 0.0f;
			}
			if (Player.AniCounter >= 4.0f)
			{
				Player.AniCounter = 0.0f;
			}

			if (Player.Chopping)
			{
				Player.AxeAni += 0.2f;
			}
			if (Player.AxeAni >= 3.0f)
			{
				Player.AxeAni = 0.0f;
				Player.Chopping = FALSE;
			}

			Player.ActuallyChop = FALSE;
			if (Player.AxeAni > 1.9f && Player.AxeAni < 2.1f)
			{
				Player.ActuallyChop = TRUE;
			}
			//PRINT("AxeAni %f", Player.AxeAni);

			SetZIndex(Player.YPos / Y_TO_Z);
			//PRINT("%f", Player.YPos / Y_TO_Z);
			
			f32 XOffset = -34;
			f32 YOffset = -74;
			s32 SpriteOffset = 0;
			s32 AxeSpriteOffset = 0;
			s32 FlameThrowerIndex = 0;
			if(Player.Flip)
			{
				SpriteOffset = 4;
				AxeSpriteOffset = 3;
				FlameThrowerIndex = 1;
			}
			if (Player.FlameThrower)
			{
				LD_RDrawSprite(S_FlameThrower[FlameThrowerIndex], Player.XPos+XOffset + WorldOffsetX, Player.YPos+YOffset + WorldOffsetY, 4);
			}
			else
			{
				LD_RDrawSprite(S_PlayerChop[(s32)Player.AxeAni+AxeSpriteOffset], Player.XPos+XOffset + WorldOffsetX, Player.YPos+YOffset + WorldOffsetY, 4);
			}
			color C = (color){1.0f, 1.0f, 1.0f, 1.0f};
			if (Player.HitFlash)
			{
				C = (color){1.0f, 0.0f, 0.0f, 1.0f};
			}
			LD_RDrawSpriteWithColor(S_Player[(s32)Player.AniCounter+SpriteOffset], Player.XPos+XOffset + WorldOffsetX, Player.YPos+YOffset + WorldOffsetY, 4, C);
			LD_RDrawSprite(PlayerShadow, Player.XPos+XOffset + WorldOffsetX, Player.YPos+YOffset + WorldOffsetY, 4);

#if RENDER_DEBUG_DOTS
			SetZIndex(10);
			LD_RDrawQuad((Player.XPos-2) + WorldOffsetX, (Player.YPos-2) + WorldOffsetY, 4, 4, Purple);
#endif
			if (Score >= TREES_ATTACK_SCORE && FightingBackMessageTimer > 0)
			{
				FightingBackMessage = TRUE;
			}

			if (FightingBackMessage)
			{
				--FightingBackMessageTimer;
				if (FightingBackMessageTimer < 120)
				{
					if (FightingBackMessageTimer <= 0)
					{
						FightingBackMessage = FALSE;
					}
					SetZIndex(10);
					DrawFontColorWithBackground("they're fighting back", (Player.XPos+WorldOffsetX)-(128+24), (Player.YPos+WorldOffsetY)-96, 2, White);
				}
			}
		}

		// Gui
		SetZIndex(10);

		color NiceRed = (color){255.0f/255.0f, 99.0f/255.0f, 99.0f/255.0f, 1.0f};
		color NiceGreen = (color){114.0f/255.0f, 153.0f/255.0f, 59.0f/255.0f, 1.0f};

		// health bar
		DrawFontColor("health", WINDOW_WIDTH/2 - 316.0f, 8, 2, Black);
		LD_RDrawQuad(WINDOW_WIDTH/2 - 208.0f, 8, (Player.Life/100.0f)*200.0f, 16, NiceRed);
		LD_RDrawQuad(WINDOW_WIDTH/2 - 208.0f, 8, 200.0f, 16, (color){0.8f, 0.8f, 0.8f, 1.0f});

		// Score bar
		DrawFontColor("brutal tree\nmurders", WINDOW_WIDTH/2 + 216.0f, 8, 2, Black);
		LD_RDrawQuad(WINDOW_WIDTH/2 + 8, 8, ((f32)Score/(f32)TREE_COUNT)*200.0f, 16, NiceGreen);
		LD_RDrawQuad(WINDOW_WIDTH/2 + 8, 8, 200.0f, 16, (color){0.8f, 0.8f, 0.8f, 1.0f});

		/*char GuiString[64];
		sprintf(GuiString, "logs %i\n"
						   "life %f\n"
						   "tree deaths %i", Player.Logs, Player.Life, Score);
		DrawFont(GuiString, 10, 10, 2);*/

		//DrawFontColor("hello world!", 10, 10+32, 4, Black);
		//DrawFont("the quick brown fox jumped over the lazy dog!", 10, 10+32+32, 1);
		
		if (Dead)
		{
			// 12
			DrawFont("game over\nyou are dead", (WINDOW_WIDTH/2)-(64*6), 10+32+32+32, 8);
		}

		if (Win)
		{
			Player.Life = 100;
			DrawFont("you win.\nyou murdered\nall the trees.\ni hope you're\nhappy with yourself", (WINDOW_WIDTH/2)-(64*9+32), 10+32+32+32, 8);
		}

		LD_UpdateWindow(&Window);
	}

	return 0;
}
