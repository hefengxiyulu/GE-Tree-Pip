#pragma once

#define NODE_MAX_POINTS_NUMBER 100

#include "Grid.h"
#include "Node.h"
#include "Statistics.h"
using namespace std;

class GQT
{
private:
	Grid *grid;
	Point botLeft, topRight;
	Node* root = NULL;
	//Statistic stat = Statistic(0, 0, 0, 0);
	double grid_width, grid_height, minX, minY, cell_width, cell_height;
	//the number of grid is x*y
	int cell_number_x, cell_number_y;
public:
	Statistic stat = Statistic(0, 0, 0, 0,0,0);
	GQT(int _cell_number_x, int _cell_number_y, Point _botLeft, Point _topRight) : botLeft(_botLeft), topRight(_topRight),
		cell_number_x(_cell_number_x), cell_number_y(_cell_number_y) {
		//enable grid
		minX = _botLeft.x;
		minY = _botLeft.y;
		grid_width = _topRight.x - _botLeft.x;
		grid_height = _topRight.y - _botLeft.y;
		cell_width = grid_width / _cell_number_x;
		cell_height = grid_height / _cell_number_y;

		Point root_center(_cell_number_x / 2, _cell_number_y / 2);
		root = new Node(root_center, true, false, Point(0, 0), Point(_cell_number_x, _cell_number_y));
		grid = new Grid(_cell_number_x, _cell_number_y, root);

	}
	Node* findNode(Point &p);
	Node* findNode_stat(Point &p);
	queue<Obj*> findNeighbor(Obj* n, Point source);
	queue<Obj*> findNeighbor_stat(Obj* n, Point source);
	void insertPoint(Point p);
	void subdivide(Node* n);
	void update_grid(Node* n);
	vector<Point> kNN(Point source, int k);
	vector<Point> kNN_stat(Point source, int k);
	void initializeVisited(Node* n);
	void initializeVisited_stat(Node* n);
	double caculateDistance(Point p, Node* n);
	double caculateDistance_stat(Point p, Node* n);
	Statistic& getStat();

};


