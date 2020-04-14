#include "pch.h"
#include "Grid.h"
#include "Node.h"
#include "GQT.h"
#include <iostream> 
#include <vector>
#include <queue>
#include <cmath> 
#include <fstream>
#include <strstream> 

using namespace std;

//comparator for pririoty queue
class Compare {
public:
	bool operator () (const Obj* l, const Obj* r) {
		return l->distance > r->distance;
	}
};

//lazy maintainance
void GQT::updateGrid(Node* n) {
	if (n->is_leaf) {
		Point bot = n->boundary_bot_left;
		Point top = n->boundary_top_right;
		for (int i = bot.IntX; i < top.IntX; i++) {
			for (int j = bot.IntY; j < top.IntY; j++) {
				grid->cells[i][j] = n;
			}
		}
		return;
	}
	updateGrid(n->nw);
	updateGrid(n->ne);
	updateGrid(n->se);
	updateGrid(n->sw);

}

//split the node containing more points than threshold
void GQT::subdivide(Node* n) {
	if (n->boundary_bot_left.IntX >= n->boundary_top_right.IntX || n->boundary_bot_left.IntY >= n->boundary_top_right.IntY)
		return;
	queue<Point> q = n->obj_array;
	Point nw_center((n->center.IntX + n->boundary_bot_left.IntX) / 2, (n->center.IntY + n->boundary_top_right.IntY) / 2);
	Point ne_center((n->center.IntX + n->boundary_top_right.IntX) / 2, (n->center.IntY + n->boundary_top_right.IntY) / 2);
	Point se_center((n->center.IntX + n->boundary_top_right.IntX) / 2, (n->center.IntY + n->boundary_bot_left.IntY) / 2);
	Point sw_center((n->center.IntX + n->boundary_bot_left.IntX) / 2, (n->center.IntY + n->boundary_bot_left.IntY) / 2);

	Point left_middle_point(n->boundary_bot_left.IntX, n->center.IntY);
	Point top_midlle_point(n->center.IntX, n->boundary_top_right.IntY);
	Point right_midlle_point(n->boundary_top_right.IntX, n->center.IntY);
	Point bot_middle_point(n->center.IntX, n->boundary_bot_left.IntY);

	n->nw = new Node(nw_center, true, false, left_middle_point, top_midlle_point);
	n->ne = new Node(ne_center, true, false, n->center, n->boundary_top_right);
	n->se = new Node(se_center, true, false, bot_middle_point, right_midlle_point);
	n->sw = new Node(sw_center, true, false, n->boundary_bot_left, n->center);
	
	n->is_leaf = false;

	while (!q.empty()) {
		Point loc = q.front();
		//north west rectangle
		if (loc.IntX >= left_middle_point.IntX && loc.IntX < top_midlle_point.IntX
			&& loc.IntY >= left_middle_point.IntY && loc.IntY < top_midlle_point.IntY) {
			n->nw->obj_array.push(q.front());
			if (n->nw->obj_array.size() > NODE_MAX_POINTS_NUMBER) subdivide(n->nw);
		}
		//north east rectangle
		else if (loc.IntX >= top_midlle_point.IntX && loc.IntX < right_midlle_point.IntX
			&& loc.IntY >= left_middle_point.IntY && loc.IntY < top_midlle_point.IntY) {
			n->ne->obj_array.push(q.front());
			if (n->ne->obj_array.size() > NODE_MAX_POINTS_NUMBER) subdivide(n->ne);
		}
		//south east rectangle
		else if (loc.IntX >= top_midlle_point.IntX && loc.IntX < right_midlle_point.IntX
			&& loc.IntY >= bot_middle_point.IntY && loc.IntY < right_midlle_point.IntY) {
			n->se->obj_array.push(q.front());
			if (n->se->obj_array.size() > NODE_MAX_POINTS_NUMBER) subdivide(n->se);
		}
		//south west rectangle
		else if (loc.IntX >= left_middle_point.IntX && loc.IntX < top_midlle_point.IntX
			&& loc.IntY >= bot_middle_point.IntY && loc.IntY < right_midlle_point.IntY) {
			n->sw->obj_array.push(q.front());
			if (n->sw->obj_array.size() > NODE_MAX_POINTS_NUMBER) subdivide(n->sw);
		}
		q.pop();
	}
	return;
}

