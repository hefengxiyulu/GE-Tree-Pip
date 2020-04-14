#include "pch.h"
#include "pip.h"
#include <fstream>
#include <strstream> 
#include <math.h>
#include <algorithm>

//Read data into tested polyon or tested point structure
//type - 0: tested polygon; 1: tested points



int pip::readData(const char* fn, int ftype)
{
	if (fn[0] == 0)
		return -1;

	//������������
	if (ftype == 0)
	{
		//����һ�飬���㶥������������ڴ�
		ifstream ifs;
		//ifs.open(fn, ios::binary);
		ifs.open(fn, ios::binary);
		if (ifs.bad())//����ڶ�д�����г������� true
		{
			printf("Can not open polygon file.\n");
			return -1;
		}
		ifs.seekg(0, ios_base::end);//��ָ���Ƶ��ļ�β
		streampos size = ifs.tellg();//����ǰget ��ָ���λ�� (��tellg) 
		ifs.seekg(0, ios_base::beg);

		int tempsize;
		tempsize = size;
		char* buf = new char[tempsize];
		char type;
		int vertexCount = 0;
		int edgeCount = 0;
		int ringCount = 0;
		int temp;
		bool isFrom0 = false;

		while (ifs.getline(buf, size))
		{
			istrstream s(buf);
			s >> type;
			switch (type)
			{
			case 'v':
				vertexCount++;          // ������ĸ���
				break;
			case 'f':
			{
				while (s >> temp)
				{
					if (temp == 0)
						isFrom0 = true;
					edgeCount++;        //ͳ�Ʊߵĸ���
				}
				ringCount++;
				break;
			}
			};
			type = ' ';
		}

		if (this->testedPolygon != NULL)
		{
			delete this->testedPolygon;
			this->testedPolygon = NULL;
		}
		this->testedPolygon = new struct GCPPolygon2D;
		this->testedPolygonCount = 1;
		this->testedPolygon->vertexCount = vertexCount;
		this->testedPolygon->edgeCount = edgeCount;
		this->testedPolygon->ringCount = ringCount;
		this->testedPolygon->vertexTable = new Point2D[vertexCount];
		this->testedPolygon->edgeTable = new Edge2D[edgeCount];
		this->testedPolygon->ringTable = new int[ringCount];

		//���ڶ��飬д���ڴ�
		ifs.clear();// ȥ�� ifs �еĴ�����(���ļ�ĩβ��ǻ��ȡʧ�ܱ�ǵ�)
		ifs.seekg(0, ios::beg);
		vertexCount = 0;
		edgeCount = 0;
		ringCount = 0;

		while (ifs.getline(buf, size))
		{
			istrstream s(buf);
			s >> type;
			switch (type)
			{
			case 'v':
				s >> this->testedPolygon->vertexTable[vertexCount].x;
				s >> this->testedPolygon->vertexTable[vertexCount].y;
				vertexCount++;
				break;
			case 'f':
			{
				//��һ����
				int temp, vertexIndex;
				s >> temp;
				if (isFrom0 == false)  temp = temp - 1;
				this->testedPolygon->edgeTable[edgeCount].startIndex = temp;
				this->testedPolygon->ringTable[ringCount] = edgeCount;
				ringCount++;
				edgeCount++;

				//������
				while (s >> vertexIndex)
				{
					if (isFrom0 == false)  vertexIndex = vertexIndex - 1;
					this->testedPolygon->edgeTable[edgeCount - 1].endIndex = vertexIndex;
					this->testedPolygon->edgeTable[edgeCount].startIndex = vertexIndex;

					edgeCount++;
				}

				//���һ����
				this->testedPolygon->edgeTable[edgeCount - 1].endIndex = temp;
			}
			}//switch 
			type = ' ';
		}//while	
	} //if
	else if (ftype == 1)
	{
		ifstream ifs2;
		ifs2.open(fn, ios::binary);
		if (ifs2.bad())
		{
			printf("Can not open point file.\n");
			delete this->testedPolygon;
			this->testedPolygon = NULL;
			return -1;
		}
		//����һ�飬��¼�������
		ifs2.seekg(0, ios_base::end);
		streampos size = ifs2.tellg();
		ifs2.seekg(0, ios_base::beg);
		int tempsize;
		tempsize = size;
		char* buf = new char[tempsize];
		char type;
		int vertexCount = 0;
		int edgeCount = 0;
		int temp;

		while (ifs2.getline(buf, size))
		{
			istrstream s(buf);
			s >> type;
			switch (type)
			{
			case 'v':
				vertexCount++;
				break;
			};
			type = ' ';
		}
		//�����ڴ�
		if (this->testedPoint != NULL)
		{
			delete[] this->testedPoint;
			this->testedPoint = NULL;
		}
		this->testedPoint = new Point2D[vertexCount];
		//���ڶ��飬д�붥��
		ifs2.clear();
		ifs2.seekg(0, ios::beg);
		vertexCount = 0;
		while (ifs2.getline(buf, size))
		{
			istrstream s(buf);
			s >> type;
			switch (type)
			{
			case 'v':
				s >> this->testedPoint[vertexCount].x >> this->testedPoint[vertexCount].y;
				vertexCount++;
				break;
			};
			type = ' ';
		}
		this->testedPointCount = vertexCount;
	}

	return 1;
}

