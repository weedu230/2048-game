// ============================================================
//  2048 — C++ Console Backend
//  Compile: g++ -o 2048 2048.cpp
// ============================================================

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>

#ifdef _WIN32
  #include <windows.h>
  void clearScreen() { system("cls"); }
  void enableColor() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode; GetConsoleMode(h, &mode);
    SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  }
#else
  void clearScreen() { system("clear"); }
  void enableColor() {}
#endif

using namespace std;

// ─── CONSTANTS ───────────────────────────────────────────────
const int SIZE         = 4;
const int WINNING_TILE = 2048;

// ANSI colors
const string RESET  = "\033[0m";
const string BOLD   = "\033[1m";
const string DIM    = "\033[2m";

struct TileStyle { string bg; string fg; };
TileStyle tileStyle(int val) {
  switch(val) {
    case    2: return {"\033[43m",  "\033[30m"};   // yellow bg, black fg
    case    4: return {"\033[33m",  "\033[30m"};   // dark yellow
    case    8: return {"\033[38;5;214m\033[48;5;214m", "\033[30m"};
    case   16: return {"\033[48;5;202m", "\033[97m"};
    case   32: return {"\033[48;5;196m", "\033[97m"};
    case   64: return {"\033[48;5;200m", "\033[97m"};
    case  128: return {"\033[48;5;135m", "\033[97m"};
    case  256: return {"\033[48;5;63m",  "\033[97m"};
    case  512: return {"\033[48;5;45m",  "\033[97m"};
    case 1024: return {"\033[48;5;42m",  "\033[97m"};
    case 2048: return {"\033[48;5;226m", "\033[30m"};
    default:   return {"\033[48;5;240m", "\033[37m"};
  }
}

// ─── GAME STRUCTURE ──────────────────────────────────────────
struct Game {
  int board[SIZE][SIZE];
  int score;
  int prevBoard[SIZE][SIZE];
  int prevScore;
  bool hasPrev;
};

// ─── PROTOTYPES ──────────────────────────────────────────────
void initializeBoard(Game* g);
void addRandomTile(Game* g);
bool moveLeft(Game* g);
bool moveRight(Game* g);
bool moveUp(Game* g);
bool moveDown(Game* g);
bool isGameOver(Game* g);
bool isGameWon(Game* g);
void printBoard(Game* g);
void displayInstructions();
bool handleInput(Game* g);
void saveScore(const string& name, int score);
void displayScores();
void undoMove(Game* g);
void saveState(Game* g);

// ─── BOARD INIT ──────────────────────────────────────────────
void initializeBoard(Game* g) {
  g->score = 0; g->hasPrev = false;
  for (int i = 0; i < SIZE; ++i)
    for (int j = 0; j < SIZE; ++j)
      g->board[i][j] = 0;
  addRandomTile(g);
  addRandomTile(g);
}

void addRandomTile(Game* g) {
  int empty[SIZE*SIZE][2]; int cnt = 0;
  for (int i = 0; i < SIZE; ++i)
    for (int j = 0; j < SIZE; ++j)
      if (g->board[i][j] == 0) { empty[cnt][0]=i; empty[cnt][1]=j; ++cnt; }
  if (!cnt) return;
  int idx = rand() % cnt;
  g->board[empty[idx][0]][empty[idx][1]] = (rand() % 10 < 9) ? 2 : 4;
}

void saveState(Game* g) {
  for (int i = 0; i < SIZE; ++i)
    for (int j = 0; j < SIZE; ++j)
      g->prevBoard[i][j] = g->board[i][j];
  g->prevScore = g->score; g->hasPrev = true;
}

void undoMove(Game* g) {
  if (!g->hasPrev) { cout << "Nothing to undo!\n"; return; }
  for (int i = 0; i < SIZE; ++i)
    for (int j = 0; j < SIZE; ++j)
      g->board[i][j] = g->prevBoard[i][j];
  g->score = g->prevScore; g->hasPrev = false;
}

// ─── SLIDE HELPER ────────────────────────────────────────────
// Slide/merge an array of SIZE elements to the left; returns score gained
int slideLeft(int row[SIZE]) {
  int gain = 0;
  // Compact
  int tmp[SIZE] = {0}; int k = 0;
  for (int j = 0; j < SIZE; ++j) if (row[j]) tmp[k++] = row[j];
  // Merge
  for (int j = 0; j < SIZE-1; ++j) {
    if (tmp[j] && tmp[j] == tmp[j+1]) {
      tmp[j] *= 2; gain += tmp[j]; tmp[j+1] = 0;
      j++; // skip merged
    }
  }
  // Re-compact
  int out[SIZE] = {0}; k = 0;
  for (int j = 0; j < SIZE; ++j) if (tmp[j]) out[k++] = tmp[j];
  bool moved = false;
  for (int j = 0; j < SIZE; ++j) { if (row[j] != out[j]) moved = true; row[j] = out[j]; }
  return moved ? gain : -1; // -1 signals no change
}

