/**************************************************************************
** File name	: RV_PointCloud.h
** Author		: ʯ����
** Date			: 2020-2-26
** Description	: ���Ƽ����߽ṹ�嶨��
**
** History		:
** �޸�:		���ǣ����㷨�����ɺ����㷨���ڲ����:2020.2.26
** �޸�:		���ǣ���RV_Normals�ṹ������_Normals(int num)����:2020.3.2
** �޸�:		���ǣ���RV_PointCloud�����Ӹ����ӿں�����2020.3.17
** �޸�:		���ǣ�����RV_PointCloudPtr�ӿڣ�����memoryͷ�ļ���2020.3.18
**************************************************************************/
#ifndef _RV_POINTS3D_H_
#define _RV_POINTS3D_H_
//���������
#include<vector>
#include <memory>
	/*
	���ܣ���ά���ƽṹRV_PointXYZ
	*/
typedef struct _Point3D
{
	float x;
	float y;
	float z;
	_Point3D(float a, float b, float c) { x = a; y = b; z = c; }
	_Point3D() { x = 0; y = 0; z = 0; }
	/*
	���ܣ�����ָ��ά���µ�����ֵ
	*/
	float operator () (int dim)
	{
		switch (dim)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			return std::numeric_limits<double>::quiet_NaN();
		}
	}
	/*
	���ܣ�����ָ��ά���µ�����ֵ
	*/
	float operator [] (int dim)
	{
		switch (dim)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			return std::numeric_limits<double>::quiet_NaN();
		}
	}
} RV_PointXYZ;
/*
���ܣ����Ʒ���RV_Normal
*/
typedef struct _Normal
{
	float nx;
	float ny;
	float nz;
	float curvature;
	_Normal(float a, float b, float c, float curv) { nx = a; ny = b; nz = c; curvature = curv; }
	_Normal() { nx = 0; ny = 0; nz = 0; curvature = 0; }

	float operator [] (int dim)
	{
		switch (dim)
		{
		case 0:
			return nx;
		case 1:
			return ny;
		case 2:
			return nz;
		case 3:
			return curvature;
		default:
			return std::numeric_limits<double>::quiet_NaN();
		}
	}
} RV_Normal;


/*
���ܣ����Ƽ���RV_PointCloud
*/
typedef struct _PointCloud
{
	std::vector<RV_PointXYZ> points;
	int size;
	int width;
	int height;//����������ƣ�height>1Ϊ������ƣ�height=1Ϊ�������
	_PointCloud() { points.resize(0); size = 0; width = 1; height = size; }
	_PointCloud(int num) { points.resize(num); size = num; width = 1; height = size; }
	//std::unique_ptr<std::vector<RV_PointXYZ>> ptr;
	void resize(int num) { points.resize(num); size = num; }
	void reserve(int num) { points.reserve(num); }
	void push_back(RV_PointXYZ pnt) { size++; points.push_back(pnt); }
	RV_PointXYZ& operator [] (size_t idx) { return points[idx]; }
	//�����ӿں���,����nanoflann��,ʹ�ÿ��Խ���kd��
	size_t kdtree_get_point_count() const { return points.size(); }
	//�����ӿں���,����nanoflann��,ʹ�ÿ��Խ���kd��
	float kdtree_get_pt(const size_t idx, const size_t dim) const
	{
		if (dim == 0) return points[idx].x;
		else if (dim == 1) return points[idx].y;
		else if (dim == 2) return points[idx].z;
		return 0.f;
	}
	//�����ӿں���,����nanoflann��,ʹ�ÿ��Խ���kd��
	template <class BBOX>
	bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }
} RV_PointCloud;
/*
���ܣ�����ָ��
*/
typedef std::shared_ptr<RV_PointCloud> RV_PointCloudPtr;
/*
���ܣ����Ʒ��߼���RV_Normals
*/
typedef struct _Normals
{
	std::vector<RV_Normal> normals;
	int size;
	int width;
	int height;//����������ƣ�height>1Ϊ������ƣ�height=1Ϊ�������
	_Normals() { normals.resize(0); size = 0; width = 1; height = size; }
	_Normals(int num) { normals.resize(num); size = num; width = 1; height = size; }
	void resize(int num) { normals.resize(num); size = num; };
} RV_Normals;


#endif // !_RV_POINTS3D_H_