double pip::calculateDis(Point2D p1, Point2D p2)
{
	double dis = 0;
	dis = sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
	return dis;
}

double pip::calculateDis(Point p, Coeffecient l) {
	return abs(l.a*p.x + l.b*p.y + l.c) / sqrt(l.a*l.a + l.b*l.b + l.c*l.c);
}

double pip::calculateDis_stat(Point p, Coeffecient l)
{
	stat_pip.cnt_multiply += 5;
	stat_pip.cnt_add += 4;
	return abs(l.a * p.x + l.b * p.y + l.c) / sqrt(l.a * l.a + l.b * l.b + l.c * l.c);
}

double pip::calculateDis(Point source, Point result) {
	double dis[3];
	dis[0] = this->calculateDis(source, this->getCoeffecient(result.edgeIdx));
	(result.edgeIdx > 0) ? dis[1] = this->calculateDis(source, this->getCoeffecient(result.edgeIdx - 1)) : dis[1] = LONG_MAX;
	(result.edgeIdx < this->testedPolygon->edgeCount - 1) ? dis[2] = this->calculateDis(source, this->getCoeffecient(result.edgeIdx - 1)) : dis[2] = LONG_MAX;
	for (int j = 1; j < 3; j++) (dis[j] < dis[0]) ? dis[0] = dis[j] : 0;
	return dis[0];
}

void pip::edgeDiscretize(double benchmark)
{
	int edgeCount = this->testedPolygon->edgeCount;
	for (int i = 0; i < edgeCount; i++)
	{
		int startIdx = this->testedPolygon->edgeTable[i].startIndex;
		int endIdx = this->testedPolygon->edgeTable[i].endIndex;
		Point2D p1, p2;
		p1 = this->testedPolygon->vertexTable[startIdx];
		p2 = this->testedPolygon->vertexTable[endIdx];

		Point tempP;
		double edgeDis = calculateDis(p1, p2);
		if (edgeDis < 1)
		{
			tempP.x = p1.x;
			tempP.y = p1.y;
			tempP.isVertex = true;
			tempP.edgeIdx = i;
			discretePoint.push_back(tempP);

			tempP.x = p2.x;
			tempP.y = p2.y;
			discretePoint.push_back(tempP);
		}
		else
		{
			int pointNum = edgeDis / benchmark;
			double x_step, y_step;
			x_step = (p2.x - p1.x) / pointNum;
			y_step = (p2.y - p1.y) / pointNum;
			//
			tempP.x = p1.x;
			tempP.y = p1.y;
			tempP.isVertex = true;
			tempP.edgeIdx = i;
			discretePoint.push_back(tempP);
			tempP.isVertex = false;
			for (int i = 1; i < pointNum; i++)
			{
				tempP.x = p1.x + i * x_step;
				tempP.y = p1.y + i * y_step;
				discretePoint.push_back(tempP);
			}
			tempP.x = p2.x;
			tempP.y = p2.y;
			tempP.isVertex = true;
			discretePoint.push_back(tempP);
		}
	}
	//delete duplicate data
	sort(discretePoint.begin(), discretePoint.end(), cmp);
	discretePoint.erase(unique(discretePoint.begin(), discretePoint.end(), equal), discretePoint.end());
}

