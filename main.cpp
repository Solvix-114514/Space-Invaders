#include <windows.h>
#include <vector>
#include <algorithm>
#include <ctime>
#include <string>
#include <cmath>

const int WIDTH = 800;
const int HEIGHT = 600;
const int PLAYER_SIZE = 50;
const int ENEMY_SIZE = 30;
const int BULLET_SIZE = 5;

// ��������
const int ENEMY_ROWS = 4;
const int ENEMY_COLS = 10;

// ��Ϸ״̬
enum GameState { PLAYING, GAME_OVER, WIN };

// ��ҽṹ��
struct Player {
    int x, y;
    int speed;
    int lives;
    bool fire;
};

// ���˽ṹ��
struct Enemy {
    int x, y;
    bool alive;
};

// �ӵ��ṹ��
struct Bullet {
    int x, y;
    int speed;
    bool active;
    bool isPlayer;
};

// ��Ϸȫ�ֱ���
GameState gameState = PLAYING;
Player player = {WIDTH/2, HEIGHT-100, 8, 3, false};
std::vector<Enemy> enemies;
std::vector<Bullet> bullets;
int score = 0;
int enemyDirection = 1;
int enemyMoveTimer = 0;

// ���ӵ����ƶ��ٶȣ������ӳ�ʱ�䣨��30����20��
const int ENEMY_MOVE_DELAY = 20;

int gameSpeed = 1;
bool gamePaused = false;

// ��ʼ������
void InitEnemies() {
    enemies.clear();
    for (int row = 0; row < ENEMY_ROWS; row++) {
        for (int col = 0; col < ENEMY_COLS; col++) {
            // �������˳�ʼλ��
            Enemy enemy = {150 + col * 60, 80 + row * 40, true};
            enemies.push_back(enemy);
        }
    }
}

// ��ʼ����Ϸ
void InitGame() {
    player = {WIDTH/2, HEIGHT-100, 8, 3, false};
    bullets.clear();
    InitEnemies();
    score = 0;
    gameState = PLAYING;
    enemyDirection = 1;
    gamePaused = false;
}

// �����ӵ�
void FireBullet(bool isPlayer) {
    Bullet bullet;
    bullet.active = true;
    bullet.isPlayer = isPlayer;
    
    // ���ӵ����ӵ��ٶȣ���5��ߵ�8��
    bullet.speed = isPlayer ? 10 : 8;
    
    if (isPlayer) {
        bullet.x = player.x + PLAYER_SIZE/2 - BULLET_SIZE/2;
        bullet.y = player.y - BULLET_SIZE;
    } else {
        // ������˷����ӵ�
        std::vector<int> aliveEnemies;
        for (int i = 0; i < enemies.size(); i++) {
            if (enemies[i].alive) aliveEnemies.push_back(i);
        }
        
        if (!aliveEnemies.empty()) {
            int idx = rand() % aliveEnemies.size();
            int enemyIdx = aliveEnemies[idx];
            bullet.x = enemies[enemyIdx].x + ENEMY_SIZE/2 - BULLET_SIZE/2;
            bullet.y = enemies[enemyIdx].y + ENEMY_SIZE;
            bullets.push_back(bullet);
        }
    }
    
    if (isPlayer) bullets.push_back(bullet);
}

// ��ײ���
bool CheckCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 &&
            x1 + w1 > x2 &&
            y1 < y2 + h2 &&
            y1 + h1 > y2);
}

