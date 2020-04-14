#pragma once
#include<iostream>
#include<vector>
#include "Node.h"
#include <windows.h>
#include <tchar.h>
#include "Statistics.h"

#define FLTYPE double
#define INSIDE 1
#define OUTSIDE 0
#define eps 1e-6

using namespace std;

struct Coeffecient {
	FLTYPE a, b, c; //coefficient of the equation of edge
	Coeffecient(FLTYPE a, FLTYPE b, FLTYPE c) : a(a), b(b), c(c) {};
};

struct Point2D {
	union {
		struct { FLTYPE x, y; };
		FLTYPE coord[2];
	};
	Point2D(FLTYPE xx = 0, FLTYPE yy = 0)
	{
		x = xx; y = yy;
	}
};

struct Edge2D {
	//FLTYPE a,b,c;   //������ֱ�߷���ϵ��
	union {
		struct { int startIndex, endIndex; };     //�ߵĶ�������
		int vertexIndex[2];
	};
};

struct GCPPolygon2D {

	struct Point2D* vertexTable;		//�����б�
	int vertexCount;					//������
	struct Edge2D* edgeTable;			//���б�
	int edgeCount;						//����
	int* ringTable;					//ring list   ռ8Byte
	int ringCount;						//ring number + 1

	struct Point2D boundingBox[2];       //��Χ��
	FLTYPE dx, dy;  //ƽ��ÿ������X,Y�����ϵ�ƽ�����ȣ����ڼ�����ѷֱ��ʣ�16Byte

	GCPPolygon2D() { memset(this, 0, sizeof(*this)); }
	~GCPPolygon2D() {
		if (vertexTable != NULL)
		{
			delete[] vertexTable;
			vertexTable = NULL;
		}
		if (edgeTable != NULL)
		{
			delete[] edgeTable;
			edgeTable = NULL;
		}
		if (ringTable != NULL)
		{
			delete[] ringTable;
			ringTable = NULL;
		}
		vertexCount = 0;
		edgeCount = 0;
		ringCount = 0;
	}
};

struct pip
{
	//
	GCPPolygon2D* testedPolygon = NULL;
	Point2D* testedPoint = NULL;
	int testedPointCount;
	int testedPolygonCount;
	Point2D grid_boundingbox[2];
	Point2D testedPointRegion[2];
	int curPolygonIndex = 0;
	int* testedResult = NULL;
	

	//Point2D *discretePoint;
	vector<Point> discretePoint;
	// member function
	int readData(const char* fn, int ftype);
	double calculateDis(Point2D p1, Point2D p2);//edge distance
	double calculateDis(Point p, Coeffecient l);
	double calculateDis_stat(Point p, Coeffecient l);
	double calculateDis(Point source, Point result);
	void edgeDiscretize(double benchmark);
	double findMinEdge();//output benchmark  find min edge distance
	void initData();
	int readTestPoint(const char* fn);
	int getPointAttri(Point2D testP, Point2D edgeStartP, Point2D edgeEndP);     //Gets the properties of the test point inside and outside the polygon
	int getPointAttri_stat(Point2D testP, Point2D edgeStartP, Point2D edgeEndP);

	bool JudgeCollineation(Point2D p1, Point2D p2, Point2D q);
	void exportTestresult(const char* filename);
	Statistic stat_pip = Statistic(0, 0, 0, 0,0,0);
	void PIP_statStorageCost(int length, int widthint, int* basic_cost, int* auxiliary_cost, int* real_memory); //memory cost

	//��knn�ҵ���ɢ��Ϊ����ζ���ʱ����ҪѰ���ٽ��ߣ����ǲ��Զ���κ��пס�����
   //�����Ҫ�����б��н���Ѱ�ң����취
	int findAdjacentVertex(Point result);
	int findAdjacentVertex_stat(Point result);

	Coeffecient getCoeffecient(int inx);
	Coeffecient getCoeffecient_stat(int inx);
	Edge2D getClosestEdge(Point source, Point result);
	Edge2D getClosestEdge_stat(Point source, Point result);

	static bool cmp(Point a, Point b)
	{
		return a.x < b.x;
	}

	static bool equal(Point a, Point b)
	{
		return (a.x == b.x) && (a.y == b.y);
	}


};
struct SmallTimer {
	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER tick0, tick1;
	double time;

	SmallTimer()
	{
		memset(this, 0, sizeof(*this));
		QueryPerformanceFrequency(&ticksPerSecond);
	}
	~SmallTimer() {};

	void start() { QueryPerformanceCounter(&tick0); }
	void end()
	{
		QueryPerformanceCounter(&tick1);
		time = (tick1.QuadPart - tick0.QuadPart) / (double)ticksPerSecond.QuadPart;
	}
};
