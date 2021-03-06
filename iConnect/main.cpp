#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const int SQUARE_WIDTH              =50;
const int SQUARE_HEIGHT             =50;

const int WINDOW_SQUARE_WIDTH       =50;
const int WINDOW_SQUARE_HEIGHT      =50;

const int DEFAULT_ROWS              =10;
const int DEFAULT_COLS              =10;
const int DEFAULT_SQUARES           =8;

const string SCREEN_TITLE           = "iConnect";
const string SQUARE_WHITE           = "white.jpg";
const string SQUARE_BLACK           = "black.jpg";
const string BACKGROUND             = "background.jpg";
const string TEXT_FONT              = "Font.ttf";
const string CORRECT_SOUND          = "correct.wav";
const string INCORRECT_SOUND        = "incorrect.wav";
const string BGMUSIC                = "FutariNoKimochi.mp3";



enum CellState {
    CELL_WHITE,
    CELL_BLACK,
    CELL_EATEN
};

enum GameState {
    GAME_PLAYING,
    GAME_WON
};

enum SquareType{
    BLACK_1,       WHITE_1,
    BLACK_2,       WHITE_2,
    BLACK_3,       WHITE_3,
    BLACK_4,       WHITE_4,
    BLACK_5,       WHITE_5,
    BLACK_6,       WHITE_6,
    BLACK_7,       WHITE_7,
    BLACK_8,       WHITE_8,
    BLACK_9,       WHITE_9,

    SQUARE_TOTAL
};

struct Cell {
    int value;
    CellState state;
};

typedef vector<vector<Cell> > Table;

struct CellPos {
    int i;
    int j;
};

struct Game {
    int nRows;
    int nCols;
    int nEaten;
    Table cells;
    CellPos lastPos;
    GameState state;
    vector <CellPos> pts;
};

struct Graphic {
    SDL_Window   *window;
    SDL_Texture  *texture;
    SDL_Texture  *texwhite;
    SDL_Texture  *texblack;
    SDL_Renderer *renderer;
};

struct Text {
    TTF_Font *font;
    string str;
    SDL_Color color;
    SDL_Texture *texture;
    SDL_Rect rect;
};