int pip::findAdjacentVertex(Point result)
{
	//��knn�ҵ���ɢ��Ϊ����ζ���ʱ����ҪѰ���ٽ��ߣ����ǲ��Զ���κ��пס���������ͨ���Ӽ�1����ȡ�ٽ���
	//�����Ҫ�����б��н���Ѱ�ң����취
	int adjacentEdgeIdx = 0;
	int edgeCount = this->testedPolygon->edgeCount;
	for (int i = 0; i < edgeCount; i++)
	{
		int startIdx = this->testedPolygon->edgeTable[i].startIndex;
		int endIdx = this->testedPolygon->edgeTable[i].endIndex;
		Point2D p1, p2;
		p1 = this->testedPolygon->vertexTable[startIdx];
		p2 = this->testedPolygon->vertexTable[endIdx];
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

int pip::findAdjacentVertex_stat(Point result)
{
	//��knn�ҵ���ɢ��Ϊ����ζ���ʱ����ҪѰ���ٽ��ߣ����ǲ��Զ���κ��пס���������ͨ���Ӽ�1����ȡ�ٽ���
	//�����Ҫ�����б��н���Ѱ�ң����취
	int adjacentEdgeIdx = 0;
	int edgeCount = this->testedPolygon->edgeCount;
	for (int i = 0; i < edgeCount; i++)
	{
		stat_pip.cnt_compare++;
		stat_pip.cnt_add++;

		int startIdx = this->testedPolygon->edgeTable[i].startIndex;
		int endIdx = this->testedPolygon->edgeTable[i].endIndex;
		Point2D p1, p2;
		p1 = this->testedPolygon->vertexTable[startIdx];
		p2 = this->testedPolygon->vertexTable[endIdx];

		this->stat_pip.cnt_compare++;
		if (i != result.edgeIdx)
		{
			if ((this->stat_pip.cnt_compare++&& this->stat_pip.cnt_compare++&&(p1.x == result.x && p1.y == result.y)) ||
				(this->stat_pip.cnt_compare++ && this->stat_pip.cnt_compare++&&(p2.x == result.x && p2.y == result.y)))
			{
				adjacentEdgeIdx = i;
				return adjacentEdgeIdx;
			}
		}
	}
	this->stat_pip.cnt_compare++;
	return adjacentEdgeIdx;
}

Coeffecient pip::getCoeffecient(int inx) {
	int startIdx = this->testedPolygon->edgeTable[inx].startIndex;
	int endIdx = this->testedPolygon->edgeTable[inx].endIndex;
	Point2D p1, p2;
	p1 = this->testedPolygon->vertexTable[startIdx];
	p2 = this->testedPolygon->vertexTable[endIdx];
	return Coeffecient(p2.y - p1.y, p1.x - p2.x, p1.y - (p2.y - p1.y)*p1.x);
}

Coeffecient pip::getCoeffecient_stat(int inx)
{
	int startIdx = this->testedPolygon->edgeTable[inx].startIndex;
	int endIdx = this->testedPolygon->edgeTable[inx].endIndex;
	Point2D p1, p2;
	p1 = this->testedPolygon->vertexTable[startIdx];
	p2 = this->testedPolygon->vertexTable[endIdx];
	stat_pip.cnt_add += 4;
	stat_pip.cnt_multiply++;
	return Coeffecient(p2.y - p1.y, p1.x - p2.x, p1.y - (p2.y - p1.y) * p1.x);
}

Edge2D pip::getClosestEdge(Point source, Point result) {
	double dis[2];
	int closest_edge_index = result.edgeIdx;
	if (result.isVertex)
	{
		// find adjacent edge
		int front_edge_Idx, back_edge_Idx;
		if (result.edgeIdx == 0)
		{
			front_edge_Idx = this->testedPolygon->edgeCount - 1;
			back_edge_Idx = result.edgeIdx + 1;
		}
		else if (result.edgeIdx == this->testedPolygon->edgeCount - 1)
		{
			front_edge_Idx = result.edgeIdx - 1;
			back_edge_Idx = 0;
		}
		else
		{
			front_edge_Idx = result.edgeIdx - 1;
			back_edge_Idx = result.edgeIdx + 1;
		}
		Point2D front_point, back_point;
		//for front edge
		int endIdx = this->testedPolygon->edgeTable[front_edge_Idx].endIndex;
		front_point = this->testedPolygon->vertexTable[endIdx];
		//for back edge
		int startIdx = this->testedPolygon->edgeTable[back_edge_Idx].startIndex;
		back_point = this->testedPolygon->vertexTable[startIdx];

		dis[0] = this->calculateDis(source, this->getCoeffecient(result.edgeIdx));
		if (front_point.x == result.x&&front_point.y == result.y)
		{
			dis[1] = this->calculateDis(source, this->getCoeffecient(front_edge_Idx));
			(dis[0] < dis[1]) ? closest_edge_index = result.edgeIdx : closest_edge_index = front_edge_Idx;
		}
		else if (back_point.x == result.x&&back_point.y == result.y)
		{
			dis[1] = this->calculateDis(source, this->getCoeffecient(back_edge_Idx));
			(dis[0] < dis[1]) ? closest_edge_index = result.edgeIdx : closest_edge_index = back_edge_Idx;
		}
		else
		{
			int adjacentEdgeIdx = this->findAdjacentVertex(result);
			dis[1] = this->calculateDis(source, this->getCoeffecient(adjacentEdgeIdx));
			(dis[0] < dis[1]) ? closest_edge_index = result.edgeIdx : closest_edge_index = adjacentEdgeIdx;
			//cout << "An error occurred while looking for adjacent edges" << endl;
		}
		//dis[0] = this->calculateDis(source, this->getCoeffecient(result.edgeIdx));
		//(result.edgeIdx > 0) ? dis[1] = this->calculateDis(source, this->getCoeffecient(result.edgeIdx - 1)) :
		//	dis[1] = this->calculateDis(source, this->getCoeffecient(this->testedPolygon->edgeCount - 1));
		//(result.edgeIdx < this->testedPolygon->edgeCount - 1) ? dis[2] = this->calculateDis(source, this->getCoeffecient(result.edgeIdx - 1)) :
		//	dis[2] = this->calculateDis(source, this->getCoeffecient(0));
		//for (int j = 1; j < 3; j++) (dis[j] < dis[0]) ? closest_edge_index += (j * 2 - 3) : 0;
	}
	return this->testedPolygon->edgeTable[closest_edge_index];
}

Edge2D pip::getClosestEdge_stat(Point source, Point result)
{
	double dis[2];
	int closest_edge_index = result.edgeIdx;
	stat_pip.cnt_compare++;
	if (result.isVertex)
	{
		// find adjacent edge
		int front_edge_Idx, back_edge_Idx;
		stat_pip.cnt_compare++;
		if (result.edgeIdx == 0)
		{
			front_edge_Idx = this->testedPolygon->edgeCount - 1;
			back_edge_Idx = result.edgeIdx + 1;
			stat_pip.cnt_add += 2;
		}
		else if (stat_pip.cnt_compare++&& stat_pip.cnt_add++&&(result.edgeIdx == this->testedPolygon->edgeCount - 1))
		{
			front_edge_Idx = result.edgeIdx - 1;
			back_edge_Idx = 0;
			stat_pip.cnt_add++;
		}
		else
		{
			front_edge_Idx = result.edgeIdx - 1;
			back_edge_Idx = result.edgeIdx + 1;
			stat_pip.cnt_add += 2;
		}
		Point2D front_point, back_point;
		//for front edge
		int endIdx = this->testedPolygon->edgeTable[front_edge_Idx].endIndex;
		front_point = this->testedPolygon->vertexTable[endIdx];
		//for back edge
		int startIdx = this->testedPolygon->edgeTable[back_edge_Idx].startIndex;
		back_point = this->testedPolygon->vertexTable[startIdx];

		dis[0] = this->calculateDis_stat(source, this->getCoeffecient_stat(result.edgeIdx));
		if (stat_pip.cnt_compare++ && (front_point.x == result.x)&& stat_pip.cnt_compare++ && (front_point.y == result.y))
		{
			dis[1] = this->calculateDis_stat(source, this->getCoeffecient_stat(front_edge_Idx));
			(dis[0] < dis[1]) ? closest_edge_index = result.edgeIdx : closest_edge_index = front_edge_Idx;
			stat_pip.cnt_compare++;
		}
		else if (stat_pip.cnt_compare++&&(back_point.x == result.x)
			&& stat_pip.cnt_compare++&&( back_point.y == result.y))
		{
			dis[1] = this->calculateDis_stat(source, this->getCoeffecient_stat(back_edge_Idx));
			(dis[0] < dis[1]) ? closest_edge_index = result.edgeIdx : closest_edge_index = back_edge_Idx;
			stat_pip.cnt_compare++;
		}
		else
		{
			int adjacentEdgeIdx = this->findAdjacentVertex_stat(result);
			dis[1] = this->calculateDis_stat(source, this->getCoeffecient_stat(adjacentEdgeIdx));
			(dis[0] < dis[1]) ? closest_edge_index = result.edgeIdx : closest_edge_index = adjacentEdgeIdx;
			stat_pip.cnt_compare++;
			//cout << "An error occurred while looking for adjacent edges" << endl;
		}
	}
	return this->testedPolygon->edgeTable[closest_edge_index];
}

double pip::findMinEdge()
{
	double minEdge = LLONG_MAX;
	//int minEdgeIdx;
	int edgeCount = this->testedPolygon->edgeCount;
	for (int i = 0; i < edgeCount; i++)
	{
		int startIdx = this->testedPolygon->edgeTable[i].startIndex;
		int endIdx = this->testedPolygon->edgeTable[i].endIndex;
		Point2D p1, p2;
		p1 = this->testedPolygon->vertexTable[startIdx];
		p2 = this->testedPolygon->vertexTable[endIdx];

		double edgeDis = calculateDis(p1, p2);
		if (edgeDis < minEdge)
		{
			minEdge = edgeDis;
		}
	}
	return minEdge;
}


//�������ݺ󣬼���һЩ��������
void pip::initData()
{
	if (this->testedPolygon == NULL)
		return;

	//�������ΰ�Χ��
	for (int i = 0; i < this->testedPolygonCount; i++)
	{
		this->testedPolygon[i].boundingBox[0].x = this->testedPolygon[i].vertexTable[0].x;
		this->testedPolygon[i].boundingBox[0].y = this->testedPolygon[i].vertexTable[0].y;
		this->testedPolygon[i].boundingBox[1].x = this->testedPolygon[i].vertexTable[0].x;
		this->testedPolygon[i].boundingBox[1].y = this->testedPolygon[i].vertexTable[0].y;
		for (int j = 0; j < this->testedPolygon[i].vertexCount; j++)//��ȡ���б��������С����ֵ
		{
			if (this->testedPolygon[i].vertexTable[j].x < this->testedPolygon[i].boundingBox[0].x)
				this->testedPolygon[i].boundingBox[0].x = this->testedPolygon[i].vertexTable[j].x;
			if (this->testedPolygon[i].vertexTable[j].y < this->testedPolygon[i].boundingBox[0].y)
				this->testedPolygon[i].boundingBox[0].y = this->testedPolygon[i].vertexTable[j].y;
			if (this->testedPolygon[i].vertexTable[j].x > this->testedPolygon[i].boundingBox[1].x)
				this->testedPolygon[i].boundingBox[1].x = this->testedPolygon[i].vertexTable[j].x;
			if (this->testedPolygon[i].vertexTable[j].y > this->testedPolygon[i].boundingBox[1].y)
				this->testedPolygon[i].boundingBox[1].y = this->testedPolygon[i].vertexTable[j].y;
		}

		//����X,Y�����ϵ�ƽ���߳�
		this->testedPolygon[i].dx = this->testedPolygon[i].dy = 0;
		for (int j = 0; j < this->testedPolygon[i].edgeCount; j++)
		{
			int start = this->testedPolygon[i].edgeTable[j].startIndex;
			int end = this->testedPolygon[i].edgeTable[j].endIndex;
			FLTYPE x0 = this->testedPolygon[i].vertexTable[start].x;
			FLTYPE x1 = this->testedPolygon[i].vertexTable[end].x;
			FLTYPE y0 = this->testedPolygon[i].vertexTable[start].y;
			FLTYPE y1 = this->testedPolygon[i].vertexTable[end].y;
			this->testedPolygon[i].dx += abs(x1 - x0);
			this->testedPolygon[i].dy += abs(y1 - y0);
		}
		this->testedPolygon[i].dx /= this->testedPolygon[i].edgeCount;//�������б�x���ֵ�ĺͣ�����ƽ��ֵ
		this->testedPolygon[i].dy /= this->testedPolygon[i].edgeCount;//�������б�y���ֵ�ĺͣ�����ƽ��ֵ

		//��������ȡ�����б��������С����ֵ���������Χ�ɵķ�������Ͻǡ����½����꣩�������Ե�����
		this->testedPointRegion[0].x = this->testedPolygon[0].boundingBox[0].x;
		this->testedPointRegion[0].y = this->testedPolygon[0].boundingBox[0].y;
		this->testedPointRegion[1].x = this->testedPolygon[0].boundingBox[1].x;
		this->testedPointRegion[1].y = this->testedPolygon[0].boundingBox[1].y;;
	}

	//���������С��Ϊ���㷽��ȶ���ΰ�Χ���Դ�(0.1%��
	FLTYPE enlarge[2];
	enlarge[0] = (this->testedPolygon[curPolygonIndex].boundingBox[1].x - this->testedPolygon[curPolygonIndex].boundingBox[0].x) * 0.01;//Դ����ô�Ϊ0.001
	enlarge[1] = (this->testedPolygon[curPolygonIndex].boundingBox[1].y - this->testedPolygon[curPolygonIndex].boundingBox[0].y) * 0.01;//ͬ��

	this->grid_boundingbox[0].x = this->testedPolygon[curPolygonIndex].boundingBox[0].x - enlarge[0];
	this->grid_boundingbox[0].y = this->testedPolygon[curPolygonIndex].boundingBox[0].y - enlarge[1];
	this->grid_boundingbox[1].x = this->testedPolygon[curPolygonIndex].boundingBox[1].x + enlarge[0];
	this->grid_boundingbox[1].y = this->testedPolygon[curPolygonIndex].boundingBox[1].y + enlarge[1];
} //initData

int pip::readTestPoint(const char* fn)
{
	if (fn[0] == 0)
		return -1;
	ifstream ifs;
	ifs.open(fn, ios::binary);
	if (ifs.bad())//����ڶ�д�����г������� true
	{
		printf("Can not open polygon file.\n");
		delete this->testedPolygon;
		this->testedPolygon = NULL;
		return -1;
	}
	ifs.seekg(0, ios_base::end);//��ָ���Ƶ��ļ�β
	streampos size = ifs.tellg();//����ǰget ��ָ���λ�� (��tellg)   sizeΪ��λָ�룬��Ϊ�����ļ�������������Ҳ�����ļ��Ĵ�С
	ifs.seekg(0, ios_base::beg);

	int tempsize;
	tempsize = size;
	char* buf = new char[tempsize];
	char type = 'p';
	int pointCount = 0;
	int edgeCount = 0;
	int temp;

	while (ifs.getline(buf, size))
	{
		istrstream s(buf);
		s >> type;
		switch (type)
		{
		case 'p':
			pointCount++;
			break;
		};
		type = ' ';
	}
	//�����ڴ�
	if (this->testedPoint != NULL)
	{
		delete[] this->testedPoint;
		this->testedPoint = NULL;
	}
	if (this->testedResult != NULL)
	{
		delete[] this->testedResult;
		this->testedResult = NULL;
	}
	this->testedResult = new int[pointCount];
	this->testedPoint = new Point2D[pointCount];

	ifs.clear();
	ifs.seekg(0, ios::beg);
	pointCount = 0;
	while (ifs.getline(buf, size))
	{
		istrstream s(buf);
		s >> type;
		switch (type)
		{
		case 'p':
			s >> this->testedPoint[pointCount].x >> this->testedPoint[pointCount].y;
			pointCount++;
			break;
		};
		type = ' ';
	}
	this->testedPointCount = pointCount;
	return 0;
}

int pip::getPointAttri(Point2D testP, Point2D edgeStartP, Point2D edgeEndP)
{
	//test point on the edge
	bool isCollineation = JudgeCollineation(edgeStartP, edgeEndP, testP);
	if (isCollineation)
	{
		return INSIDE;
	}
	//double Tmp = (edgeStartP.y - edgeEndP.y) * testP.x + (edgeEndP.x - edgeStartP.x) * testP.y + 
	//	edgeStartP.x * edgeEndP.y - edgeEndP.x * edgeStartP.y;
	//
	Point a, b;
	a.x = edgeEndP.x - edgeStartP.x;
	a.y = edgeEndP.y - edgeStartP.y;
	b.x = testP.x - edgeStartP.x;
	b.y = testP.y - edgeStartP.y;
	double product = a.x * b.y - a.y * b.x;

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
		//collineation �ӳ��� ���״����
		return OUTSIDE;
	}
}

int pip::getPointAttri_stat(Point2D testP, Point2D edgeStartP, Point2D edgeEndP)
{
	//test point on the edge
	bool isCollineation = JudgeCollineation(edgeStartP, edgeEndP, testP);
	stat_pip.cnt_add += 5;
	stat_pip.cnt_multiply += 2;
	stat_pip.cnt_compare += 9;
	
	stat_pip.cnt_compare++;
	if (isCollineation)
	{
		return INSIDE;
	}
	//double Tmp = (edgeStartP.y - edgeEndP.y) * testP.x + (edgeEndP.x - edgeStartP.x) * testP.y + 
	//	edgeStartP.x * edgeEndP.y - edgeEndP.x * edgeStartP.y;
	//
	Point a, b;
	a.x = edgeEndP.x - edgeStartP.x;
	a.y = edgeEndP.y - edgeStartP.y;
	b.x = testP.x - edgeStartP.x;
	b.y = testP.y - edgeStartP.y;
	double product = a.x * b.y - a.y * b.x;
	stat_pip.cnt_add += 5;
	stat_pip.cnt_multiply += 2;

	if (stat_pip.cnt_compare++&&(product > 0))
	{
		return INSIDE;//right side
	}
	else if (stat_pip.cnt_compare++ && (product < 0))
	{
		return OUTSIDE;  //left side
	}
	else
	{
		//collineation �ӳ��� ���״����
		return OUTSIDE;
	}
}

bool pip::JudgeCollineation(Point2D p1, Point2D p2, Point2D q)
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

void pip::exportTestresult(const char* filename)
{
	if (filename == NULL)
		return;
	FILE* fp;
	fopen_s(&fp, filename, "w");
	for (int i = 0; i < this->testedPointCount; i++)
		fprintf(fp, "p %d %d \n", i, this->testedResult[i]);
}

void pip::PIP_statStorageCost(long long int* basic_cost, long long int* auxiliary_cost)
{
	//length ��width������ֱ��� *basic_cost  �洢����Ρ�����㿪��   *auxiliary_cost �������ṹ����  *real_memoryΪ��ȷ�ڴ�ռ�ͳ��
	//�����ṹ
	int temp;
	if (this->testedPolygon == NULL)
		return;
	temp = sizeof(GCPPolygon2D);                                                        //96�ֽ� ��16�ֽ�
	temp = sizeof(struct Point2D);                                                          //16�ֽ�
	temp = sizeof(struct Edge2D);                                                          //8�ֽ�

	*basic_cost = sizeof(GCPPolygon2D) +                                         //168
		sizeof(struct Point2D) * this->testedPolygon->vertexCount +
		sizeof(struct Edge2D) * this->testedPolygon->edgeCount - 16;

	//discrete point memory
	int discrete_num = discretePoint.size();
	int discrete_point_memory = sizeof(Point) * discrete_num;
	
	*auxiliary_cost = discrete_point_memory;
}
