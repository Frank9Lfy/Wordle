#include <graphics.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//颜色定义
#define GREEN      RGB(106, 170, 100)
#define YELLOW     RGB(201, 180, 88)
#define LIGHTGRAY  RGB(211, 211, 211)
#define DARKGRAY   RGB(120, 124, 126)
#define LIGHTRED   RGB(255, 180, 180)

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 720;
const int WORD_LENGTH = 5;
const int MAX_ATTEMPTS = 6;
const int CELL_SIZE = 60;

//游戏状态
enum { PAGE_MAIN_MENU, PAGE_GAME, PAGE_HELP, PAGE_STATS, PAGE_EXIT } current_page = PAGE_MAIN_MENU;

//基础游戏结构
struct Game {
    char target_word[6];
    char guesses[MAX_ATTEMPTS][6];
    int current_attempt;
    int game_over;
    int won;
    int invalid_word;
    int hint_used;
    char hint_message[100];
    int hard_mode;
    int key_status[26]; // 键盘状态：0=未用, 1=灰, 2=黄, 3=绿
} game;

//开足够大的二维数组保存词库
char word_list[1000][6];
int word_count = 0;

//按钮结构
struct Button {
    int x, y, w, h;
    const char* text;
    int page_id;
};

Button mainButtons[5];
Button backBtn;

//统计数据结构
struct Statistics {
    int games_played;
    int games_won;
    int current_streak;
    int max_streak;
    int guess_distribution[MAX_ATTEMPTS];
};

Statistics stats = {0};

// 屏幕键盘布局
const char* keyboard_rows[3] = {
    "QWERTYUIOP",
    "ASDFGHJKL",
    "ZXCVBNM"
};
const int row_lengths[3] = {10, 9, 7};

struct Key {
    int x, y, w, h;
    char ch;
} keys[26];

// 函数声明
//工具函数
void initGraphics();
int randInt(int max);
void loadWordList();
void saveStats();
void loadStats();
void updateStats();
void startNewGame(int hard_mode);
int isValidWord(const char* word);
int satisfiesHardMode(const char* guess);
void checkGuess(char* guess, int* result);
void provideHint();
void updateKeyboardStatus();
void initKeyboardLayout();

//绘图函数
void drawKeyboard();
void drawCell(int x, int y, int size, char c, int color);
void drawMainMenu();
void drawGameScreen();
void drawHelpScreen();
void drawStatsScreen();

//事件处理
int handleMouseClick();
void handleGameInput();

//主函数
int main() {
    initGraphics();
    initKeyboardLayout(); // 初始化键盘布局
    loadWordList();
    loadStats();
    bool running = true;
    while (running) {
        int click = handleMouseClick();
        if (click == PAGE_EXIT) {
            break;
        } else if (click == PAGE_HELP || click == PAGE_STATS || click == PAGE_MAIN_MENU) {
            current_page = (decltype(current_page))click;
        }

            switch (current_page) {
            case PAGE_MAIN_MENU:
                drawMainMenu();
                break;
            case PAGE_GAME:
                drawGameScreen();
                handleGameInput();
                break;
            case PAGE_HELP:
                drawHelpScreen();
                break;
            case PAGE_STATS:
                drawStatsScreen();
                break;
        }

        FlushBatchDraw();
        Sleep(16);
    }

    EndBatchDraw();
    closegraph();
    return 0;
}

//工具函数实现
void initGraphics() {
    initgraph(SCREEN_WIDTH, SCREEN_HEIGHT);
    setbkcolor(WHITE);
    settextstyle(24, 0, "SimSun");
    BeginBatchDraw();
}

int randInt(int max) {
    return rand() % max;
}

