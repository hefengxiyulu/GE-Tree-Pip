// GE-tree.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include "Grid.h"
#include "Node.h" 
#include "GQT.h"
#include <iostream> 
#include <vector>
#include <queue>
#include <cmath> 
#include "pip.h"
#include"GE_Tree_PIP.cuh"
using namespace std;

extern "C" void InitGE_Tree_PIP(GE_TREE_PIP_DATA **ha_pip, GE_TREE_PIP_DATA **da_pip, pip &testPip, GQT &test, unsigned int testSize);
extern "C" void CopyPipValuetoHost(GE_TREE_PIP_DATA *ha_pip, pip &testPip, GQT &test, unsigned int testSize);
extern "C" double Pip_With_Cuda(GE_TREE_PIP_DATA *h_pip, GE_TREE_PIP_DATA *d_pip, pip &testPip, GQT &test, unsigned int testSize);
extern "C" void DeinitGE_Tree_PIP(GE_TREE_PIP_DATA* h_pip, GE_TREE_PIP_DATA *d_pip);

int main()
{
	SmallTimer timer;
	bool isStatistic = false;
	//import polygon data
	pip testPip;
	testPip.readData("pol100.obj", 0);

	timer.start();
	double minEdge = testPip.findMinEdge();
	double benchmark = 1;
	if (minEdge < 3)
	{
		benchmark = 0.5;
	}
	else
	{
		benchmark = minEdge / 5.0;
	}
	testPip.edgeDiscretize(benchmark);
	testPip.initData();

	//this is a test
	//GQT test(1024, 1024, Point(-180.0, -160.0), Point(180.0, 160.0));
	Point p1, p2;
	p1.x = testPip.grid_boundingbox[0].x;
	p1.y = testPip.grid_boundingbox[0].y;
	p2.x = testPip.grid_boundingbox[1].x;
	p2.y = testPip.grid_boundingbox[1].y;
	GQT test(1024, 1024, p1, p2);

	int yy = testPip.discretePoint.size();
	Point p3;
	for (int i = 0; i < testPip.discretePoint.size(); i++)
	{
		test.insertPoint(testPip.discretePoint[i]);
	}
	timer.end();
	printf("PIP Preprocessing time %f\n", timer.time);

	// test code
	if (!isStatistic)
	{
		//test points
		testPip.readTestPoint("testPoint100.txt");
		timer.start();
		Point p4;
		for (int i = 0; i < testPip.testedPointCount; i++)
		{
			p4.x = testPip.testedPoint[i].x;
			p4.y = testPip.testedPoint[i].y;
			if (p4.x< p1.x || p4.x>p2.x || p4.y<p1.y || p4.y>p2.y)
			{
				testPip.testedResult[i] = OUTSIDE;  //Out of the coordinates of the rectangle
			}
			else
			{
				//find the nearest vertex  return only one
				vector<Point> result = test.kNN(p4, 1);
				Edge2D edge;
				for (int j = 0; j < result.size(); j++) //only one
				{
					edge = testPip.getClosestEdge(p4, result[j]);
				}
				Point2D edgeStartP, edgeEndP, testP;
				edgeStartP = testPip.testedPolygon->vertexTable[edge.startIndex];
				edgeEndP = testPip.testedPolygon->vertexTable[edge.endIndex];
				testP.x = p4.x;
				testP.y = p4.y;
				testPip.testedResult[i] = testPip.getPointAttri(testP, edgeStartP, edgeEndP);//INSIDE 1    OUTSIDE 0
				//cout << "Point num:" << i << " result:" << testPip.testedResult[i] << endl;
			}
		}
		timer.end();
		printf("PIP test time %f\n", timer.time);
		testPip.exportTestresult("GETree_result100.txt");
	}
	else
	{
		//test points  data statistic
		testPip.readTestPoint("testPoint100.txt");
		Point p4;
		for (int i = 0; i < testPip.testedPointCount; i++)
		{
			testPip.stat_pip.cnt_add++;
			testPip.stat_pip.cnt_compare++;

			p4.x = testPip.testedPoint[i].x;
			p4.y = testPip.testedPoint[i].y;
			if ((testPip.stat_pip.cnt_compare++&&p4.x< p1.x) ||(testPip.stat_pip.cnt_compare++ && p4.x>p2.x) ||
				(testPip.stat_pip.cnt_compare++ && p4.y<p1.y) || (testPip.stat_pip.cnt_compare++ && p4.y>p2.y))
			{
				testPip.testedResult[i] = OUTSIDE;  //Out of the coordinates of the rectangle
			}
			else
			{
				//find the nearest vertex  return only one
				vector<Point> result = test.kNN_stat(p4, 1);
				Edge2D edge;
				for (int j = 0; j < result.size(); j++) //only one
				{
					testPip.stat_pip.cnt_add++;
					testPip.stat_pip.cnt_compare++;

					edge = testPip.getClosestEdge_stat(p4, result[j]);
				}
				testPip.stat_pip.cnt_compare++;

				Point2D edgeStartP, edgeEndP, testP;
				edgeStartP = testPip.testedPolygon->vertexTable[edge.startIndex];
				edgeEndP = testPip.testedPolygon->vertexTable[edge.endIndex];
				testP.x = p4.x;
				testP.y = p4.y;
				testPip.testedResult[i] = testPip.getPointAttri_stat(testP, edgeStartP, edgeEndP);//INSIDE 1    OUTSIDE 0
				//cout << "Point num:" << i << " result:" << testPip.testedResult[i] << endl;
			}
		}
		testPip.stat_pip.cnt_compare++;
	}
	//////// 
	test.setNodeNumber();
	// GPU  test
	unsigned int testSize = testPip.testedPointCount;
	GE_TREE_PIP_DATA *h_GE_Tree_PIP;
	GE_TREE_PIP_DATA *d_GE_Tree_PIP;
	InitGE_Tree_PIP(&h_GE_Tree_PIP, &d_GE_Tree_PIP, testPip,test, testSize);
	CopyPipValuetoHost(h_GE_Tree_PIP, testPip, test, testSize);
	double GPU_test_time = Pip_With_Cuda(h_GE_Tree_PIP, d_GE_Tree_PIP, testPip, test, testSize);
	DeinitGE_Tree_PIP(h_GE_Tree_PIP, d_GE_Tree_PIP);



	//data statistic
	long long int total_add = testPip.stat_pip.cnt_add + test.stat.cnt_add;
	long long int total_compare = testPip.stat_pip.cnt_compare + test.stat.cnt_compare;
	long long int total_multiply = testPip.stat_pip.cnt_multiply + test.stat.cnt_multiply;
	long long int total_memory = test.stat.cnt_memory;
	cout << "total add:" << total_add << " total compare:" << total_compare << " total multiply:" << total_multiply <<
		" total memory:" << total_memory << endl;

	long long int memory[3]; //memory[0]: polygon basic_cost, memory[1]: polygon auxiliary_cost, memory[2]: GE-Tree  auxiliary_cost
	//pip memory cost
	testPip.PIP_statStorageCost(&memory[0], &memory[1]);
	//GE-Tree memory cost
	test.GEtree_statStorageCost(1024, 1024, &memory[2]);

	double real_total_auxiliary_cost = (double)(memory[1] + memory[2]) / 1024.0;
	double real_total_basic_cost = (double)(memory[0]) / 1024.0;
	cout << "real total basic memory:" << real_total_basic_cost << "KB" << endl;
	cout << "real total auxiliary memory:" << real_total_auxiliary_cost << "KB" << endl;
	return 0;
}

