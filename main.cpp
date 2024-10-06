#include <iostream>
#include <conio.h> // _kbhit(), _getch() ���
#include <windows.h> // Sleep(), SetConsoleCursorPosition ���
#include <ctime> // rand() ���
#include <mmsystem.h> // PlaySound() ���
#pragma comment(lib, "winmm.lib") // PlaySound() ����� ���� ���̺귯�� ��ũ
using namespace std;

const int width = 15;
const int height = 15;
int field[height][width] = { 0 };
int score = 0; // ���� �ʱ�ȭ

// ��Ʈ���� ��� ��� ���� (4x4 ũ��)
int tetromino[7][4][4] = {
    {{1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // I
    {{1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // O
    {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // S
    {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // Z
    {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // J
    {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // L
    {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}  // T
};

// ���� ��� ����
int currentTetromino[4][4];
int currentX = width / 2 - 2;
int currentY = 0;
bool isBlockFixed = false; // ���� ����� �����Ǿ����� ����
bool gameOverFlag = false; // ���� ���� ���� �÷���
bool congratulated = false; // ���� �޽��� ǥ�� ����
bool waitingForNextLevel = false; // ���� ���� ��� ���� �÷���

// �ܼ� â�� Ŀ���� ����� �Լ�
void hideCursor() {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(console, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(console, &cursorInfo);
}

// Ŀ���� �ܼ��� Ư�� ��ġ�� �̵��ϴ� �Լ�
void setCursorPosition(int x, int y) {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD position = { (short)x, (short)y };
    SetConsoleCursorPosition(console, position);
}

// ��� ���� �Լ�
void copyTetromino(int index) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            currentTetromino[y][x] = tetromino[index][y][x];
        }
    }
}

// �ʰ� �׵θ� ���, ���� ǥ�� �Լ�
void draw() {
    setCursorPosition(0, 0); // Ŀ���� �ܼ� â�� ù ��° ��ġ�� �̵�

    // ��� ���� ��� (�߾� ����)
    string title = "INNY's TETRIS";
    int titlePosition = (width + 2 - title.length()) / 2; // �߾� ����
    cout << string(titlePosition, ' ') << title << endl;

    // ��� �׵θ� ���
    for (int x = 0; x < width + 2; x++) {
        cout << "��";
    }
    cout << "   Score: " << score << endl; // ���� ���

    // ���� �ʵ�� �翷 �׵θ� ���
    for (int y = 0; y < height; y++) {
        cout << "��"; // ���� �׵θ�
        for (int x = 0; x < width; x++) {
            if (field[y][x] == 1) {
                cout << "��";
            }
            else {
                bool isTetrominoPart = false;
                for (int ty = 0; ty < 4; ty++) {
                    for (int tx = 0; tx < 4; tx++) {
                        if (currentTetromino[ty][tx] == 1 && currentY + ty == y && currentX + tx == x) {
                            cout << "��";
                            isTetrominoPart = true;
                        }
                    }
                }
                if (!isTetrominoPart) {
                    cout << " ";
                }
            }
        }
        cout << "��" << endl; // ������ �׵θ�
    }

    // �ϴ� �׵θ� ���
    for (int x = 0; x < width + 2; x++) {
        cout << "��";
    }
    cout << endl;

    // ���� ���� ���� ǥ��
    if (gameOverFlag) {
        string failMessage = "FAIL";
        string retryMessage = "RETRY (Y/N)";
        int centerXFail = (width + 2 - failMessage.length()) / 2;
        int centerXRetry = (width + 2 - retryMessage.length()) / 2;
        setCursorPosition(centerXFail, height / 2);
        cout << failMessage << endl;
        setCursorPosition(centerXRetry, height / 2 + 1);
        cout << retryMessage << endl; // Retry �޽��� ǥ��
    }

    // ���� �޽��� ǥ��
    if (congratulated) {
        string congratsMessage = "CONGRATULATION!";
        string nextLevelMessage = "NEXT STAGE?(Y/N)";
        int centerXCongrats = (width + 2 - congratsMessage.length()) / 2;
        int centerXNextLevel = (width + 2 - nextLevelMessage.length()) / 2;
        setCursorPosition(centerXCongrats, height / 2);
        cout << congratsMessage << endl;
        setCursorPosition(centerXNextLevel, height / 2 + 1);
        cout << nextLevelMessage << endl; // Next Level �޽��� ǥ��

        // Y/N �Է� ó��
        char input = _getch();
        if (input == 'y' || input == 'Y') {
            
            // ���� ���� ����
            setCursorPosition(centerXCongrats, height / 2);
            cout << string(congratsMessage.length(), ' '); // ���� �޽��� �����
            setCursorPosition(centerXNextLevel, height / 2 + 1);
            cout << string(nextLevelMessage.length(), ' '); // ���� �ܰ� �޽��� �����

            // ��ȥ ���� �޽��� ���
            waitingForNextLevel = true;
            string marriageMessage1 = "    WILL YOU";
            string marriageMessage2 = " MARRY ME? (Y/N)";
            int centerXMarriage1 = (width + 2 - marriageMessage1.length()) / 2;
            int centerXMarriage2 = (width + 2 - marriageMessage2.length()) / 2;
            setCursorPosition(centerXMarriage1, height/2); // �߾ӿ� ���
            setCursorPosition(centerXMarriage2, height/2); // �߾ӿ� ���
            cout << marriageMessage1 << endl;
            cout << marriageMessage2 << endl;

            // �ٽ� Y/N �Է� ó�� �� ��Ʈ ó��
            char marriageInput = _getch();
            if (marriageInput == 'y' || marriageInput == 'Y') {
                
                // BGM ����
                PlaySound(NULL, 0, 0);

                // Wedding ���� ���
                PlaySound(TEXT("Wedding.wav"), NULL, SND_FILENAME | SND_ASYNC);

                // ���� ���� �����
                for (int i = 0; i < height; ++i) {
                    setCursorPosition(0, i);
                    for (int j = 0; j < width + 2; ++j) cout << " "; // �� �������� ����
                }
                
                // ��Ʈ�� �� �߾ӿ� �׸���
                setCursorPosition(0, 0);
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                cout << "�������������������������������" << endl;
                
                _getch();

                // ��Ʈ�� �׷��� �� �� �̻� �ٸ� �޼����� ������� ����
                return;  // ���� ����
            }
            else if (marriageInput == 'n' || marriageInput == 'N') {
                return;  // ���� ����
            }
        }
        else if (input == 'n' || input == 'N') {
            return;  // ���� ����
        }
    }
}

// �浹 ���� �Լ�
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

// ����� �ʵ忡 �����ϴ� �Լ�
void placeTetromino() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentTetromino[y][x] == 1) {
                field[currentY + y][currentX + x] = 1;
            }
        }
    }
}

// ���� ���� �Լ�
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
            // ���� �߰�
            score += 10;

            for (int y2 = y; y2 > 0; y2--) {
                for (int x = 0; x < width; x++) {
                    field[y2][x] = field[y2 - 1][x];
                }
            }
            y++; // �� �� ���� �� ��Ȯ��
        }
    }
}

