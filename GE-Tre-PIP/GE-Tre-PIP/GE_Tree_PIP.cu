#include<iostream> 
#include <stdio.h>
#include"GE_Tree_PIP.cuh"
#include"cuda.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include<math.h>

#include <helper_cuda.h>
#include <helper_functions.h>

__device__ double PointDistance_GPU(Point_GPU s, Point_GPU d)
{
	return sqrtf((s.IntX - d.IntX) * (s.IntX - d.IntX) + (s.IntY - d.IntY) * (s.IntY - d.IntY));
}

__device__ double caculateDistance(Point_GPU p, Node_GPU* n)
{
	//left under
	if (p.IntX < n->boundary_bot_left.IntX && p.IntY < n->boundary_bot_left.IntY) {
		//printf("%lf", PointDistance_GPU(p, n->boundary_bot_left));
		return PointDistance_GPU(p, n->boundary_bot_left);
	}
		
	//left middle 
	else if (p.IntX <= n->boundary_bot_left.IntX && p.IntY >= n->boundary_bot_left.IntY && p.IntY <= n->boundary_top_right.IntY)
		return n->boundary_bot_left.IntX - p.IntX;
	//left up
	else if (p.IntX < n->boundary_bot_left.IntX && p.IntY > n->boundary_top_right.IntY) 
	{
		Point_GPU temp;
		temp.IntX = n->boundary_bot_left.IntX;
		temp.IntY = n->boundary_top_right.IntY;
		return PointDistance_GPU(p, temp);
	}
	//up
	else if (p.IntX >= n->boundary_bot_left.IntX  && p.IntX <= n->boundary_top_right.IntX && p.IntY >= n->boundary_top_right.IntY)
		return p.IntY - n->boundary_top_right.IntY;
	//right up
	else if (p.IntX > n->boundary_top_right.IntX && p.IntY > n->boundary_top_right.IntY) {
		//printf("%lf", PointDistance_GPU(p, n->boundary_top_right));
		return PointDistance_GPU(p, n->boundary_top_right);
	}
	//right middle
	else if (p.IntX >= n->boundary_top_right.IntX && p.IntY >= n->boundary_bot_left.IntY && p.IntY <= n->boundary_top_right.IntY)
		return p.IntX - n->boundary_top_right.IntX;
	//right under
	else if (p.IntX > n->boundary_top_right.IntX && p.IntY < n->boundary_bot_left.IntY) {
		Point_GPU temp;
		temp.IntX = n->boundary_top_right.IntX;
		temp.IntY = n->boundary_bot_left.IntY;
		return PointDistance_GPU(p, temp);
	}
	//under
	else if (p.IntX >= n->boundary_bot_left.IntX  && p.IntX <= n->boundary_top_right.IntX && p.IntY <= n->boundary_bot_left.IntY)
		return n->boundary_bot_left.IntY - p.IntY;
	else return 0;
}

__device__ Node_GPU* findNode(Point_GPU &p, GE_TREE_PIP_DATA* tree)
{
	int Ix = floor((p.x - *(tree->d_minX)) / *(tree->d_cell_width));
	int Iy = floor((p.y - *(tree->d_minY)) / *(tree->d_cell_height));
	p.IntX = Ix;
	p.IntY = Iy;
	return &(tree->d_quad_tree[*(tree->d_Gcell + Ix * 1024 + Iy)]);
}

__device__ QNode_GPU* QPushObj(QNode_GPU* h, Obj_GPU* obj)
{
	QNode_GPU* newnode = (QNode_GPU*)malloc(sizeof QNode_GPU);
	newnode->obj = obj;
	newnode->next = 0;
	if (!h) return newnode;
	QNode_GPU* temp = h;
	while (temp->next) temp = temp->next;
	temp->next = newnode;
	return h;
}

__device__ PQNode_GPU* PQPush(PQNode_GPU* h, Obj_GPU* obj)
{
	PQNode_GPU* newnode = (PQNode_GPU*)malloc(sizeof PQNode_GPU);
	*newnode = { obj, 0 };
	if (!h) return newnode;
	if (h->item->distance > obj->distance) {
		newnode->next = h;
		return newnode;
	}
	PQNode_GPU* pre = h;
	PQNode_GPU* cur = pre;
	while ((pre = cur) && (cur = cur->next))
		if (cur->item->distance > obj->distance) {
			pre->next = newnode;
			newnode->next = cur;
			return h;
		}
	pre->next = newnode;
	return h;
}

__device__ bool PQEmpty(PQNode_GPU* h) 
{
	if (h) return false;
	else return true;
}

__device__ Obj_GPU* PQTop(PQNode_GPU*h)
{
	return h->item;
}

__device__ PQNode_GPU* PQPop(PQNode_GPU* h) 
{
	if (!h) return 0;
	PQNode_GPU* temp;
	temp = h;
	h = h->next;
	free(temp);
	return h;
}

__device__ void PQClear(PQNode_GPU* h) 
{
	PQNode_GPU* temp = h;
	while (temp = h) {
		h = h->next;
		//free(temp);
	}
}

__device__ bool QEmpty(QNode_GPU* h) 
{
	if (h) return false;
	else return true;
}

__device__ Obj_GPU* QFrontObj(QNode_GPU* h)
{
	return h->obj;
}

__device__ QNode_GPU* QPop(QNode_GPU* h) 
{
	if (!h) return 0;
	QNode_GPU* temp;
	temp = h;
	h = h->next;
	//cudaFree(temp);
	return h;
}