void loadWordList() {
    FILE* fp = fopen("words.txt", "r");
    if (!fp) {
        const char* fallback[] = {
            "HELLO", "WORLD", "APPLE", "TRAIN", "GUESS", "CODES", "SLATE", "CRANE",
            "AUDIO", "STARE", "TEARS", "ALONE", "ADIEU", "LATER", "WHILE", "ABACK",
            "CRAZE", "PLUMB", "FJORD", "GYPSY", "QUART", "ZEBRA", "JUMBO", "VIXEN"
        };
        word_count = sizeof(fallback) / sizeof(fallback[0]);
        for (int i = 0; i < word_count && i < 1000; i++) {
            strcpy(word_list[i], fallback[i]);
        }
        return;
    }

    word_count = 0;
    char line[100];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;
        if (strlen(line) == 5) {
            int valid = 1;
            for (int i = 0; i < 5; i++) {
                if (line[i] >= 'a' && line[i] <= 'z') line[i] = line[i] - 'a' + 'A';
                if (line[i] < 'A' || line[i] > 'Z') {
                    valid = 0;
                    break;
                }
            }
            if (valid && word_count < 1000) {
                strcpy(word_list[word_count], line);
                word_count++;
            }
        }
    }
    fclose(fp);
}

void saveStats() {
    FILE* fp = fopen("stats.dat", "wb");
    if (fp) {
        fwrite(&stats, sizeof(Statistics), 1, fp);
        fclose(fp);
    }
}

void loadStats() {
    FILE* fp = fopen("stats.dat", "rb");
    if (fp) {
        fread(&stats, sizeof(Statistics), 1, fp);
        fclose(fp);
    } else {
        memset(&stats, 0, sizeof(Statistics));
    }
}

void updateStats() {
    stats.games_played++;
    if (game.won) {
        stats.games_won++;
        stats.current_streak++;
        if (stats.current_streak > stats.max_streak) {
            stats.max_streak = stats.current_streak;
        }
        if (game.current_attempt <= MAX_ATTEMPTS) {
            stats.guess_distribution[game.current_attempt - 1]++;
        }
    } else {
        stats.current_streak = 0;
    }
    saveStats();
}

void startNewGame(int hard_mode) {
    srand((unsigned)time(0));
    strcpy(game.target_word, word_list[randInt(word_count)]);
    for (int i = 0; i < MAX_ATTEMPTS; i++) {
        game.guesses[i][0] = '\0';
    }
    for (int i = 0; i < 26; i++) {
        game.key_status[i] = 0;
    }
    game.current_attempt = 0;
    game.game_over = 0;
    game.won = 0;
    game.invalid_word = 0;
    game.hint_used = 0;
    game.hint_message[0] = '\0';
    game.hard_mode = hard_mode;
    current_page = PAGE_GAME;
}

int isValidWord(const char* word) {
    for (int i = 0; i < word_count; i++) {
        if (strcmp(word, word_list[i]) == 0) return 1;
    }
    return 0;
}

int satisfiesHardMode(const char* guess) {
    if (!game.hard_mode) return 1;

    char must_use[256] = {0};
    char fixed_pos[5] = {0};

    for (int row = 0; row < game.current_attempt; row++) {
        int result[WORD_LENGTH];
        checkGuess(game.guesses[row], result);
        for (int i = 0; i < WORD_LENGTH; i++) {
            char c = game.guesses[row][i];
            if (result[i] == GREEN) {
                fixed_pos[i] = c;
            }
            if (result[i] == GREEN || result[i] == YELLOW) {
                must_use[(unsigned char)c] = 1;
            }
        }
    }

    for (int i = 0; i < WORD_LENGTH; i++) {
        if (fixed_pos[i] && guess[i] != fixed_pos[i]) {
            return 0;
        }
    }

    char has[256] = {0};
    for (int i = 0; i < WORD_LENGTH; i++) {
        has[(unsigned char)guess[i]] = 1;
    }
    for (int c = 'A'; c <= 'Z'; c++) {
        if (must_use[c] && !has[c]) {
            return 0;
        }
    }

    return 1;
}

