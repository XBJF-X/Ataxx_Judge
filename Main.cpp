#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <cstdio>
#include <vector>
using namespace std;

bool fileExists(const string& filename) {
    ifstream file(filename);
    return file.good();
}

void syncFileToVector(const string& filename, vector<string>& vec) {
    vec.clear();
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        vec.push_back(line);
    }
    file.close();
}

void syncVectorToFile(const string& filename, const vector<string>& vec) {
    ofstream file(filename);
    for (const auto& line : vec) {
        file << line << endl;
    }
    file.close();
}
void copy_and_overwrite_file(const string& source_path, const string& destination_path) {
    ifstream source_file(source_path, ios::binary);
    ofstream destination_file(destination_path, ios::binary);
    if (!source_file.is_open() || !destination_file.is_open()) {
        cerr << "无法打开文件。" << endl;
        return;
    }

    // 使用流缓冲区来复制文件
    source_file.seekg(0, ios::end);
    streamsize size = source_file.tellg();
    source_file.seekg(0, ios::beg);

    char* buffer = new char[size];
    source_file.read(buffer, size);
    destination_file.write(buffer, size);
    delete[] buffer;
    source_file.close();
    destination_file.close();
}
static int delta[24][2] = { { 1,1 },{ 0,1 },{ -1,1 },{ -1,0 },
{ -1,-1 },{ 0,-1 },{ 1,-1 },{ 1,0 },
{ 2,0 },{ 2,1 },{ 2,2 },{ 1,2 },
{ 0,2 },{ -1,2 },{ -2,2 },{ -2,1 },
{ -2,0 },{ -2,-1 },{ -2,-2 },{ -1,-2 },
{ 0,-2 },{ 1,-2 },{ 2,-2 },{ 2,-1 } };
int currBotColor; // 我所执子颜色（1为黑，-1为白，棋盘状态亦同）
int gridInfo[7][7] = { 0 }; // 先x后y，记录棋盘状态
int blackPieceCount , whitePieceCount ;
// 判断是否在地图内
inline bool inMap(int x, int y)
{
    if (x < 0 || x > 6 || y < 0 || y > 6)
        return false;
    return true;
}
// 向Direction方向改动坐标，并返回是否越界
inline bool MoveStep(int& x, int& y, int Direction)
{
    x = x + delta[Direction][0];
    y = y + delta[Direction][1];
    return inMap(x, y);
}
// 在坐标处落子，检查是否合法或模拟落子
bool ProcStep(int x0, int y0, int x1, int y1, int color)
{
    //printf("当前颜色：%d\n", color);
    //printf("当前移动：(%d,%d)→（%d,%d)\n", x0, y0, x1, y1);
    if (color == 0)
    {
        //printf("color等于0\n");
        return false;
    }
    if (x1 == -1) // 无路可走，跳过此回合
    {
        //printf("无路可走\n");
        return true;
    }
    if (!inMap(x0, y0) || !inMap(x1, y1)) // 超出边界
    {
        //printf("越界\n");
        return false;
    }
    if (gridInfo[x0][y0] != color) {
        //printf("当前起点颜色为：%d\n", gridInfo[x0][y0]);
        return false;
    }

    int dx, dy, x, y, currCount = 0, dir;
    int effectivePoints[8][2];
    dx = abs((x0 - x1)), dy = abs((y0 - y1));
    if ((dx == 0 && dy == 0) || dx > 2 || dy > 2) // 保证不会移动到原来位置，而且移动始终在5×5区域内
    {
        //printf("移动超出范围或原点\n");
        return false;
    }

    if (gridInfo[x1][y1] != 0) // 保证移动到的位置为空
    {
        //printf("终点不为空\n");
        return false;
    }

    if (dx == 2 || dy == 2) // 如果走的是5×5的外围，则不是复制粘贴
        gridInfo[x0][y0] = 0;
    else {
        if (color == 1)
            blackPieceCount++;
        else
            whitePieceCount++;
    }
    //printf("终点改色\n");
    gridInfo[x1][y1] = color;
    for (dir = 0; dir < 8; dir++) // 影响邻近8个位置
    {
        x = x1 + delta[dir][0];
        y = y1 + delta[dir][1];
        if (!inMap(x, y))
            continue;
        if (gridInfo[x][y] == -color) {
            effectivePoints[currCount][0] = x;
            effectivePoints[currCount][1] = y;
            currCount++;
            gridInfo[x][y] = color;
        }
    }
    //printf("终点附近染色\n");
    if (currCount != 0) {
        if (color == 1) {
            blackPieceCount += currCount;
            whitePieceCount -= currCount;
        }
        else {
            whitePieceCount += currCount;
            blackPieceCount -= currCount;
        }
    }
    return true;
}
void print_board() {
#ifndef _BOTZONE_ONLINE
    freopen("in.txt", "r", stdin);
#endif // !_BOTZONE_ONLINE
    int x0, y0, x1, y1;
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 7; j++) {
            gridInfo[i][j] = 0;
        }
    }
    // 初始化棋盘
    blackPieceCount = 2; whitePieceCount = 2;
    gridInfo[0][0] = gridInfo[6][6] = 1;  //黑
    gridInfo[6][0] = gridInfo[0][6] = -1; //白
                                          // 分析自己收到的输入和自己过往的输出，并恢复状态
    int turnID;
    currBotColor = -1; // 假定我方是白方
    cin >> turnID;
    for (int i = 0; i < turnID - 1; i++) {
        // 根据这些输入输出逐渐恢复状态到当前回合
        cin >> x0 >> y0 >> x1 >> y1;
        if (x1 >= 0)
        {
            //printf("模拟对方落子\n");
            ProcStep(x0, y0, x1, y1, -currBotColor); // 模拟对方落子
        }
        else
            currBotColor = 1; // 第一回合收到坐标是-1, -1，-1, -1说明我是黑方
        cin >> x0 >> y0 >> x1 >> y1;
        if (x1 >= 0)
        {
            //printf("模拟己方落子\n");
            ProcStep(x0, y0, x1, y1, currBotColor); // 模拟己方落子
        }

    }
    // 看看自己本回合输入，即对方上一轮的决策结果
    cin >> x0 >> y0 >> x1 >> y1;
    if (x1 >= 0)
        ProcStep(x0, y0, x1, y1, -currBotColor); // 模拟对方落子
    else
        currBotColor = 1;

    //此时gridInfo[][]里存储的就是当前棋盘的所有棋子信息
    printf("    0   1   2   3   4   5   6");
    printf("\n   ——— ——— ——— ——— ——— ——— ———\n");
    for (int y0 = 0; y0 < 7; y0++) {
        printf("%d |", y0);
        for (int x0 = 0; x0 < 7; x0++) {
            if (gridInfo[x0][y0] == 1) {
                printf(" ● |");
            }
            if (gridInfo[x0][y0] == -1) {
                printf(" ○ |");
            }
            if (gridInfo[x0][y0] == 0) {
                printf("   |");
            }
        }
        printf("\n   ——— ——— ——— ——— ——— ——— ———\n");
    }
}
void generateRandomWeightMatrix(ofstream& outFile) {
    // 设置随机种子
    srand(static_cast<unsigned int>(time(0)));
    //int temp[7];
    //// 生成前7行，每行7个元素，取值范围1-10
    //for (int i = 0; i < 7; i++) {
    //    temp[i]= rand() % 5 + 1; // 生成1到5的随机整数
    //}
    //outFile << temp[0] << " " << temp[1] << " " << temp[1] << " " << temp[1] << " " << temp[1] << " " << temp[1] << " " << temp[1]<<endl;
    //outFile << temp[1] << " " << temp[2] << " " << temp[3] << " " << temp[3] << " " << temp[3] << " " << temp[3] << " " << temp[1] << endl;
    //outFile << temp[1] << " " << temp[3] << " " << temp[4] << " " << temp[5] << " " << temp[5] << " " << temp[3] << " " << temp[1] << endl;
    //outFile << temp[1] << " " << temp[3] << " " << temp[5] << " " << temp[6] << " " << temp[5] << " " << temp[3] << " " << temp[1] << endl;
    //outFile << temp[1] << " " << temp[3] << " " << temp[5] << " " << temp[5] << " " << temp[4] << " " << temp[3] << " " << temp[1] << endl;
    //outFile << temp[1] << " " << temp[3] << " " << temp[3] << " " << temp[3] << " " << temp[3] << " " << temp[2] << " " << temp[1] << endl;
    //outFile << temp[1] << " " << temp[1] << " " << temp[1] << " " << temp[1] << " " << temp[1] << " " << temp[1] << " " << temp[0] << endl;
    //outFile << "0 1 0" << endl;

    for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 7; ++col) {
            int value = rand() % 5 + 1; // 生成1到10的随机整数
            outFile << value << " ";
        }
        outFile << endl;
    }

    //// 生成第八行，三个权重系数，取值范围1-5
    //for (int w = 0; w < 3; ++w) {
    //    int weight = rand() % 5 + 1; // 生成1到5的随机整数
    //    outFile << 1 << " ";
    //}
    //outFile << endl;
    outFile << "0 1 0" << endl;
}
int main() {
    //ofstream outFile_1("weight_1.txt");
    //generateRandomWeightMatrix(outFile_1);
    //outFile_1.close();
    
    while (true) {
        ofstream outFile_2("weight_2.txt");
        generateRandomWeightMatrix(outFile_2);
        outFile_2.close();
        copy_and_overwrite_file("src/start.txt", "in.txt");
        copy_and_overwrite_file("src/blank.txt", "BLACK.txt");
        copy_and_overwrite_file("src/blank.txt", "WHITE.txt");
        int p1, p2;
        //printf("请输入P1 VS P2：\n");
        //cin >> p1 >> p2;
        p1 = 14, p2 = 15;
        string compile_cmd_1 = "g++ " + to_string(p1) + ".cpp -o " + to_string(p1);
        string compile_cmd_2 = "g++ " + to_string(p2) + ".cpp -o " + to_string(p2);
        // 编译程序
        if (!fileExists(to_string(p1) + ".exe")) {
            int compile_status_1 = system(compile_cmd_1.c_str());
            if (compile_status_1 != 0) {
                cerr << "P1编译失败 (code:" << compile_status_1 << ")" << endl;
                return 1;
            }
        }
        else {
            printf("P1.exe已经存在，跳过编译\n");
        }
        if (!fileExists(to_string(p2) + ".exe")) {
            int compile_status_2 = system(compile_cmd_2.c_str());
            if (compile_status_2 != 0) {
                cerr << "P2编译失败 (code:" << compile_status_2 << ")" << endl;
                return 1;
            }
        }
        else {
            printf("P2.exe已经存在，跳过编译\n");
        }
        string app1 = to_string(p1) + ".exe";
        string app2 = to_string(p2) + ".exe";
        vector<string> fileContent;
        const string magicLine = "-1 -1 -1 -1";
        // 初始加载文件内容
        syncFileToVector("in.txt", fileContent);
        int round = 1; // 添加一个回合计数器
        while (true) {
            int flag;
            //========= P1 执行阶段 =========//
            // 更新输入文件
            syncVectorToFile("in.txt", fileContent);
            //print_board();
            // 执行P1并捕获输出
            printf("第%d回合 - 黑方（%d）开始思考\n", round, p1);
            system((app1 + " >> in.txt").c_str());
            syncFileToVector("in.txt", fileContent);
            // 删除第二行（索引1）
            if (fileContent.size() > 1) {
                fileContent.erase(fileContent.begin() + 1);
            }
            if (fileContent.back() == "-1 -1 -1 -1") {
                print_board();
                cout << "黑方（" << p1 << "）无路可走" << endl;
                if (blackPieceCount > whitePieceCount) { 
                    printf("黑（%d）：白（%d）,黑方（%d）获胜\n", blackPieceCount, whitePieceCount, p1); 
                }
                if (blackPieceCount < whitePieceCount) {
                    printf("黑（%d）：白（%d）,白方（%d）获胜\n", blackPieceCount, whitePieceCount, p2);
                    copy_and_overwrite_file("weight_2.txt", "weight_1.txt");
                }
                break;
            }
            string temp = fileContent.back();
            printf("黑方落子：（%c,%c） -> （%c,%c）\n", temp[0], temp[2], temp[4], temp[6]);

            //========= P2 执行阶段 =========//
            // 更新修改后的内容
            syncVectorToFile("in.txt", fileContent);
            //print_board();
            // 执行P2并捕获输出
            printf("第%d回合 - 白方（%d）开始思考\n", round, p2);
            system((app2 + " >> in.txt").c_str());
            syncFileToVector("in.txt", fileContent);
            round++; // 增加回合计数器
            // 更新回合数
            fileContent[0] = to_string(round);
            // 插入魔法行到第二行
            if (fileContent.size() >= 1) {
                fileContent.insert(fileContent.begin() + 1, magicLine);
            }
            else {
                fileContent.push_back(magicLine);
            }
            if (fileContent.back() == "-1 -1 -1 -1") {
                print_board();
                cout << "白方（" << p2 << "）无路可走" << endl;
                if (blackPieceCount > whitePieceCount) { printf("黑（%d）：白（%d）,黑方（%d）获胜\n", blackPieceCount, whitePieceCount, p1); }
                if (blackPieceCount < whitePieceCount) { 
                    printf("黑（%d）：白（%d）,白方（%d）获胜\n", blackPieceCount, whitePieceCount, p2); 
                    copy_and_overwrite_file("weight_2.txt", "weight_1.txt");
                }
                break;
            }
            temp = fileContent.back();
            printf("白方落子：（%c,%c） -> （%c,%c）\n", temp[0], temp[2], temp[4], temp[6]);
            if (round > 200) {
                break;
            }
        }
        if (remove(app1.c_str()) == 0) {
            cout << "文件 " << app1.c_str() << " 已成功删除。" << endl;
        }
        else {
            cerr << "无法删除文件 " << app1.c_str() << endl;
            // 输出错误码以供调试
            cerr << "错误码: " << errno << endl;
        }
        if (remove(app2.c_str()) == 0) {
            cout << "文件 " << app2.c_str() << " 已成功删除。" << endl;
        }
        else {
            cerr << "无法删除文件 " << app2.c_str() << endl;
            // 输出错误码以供调试
            cerr << "错误码: " << errno << endl;
        }
    }
    return 0;
}