void GQT::subdivide_stat(Node* n) {
	if (n->boundary_bot_left.IntX >= n->boundary_top_right.IntX || n->boundary_bot_left.IntY >= n->boundary_top_right.IntY)
		return;
	queue<Point> q = n->obj_array;
	Point nw_center((n->center.IntX + n->boundary_bot_left.IntX) / 2, (n->center.IntY + n->boundary_top_right.IntY) / 2);
	Point ne_center((n->center.IntX + n->boundary_top_right.IntX) / 2, (n->center.IntY + n->boundary_top_right.IntY) / 2);
	Point se_center((n->center.IntX + n->boundary_top_right.IntX) / 2, (n->center.IntY + n->boundary_bot_left.IntY) / 2);
	Point sw_center((n->center.IntX + n->boundary_bot_left.IntX) / 2, (n->center.IntY + n->boundary_bot_left.IntY) / 2);

	Point left_middle_point(n->boundary_bot_left.IntX, n->center.IntY);
	Point top_midlle_point(n->center.IntX, n->boundary_top_right.IntY);
	Point right_midlle_point(n->boundary_top_right.IntX, n->center.IntY);
	Point bot_middle_point(n->center.IntX, n->boundary_bot_left.IntY);

	n->nw = new Node(nw_center, true, false, left_middle_point, top_midlle_point);
	n->ne = new Node(ne_center, true, false, n->center, n->boundary_top_right);
	n->se = new Node(se_center, true, false, bot_middle_point, right_midlle_point);
	n->sw = new Node(sw_center, true, false, n->boundary_bot_left, n->center);

	stat.cnt_memory += (sizeof(*n->nw) + sizeof(*n->ne) + sizeof(*n->se) + sizeof(*n->sw));
	n->is_leaf = false;

	while (!q.empty()) {
		Point loc = q.front();
		//north west rectangle
		if (loc.IntX >= left_middle_point.IntX && loc.IntX < top_midlle_point.IntX
			&& loc.IntY >= left_middle_point.IntY && loc.IntY < top_midlle_point.IntY) {
			n->nw->obj_array.push(q.front());
			if (n->nw->obj_array.size() > NODE_MAX_POINTS_NUMBER) subdivide(n->nw);
		}
		//north east rectangle
		else if (loc.IntX >= top_midlle_point.IntX && loc.IntX < right_midlle_point.IntX
			&& loc.IntY >= left_middle_point.IntY && loc.IntY < top_midlle_point.IntY) {
			n->ne->obj_array.push(q.front());
			if (n->ne->obj_array.size() > NODE_MAX_POINTS_NUMBER) subdivide(n->ne);
		}
		//south east rectangle
		else if (loc.IntX >= top_midlle_point.IntX && loc.IntX < right_midlle_point.IntX
			&& loc.IntY >= bot_middle_point.IntY && loc.IntY < right_midlle_point.IntY) {
			n->se->obj_array.push(q.front());
			if (n->se->obj_array.size() > NODE_MAX_POINTS_NUMBER) subdivide(n->se);
		}
		//south west rectangle
		else if (loc.IntX >= left_middle_point.IntX && loc.IntX < top_midlle_point.IntX
			&& loc.IntY >= bot_middle_point.IntY && loc.IntY < right_midlle_point.IntY) {
			n->sw->obj_array.push(q.front());
			if (n->sw->obj_array.size() > NODE_MAX_POINTS_NUMBER) subdivide(n->sw);
		}
		q.pop();
	}
	return;
}

void GQT::initializeVisited(Node* n) {
	if (!n) return;
	initializeVisited(n->nw);
	initializeVisited(n->ne);
	initializeVisited(n->sw);
	initializeVisited(n->se);
	if (n->visited) n->visited = false;
}

void GQT::initializeVisited_stat(Node* n) 
{
	stat.cnt_compare++;
	if (!n) return;
	initializeVisited_stat(n->nw);
	initializeVisited_stat(n->ne);
	initializeVisited_stat(n->sw);
	initializeVisited_stat(n->se);
	if (n->visited) n->visited = false;
	stat.cnt_compare++;
}