void checkGuess(char* guess, int* result) {
    int used[WORD_LENGTH] = {0};
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (guess[i] == game.target_word[i]) {
            result[i] = 3; // GREEN
            used[i] = 1;
        } else {
            result[i] = 1; // LIGHTGRAY
        }
    }
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (result[i] != 3) {
            for (int j = 0; j < WORD_LENGTH; j++) {
                if (!used[j] && guess[i] == game.target_word[j]) {
                    result[i] = 2; // YELLOW
                    used[j] = 1;
                    break;
                }
            }
        }
    }
}

void provideHint() {
    if (game.hint_used || game.game_over || game.current_attempt == 0) return;

    char* last_guess = game.guesses[game.current_attempt - 1];
    int result[WORD_LENGTH];
    checkGuess(last_guess, result);

    int yellow_index = -1;
    for (int i = 0; i < WORD_LENGTH; i++) {
        if (result[i] == 2) {
            yellow_index = i;
            break;
        }
    }

    if (yellow_index != -1) {
        char letter = last_guess[yellow_index];
        for (int pos = 0; pos < WORD_LENGTH; pos++) {
            if (game.target_word[pos] == letter && last_guess[pos] != letter) {
                sprintf(game.hint_message, "建议：将 '%c' 移到第 %d 位", letter, pos + 1);
                game.hint_used = 1;
                return;
            }
        }
        sprintf(game.hint_message, "字母 '%c' 在词中但位置不对", letter);
    } else {
        int guess_has[256] = {0};
        for (int i = 0; i < WORD_LENGTH; i++) {
            guess_has[(unsigned char)last_guess[i]] = 1;
        }
        for (int i = 0; i < WORD_LENGTH; i++) {
            char c = game.target_word[i];
            if (!guess_has[(unsigned char)c]) {
                sprintf(game.hint_message, "建议：尝试包含字母 '%c'", c);
                game.hint_used = 1;
                return;
            }
        }
        strcpy(game.hint_message, "已覆盖所有字母，请调整位置！");
    }
    game.hint_used = 1;
}

//键盘相关
void updateKeyboardStatus() {
    int new_status[26] = {0};

    for (int row = 0; row < game.current_attempt; row++) {
        int result[WORD_LENGTH];
        checkGuess(game.guesses[row], result);
        for (int i = 0; i < WORD_LENGTH; i++) {
            char c = game.guesses[row][i];
            int idx = c - 'A';
            if (result[i] == 3) {
                new_status[idx] = 3;
            } else if (result[i] == 2) {
                if (new_status[idx] != 3) new_status[idx] = 2;
            } else {
                if (new_status[idx] == 0) new_status[idx] = 1;
            }
        }
    }

    for (int i = 0; i < 26; i++) {
        game.key_status[i] = new_status[i];
    }
}

void initKeyboardLayout() {
    int start_y = 500;
    for (int r = 0; r < 3; r++) {
        int len = row_lengths[r];
        int start_x = (SCREEN_WIDTH - len * 60) / 2;
        const char* row = keyboard_rows[r];
        for (int i = 0; i < len; i++) {
            char c = row[i];
            int idx = c - 'A';
            keys[idx].ch = c;
            keys[idx].x = start_x + i * 60;
            keys[idx].y = start_y + r * 55;
            keys[idx].w = 55;
            keys[idx].h = 45;
        }
    }
}

