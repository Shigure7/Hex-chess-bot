#include <iostream>
#include <algorithm>
#include <string>
#include <ctime>
#include <cstdlib>
using namespace std;
struct Point {
	int x;
	int y;
	int value;
};
struct GameTreeNode {
	GameTreeNode* child[12][12] = { 0 };
	GameTreeNode* parent;
	int x = 0;
	int y = 0;
	int d = 0; //保存搜索高度
	int pb = 0; //当前棋盘红蓝方的潜力值
	int pr = 0;
	int value = 0;
	bool ifend = false;
	Point childpoint[150] = { 0 };
	int cnt = 0;
	int movebadness[12][12] = { 0 }; //当前棋盘的移动误差
	int minbadness = 1000000;
	GameTreeNode(int sx = 0, int sy = 0, GameTreeNode* p = NULL) {
		x = sx; y = sy; parent = p;
	}
};
const bool RED = true, BLUE = false;
const int SIZE = 12, MAX = 1000, MAXI = 100000000;
clock_t start;
int board[SIZE][SIZE] = { 0 };//本方1，对方-1，空白0
int dx[6] = { 0,1,1,0,-1,-1 }, dy[6] = { -1,-1,0,1,1,0 }; //一个坐标的6个相邻坐标
int dx1[3] = { -1,0,1 }, dy1[3] = { 0,-1,-1 }, dx2[3] = { 1,0,-1 }, dy2[3] = { 0,1,1 }; //蓝方求潜力值
int dx3[3] = { 0,-1,-1 }, dy3[3] = { -1,0,1 }, dx4[3] = { 1,1,0 }, dy4[3] = { -1,0,1 }; //红方求潜力值
int cnt1 = 0, cnt2 = 0;
//以下为Hex策略
bool IsEdgePattens(int px, int py, int flag) {
	if (flag == 1 && board[px][py - 1] == 0 && board[px + 1][py - 1] == 0 && board[px][py - 2] == 0 && board[px + 1][py - 2] == 0 && board[px + 2][py - 2] == 0) {
		return true;
	}
	if (flag == 2 && board[px - 1][py + 1] == 0 && board[px][py + 1] == 0 && board[px - 2][py + 2] == 0 && board[px - 1][py + 2] == 0 && board[px][py + 2] == 0) {
		return true;
	}
	if (flag == 3 && board[px - 1][py] == 0 && board[px - 1][py + 1] == 0 && board[px - 2][py] == 0 && board[px - 2][py + 1] == 0 && board[px - 2][py + 2] == 0) {
		return true;
	}
	if (flag == 4 && board[px + 1][py - 1] == 0 && board[px + 1][py] == 0 && board[px + 2][py - 2] == 0 && board[px + 2][py - 1] == 0 && board[px + 2][py] == 0) {
		return true;
	}
	return false;
}
bool IfNotMyOutposts(int mx, int my, int px, int py, int flag) { //判断是否没有我的前哨结点
	int mycolor = board[mx][my];
	if (flag == 0) { //向右下边扩展搜索
		for (int i = 1; (my + i * dy[3] <= 11) || (py + i * dy[3] <= 11); i++) {
			if (my + i * dy[3] <= 11 && board[mx][my + i * dy[3]] == mycolor) return false;
			if (py + i * dy[3] <= 11 && board[px][py + i * dy[3]] == mycolor) return false;
		}
	}
	else if (flag == 1) { //往右上扩展搜索
		for (int i = 1; (mx + i * dx[4] >= 1 && my + i * dy[4] <= 11) || (px + i * dx[4] >= 1 && py + i * dy[4] <= 11); i++) {
			if (mx + i * dx[4] >= 1 && my + i * dy[4] <= 11 && board[mx + i * dx[4]][my + i * dy[4]] == mycolor) return false;
			if (px + i * dx[4] >= 1 && py + i * dy[4] <= 11 && board[px + i * dx[4]][py + i * dy[4]] == mycolor) return false;
		}
	}
	else if (flag == 2) { //向右上边扩展搜索
		for (int i = 1; (mx + i * dx[5] >= 1) || (px + i * dx[5] >= 1); i++) {
			if (mx + i * dx[5] >= 1 && board[mx + i * dx[5]][my] == mycolor) return false;
			if (px + i * dx[5] >= 1 && board[px + i * dx[5]][py] == mycolor) return false;
		}
	}
	else if (flag == 3) { //向左上边扩展搜索
		for (int i = 1; (my + i * dy[0] >= 1) || (py + i * dy[0] >= 1); i++) {
			if (my + i * dy[0] >= 1 && board[mx][my + i * dy[0]] == mycolor) return false;
			if (py + i * dy[0] >= 1 && board[px][py + i * dy[0]] == mycolor) return false;
		}
	}
	else if (flag == 4) { //往左下扩展搜索
		for (int i = 1; (mx + i * dx[1] <= 11 && my + i * dy[1] >= 1) || (px + i * dx[1] <= 11 && py + i * dy[1] >= 1); i++) {
			if (mx + i * dx[1] <= 11 && my + i * dy[1] >= 1 && board[mx + i * dx[1]][my + i * dy[1]] == mycolor) return false;
			if (px + i * dx[1] <= 11 && py + i * dy[1] >= 1 && board[px + i * dx[1]][py + i * dy[1]] == mycolor) return false;
		}
	}
	else if (flag == 5) { //向左下边扩展搜索
		for (int i = 1; (mx + i * dx[2] <= 11) || (px + i * dx[2] <= 11); i++) {
			if (mx + i * dx[2] <= 11 && board[mx + i * dx[2]][my] == mycolor) return false;
			if (px + i * dx[2] <= 11 && board[px + i * dx[2]][py] == mycolor) return false;
		}
	}
	return true;
}
int IsLadder(int mx, int my, int px, int py) { //将我方和对方近1步下法保存在mx，my，px，py中,然后判断是否在建梯子
	bool pcolor = (board[px][py] == board[2][3]) ? RED : BLUE;
	if (pcolor == RED) {
		for (int i = 0; i < 6; i++) {
			if (px + dx[i] <= 11 && py + dy[i] <= 11 && mx + dx[i] <= 11 && my + dy[i] <= 11) {
				if (board[px][py] == board[px + dx[i]][py + dy[i]] && board[mx][my] == board[mx + dx[i]][my + dy[i]]) {
					if (i == 2) {
						if ((mx + dx[0] == px && my + dy[0] == py) || (mx + dx[4] == px && my + dy[4] == py)) {
							if (mx - 2 >= 1 && IfNotMyOutposts(mx, my, px, py, i)) { return 5; }
						}
					}
					else if (i == 5) {
						if ((mx + dx[1] == px && my + dy[1] == py) || (mx + dx[3] == px && my + dy[3] == py)) {
							if (mx + 2 >= 1 && IfNotMyOutposts(mx, my, px, py, i)) { return 2; }
						}
					}
					else if (i == 1) {
						if (mx + my < 12 && mx + dx[3] == px && my + dy[3] == py) {
							if (mx - 2 >= 1 && mx - 2 <= 11 && my + 2 >= 1 && my + 2 <= 11)
								if (IfNotMyOutposts(mx, my, px, py, i)) { return 4; }
						}
					}
					else if (i == 4) {
						if (mx + my > 12 && mx + dx[0] == px && my + dy[0] == py) {
							if (mx + 2 >= 1 && mx + 2 <= 11 && my - 2 >= 1 && my - 2 <= 11)
								if (IfNotMyOutposts(mx, my, px, py, i)) { return 1; }
						}
					}
				}
			}
		}
	}
	if (pcolor == BLUE) {
		for (int i = 0; i < 6; i++) {
			if (px + dx[i] <= 11 && py + dy[i] <= 11 && mx + dx[i] <= 11 && my + dy[i] <= 11) {
				if (board[px][py] == board[px + dx[i]][py + dy[i]] && board[mx][my] == board[mx + dx[i]][my + dy[i]]) {
					if (i == 0) {
						if ((mx + dx[2] == px && my + dy[2] == py) || (mx + dx[4] == px && my + dy[4] == py)) {
							if (my + 2 <= 11 && IfNotMyOutposts(mx, my, px, py, i)) { return 3; }
						}
					}
					else if (i == 3) {
						if ((mx + dx[1] == px && my + dy[1] == py) || (mx + dx[5] == px && my + dy[5] == py)) {
							if (my - 2 >= 1 && IfNotMyOutposts(mx, my, px, py, i)) { return 0; }
						}
					}
					else if (i == 1) {
						if (mx + my > 12 && mx + dx[5] == px && my + dy[5] == py) {
							if (mx - 2 >= 1 && mx - 2 <= 11 && my + 2 >= 1 && my + 2 <= 11)
								if (IfNotMyOutposts(mx, my, px, py, i)) { return 4; }
						}
					}
					else if (i == 4) {
						if (mx + my < 12 && mx + dx[2] == px && my + dy[2] == py) {
							if (mx + 2 >= 1 && mx + 2 <= 11 && my - 2 >= 1 && my - 2 <= 11)
								if (IfNotMyOutposts(mx, my, px, py, i)) { return 1; }
						}
					}
				}
			}
		}
	}
	return -1;
}
//以下为评估函数代码
void BFSevaluate(int(&visited)[SIZE][SIZE], int(&map)[SIZE][SIZE], int sx, int sy, int samecolor) {
	int Q[200][2] = { 0 }, front = 0, rear = 0;
	Q[rear][0] = sx;
	Q[rear][1] = sy;
	rear++;
	visited[sx][sy] += 1;
	while (front != rear) {
		int x = Q[front][0], y = Q[front][1];
		front++;
		for (int i = 0; i < 6; i++) {
			int nx = x + dx[i], ny = y + dy[i];
			if (nx >= 1 && nx <= 11 && ny >= 1 && ny <= 11 && board[nx][ny] == samecolor && visited[nx][ny] < visited[sx][sy]) {
				Q[rear][0] = nx;
				Q[rear][1] = ny;
				rear++;
				visited[nx][ny] = visited[sx][sy];
				map[nx][ny] = map[sx][sy];
			}
		}
	}
}
void evaluateblue1(int(&mapblue1)[SIZE][SIZE], int samecolor, int difcolor, int pcolor) { //求所有点到左上边的双距离
	int visited[SIZE][SIZE] = { 0 };
	if (pcolor == samecolor) {
		for (int i = 1; i <= 9; i++) {
			if (board[i][3] == pcolor && IsEdgePattens(i, 3, 1)) {
				visited[i][3] = 100;
			}
		}
	}
	for (int i = 1; i <= 11; i++)
		mapblue1[i][1] = (board[i][1] == samecolor) ? 0 : ((board[i][1] == difcolor) ? MAX : 1);
	for (int j = 2; j <= 11; j++)
		for (int i = 1; i <= 11; i++) {
			if (board[i][j] == difcolor) { mapblue1[i][j] = MAX; continue; }
			int min1 = 2 * MAX, min2 = 2 * MAX;
			for (int k = 0; k < 3; k++) {
				int nx = i + dx1[k], ny = j + dy1[k], mx = i + dx2[k], my = j + dy2[k];
				if (nx >= 1 && nx <= 11 && ny >= 1 && ny <= 11) {
					if (board[nx][ny] == samecolor) {
						if (mapblue1[nx][ny] <= min1 && mapblue1[nx][ny] < min2) {
							min1 = mapblue1[nx][ny]; min2 = min1;
						}
						else if (mapblue1[nx][ny] < min2) {
							min2 = mapblue1[nx][ny];
						}
					}
					else {
						if (mapblue1[nx][ny] <= min1 && mapblue1[nx][ny] < min2) {
							min2 = min1; min1 = mapblue1[nx][ny];
						}
						else if (mapblue1[nx][ny] < min2) {
							min2 = mapblue1[nx][ny];
						}
					}
				}
				if (mx >= 1 && mx <= 11 && my >= 1 && my <= 11 && board[mx][my] == samecolor && visited[mx][my] != 0) {
					if (mapblue1[mx][my] <= min1 && mapblue1[mx][my] < min2) {
						min1 = mapblue1[mx][my]; min2 = min1;
					}
					else if (mapblue1[mx][my] < min2) {
						min2 = mapblue1[mx][my];
					}
				}
			}
			int potential = min2 + 1;
			if (board[i][j] == samecolor && (visited[i][j] == 0 || mapblue1[i][j] > potential - 1)) {
				mapblue1[i][j] = potential - 1;
				BFSevaluate(visited, mapblue1, i, j, samecolor);
			}
			else if (board[i][j] != samecolor) mapblue1[i][j] = potential;
		}
	//修正第11列潜力值
	int min1 = MAX * MAX, min2 = MAX * MAX;
	for (int i = 1; i <= 11; i++) {
		if (board[i][11] == samecolor) {
			if (mapblue1[i][11] <= min1 && mapblue1[i][11] < min2) {
				min1 = mapblue1[i][11]; min2 = min1;
			}
			else if (mapblue1[i][11] < min2) {
				min2 = mapblue1[i][11];
			}
		}
		else {
			if (mapblue1[i][11] <= min1 && mapblue1[i][11] < min2) {
				min2 = min1; min1 = mapblue1[i][11];
			}
			else if (mapblue1[i][11] < min2) {
				min2 = mapblue1[i][11];
			}
		}
	}
	int min = min2;
	for (int i = 1; i <= 11; i++) {
		if (board[i][11] != difcolor) {
			if (board[i][11] == samecolor && mapblue1[i][11] > min) {
				mapblue1[i][11] = min;
				BFSevaluate(visited, mapblue1, i, 11, samecolor);
			}
			else if (board[i][11] == 0 && mapblue1[i][11] > min + 1) mapblue1[i][11] = min + 1;
		}
	}
}
void evaluateblue2(int(&mapblue2)[SIZE][SIZE], int samecolor, int difcolor, int pcolor) { //求所有点到右下边的双距离
	int visited[SIZE][SIZE] = { 0 };
	if (pcolor == samecolor) {
		for (int i = 11; i >= 3; i--) {
			if (board[i][9] == pcolor && IsEdgePattens(i, 9, 2)) {
				visited[i][9] = 100;
			}
		}
	}
	for (int i = 1; i <= 11; i++)
		mapblue2[i][11] = (board[i][11] == samecolor) ? 0 : ((board[i][11] == difcolor) ? MAX : 1);
	for (int j = 10; j >= 1; j--)
		for (int i = 11; i >= 1; i--) {
			if (board[i][j] == difcolor) { mapblue2[i][j] = MAX; continue; }
			int min1 = 2 * MAX, min2 = 2 * MAX;
			for (int k = 0; k < 3; k++) {
				int nx = i + dx2[k], ny = j + dy2[k], mx = i + dx1[k], my = j + dy1[k];
				if (nx >= 1 && nx <= 11 && ny >= 1 && ny <= 11) {
					if (board[nx][ny] == samecolor) {
						if (mapblue2[nx][ny] <= min1 && mapblue2[nx][ny] < min2) {
							min1 = mapblue2[nx][ny]; min2 = min1;
						}
						else if (mapblue2[nx][ny] < min2) {
							min2 = mapblue2[nx][ny];
						}
					}
					else {
						if (mapblue2[nx][ny] <= min1 && mapblue2[nx][ny] < min2) {
							min2 = min1; min1 = mapblue2[nx][ny];
						}
						else if (mapblue2[nx][ny] < min2) {
							min2 = mapblue2[nx][ny];
						}
					}
				}
				if (mx >= 1 && mx <= 11 && my >= 1 && my <= 11 && board[mx][my] == samecolor && visited[mx][my] != 0) {
					if (mapblue2[mx][my] <= min1 && mapblue2[mx][my] < min2) {
						min1 = mapblue2[mx][my]; min2 = min1;
					}
					else if (mapblue2[mx][my] < min2) {
						min2 = mapblue2[mx][my];
					}
				}
			}
			int potential = min2 + 1;
			if (board[i][j] == samecolor && (visited[i][j] == 0 || mapblue2[i][j] > potential - 1)) {
				mapblue2[i][j] = potential - 1;
				BFSevaluate(visited, mapblue2, i, j, samecolor);
			}
			else if (board[i][j] != samecolor) mapblue2[i][j] = potential;
		}
	//求第1列潜力值
	int min1 = 2 * MAX, min2 = 2 * MAX;
	for (int i = 1; i <= 11; i++) {
		if (board[i][1] == samecolor) {
			if (mapblue2[i][1] <= min1 && mapblue2[i][1] < min2) {
				min1 = mapblue2[i][1]; min2 = min1;
			}
			else if (mapblue2[i][1] < min2) {
				min2 = mapblue2[i][1];
			}
		}
		else {
			if (mapblue2[i][1] <= min1 && mapblue2[i][1] < min2) {
				min2 = min1; min1 = mapblue2[i][1];
			}
			else if (mapblue2[i][1] < min2) {
				min2 = mapblue2[i][1];
			}
		}
	}
	int min = min2;
	for (int i = 11; i >= 1; i--) {
		if (board[i][1] != difcolor) {
			if (board[i][1] == samecolor && mapblue2[i][1] > min) {
				mapblue2[i][1] = min;
				BFSevaluate(visited, mapblue2, i, 1, samecolor);
			}
			else if (board[i][1] == 0 && mapblue2[i][1] > min + 1) mapblue2[i][1] = min + 1;
		}
	}
}
void evaluatered1(int(&mapred1)[SIZE][SIZE], int samecolor, int difcolor, int pcolor) { //求所有点到右上边的双距离
	int visited[SIZE][SIZE] = { 0 };
	if (pcolor == samecolor) {
		for (int j = 1; j <= 9; j++) {
			if (board[3][j] == pcolor && IsEdgePattens(3, j, 3)) {
				visited[3][j] = 100;
			}
		}
	}
	for (int j = 1; j <= 11; j++)
		mapred1[1][j] = (board[1][j] == samecolor) ? 0 : ((board[1][j] == difcolor) ? MAX : 1);
	for (int i = 2; i <= 11; i++)
		for (int j = 1; j <= 11; j++) {
			if (board[i][j] == difcolor) { mapred1[i][j] = MAX; continue; }
			int min1 = 2 * MAX, min2 = 2 * MAX;
			for (int k = 0; k < 3; k++) {
				int nx = i + dx3[k], ny = j + dy3[k], mx = i + dx4[k], my = j + dy4[k];
				if (nx >= 1 && nx <= 11 && ny >= 1 && ny <= 11) {
					if (board[nx][ny] == samecolor) {
						if (mapred1[nx][ny] <= min1 && mapred1[nx][ny] < min2) {
							min1 = mapred1[nx][ny]; min2 = min1;
						}
						else if (mapred1[nx][ny] < min2) {
							min2 = mapred1[nx][ny];
						}
					}
					else {
						if (mapred1[nx][ny] <= min1 && mapred1[nx][ny] < min2) {
							min2 = min1; min1 = mapred1[nx][ny];
						}
						else if (mapred1[nx][ny] < min2) {
							min2 = mapred1[nx][ny];
						}
					}
				}
				if (mx >= 1 && mx <= 11 && my >= 1 && my <= 11 && board[mx][my] == samecolor && visited[mx][my] != 0) {
					if (mapred1[mx][my] <= min1 && mapred1[mx][my] < min2) {
						min1 = mapred1[mx][my]; min2 = min1;
					}
					else if (mapred1[mx][my] < min2) {
						min2 = mapred1[mx][my];
					}
				}
			}
			int potential = min2 + 1;
			if (board[i][j] == samecolor && (visited[i][j] == 0 || mapred1[i][j] > potential - 1)) {
				mapred1[i][j] = potential - 1;
				BFSevaluate(visited, mapred1, i, j, samecolor);
			}
			else if (board[i][j] != samecolor) mapred1[i][j] = potential;
		}
	//修正第11行潜力值
	int min1 = 2 * MAX, min2 = 2 * MAX;
	for (int j = 1; j <= 11; j++) {
		if (board[11][j] == samecolor) {
			if (mapred1[11][j] <= min1 && mapred1[11][j] < min2) {
				min1 = mapred1[11][j]; min2 = min1;
			}
			else if (mapred1[11][j] < min2) {
				min2 = mapred1[11][j];
			}
		}
		else {
			if (mapred1[11][j] <= min1 && mapred1[11][j] < min2) {
				min2 = min1; min1 = mapred1[11][j];
			}
			else if (mapred1[11][j] < min2) {
				min2 = mapred1[11][j];
			}
		}
	}
	int min = min2;
	for (int j = 1; j <= 11; j++) {
		if (board[11][j] != difcolor) {
			if (board[11][j] == samecolor && mapred1[11][j] > min) {
				mapred1[11][j] = min;
				BFSevaluate(visited, mapred1, 11, j, samecolor);
			}
			else if (board[11][j] == 0 && mapred1[11][j] > min + 1) mapred1[11][j] = min + 1;
		}
	}
}
void evaluatered2(int(&mapred2)[SIZE][SIZE], int samecolor, int difcolor, int pcolor) { //求所有点到左下边的双距离
	int visited[SIZE][SIZE] = { 0 };
	if (pcolor == samecolor) {
		for (int j = 11; j >= 3; j--) {
			if (board[9][j] == pcolor && IsEdgePattens(9, j, 4)) {
				visited[9][j] = 100;
			}
		}
	}
	for (int j = 1; j <= 11; j++)
		mapred2[11][j] = (board[11][j] == samecolor) ? 0 : ((board[11][j] == difcolor) ? MAX : 1);
	for (int i = 10; i >= 1; i--)
		for (int j = 11; j >= 1; j--) {
			if (board[i][j] == difcolor) { mapred2[i][j] = MAX; continue; }
			int min1 = 2 * MAX, min2 = 2 * MAX;
			for (int k = 0; k < 3; k++) {
				int nx = i + dx4[k], ny = j + dy4[k], mx = i + dx3[k], my = j + dy3[k];
				if (nx >= 1 && nx <= 11 && ny >= 1 && ny <= 11) {
					if (board[nx][ny] == samecolor) {
						if (mapred2[nx][ny] <= min1 && mapred2[nx][ny] < min2) {
							min1 = mapred2[nx][ny]; min2 = min1;
						}
						else if (mapred2[nx][ny] < min2) {
							min2 = mapred2[nx][ny];
						}
					}
					else {
						if (mapred2[nx][ny] <= min1 && mapred2[nx][ny] < min2) {
							min2 = min1; min1 = mapred2[nx][ny];
						}
						else if (mapred2[nx][ny] < min2) {
							min2 = mapred2[nx][ny];
						}
					}
				}
				if (mx >= 1 && mx <= 11 && my >= 1 && my <= 11 && board[mx][my] == samecolor && visited[mx][my] != 0) {
					if (mapred2[mx][my] <= min1 && mapred2[mx][my] < min2) {
						min1 = mapred2[mx][my]; min2 = min1;
					}
					else if (mapred2[mx][my] < min2) {
						min2 = mapred2[mx][my];
					}
				}
			}
			int potential = min2 + 1;
			if (board[i][j] == samecolor && (visited[i][j] == 0 || mapred2[i][j] > potential - 1)) {
				mapred2[i][j] = potential - 1;
				BFSevaluate(visited, mapred2, i, j, samecolor);
			}
			else if (board[i][j] != samecolor) mapred2[i][j] = potential;
		}
	//修正第1行潜力值
	int min1 = 2 * MAX, min2 = 2 * MAX;
	for (int j = 1; j <= 11; j++) {
		if (board[1][j] == samecolor) {
			if (mapred2[1][j] <= min1 && mapred2[1][j] < min2) {
				min1 = mapred2[1][j]; min2 = min1;
			}
			else if (mapred2[1][j] < min2) {
				min2 = mapred2[1][j];
			}
		}
		else {
			if (mapred2[1][j] <= min1 && mapred2[1][j] < min2) {
				min2 = min1; min1 = mapred2[1][j];
			}
			else if (mapred2[1][j] < min2) {
				min2 = mapred2[1][j];
			}
		}
	}
	int min = min2;
	for (int j = 11; j >= 1; j--) {
		if (board[1][j] != difcolor) {
			if (board[1][j] == samecolor && mapred2[1][j] > min) {
				mapred2[1][j] = min;
				BFSevaluate(visited, mapred2, 1, j, samecolor);
			}
			else if (board[1][j] == 0 && mapred2[1][j] > min + 1) mapred2[1][j] = min + 1;
		}
	}
}
bool cmp(Point a, Point b) {
	return a.value < b.value;
}
int evaluate(int sx, int sy) { //对当前局面进行估值,sx,sy为当前下的点
	cnt1++;
	int mapblue1[SIZE][SIZE] = { 0 }, mapblue2[SIZE][SIZE] = { 0 }, mapred1[SIZE][SIZE] = { 0 }, mapred2[SIZE][SIZE] = { 0 };
	int E = 0, M = 11 * 11, pb = MAX * MAX, pr = MAX * MAX, ab = 0, ar = 0;
	int red = board[2][3], blue = -red;
	evaluateblue1(mapblue1, blue, red, -1);
	evaluateblue2(mapblue2, blue, red, -1);
	evaluatered1(mapred1, red, blue, -1);
	evaluatered2(mapred2, red, blue, -1);
	for (int i = 1; i < 12; i++) {
		for (int j = 1; j < 12; j++) {
			if (board[i][j] == 0) {
				if (pb > mapblue1[i][j] + mapblue2[i][j]) { pb = mapblue1[i][j] + mapblue2[i][j]; ab = 0; }
				else if (pb == mapblue1[i][j] + mapblue2[i][j]) { ab++; }
				if (pr > mapred1[i][j] + mapred2[i][j]) { pr = mapred1[i][j] + mapred2[i][j]; ar = 0; }
				else if (pr == mapred1[i][j] + mapred2[i][j]) { ar++; }
			}
		}
	}
	if (board[sx][sy] == blue) { E = M * (pr - pb) - (ar - ab); } //估值函数越大越有利
	else { E = M * (pb - pr) - (ab - ar); }
	return E;
}
void evaluate(GameTreeNode* t) { //对当前局面进行评估
	cnt1++;
	int mapblue1[SIZE][SIZE] = { 0 }, mapblue2[SIZE][SIZE] = { 0 }, mapred1[SIZE][SIZE] = { 0 }, mapred2[SIZE][SIZE] = { 0 };
	int pb = MAX * MAX, pr = MAX * MAX;
	int red = board[2][3], blue = -red;
	int pcolor = -board[t->x][t->y]; //对方的颜色
	evaluateblue1(mapblue1, blue, red, pcolor);
	evaluateblue2(mapblue2, blue, red, pcolor);
	evaluatered1(mapred1, red, blue, pcolor);
	evaluatered2(mapred2, red, blue, pcolor);
	for (int i = 1; i < 12; i++) {
		for (int j = 1; j < 12; j++) {
			if (board[i][j] == 0) {
				if (pb > mapblue1[i][j] + mapblue2[i][j]) { pb = mapblue1[i][j] + mapblue2[i][j]; }
				if (pr > mapred1[i][j] + mapred2[i][j]) { pr = mapred1[i][j] + mapred2[i][j]; }
				t->childpoint[t->cnt].x = i; t->childpoint[t->cnt].y = j;
				t->childpoint[t->cnt].value = mapblue1[i][j] + mapblue2[i][j] + mapred1[i][j] + mapred2[i][j];
				t->movebadness[i][j] = t->childpoint[t->cnt].value;
				t->cnt++;
				if (t->minbadness > t->movebadness[i][j]) t->minbadness = t->movebadness[i][j];
			}
		}
	}
	t->pb = pb; t->pr = pr;
	sort(t->childpoint, t->childpoint + t->cnt, cmp);
}
//开局评估
int MoveBadness(int(&movebadness)[SIZE][SIZE], int mx, int my, int px, int py, int& x, int& y, int& k) {
	int mapblue1[SIZE][SIZE] = { 0 }, mapblue2[SIZE][SIZE] = { 0 }, mapred1[SIZE][SIZE] = { 0 }, mapred2[SIZE][SIZE] = { 0 };
	int totalmin = MAX * MAX;
	int red = board[2][3], blue = -red;
	evaluateblue1(mapblue1, blue, red, -1);
	evaluateblue2(mapblue2, blue, red, -1);
	evaluatered1(mapred1, red, blue, -1);
	evaluatered2(mapred2, red, blue, -1);
	for (int i = 1; i < 12; i++) {
		for (int j = 1; j < 12; j++) {
			if (board[i][j] == 0) {
				movebadness[i][j] = mapblue1[i][j] + mapblue2[i][j] + mapred1[i][j] + mapred2[i][j];
				if (totalmin > movebadness[i][j]) totalmin = movebadness[i][j];
			}
		}
	}
	k = IsLadder(mx, my, px, py); //判断当前是否在建梯子
	if (k != -1) {
		x = mx + dx[k]; y = my + dy[k];
		if (movebadness[x + dx[k]][y + dy[k]] > movebadness[x][y]) {
			movebadness[x + dx[k]][y + dy[k]] = movebadness[x][y] - 1;
		}
	}
	return totalmin;
}
void VantagePointSearch(int(&E)[SIZE][SIZE], int mx, int my, int px, int py, int& new_x, int& new_y, int& k) {
	int movebadness[SIZE][SIZE] = { 0 };
	int totalmin = MAX * MAX, bestvalue = -MAX * MAX, x = 0, y = 0;
	totalmin = MoveBadness(movebadness, mx, my, px, py, x, y, k);
	for (int i = 1; i < 12; i++) {
		for (int j = 1; j < 12; j++) {
			if (board[i][j] == 0) {
				board[i][j] = 1;
				E[i][j] = evaluate(i, j) - (movebadness[i][j] - totalmin);
				board[i][j] = 0;
				if (bestvalue < E[i][j]) { new_x = i; new_y = j; bestvalue = E[i][j]; }
				//printf("%d %d %d\t", i, j, E[i][j]);
			}
		}
		//printf("\n");
	}
	if (k != -1 && x == new_x && y == new_y && board[x + dx[k]][y + dy[k]] == 0 && E[x][y] - E[x + dx[k]][y + dy[k]] < 200) {
		new_x = x + dx[k]; new_y = y + dy[k];
	}
	//printf("\n%d %d %d\n", new_x + 1, new_y + 1, bestvalue);
}
//以下为博弈树搜索代码
int GameTreeDFS(GameTreeNode*& t, int alpha, int beta, int depth, int childcnt, int dd) {
	if ((double)(clock() - start) / CLOCKS_PER_SEC > 0.97) return 0;
	if (depth == 0) {
		if (t->ifend == false) {
			t->value = evaluate(t->x, t->y) - (t->parent->movebadness[t->x][t->y] - t->parent->minbadness);
			t->ifend = true;
		}
		return t->value;
	}

	int bestvalue = -MAX * MAX, value = 0, x = 0, y = 0, d = t->d, cnt = 1;
	bool mycolor = (board[t->x][t->y] == board[2][3]) ? RED : BLUE;
	if (t->cnt == 0) evaluate(t);

	x = t->childpoint[0].x, y = t->childpoint[0].y;
	if (t->child[x][y] == NULL) t->child[x][y] = new GameTreeNode(x, y, t);
	board[x][y] = -board[t->x][t->y];
	bestvalue = -GameTreeDFS(t->child[x][y], -beta, -alpha, depth - 1, 0, dd + 1);
	board[x][y] = 0;
	alpha = bestvalue;

	if (childcnt < 30) {
		cnt = (t->cnt > 20) ? 20 : t->cnt;
		for (int i = 1; i < cnt && t->childpoint[i].value - t->minbadness < 300; i++) {
			x = t->childpoint[i].x, y = t->childpoint[i].y;
			if (t->child[x][y] == NULL) t->child[x][y] = new GameTreeNode(x, y, t);
			board[x][y] = -board[t->x][t->y];
			value = -GameTreeDFS(t->child[x][y], -beta, -alpha, depth - 1, i, dd + 1);
			board[x][y] = 0;
			if (value > bestvalue) { bestvalue = value; }
			if (alpha < bestvalue) alpha = bestvalue;
			if (bestvalue >= beta) { break; }
		}
	}

	if (d == 0) {
		if (mycolor == RED) { d = t->parent->pr - t->pr; }
		else { d = t->parent->pb - t->pb; }
		if (depth % 2 == 0) {
			if (d % 2 == 1) d--;
			if (d == 0) {
				t->d = 0;
				if (t->ifend == false) {
					t->value = evaluate(t->x, t->y) - (t->parent->movebadness[t->x][t->y] - t->parent->minbadness);
					t->ifend = true;
				}
				return t->value;
			}
		}
		if (depth % 2 == 1 && d % 2 == 0) d = (d == 0) ? d + 1 : d - 1;
		if (d < 0) { t->d = 0; return (dd == 1) ? -MAXI : MAXI; }
		t->d = d;
	}
	if (t->parent->d < d + 1) t->parent->d = d + 1;
	if (d > depth) d = depth;

	for (int i = cnt; i < t->cnt && t->childpoint[i].value - t->minbadness < 300; i++) {
		x = t->childpoint[i].x, y = t->childpoint[i].y;
		if (t->child[x][y] == NULL) t->child[x][y] = new GameTreeNode(x, y, t);
		board[x][y] = -board[t->x][t->y];
		value = -GameTreeDFS(t->child[x][y], -beta, -alpha, d - 1, i, dd + 1);
		if (value > bestvalue) { bestvalue = value; }
		board[x][y] = 0;
		if (alpha < bestvalue) alpha = bestvalue;
		if (bestvalue >= beta) { break; }
	}
	return bestvalue;
}
void GameTreeSearch(int E[SIZE][SIZE], int mx, int my, int px, int py, int& new_x, int& new_y, int& flag2) {
	GameTreeNode* head = new GameTreeNode(px, py);
	int bestvalue = -MAX * MAX, value = 0, alpha = -MAX * MAX, beta = MAX * MAX, sx = 0, sy = 0, d = 0;
	evaluate(head);
	while ((double)(clock() - start) / CLOCKS_PER_SEC < 0.9) {
		d += 2;
		for (int i = 0; i < head->cnt && head->childpoint[i].value - head->minbadness < 500; i++) {
			int x = head->childpoint[i].x, y = head->childpoint[i].y;
			if (head->child[x][y] == NULL) head->child[x][y] = new GameTreeNode(x, y, head);
			board[x][y] = 1;
			value = (int)(0.5 * GameTreeDFS(head->child[x][y], alpha, beta, d, 1, 1) + 0.5 * E[x][y]);
			board[x][y] = 0;
			if (bestvalue < value) {
				sx = x; sy = y;
				bestvalue = value;
				alpha = value;
			} //我方取极大值
			//printf("%d %d %d %d\n", x, y, head->child[x][y]->d, value);
		}
		if ((double)(clock() - start) / CLOCKS_PER_SEC < 0.97 && sx != 0 && sy != 0) { new_x = sx; new_y = sy; flag2 = 1; }
	}
}
int main() {
	int x, y, n, mx = 0, my = 0;
	start = clock();
	//恢复目前的棋盘信息
	cin >> n;
	for (int i = 0; i < n - 1; i++) {
		cin >> x >> y; if (x != -1) board[x + 1][y + 1] = -1;	//对方
		cin >> x >> y; if (x != -1) board[x + 1][y + 1] = 1;	//我方
		if (i == n - 2) { mx = x + 1; my = y + 1; }
	}
	cin >> x >> y;
	if (x != -1) board[++x][++y] = -1;	//对方
	else { cout << 1 << ' ' << 2 << endl; return 0; }
	if (n == 1) { cout << 7 << ' ' << 3 << endl;	return 0; }
	if (n == 2 && board[2][3] == 1) {
		if (x > 6 && y < 6) { cout << x - 3 << ' ' << y + 2 << endl; return 0; }
		else if (y >= 6) { cout << 7 << ' ' << 3 << endl; return 0; }
	}
	//此时board[][]里存储的就是当前棋盘的所有棋子信息,x和y存的是对方最近一步下的棋,mx和my是我方最近一步下的棋

	int E[SIZE][SIZE] = { 0 };
	int new_x = 0, new_y = 0, x1 = 0, y1 = 0, x2 = 0, y2 = 0, flag1 = -1, flag2 = 0;
	VantagePointSearch(E, mx, my, x, y, x1, y1, flag1);
	GameTreeSearch(E, mx, my, x, y, x2, y2, flag2);
	//printf("耗时：%f s\n", (double)(clock() - start) / CLOCKS_PER_SEC);
	//printf("%d\n", cnt1);
	if (flag2 == 1) {
		if (flag1 != -1 && x2 + dx[flag1] == x1 && y2 + dy[flag1] == y1) { new_x = x1 - 1; new_y = y1 - 1; }
		else { new_x = x2 - 1; new_y = y2 - 1; }
	}
	else { new_x = x1 - 1; new_y = y1 - 1; }

	// 向平台输出决策结果
	cout << new_x << ' ' << new_y << endl; //注意棋盘位置被更改
	return 0;
}