struct Audio {
    Mix_Chunk *correct;
    Mix_Chunk *incorrect;
    Mix_Music *music;
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool initGraphic(Graphic &g, int nRows, int nCols);
bool initText(Text &text, SDL_Renderer *renderer);
bool initAudio(Audio &au);
void finalizeGraphic_Text_Audio(Graphic &g, Text &t, Audio &au);
SDL_Texture* createTexture(SDL_Renderer* renderer, const string &path);
void err(const string &mes);

void initRect(vector<SDL_Rect> &rects);
void initGame(Game &Game, int nRows, int nCols, int nSquare);
void randomSquares(Table &cells);

void drawText(Text &text, SDL_Renderer *renderer);
void drawTextWin(Text &text, SDL_Renderer *renderer);
void drawTable(Game &game, const Graphic &graphic, const vector<SDL_Rect> rects, Text&);

void updateGame(Game &game, const SDL_Event &event, Audio&);
void processGame(Game &game, CellPos &pos, Audio&);
void resetGame(Table&);

bool checkGame(Game&, CellPos&, CellPos&);
CellPos getPoint(int&, int&);
bool check1Line(Table&, CellPos&, CellPos&);
int  check2Lines(Table&, CellPos&, CellPos&);
int check3LinesX(Table&, CellPos&, CellPos&);
int check3LinesY(Table&, CellPos&, CellPos&);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* arvg[]){
    srand(time(0));
    int nRows     = DEFAULT_ROWS,
        nCols     = DEFAULT_COLS,
        nSquares  = DEFAULT_SQUARES;

    Graphic graphic;
    Text text;
    Audio audio;
    if (!initGraphic(graphic, nRows, nCols) || !initText(text, graphic.renderer) || !initAudio(audio)) {
        finalizeGraphic_Text_Audio(graphic, text, audio);
        return EXIT_FAILURE;
    }

    Mix_PlayMusic(audio.music, -1);

    vector<SDL_Rect> rects;
    initRect(rects);
    Game game;
    initGame(game, nRows, nCols, nSquares);

    bool quit=false;
    SDL_Event event;
    while (!quit){
        drawTable(game, graphic, rects, text);

        while (SDL_PollEvent(&event) != 0){
            if (event.type == SDL_QUIT){
                quit=true;
                break;
            }

            updateGame(game, event, audio);

        }
    }

    finalizeGraphic_Text_Audio(graphic, text, audio);
    return EXIT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool initGraphic(Graphic &g, int nRows, int nCols) {
    g.window   = NULL;
    g.renderer = NULL;
    g.texture  = NULL;
    g.texwhite = NULL;
    g.texblack = NULL;

    int SDL_flags=SDL_INIT_VIDEO | SDL_INIT_AUDIO;
    if (SDL_Init(SDL_flags) != 0) {
        err("Khởi tạo SDL thất bại. Hãy kiểm tra lại.");
        return false;
    }

    int IMG_flags=IMG_INIT_JPG;
    if (!(IMG_Init(IMG_flags) & IMG_flags)) {
        err("Khởi tạo SDL_image thất bại. Hãy kiểm tra lại.");
        return false;
    }

    g.window = SDL_CreateWindow(SCREEN_TITLE.c_str(),
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                DEFAULT_ROWS*52-2,
                                DEFAULT_COLS*52-2+3g0,
                                SDL_WINDOW_SHOWN);
    if (g.window==NULL){
        err("Tạo Window thất bại. Hãy kiểm tra lại.");
        return false;
    }

    g.renderer = SDL_CreateRenderer(g.window, -1, SDL_RENDERER_ACCELERATED);
    if (g.renderer == NULL) {
        err("Tạo Renderer thất bại. Hãy kiểm tra lại.");
        return false;
    }

    g.texture=createTexture(g.renderer, BACKGROUND);
    if (g.texture==NULL){
        err("Tạo Texture từ " + BACKGROUND + " thất bại. Hãy kiểm tra lại.");
        return false;
    }
    g.texwhite=createTexture(g.renderer, SQUARE_WHITE);
    if (g.texwhite==NULL){
        err("Tạo Texture từ " + SQUARE_WHITE + " thất bại. Hãy kiểm tra lại.");
        return false;
    }
    g.texblack=createTexture(g.renderer, SQUARE_BLACK);
    if (g.texblack==NULL){
        err("Tạo Texture từ " + SQUARE_BLACK + " thất bại. Hãy kiểm tra lại.");
        return false;
    }

    return true;
}

SDL_Texture* createTexture(SDL_Renderer* renderer, const string &path){
    SDL_Surface *surface = IMG_Load(path.c_str());
    if (surface==NULL){
        err("Không tải được " + path + " ! "+ IMG_GetError());
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

bool initText(Text &text, SDL_Renderer *renderer){
    if (TTF_Init() != 0) {
        err("Khởi tạo SDL_ttf thất bại. Hãy kiểm tra lại.");
        return false;
    }
    text.font=NULL;
    text.font=TTF_OpenFont(TEXT_FONT.c_str(), 30);
    if(text.font==NULL) {
        err(TTF_GetError());
        return false;
    }

    text.texture  = NULL;
    text.color    = (SDL_Color) {255, 135, 135};
    text.rect     = (SDL_Rect)  {350, 10, 100, 30};
    text.str      = "";

	return true;
}

void drawText(Text &text, SDL_Renderer *renderer){
    text.str = "Time: ";
    Uint32 time = SDL_GetTicks()/1000;
    ostringstream convert;
    convert << time;
    string strtime = convert.str();
    text.str += strtime + "s";

    SDL_Surface *surface = TTF_RenderText_Solid(text.font, text.str.c_str(), text.color);
    text.texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, text.texture, NULL, &text.rect);
}

void drawTextWin(Text &text, SDL_Renderer *renderer){
    text.str  = "You Win!";
    text.rect = (SDL_Rect)  {150, 150, 200, 100};
    SDL_Surface *surface = TTF_RenderText_Solid(text.font, text.str.c_str(), text.color);
    text.texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, text.texture, NULL, &text.rect);
}

bool initAudio(Audio &au){
    au.correct   = NULL;
    au.incorrect = NULL;
    au.music     = NULL;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1){
        err(Mix_GetError());
        return false;
    }

    au.correct = Mix_LoadWAV(CORRECT_SOUND.c_str());
    if (au.correct == NULL){
        err("Không tải được " + CORRECT_SOUND + " !");
        return false;
    }

    au.incorrect = Mix_LoadWAV(INCORRECT_SOUND.c_str());
    if (au.incorrect == NULL){
        err("Không tải được " + INCORRECT_SOUND + " !");
        return false;
    }

    au.music = Mix_LoadMUS(BGMUSIC.c_str());
    if (au.music == NULL){
        err("Không tải được " + BGMUSIC + " !");
        return false;
    }

    return true;
}