void drawKeyboard() {
    COLORREF key_colors[] = { LIGHTGRAY, DARKGRAY, YELLOW, GREEN };
    for (int i = 0; i < 26; i++) {
        if (keys[i].ch == 0) continue;
        int status = game.key_status[i];
        setfillcolor(key_colors[status]);
        settextcolor(BLACK);
        solidroundrect(keys[i].x, keys[i].y,
                      keys[i].x + keys[i].w,
                      keys[i].y + keys[i].h, 8, 8);
        roundrect(keys[i].x, keys[i].y,
                  keys[i].x + keys[i].w,
                  keys[i].y + keys[i].h, 8, 8);
        char txt[2] = {keys[i].ch, '\0'};
        outtextxy(keys[i].x + 15, keys[i].y + 10, txt);
    }

    setfillcolor(LIGHTRED);
    solidroundrect(SCREEN_WIDTH - 100, SCREEN_HEIGHT - 170, SCREEN_WIDTH - 30, SCREEN_HEIGHT - 125, 8, 8);
    roundrect(SCREEN_WIDTH - 100, SCREEN_HEIGHT - 170, SCREEN_WIDTH - 30, SCREEN_HEIGHT - 125, 8, 8);
    outtextxy(SCREEN_WIDTH - 90, SCREEN_HEIGHT - 160, "DEL");
}

//绘图函数
void drawCell(int x, int y, int size, char c, int color_idx) {
    COLORREF colors[] = { WHITE, LIGHTGRAY, YELLOW, GREEN };
    COLORREF fill = (color_idx >= 0 && color_idx <= 3) ? colors[color_idx] : RED;

    setlinecolor(BLACK);
    setfillcolor(fill);
    fillrectangle(x, y, x + size, y + size);
    rectangle(x, y, x + size, y + size);
    if (c != ' ') {
        char ch[2] = { c, '\0' };
        settextcolor(BLACK);
        outtextxy(x + size/2 - 8, y + size/2 - 12, ch);
    }
}

void drawMainMenu() {
    setfillcolor(RGB(245, 245, 255));
    fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // 装饰波浪
    setfillcolor(RGB(230, 240, 255));
    for (int i = -100; i < SCREEN_WIDTH + 100; i += 80) {
        fillellipse(i, -30, 100, 60);
    }

    // 标题带阴影
    settextcolor(RGB(80, 80, 80));
    outtextxy(432, 72, "W O R D L E");
    settextcolor(GREEN);
    outtextxy(430, 70, "W O R D L E");

    settextcolor(DARKGRAY);
    outtextxy(417, 142, "Enjoy yourself!");
    settextcolor(BLACK);
    outtextxy(415, 140, "Enjoy yourself!");

    // 按钮背景板
    setfillcolor(WHITE);
    solidroundrect(350, 180, 650, 550, 20, 20);
    setlinecolor(LIGHTGRAY);
    roundrect(350, 180, 650, 550, 20, 20);

    mainButtons[0] = { 380, 200, 240, 50, "简单模式", PAGE_GAME };
    mainButtons[1] = { 380, 270, 240, 50, "游戏说明", PAGE_HELP };
    mainButtons[2] = { 380, 340, 240, 50, "统计数据", PAGE_STATS };
    mainButtons[3] = { 380, 410, 240, 50, "退出游戏", PAGE_EXIT };
    mainButtons[4] = { 380, 480, 240, 50, "困难模式", PAGE_GAME };

    // 制作信息
    settextcolor(DARKGRAY);
    outtextxy(10, 600, "制作者：健雄书院");
    outtextxy(90, 640, "王开251880151 陆飞宇251880131");
    outtextxy(90, 680, "曾秋伊251880488 孙阳251880511");

    // 按钮绘制
    for (int i = 0; i < 5; i++) {
        COLORREF btnColor = (i == 0 || i == 4) ? RGB(220, 240, 220) : LIGHTGRAY;
        setfillcolor(btnColor);
        solidroundrect(mainButtons[i].x, mainButtons[i].y,
                       mainButtons[i].x + mainButtons[i].w,
                       mainButtons[i].y + mainButtons[i].h, 10, 10);
        setlinecolor(BLACK);
        roundrect(mainButtons[i].x, mainButtons[i].y,
                  mainButtons[i].x + mainButtons[i].w,
                  mainButtons[i].y + mainButtons[i].h, 10, 10);
        settextcolor(BLACK);
        outtextxy(mainButtons[i].x + 60, mainButtons[i].y + 15, mainButtons[i].text);
    }
}