// ��� ȸ�� �Լ�
void rotate() {
    int temp[4][4] = { 0 };
    // ȸ��
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            temp[y][x] = currentTetromino[3 - x][y]; // 90�� ȸ��
        }
    }

    // ȸ�� ���� ���� Ȯ��
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (temp[y][x] == 1) {
                int newX = currentX + x;
                int newY = currentY + y;
                if (newX < 0 || newX >= width || newY >= height || field[newY][newX] == 1) {
                    return; // ȸ�� �Ұ���
                }
            }
        }
    }

    // ȸ�� ������ ���, ���� ��� ������Ʈ
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            currentTetromino[y][x] = temp[y][x];
        }
    }
}

// ���� ����
void gameLoop() {
    // WAV ���� ��� (���� ���� ���� �� BGM ���)
    PlaySound(TEXT("BGM.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    
    srand(static_cast<unsigned int>(time(0))); // ���� �ʱ�ȭ
    copyTetromino(rand() % 7); // ù ��� �������� ����

    while (!gameOverFlag) {
        // �Է� ó��
        while (_kbhit()) {
            char input = _getch();
            if (input == 75) { // ���� ȭ��ǥ
                if (!checkCollision(-1, 0)) {
                    currentX--;
                }
            }
            else if (input == 77) { // ������ ȭ��ǥ
                if (!checkCollision(1, 0)) {
                    currentX++;
                }
            }
            else if (input == 80) { // �Ʒ� ȭ��ǥ
                if (!checkCollision(0, 1)) {
                    currentY++;
                }
            }
            else if (input == 72) { // �� ȭ��ǥ (ȸ��)
                rotate();
                if (checkCollision(0, 0)) {
                    // ���� ���·� �ǵ�����
                    rotate();
                    rotate();
                    rotate();
                }
            }
            else if (input == 32) { // �����̽��� (���� �ϰ�)
                while (!checkCollision(0, 1)) {
                    currentY++;
                }
            }

            // ���� ���� ���¿��� Y/N �Է� ó��
            if (gameOverFlag) {      
                if (input == 'y' || input == 'Y') {
                    // ����� ����
                    score = 0; // ���� �ʱ�ȭ
                    gameOverFlag = false; // ���� ���� �÷��� ����
                    for (int y = 0; y < height; y++) {
                        for (int x = 0; x < width; x++) {
                            field[y][x] = 0; // �ʵ� �ʱ�ȭ
                        }
                    }
                    congratulated = false; // ���� �޽��� ǥ�� ���� �ʱ�ȭ
                    gameLoop(); // ���� ���� �����
                }
                else if (input == 'n' || input == 'N') {
                    return; // ���� ����
                }
            }

            // ���� ���� ��� ���¿��� Y/N �Է� ó��
            if (waitingForNextLevel) {
                if (input == 'y' || input == 'Y') {
                    // ��ȥ ���� ����
                    for (int y = 0; y < height; y++) {
                        for (int x = 0; x < width; x++) {
                            field[y][x] = 1; // �ʵ� ä���
                        }
                    }
                    return; // ���� ����
                }
                else if (input == 'n' || input == 'N') {
                    return; // ���� ����
                }
            }
        }

        // �߷� ����
        if (!checkCollision(0, 1)) {
            currentY++;
        }
        else {
            isBlockFixed = true;
        }

        if (isBlockFixed) {
            placeTetromino();
            removeLine(); // ���� ����

            // ����� �ʵ� �ֻ�ܿ� ��Ҵ��� Ȯ���Ͽ� ���� ���� ���� ����
            if (currentY <= 0) {
                gameOverFlag = true; // ���� ����

                // BGM ����
                PlaySound(NULL, 0, 0);

                // Gameover ���� �� �� ���
                PlaySound(TEXT("Gameover.wav"), NULL, SND_FILENAME | SND_SYNC);
            }

            // ���� 50�� �̻� �� ���� �޽��� ǥ�� �� ���� ����
            if (score >= 50 && !congratulated) {
                
                // BGM ����
                PlaySound(NULL, 0, 0);

                // Success ���� ���
                PlaySound(TEXT("Success.wav"), NULL, SND_FILENAME | SND_ASYNC);
                
                congratulated = true; // ���� �޽��� ǥ��
                waitingForNextLevel = true; // ���� ���� ��� ���·� ����
                draw(); // ȭ�� ������Ʈ
                Sleep(200); // ��� ���
                return; // ���� ����
            }

            currentX = width / 2 - 2; // ���ο� ��� ��ġ �ʱ�ȭ
            currentY = 0;
            copyTetromino(rand() % 7); // ���ο� ��� �������� ����
            isBlockFixed = false; // ��� ���� ����
        }

        // ���� ���� ���
        draw();
        Sleep(200); // ȭ�� ������Ʈ �ӵ� ����
    }

    // ���� ���� �� �Է� ���
    while (gameOverFlag) {
        if (_kbhit()) {
            char input = _getch();
            if (input == 'y' || input == 'Y') {
                // ����� ����
                score = 0; // ���� �ʱ�ȭ
                gameOverFlag = false; // ���� ���� �÷��� ����
                for (int y = 0; y < height; y++) {
                    for (int x = 0; x < width; x++) {
                        field[y][x] = 0; // �ʵ� �ʱ�ȭ
                    }
                }
                congratulated = false; // ���� �޽��� ǥ�� ���� �ʱ�ȭ
                gameLoop(); // ���� ���� �����
            }
            else if (input == 'n' || input == 'N') {
                return; // ���� ����
            }
        }
    }
}

int main() {
    hideCursor(); // Ŀ�� �����

    cout << "�ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ�" << endl;
    cout << "�ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ�" << endl;
    cout << "�ڡڡڡ� INNY's  TETRIS �ڡڡڡڡ�" << endl;
    cout << "�ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ�" << endl;
    cout << "�ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ�" << endl;
    cout << "Press any button to Start" << endl;
    cout << "�ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ�" << endl;
    cout << "�ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ�" << endl;
    cout << "�ڡڡڡ� October 6 2024 �ڡڡڡڡ�" << endl;
    cout << "�ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ�" << endl;
    cout << "�ڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡڡ�" << endl;

    _getch();

    system("cls");

    gameLoop(); // ���� ���� ����

    return 0;
}