//get distance
double GQT::caculateDistance(Point p, Node* n)
{
	//left under
	if (p.IntX < n->boundary_bot_left.IntX && p.IntY < n->boundary_bot_left.IntY)
		return p.caculateDistance(n->boundary_bot_left);
	//left middle 
	else if (p.IntX <= n->boundary_bot_left.IntX && p.IntY >= n->boundary_bot_left.IntY && p.IntY <= n->boundary_top_right.IntY)
		return n->boundary_bot_left.IntX - p.IntX;
	//left up
	else if (p.IntX < n->boundary_bot_left.IntX && p.IntY > n->boundary_top_right.IntY)
		return p.caculateDistance(Point(n->boundary_bot_left.IntX, n->boundary_top_right.IntY));
	//up
	else if (p.IntX >= n->boundary_bot_left.IntX  && p.IntX <= n->boundary_top_right.IntX && p.IntY >= n->boundary_top_right.IntY)
		return p.IntY - n->boundary_top_right.IntY;
	//right up
	else if (p.IntX > n->boundary_top_right.IntX && p.IntY > n->boundary_top_right.IntY)
		return p.caculateDistance(n->boundary_top_right);
	//right middle
	else if (p.IntX >= n->boundary_top_right.IntX && p.IntY >= n->boundary_bot_left.IntY && p.IntY <= n->boundary_top_right.IntY)
		return p.IntX - n->boundary_top_right.IntX;
	//right under
	else if (p.IntX > n->boundary_top_right.IntX && p.IntY < n->boundary_bot_left.IntY)
		return p.caculateDistance(Point(n->boundary_top_right.IntX, n->boundary_bot_left.IntY));
	//under
	else if (p.IntX >= n->boundary_bot_left.IntX  && p.IntX <= n->boundary_top_right.IntX && p.IntY <= n->boundary_bot_left.IntY)
		return n->boundary_bot_left.IntY - p.IntY;
	else throw "error!";
}