void drawGameScreen() {
    setfillcolor(RGB(250, 250, 255));
    fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // 顶部状态栏
    setfillcolor(RGB(245, 250, 255));
    fillrectangle(0, 0, SCREEN_WIDTH, 60);
    setlinecolor(LIGHTGRAY);
    line(0, 60, SCREEN_WIDTH, 60);

    settextstyle(24, 0, "SimSun");
    settextcolor(BLACK);
    char title[100];
    sprintf(title, "WORDLE - 第 %d / %d 轮", game.current_attempt + 1, MAX_ATTEMPTS);
    outtextxy(410, 20, title);

    int startX = (SCREEN_WIDTH - WORD_LENGTH * CELL_SIZE) / 2;
    int startY = 80;

    for (int row = 0; row < MAX_ATTEMPTS; row++) {
        for (int col = 0; col < WORD_LENGTH; col++) {
            int x = startX + col * CELL_SIZE;
            int y = startY + row * (CELL_SIZE + 10);
            char ch = ' ';
            int color_idx = 0; // WHITE

            if (row < game.current_attempt) {
                int result[WORD_LENGTH];
                checkGuess(game.guesses[row], result);
                ch = game.guesses[row][col];
                color_idx = result[col];
            } else if (row == game.current_attempt && col < (int)strlen(game.guesses[row])) {
                ch = game.guesses[row][col];
                if (game.invalid_word && strlen(game.guesses[row]) == WORD_LENGTH) {
                    color_idx = -1; // RED
                } else {
                    color_idx = 1; // LIGHTGRAY
                }
            }
            drawCell(x, y, CELL_SIZE, ch, color_idx);
        }
    }

    settextcolor(BLACK);
    outtextxy(10, 60, "输入5个大写字母，按回车提交");
    outtextxy(10, 100, "ESC: 返回主菜单");

    if (game.invalid_word && strlen(game.guesses[game.current_attempt]) == WORD_LENGTH) {
        settextcolor(RED);
        settextstyle(20, 0, "SimSun");
        if (game.hint_message[0] != '\0') {
            outtextxy(30, 145, game.hint_message);  // 显示实际的错误信息
        } else {
            outtextxy(30, 145, "输入的单词不在词典中，请重新输入");  // 保持原有逻辑
        }
        settextstyle(24, 0, "SimSun");
        settextcolor(BLACK);
    }

    // 提示区域背景
    if (!game.hint_used || game.hint_message[0] != '\0') {
        setfillcolor(RGB(240, 248, 255));
        solidroundrect(680, 430, 980, 480, 10, 10);
        setlinecolor(LIGHTGRAY);
        roundrect(680, 430, 980, 480, 10, 10);
    }

    if (game.hint_message[0] != '\0') {
        settextcolor(BLUE);
        settextstyle(20, 0, "SimSun");
        outtextxy(700, 450, game.hint_message);
        settextstyle(24, 0, "SimSun");
        settextcolor(BLACK);
    }

    if (game.game_over) {
        settextcolor(game.won ? GREEN : RED);
        char msg[100];
        if (game.won) {
            strcpy(msg, "恭喜！你猜对了！");
        } else {
            sprintf(msg, "答案是：%s", game.target_word);
        }
        outtextxy(250, 560, msg);
        settextstyle(20, 0, "SimSun");
        settextcolor(BLUE);
        outtextxy(500, 560, "按空格键开始下一局");
    }

    // 提示按钮
    int btn_x = 860, btn_y = 30, btn_w = 100, btn_h = 40;
    COLORREF btn_color;
    if (!game.hint_used && !game.game_over) {
        btn_color = GREEN;
    } else {
        btn_color = RGB(200, 200, 200); // 灰色表示不可用
    }
    setfillcolor(btn_color);
    solidroundrect(btn_x, btn_y, btn_x + btn_w, btn_y + btn_h, 8, 8);
    setlinecolor(BLACK);
    roundrect(btn_x, btn_y, btn_x + btn_w, btn_y + btn_h, 8, 8);
    settextcolor(BLACK);
    outtextxy(btn_x + 20, btn_y + 10, game.hint_used ? "已使用" : "提示");

    // 分隔线
    setlinecolor(LIGHTGRAY);
    line(0, 480, SCREEN_WIDTH, 480);

    // 绘制屏幕键盘
    if (!game.game_over) {
        drawKeyboard();
    }
}

