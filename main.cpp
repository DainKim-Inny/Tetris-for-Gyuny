#include <iostream>
#include <conio.h> // _kbhit(), _getch() 사용
#include <windows.h> // Sleep(), SetConsoleCursorPosition 사용
#include <ctime> // rand() 사용
#include <mmsystem.h> // PlaySound() 사용
#pragma comment(lib, "winmm.lib") // PlaySound() 사용을 위한 라이브러리 링크
using namespace std;

const int width = 15;
const int height = 15;
int field[height][width] = { 0 };
int score = 0; // 점수 초기화

// 테트리스 블록 모양 정의 (4x4 크기)
int tetromino[7][4][4] = {
    {{1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // I
    {{1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // O
    {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // S
    {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // Z
    {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // J
    {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // L
    {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}  // T
};

// 현재 블록 상태
int currentTetromino[4][4];
int currentX = width / 2 - 2;
int currentY = 0;
bool isBlockFixed = false; // 현재 블록이 고정되었는지 여부
bool gameOverFlag = false; // 게임 오버 상태 플래그
bool congratulated = false; // 축하 메시지 표시 여부
bool waitingForNextLevel = false; // 다음 레벨 대기 상태 플래그

// 콘솔 창의 커서를 숨기는 함수
void hideCursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(console, &cursorInfo);
}

// 커서를 콘솔의 특정 위치로 이동하는 함수
void setCursorPosition(int x, int y) {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD position = { (short)x, (short)y };
    SetConsoleCursorPosition(console, position);
}

// 블록 복사 함수
void copyTetromino(int index) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            currentTetromino[y][x] = tetromino[index][y][x];
        }
    }
}

// 맵과 테두리 출력, 점수 표시 함수
void draw() {
    setCursorPosition(0, 0); // 커서를 콘솔 창의 첫 번째 위치로 이동

    // 상단 제목 출력 (중앙 정렬)
    string title = "INNY's TETRIS";
    int titlePosition = (width + 2 - title.length()) / 2; // 중앙 정렬
    cout << string(titlePosition, ' ') << title << endl;

    // 상단 테두리 출력
    for (int x = 0; x < width + 2; x++) {
        cout << "■";
    }
    cout << "   Score: " << score << endl; // 점수 출력

    // 게임 필드와 양옆 테두리 출력
    for (int y = 0; y < height; y++) {
        cout << "■"; // 왼쪽 테두리
        for (int x = 0; x < width; x++) {
            if (field[y][x] == 1) {
                cout << "■";
            }
            else {
                bool isTetrominoPart = false;
                for (int ty = 0; ty < 4; ty++) {
                    for (int tx = 0; tx < 4; tx++) {
                        if (currentTetromino[ty][tx] == 1 && currentY + ty == y && currentX + tx == x) {
                            cout << "■";
                            isTetrominoPart = true;
                        }
                    }
                }
                if (!isTetrominoPart) {
                    cout << " ";
                }
            }
        }
        cout << "■" << endl; // 오른쪽 테두리
    }

    // 하단 테두리 출력
    for (int x = 0; x < width + 2; x++) {
        cout << "■";
    }
    cout << endl;

    // 게임 오버 문구 표시
    if (gameOverFlag) {
        string failMessage = "FAIL";
        string retryMessage = "RETRY (Y/N)";
        int centerXFail = (width + 2 - failMessage.length()) / 2;
        int centerXRetry = (width + 2 - retryMessage.length()) / 2;
        setCursorPosition(centerXFail, height / 2);
        cout << failMessage << endl;
        setCursorPosition(centerXRetry, height / 2 + 1);
        cout << retryMessage << endl; // Retry 메시지 표시
    }

    // 축하 메시지 표시
    if (congratulated) {
        string congratsMessage = "CONGRATULATION!";
        string nextLevelMessage = "NEXT STAGE?(Y/N)";
        int centerXCongrats = (width + 2 - congratsMessage.length()) / 2;
        int centerXNextLevel = (width + 2 - nextLevelMessage.length()) / 2;
        setCursorPosition(centerXCongrats, height / 2);
        cout << congratsMessage << endl;
        setCursorPosition(centerXNextLevel, height / 2 + 1);
        cout << nextLevelMessage << endl; // Next Level 메시지 표시

        // Y/N 입력 처리
        char input = _getch();
        if (input == 'y' || input == 'Y') {
            
            // 기존 문구 삭제
            setCursorPosition(centerXCongrats, height / 2);
            cout << string(congratsMessage.length(), ' '); // 축하 메시지 지우기
            setCursorPosition(centerXNextLevel, height / 2 + 1);
            cout << string(nextLevelMessage.length(), ' '); // 다음 단계 메시지 지우기

            // 결혼 제안 메시지 출력
            waitingForNextLevel = true;
            string marriageMessage1 = "    WILL YOU";
            string marriageMessage2 = " MARRY ME? (Y/N)";
            int centerXMarriage1 = (width + 2 - marriageMessage1.length()) / 2;
            int centerXMarriage2 = (width + 2 - marriageMessage2.length()) / 2;
            setCursorPosition(centerXMarriage1, height/2); // 중앙에 출력
            setCursorPosition(centerXMarriage2, height/2); // 중앙에 출력
            cout << marriageMessage1 << endl;
            cout << marriageMessage2 << endl;

            // 다시 Y/N 입력 처리 및 하트 처리
            char marriageInput = _getch();
            if (marriageInput == 'y' || marriageInput == 'Y') {
                
                // BGM 중지
                PlaySound(NULL, 0, 0);

                // Wedding 사운드 재생
                PlaySound(TEXT("Wedding.wav"), NULL, SND_FILENAME | SND_ASYNC);

                // 기존 문구 지우기
                for (int i = 0; i < height; ++i) {
                    setCursorPosition(0, i);
                    for (int j = 0; j < width + 2; ++j) cout << " "; // 빈 공간으로 덮기
                }
                
                // 하트를 맵 중앙에 그리기
                setCursorPosition(0, 0);
                cout << "□□□□□□■■■□□□□□□□□□□□□■■■□□□□□□" << endl;
                cout << "□□□□□■■■■■□□□□□□□□□□■■■■■□□□□□" << endl;
                cout << "□□□□■■■■■■■□□□□□□□□■■■■■■■□□□□" << endl;
                cout << "□□□■■■■■■■■■□□□□□□■■■■■■■■■□□□" << endl;
                cout << "□□■■■■■■■■■■■□□□□■■■■■■■■■■■□□" << endl;
                cout << "□■■■■■■■■■■■■■□□■■■■■■■■■■■■■□" << endl;
                cout << "■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■" << endl;
                cout << "■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■" << endl;
                cout << "■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■" << endl;
                cout << "■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■" << endl;
                cout << "■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■" << endl;
                cout << "■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■" << endl;
                cout << "■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■" << endl;
                cout << "□■■■■■■■■■■■■■■■■■■■■■■■■■■■■□" << endl;
                cout << "□□□■■■■■■■■■■■■■■■■■■■■■■■■□□□" << endl;
                cout << "□□□□□■■■■■■■■■■■■■■■■■■■■□□□□□" << endl;
                cout << "□□□□□□□■■■■■■■■■■■■■■■■□□□□□□□" << endl;
                cout << "□□□□□□□□□■■■■■■■■■■■■□□□□□□□□□" << endl;
                cout << "□□□□□□□□□□□■■■■■■■■□□□□□□□□□□□" << endl;
                cout << "□□□□□□□□□□□□□■■■■□□□□□□□□□□□□□" << endl;
                cout << "□□□□□□□□□□□□□□■■□□□□□□□□□□□□□□" << endl;
                
                _getch();

                // 하트가 그려진 후 더 이상 다른 메세지를 출력하지 않음
                return;  // 게임 종료
            }
            else if (marriageInput == 'n' || marriageInput == 'N') {
                return;  // 게임 종료
            }
        }
        else if (input == 'n' || input == 'N') {
            return;  // 게임 종료
        }
    }
}

// 충돌 감지 함수
bool checkCollision(int dx, int dy) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentTetromino[y][x] == 1) {
                int newX = currentX + x + dx;
                int newY = currentY + y + dy;
                if (newX < 0 || newX >= width || newY >= height || (newY >= 0 && field[newY][newX] == 1)) {
                    return true;
                }
            }
        }
    }
    return false;
}

