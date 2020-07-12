
#include <iostream>                 //��׼�������ͷ�ļ�
#include <pcl/io/pcd_io.h>          //I/O����ͷ�ļ�
#include <pcl/point_types.h>        //�����Ͷ���ͷ�ļ�
#define BOOST_TYPEOF_EMULATION  //Ҫ����#include <pcl/registration/icp.h>ǰ
#include <pcl/registration/icp.h>   //ICP��׼�����ͷ�ļ�
#include <pcl/visualization/cloud_viewer.h>//���Ʋ鿴����ͷ�ļ�
#include <pcl/visualization/pcl_visualizer.h>//���ӻ�ͷ�ļ�

typedef pcl::PointXYZ PointT;
typedef pcl::PointCloud<PointT> PointCloudT;

bool next_iteration = false;

void print4x4Matrix(const Eigen::Matrix4d & matrix)    //��ӡ��ת�����ƽ�ƾ���
{
	printf("Rotation matrix :\n");
	printf("    | %6.3f %6.3f %6.3f | \n", matrix(0, 0), matrix(0, 1), matrix(0, 2));
	printf("R = | %6.3f %6.3f %6.3f | \n", matrix(1, 0), matrix(1, 1), matrix(1, 2));
	printf("    | %6.3f %6.3f %6.3f | \n", matrix(2, 0), matrix(2, 1), matrix(2, 2));
	printf("Translation vector :\n");
	printf("t = < %6.3f, %6.3f, %6.3f >\n\n", matrix(0, 3), matrix(1, 3), matrix(2, 3));
}

void KeyboardEventOccurred(const pcl::visualization::KeyboardEvent& event, void* nothing)
{  //ʹ�ÿո�������ӵ�����������������ʾ
	if (event.getKeySym() == "space" && event.keyDown())
		next_iteration = true;
}