void drawHelpScreen(){
    setfillcolor(RGB(252, 252, 255));
    fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    setfillcolor(RGB(230, 245, 255));
    fillrectangle(0, 0, SCREEN_WIDTH, 80);
    settextcolor(BLUE);
    settextstyle(32, 0, "SimSun");
    outtextxy(380, 20, "游戏说明");
    settextstyle(24, 0, "SimSun");
    settextcolor(BLACK);

    setfillcolor(WHITE);
    solidroundrect(80, 100, 920, 500, 15, 15);
    setlinecolor(LIGHTGRAY);
    roundrect(80, 100, 920, 500, 15, 15);

    int y = 130;
    outtextxy(120, y, "1. 猜一个5字母英文单词"); y += 40;
    outtextxy(120, y, "2. 绿色：字母位置正确"); y += 40;
    outtextxy(120, y, "3. 黄色：字母存在但位置错"); y += 40;
    outtextxy(120, y, "4. 灰色：字母不存在"); y += 40;
    outtextxy(120, y, "5. 你有6次机会"); y += 40;
    outtextxy(120, y, "6. 困难模式规则："); y += 30;
    outtextxy(140, y, "   必须包含所有已揭示的黄/绿字母"); y += 30;
    outtextxy(140, y, "   绿色字母必须保持在原位置"); y += 40;
    outtextxy(120, y, "7. 单词无效时格子变红"); y += 40;
    outtextxy(120, y, "8. 每局仅可使用一次提示");

    backBtn = { 400, 500, 200, 50, "返回", PAGE_MAIN_MENU };
    setfillcolor(RGB(240, 240, 240));
    solidroundrect(backBtn.x, backBtn.y, backBtn.x + backBtn.w, backBtn.y + backBtn.h, 12, 12);
    setlinecolor(DARKGRAY);
    roundrect(backBtn.x, backBtn.y, backBtn.x + backBtn.w, backBtn.y + backBtn.h, 12, 12);
    settextcolor(BLACK);
    outtextxy(backBtn.x + 70, backBtn.y + 15, "返回");
}

void drawStatsScreen() {
    setfillcolor(RGB(252, 252, 255));
    fillrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    setfillcolor(RGB(240, 250, 240));
    fillrectangle(0, 0, SCREEN_WIDTH, 80);
    settextcolor(GREEN);
    settextstyle(32, 0, "SimSun");
    outtextxy(380, 20, "统计数据");
    settextstyle(24, 0, "SimSun");
    settextcolor(BLACK);

    setfillcolor(WHITE);
    solidroundrect(80, 100, 920, 600, 15, 15);
    setlinecolor(LIGHTGRAY);
    roundrect(80, 100, 920, 600, 15, 15);

    int y = 130;
    char buffer[100];
    sprintf(buffer, "游戏总次数: %d", stats.games_played);
    outtextxy(120, y, buffer); y += 40;
    sprintf(buffer, "获胜次数: %d", stats.games_won);
    outtextxy(120, y, buffer); y += 40;
    int win_rate = stats.games_played > 0 ? (stats.games_won * 100 / stats.games_played) : 0;
    sprintf(buffer, "胜率: %d%%", win_rate);
    outtextxy(120, y, buffer); y += 40;
    sprintf(buffer, "当前连胜: %d", stats.current_streak);
    outtextxy(120, y, buffer); y += 40;
    sprintf(buffer, "最大连胜: %d", stats.max_streak);
    outtextxy(120, y, buffer); y += 50;

    outtextxy(120, y, "猜测成功分布："); y += 40;
    for (int i = 0; i < MAX_ATTEMPTS; i++) {
        sprintf(buffer, "第%d次：%d", i+1, stats.guess_distribution[i]);
        outtextxy(140, y, buffer);
        y += 35;
    }

    backBtn = { 400, 580, 200, 50, "返回", PAGE_MAIN_MENU };
    setfillcolor(RGB(240, 240, 240));
    solidroundrect(backBtn.x, backBtn.y, backBtn.x + backBtn.w, backBtn.y + backBtn.h, 12, 12);
    setlinecolor(DARKGRAY);
    roundrect(backBtn.x, backBtn.y, backBtn.x + backBtn.w, backBtn.y + backBtn.h, 12, 12);
    settextcolor(BLACK);
    outtextxy(backBtn.x + 70, backBtn.y + 15, "返回");
}