//__device__ QNode_GPU* findNeighbor(Obj_GPU* element, Point_GPU source, GE_TREE_PIP_DATA* tree)
//{
//	//printf("test %d", *(tree->d_node_number));
//	//for (int i = 0; i < *(tree->d_node_number); i++) printf("%d\n",tree->d_quad_tree[i].number);
//	Node_GPU* n = NULL;
//	int temp1 = -1;
//	int temp2 = -1;
//	QNode_GPU* result = NULL;
//	Point_GPU p;
//	Obj_GPU* new_obj = 0;
//	int Ix, Iy;
//	double distance;
//	if (element->isNode) n = element->node;
//	else
//	{
//		p = element->pos;
//		Ix = floor((p.x - *(tree->d_minX)) / *(tree->d_cell_width));
//		Iy = floor((p.y - *(tree->d_minY)) / *(tree->d_cell_height));
//		n = &(tree->d_quad_tree[tree->d_Gcell[Ix * 1024 + Iy]]);
//	}
//	//printf("%d,%d,%d,%d\n",n->boundary_bot_left.IntX,n->boundary_bot_left.IntX,n->boundary_top_right.IntX,n->boundary_top_right.IntY);
//	for (int i = n->boundary_bot_left.IntY; i < n->boundary_top_right.IntY; i++) {
//		if (n->boundary_bot_left.IntX > 0) {
//			//printf("%d,%d,%d\n", tree->d_quad_tree[tree->d_Gcell[(n->boundary_bot_left.IntX - 1) * 1024 + i]].visited, (n->boundary_bot_left.IntX - 1) * 1024 + i, temp1);
//			//printf("%d,%d,%d\n", (n->boundary_bot_left.IntX - 1) * 1024 + i, (n->boundary_top_right.IntX + 1) * 1024 + i, i);
//			
//			if ((!((tree->d_quad_tree[tree->d_Gcell[(n->boundary_bot_left.IntX - 1) * 1024 + i]]).visited)) &&
//				(tree->d_Gcell[(n->boundary_bot_left.IntX - 1) * 1024 + i] != temp1)) {
//				temp1 = tree->d_Gcell[(n->boundary_bot_left.IntX - 1) * 1024 + i];
//				new_obj = (Obj_GPU*)malloc(sizeof(Obj_GPU));
//				new_obj->distance = caculateDistance(source, &(tree->d_quad_tree[temp1]));
//				//printf("a %lf,%d", new_obj->distance,i);
//				new_obj->isNode = true;
//				new_obj->node = &(tree->d_quad_tree[temp1]);
//				result = QPushObj(result, new_obj);
//
//			}
//		}
//		if (n->boundary_top_right.IntX < *(tree->d_cell_number_x) - 1) {
//			//printf("%d, %d\n", tree->d_Gcell[(n->boundary_top_right.IntX + 1) * 1024 + i], i);
//			if (!((tree->d_quad_tree[tree->d_Gcell[(n->boundary_top_right.IntX + 1) * 1024 + i]]).visited) &&
//				(tree->d_Gcell[(n->boundary_top_right.IntX + 1) * 1024 + i] != temp2)) {
//				temp2 = tree->d_Gcell[(n->boundary_top_right.IntX + 1) * 1024 + i];
//				new_obj = (Obj_GPU*)malloc(sizeof(Obj_GPU));
//				new_obj->distance = caculateDistance(source, &(tree->d_quad_tree[temp2]));
//				//printf("b %lf,%d", new_obj->distance, i);
//				new_obj->isNode = true;
//				new_obj->node = &(tree->d_quad_tree[temp2]);
//				result = QPushObj(result, new_obj);
//			}
//		}
//		/*while (!QEmpty(result)) {
//			Obj_GPU* obj = QFrontObj(result);
//			printf("%d,", obj->node->number);
//			result = QPop(result);
//		}*/
//		
//	}
//	temp1 = -1;
//	temp2 = -1;
//	//printf("zz!\n");
//	//up and down bound
//	for (int i = n->boundary_bot_left.IntX; i < n->boundary_top_right.IntX; i++) {
//		if (n->boundary_top_right.IntY < *(tree->d_cell_number_y) - 1) {
//			if (!((tree->d_quad_tree[tree->d_Gcell[i * 1024 + n->boundary_top_right.IntY +1]]).visited) &&
//				tree->d_Gcell[i * 1024 + n->boundary_top_right.IntY + 1] != temp1) {
//				temp1 = tree->d_Gcell[i * 1024 + n->boundary_top_right.IntY + 1];
//				new_obj = (Obj_GPU*)malloc(sizeof(Obj_GPU));
//				new_obj->distance = caculateDistance(source, &(tree->d_quad_tree[temp1]));
//				new_obj->isNode = true;
//				new_obj->node = &(tree->d_quad_tree[temp1]);
//				result = QPushObj(result, new_obj);
//			}
//		}
//		if (n->boundary_bot_left.IntY > 0) {
//			if (!((tree->d_quad_tree[tree->d_Gcell[i * 1024 + (n->boundary_bot_left.IntY - 1)]]).visited) &&
//				tree->d_Gcell[i * 1024 + (n->boundary_bot_left.IntY - 1)] != temp2) {
//				temp2 = tree->d_Gcell[i * 1024 + (n->boundary_bot_left.IntY - 1)];
//				new_obj = (Obj_GPU*)malloc(sizeof(Obj_GPU));
//				new_obj->distance = caculateDistance(source, &(tree->d_quad_tree[temp2]));
//				new_obj->isNode = true;
//				new_obj->node = &(tree->d_quad_tree[temp2]);
//				result = QPushObj(result, new_obj);
//			}
//		}
//	}
//	temp1 = -1;
//	temp2 = -1;
	//printf("a!\n");
	//corners
	//printf("%d,%d\n", n->boundary_bot_left.IntX, n->boundary_top_right.IntY);
	//if (n->boundary_bot_left.IntX > 0 && n->boundary_bot_left.IntY > 0) {
	//	temp1 = (n->boundary_bot_left.IntX - 1) * 1024 + n->boundary_bot_left.IntY - 1;
	//	if (!((tree->d_quad_tree[tree->d_Gcell[temp1]]).visited))
	//	{
	//		new_obj = (Obj_GPU*)malloc(sizeof(Obj_GPU));
	//		new_obj->distance = caculateDistance(source, &(tree->d_quad_tree[tree->d_Gcell[temp1]]));
	//		printf("%lf\n", new_obj->distance);

	//		new_obj->isNode = true;
	//		new_obj->node = &(tree->d_quad_tree[tree->d_Gcell[temp1]]);
	//		result = QPushObj(result, new_obj);
	//	}
	//}
	////printf("a!\n");
	////printf("%d,%d,%d", n->boundary_bot_left.IntX, n->boundary_top_right.IntY, *(tree->d_cell_number_y) - 1);
	//if (n->boundary_bot_left.IntX > 0 && n->boundary_top_right.IntY < (*(tree->d_cell_number_y) - 1)) {
	//	temp1 = (n->boundary_bot_left.IntX - 1) * 1024 + n->boundary_top_right.IntY + 1;
	//	if (!((tree->d_quad_tree[tree->d_Gcell[temp1]]).visited))
	//	{
	//		
	//		new_obj = (Obj_GPU*)malloc(sizeof(Obj_GPU));
	//		new_obj->distance = caculateDistance(source, &(tree->d_quad_tree[tree->d_Gcell[temp1]]));
	//		new_obj->isNode = true;
	//		new_obj->node = &(tree->d_quad_tree[tree->d_Gcell[temp1]]);
	//		result = QPushObj(result, new_obj);
	//	}
	//}
	//printf("c!\n");
	//if (n->boundary_top_right.IntX < *(tree->d_cell_number_x) - 1 && n->boundary_top_right.IntY < *(tree->d_cell_number_y) - 1) {
	//	temp1 = (n->boundary_top_right.IntX + 1) * 1024 + n->boundary_bot_left.IntY + 1;
	//	if (!((tree->d_quad_tree[tree->d_Gcell[temp1]]).visited)) {
	//		new_obj = (Obj_GPU*)malloc(sizeof(Obj_GPU));
	//		new_obj->distance = caculateDistance(source, &(tree->d_quad_tree[tree->d_Gcell[temp1]]));
	//		new_obj->isNode = true;
	//		new_obj->node = &(tree->d_quad_tree[tree->d_Gcell[temp1]]);
	//		result = QPushObj(result, new_obj);
	//	}
	//}
	////printf("d!\n");
	//if (n->boundary_top_right.IntX < *(tree->d_cell_number_x) - 1 && n->boundary_bot_left.IntY > 0) {
	//	temp1 = (n->boundary_top_right.IntX + 1) * 1024 + n->boundary_bot_left.IntY - 1;
	//	if (!((tree->d_quad_tree[tree->d_Gcell[temp1]]).visited)) {
	//		new_obj = (Obj_GPU*)malloc(sizeof(Obj_GPU));
	//		new_obj->distance = caculateDistance(source, &(tree->d_quad_tree[tree->d_Gcell[temp1]]));
	//		new_obj->isNode = true;
	//		new_obj->node = &(tree->d_quad_tree[tree->d_Gcell[temp1]]);
	//		result = QPushObj(result, new_obj);
	//	}
	//}