// ─── MOVES ───────────────────────────────────────────────────
bool moveLeft(Game* g) {
  bool moved = false;
  for (int i = 0; i < SIZE; ++i) {
    int g2 = slideLeft(g->board[i]);
    if (g2 >= 0) { moved = true; g->score += g2; }
  }
  return moved;
}

bool moveRight(Game* g) {
  bool moved = false;
  for (int i = 0; i < SIZE; ++i) {
    int row[SIZE];
    for (int j = 0; j < SIZE; ++j) row[j] = g->board[i][SIZE-1-j];
    int g2 = slideLeft(row);
    if (g2 >= 0) {
      moved = true; g->score += g2;
      for (int j = 0; j < SIZE; ++j) g->board[i][SIZE-1-j] = row[j];
    }
  }
  return moved;
}

bool moveUp(Game* g) {
  bool moved = false;
  for (int j = 0; j < SIZE; ++j) {
    int col[SIZE];
    for (int i = 0; i < SIZE; ++i) col[i] = g->board[i][j];
    int g2 = slideLeft(col);
    if (g2 >= 0) {
      moved = true; g->score += g2;
      for (int i = 0; i < SIZE; ++i) g->board[i][j] = col[i];
    }
  }
  return moved;
}

bool moveDown(Game* g) {
  bool moved = false;
  for (int j = 0; j < SIZE; ++j) {
    int col[SIZE];
    for (int i = 0; i < SIZE; ++i) col[i] = g->board[SIZE-1-i][j];
    int g2 = slideLeft(col);
    if (g2 >= 0) {
      moved = true; g->score += g2;
      for (int i = 0; i < SIZE; ++i) g->board[SIZE-1-i][j] = col[i];
    }
  }
  return moved;
}

// ─── GAME STATE ──────────────────────────────────────────────
bool isGameOver(Game* g) {
  for (int i = 0; i < SIZE; ++i)
    for (int j = 0; j < SIZE; ++j) {
      if (!g->board[i][j]) return false;
      if (i < SIZE-1 && g->board[i][j] == g->board[i+1][j]) return false;
      if (j < SIZE-1 && g->board[i][j] == g->board[i][j+1]) return false;
    }
  return true;
}

bool isGameWon(Game* g) {
  for (int i = 0; i < SIZE; ++i)
    for (int j = 0; j < SIZE; ++j)
      if (g->board[i][j] >= WINNING_TILE) return true;
  return false;
}

// ─── DISPLAY ─────────────────────────────────────────────────
void printBoard(Game* g) {
  clearScreen();
  cout << "\n";
  cout << BOLD << "\033[38;5;226m  ██████╗ ██████╗ ██╗  ██╗ █████╗ \n";
  cout <<         "  ╚════██╗██╔═══██╗██║  ██║██╔══██╗\n";
  cout <<         "   █████╔╝██║   ██║███████║╚█████╔╝\n";
  cout <<         "  ██╔═══╝ ██║   ██║╚════██║██╔══██╗\n";
  cout <<         "  ███████╗╚██████╔╝     ██║╚█████╔╝\n";
  cout <<         "  ╚══════╝ ╚═════╝      ╚═╝ ╚════╝ \n" << RESET;
  cout << "\n";

  // Score bar
  cout << "  " << BOLD << "SCORE  " << RESET;
  cout << "\033[38;5;226m" << BOLD << setw(6) << g->score << RESET;
  cout << "  " << DIM << "│" << RESET << "\n\n";

  // Board border
  string hLine = "  ╔";
  for (int j = 0; j < SIZE; ++j) hLine += string(7,'═') + (j<SIZE-1?"╦":"╗");
  string midLine = "  ╠";
  for (int j = 0; j < SIZE; ++j) midLine += string(7,'═') + (j<SIZE-1?"╬":"╣");
  string botLine = "  ╚";
  for (int j = 0; j < SIZE; ++j) botLine += string(7,'═') + (j<SIZE-1?"╩":"╝");

  cout << hLine << "\n";
  for (int i = 0; i < SIZE; ++i) {
    cout << "  ║";
    for (int j = 0; j < SIZE; ++j) {
      int v = g->board[i][j];
      if (v == 0) {
        cout << "   " << DIM << "·" << RESET << "   ║";
      } else {
        auto [bg, fg] = tileStyle(v);
        string s = to_string(v);
        int pad = (6 - (int)s.size()) / 2;
        int rpad = 6 - (int)s.size() - pad;
        cout << bg << fg << BOLD;
        cout << string(pad,' ') << s << string(rpad,' ');
        cout << RESET << "║";
      }
    }
    cout << "\n";
    if (i < SIZE-1) cout << midLine << "\n";
  }
  cout << botLine << "\n\n";
  cout << "  " << DIM << "w/↑=Up  a/←=Left  s/↓=Down  d/→=Right  u=Undo  q=Quit\n" << RESET;
}