//事件处理
int handleMouseClick() {
    MOUSEMSG msg = {0};
    if (MouseHit()) {
        msg = GetMouseMsg();
        if (msg.uMsg == WM_LBUTTONDOWN) {
            if (current_page == PAGE_MAIN_MENU) {
                for (int i = 0; i < 5; i++) {
                    if (msg.x >= mainButtons[i].x && msg.x <= mainButtons[i].x + mainButtons[i].w &&
                        msg.y >= mainButtons[i].y && msg.y <= mainButtons[i].y + mainButtons[i].h) {
                        if (i == 0) {
                            startNewGame(0);
                            return -1;
                        } else if (i == 4) {
                            startNewGame(1);
                            return -1;
                        } else {
                            return mainButtons[i].page_id;
                        }
                    }
                }
            }
            else if (current_page == PAGE_GAME) {
                // 提示按钮
                int btn_x = 860, btn_y = 30, btn_w = 100, btn_h = 40;
                if (!game.hint_used && !game.game_over &&
                    msg.x >= btn_x && msg.x <= btn_x + btn_w &&
                    msg.y >= btn_y && msg.y <= btn_y + btn_h) {
                    provideHint();
                    return -1;
                }

                // 屏幕键盘点击
                if (!game.game_over && game.current_attempt < MAX_ATTEMPTS) {
                    for (int i = 0; i < 26; i++) {
                        if (keys[i].ch == 0) continue;
                        if (msg.x >= keys[i].x && msg.x <= keys[i].x + keys[i].w &&
                            msg.y >= keys[i].y && msg.y <= keys[i].y + keys[i].h) {
                            int len = strlen(game.guesses[game.current_attempt]);
                            if (len < WORD_LENGTH) {
                                game.guesses[game.current_attempt][len] = keys[i].ch;
                                game.guesses[game.current_attempt][len + 1] = '\0';
                                game.invalid_word = 0;
                            }
                            return -1;
                        }
                    }
                    // 退格键：
                    int del_x1 = SCREEN_WIDTH - 100, del_x2 = SCREEN_WIDTH - 30;
                    int del_y1 = SCREEN_HEIGHT - 170, del_y2 = SCREEN_HEIGHT - 125;
                    if (msg.x >= del_x1 && msg.x <= del_x2 &&
                        msg.y >= del_y1 && msg.y <= del_y2) {
                        int len = strlen(game.guesses[game.current_attempt]);
                        if (len > 0) {
                            game.guesses[game.current_attempt][len - 1] = '\0';
                            game.invalid_word = 0;
                        }
                        return -1;
                    }
                }
            }
            else if (current_page == PAGE_HELP || current_page == PAGE_STATS) {
                if (msg.x >= backBtn.x && msg.x <= backBtn.x + backBtn.w &&
                    msg.y >= backBtn.y && msg.y <= backBtn.y + backBtn.h) {
                    return PAGE_MAIN_MENU;
                }
            }
        }
    }
    return -1;
}

