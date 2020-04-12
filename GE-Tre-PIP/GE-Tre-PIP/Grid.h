#pragma once
#include <vector>
#include "Node.h"

using namespace std;

//the grid
class Grid {
public:
	vector<vector<Node*>> cells;
	Grid(int _cell_number_x, int _cell_number_y, Node *_root)
	{
		//create grid
		cells.resize(_cell_number_x);
		for (int i = 0; i < _cell_number_x; i++) {
			cells[i].resize(_cell_number_y, _root);
		}

	}
};