//	return result;
//}

//__device__ Point_GPU kNN(Point_GPU source, GE_TREE_PIP_DATA* tree) {
//	printf("KNN!\n");
//	Point_GPU result;
//	Node_GPU* leafnode = findNode(source, tree);
//	PQNode_GPU* obj_set = NULL;
//	for (int i = 0; i < (*tree->d_node_number); i++)  (tree->d_quad_tree[i]).visited = false; 
//	//initializeVisited(tree->root);
//	Obj_GPU* leaf_obj = (Obj_GPU*)malloc(sizeof(Obj_GPU));
//	leaf_obj->distance = 0;
//	leaf_obj->node = leafnode;
//	leaf_obj->isNode = true;
//	obj_set = PQPush(obj_set, leaf_obj);
//	while (!PQEmpty(obj_set)) {
//		Obj_GPU* element = PQTop(obj_set);
//		obj_set = PQPop(obj_set);
//		if (!element->isNode) {
//			result = element->pos;
//			PQClear(obj_set);
//			//printf("result %d,%d\n",result.IntX,result.IntY);
//			return result;
//		}
//		else
//		{
//			//printf("%d\n",element->node->number);
//			if (!element->node->visited) {
//				QNode_GPU* neighbor = findNeighbor(element, source, tree);
//				element->node->visited = true;
//				//printf("a %d\n",element->node->number);
//				for (int i = 0; i < (element->node->number); i++) {
//					
//					Obj_GPU* obj = (Obj_GPU*)malloc(sizeof(Obj_GPU));
//					//printf("b %d\n", i);
//					obj->pos.IntX = tree->d_point_set[element->node->obj_array[i]].IntX;
//					obj->pos.IntY = tree->d_point_set[element->node->obj_array[i]].IntY;
//					obj->pos.x = tree->d_point_set[element->node->obj_array[i]].x;
//					obj->pos.y = tree->d_point_set[element->node->obj_array[i]].y;
//					obj->pos.isVertex = tree->d_point_set[element->node->obj_array[i]].isVertex;
//					obj->pos.edgeIdx = tree->d_point_set[element->node->obj_array[i]].edgeIdx;
//
//					obj->isNode = false;
//					obj->distance = PointDistance_GPU(source, obj->pos);
//					//printf("%lf,%d\n", obj->distance,i);
//					obj_set = PQPush(obj_set, obj);
//				}
//				//printf("aa\n");
//				while (!PQEmpty(obj_set) && element == PQTop(obj_set)) obj_set = PQPop(obj_set);
//				//free(element);
//				while (!QEmpty(neighbor)) {
//					//neighbor.front()->node->visited = true;
//					Obj_GPU* temp = QFrontObj(neighbor);
//					//printf("%d,", temp->node->number);
//					obj_set = PQPush(obj_set, QFrontObj(neighbor));
//					neighbor = QPop(neighbor);
//				}
//				
//			}
//		}
//	}
//	return result;
//}
//
__device__ void _findNeighbor(int* result, int * size,  Node_GPU* n, GE_TREE_PIP_DATA* tree)
{
	int temp1 = -1;
	int temp2 = -1;
	for (int i = n->boundary_bot_left.IntY; i < n->boundary_top_right.IntY; i++) {
		if (n->boundary_bot_left.IntX > 0) {
			if ((!((tree->d_quad_tree[tree->d_Gcell[(n->boundary_bot_left.IntX - 1) * 1024 + i]]).visited)) &&
				(tree->d_Gcell[(n->boundary_bot_left.IntX - 1) * 1024 + i] != temp1)) {
				temp1 = tree->d_Gcell[(n->boundary_bot_left.IntX - 1) * 1024 + i];
				result[*size] = temp1;
				(*size)++;
			}
		}
		if (n->boundary_top_right.IntX < *(tree->d_cell_number_x) - 1) {
			if (!((tree->d_quad_tree[tree->d_Gcell[(n->boundary_top_right.IntX + 1) * 1024 + i]]).visited) &&
				(tree->d_Gcell[(n->boundary_top_right.IntX + 1) * 1024 + i] != temp2)) {
				temp2 = tree->d_Gcell[(n->boundary_top_right.IntX + 1) * 1024 + i];
				result[*size] = temp2;
				(*size)++;
			}
		}
	}
	temp1 = -1;
	temp2 = -1;
	//up and down bound
	for (int i = n->boundary_bot_left.IntX; i < n->boundary_top_right.IntX; i++) {
		if (n->boundary_top_right.IntY < *(tree->d_cell_number_y) - 1) {
			if (!((tree->d_quad_tree[tree->d_Gcell[i * 1024 + n->boundary_top_right.IntY + 1]]).visited) &&
				tree->d_Gcell[i * 1024 + n->boundary_top_right.IntY + 1] != temp1) {
				temp1 = tree->d_Gcell[i * 1024 + n->boundary_top_right.IntY + 1];
				result[*size] = temp1;
				(*size)++;
			}
		}
		if (n->boundary_bot_left.IntY > 0) {
			if (!((tree->d_quad_tree[tree->d_Gcell[i * 1024 + (n->boundary_bot_left.IntY - 1)]]).visited) &&
				tree->d_Gcell[i * 1024 + (n->boundary_bot_left.IntY - 1)] != temp2) {
				temp2 = tree->d_Gcell[i * 1024 + (n->boundary_bot_left.IntY - 1)];
				result[*size] = temp2;
				(*size)++;
			}
		}
	}
	temp1 = -1;
	temp2 = -1;
	//corners
	//printf("%d,%d\n", n->boundary_bot_left.IntX, n->boundary_top_right.IntY);
	if (n->boundary_bot_left.IntX > 0 && n->boundary_bot_left.IntY > 0) {
		temp1 = tree->d_Gcell[(n->boundary_bot_left.IntX - 1) * 1024 + n->boundary_bot_left.IntY - 1];
		if (!((tree->d_quad_tree[temp1]).visited))
		{
			result[*size] = temp1;
			(*size)++;
		}
	}
	if (n->boundary_bot_left.IntX > 0 && n->boundary_top_right.IntY < (*(tree->d_cell_number_y) - 1)) {
		temp1 = tree->d_Gcell[(n->boundary_bot_left.IntX - 1) * 1024 + n->boundary_top_right.IntY + 1];
		if (!((tree->d_quad_tree[temp1]).visited))
		{
			
			result[*size] = temp1;
			(*size)++;
		}
	}
	if (n->boundary_top_right.IntX < *(tree->d_cell_number_x) - 1 && n->boundary_top_right.IntY < *(tree->d_cell_number_y) - 1) {
		temp1 = tree->d_Gcell[(n->boundary_top_right.IntX + 1) * 1024 + n->boundary_bot_left.IntY + 1];
		if (!((tree->d_quad_tree[temp1]).visited)) {
			result[*size] = temp1;
			(*size)++;
		}
	}
	if (n->boundary_top_right.IntX < *(tree->d_cell_number_x) - 1 && n->boundary_bot_left.IntY > 0) {
		temp1 = tree->d_Gcell[(n->boundary_top_right.IntX + 1) * 1024 + n->boundary_bot_left.IntY - 1];
		if (!((tree->d_quad_tree[temp1]).visited)) {
			result[*size] = temp1;
			(*size)++;
		}
	}
	return;
}