// ������Ϸ�߼�
void UpdateGame() {
    if (gamePaused || gameState != PLAYING) return;
    
    // ��ҿ���
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) 
        player.x = std::max(0, player.x - player.speed);
    
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) 
        player.x = std::min(WIDTH - PLAYER_SIZE, player.x + player.speed);
    
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        if (!player.fire) {
            FireBullet(true);
            player.fire = true;
        }
    } else {
        player.fire = false;
    }
    
    // �ƶ��ӵ�
    for (auto& bullet : bullets) {
        if (bullet.active) {
            bullet.y += bullet.isPlayer ? -bullet.speed : bullet.speed;
            
            // ����ӵ��Ƿ����
            if (bullet.y < 0 || bullet.y > HEIGHT) {
                bullet.active = false;
            }
            
            // ����ӵ����е���
            if (bullet.isPlayer) {
                for (auto& enemy : enemies) {
                    if (enemy.alive && CheckCollision(
                        bullet.x, bullet.y, BULLET_SIZE, BULLET_SIZE,
                        enemy.x, enemy.y, ENEMY_SIZE, ENEMY_SIZE)) {
                        enemy.alive = false;
                        bullet.active = false;
                        score += 10;
                        break;
                    }
                }
            } 
            // �����ӵ��������
            else if (CheckCollision(
                bullet.x, bullet.y, BULLET_SIZE, BULLET_SIZE,
                player.x, player.y, PLAYER_SIZE, PLAYER_SIZE)) {
                bullet.active = false;
                player.lives--;
                if (player.lives <= 0) {
                    gameState = GAME_OVER;
                }
            }
        }
    }
    
    // �Ƴ���Ч�ӵ�
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), 
        [](const Bullet& b) { return !b.active; }), bullets.end());
    
    // �ƶ�����
    if (++enemyMoveTimer > ENEMY_MOVE_DELAY) {
        enemyMoveTimer = 0;
        bool changeDirection = false;
        
        for (auto& enemy : enemies) {
            if (!enemy.alive) continue;
            
            // ���ӵ����ƶ��ٶ�
            enemy.x += 30 * enemyDirection;
            
            // ���߽���ײ
            if (enemy.x <= 0 || enemy.x + ENEMY_SIZE >= WIDTH) {
                changeDirection = true;
            }
            
            // �����˵���ײ�
            if (enemy.y + ENEMY_SIZE >= player.y) {
                gameState = GAME_OVER;
                return;
            }
        }
        
        if (changeDirection) {
            enemyDirection *= -1;
            for (auto& enemy : enemies) {
                if (enemy.alive) {
                    enemy.y += 20; // ��������
                }
            }
        }
        
        // ���˹���Ƶ��
        if (rand() % 100 < 60) {
            FireBullet(false);
        }
    }
    
    // ���ʤ������
    bool allDead = true;
    for (const auto& enemy : enemies) {
        if (enemy.alive) {
            allDead = false;
            break;
        }
    }
    
    if (allDead) {
        gameState = WIN;
    }
}

