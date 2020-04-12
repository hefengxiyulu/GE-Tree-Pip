#pragma once

#include <queue>

using namespace std;
class Node;

struct Point
{
	double x;
	double y;
	int IntX;
	int IntY;
	bool isVertex;
	int edgeIdx;
	Point() : x(0), y(0) {};
	Point(double _x, double _y) : x(_x), y(_y) {};
	Point(int _x, int _y) : IntX(_x), IntY(_y) {};
	double caculateDistance(Point p) { return sqrt((this->IntX - p.IntX)*(this->IntX - p.IntX) + (this->IntY - p.IntY)*(this->IntY - p.IntY)); }
};

// The object may be node or point combining the distance from the source point 
struct Obj
{
public:
	Point pos;
	Node *node = NULL;
	double distance;
	bool isNode;
	Obj(Point _pos, double _distance) : pos(_pos), distance(_distance), isNode(false) {}
	Obj(Node* _node, double _distance) : node(_node), distance(_distance), isNode(true) {}
};

//the node of quadtree
class Node {
public:
	Node *sw = NULL;
	Node *se = NULL;
	Node *nw = NULL;
	Node *ne = NULL;
	double distance;
	//check leaf or visited in grid maintainance and kNN
	bool is_leaf, visited;
	Point center, boundary_bot_left, boundary_top_right;
	queue<Point> obj_array;
	Node(Point _center, bool _is_leaf, bool _visited, Point _boundary_bot_left, Point _boundary_top_right)
		: center(_center), is_leaf(_is_leaf), visited(_visited), boundary_bot_left(_boundary_bot_left), boundary_top_right(_boundary_top_right) {};
};