double GQT::caculateDistance_stat(Point p, Node* n)
{
	//left under
	if (stat.cnt_compare++ && (p.IntX < n->boundary_bot_left.IntX) 
		&& stat.cnt_compare++ && (p.IntY < n->boundary_bot_left.IntY))
	{
		stat.cnt_add += 5;
		stat.cnt_multiply += 2;
		return p.caculateDistance(n->boundary_bot_left);
	}
	//left middle 
	else if (stat.cnt_compare++ && (p.IntX <= n->boundary_bot_left.IntX) 
		&& stat.cnt_compare++ &&(p.IntY >= n->boundary_bot_left.IntY) 
		&& stat.cnt_compare++ && (p.IntY <= n->boundary_top_right.IntY))
	{
		stat.cnt_add++;
		return n->boundary_bot_left.IntX - p.IntX;
	}
	//left up
	else if (stat.cnt_compare++ && (p.IntX < n->boundary_bot_left.IntX)
		&& stat.cnt_compare++ && (p.IntY > n->boundary_top_right.IntY))
	{
		stat.cnt_add += 5;
		stat.cnt_multiply += 2;
		return p.caculateDistance(Point(n->boundary_bot_left.IntX, n->boundary_top_right.IntY));
	}
	//up
	else if (stat.cnt_compare++&&(p.IntX >= n->boundary_bot_left.IntX) 
		&&stat.cnt_compare++&&(p.IntX <= n->boundary_top_right.IntX) 
		&&stat.cnt_compare++ && ( p.IntY >= n->boundary_top_right.IntY))
	{
		stat.cnt_add++;
		return p.IntY - n->boundary_top_right.IntY;
	}
	//right up
	else if (stat.cnt_compare++ &&( p.IntX > n->boundary_top_right.IntX)
		&& stat.cnt_compare++ && (p.IntY > n->boundary_top_right.IntY))
	{
		stat.cnt_add += 5;
		stat.cnt_multiply += 2;
		return p.caculateDistance(n->boundary_top_right);
	}
	//right middle
	else if (stat.cnt_compare++ && (p.IntX >= n->boundary_top_right.IntX)
		&& stat.cnt_compare++ && (p.IntY >= n->boundary_bot_left.IntY)
		&& stat.cnt_compare++ && (p.IntY <= n->boundary_top_right.IntY))
	{
		stat.cnt_add++;
		return p.IntX - n->boundary_top_right.IntX;
	}
	//right under
	else if (stat.cnt_compare++ && (p.IntX > n->boundary_top_right.IntX)
		&& stat.cnt_compare++ && (p.IntY < n->boundary_bot_left.IntY))
	{
		stat.cnt_add += 5;
		stat.cnt_multiply += 2;
		return p.caculateDistance(Point(n->boundary_top_right.IntX, n->boundary_bot_left.IntY));
	}
	//under
	else if (stat.cnt_compare++ && (p.IntX >= n->boundary_bot_left.IntX)
		&& stat.cnt_compare++ && (p.IntX <= n->boundary_top_right.IntX)
		&& stat.cnt_compare++ && (p.IntY <= n->boundary_bot_left.IntY))
	{
		stat.cnt_add++;
		return n->boundary_bot_left.IntY - p.IntY;
	}
	else throw "error!";
}
//insert new point(establish the quadtree and update the grid)
void GQT::insertPoint(Point p) {
	//check whether over the threshold
	//if over then split

	Node* n = findNode(p);
	if (n->obj_array.size() >= NODE_MAX_POINTS_NUMBER)
	{
		n->obj_array.push(p);
		subdivide(n);
		updateGrid(n);
	}
	else n->obj_array.push(p);
}
//kNN method
vector<Point> GQT::kNN(Point source, int k) {
	vector<Point> result;
	Node* leafnode = findNode(source);
	priority_queue<Obj*, vector<Obj*>, Compare> obj_set;
	initializeVisited(this->root);
	obj_set.push(new Obj(leafnode, 0));
	while (!obj_set.empty()) {
		Obj* element = obj_set.top();
		obj_set.pop();
		if (!element->isNode) {
			result.push_back(element->pos);
			if (result.size() == k) {
				while (!obj_set.empty()) {
					delete obj_set.top();
					obj_set.pop();
				}
				return result;
			}
		}
		else
		{
			if (!element->node->visited) {
				queue<Point> temp;
				queue<Obj*> neighbor = findNeighbor(element, source);
				element->node->visited = true;
				while (!element->node->obj_array.empty()) {
					temp.push(element->node->obj_array.front());
					obj_set.push(new Obj(temp.back(), source.caculateDistance(temp.back())));
					element->node->obj_array.pop();
				}
				element->node->obj_array.swap(temp);
				while (!obj_set.empty() && element == obj_set.top()) obj_set.pop();
				delete element;
				while (!neighbor.empty()) {
					//neighbor.front()->node->visited = true;
					obj_set.push(neighbor.front());
					neighbor.pop();
				}
			}
		}
	}
	return result;
}

vector<Point> GQT::kNN_stat(Point source, int k) {
	vector<Point> result;
	Node* leafnode = findNode_stat(source);
	priority_queue<Obj*, vector<Obj*>, Compare> obj_set;
	initializeVisited_stat(this->root);
	obj_set.push(new Obj(leafnode, 0));
	stat.cnt_memory += sizeof(obj_set.top());
	
	while (!obj_set.empty()) 
	{
		stat.cnt_compare++;

		Obj* element = obj_set.top();
		obj_set.pop();
		stat.cnt_compare++;
		if (!element->isNode) 
		{
			result.push_back(element->pos);
			stat.cnt_compare++;
			if (result.size() == k) 
			{
				while (!obj_set.empty()) 
				{
					stat.cnt_compare++;
					stat.cnt_memory -= sizeof(obj_set.top());
					delete obj_set.top();
					obj_set.pop();
				}
				stat.cnt_compare++;
				return result;
			}
		}
		else
		{
			stat.cnt_compare++;
			if (!element->node->visited) 
			{
				queue<Point> temp;
				queue<Obj*> neighbor = findNeighbor_stat(element, source);
				element->node->visited = true;

				while (!element->node->obj_array.empty()) 
				{
					stat.cnt_compare++;
					temp.push(element->node->obj_array.front());
					obj_set.push(new Obj(temp.back(), source.caculateDistance(temp.back())));
					stat.cnt_add += 5;
					stat.cnt_multiply += 2;

					stat.cnt_memory += sizeof(*obj_set.top());
					element->node->obj_array.pop();
				}
				stat.cnt_compare++;

				element->node->obj_array.swap(temp);
				while (stat.cnt_compare++&&(!obj_set.empty() )
					&& stat.cnt_compare++ && (element == obj_set.top()) )
				{
					obj_set.pop();
				}
				stat.cnt_memory -= sizeof(*element);
				delete element;
				while (!neighbor.empty()) 
				{
					stat.cnt_compare++;
					//neighbor.front()->node->visited = true;
					obj_set.push(neighbor.front());
					neighbor.pop();
				}
				stat.cnt_compare++;
			}
		}
	}
	stat.cnt_compare++;
	return result;
}
//access to the node which contains certain point
Node* GQT::findNode(Point &p) {
	int Ix = floor((p.x - minX) / cell_width);
	int Iy = floor((p.y - minY) / cell_height);
	p.IntX = Ix;
	p.IntY = Iy;
	return grid->cells[Ix][Iy];
}