__device__ int _findMostClosedPoint(int* points, Point_GPU source, int size, GE_TREE_PIP_DATA* tree) {
	int min = LONG_MAX;
	double distance;
	int index;
	for (int i = 0; i < size; i++) {
		distance = sqrt((source.x - tree->d_point_set[points[i]].x) * (source.x - tree->d_point_set[points[i]].x) + (source.y - tree->d_point_set[points[i]].y) * (source.y - tree->d_point_set[points[i]].y));
		if (min > distance) {
			index = points[i];
			min = distance;
		}
	}
	return index;
}

__device__  Point_GPU _kNN(Point_GPU source, GE_TREE_PIP_DATA* tree) {
	Node_GPU* leafnode = findNode(source, tree);
	//printf("KNN!");
	int points[10000];
	int neighbor_size = 0;
	int neighbor[1000]; 
	int points_size = 0;
	// initialize visited
	for (int i = 0; i < *(tree->d_node_number); i++)  (tree->d_quad_tree[i]).visited = false;
	//process leafnode
	if (leafnode->number != 0) points[0] = _findMostClosedPoint(leafnode->obj_array, source, leafnode->number, tree);
	points_size++;
	//printf("KNN!");

	leafnode->visited = true;

	//process 1st neighbor
	//printf("!!!");
	_findNeighbor(neighbor, &neighbor_size, leafnode, tree);
	for (int i = 0; i < neighbor_size; i++) {
		points[points_size] =  _findMostClosedPoint(tree->d_quad_tree[neighbor[i]].obj_array, source, tree->d_quad_tree[neighbor[i]].number, tree);
		points_size ++;
		tree->d_quad_tree[neighbor[i]].visited = true;
	}
	//return min distance
	if (points_size != 0) {
		double min = LONG_MAX;
		int index;
		for (int i = 0; i < points_size; i++) {
			double distance = sqrtf((source.x - tree->d_point_set[points[i]].x) *(source.x - tree->d_point_set[points[i]].x) +
				(source.y - tree->d_point_set[points[i]].y) *(source.y - tree->d_point_set[points[i]].y));
			if (distance < min) {
				min = distance;
				index = points[i];
			}
		}
		//printf("%d, %d\n", tree->d_point_set[index].isVertex, tree->d_point_set[index].edgeIdx);
		return tree->d_point_set[index];
	}
	//process 2nd neighbor
	else {
		for (int i = 0; i < neighbor_size; i++) {
			neighbor_size = 0;
			int new_neighbor[1000];
			 _findNeighbor(new_neighbor, &neighbor_size, &(tree->d_quad_tree[neighbor[i]]), tree);
			for (int k = 0; k < neighbor_size; i++) {
				points[points_size] = _findMostClosedPoint(tree->d_quad_tree[neighbor[k]].obj_array, source, tree->d_quad_tree[neighbor[k]].number, tree);
				points_size ++;
				tree->d_quad_tree[neighbor[k]].visited = true;
			}
		}
		double min = LONG_MAX;
		int index;
		for (int i = 0; i < points_size; i++) {
			double distance = sqrtf((source.x - tree->d_point_set[points[i]].x) *(source.x - tree->d_point_set[points[i]].x) +
				(source.y - tree->d_point_set[points[i]].y) *(source.y - tree->d_point_set[points[i]].y));
			if (distance < min) {
				min = distance;
				index = points[i];
			}
		}
		//printf("%d, %d\n", tree->d_point_set[index].isVertex, tree->d_point_set[index].edgeIdx);
		return tree->d_point_set[index];
	}
}

__device__ double calculateDis_GPU(Point_GPU p, Coeffecient_GPU l)
{
	return abs(l.a*p.x + l.b*p.y + l.c) / sqrt(l.a*l.a + l.b*l.b + l.c*l.c);
}

__device__ int findAdjacentVertex_GPU(Point_GPU result, GE_TREE_PIP_DATA *d_pip)
{
	//当knn找到的散点为多边形顶点时，需要寻找临近边，但是测试多边形含有孔、洞，不能通过加减1来获取临近边
	//因此需要在所有边中进行寻找，笨办法
	int adjacentEdgeIdx = 0;
	int edgeCount = *d_pip->d_edge_count;
	//printf("%d\n", edgeCount);
	for (int i = 0; i < edgeCount; i++)
	{
		int startIdx =d_pip->d_edgeTable[i].startIndex;
		int endIdx = d_pip->d_edgeTable[i].endIndex;
		Point_GPU p1, p2;
		p1 = d_pip->d_vertexTable[startIdx];
		p2 = d_pip->d_vertexTable[endIdx];
		//printf("%d\n", result.edgeIdx);
		if (i != result.edgeIdx)
		{
			if ((p1.x == result.x && p1.y == result.y) || (p2.x == result.x && p2.y == result.y))
			{
				adjacentEdgeIdx = i;
				return adjacentEdgeIdx;
			}
		}
	}
	return adjacentEdgeIdx;
}