void finalizeGraphic_Text_Audio(Graphic &g, Text &t, Audio &a) {
    SDL_DestroyTexture(g.texture);
    SDL_DestroyTexture(g.texwhite);
    SDL_DestroyTexture(g.texblack);
    SDL_DestroyRenderer(g.renderer);
    SDL_DestroyWindow(g.window);
    SDL_DestroyTexture(t.texture);
    TTF_CloseFont(t.font);
    Mix_CloseAudio();

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void initRect(vector<SDL_Rect> &rects){
    for (int i=0; i<SQUARE_TOTAL/2; i++){
        SDL_Rect rect={0, 0, SQUARE_WIDTH, SQUARE_HEIGHT};
        rect.x=(i/3)*SQUARE_WIDTH;
        rect.y=(i%3)*SQUARE_HEIGHT;
        rects.push_back(rect);
    }
}

void initGame(Game &game, int nRows, int nCols, int nSquares){
    game.cells = Table(nRows, vector<Cell> (nCols));
    for (int i=0; i<nRows; i++){
        for (int j=0; j<nCols; j++){
            game.cells[i][j]=(Cell) {0, CELL_EATEN};
        }
    }
    randomSquares(game.cells);
    game.nRows   = nRows;
    game.nCols   = nCols;
    game.nEaten  = 0;
    game.lastPos = (CellPos) {0, 0};
    game.state   = GAME_PLAYING;
    game.pts.clear();
}

void randomSquares(Table &cells){
    int maxVal=(DEFAULT_ROWS-2)*(DEFAULT_COLS-2),
        nSquares=DEFAULT_SQUARES;
    vector<int> num(nSquares, 0);
    for (int i=0; i<nSquares; i++){
        if (i==nSquares-1){
            num[i]=maxVal;
        } else {
        num[i]=(maxVal)/(nSquares-i);
        maxVal-=num[i];
        }
    }
    int value;
    for (int i=1; i<DEFAULT_ROWS-1; i++){
        for (int j=1; j<DEFAULT_COLS-1; j++){
            do {
                value=rand()%nSquares+1;
            } while (num[value-1]==0);
            num[value-1]--;
            cells[i][j].value=value;
            cells[i][j].state=CELL_WHITE;
        }
    }
}

void drawTable(Game &game, const Graphic &graphic, const vector<SDL_Rect> rects, Text &text) {
    SDL_RenderClear(graphic.renderer);
    SDL_RenderCopy(graphic.renderer, graphic.texture, NULL, NULL);
    if (game.state == GAME_PLAYING){
        drawText(text, graphic.renderer);
    }
    else {
        drawTextWin(text, graphic.renderer);
    }
    for (int i=1; i<game.nRows-1; i++){
        for (int j=1; j<game.nCols-1; j++){
            int value=game.cells[i][j].value;
            int state=game.cells[i][j].state;
            SDL_Rect dstRect = {j*WINDOW_SQUARE_WIDTH+2+(j-1)*2,
                                i*WINDOW_SQUARE_HEIGHT+30+2+(i-1)*2,
                                WINDOW_SQUARE_WIDTH,
                                WINDOW_SQUARE_HEIGHT};
            SDL_Rect srcRect=rects[value-1];
            if (state==CELL_WHITE){
                SDL_RenderCopy(graphic.renderer, graphic.texwhite, &srcRect, &dstRect);
            }
            if (state==CELL_BLACK){
                SDL_RenderCopy(graphic.renderer, graphic.texblack, &srcRect, &dstRect);
            }
        }
    }

    if (!game.pts.empty()) {
        for (int i=0; i<game.pts.size()-1; i++){
            SDL_RenderDrawLine(graphic.renderer, game.pts[i].i, game.pts[i].j, game.pts[i+1].i, game.pts[i+1].j);
        }
        game.pts.clear();
        SDL_RenderPresent(graphic.renderer);
        SDL_Delay(500);
    }

    SDL_RenderPresent(graphic.renderer);
}

void updateGame(Game &game, const SDL_Event &event, Audio &audio){
    if (game.state != GAME_PLAYING) return;

    if (event.type != SDL_MOUSEBUTTONDOWN) return;

    SDL_MouseButtonEvent mouse=event.button;
    int square=WINDOW_SQUARE_WIDTH+2;
    if (!((square<=mouse.x && mouse.x<square*(DEFAULT_COLS-1)) &&
          (square+30<=mouse.y && mouse.y<=square*(DEFAULT_ROWS-1)+30))){
        return;
    }

    CellPos pos = (CellPos) {(mouse.y-30)/square, (mouse.x)/square};
    CellState state = game.cells[pos.i][pos.j].state;
    if (state==CELL_EATEN || state==CELL_BLACK){
        return;
    }
    processGame(game, pos, audio);
}

void processGame(Game &game, CellPos &pos, Audio &audio){
    int rows=DEFAULT_ROWS-2;
    int cols=DEFAULT_COLS-2;
    bool check=false;
    for (int i1=1; i1<=rows; i1++){
    for (int j1=1; j1<=cols; j1++){
        for (int i2=1; i2<=rows; i2++){
        for (int j2=1; j2<=cols; j2++){
            if ( (i1 != i2 || j1 != j2) && (game.cells[i1][j1].state == CELL_WHITE && game.cells[i2][j2].state == CELL_WHITE) ){
                game.cells[i1][j1].state = CELL_BLACK;
                game.cells[i2][j2].state = CELL_BLACK;
                CellPos pos1 = (CellPos) {i1, j1};
                CellPos pos2 = (CellPos) {i2, j2};
                if (checkGame(game, pos1, pos2) == true) {
                    game.pts.clear();
                    check=true;
                }
                game.cells[i1][j1].state = CELL_WHITE;
                game.cells[i2][j2].state = CELL_WHITE;
            }
        }
        }
    }
    }
    if (!check) resetGame(game.cells);


    int maxVal=(game.nRows-2)*(game.nCols-2);
    CellPos last = game.lastPos;
    game.cells[pos.i][pos.j].state = CELL_BLACK;
    if (last.i == 0) {
        game.lastPos = pos;
        return;
    }

    if (checkGame(game, last, pos)) {
        game.cells[last.i][last.j].state = CELL_EATEN;
        game.cells[pos.i][pos.j].state = CELL_EATEN;
        game.nEaten+=2;
        Mix_PlayChannelTimed(-1, audio.correct, 1, 1000);
    }
    else {
        game.cells[last.i][last.j].state = CELL_WHITE;
        game.cells[pos.i][pos.j].state = CELL_WHITE;
        Mix_PlayChannelTimed(-1, audio.incorrect, 1, 500);
    }

    game.lastPos.i=0;
    if (game.nEaten == maxVal){
        game.state=GAME_WON;
        return;
    }


}

bool checkGame(Game &game, CellPos &pos1, CellPos &pos2){
    game.pts.clear();
    if (game.cells[pos1.i][pos1.j].value != game.cells[pos2.i][pos2.j].value) {
        return false;
    }
    if (check1Line(game.cells, pos1, pos2)){
        game.pts.push_back(getPoint(pos1.i, pos1.j));
        game.pts.push_back(getPoint(pos2.i, pos2.j));
        return true;
    }

    if (check2Lines(game.cells, pos1, pos2) == 1){
        game.pts.push_back(getPoint(pos1.i, pos1.j));
        game.pts.push_back(getPoint(pos1.i, pos2.j));
        game.pts.push_back(getPoint(pos2.i, pos2.j));
        return true;
    }

    if (check2Lines(game.cells, pos1, pos2) == 2){
        game.pts.push_back(getPoint(pos1.i, pos1.j));
        game.pts.push_back(getPoint(pos2.i, pos1.j));
        game.pts.push_back(getPoint(pos2.i, pos2.j));
        return true;
    }

    int j=check3LinesX(game.cells, pos1, pos2);
    if (j!=-1) {
        game.pts.push_back(getPoint(pos1.i, pos1.j));
        game.pts.push_back(getPoint(pos1.i, j));
        game.pts.push_back(getPoint(pos2.i, j));
        game.pts.push_back(getPoint(pos2.i, pos2.j));
        return true;
    }

    int i=check3LinesY(game.cells, pos1, pos2);
    if (i!=-1) {
        game.pts.push_back(getPoint(pos1.i, pos1.j));
        game.pts.push_back(getPoint(i, pos1.j));
        game.pts.push_back(getPoint(i, pos2.j));
        game.pts.push_back(getPoint(pos2.i, pos2.j));
        return true;
    }

    return false;
}

CellPos getPoint(int &i, int &j){
    CellPos point;
    point.i = j*52+26;
    point.j = i*52+26+30;
    return point;
}

/// Đi theo đường chữ I
bool check1Line(Table &cells, CellPos &pos1, CellPos &pos2){
    if (pos1.i != pos2.i && pos1.j != pos2.j) return false;

    if (pos1.i==pos2.i){
        int i=pos1.i;
        if (pos1.j<pos2.j){
            for (int j=pos1.j; j<=pos2.j; j++){
                if (cells[i][j].state == CELL_WHITE ) return false;
            }
            return true;
        }
        else {
            for (int j=pos2.j; j<=pos1.j; j++){
                if (cells[i][j].state == CELL_WHITE) return false;
            }
            return true;
        }
    }

    if (pos1.j==pos2.j){
        int j=pos1.j;
        if (pos1.i<pos2.i){
            for (int i=pos1.i; i<=pos2.i; i++){
                if (cells[i][j].state == CELL_WHITE) return false;
            }
            return true;
        }
        else {
            for (int i=pos2.i; i<=pos1.i; i++){
                if (cells[i][j].state == CELL_WHITE) return false;
            }
            return true;
        }
    }
    return true;
}

/// Đi theo đường chữ L
int check2Lines(Table &cells, CellPos &pos1, CellPos &pos2){
    CellPos point1 = (CellPos) {pos1.i, pos2.j},
            point2 = (CellPos) {pos2.i, pos1.j};
    if (check1Line(cells, pos1, point1) && check1Line(cells, pos2, point1)) return 1;
    if (check1Line(cells, pos1, point2) && check1Line(cells, pos2, point2)) return 2;
    return -1;
}

/// Đi theo đường chữ Z hoặc U
int check3LinesX(Table &cells, CellPos &pos1, CellPos &pos2){
    CellPos pMax=pos1,
            pMin=pos2;
    if (pos1.j<pos2.j){
        pMax=pos2;
        pMin=pos1;
    }

    for (int j=pMin.j+1; j<pMax.j; j++){               // Đi chữ Z
        CellPos point1=(CellPos) {pMin.i, j},
                point2=(CellPos) {pMax.i, j};
        if (check1Line(cells, pMin, point1) &&
            check1Line(cells, point1, point2) &&
            check1Line(cells, point2, pMax)) return j;
    }

    for (int j=pMin.j-1; j>=0; j--){                    // Đi chữ U
        CellPos point1=(CellPos) {pMin.i, j},
                point2=(CellPos) {pMax.i, j};
        if (check1Line(cells, pMin, point1) &&
            check1Line(cells, point1, point2) &&
            check1Line(cells, point2, pMax)) return j;
    }
    for (int j=pMax.j+1; j<DEFAULT_COLS; j++){          // Đi chữ U
        CellPos point1=(CellPos) {pMin.i, j},
                point2=(CellPos) {pMax.i, j};
        if (check1Line(cells, pMin, point1) &&
            check1Line(cells, point1, point2) &&
            check1Line(cells, point2, pMax)) return j;
    }
    return -1;
}

int check3LinesY(Table &cells, CellPos &pos1, CellPos &pos2){
    CellPos pMax=pos1,
            pMin=pos2;
    if (pos1.i<pos2.i){
        pMax=pos2;
        pMin=pos1;
    }

    for (int i=pMin.i+1; i<pMax.i; i++){               // Đi chữ Z
        CellPos point1=(CellPos) {i, pMin.j},
                point2=(CellPos) {i, pMax.j};
        if (check1Line(cells, pMin, point1) &&
            check1Line(cells, point1, point2) &&
            check1Line(cells, point2, pMax)) return i;
    }

    for (int i=pMin.i-1; i>=0; i--){                    // Đi chữ U
        CellPos point1=(CellPos) {i, pMin.j},
                point2=(CellPos) {i, pMax.j};
        if (check1Line(cells, pMin, point1) &&
            check1Line(cells, point1, point2) &&
            check1Line(cells, point2, pMax)) return i;
    }
    for (int i=pMax.i+1; i<DEFAULT_ROWS; i++){          // Đi chữ U
        CellPos point1=(CellPos) {i, pMin.j},
                point2=(CellPos) {i, pMax.j};
        if (check1Line(cells, pMin, point1) &&
            check1Line(cells, point1, point2) &&
            check1Line(cells, point2, pMax)) return i;
    }
    return -1;
}

void resetGame(Table &cells){
    vector<int> num(DEFAULT_SQUARES, 0);
    for (int i=1; i<DEFAULT_ROWS-1; i++){
        for (int j=1; j<DEFAULT_COLS-1; j++){
            if (cells[i][j].state == CELL_WHITE){
                num[cells[i][j].value-1]++;
            }
        }
    }

    int value,
        nSquares=DEFAULT_SQUARES;
    for (int i=1; i<DEFAULT_ROWS-1; i++){
        for (int j=1; j<DEFAULT_COLS-1; j++){
            if (cells[i][j].state == CELL_WHITE){
                do {
                    value=rand()%nSquares+1;
                } while (num[value-1]==0);
                num[value-1]--;
                cells[i][j].value=value;
            }
        }
    }
}

void err(const string &mes){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Lỗi", mes.c_str(), NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////                                           11:20  27/5/2020 Trangg