// 블록을 필드에 고정하는 함수
void placeTetromino() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentTetromino[y][x] == 1) {
                field[currentY + y][currentX + x] = 1;
            }
        }
    }
}

// 라인 제거 함수
void removeLine() {
    for (int y = height - 1; y >= 0; y--) {
        bool fullLine = true;
        for (int x = 0; x < width; x++) {
            if (field[y][x] == 0) {
                fullLine = false;
                break;
            }
        }
        if (fullLine) {
            // 점수 추가
            score += 10;

            for (int y2 = y; y2 > 0; y2--) {
                for (int x = 0; x < width; x++) {
                    field[y2][x] = field[y2 - 1][x];
                }
            }
            y++; // 한 줄 삭제 후 재확인
        }
    }
}

// 블록 회전 함수
void rotate() {
    int temp[4][4] = { 0 };
    // 회전
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            temp[y][x] = currentTetromino[3 - x][y]; // 90도 회전
        }
    }

    // 회전 가능 여부 확인
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (temp[y][x] == 1) {
                int newX = currentX + x;
                int newY = currentY + y;
                if (newX < 0 || newX >= width || newY >= height || field[newY][newX] == 1) {
                    return; // 회전 불가능
                }
            }
        }
    }

    // 회전 가능할 경우, 현재 블록 업데이트
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            currentTetromino[y][x] = temp[y][x];
        }
    }
}

