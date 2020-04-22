#include <stdio.h>
#include"pip.h"
#include"GQT.h"

struct Point_GPU
{
	double x;
	double y;
	int IntX;
	int IntY;
	bool isVertex;
	int edgeIdx;
};

//the node of quadtree
//struct Node_GPU {
//	Node *sw = NULL;
//	Node *se = NULL;
//	Node *nw = NULL;
//	Node *ne = NULL;
//	double distance;
//	int number;
//	//check leaf or visited in grid maintainance and kNN
//	bool is_leaf, visited;
//	Point center, boundary_bot_left, boundary_top_right;
//	//QNode* obj_array = NULL;
//	int obj_array[NODE_MAX_POINTS_NUMBER + 1];
//};
struct Node_GPU
{
	double distance;
	int number;
	//check leaf or visited in grid maintainance and kNN
	bool is_leaf, visited;
	Point_GPU center, boundary_bot_left, boundary_top_right;
	//QNode* obj_array = NULL;
	int obj_array[NODE_MAX_POINTS_NUMBER + 1];

	//int real_point_num;//用于记录obj_array中真正存储散点的总数
};

struct Obj_GPU
{
public:
	Point_GPU pos;
	Node_GPU *node = NULL;
	double distance;
	bool isNode;
	Obj_GPU(Point_GPU _pos, double _distance) : pos(_pos), distance(_distance), isNode(false) {}
	Obj_GPU(Node_GPU* _node, double _distance) : node(_node), distance(_distance), isNode(true) {}
};


//struct GQT_GPU
//{
//	Point point_set[MAX_NUMBER_AMOUNT];
//	Node quad_tree[MAX_NUMBER_AMOUNT];
//	int* cells[CELL_NUMBER_X][CELL_NUMBER_X];
//	Point botLeft, topRight;
//	Node_GPU* root = NULL;
//	double grid_width, grid_height, minX, minY, cell_width, cell_height;
//	//the number of grid is x*y
//	int cell_number_x, cell_number_y;
//};

struct QNode_GPU {
	union {
		Point item;
		Obj_GPU* obj;
	};
	QNode_GPU* next = 0;
};

struct PQNode_GPU
{
	Obj_GPU* item;
	PQNode_GPU* next = 0;
};
struct Coeffecient_GPU {
	double a, b, c; //coefficient of the equation of edge
	//Coeffecient_GPU(double a, double b, double c) : a(a), b(b), c(c) {};
};

struct GE_TREE_PIP_DATA
{
	//host CPU 
	Point_GPU *h_testpoint;   // test point 
	int *h_testedresult;  // test result

	Point_GPU *h_point_set;   //discrete point
	int *h_point_set_size;       //散点总数
	Node_GPU* h_quad_tree;
	int* h_Gcell;

	Point_GPU *h_botLeft;
	Point_GPU *h_topRight;

	double *h_grid_width;
	double *h_grid_height;
	double	*h_minX;
	double *h_minY;
	double *h_cell_width;
	double *h_cell_height;

	int *h_cell_number_x;
	int *h_cell_number_y;
	int *h_node_number;

	int *h_edge_count;
	Point_GPU *h_vertexTable;
	Edge2D *h_edgeTable;

	Point_GPU *h_n_Point;
	/////////////////////////////////////////////////

	//device GPU
	Point_GPU *d_testpoint;    // test point 
	int *d_testedresult;   // test result

	Point_GPU *d_point_set;   //discrete point
	int *d_point_set_size;       //散点总数
	Node_GPU* d_quad_tree;
	int* d_Gcell;

	Point_GPU *d_botLeft;
	Point_GPU *d_topRight;

	double *d_grid_width;
	double *d_grid_height;
	double	*d_minX;
	double *d_minY;
	double *d_cell_width;
	double *d_cell_height;

	int *d_cell_number_x;
	int *d_cell_number_y;
	int *d_node_number;

	int *d_edge_count;
	Point_GPU *d_vertexTable;
	Edge2D *d_edgeTable;

	Point_GPU *d_n_point;    //用于测试GPU端knn查找结果
};

extern "C" void InitGE_Tree_PIP(GE_TREE_PIP_DATA **ha_pip, GE_TREE_PIP_DATA **da_pip, pip &testPip, GQT &test, unsigned int testSize);
extern "C" void CopyPipValuetoHost(GE_TREE_PIP_DATA *ha_pip, pip &testPip, GQT &test, unsigned int testSize);
extern "C" double Pip_With_Cuda(GE_TREE_PIP_DATA *h_pip, GE_TREE_PIP_DATA *d_pip, pip &testPip, GQT &test, unsigned int testSize);
extern "C" void DeinitGE_Tree_PIP(GE_TREE_PIP_DATA* h_pip, GE_TREE_PIP_DATA *d_pip);
extern "C" void export_GPU_Testresult(const char* filename, GE_TREE_PIP_DATA *h_pip);