void handleGameInput() {
    ExMessage m;
    while (peekmessage(&m, EM_KEY)) {
        if (m.message != WM_KEYDOWN) continue;
        if (m.vkcode == VK_ESCAPE) {
            current_page = PAGE_MAIN_MENU;
            return;
        }
    }

    static bool keyWasPressed[256] = {false};

    if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
        if (!keyWasPressed[VK_RETURN]) {
            keyWasPressed[VK_RETURN] = true;
            if (!game.game_over && game.current_attempt < MAX_ATTEMPTS) {
                int len = (int)strlen(game.guesses[game.current_attempt]);
                if (len == WORD_LENGTH) {
                    if (!isValidWord(game.guesses[game.current_attempt])) {
                        game.invalid_word = 1;
                        game.hint_message[0] = '\0';
                    } else if (!satisfiesHardMode(game.guesses[game.current_attempt])) {
                        game.invalid_word = 1;
                        strcpy(game.hint_message, "困难模式：必须使用已揭示的线索！");
                    } else {
                        game.invalid_word = 0;
                        game.hint_message[0] = '\0';
                        if (strcmp(game.guesses[game.current_attempt], game.target_word) == 0) {
                            game.won = 1;
                            game.game_over = 1;
                        }
                        game.current_attempt++;
                        if (game.current_attempt >= MAX_ATTEMPTS) {
                            game.game_over = 1;
                        }
                        if (game.game_over) {
                            updateStats();
                        } else {
                            updateKeyboardStatus();
                        }
                    }
                }
            }
        }
    } else {
        keyWasPressed[VK_RETURN] = false;
    }

    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        if (!keyWasPressed[VK_SPACE]) {
            keyWasPressed[VK_SPACE] = true;
            if (game.game_over) {
                startNewGame(game.hard_mode);
            }
        }
    } else {
        keyWasPressed[VK_SPACE] = false;
    }

    if (GetAsyncKeyState(VK_BACK) & 0x8000) {
        if (!keyWasPressed[VK_BACK]) {
            keyWasPressed[VK_BACK] = true;
            if (!game.game_over && game.current_attempt < MAX_ATTEMPTS) {
                int len = (int)strlen(game.guesses[game.current_attempt]);
                if (len > 0) {
                    game.guesses[game.current_attempt][len - 1] = '\0';
                    game.invalid_word = 0;
                }
            }
        }
    } else {
        keyWasPressed[VK_BACK] = false;
    }

    for (int vk = 'A'; vk <= 'Z'; vk++) {
        if (GetAsyncKeyState(vk) & 0x8000) {
            if (!keyWasPressed[vk]) {
                keyWasPressed[vk] = true;
                if (!game.game_over && game.current_attempt < MAX_ATTEMPTS) {
                    int len = (int)strlen(game.guesses[game.current_attempt]);
                    if (len < WORD_LENGTH) {
                        game.guesses[game.current_attempt][len] = (char)vk;
                        game.guesses[game.current_attempt][len + 1] = '\0';
                        game.invalid_word = 0;
                    }
                }
            }
        } else {
            keyWasPressed[vk] = false;
        }
    }

    for (int vk = 'a'; vk <= 'z'; vk++) {
        if (GetAsyncKeyState(vk) & 0x8000) {
            if (!keyWasPressed[vk]) {
                keyWasPressed[vk] = true;
                if (!game.game_over && game.current_attempt < MAX_ATTEMPTS) {
                    int len = (int)strlen(game.guesses[game.current_attempt]);
                    if (len < WORD_LENGTH) {
                        game.guesses[game.current_attempt][len] = (char)(vk - 32);
                        game.guesses[game.current_attempt][len + 1] = '\0';
                        game.invalid_word = 0;
                    }
                }
            }
        } else {
            keyWasPressed[vk] = false;
        }
    }
}