__device__ Coeffecient_GPU getCoeffecient_GPU(int inx, GE_TREE_PIP_DATA *d_pip)
{
	int startIdx = d_pip->d_edgeTable[inx].startIndex;
	int endIdx = d_pip->d_edgeTable[inx].endIndex;
	Point_GPU p1, p2;
	p1 = d_pip->d_vertexTable[startIdx];
	p2 = d_pip->d_vertexTable[endIdx];
	Coeffecient_GPU Coeff;
	Coeff.a = p2.y - p1.y;
	Coeff.b = p1.x - p2.x;
	Coeff.c = p1.y - (p2.y - p1.y)*p1.x;
	return Coeff;
}

__device__ Edge2D getClosestEdge_GPU(Point_GPU source, Point_GPU result, GE_TREE_PIP_DATA *d_pip)
{
	//printf("Result Point:(%f,%f)\n", result.x, result.y);
	//printf("Source Point:(%f,%f)\n", source.x, source.y);
	double dis[2];
	int closest_edge_index = result.edgeIdx;
	//printf("closest_edge_index:%d\n", closest_edge_index);
	//printf("isVertex:%d\n", result.isVertex);
	//printf("edge count :%d\n", *d_pip->d_edge_count);
	if (result.isVertex)
	{
		// find adjacent edge
		int front_edge_Idx, back_edge_Idx;
		if (result.edgeIdx == 0)
		{
			front_edge_Idx = (*d_pip->d_edge_count) - 1;
			back_edge_Idx = result.edgeIdx + 1;
		}
		else if (result.edgeIdx == (*d_pip->d_edge_count) - 1)
		{
			front_edge_Idx = result.edgeIdx - 1;
			back_edge_Idx = 0;
		}
		else
		{
			front_edge_Idx = result.edgeIdx - 1;
			back_edge_Idx = result.edgeIdx + 1;
		}
		//printf("front_edge_Idx:%d,back_edge_Idx: %d\n", front_edge_Idx, back_edge_Idx);

		Point_GPU front_point, back_point;

		//for front edge
		int endIdx = d_pip->d_edgeTable[front_edge_Idx].endIndex;
		front_point = d_pip->d_vertexTable[endIdx];

		//for back edge
		int startIdx = d_pip->d_edgeTable[back_edge_Idx].startIndex;
		back_point = d_pip->d_vertexTable[startIdx];
		//printf("front_point:(%f,%f),back_point:(%f,%f)\n", front_point.x, front_point.y, back_point.x, back_point.y);
		dis[0] = calculateDis_GPU(source, getCoeffecient_GPU(result.edgeIdx, d_pip));
		if (front_point.x == result.x&&front_point.y == result.y)
		{
			dis[1] = calculateDis_GPU(source, getCoeffecient_GPU(front_edge_Idx, d_pip));
			(dis[0] < dis[1]) ? closest_edge_index = result.edgeIdx : closest_edge_index = front_edge_Idx;
		}
		else if (back_point.x == result.x&&back_point.y == result.y)
		{
			dis[1] = calculateDis_GPU(source, getCoeffecient_GPU(back_edge_Idx, d_pip));
			(dis[0] < dis[1]) ? closest_edge_index = result.edgeIdx : closest_edge_index = back_edge_Idx;
		}
		else
		{
			int adjacentEdgeIdx = findAdjacentVertex_GPU(result, d_pip);
			//printf("adjacentEdgeIdx:%d\n", adjacentEdgeIdx);
			dis[1] = calculateDis_GPU(source, getCoeffecient_GPU(adjacentEdgeIdx,d_pip));
			(dis[0] < dis[1]) ? closest_edge_index = result.edgeIdx : closest_edge_index = adjacentEdgeIdx;
			//cout << "An error occurred while looking for adjacent edges" << endl;
		}
		//printf("dis[0]: %f,dis[1]:%f, dis[2]:%f\n", dis[0], dis[1], dis[2]);
	}
	//printf("%d, %d\n", d_pip->d_edgeTable[closest_edge_index].startIndex, d_pip->d_edgeTable[closest_edge_index].endIndex);
	return d_pip->d_edgeTable[closest_edge_index];
}

