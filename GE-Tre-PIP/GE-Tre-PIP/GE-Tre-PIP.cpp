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
using namespace std;

int main()
{
	SmallTimer timer;
	bool isStatistic = false;

	//import polygon data
	pip testPip;
	testPip.readData("pol3.obj", 0);
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

	if (!isStatistic)
	{
		//test points
		testPip.readTestPoint("testPoint3.txt");
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
		printf("PIPprocess time %f\n", timer.time);
		testPip.exportTestresult("GETree_result3.txt");
	}
	else
	{
		//test points  data statistic
		testPip.readTestPoint("testPoint3.txt");
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
	//data statistic
	long long int total_add = testPip.stat_pip.cnt_add + test.stat.cnt_add;
	long long int total_compare = testPip.stat_pip.cnt_compare + test.stat.cnt_compare;
	long long int total_multiply = testPip.stat_pip.cnt_multiply + test.stat.cnt_multiply;
	long long int total_memory = test.stat.cnt_memory;
	cout << "total add:" << total_add << " total compare:" << total_compare << " total multiply:" << total_multiply <<
		" total memory:" << total_memory << endl;
	long long int memory[3];
	testPip.PIP_statStorageCost(0, 0, &memory[0], &memory[1], &memory[2]);
	return 0;
}