void displayInstructions() {
  clearScreen();
  cout << "\n\033[38;5;226m" << BOLD;
  cout << "  Welcome to 2048!\n\n" << RESET;
  cout << "  Combine tiles to reach \033[38;5;226m" << BOLD << "2048" << RESET << ".\n\n";
  cout << "  Controls:\n";
  cout << "    w / ↑   Move Up\n";
  cout << "    s / ↓   Move Down\n";
  cout << "    a / ←   Move Left\n";
  cout << "    d / →   Move Right\n";
  cout << "    u       Undo last move\n";
  cout << "    q       Quit\n\n";
  cout << "  Press Enter to start...";
  cin.ignore(); cin.get();
}

// ─── INPUT ───────────────────────────────────────────────────
bool handleInput(Game* g) {
  cout << "\n  Move: ";
  char mv;
  if (!(cin >> mv)) return false;
  if (mv == 'q' || mv == 'Q') return false;
  if (mv == 'u' || mv == 'U') { undoMove(g); return true; }

  bool valid = false;
  saveState(g);
  switch (mv) {
    case 'w': case 'A': valid = moveUp(g);    break;
    case 's': case 'B': valid = moveDown(g);  break;
    case 'a': case 'D': valid = moveLeft(g);  break;
    case 'd': case 'C': valid = moveRight(g); break;
    default: cout << "  Invalid key! Use w/a/s/d.\n"; return true;
  }
  if (valid) addRandomTile(g);
  else { g->hasPrev = false; cout << "  No movement!\n"; }
  return true;
}

// ─── SCOREBOARD ──────────────────────────────────────────────
void saveScore(const string& name, int score) {
  ofstream f("scores.txt", ios::app);
  if (f.is_open()) f << name << " : " << score << "\n";
  else cout << "  Error saving score!\n";
}

void displayScores() {
  struct Entry { string name; int score; };
  vector<Entry> entries;
  ifstream f("scores.txt");
  if (f.is_open()) {
    string line;
    while (getline(f, line)) {
      auto pos = line.rfind(" : ");
      if (pos != string::npos) {
        string name = line.substr(0, pos);
        int sc = stoi(line.substr(pos+3));
        entries.push_back({name, sc});
      }
    }
  }
  sort(entries.begin(), entries.end(), [](auto& a, auto& b){ return a.score > b.score; });

  cout << "\n\033[38;5;226m" << BOLD << "  ══ SCOREBOARD ══\n" << RESET;
  if (entries.empty()) { cout << "  No scores yet.\n"; return; }
  const string medals[] = {"🥇","🥈","🥉"};
  for (int i = 0; i < (int)min((int)entries.size(), 10); ++i) {
    string medal = (i < 3) ? medals[i] : "  ";
    cout << "  " << medal << " " << setw(15) << left << entries[i].name
         << "  " << BOLD << "\033[38;5;226m" << entries[i].score << RESET << "\n";
  }
}

// ─── MAIN ────────────────────────────────────────────────────
int main() {
  enableColor();
  srand(static_cast<unsigned>(time(0)));

  Game game;
  initializeBoard(&game);
  displayInstructions();

  bool won = false;
  while (true) {
    printBoard(&game);
    if (isGameWon(&game) && !won) {
      won = true;
      cout << "\n  \033[38;5;226m" << BOLD << "🎉 YOU REACHED 2048! Keep going!\n" << RESET;
    }
    if (isGameOver(&game)) {
      cout << "\n  GAME OVER! Final score: " << BOLD << "\033[38;5;226m" << game.score << RESET << "\n\n";
      break;
    }
    if (!handleInput(&game)) break;
  }

  printBoard(&game);
  cout << "\n  Final score: " << BOLD << "\033[38;5;226m" << game.score << RESET << "\n";
  cout << "  Enter your name to save: ";
  cin.ignore();
  string name; getline(cin, name);
  if (!name.empty()) saveScore(name, game.score);
  displayScores();

  cout << "\n  Thanks for playing! Press Enter to exit...";
  cin.get();
  return 0;
}