int main(int argc, char** argv)
{
	// ��������ָ��
	PointCloudT::Ptr cloud_in(new PointCloudT);  // ԭʼ����
	PointCloudT::Ptr cloud_icp(new PointCloudT);  // ICP �������
	PointCloudT::Ptr cloud_tr(new PointCloudT);  // ƥ�����

	//��ȡpcd�ļ�
	if (pcl::io::loadPCDFile<pcl::PointXYZ>("depth_0.pcd", *cloud_in) == -1)
	{
		PCL_ERROR("Couldn't read file1 \n");
		return (-1);
	}
	std::cout << "Loaded " << cloud_in->size() << " data points from file1" << std::endl;

	if (pcl::io::loadPCDFile<pcl::PointXYZ>("depth_4.pcd", *cloud_icp) == -1)
	{
		PCL_ERROR("Couldn't read file2 \n");
		return (-1);
	}
	std::cout << "Loaded " << cloud_icp->size() << " data points from file2" << std::endl;

	int iterations = 5;  // Ĭ�ϵ�ICP��������
	*cloud_tr = *cloud_icp;
	//icp��׼
	pcl::IterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ> icp; //����ICP��������ICP��׼
	icp.setMaximumIterations(iterations);    //��������������iterations=true
	icp.setInputCloud(cloud_tr); //�����������
	icp.setInputTarget(cloud_in); //����Ŀ����ƣ�������ƽ��з���任���õ�Ŀ����ƣ�
	icp.align(*cloud_icp);          //ƥ���Դ����
	//pcl::io::savePLYFile("Final.ply",*cloud_icp); //����
	icp.setMaximumIterations(1);  // ����Ϊ1�Ա��´ε���
	std::cout << "Applied " << iterations << " ICP iteration(s)" << std::endl;
	if (icp.hasConverged())//icp.hasConverged ()=1��true������任������ʺ�������
	{
		std::cout << "\nICP has converged, score is " << icp.getFitnessScore() << std::endl;
		std::cout << "\nICP transformation " << iterations << " : cloud_icp -> cloud_in" << std::endl;
	}
	else
	{
		PCL_ERROR("\nICP has not converged.\n");
		return (-1);
	}


	//���ӻ�
	pcl::visualization::PCLVisualizer viewer("ICP demo");
	// ���������۲��ӵ�
	int v1(0);
	int v2(1);
	viewer.createViewPort(0.0, 0.0, 0.5, 1.0, v1);
	viewer.createViewPort(0.5, 0.0, 1.0, 1.0, v2);
	// ������ʾ����ɫ��Ϣ
	float bckgr_gray_level = 0.0;  // Black
	float txt_gray_lvl = 1.0 - bckgr_gray_level;
	// ԭʼ�ĵ�������Ϊ��ɫ��
	pcl::visualization::PointCloudColorHandlerCustom<PointT> cloud_in_color_h(cloud_in, (int)255 * txt_gray_lvl, (int)255 * txt_gray_lvl,
		(int)255 * txt_gray_lvl);

	viewer.addPointCloud(cloud_in, cloud_in_color_h, "cloud_in_v1", v1);//����ԭʼ�ĵ��ƶ�����ʾΪ��ɫ
	viewer.addPointCloud(cloud_in, cloud_in_color_h, "cloud_in_v2", v2);

	// ת����ĵ�����ʾΪ��ɫ
	pcl::visualization::PointCloudColorHandlerCustom<PointT> cloud_tr_color_h(cloud_tr, 20, 180, 20);
	viewer.addPointCloud(cloud_tr, cloud_tr_color_h, "cloud_tr_v1", v1);

	// ICP��׼��ĵ���Ϊ��ɫ
	pcl::visualization::PointCloudColorHandlerCustom<PointT> cloud_icp_color_h(cloud_icp, 180, 20, 20);
	viewer.addPointCloud(cloud_icp, cloud_icp_color_h, "cloud_icp_v2", v2);

	// �����ı��������ڸ��Ե��ӿڽ���
	//��ָ���ӿ�viewport=v1����ַ�����white ������������"icp_info_1"������ַ�����ID��־����10��15��Ϊ����16Ϊ�ַ���С ����ֱ���RGBֵ
	viewer.addText("White: Original point cloud\nGreen: Matrix transformed point cloud", 10, 15, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "icp_info_1", v1);
	viewer.addText("White: Original point cloud\nRed: ICP aligned point cloud", 10, 15, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "icp_info_2", v2);

	std::stringstream ss;
	ss << iterations;            //����ĵ����Ĵ���
	std::string iterations_cnt = "ICP iterations = " + ss.str();
	viewer.addText(iterations_cnt, 10, 60, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "iterations_cnt", v2);

	// ���ñ�����ɫ
	viewer.setBackgroundColor(bckgr_gray_level, bckgr_gray_level, bckgr_gray_level, v1);
	viewer.setBackgroundColor(bckgr_gray_level, bckgr_gray_level, bckgr_gray_level, v2);

	// �������������ͷ���
	viewer.setCameraPosition(-3.68332, 2.94092, 5.71266, 0.289847, 0.921947, -0.256907, 0);
	viewer.setSize(1280, 1024);  // ���ӻ����ڵĴ�С

	// ע�ᰴ���ص�����
	viewer.registerKeyboardCallback(&KeyboardEventOccurred, (void*)NULL);

	//��ʾ
	while (!viewer.wasStopped())
	{
		viewer.spinOnce();

		//���¿ո���ĺ���
		if (next_iteration)
		{
			// ���������㷨
			icp.align(*cloud_icp);
			if (icp.hasConverged())
			{
				printf("\033[11A");  // Go up 11 lines in terminal output.
				printf("\nICP has converged, score is %+.0e\n", icp.getFitnessScore());
				std::cout << "\nICP transformation " << ++iterations << " : cloud_icp -> cloud_in" << std::endl;

				ss.str("");
				ss << iterations;
				std::string iterations_cnt = "ICP iterations = " + ss.str();
				viewer.updateText(iterations_cnt, 10, 60, 16, txt_gray_lvl, txt_gray_lvl, txt_gray_lvl, "iterations_cnt");
				viewer.updatePointCloud(cloud_icp, cloud_icp_color_h, "cloud_icp_v2");
			}
			else
			{
				PCL_ERROR("\nICP has not converged.\n");
				return (-1);
			}
		}
		next_iteration = false;
	}


	return 0;
}