Node* GQT::findNode_stat(Point &p) {
	int Ix = floor((p.x - minX) / cell_width);
	int Iy = floor((p.y - minY) / cell_height);
	stat.cnt_add += 2; 
	stat.cnt_multiply += 2;
	p.IntX = Ix;
	p.IntY = Iy;
	return grid->cells[Ix][Iy];
}

queue<Obj*> GQT::findNeighbor(Obj* element, Point source) {
	Node* n = NULL;
	Node* temp1 = NULL;
	Node* temp2 = NULL;
	queue<Obj*> result;
	Point p;
	int Ix, Iy;
	double distance;
	if (element->isNode) n = element->node;
	else
	{
		p = element->pos;
		Ix = floor((p.x - minX) / cell_width);
		Iy = floor((p.y - minY) / cell_height);
		n = grid->cells[Ix][Iy];
	}

	for (int i = n->boundary_bot_left.IntY; i < n->boundary_top_right.IntY; i++) {
		if (n->boundary_bot_left.IntX > 0) {
			if (!grid->cells[n->boundary_bot_left.IntX - 1][i]->visited &&
				grid->cells[n->boundary_bot_left.IntX - 1][i] != temp1) {
				temp1 = grid->cells[n->boundary_bot_left.IntX - 1][i];
				result.push(new Obj(temp1, this->caculateDistance(source, temp1)));
			}
		}
		if (n->boundary_top_right.IntX < cell_number_x - 1) {
			if (!grid->cells[n->boundary_top_right.IntX + 1][i]->visited &&
				grid->cells[n->boundary_top_right.IntX + 1][i] != temp2) {
				temp2 = grid->cells[n->boundary_top_right.IntX + 1][i];
				result.push(new Obj(temp2, this->caculateDistance(source, temp2)));
			}

		}
	}
	temp1 = NULL;
	temp2 = NULL;
	//up and down bound
	for (int i = n->boundary_bot_left.IntX; i < n->boundary_top_right.IntX; i++) {
		if (n->boundary_top_right.IntY < cell_number_y - 1) {
			if (!grid->cells[i][n->boundary_top_right.IntY + 1]->visited &&
				grid->cells[i][n->boundary_top_right.IntY + 1] != temp1) {
				temp1 = grid->cells[i][n->boundary_top_right.IntY + 1];
				result.push(new Obj(temp1, this->caculateDistance(source, temp1)));
			}
		}
		if (n->boundary_bot_left.IntY > 0) {
			if (!grid->cells[i][n->boundary_bot_left.IntY - 1]->visited &&
				grid->cells[i][n->boundary_bot_left.IntY - 1] != temp2) {
				temp2 = grid->cells[i][n->boundary_bot_left.IntY - 1];
				result.push(new Obj(temp2, this->caculateDistance(source, temp2)));
			}
		}
	}
	temp1 = NULL;
	temp2 = NULL;
	//corners
	if (n->boundary_bot_left.IntX > 0 && n->boundary_bot_left.IntY > 0) {
		temp1 = grid->cells[n->boundary_bot_left.IntX - 1][n->boundary_bot_left.IntY - 1];
		if (!temp1->visited)
			result.push(new Obj(temp1, this->caculateDistance(source, temp1)));
	}
	if (n->boundary_bot_left.IntX > 0 && n->boundary_top_right.IntY < cell_number_y - 1) {
		temp1 = grid->cells[n->boundary_bot_left.IntX - 1][n->boundary_top_right.IntY + 1];
		if (!temp1->visited)
			result.push(new Obj(temp1, this->caculateDistance(source, temp1)));
	}
	if (n->boundary_top_right.IntX < cell_number_x - 1 && n->boundary_top_right.IntY < cell_number_y - 1) {
		temp1 = grid->cells[n->boundary_top_right.IntX + 1][n->boundary_bot_left.IntY + 1];
		if (!temp1->visited)
			result.push(new Obj(temp1, this->caculateDistance(source, temp1)));
	}
	if (n->boundary_top_right.IntX < cell_number_x - 1 && n->boundary_bot_left.IntY > 0) {
		temp1 = grid->cells[n->boundary_top_right.IntX + 1][n->boundary_bot_left.IntY - 1];
		if (!temp1->visited)
			result.push(new Obj(temp1, this->caculateDistance(source, temp1)));
	}
	return result;
}