// 게임 루프
void gameLoop() {
    // WAV 파일 재생 (게임 루프 시작 시 BGM 재생)
    PlaySound(TEXT("BGM.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    
    srand(static_cast<unsigned int>(time(0))); // 난수 초기화
    copyTetromino(rand() % 7); // 첫 블록 랜덤으로 선택

    while (!gameOverFlag) {
        // 입력 처리
        while (_kbhit()) {
            char input = _getch();
            if (input == 75) { // 왼쪽 화살표
                if (!checkCollision(-1, 0)) {
                    currentX--;
                }
            }
            else if (input == 77) { // 오른쪽 화살표
                if (!checkCollision(1, 0)) {
                    currentX++;
                }
            }
            else if (input == 80) { // 아래 화살표
                if (!checkCollision(0, 1)) {
                    currentY++;
                }
            }
            else if (input == 72) { // 위 화살표 (회전)
                rotate();
                if (checkCollision(0, 0)) {
                    // 원래 상태로 되돌리기
                    rotate();
                    rotate();
                    rotate();
                }
            }
            else if (input == 32) { // 스페이스바 (빠른 하강)
                while (!checkCollision(0, 1)) {
                    currentY++;
                }
            }

            // 게임 오버 상태에서 Y/N 입력 처리
            if (gameOverFlag) {      
                if (input == 'y' || input == 'Y') {
                    // 재시작 로직
                    score = 0; // 점수 초기화
                    gameOverFlag = false; // 게임 오버 플래그 해제
                    for (int y = 0; y < height; y++) {
                        for (int x = 0; x < width; x++) {
                            field[y][x] = 0; // 필드 초기화
                        }
                    }
                    congratulated = false; // 축하 메시지 표시 여부 초기화
                    gameLoop(); // 게임 루프 재시작
                }
                else if (input == 'n' || input == 'N') {
                    return; // 게임 종료
                }
            }

            // 다음 레벨 대기 상태에서 Y/N 입력 처리
            if (waitingForNextLevel) {
                if (input == 'y' || input == 'Y') {
                    // 결혼 제안 수락
                    for (int y = 0; y < height; y++) {
                        for (int x = 0; x < width; x++) {
                            field[y][x] = 1; // 필드 채우기
                        }
                    }
                    return; // 게임 종료
                }
                else if (input == 'n' || input == 'N') {
                    return; // 게임 종료
                }
            }
        }

        // 중력 적용
        if (!checkCollision(0, 1)) {
            currentY++;
        }
        else {
            isBlockFixed = true;
        }

        if (isBlockFixed) {
            placeTetromino();
            removeLine(); // 라인 제거

            // 블록이 필드 최상단에 닿았는지 확인하여 게임 오버 상태 설정
            if (currentY <= 0) {
                gameOverFlag = true; // 게임 종료

                // BGM 중지
                PlaySound(NULL, 0, 0);

                // Gameover 사운드 한 번 재생
                PlaySound(TEXT("Gameover.wav"), NULL, SND_FILENAME | SND_SYNC);
            }

            // 점수 50점 이상 시 축하 메시지 표시 및 게임 멈춤
            if (score >= 50 && !congratulated) {
                
                // BGM 중지
                PlaySound(NULL, 0, 0);

                // Success 사운드 재생
                PlaySound(TEXT("Success.wav"), NULL, SND_FILENAME | SND_ASYNC);
                
                congratulated = true; // 축하 메시지 표시
                waitingForNextLevel = true; // 다음 레벨 대기 상태로 변경
                draw(); // 화면 업데이트
                Sleep(200); // 잠시 대기
                return; // 게임 멈춤
            }

            currentX = width / 2 - 2; // 새로운 블록 위치 초기화
            currentY = 0;
            copyTetromino(rand() % 7); // 새로운 블록 랜덤으로 선택
            isBlockFixed = false; // 블록 고정 해제
        }

        // 게임 상태 출력
        draw();
        Sleep(200); // 화면 업데이트 속도 조절
    }

    // 게임 오버 후 입력 대기
    while (gameOverFlag) {
        if (_kbhit()) {
            char input = _getch();
            if (input == 'y' || input == 'Y') {
                // 재시작 로직
                score = 0; // 점수 초기화
                gameOverFlag = false; // 게임 오버 플래그 해제
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        field[y][x] = 0; // 필드 초기화
                    }
                }
                congratulated = false; // 축하 메시지 표시 여부 초기화
                gameLoop(); // 게임 루프 재시작
            }
            else if (input == 'n' || input == 'N') {
                return; // 게임 종료
            }
        }
    }
}

int main() {
    hideCursor(); // 커서 숨기기

    cout << "★★★★★★★★★★★★★★★★★★★★★★★★★" << endl;
    cout << "★★★★★★★★★★★★★★★★★★★★★★★★★" << endl;
    cout << "★★★★ INNY's  TETRIS ★★★★★" << endl;
    cout << "★★★★★★★★★★★★★★★★★★★★★★★★★" << endl;
    cout << "★★★★★★★★★★★★★★★★★★★★★★★★★" << endl;
    cout << "Press any button to Start" << endl;
    cout << "★★★★★★★★★★★★★★★★★★★★★★★★★" << endl;
    cout << "★★★★★★★★★★★★★★★★★★★★★★★★★" << endl;
    cout << "★★★★ October 6 2024 ★★★★★" << endl;
    cout << "★★★★★★★★★★★★★★★★★★★★★★★★★" << endl;
    cout << "★★★★★★★★★★★★★★★★★★★★★★★★★" << endl;

    _getch();

    system("cls");

    gameLoop(); // 게임 루프 시작

    return 0;
}