__device__ bool JudgeCollineation_GPU(Point_GPU p1, Point_GPU p2, Point_GPU q)
{
	double v1, v2;
	v1 = (q.x - p1.x) * (p1.y - p2.y);
	v2 = (p1.x - p2.x) * (q.y - p1.y);
	if ((fabs(v1 - v2) < eps)
		&& (q.x >= min(p1.x, p2.x) && q.x <= max(p1.x, p2.x))
		&& (q.y >= min(p1.y, p2.y)) && (q.y <= max(p1.y, p2.y)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

__device__ int getPointAttri_GPU(Point_GPU testP, Point_GPU edgeStartP, Point_GPU edgeEndP)
{
	//printf("testP:(%f,%f), edgestartP(%f,%f), edgeendP(%f,%f)\n", testP.x, testP.y, edgeStartP.x, edgeStartP.y, edgeEndP.x, edgeEndP.y);
	//test point on the edge
	bool isCollineation = JudgeCollineation_GPU(edgeStartP, edgeEndP, testP);
	//printf("isCollineation:%d\n", isCollineation);
	if (isCollineation)
	{
		return INSIDE;
	}
	//double Tmp = (edgeStartP.y - edgeEndP.y) * testP.x + (edgeEndP.x - edgeStartP.x) * testP.y + 
	//	edgeStartP.x * edgeEndP.y - edgeEndP.x * edgeStartP.y;
	//
	Point_GPU a, b;
	a.x = edgeEndP.x - edgeStartP.x;
	a.y = edgeEndP.y - edgeStartP.y;
	b.x = testP.x - edgeStartP.x;
	b.y = testP.y - edgeStartP.y;
	double product = a.x * b.y - a.y * b.x;
	//printf("product:%f\n", product);
	if (product > 0)
	{
		return INSIDE;//right side
	}
	else if (product < 0)
	{
		return OUTSIDE;  //left side
	}
	else
	{
		//collineation 延长线 简易处理吧
		return OUTSIDE;
	}
}

__global__ void GE_Tree_PIP_Kernal(GE_TREE_PIP_DATA  *d_pip)
{
	//set thread ID
	int ix = threadIdx.x + blockIdx.x * blockDim.x;
	int iy = threadIdx.y + blockIdx.y * blockDim.y;
	unsigned int Idx = iy * blockDim.x * gridDim.x + ix;

	//printf("Idx=%d\n", Idx);
	
	if (Idx >= 1000000)
	{
		return;
	}
	//printf("point  set number:%d\n", *(d_pip->d_point_set_size));
	//printf("edge num:%d\n", *d_pip->d_edge_count);
	Point_GPU testPoint;
	testPoint = d_pip->d_testpoint[Idx];
	//printf("Boundary:(%f,%f),(%f,%f),testPoint:(%f,%f)\n", d_pip->d_botLeft->x, 
	//	d_pip->d_botLeft->y, d_pip->d_topRight->x, d_pip->d_topRight->y, testPoint.x, testPoint.y);

	//printf("Out rectangular box? %d,%d,%d,%d\n", testPoint.x < d_pip->d_botLeft->x, testPoint.x>d_pip->d_topRight->x,
	//	testPoint.y < d_pip->d_botLeft->y, testPoint.y>d_pip->d_topRight->y);

	if ((testPoint.x<d_pip->d_botLeft->x) || (testPoint.x>d_pip->d_topRight->x)
		|| (testPoint.y<d_pip->d_botLeft->y) || (testPoint.y>d_pip->d_topRight->y))
	{
		//printf("%d,%d,%d,%d\n", testPoint.x < d_pip->d_botLeft->x, testPoint.y>d_pip->d_topRight->x,
		//	testPoint.y < d_pip->d_botLeft->y, testPoint.y>d_pip->d_topRight->y);
		d_pip->d_testedresult[Idx] = OUTSIDE;
		return;
	}
	//printf("1111Out rectangular box!\n");
	Point_GPU result = _kNN(testPoint, d_pip);  //输入的参数
	d_pip->d_n_point[Idx] = result;   //used for knn test
	//printf("Result Point:(%f,%f)\n", result.x, result.y);
	Edge2D edge;
	edge = getClosestEdge_GPU(testPoint, result, d_pip);
	//printf("nearest edge: startIdx=%d, endIdx=%d\n", edge.startIndex, edge.endIndex);
	Point_GPU edgeStartP, edgeEndP, testP;
	edgeStartP = d_pip->d_vertexTable[edge.startIndex];
	edgeEndP = d_pip->d_vertexTable[edge.endIndex];
   /* printf("Index:%d, %d, start vertex:%f,%f,end vertex:%f,%f\n", edge.startIndex, edge.endIndex, d_pip->d_vertexTable[edge.startIndex].x, 
		d_pip->d_vertexTable[edge.startIndex].y,	d_pip->d_vertexTable[edge.endIndex].x, d_pip->d_vertexTable[edge.endIndex].y);*/
	d_pip->d_testedresult[Idx]= getPointAttri_GPU(testPoint, edgeStartP, edgeEndP);
	return;
}

void InitGE_Tree_PIP(GE_TREE_PIP_DATA **ha_pip, GE_TREE_PIP_DATA **da_pip, pip &testPip, GQT &test, unsigned int testSize)
{
	//cllocate stucture memory
	cudaFree(0);
	cudaMallocHost((void **)ha_pip, sizeof(GE_TREE_PIP_DATA));
	cudaMalloc((void **)da_pip, sizeof(GE_TREE_PIP_DATA));
	GE_TREE_PIP_DATA *h_pip = *ha_pip;

	//host CPU
	cudaMallocHost((void **)&h_pip->h_testpoint, testSize * sizeof(Point_GPU));   //测试点   结构体嵌套
	cudaMallocHost((void **)&h_pip->h_testedresult, testSize * sizeof(int));         //测试结果

	cudaMallocHost((void **)&h_pip->h_point_set, testPip.discretePoint.size() * sizeof(Point_GPU));
	cudaMallocHost((void **)&h_pip->h_point_set_size, sizeof(int));    //散点总数
	cudaMallocHost((void **)&h_pip->h_quad_tree, test.node_number * sizeof(Node_GPU));
	cudaMallocHost((void **)&h_pip->h_Gcell, 1024 * 1024 * sizeof(int));

	cudaMallocHost((void **)&h_pip->h_botLeft, sizeof(Point_GPU));
	cudaMallocHost((void **)&h_pip->h_topRight, sizeof(Point_GPU));

	cudaMallocHost((void **)&h_pip->h_grid_width, sizeof(double));
	cudaMallocHost((void **)&h_pip->h_grid_height, sizeof(double));
	cudaMallocHost((void **)&h_pip->h_minX, sizeof(double));
	cudaMallocHost((void **)&h_pip->h_minY, sizeof(double));
	cudaMallocHost((void **)&h_pip->h_cell_width, sizeof(double));
	cudaMallocHost((void **)&h_pip-> h_cell_height, sizeof(double));

	cudaMallocHost((void **)&h_pip->h_cell_number_x, sizeof(int));
	cudaMallocHost((void **)&h_pip->h_cell_number_y, sizeof(int));
	cudaMallocHost((void **)&h_pip->h_node_number, sizeof(int));

	cudaMallocHost((void **)&h_pip->h_edge_count, sizeof(int));
	cudaMallocHost((void **)&h_pip->h_vertexTable, testPip.testedPolygon->vertexCount * sizeof(Point_GPU));
	cudaMallocHost((void **)&h_pip->h_edgeTable, testPip.testedPolygon->edgeCount * sizeof(Edge2D));

	cudaMallocHost((void **)&h_pip->h_n_Point, testSize * sizeof(Point_GPU));
	//////////////////////////////////////////////////////////////////////////////////////

   //device GPU
	cudaMalloc((void **)&h_pip->d_testpoint, testSize * sizeof(Point_GPU));                   //测试点   结构体嵌套  静态申请内存
	cudaMalloc((void **)&h_pip->d_testedresult, testSize * sizeof(int));                   //测试结果

	cudaMalloc((void **)&h_pip->d_point_set, testPip.discretePoint.size() * sizeof(Point_GPU));
	cudaMalloc((void **)&h_pip->d_point_set_size, sizeof(int));
	cudaMalloc((void **)&h_pip->d_quad_tree, test.node_number * sizeof(Node_GPU));
	cudaMalloc((void **)&h_pip->d_Gcell, 1024 * 1024 * sizeof(int));

	cudaMalloc((void **)&h_pip->d_botLeft, sizeof(Point_GPU));
	cudaMalloc((void **)&h_pip->d_topRight, sizeof(Point_GPU));

	cudaMalloc((void **)&h_pip->d_grid_width, sizeof(double));
	cudaMalloc((void **)&h_pip->d_grid_height, sizeof(double));
	cudaMalloc((void **)&h_pip->d_minX, sizeof(double));
	cudaMalloc((void **)&h_pip->d_minY, sizeof(double));
	cudaMalloc((void **)&h_pip->d_cell_width, sizeof(double));
	cudaMalloc((void **)&h_pip->d_cell_height, sizeof(double));

	cudaMalloc((void **)&h_pip->d_cell_number_x, sizeof(int));
	cudaMalloc((void **)&h_pip->d_cell_number_y, sizeof(int));
	cudaMalloc((void **)&h_pip->d_node_number, sizeof(int));

	cudaMalloc((void **)&h_pip->d_edge_count, sizeof(int));
	cudaMalloc((void **)&h_pip->d_vertexTable, testPip.testedPolygon->vertexCount * sizeof(Point_GPU));
	cudaMalloc((void**)&h_pip->d_edgeTable, testPip.testedPolygon->edgeCount * sizeof(Edge2D));

	cudaMalloc((void **)&h_pip->d_n_point, testSize * sizeof(Point_GPU));

	//exchange data
	cudaMemcpy(*da_pip, *ha_pip, sizeof(GE_TREE_PIP_DATA), cudaMemcpyHostToDevice);
}

void CopyPipValuetoHost(GE_TREE_PIP_DATA *ha_pip, pip &testPip, GQT &test, unsigned int testSize)
{
	//测试点数据复制
	for (int i = 0; i < testSize; i++)
	{
		ha_pip->h_testpoint[i].x = testPip.testedPoint[i].x;
		ha_pip->h_testpoint[i].y = testPip.testedPoint[i].y;
	}
	//复制离散点
	for (int  i = 0; i < testPip.discretePoint.size(); i++)
	{
		ha_pip->h_point_set[i].x = testPip.discretePoint[i].x;
		ha_pip->h_point_set[i].y = testPip.discretePoint[i].y;
	}

	ha_pip->h_point_set_size = &(testPip.discretePoint_size);   //离散点总数

	//复制边界框坐标
	ha_pip->h_botLeft->x = testPip.grid_boundingbox[0].x;
	ha_pip->h_botLeft->y = testPip.grid_boundingbox[0].y;
	ha_pip->h_topRight->x = testPip.grid_boundingbox[1].x;
	ha_pip->h_topRight->y = testPip.grid_boundingbox[1].y;
	//
	ha_pip->h_grid_width = &(test.grid_width);
	ha_pip->h_grid_height = &(test.grid_height);
	ha_pip->h_minX = &(test.minX);
	ha_pip->h_minY = &(test.minY);
	ha_pip->h_cell_width = &(test.cell_width);
	ha_pip->h_cell_height = &(test.cell_height);

	ha_pip->h_cell_number_x = &(test.cell_number_x);
	ha_pip->h_cell_number_y = &(test.cell_number_y);
	ha_pip->h_node_number = &(test.node_number);

	ha_pip->h_edge_count = &(testPip.testedPolygon->edgeCount);

	for (int i = 0; i < testPip.testedPolygon->vertexCount; i++)
	{
		ha_pip->h_vertexTable[i].x = testPip.testedPolygon->vertexTable[i].x;
		ha_pip->h_vertexTable[i].y = testPip.testedPolygon->vertexTable[i].y;
	}

	for (int i = 0; i < testPip.testedPolygon->edgeCount; i++)
	{
		ha_pip->h_edgeTable[i].startIndex = testPip.testedPolygon->edgeTable[i].startIndex;
		ha_pip->h_edgeTable[i].endIndex = testPip.testedPolygon->edgeTable[i].endIndex;
	}

	//复制Node和cell
	int node_cnt = 0;
	int point_cnt = 0;
	//initialize visited
	for (int i = 0; i < 1024; i++)
		for (int j = 0; j < 1024; j++)
			test.grid->cells[i][j]->visited = false;
	// cell  data
	for (int i = 0; i < 1024; i++)
	{
		for (int j = 0; j < 1024; j++)
		{
			Node* n = test.grid->cells[i][j];
			if (!test.grid->cells[i][j]->visited) {
				ha_pip->h_quad_tree[node_cnt].distance = n->distance;
				ha_pip->h_quad_tree[node_cnt].is_leaf = n->is_leaf;
				ha_pip->h_quad_tree[node_cnt].visited = n->visited;

				ha_pip->h_quad_tree[node_cnt].center.edgeIdx = n->center.edgeIdx;
				ha_pip->h_quad_tree[node_cnt].center.IntX = n->center.IntX;
				ha_pip->h_quad_tree[node_cnt].center.IntY = n->center.IntY;
				ha_pip->h_quad_tree[node_cnt].center.isVertex = n->center.isVertex;
				ha_pip->h_quad_tree[node_cnt].center.x = n->center.x;
				ha_pip->h_quad_tree[node_cnt].center.y = n->center.y;

				ha_pip->h_quad_tree[node_cnt].boundary_bot_left.edgeIdx = n->boundary_bot_left.edgeIdx;
				ha_pip->h_quad_tree[node_cnt].boundary_bot_left.IntX = n->boundary_bot_left.IntX;
				ha_pip->h_quad_tree[node_cnt].boundary_bot_left.IntY = n->boundary_bot_left.IntY;
				ha_pip->h_quad_tree[node_cnt].boundary_bot_left.isVertex = n->boundary_bot_left.isVertex;
				ha_pip->h_quad_tree[node_cnt].boundary_bot_left.x = n->boundary_bot_left.x;
				ha_pip->h_quad_tree[node_cnt].boundary_bot_left.y = n->boundary_bot_left.y;

				ha_pip->h_quad_tree[node_cnt].boundary_top_right.edgeIdx = n->boundary_top_right.edgeIdx;
				ha_pip->h_quad_tree[node_cnt].boundary_top_right.IntX = n->boundary_top_right.IntX;
				ha_pip->h_quad_tree[node_cnt].boundary_top_right.IntY = n->boundary_top_right.IntY;
				ha_pip->h_quad_tree[node_cnt].boundary_top_right.isVertex = n->boundary_top_right.isVertex;
				ha_pip->h_quad_tree[node_cnt].boundary_top_right.x = n->boundary_top_right.x;
				ha_pip->h_quad_tree[node_cnt].boundary_top_right.y = n->boundary_top_right.y;

				int k = 0;
				while (!n->obj_array.empty()) {
					ha_pip->h_point_set[point_cnt].x = n->obj_array.front().x;
					ha_pip->h_point_set[point_cnt].y = n->obj_array.front().y;
					ha_pip->h_point_set[point_cnt].IntX = n->obj_array.front().IntX;
					ha_pip->h_point_set[point_cnt].IntY = n->obj_array.front().IntY;
					ha_pip->h_point_set[point_cnt].isVertex = n->obj_array.front().isVertex;
					ha_pip->h_point_set[point_cnt].edgeIdx = n->obj_array.front().edgeIdx;

					n->obj_array.pop();
					ha_pip->h_quad_tree[node_cnt].obj_array[k] = point_cnt;
					k++;
					point_cnt++;
				}
				ha_pip->h_quad_tree[node_cnt].number = k;
				node_cnt++;
				test.grid->cells[i][j]->visited = true;
				*(ha_pip->h_Gcell + 1024*i+j) = node_cnt - 1;
			}
			else*(ha_pip->h_Gcell + 1024 * i + j) = node_cnt - 1;
		}
	}
}

double Pip_With_Cuda(GE_TREE_PIP_DATA *h_pip, GE_TREE_PIP_DATA *d_pip, pip &testPip, GQT &test, unsigned int testSize)
{
	cudaSetDevice(0);
	//copy the host value to the device
	//复制测试点
	cudaMemcpy(h_pip->d_testpoint, h_pip->h_testpoint, testSize * sizeof(Point_GPU), cudaMemcpyHostToDevice);
	//复制离散点
	cudaMemcpy(h_pip->d_point_set, h_pip->h_point_set, testPip.discretePoint.size() * sizeof(Point_GPU), cudaMemcpyHostToDevice);
	//复制离散点总数
	cudaMemcpy(h_pip->d_point_set_size, h_pip->h_point_set_size, sizeof(int), cudaMemcpyHostToDevice);

	cudaMemcpy(h_pip->d_quad_tree, h_pip->h_quad_tree, test.node_number * sizeof(Node_GPU), cudaMemcpyHostToDevice);
	cudaMemcpy(h_pip->d_Gcell, h_pip->h_Gcell, 1024 * 1024 * sizeof(int), cudaMemcpyHostToDevice);

	cudaMemcpy(h_pip->d_botLeft, h_pip->h_botLeft, sizeof(Point_GPU), cudaMemcpyHostToDevice);
	cudaMemcpy(h_pip->d_topRight, h_pip->h_topRight, sizeof(Point_GPU), cudaMemcpyHostToDevice);

	cudaMemcpy(h_pip->d_grid_width, h_pip->h_grid_width, sizeof(double), cudaMemcpyHostToDevice);
	cudaMemcpy(h_pip->d_grid_height, h_pip->h_grid_height, sizeof(double), cudaMemcpyHostToDevice);
	cudaMemcpy(h_pip->d_minX, h_pip->h_minX, sizeof(double), cudaMemcpyHostToDevice);
	cudaMemcpy(h_pip->d_minY, h_pip->h_minY, sizeof(double), cudaMemcpyHostToDevice);
	cudaMemcpy(h_pip->d_cell_width, h_pip->h_cell_width, sizeof(double), cudaMemcpyHostToDevice);
	cudaMemcpy(h_pip->d_cell_height, h_pip->h_cell_height, sizeof(double), cudaMemcpyHostToDevice);

	cudaMemcpy(h_pip->d_cell_number_x, h_pip->h_cell_number_x, sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(h_pip->d_cell_number_y, h_pip->h_cell_number_y, sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(h_pip->d_node_number, h_pip->h_node_number, sizeof(int), cudaMemcpyHostToDevice);

	cudaMemcpy(h_pip->d_edge_count, h_pip->h_edge_count, sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(h_pip->d_vertexTable, h_pip->h_vertexTable, testPip.testedPolygon->vertexCount * sizeof(Point_GPU), cudaMemcpyHostToDevice);
	cudaMemcpy(h_pip->d_edgeTable, h_pip->h_edgeTable, testPip.testedPolygon->edgeCount * sizeof(Edge2D), cudaMemcpyHostToDevice);

	cudaDeviceSynchronize();

	dim3 grid(100, 20, 1), block(16, 32, 1);
	SmallTimer timers;

	timers.start();
	GE_Tree_PIP_Kernal << <grid, block >> > (d_pip);
	cudaDeviceSynchronize();
	timers.end();
	printf("Only GE-Tree-PIP GPU calculate time %f\n", timers.time);

	cudaMemcpy(h_pip->h_n_Point, h_pip->d_n_point, testSize * sizeof(Point_GPU), cudaMemcpyDeviceToHost);
	//for (int i = 0; i < testSize; i++)
	//{
	//	printf("n_point[%d]: %f,%f\n", i, h_pip->h_n_Point[i].x, h_pip->h_n_Point[i].y);
	//}

	cudaMemcpy(h_pip->h_testedresult, h_pip->d_testedresult, testSize * sizeof(int), cudaMemcpyDeviceToHost);
	cudaDeviceSynchronize();
	export_GPU_Testresult("GPU_result.txt", h_pip);
	return timers.time;
}

void DeinitGE_Tree_PIP(GE_TREE_PIP_DATA* h_pip, GE_TREE_PIP_DATA *d_pip)
{
	//free host memory
	cudaFreeHost(h_pip->h_testpoint);
	cudaFreeHost(h_pip->h_testedresult);

	cudaFreeHost(h_pip->h_point_set);
	cudaFreeHost(h_pip->h_point_set_size);
	cudaFreeHost(h_pip->h_quad_tree);
	cudaFreeHost(h_pip->h_Gcell);

	cudaFreeHost(h_pip->h_botLeft);
	cudaFreeHost(h_pip->h_topRight);

	cudaFreeHost(h_pip->h_grid_width);
	cudaFreeHost(h_pip->h_grid_height);
	cudaFreeHost(h_pip->h_minX);
	cudaFreeHost(h_pip->h_minY);
	cudaFreeHost(h_pip->h_cell_width);
	cudaFreeHost(h_pip->h_cell_height);

	cudaFreeHost(h_pip->h_cell_number_x);
	cudaFreeHost(h_pip->h_cell_number_y);
	cudaFreeHost(h_pip->h_node_number);

	cudaFreeHost(h_pip->h_edge_count);
	cudaFreeHost(h_pip->h_vertexTable);
	cudaFreeHost(h_pip->h_edgeTable);
	cudaFreeHost(h_pip->h_n_Point);

	// free device memory
	cudaFree(h_pip->d_testpoint);
	cudaFree(h_pip->d_testedresult);

	cudaFree(h_pip->d_point_set);
	cudaFree(h_pip->d_point_set_size);
	cudaFree(h_pip->d_quad_tree);
	cudaFree(h_pip->d_Gcell);

	cudaFree(h_pip->d_botLeft);
	cudaFree(h_pip->d_topRight);

	cudaFree(h_pip->d_grid_width);
	cudaFree(h_pip->d_grid_height);
	cudaFree(h_pip->d_minX);
	cudaFree(h_pip->d_minY);
	cudaFree(h_pip->d_cell_width);
	cudaFree(h_pip->d_cell_height);

	cudaFree(h_pip->d_cell_number_x);
	cudaFree(h_pip->d_cell_number_y);
	cudaFree(h_pip->d_node_number);

	cudaFree(h_pip->d_edge_count);
	cudaFree(h_pip->d_vertexTable);
	cudaFree(h_pip->d_edgeTable);
	cudaFree(h_pip->d_n_point);
	//release structure memory
	cudaFreeHost(h_pip);
	cudaFree(d_pip);
}

void export_GPU_Testresult(const char* filename, GE_TREE_PIP_DATA *h_pip)
{
	if (filename == NULL)
		return;
	FILE* fp;
	fopen_s(&fp, filename, "w");
	for (int i = 0; i < 1000000; i++)
		fprintf(fp, "p %d %d \n", i, h_pip->h_testedresult[i]);
}