queue<Obj*> GQT::findNeighbor_stat(Obj* element, Point source) {
	Node* n = NULL;
	Node* temp1 = NULL;
	Node* temp2 = NULL;
	queue<Obj*> result;
	Point p;
	int Ix, Iy;
	double distance;
	stat.cnt_compare++;
	if (element->isNode) n = element->node;
	else
	{
		p = element->pos;
		Ix = floor((p.x - minX) / cell_width);
		Iy = floor((p.y - minY) / cell_height);
		stat.cnt_add += 2; 
		stat.cnt_multiply += 2;
		n = grid->cells[Ix][Iy];
	}
	
	for (int i = n->boundary_bot_left.IntY; i < n->boundary_top_right.IntY; i++) 
	{
		stat.cnt_compare++;
		stat.cnt_add++;

		stat.cnt_compare++;
		if (n->boundary_bot_left.IntX > 0) {
			if ((stat.cnt_compare++&&stat.cnt_add++&&(!grid->cells[n->boundary_bot_left.IntX - 1][i]->visited ))&&
				(stat.cnt_compare++ &&stat.cnt_add++&& (grid->cells[n->boundary_bot_left.IntX - 1][i] != temp1))) {
				
				temp1 = grid->cells[n->boundary_bot_left.IntX - 1][i];
				stat.cnt_add++;
				result.push(new Obj(temp1, this->caculateDistance_stat(source, temp1)));
				stat.cnt_memory += sizeof(*result.front());
			}
		}

		stat.cnt_compare++;
		stat.cnt_add++;
		if (n->boundary_top_right.IntX < cell_number_x - 1)
		{
			if (stat.cnt_compare++&&stat.cnt_add++&&(!grid->cells[n->boundary_top_right.IntX + 1][i]->visited) 
				&& stat.cnt_compare++ && stat.cnt_add++ && (grid->cells[n->boundary_top_right.IntX + 1][i] != temp2))
			{
				stat.cnt_add++;
				temp2 = grid->cells[n->boundary_top_right.IntX + 1][i];

				result.push(new Obj(temp2, this->caculateDistance_stat(source, temp2)));
				stat.cnt_memory += sizeof(*result.front());
			}

		}
	}
	stat.cnt_compare++;
	temp1 = NULL;
	temp2 = NULL;
	//up and down bound
	for (int i = n->boundary_bot_left.IntX; i < n->boundary_top_right.IntX; i++) 
	{
		stat.cnt_compare++;
		stat.cnt_add++;

		stat.cnt_compare++;
		stat.cnt_add++;
		if (n->boundary_top_right.IntY < cell_number_y - 1) 
		{
			if (stat.cnt_compare++&& stat.cnt_add++&&(!grid->cells[i][n->boundary_top_right.IntY + 1]->visited )
				&& stat.cnt_compare++ && stat.cnt_add++ && (grid->cells[i][n->boundary_top_right.IntY + 1] != temp1))
			{
				stat.cnt_add++;
				temp1 = grid->cells[i][n->boundary_top_right.IntY + 1];
				result.push(new Obj(temp1, this->caculateDistance_stat(source, temp1)));
				stat.cnt_memory += sizeof(*result.front());
			}
		}
		stat.cnt_compare++;
		if (n->boundary_bot_left.IntY > 0) 
		{
			if (stat.cnt_compare++ && stat.cnt_add++ && (!grid->cells[i][n->boundary_bot_left.IntY - 1]->visited )
				&& stat.cnt_compare++ && stat.cnt_add++ && (grid->cells[i][n->boundary_bot_left.IntY - 1] != temp2))
			{
				stat.cnt_add++;
				temp2 = grid->cells[i][n->boundary_bot_left.IntY - 1];
				result.push(new Obj(temp2, this->caculateDistance_stat(source, temp2)));
				stat.cnt_memory += sizeof(*result.back());
			}
		}
	}
	stat.cnt_compare++;
	temp1 = NULL;
	temp2 = NULL;
	//corners
	if (stat.cnt_compare++&&(n->boundary_bot_left.IntX > 0 )
		&& stat.cnt_compare++&&( n->boundary_bot_left.IntY > 0))
	{
		stat.cnt_add += 2;
		temp1 = grid->cells[n->boundary_bot_left.IntX - 1][n->boundary_bot_left.IntY - 1];
		
		stat.cnt_compare++;
		if (!temp1->visited)
		{
			result.push(new Obj(temp1, this->caculateDistance_stat(source, temp1)));
			stat.cnt_memory += sizeof(*result.front());
		}

	}
	if (stat.cnt_compare++&&(n->boundary_bot_left.IntX > 0 )
		&& stat.cnt_compare++ && stat.cnt_add++&&(n->boundary_top_right.IntY < cell_number_y - 1))
	{
		stat.cnt_add += 2;
		temp1 = grid->cells[n->boundary_bot_left.IntX - 1][n->boundary_top_right.IntY + 1];
		
		stat.cnt_compare++;
		if (!temp1->visited) 
		{
			result.push(new Obj(temp1, this->caculateDistance_stat(source, temp1)));
			stat.cnt_memory += sizeof(*result.front());
		}
	}
	if (stat.cnt_compare++&&stat.cnt_add++&&(n->boundary_top_right.IntX < cell_number_x - 1)
		&& stat.cnt_compare++ && stat.cnt_add++ && ( n->boundary_top_right.IntY < cell_number_y - 1))
	{
		stat.cnt_add += 2;
		temp1 = grid->cells[n->boundary_top_right.IntX + 1][n->boundary_bot_left.IntY + 1];
		
		stat.cnt_compare++;
		if (!temp1->visited) 
		{
			result.push(new Obj(temp1, this->caculateDistance_stat(source, temp1)));
			stat.cnt_memory += sizeof(*result.front());
		}
	}
	if (stat.cnt_compare++ && stat.cnt_add++ && (n->boundary_top_right.IntX < cell_number_x - 1)
		&& stat.cnt_compare++ && (n->boundary_bot_left.IntY > 0) )
	{
		stat.cnt_add += 2;
		temp1 = grid->cells[n->boundary_top_right.IntX + 1][n->boundary_bot_left.IntY - 1];
		
		stat.cnt_compare++;
		if (!temp1->visited) {
			result.push(new Obj(temp1, this->caculateDistance_stat(source, temp1)));
			stat.cnt_memory += sizeof(*result.front());
		}
	}
	return result;
}

Statistic& GQT::getStat() {
	return stat;
}

void GQT::GEtree_statStorageCost(int length, int widthint, long long int* auxiliary_cost)
{
	int temp = 0;
	cout << sizeof(bool) << endl;
	queue<Node*> traverse;
	// cell memory
	for (int i = 0; i < length; i++)
		for (int j = 0; j < widthint; j++) {
			temp += sizeof(grid->cells[i][j]);
		}
	// node memory
	traverse.push(root);
	while (!traverse.empty()) {
		Node* n = traverse.front();
		temp += sizeof(*n) + (n->obj_array.size()) * sizeof(Point) - sizeof(Point);
		if (n->se) traverse.push(n->se);
		if (n->sw) traverse.push(n->sw);
		if (n->ne) traverse.push(n->ne);
		if (n->nw) traverse.push(n->nw);
		traverse.pop();
	}
	// GQT memory
	temp += sizeof(Point) * 2 + 6 * sizeof(double) + 2 * sizeof(int) + sizeof(Node*) + sizeof(Grid*);
	(*auxiliary_cost) = temp;
	return;
}