// ������Ϸ
void DrawGame(HDC hdc) {
    // ���Ʊ���
    HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 40));
    RECT bgRect = {0, 0, WIDTH, HEIGHT};
    FillRect(hdc, &bgRect, bgBrush);
    DeleteObject(bgBrush);
    
    // �������Ǳ���
    for (int i = 0; i < 100; i++) {
        int x = rand() % WIDTH;
        int y = rand() % HEIGHT;
        SetPixel(hdc, x, y, RGB(255, 255, 255));
    }
    
    // �������
    HBRUSH playerBrush = CreateSolidBrush(RGB(0, 255, 0));
    RECT playerRect = {player.x, player.y, player.x + PLAYER_SIZE, player.y + PLAYER_SIZE/3};
    FillRect(hdc, &playerRect, playerBrush);
    DeleteObject(playerBrush);
    
    // ���Ƶ���
    for (const auto& enemy : enemies) {
        if (enemy.alive) {
            HBRUSH enemyBrush = CreateSolidBrush(RGB(255, 50, 50));
            Ellipse(hdc, enemy.x, enemy.y, enemy.x + ENEMY_SIZE, enemy.y + ENEMY_SIZE);
            DeleteObject(enemyBrush);
        }
    }
    
    // �����ӵ�
    for (const auto& bullet : bullets) {
        HBRUSH bulletBrush = CreateSolidBrush(bullet.isPlayer ? RGB(0, 200, 255) : RGB(255, 255, 0));
        RECT bulletRect = {bullet.x, bullet.y, bullet.x + BULLET_SIZE, bullet.y + BULLET_SIZE};
        FillRect(hdc, &bulletRect, bulletBrush);
        DeleteObject(bulletBrush);
    }
    
    // ���Ʒ���������ֵ
    HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
    SelectObject(hdc, hFont);
    SetTextColor(hdc, RGB(0, 255, 0));
    SetBkMode(hdc, TRANSPARENT);
    
    std::string scoreStr = "Score: " + std::to_string(score);
    TextOutA(hdc, 10, 10, scoreStr.c_str(), scoreStr.length());
    
    std::string livesStr = "Lives: " + std::to_string(player.lives);
    TextOutA(hdc, WIDTH - 100, 10, livesStr.c_str(), livesStr.length());
    
    // ������Ϸ״̬
    if (gameState == GAME_OVER) {
        HFONT bigFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
        SelectObject(hdc, bigFont);
        SetTextColor(hdc, RGB(255, 0, 0));
        std::string gameOver = "GAME OVER!";
        TextOutA(hdc, WIDTH/2 - 120, HEIGHT/2 - 50, gameOver.c_str(), gameOver.length());
        DeleteObject(bigFont);
        
        std::string restart = "Press 'R' to Restart";
        TextOutA(hdc, WIDTH/2 - 100, HEIGHT/2 + 20, restart.c_str(), restart.length());
    } 
    else if (gameState == WIN) {
        HFONT bigFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
        SelectObject(hdc, bigFont);
        SetTextColor(hdc, RGB(0, 255, 0));
        std::string winMsg = "YOU WIN!";
        TextOutA(hdc, WIDTH/2 - 80, HEIGHT/2 - 50, winMsg.c_str(), winMsg.length());
        DeleteObject(bigFont);
        
        std::string restart = "Press 'R' to Restart";
        TextOutA(hdc, WIDTH/2 - 100, HEIGHT/2 + 20, restart.c_str(), restart.length());
    }
    
    if (gamePaused) {
        HFONT bigFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
        SelectObject(hdc, bigFont);
        SetTextColor(hdc, RGB(255, 255, 0));
        std::string paused = "PAUSED";
        TextOutA(hdc, WIDTH/2 - 80, HEIGHT/2 - 50, paused.c_str(), paused.length());
        DeleteObject(bigFont);
    }
    
    DeleteObject(hFont);
}

// ���ڹ��̺���
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
            SetTimer(hWnd, 1, 16, NULL); // 60 FPS
            srand(static_cast<unsigned>(time(0)));
            InitGame();
            break;
            
        case WM_TIMER:
            UpdateGame();
            InvalidateRect(hWnd, NULL, TRUE);
            break;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            DrawGame(hdc);
            EndPaint(hWnd, &ps);
            break;
        }
            
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                PostQuitMessage(0);
            }
            else if (wParam == 'R' || wParam == 'r') {
                if (gameState != PLAYING) InitGame();
            }
            else if (wParam == 'P' || wParam == 'p') {
                gamePaused = !gamePaused;
            }
            break;
            
        case WM_DESTROY:
            KillTimer(hWnd, 1);
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// ������
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char* CLASS_NAME = "SpaceInvadersWindowClass";
    
    // ע�ᴰ����
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);
    
    // ��������
    HWND hWnd = CreateWindow(
        CLASS_NAME,
        "Space Invaders - Enhanced Edition",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,
        NULL, NULL, hInstance, NULL
    );
    
    if (hWnd == NULL) return 0;
    
    // �������ڴ�С����Ӧ�ͻ���
    RECT rc = {0, 0, WIDTH, HEIGHT};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME, FALSE);
    SetWindowPos(hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);
    
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    // ��Ϣѭ��
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
